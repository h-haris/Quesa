/*  NAME:
 Q3DcontrollerXPC.mm

 DESCRIPTION:
 XPC-based implementation of Quesa controller.
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

#import "Q3DcontrollerXPC.h"

#import <Foundation/Foundation.h>

// Maximum number of values supported by a controller
// Matches the boundary used by ControllerCoreOSX.framework
#define Q3_CONTROLLER_MAX_VALUECOUNT 256

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

@implementation Q3DcontrollerXPC

#pragma mark - Lifecycle

- (instancetype)initWithParametersDB:(id)aDB
                      controllerUUID:(NSString *)aUUID
                     driverStateUUID:(NSString *)aDriverStateUUID
                       controllerRef:(TQ3ControllerRef)aControllerRef
                          valueCount:(TQ3Uns32)valCnt
                        channelCount:(TQ3Uns32)chanCnt
                           signature:(NSString *)sig
                 hasSetChannelMethod:(TQ3Boolean)hasSCMthd
                 hasGetChannelMethod:(TQ3Boolean)hasGCMthd
{
    if (self = [super init])
    {
        _publicDB = aDB;
        _UUID = [aUUID copy];
        _driverStateUUID = [aDriverStateUUID copy];
        _controllerRef = aControllerRef;
        
        _valueCount = valCnt;
        if (_valueCount > Q3_CONTROLLER_MAX_VALUECOUNT)
            _valueCount = Q3_CONTROLLER_MAX_VALUECOUNT;
        
        _channelCount = chanCnt;
        _signature = [sig copy];
        _hasSetChannelMethod = hasSCMthd;
        _hasGetChannelMethod = hasGCMthd;
        
        _valuesRef = new float[_valueCount]();
        _trackerUUID = nil;
        _serialNumber = 1;
        _theButtons = 0;
        _isDecommissioned = kQ3False;
        _isActive = kQ3False;
        
        // Channel methods are stored as direct function pointers (set by Q3DdbXPC via newController:)
        _channelSetMethod = NULL;
        _channelGetMethod = NULL;
        
#if Q3_DEBUG
        NSLog(@"Q3DcontrollerXPC initialized: %@", _UUID);
#endif
    }
    return self;
}

- (void)dealloc
{
    if (_valuesRef)
    {
        delete[] _valuesRef;
        _valuesRef = nullptr;
    }
    
    [_trackerConnection invalidate];
    
    // Note: [super dealloc] is automatically called by ARC
}

#pragma mark - Private Setup Methods

// setupDriverStateConnection removed — channel methods are now stored as direct function pointers

- (void)setupTrackerConnection
{
    NSString *trackerUUID = _trackerUUID;
    if (!trackerUUID)
        return;

    if (_trackerConnection)
    {
        [_trackerConnection invalidate];
        _trackerConnection = nil;
    }

    NSXPCListenerEndpoint *endpoint = Q3TrackerXPC_EndpointForUUID(trackerUUID);
    if (!endpoint)
    {
        NSLog(@"setupTrackerConnection: no endpoint for tracker UUID %@", trackerUUID);
        return;
    }

    _trackerConnection = [[NSXPCConnection alloc] initWithListenerEndpoint:endpoint];
    _trackerConnection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(Q3XPCTracker)];

    _trackerConnection.interruptionHandler = ^{
        NSLog(@"Tracker connection interrupted for %@", self.trackerUUID);
    };

    _trackerConnection.invalidationHandler = ^{
        NSLog(@"Tracker connection invalidated for %@", self.trackerUUID);
        self.trackerConnection = nil;
    };

    [_trackerConnection resume];
}

#pragma mark - Q3XPCController Protocol

- (void)decommissionControllerWithReply:(void (^)(TQ3Status))reply
{
    [self setActivation:kQ3False reply:^(TQ3Status status) {
        self.isDecommissioned = kQ3True;
        reply(status);
    }];
}

- (void)getSignatureWithReply:(void (^)(NSString * _Nullable, TQ3Status))reply
{
    if (_isDecommissioned == kQ3True)
        reply(@"", kQ3Success);  // Empty string so caller's buffer gets zeroed
    else
        reply(_signature, kQ3Success);
}

- (void)setActivation:(TQ3Boolean)active reply:(void (^)(TQ3Status))reply
{
    _isActive = active;

    // Increment list serial number
    [_publicDB incControllerListSerialNumber];
    
    // Notify tracker if present
    if (_trackerUUID && _trackerConnection)
    {
        [[_trackerConnection remoteObjectProxy] callNotificationWithController:_UUID
                                                                         reply:^(TQ3Status status) {
            // Notification sent
        }];
    }
    
    reply(kQ3Success);
}

- (void)getActivationWithReply:(void (^)(TQ3Boolean, TQ3Status))reply
{
    reply(_isActive, kQ3Success);
}

- (void)getValueCountWithReply:(void (^)(TQ3Uns32, TQ3Status))reply
{
    TQ3Uns32 count = _isDecommissioned ? 0 : _valueCount;
    reply(count, kQ3Success);
}

- (void)setButtons:(TQ3Uns32)buttons reply:(void (^)(TQ3Status))reply
{
    if (_isActive == kQ3True)
    {
        TQ3Uns32 buttonMask = _theButtons ^ buttons;
        _theButtons = buttons;

        if (_trackerUUID && _trackerConnection)
        {
            [[_trackerConnection remoteObjectProxy] changeButtonsWithController:_UUID
                                                                        buttons:buttons
                                                                     buttonMask:buttonMask
                                                                          reply:^(TQ3Status status) {
                reply(kQ3Success);
            }];
            return;
        }
    }

    reply(kQ3Success);
}

- (void)getButtonsWithReply:(void (^)(TQ3Uns32, TQ3Status))reply
{
    reply(_theButtons, kQ3Success);
}

- (void)hasTrackerWithReply:(void (^)(TQ3Boolean, TQ3Status))reply
{
    TQ3Boolean hasTracker = kQ3False;
    
    if (_trackerUUID && _trackerConnection)
    {
        [[_trackerConnection remoteObjectProxy] getActivationWithReply:^(TQ3Boolean trackerActive, TQ3Status status) {
            TQ3Boolean result = (trackerActive == kQ3True && self.isActive == kQ3True) ? kQ3True : kQ3False;
            reply(result, kQ3Success);
        }];
    }
    else
    {
        reply(kQ3False, kQ3Success);
    }
}

- (void)track2DCursorWithReply:(void (^)(TQ3Boolean, TQ3Status))reply
{
    TQ3Boolean track = (_trackerUUID == nil && _isActive == kQ3True) ? kQ3True : kQ3False;
    reply(track, kQ3Success);
}

- (void)track3DCursorWithReply:(void (^)(TQ3Boolean, TQ3Status))reply
{
    TQ3Boolean track = (_trackerUUID == nil && _isActive == kQ3True) ? kQ3True : kQ3False;
    reply(track, kQ3Success);
}

- (void)getTrackerPositionWithReply:(void (^)(TQ3Point3D, TQ3Status))reply
{
    if (_trackerUUID && _trackerConnection)
    {
        [[_trackerConnection remoteObjectProxy] getPositionWithReply:^(TQ3Uns32 serialNumber,
                                                                        TQ3Point3D position,
                                                                        TQ3Vector3D delta,
                                                                        TQ3Boolean changed,
                                                                        TQ3Status status) {
            reply(position, status);
        }];
    }
    else
    {
        TQ3Point3D defaultPos = {0.0f, 0.0f, 0.0f};
        reply(defaultPos, kQ3Success);
    }
}

- (void)setTrackerPosition:(TQ3Point3D)position reply:(void (^)(TQ3Status))reply
{
    if (_isActive == kQ3True && _trackerUUID && _trackerConnection)
    {
        [[_trackerConnection remoteObjectProxy] setPositionWithController:_UUID
                                                                 position:position
                                                                    reply:reply];
    }
    else
    {
        reply(kQ3Success);
    }
}

- (void)moveTrackerPosition:(TQ3Vector3D)delta reply:(void (^)(TQ3Status))reply
{
    if (_isActive == kQ3True && _trackerUUID && _trackerConnection)
    {
        [[_trackerConnection remoteObjectProxy] movePositionWithController:_UUID
                                                                     delta:delta
                                                                     reply:reply];
    }
    else
    {
        reply(kQ3Success);
    }
}

- (void)getTrackerOrientationWithReply:(void (^)(TQ3Quaternion, TQ3Status))reply
{
    if (_trackerUUID && _trackerConnection)
    {
        [[_trackerConnection remoteObjectProxy] getOrientationWithReply:^(TQ3Uns32 serialNumber,
                                                                           TQ3Quaternion orientation,
                                                                           TQ3Quaternion delta,
                                                                           TQ3Boolean changed,
                                                                           TQ3Status status) {
            reply(orientation, status);
        }];
    }
    else
    {
        TQ3Quaternion defaultOrient = {1.0f, 0.0f, 0.0f, 0.0f};
        reply(defaultOrient, kQ3Success);
    }
}

- (void)setTrackerOrientation:(TQ3Quaternion)orientation reply:(void (^)(TQ3Status))reply
{
    if (_isActive == kQ3True && _trackerUUID && _trackerConnection)
    {
        [[_trackerConnection remoteObjectProxy] setOrientationWithController:_UUID
                                                                  orientation:orientation
                                                                        reply:reply];
    }
    else
    {
        reply(kQ3Success);
    }
}

- (void)moveTrackerOrientation:(TQ3Quaternion)delta reply:(void (^)(TQ3Status))reply
{
    if (_isActive == kQ3True && _trackerUUID && _trackerConnection)
    {
        [[_trackerConnection remoteObjectProxy] moveOrientationWithController:_UUID
                                                                        delta:delta
                                                                        reply:reply];
    }
    else
    {
        reply(kQ3Success);
    }
}

- (void)getValuesWithReply:(void (^)(NSArray<NSNumber *> * _Nullable, TQ3Uns32, TQ3Boolean, TQ3Uns32, TQ3Status))reply
{
    NSArray<NSNumber *> *values = nil;
    
    if (_valueCount > 0)
    {
        NSMutableArray *mutableValues = [NSMutableArray arrayWithCapacity:_valueCount];
        for (TQ3Uns32 i = 0; i < _valueCount; i++)
        {
            [mutableValues addObject:@(_valuesRef[i])];
        }
        values = [mutableValues copy];
    }
    
    reply(values, _valueCount, _isActive, _serialNumber, kQ3Success);
}

- (void)setValues:(NSArray<NSNumber *> *)values ofCount:(TQ3Uns32)valueCount reply:(void (^)(TQ3Status))reply
{
    if (_isActive)
    {
        TQ3Uns32 maxCount = MIN(valueCount, _valueCount);
        
        for (TQ3Uns32 i = 0; i < maxCount; i++)
        {
            _valuesRef[i] = [values[i] floatValue];
        }
        
        _serialNumber++;
    }
    
    reply(kQ3Success);
}

- (void)setTracker:(NSString *)aTrackerUUID attachToSysCursor:(TQ3Boolean)attachToSysCrsr reply:(void (^)(TQ3Status))reply
{
    // Notify old tracker
    if (_trackerUUID && _trackerConnection)
    {
        [[_trackerConnection remoteObjectProxy] callNotificationWithController:_UUID
                                                                         reply:^(TQ3Status status) {
            // Old tracker notified
        }];
        [_trackerConnection invalidate];
        _trackerConnection = nil;
    }
    
    _trackerUUID = [aTrackerUUID copy];
    
    if (_trackerUUID)
    {
        [self setupTrackerConnection];
        
        // Notify new tracker
        if (_trackerConnection)
        {
            [[_trackerConnection remoteObjectProxy] callNotificationWithController:_UUID
                                                                             reply:^(TQ3Status status) {
                reply(kQ3Success);
            }];
        }
        else
        {
            reply(kQ3Failure);
        }
    }
    else
    {
        reply(kQ3Success);
    }
}

- (void)setChannel:(TQ3Uns32)channel withData:(NSData *)data ofSize:(TQ3Uns32)dataSize reply:(void (^)(TQ3Status))reply
{
    if (_channelSetMethod && data)
    {
        _channelSetMethod(_controllerRef, channel, data.bytes, dataSize);
    }
    reply(kQ3Success);
}

- (void)getChannel:(TQ3Uns32)channel reply:(void (^)(NSData * _Nullable, TQ3Uns32, TQ3Status))reply
{
    uint8_t buf[256] = {0};
    TQ3Uns32 size = sizeof(buf);
    if (_channelGetMethod)
    {
        _channelGetMethod(_controllerRef, channel, buf, &size);  // return value intentionally ignored
    }
    // QD3D always overrides the status to kQ3Success
    reply([NSData dataWithBytes:buf length:size], size, kQ3Success);
}

- (void)newStateWithReply:(void (^)(NSString * _Nullable, TQ3Status))reply
{
    // State save/restore is handled by CC3OSXControllerState_SaveAndReset/Restore directly
    reply(nil, kQ3Failure);
}

- (void)deleteStateWithUUID:(NSString *)stateUUID reply:(void (^)(TQ3Status))reply
{
    reply(kQ3Failure);
}

- (void)saveResetStateWithUUID:(NSString *)stateUUID reply:(void (^)(TQ3Status))reply
{
    reply(kQ3Failure);
}

- (void)restoreStateWithUUID:(NSString *)stateUUID reply:(void (^)(TQ3Status))reply
{
    reply(kQ3Failure);
}

#pragma mark - NSXPCListenerDelegate

- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    // Export this controller instance via the anonymous listener
    newConnection.exportedInterface = [NSXPCInterface interfaceWithProtocol:@protocol(Q3XPCController)];
    newConnection.exportedObject = self;
    
    newConnection.interruptionHandler = ^{
        NSLog(@"Controller %@ connection interrupted", self.UUID);
    };
    
    newConnection.invalidationHandler = ^{
        NSLog(@"Controller %@ connection invalidated", self.UUID);
    };
    
    [newConnection resume];
    
#if Q3_DEBUG
    NSLog(@"Accepted connection to controller: %@", self.UUID);
#endif
    
    return YES;
}

@end
