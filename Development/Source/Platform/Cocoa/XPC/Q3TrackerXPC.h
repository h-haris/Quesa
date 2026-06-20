/*  NAME:
 Q3TrackerXPC.h

 DESCRIPTION:
 Header for XPC-based implementation of Quesa tracker.

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
#import "IPCprotocolXPC.h"
#include "QuesaController.h"

NS_ASSUME_NONNULL_BEGIN

// ============================================================================
// Q3TrackerXPC — in-process XPC tracker object
// ============================================================================

@interface Q3TrackerXPC : NSObject <Q3XPCTracker, NSXPCListenerDelegate>

@property (nonatomic, assign) TQ3Boolean isActive;
@property (nonatomic, assign) TQ3Uns32   theButtons;
@property (nonatomic, assign) TQ3Point3D     position;
@property (nonatomic, assign) TQ3Quaternion  orientation;
@property (nonatomic, assign) TQ3Uns32   positionSerialNumber;
@property (nonatomic, assign) TQ3Uns32   orientationSerialNumber;
@property (nonatomic, assign) float      positionThreshold;
@property (nonatomic, assign) float      orientationThreshold;
@property (nonatomic, assign) TQ3TrackerNotifyFunc notifyFunc;
@property (nonatomic, assign) TQ3Object  trackerObject;
@property (nonatomic, copy)   NSString  *trackerUUID;
@property (nonatomic, readonly, nullable) NSXPCListenerEndpoint *listenerEndpoint;

- (instancetype)initWithUUID:(NSString *)uuid
                  notifyFunc:(nullable TQ3TrackerNotifyFunc)func
               trackerObject:(nullable TQ3Object)obj;

- (void)addEventTimestamp:(TQ3Uns32)ts
                  buttons:(TQ3Uns32)btns
                 position:(nullable const TQ3Point3D *)pos
              orientation:(nullable const TQ3Quaternion *)orient;

- (TQ3Status)getEventAtOrBeforeTimestamp:(TQ3Uns32)ts
                                 buttons:(nullable TQ3Uns32 *)btns
                                position:(nullable TQ3Point3D *)pos
                             orientation:(nullable TQ3Quaternion *)orient;

@end

// Registry of active tracker objects (keyed by UUID)
void Q3TrackerXPC_Register(NSString *uuid, Q3TrackerXPC *tracker);
void Q3TrackerXPC_Unregister(NSString *uuid);
NSXPCListenerEndpoint * _Nullable Q3TrackerXPC_EndpointForUUID(NSString *uuid);
Q3TrackerXPC * _Nullable Q3TrackerXPC_ForUUID(NSString *uuid);

NS_ASSUME_NONNULL_END
