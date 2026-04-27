/*  NAME:
 Q3DdbXPC.mm

 DESCRIPTION:
 XPC-based Device Database server implementation.
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

#import "Q3DdbXPC.h"
#import "Q3DcontrollerXPC.h"

#if Q3_DEBUG
#import <Foundation/Foundation.h>
#endif

@implementation Q3DdbXPC

#pragma mark - Lifecycle

- (instancetype)initForInProcess
{
    if (self = [super init])
    {
        _controllerListSerialNumber = 0;
        _controllerObjects = [NSMutableArray arrayWithCapacity:2];

#if Q3_DEBUG
        NSLog(@"Q3DdbXPC initialized for in-process use");
#endif
    }
    return self;
}

#pragma mark - Public Methods

- (void)incControllerListSerialNumber
{
    _controllerListSerialNumber++;
}

#pragma mark - NSXPCListenerDelegate

- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    newConnection.exportedInterface = [NSXPCInterface interfaceWithProtocol:@protocol(Q3XPCDeviceDB)];
    newConnection.exportedObject = self;
    
    newConnection.interruptionHandler = ^{
        NSLog(@"XPC connection interrupted");
    };
    
    newConnection.invalidationHandler = ^{
        NSLog(@"XPC connection invalidated");
    };
    
    [newConnection resume];
    
#if Q3_DEBUG
    NSLog(@"Accepted new XPC connection");
#endif
    
    return YES;
}

#pragma mark - Helper Methods

- (NSUInteger)dbIndexOfTrackerUUID:(NSString *)aTrackerUUID
{
    return [_controllerObjects indexOfObjectPassingTest:^BOOL(Q3DcontrollerXPC *obj, NSUInteger idx, BOOL *stop) {
        NSString *trackerUUID = obj.trackerUUID;
        return trackerUUID != nil && [aTrackerUUID isEqualToString:trackerUUID];
    }];
}

- (NSUInteger)dbIndexOfSignature:(NSString *)aDriverSignature
{
    return [_controllerObjects indexOfObjectPassingTest:^BOOL(Q3DcontrollerXPC *obj, NSUInteger idx, BOOL *stop) {
        return [aDriverSignature isEqualToString:obj.signature];
    }];
}

- (NSUInteger)dbIndexOfControllerUUID:(NSString *)aControllerUUID
{
    return [_controllerObjects indexOfObjectPassingTest:^BOOL(Q3DcontrollerXPC *obj, NSUInteger idx, BOOL *stop) {
        return [aControllerUUID isEqualToString:obj.UUID];
    }];
}

#pragma mark - Q3XPCDeviceDB Protocol

- (void)reNewCC3ControllerWithUUID:(NSString *)aControllerUUID
                    ctrlDriverUUID:(NSString *)aControllerDriverUUID
                     controllerRef:(NSString *)aControllerRefString
                        valueCount:(TQ3Uns32)valCnt
                      channelCount:(TQ3Uns32)chanCnt
                         signature:(NSString *)sig
               hasSetChannelMethod:(TQ3Boolean)hasSCMthd
               hasGetChannelMethod:(TQ3Boolean)hasGCMthd
                             reply:(void (^)(NSString * _Nullable, TQ3Status))reply
{
    Q3DcontrollerXPC *theControllerObject = nil;
    
    NSUInteger foundSignatureAt = [self dbIndexOfSignature:sig];
    
    if (foundSignatureAt != NSNotFound)
    {
        theControllerObject = _controllerObjects[foundSignatureAt];
    }
    else
    {
        theControllerObject = [[Q3DcontrollerXPC alloc] initWithParametersDB:self
                                                              controllerUUID:aControllerUUID
                                                             driverStateUUID:aControllerDriverUUID
                                                               controllerRef:(__bridge TQ3ControllerRef)aControllerRefString
                                                                  valueCount:valCnt
                                                                channelCount:chanCnt
                                                                   signature:sig
                                                         hasSetChannelMethod:hasSCMthd
                                                         hasGetChannelMethod:hasGCMthd];
        
        [_controllerObjects addObject:theControllerObject];
    }
    
    // Recommission and activate
    theControllerObject.isDecommissioned = kQ3False;
    theControllerObject.serialNumber = 1;
    theControllerObject.theButtons = 0;
    
    [theControllerObject setActivation:kQ3True reply:^(TQ3Status status) {
        reply(theControllerObject.UUID, status);
    }];
}

- (void)nextCC3Controller:(NSString *)currentControllerUUID reply:(void (^)(NSString * _Nullable))reply
{
    NSUInteger idx;

    if (currentControllerUUID == nil)
    {
        idx = 0;
    }
    else
    {
        idx = [self dbIndexOfControllerUUID:currentControllerUUID];
        if (idx == NSNotFound)
        {
            reply(nil);
            return;
        }
        idx++;
    }

    // Skip decommissioned controllers
    while (idx < _controllerObjects.count && _controllerObjects[idx].isDecommissioned == kQ3True)
        idx++;

    if (idx >= _controllerObjects.count)
        reply(nil);
    else
        reply(_controllerObjects[idx].UUID);
}

- (void)getListChangedWithReply:(void (^)(TQ3Boolean, TQ3Uns32, TQ3Status))reply
{
    reply(kQ3True, _controllerListSerialNumber, kQ3Success);
}

- (void)trackerDeleted:(NSString *)deletedTrackerUUID reply:(void (^)(TQ3Status))reply
{
    NSUInteger foundOldTrackerAt = [self dbIndexOfTrackerUUID:deletedTrackerUUID];
    
    if (foundOldTrackerAt != NSNotFound)
    {
        Q3DcontrollerXPC *theControllerObject = _controllerObjects[foundOldTrackerAt];
        [theControllerObject setTracker:nil attachToSysCursor:kQ3True reply:^(TQ3Status status) {
            reply(status);
        }];
    }
    else
    {
        reply(kQ3Failure);
    }
}

- (void)isKnownSignature:(NSString *)aDriverSignature reply:(void (^)(TQ3Boolean))reply
{
    NSUInteger index = [self dbIndexOfSignature:aDriverSignature];
    reply(index != NSNotFound ? kQ3True : kQ3False);
}

- (void)connectionForController:(NSString *)controllerUUID reply:(void (^)(NSXPCListenerEndpoint * _Nullable))reply
{
    NSUInteger idx = [self dbIndexOfControllerUUID:controllerUUID];
    
    if (idx != NSNotFound)
    {
        // Create anonymous listener for this controller
        NSXPCListener *anonymousListener = [NSXPCListener anonymousListener];
        Q3DcontrollerXPC *controller = _controllerObjects[idx];
        
        anonymousListener.delegate = self;
        // In a full implementation, you'd set up a delegate that exports the controller object
        [anonymousListener resume];
        
        reply(anonymousListener.endpoint);
    }
    else
    {
        reply(nil);
    }
}

- (void)newController:(NSDictionary *)controllerData reply:(void (^)(NSString * _Nullable))reply
{
    NSString *signature = controllerData[@"signature"] ?: @"";
    TQ3Uns32 valueCount = [controllerData[@"valueCount"] unsignedIntValue];
    TQ3Uns32 channelCount = [controllerData[@"channelCount"] unsignedIntValue];

    // Extract channel method function pointers (passed as uint64)
    TQ3ChannelSetMethod setMethod = NULL;
    TQ3ChannelGetMethod getMethod = NULL;
    NSNumber *setMethodNum = controllerData[@"channelSetMethod"];
    NSNumber *getMethodNum = controllerData[@"channelGetMethod"];
    if (setMethodNum) setMethod = (TQ3ChannelSetMethod)(uintptr_t)[setMethodNum unsignedLongLongValue];
    if (getMethodNum) getMethod = (TQ3ChannelGetMethod)(uintptr_t)[getMethodNum unsignedLongLongValue];

    Q3DcontrollerXPC *theControllerObject = nil;

    // Check if a controller with the same signature already exists (matches PDO behavior)
    NSUInteger foundSignatureAt = [self dbIndexOfSignature:signature];

    if (foundSignatureAt != NSNotFound)
    {
        // Reuse existing controller with same signature
        theControllerObject = _controllerObjects[foundSignatureAt];
    }
    else
    {
        // Create new controller
        NSString *controllerUUID = [[NSUUID UUID] UUIDString];

        theControllerObject = [[Q3DcontrollerXPC alloc] initWithParametersDB:self
                                                               controllerUUID:controllerUUID
                                                              driverStateUUID:nil
                                                                controllerRef:nil
                                                                   valueCount:valueCount
                                                                 channelCount:channelCount
                                                                    signature:signature
                                                          hasSetChannelMethod:(setMethod != NULL ? kQ3True : kQ3False)
                                                          hasGetChannelMethod:(getMethod != NULL ? kQ3True : kQ3False)];

        [_controllerObjects addObject:theControllerObject];
    }

    // Update channel methods (important for re-creation case)
    theControllerObject.channelSetMethod = setMethod;
    theControllerObject.channelGetMethod = getMethod;

    // Recommission and activate (matches PDO behavior)
    theControllerObject.isDecommissioned = kQ3False;
    theControllerObject.serialNumber = 1;
    theControllerObject.theButtons = 0;
    theControllerObject.isActive = kQ3True;

    [self incControllerListSerialNumber];
    reply(theControllerObject.UUID);
}

- (void)decommissionController:(NSString *)controllerUUID reply:(void (^)(void))reply
{
    NSUInteger idx = [self dbIndexOfControllerUUID:controllerUUID];

    if (idx != NSNotFound)
    {
        Q3DcontrollerXPC *controller = _controllerObjects[idx];
        controller.isDecommissioned = kQ3True;
        controller.isActive = kQ3False;
        // Keep controller in array so post-decommission queries (GetActivation, GetSignature, etc.) still work.
        // nextCC3Controller: skips decommissioned controllers.
        [self incControllerListSerialNumber];
    }

    reply();
}

- (void)newTrackerWithReply:(void (^)(NSString * _Nullable))reply
{
    // Create a new tracker UUID
    NSString *trackerUUID = [[NSUUID UUID] UUIDString];
    reply(trackerUUID);
}

- (void)deleteTracker:(NSString *)trackerUUID reply:(void (^)(void))reply
{
    // Find and remove tracker from controller
    NSUInteger foundTrackerAt = [self dbIndexOfTrackerUUID:trackerUUID];
    
    if (foundTrackerAt != NSNotFound)
    {
        Q3DcontrollerXPC *theControllerObject = _controllerObjects[foundTrackerAt];
        [theControllerObject setTracker:nil attachToSysCursor:kQ3True reply:^(TQ3Status status) {
            // Tracker removed
        }];
    }
    
    reply();
}

- (void)getButtonsForTracker:(NSString *)trackerUUID reply:(void (^)(TQ3Uns32, TQ3Status))reply
{
    NSUInteger foundTrackerAt = [self dbIndexOfTrackerUUID:trackerUUID];
    
    if (foundTrackerAt != NSNotFound)
    {
        Q3DcontrollerXPC *controller = _controllerObjects[foundTrackerAt];
        [controller getButtonsWithReply:^(TQ3Uns32 buttons, TQ3Status status) {
            reply(buttons, status);
        }];
    }
    else
    {
        reply(0, kQ3Failure);
    }
}

- (void)setEventCoordinatesForTracker:(NSString *)trackerUUID
                            timestamp:(TQ3Uns32)timeStamp
                              buttons:(TQ3Uns32)buttons
                             position:(TQ3Point3D)position
                          orientation:(TQ3Quaternion)orientation
                                reply:(void (^)(TQ3Status))reply
{
    Q3TrackerXPC *tracker = Q3TrackerXPC_ForUUID(trackerUUID);
    if (tracker)
    {
        [tracker addEventTimestamp:timeStamp
                           buttons:buttons
                          position:&position
                       orientation:&orientation];
        reply(kQ3Success);
    }
    else
    {
        reply(kQ3Failure);
    }
}

- (void)getEventCoordinatesForTracker:(NSString *)trackerUUID
                            timestamp:(TQ3Uns32)timeStamp
                                reply:(void (^)(TQ3Uns32, TQ3Point3D, TQ3Quaternion, TQ3Status))reply
{
    Q3TrackerXPC *tracker = Q3TrackerXPC_ForUUID(trackerUUID);
    if (tracker)
    {
        TQ3Uns32 buttons = 0;
        TQ3Point3D pos = {0.0f, 0.0f, 0.0f};
        TQ3Quaternion orient = {1.0f, 0.0f, 0.0f, 0.0f};
        TQ3Status status = [tracker getEventAtOrBeforeTimestamp:timeStamp
                                                        buttons:&buttons
                                                       position:&pos
                                                    orientation:&orient];
        reply(buttons, pos, orient, status);
    }
    else
    {
        TQ3Point3D zeroPos = {0.0f, 0.0f, 0.0f};
        TQ3Quaternion identityQuat = {1.0f, 0.0f, 0.0f, 0.0f};
        reply(0, zeroPos, identityQuat, kQ3Failure);
    }
}

@end

#endif // QUESA_USE_XPC
