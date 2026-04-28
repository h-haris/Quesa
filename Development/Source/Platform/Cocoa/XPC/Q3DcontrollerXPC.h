/*  NAME:
 Q3DcontrollerXPC.h

 DESCRIPTION:
 Header for XPC-based implementation of Quesa controller.

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
#import "Q3Ddb.h"
#import "IPCprotocolXPC.h"
#import "Q3TrackerXPC.h"
#include "QuesaController.h"

NS_ASSUME_NONNULL_BEGIN

@interface Q3DcontrollerXPC : NSObject <Q3XPCController, NSXPCListenerDelegate>

// Core properties
@property (nonatomic, weak, nullable) id publicDB;
@property (nonatomic, copy) NSString *UUID;
@property (nonatomic, copy, nullable) NSString *driverStateUUID;
@property (nonatomic, copy) NSString *signature;
@property (nonatomic, assign) TQ3ControllerRef controllerRef;

// XPC Connection to driver state (lazy-connected from driverStateEndpoint)
@property (nonatomic, strong, nullable) NSXPCConnection       *driverStateConnection;
// Endpoint supplied by the driver process so the DB can call back for channel ops
@property (nonatomic, strong, nullable) NSXPCListenerEndpoint *driverStateEndpoint;

// Tracker
@property (nonatomic, copy, nullable) NSString *trackerUUID;
@property (nonatomic, strong, nullable) NSXPCListenerEndpoint *trackerEndpoint;
@property (nonatomic, strong, nullable) NSXPCConnection *trackerConnection;

// Controller state
@property (nonatomic, assign) TQ3Uns32 valueCount;
@property (nonatomic, assign) TQ3Uns32 channelCount;
@property (nonatomic, assign) TQ3Boolean hasSetChannelMethod;
@property (nonatomic, assign) TQ3Boolean hasGetChannelMethod;
@property (nonatomic, assign) TQ3Boolean isActive;
@property (nonatomic, assign) TQ3Boolean isDecommissioned;
@property (nonatomic, assign) TQ3Uns32 serialNumber;
@property (nonatomic, assign) TQ3Uns32 theButtons;

// Channel methods (C function pointers stored directly)
@property (nonatomic, assign, nullable) TQ3ChannelSetMethod channelSetMethod;
@property (nonatomic, assign, nullable) TQ3ChannelGetMethod channelGetMethod;

// Values storage
@property (nonatomic, assign, nullable) float *valuesRef;

// Initializer
- (instancetype)initWithParametersDB:(id)aDB
                      controllerUUID:(NSString *)aUUID
                     driverStateUUID:(NSString *)aDriverStateUUID
                       controllerRef:(TQ3ControllerRef)aControllerRef
                          valueCount:(TQ3Uns32)valCnt
                        channelCount:(TQ3Uns32)chanCnt
                           signature:(NSString *)sig
                 hasSetChannelMethod:(TQ3Boolean)hasSCMthd
                 hasGetChannelMethod:(TQ3Boolean)hasGCMthd;

@end

NS_ASSUME_NONNULL_END
