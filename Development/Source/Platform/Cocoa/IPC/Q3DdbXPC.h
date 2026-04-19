/*  NAME:
 Q3DdbXPC.h

 DESCRIPTION:
 Header for XPC-based Device Database implementation.
 Can be used for both external XPC service and in-process anonymous listeners.

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

@class Q3DcontrollerXPC;

NS_ASSUME_NONNULL_BEGIN

/*!
 * @class Q3DdbXPC
 * @abstract Device database that manages controllers and trackers.
 * @discussion This class can work in two modes:
 *             1. External XPC Service: Uses Mach service name for IPC
 *             2. In-Process: Uses anonymous NSXPCListener for same-process communication
 *
 * For in-process usage (MachXPC-style), create an instance and use anonymousListener:
 *     Q3DdbXPC *db = [[Q3DdbXPC alloc] init];
 *     NSXPCListener *listener = [NSXPCListener anonymousListener];
 *     listener.delegate = db;
 *     [listener resume];
 */
@interface Q3DdbXPC : NSObject <NSXPCListenerDelegate, Q3XPCDeviceDB>

/// XPC listener (can be anonymous for in-process or service for external)
@property (nonatomic, strong) NSXPCListener *listener;

/// Array of controller objects managed by this database
@property (nonatomic, strong) NSMutableArray<Q3DcontrollerXPC *> *controllerObjects;

/// Serial number incremented when controller list changes
@property (nonatomic, assign) TQ3Uns32 controllerListSerialNumber;

/*!
 * @method initForInProcess
 * @abstract Initialize for in-process use with anonymous listener
 * @discussion Use this for MachXPC-style in-process communication
 * @return Initialized instance ready for anonymous listener
 */
- (instancetype)initForInProcess;

/*!
 * @method run
 * @abstract Start the XPC listener
 * @discussion Call this to begin accepting XPC connections
 */
- (void)run;

/*!
 * @method incControllerListSerialNumber
 * @abstract Increment the controller list serial number
 * @discussion Called when controllers are added/removed/changed
 */
- (void)incControllerListSerialNumber;

@end

NS_ASSUME_NONNULL_END
