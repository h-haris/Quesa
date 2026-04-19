/*  NAME:
 IPCprotocolXPC.h
 
 DESCRIPTION:
 Header file with protocols used for IPC via XPC.
 
 Implementation of Quesa controller API calls.
 
 Under macOS the communication between driver, device server and client
 is implemented as IPC via XPC. This header defines the used protocols.
 
 This replaces the deprecated PDO (Portable Distributed Objects) implementation.
 
    COPYRIGHT:
        Copyright (c) 2013-2026, Quesa Developers. All rights reserved.

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

// XPC Service Names
#define kQuesa3DDeviceServerXPC         "com.quesa.osx.3ddevice.server.xpc"
#define kQuesa3DDeviceControllerXPC     "com.quesa.osx.3ddevice.controller.xpc"
#define kQuesa3DDeviceTrackerXPC        "com.quesa.osx.3ddevice.tracker.xpc"

// Note: In XPC, we use NSString* for controller references instead of void*
// This allows proper serialization across process boundaries

//=============================================================================
// Controller XPC Protocol
//=============================================================================
@protocol Q3XPCController <NSObject>

// Controller Management
- (void)decommissionControllerWithReply:(void (^)(TQ3Status status))reply;
- (void)getSignatureWithReply:(void (^)(NSString * _Nullable signature, TQ3Status status))reply;

// Activation
- (void)setActivation:(TQ3Boolean)active reply:(void (^)(TQ3Status status))reply;
- (void)getActivationWithReply:(void (^)(TQ3Boolean active, TQ3Status status))reply;

// Values
- (void)getValueCountWithReply:(void (^)(TQ3Uns32 valueCount, TQ3Status status))reply;
- (void)setValues:(NSArray<NSNumber *> *)values 
          ofCount:(TQ3Uns32)valueCount
            reply:(void (^)(TQ3Status status))reply;
- (void)getValuesWithReply:(void (^)(NSArray<NSNumber *> * _Nullable values,
                                     TQ3Uns32 valueCount,
                                     TQ3Boolean isActive,
                                     TQ3Uns32 serialNumber,
                                     TQ3Status status))reply;

// Buttons
- (void)setButtons:(TQ3Uns32)buttons reply:(void (^)(TQ3Status status))reply;
- (void)getButtonsWithReply:(void (^)(TQ3Uns32 buttons, TQ3Status status))reply;

// Tracker
- (void)hasTrackerWithReply:(void (^)(TQ3Boolean hasTracker, TQ3Status status))reply;
- (void)track2DCursorWithReply:(void (^)(TQ3Boolean trackSys2DCursor, TQ3Status status))reply;
- (void)track3DCursorWithReply:(void (^)(TQ3Boolean trackSys3DCursor, TQ3Status status))reply;
- (void)setTracker:(NSString * _Nullable)trackerUUID 
 attachToSysCursor:(TQ3Boolean)attachToSysCrsr
             reply:(void (^)(TQ3Status status))reply;

// Tracker Position
- (void)getTrackerPositionWithReply:(void (^)(TQ3Point3D position, TQ3Status status))reply;
- (void)setTrackerPosition:(TQ3Point3D)position reply:(void (^)(TQ3Status status))reply;
- (void)moveTrackerPosition:(TQ3Vector3D)delta reply:(void (^)(TQ3Status status))reply;

// Tracker Orientation
- (void)getTrackerOrientationWithReply:(void (^)(TQ3Quaternion orientation, TQ3Status status))reply;
- (void)setTrackerOrientation:(TQ3Quaternion)orientation reply:(void (^)(TQ3Status status))reply;
- (void)moveTrackerOrientation:(TQ3Quaternion)delta reply:(void (^)(TQ3Status status))reply;

// Channels
- (void)setChannel:(TQ3Uns32)channel
          withData:(NSData * _Nullable)data
            ofSize:(TQ3Uns32)dataSize
             reply:(void (^)(TQ3Status status))reply;
- (void)getChannel:(TQ3Uns32)channel
             reply:(void (^)(NSData * _Nullable data, TQ3Uns32 dataSize, TQ3Status status))reply;

// Controller State
- (void)newStateWithReply:(void (^)(NSString * _Nullable stateUUID, TQ3Status status))reply;
- (void)deleteStateWithUUID:(NSString *)stateUUID reply:(void (^)(TQ3Status status))reply;
- (void)saveResetStateWithUUID:(NSString *)stateUUID reply:(void (^)(TQ3Status status))reply;
- (void)restoreStateWithUUID:(NSString *)stateUUID reply:(void (^)(TQ3Status status))reply;

@end


//=============================================================================
// Device Database XPC Protocol
//=============================================================================
@protocol Q3XPCDeviceDB <NSObject>

// Controller Registration
- (void)reNewCC3ControllerWithUUID:(NSString *)aDriverUUID
                    ctrlDriverUUID:(NSString *)aControllerDriverUUID
                     controllerRef:(NSString *)aControllerRefString
                        valueCount:(TQ3Uns32)valCnt
                      channelCount:(TQ3Uns32)chanCnt
                         signature:(NSString *)sig
               hasSetChannelMethod:(TQ3Boolean)hasSCMthd
               hasGetChannelMethod:(TQ3Boolean)hasGCMthd
                             reply:(void (^)(NSString * _Nullable uuid, TQ3Status status))reply;

// Controller Enumeration
- (void)nextCC3Controller:(NSString * _Nullable)currentControllerUUID
                    reply:(void (^)(NSString * _Nullable nextControllerUUID))reply;

// List Management
- (void)getListChangedWithReply:(void (^)(TQ3Boolean listChanged, TQ3Uns32 serialNumber, TQ3Status status))reply;

// Tracker Management
- (void)trackerDeleted:(NSString *)deletedTrackerUUID reply:(void (^)(TQ3Status status))reply;

// Signature Validation
- (void)isKnownSignature:(NSString *)aDriverSignature reply:(void (^)(TQ3Boolean isKnown))reply;

// Connection to specific controller
- (void)connectionForController:(NSString *)controllerUUID
                          reply:(void (^)(NSXPCListenerEndpoint * _Nullable endpoint))reply;

@end


//=============================================================================
// Controller Driver State XPC Protocol
//=============================================================================
@protocol Q3XPCControllerDriverState <NSObject>

- (void)setChannel:(TQ3Uns32)channel
          withData:(NSData * _Nullable)data
            ofSize:(TQ3Uns32)dataSize
             reply:(void (^)(TQ3Status status))reply;

- (void)getChannel:(TQ3Uns32)channel
             reply:(void (^)(NSData * _Nullable data, TQ3Uns32 dataSize, TQ3Status status))reply;

- (void)newDrvStateWithUUID:(NSString *)stateUUID reply:(void (^)(TQ3Status status))reply;
- (void)deleteDrvStateWithUUID:(NSString *)stateUUID reply:(void (^)(TQ3Status status))reply;
- (void)saveDrvResetStateWithUUID:(NSString *)stateUUID reply:(void (^)(TQ3Status status))reply;
- (void)restoreDrvStateWithUUID:(NSString *)stateUUID reply:(void (^)(TQ3Status status))reply;

@end


//=============================================================================
// Tracker XPC Protocol
//=============================================================================
@protocol Q3XPCTracker <NSObject>

// Notification
- (void)callNotificationWithController:(NSString *)controllerUUID 
                                 reply:(void (^)(TQ3Status status))reply;

// Activation
- (void)setActivation:(TQ3Boolean)active reply:(void (^)(TQ3Status status))reply;
- (void)getActivationWithReply:(void (^)(TQ3Boolean active, TQ3Status status))reply;

// Buttons
- (void)changeButtonsWithController:(NSString *)controllerUUID
                            buttons:(TQ3Uns32)theButtons
                         buttonMask:(TQ3Uns32)aButtonMask
                              reply:(void (^)(TQ3Status status))reply;

// Position
- (void)getPositionWithReply:(void (^)(TQ3Uns32 serialNumber,
                                       TQ3Point3D position,
                                       TQ3Vector3D delta,
                                       TQ3Boolean changed,
                                       TQ3Status status))reply;
- (void)setPositionWithController:(NSString *)controllerUUID
                         position:(TQ3Point3D)aPosition
                            reply:(void (^)(TQ3Status status))reply;
- (void)movePositionWithController:(NSString *)controllerUUID
                             delta:(TQ3Vector3D)aDelta
                             reply:(void (^)(TQ3Status status))reply;

// Orientation
- (void)getOrientationWithReply:(void (^)(TQ3Uns32 serialNumber,
                                          TQ3Quaternion orientation,
                                          TQ3Quaternion delta,
                                          TQ3Boolean changed,
                                          TQ3Status status))reply;
- (void)setOrientationWithController:(NSString *)controllerUUID
                         orientation:(TQ3Quaternion)anOrientation
                               reply:(void (^)(TQ3Status status))reply;
- (void)moveOrientationWithController:(NSString *)controllerUUID
                                delta:(TQ3Quaternion)aDelta
                                reply:(void (^)(TQ3Status status))reply;

@end

#endif /* IPCprotocolXPC_h */
