/*  NAME:
 Q3TrackerXPC.mm

 DESCRIPTION:
 XPC-based implementation of Quesa tracker.
 Replaces the deprecated PDO implementation.

    COPYRIGHT:
        Copyright (c) 2011-2026, Quesa Developers. All rights reserved.

        For the current release of Quesa, please see:

            <https://github.com/jwwalker/Quesa>

        For the current release of Quesa including 3D device support,
        please see: <https://github.com/h-haris/Quesa>

        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions
        are met:

            o Redistributions of source code must retain the above copyright
              notice, this list of conditions and the following disclaimer.

            o Redistributions in binary form must reproduce the above
              copyright notice, this list of conditions and the following
              disclaimer in the documentation and/or other materials provided
              with the distribution.

            o Neither the name of Quesa nor the names of its contributors
              may be used to endorse or promote products derived from this
              software without specific prior written permission.

        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
        "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
        LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
        A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
        OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
        SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
        TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
        PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
        LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
        NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
        SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    ___________________________________________________________________________
 */

#import <Foundation/Foundation.h>

// Only compile this file for XPC builds
#if defined(QUESA_USE_XPC)

#import "Q3TrackerXPC.h"

// ============================================================================
// Q3TrackerXPC registry
// ============================================================================

static NSMutableDictionary<NSString *, Q3TrackerXPC *> *sTrackerObjects = nil;

void Q3TrackerXPC_Register(NSString *uuid, Q3TrackerXPC *tracker) {
    if (!sTrackerObjects) sTrackerObjects = [NSMutableDictionary dictionary];
    sTrackerObjects[uuid] = tracker;
}

void Q3TrackerXPC_Unregister(NSString *uuid) {
    [sTrackerObjects removeObjectForKey:uuid];
}

NSXPCListenerEndpoint *Q3TrackerXPC_EndpointForUUID(NSString *uuid) {
    return sTrackerObjects[uuid].listenerEndpoint;
}

Q3TrackerXPC * _Nullable Q3TrackerXPC_ForUUID(NSString *uuid) {
    return sTrackerObjects[uuid];
}

// ============================================================================
// Q3TrackerXPC implementation
// ============================================================================

// Event record stored in the tracker's event FIFO buffer
typedef struct {
    TQ3Uns32 timestamp;
    TQ3Uns32 buttons;
    TQ3Point3D position;
    TQ3Quaternion orientation;
} Q3TrackerEventRecord;

@implementation Q3TrackerXPC {
    NSXPCListener *_listener;
    NSMutableArray<NSData *> *_eventBuffer; // ordered FIFO, sorted by timestamp
}

@synthesize listenerEndpoint = _listenerEndpoint;

- (instancetype)initWithUUID:(NSString *)uuid
                  notifyFunc:(TQ3TrackerNotifyFunc)func
               trackerObject:(TQ3Object)obj
{
    if (self = [super init])
    {
        _trackerUUID = [uuid copy];
        _notifyFunc = func;
        _trackerObject = obj;
        _isActive = kQ3False;
        _theButtons = 0;
        _position = (TQ3Point3D){0.0f, 0.0f, 0.0f};
        _orientation = (TQ3Quaternion){1.0f, 0.0f, 0.0f, 0.0f};
        _positionSerialNumber = 1;
        _orientationSerialNumber = 1;
        _positionThreshold = 0.0f;
        _orientationThreshold = 0.0f;

        _eventBuffer = [NSMutableArray array];

        _listener = [NSXPCListener anonymousListener];
        _listener.delegate = self;
        [_listener resume];
        _listenerEndpoint = _listener.endpoint;
    }
    return self;
}

- (void)addEventTimestamp:(TQ3Uns32)ts
                  buttons:(TQ3Uns32)btns
                 position:(const TQ3Point3D *)pos
              orientation:(const TQ3Quaternion *)orient
{
    Q3TrackerEventRecord rec;
    rec.timestamp = ts;
    rec.buttons = btns;
    rec.position = pos ? *pos : (TQ3Point3D){0.0f, 0.0f, 0.0f};
    rec.orientation = orient ? *orient : (TQ3Quaternion){1.0f, 0.0f, 0.0f, 0.0f};
    [_eventBuffer addObject:[NSData dataWithBytes:&rec length:sizeof(rec)]];
}

// Returns kQ3Success if an event with timestamp <= ts is found; removes it from buffer.
- (TQ3Status)getEventAtOrBeforeTimestamp:(TQ3Uns32)ts
                                 buttons:(TQ3Uns32 *)btns
                                position:(TQ3Point3D *)pos
                             orientation:(TQ3Quaternion *)orient
{
    for (NSUInteger i = 0; i < _eventBuffer.count; i++) {
        Q3TrackerEventRecord rec;
        [_eventBuffer[i] getBytes:&rec length:sizeof(rec)];
        if (rec.timestamp <= ts) {
            if (btns)   *btns = rec.buttons;
            if (pos)    *pos  = rec.position;
            if (orient) *orient = rec.orientation;
            [_eventBuffer removeObjectAtIndex:i];
            return kQ3Success;
        }
    }
    return kQ3Failure;
}

#pragma mark - NSXPCListenerDelegate

- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)conn
{
    conn.exportedInterface = [NSXPCInterface interfaceWithProtocol:@protocol(Q3XPCTracker)];
    conn.exportedObject = self;
    [conn resume];
    return YES;
}

#pragma mark - Q3XPCTracker Protocol

- (void)callNotificationWithController:(NSString *)controllerUUID reply:(void (^)(TQ3Status))reply
{
    if (_notifyFunc && _trackerObject)
    {
        _notifyFunc(_trackerObject, (__bridge TQ3ControllerRef)controllerUUID);
    }
    reply(kQ3Success);
}

- (void)setActivation:(TQ3Boolean)active reply:(void (^)(TQ3Status))reply
{
    _isActive = active;
    reply(kQ3Success);
}

- (void)getActivationWithReply:(void (^)(TQ3Boolean, TQ3Status))reply
{
    reply(_isActive, kQ3Success);
}

- (void)changeButtonsWithController:(NSString *)controllerUUID
                            buttons:(TQ3Uns32)theButtons
                         buttonMask:(TQ3Uns32)aButtonMask
                              reply:(void (^)(TQ3Status))reply
{
    TQ3Uns32 newButtons = (_theButtons & ~aButtonMask) | (theButtons & aButtonMask);
    BOOL changed = (newButtons != _theButtons);
    _theButtons = newButtons;

    if (changed && _notifyFunc && _trackerObject)
    {
        _notifyFunc(_trackerObject, (__bridge TQ3ControllerRef)controllerUUID);
    }

    reply(kQ3Success);
}

- (void)getPositionWithReply:(void (^)(TQ3Uns32, TQ3Point3D, TQ3Vector3D, TQ3Boolean, TQ3Status))reply
{
    TQ3Vector3D zeroDelta = {0.0f, 0.0f, 0.0f};
    if (_isActive == kQ3True)
    {
        reply(_positionSerialNumber, _position, zeroDelta, kQ3False, kQ3Success);
    }
    else
    {
        TQ3Point3D zeroPos = {0.0f, 0.0f, 0.0f};
        reply(0, zeroPos, zeroDelta, kQ3False, kQ3Success);
    }
}

- (void)setPositionWithController:(NSString *)controllerUUID
                         position:(TQ3Point3D)aPosition
                            reply:(void (^)(TQ3Status))reply
{
    _position = aPosition;
    _positionSerialNumber++;

    if (_notifyFunc && _trackerObject)
    {
        _notifyFunc(_trackerObject, (__bridge TQ3ControllerRef)controllerUUID);
    }

    reply(kQ3Success);
}

- (void)movePositionWithController:(NSString *)controllerUUID
                             delta:(TQ3Vector3D)aDelta
                             reply:(void (^)(TQ3Status))reply
{
    _position.x += aDelta.x;
    _position.y += aDelta.y;
    _position.z += aDelta.z;
    _positionSerialNumber++;

    if (_notifyFunc && _trackerObject)
    {
        _notifyFunc(_trackerObject, (__bridge TQ3ControllerRef)controllerUUID);
    }

    reply(kQ3Success);
}

- (void)getOrientationWithReply:(void (^)(TQ3Uns32, TQ3Quaternion, TQ3Quaternion, TQ3Boolean, TQ3Status))reply
{
    TQ3Quaternion identityDelta = {1.0f, 0.0f, 0.0f, 0.0f};
    if (_isActive == kQ3True)
    {
        reply(_orientationSerialNumber, _orientation, identityDelta, kQ3False, kQ3Success);
    }
    else
    {
        TQ3Quaternion identityOrient = {1.0f, 0.0f, 0.0f, 0.0f};
        reply(0, identityOrient, identityDelta, kQ3False, kQ3Success);
    }
}

- (void)setOrientationWithController:(NSString *)controllerUUID
                         orientation:(TQ3Quaternion)anOrientation
                               reply:(void (^)(TQ3Status))reply
{
    _orientation = anOrientation;
    _orientationSerialNumber++;

    if (_notifyFunc && _trackerObject)
    {
        _notifyFunc(_trackerObject, (__bridge TQ3ControllerRef)controllerUUID);
    }

    reply(kQ3Success);
}

- (void)moveOrientationWithController:(NSString *)controllerUUID
                                delta:(TQ3Quaternion)aDelta
                                reply:(void (^)(TQ3Status))reply
{
    // Quaternion multiply: new = aDelta * current
    TQ3Quaternion cur = _orientation;
    _orientation.w = aDelta.w*cur.w - aDelta.x*cur.x - aDelta.y*cur.y - aDelta.z*cur.z;
    _orientation.x = aDelta.w*cur.x + aDelta.x*cur.w + aDelta.y*cur.z - aDelta.z*cur.y;
    _orientation.y = aDelta.w*cur.y - aDelta.x*cur.z + aDelta.y*cur.w + aDelta.z*cur.x;
    _orientation.z = aDelta.w*cur.z + aDelta.x*cur.y - aDelta.y*cur.x + aDelta.z*cur.w;
    _orientationSerialNumber++;

    if (_notifyFunc && _trackerObject)
    {
        _notifyFunc(_trackerObject, (__bridge TQ3ControllerRef)controllerUUID);
    }

    reply(kQ3Success);
}

@end

#endif // QUESA_USE_XPC
