/*  NAME:
 ControllerCoreOSX_XPC.mm

 DESCRIPTION:
 XPC-based client connection implementation for Quesa controllers.
 Replaces the deprecated PDO implementation.

    COPYRIGHT:
        Copyright (c) 1999-2026, Quesa Developers. All rights reserved.

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

#include "E3Prefix.h"
#import "IPCprotocolXPC.h"
#import "ControllerCoreOSX.h"
#import "ControllerCoreOSXinternals.h"
#import "Q3DdbXPC.h"           // In-process device database
#import "Q3DcontrollerXPC.h"   // In-process controller
// Note: No external XPC service needed - using anonymous listeners!

#import <Foundation/Foundation.h>

//=============================================================================
//      Internal variables
//-----------------------------------------------------------------------------

// In-process device database (no external XPC service needed!)
static Q3DdbXPC *sharedDeviceDB = nil;
static NSXPCListener *deviceDBListener = nil;

// Controller endpoints (using anonymous listeners)
static NSMutableDictionary<NSString *, NSXPCConnection *> *controllerConnections = nil;
static NSMutableDictionary<NSString *, NSXPCListener *> *controllerListeners = nil;

//=============================================================================
//      Internal function prototypes
//-----------------------------------------------------------------------------

static void CC3OSX_CleanupXPCConnections(void);

#pragma mark - In-Process XPC Setup (MachXPC-style)

// Initialize in-process device database with anonymous XPC listener
static void initializeInProcessDeviceDB(void)
{
    if (sharedDeviceDB != nil)
        return;

    // Create the device database instance (runs in-process!)
    sharedDeviceDB = [[Q3DdbXPC alloc] initForInProcess];

    // Create anonymous listener (no external service needed)
    deviceDBListener = [NSXPCListener anonymousListener];
    deviceDBListener.delegate = sharedDeviceDB;
    [deviceDBListener resume];

    // Initialize storage
    if (controllerConnections == nil)
        controllerConnections = [NSMutableDictionary dictionary];
    if (controllerListeners == nil)
        controllerListeners = [NSMutableDictionary dictionary];

    atexit(CC3OSX_CleanupXPCConnections);

#if Q3_DEBUG
    NSLog(@"✅ Initialized in-process device DB (MachXPC-style - no external XPC service!)");
#endif
}

// Get a connection to the in-process device database
static TQ3Status connectionToDeviceDB(NSXPCConnection **outConnection)
{
    static NSXPCConnection *inProcessConnection = nil;

    if (inProcessConnection == nil)
    {
        initializeInProcessDeviceDB();

        // Create connection using anonymous endpoint (in-process!)
        inProcessConnection = [[NSXPCConnection alloc]
            initWithListenerEndpoint:deviceDBListener.endpoint];

        inProcessConnection.remoteObjectInterface =
            [NSXPCInterface interfaceWithProtocol:@protocol(Q3XPCDeviceDB)];

        inProcessConnection.interruptionHandler = ^{
            NSLog(@"In-process device DB connection interrupted");
        };

        inProcessConnection.invalidationHandler = ^{
            NSLog(@"In-process device DB connection invalidated");
            inProcessConnection = nil;
        };

        [inProcessConnection resume];

#if Q3_DEBUG
        NSLog(@"Connected to in-process device DB via anonymous endpoint");
#endif
    }

    *outConnection = inProcessConnection;
    return kQ3Success;
}

// Get connection to a specific controller (using in-process anonymous endpoint)
static NSXPCConnection *connectionForController(NSString *controllerUUID)
{
    if (controllerConnections == nil)
    {
        controllerConnections = [NSMutableDictionary dictionary];
    }

    NSXPCConnection *connection = controllerConnections[controllerUUID];

    if (connection == nil)
    {
        // Look up the in-process controller object from the shared device database
        Q3DcontrollerXPC *controller = nil;
        for (Q3DcontrollerXPC *c in sharedDeviceDB.controllerObjects)
        {
            if ([c.UUID isEqualToString:controllerUUID])
            {
                controller = c;
                break;
            }
        }

        if (controller)
        {
            // Create anonymous listener for this controller and store it persistently.
            // It must outlive the connection — a local ARC variable would be released
            // when this function returns, invalidating the connection asynchronously.
            NSXPCListener *controllerListener = [NSXPCListener anonymousListener];
            controllerListener.delegate = (id<NSXPCListenerDelegate>)controller;
            [controllerListener resume];
            controllerListeners[controllerUUID] = controllerListener;

            // Create connection using the anonymous endpoint (all in-process!)
            connection = [[NSXPCConnection alloc]
                initWithListenerEndpoint:controllerListener.endpoint];

            connection.remoteObjectInterface =
                [NSXPCInterface interfaceWithProtocol:@protocol(Q3XPCController)];

            connection.interruptionHandler = ^{
                NSLog(@"Controller connection interrupted: %@", controllerUUID);
            };

            connection.invalidationHandler = ^{
                [controllerConnections removeObjectForKey:controllerUUID];
                [controllerListeners removeObjectForKey:controllerUUID];
            };

            [connection resume];
            controllerConnections[controllerUUID] = connection;

#if Q3_DEBUG
            NSLog(@"Created in-process connection to controller: %@", controllerUUID);
#endif
        }
    }

    return connection;
}

#pragma mark - Internal Tracker Data Structure for XPC

//=============================================================================
//      Internal Tracker Data Structure for XPC
//-----------------------------------------------------------------------------
typedef struct TC3TrackerInstanceDataXPC
{
    NSString *trackerUUID;
    void *trackerXPCObject; // Q3TrackerXPC* (unretained; kept alive by sTrackerObjects registry)
} TC3TrackerInstanceDataXPC;

// Find the controller XPC connection that currently owns a given tracker UUID.
static NSXPCConnection *connectionForTrackerUUID(NSString *trackerUUID)
{
    if (sharedDeviceDB == nil) return nil;
    for (Q3DcontrollerXPC *c in sharedDeviceDB.controllerObjects)
    {
        if ([c.trackerUUID isEqualToString:trackerUUID])
            return connectionForController(c.UUID);
    }
    return nil;
}

#pragma mark - Public Controller Functions (Synchronous Wrappers)

//=============================================================================
//      CC3OSXController_GetListChanged
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_GetListChanged(TQ3Boolean *listChanged, TQ3Uns32 *serialNumber)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSXPCConnection *connection = nil;
    if (connectionToDeviceDB(&connection) == kQ3Success)
    {
        TQ3Uns32 lastKnownSerialNumber = *serialNumber;
        [[connection remoteObjectProxy] getListChangedWithReply:^(TQ3Boolean changed,
                                                                   TQ3Uns32 serNum,
                                                                   TQ3Status stat) {
            // Mirror PDO behavior: changed if serial number differs from caller's last known
            *listChanged = (serNum != lastKnownSerialNumber) ? kQ3True : kQ3False;
            *serialNumber = serNum;
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_Next
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_Next(TQ3ControllerRef controllerRef, TQ3ControllerRef *nextControllerRef)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSXPCConnection *connection = nil;
    if (connectionToDeviceDB(&connection) == kQ3Success)
    {
        NSString *currentUUID = (__bridge NSString *)controllerRef;

        [[connection remoteObjectProxy] nextCC3Controller:currentUUID
                                                    reply:^(NSString *nextUUID) {
            // __bridge_retained transfers ownership to the void* caller (caller retains for lifetime of the ref)
            *nextControllerRef = nextUUID ? (__bridge_retained TQ3ControllerRef)[nextUUID copy] : nullptr;
            status = kQ3Success;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_SetActivation
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_SetActivation(TQ3ControllerRef controllerRef, TQ3Boolean active)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] setActivation:active reply:^(TQ3Status stat) {
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_GetActivation
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_GetActivation(TQ3ControllerRef controllerRef, TQ3Boolean *active)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] getActivationWithReply:^(TQ3Boolean act, TQ3Status stat) {
            *active = act;
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_GetSignature
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_GetSignature(TQ3ControllerRef controllerRef, char *signature, TQ3Uns32 numChars)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] getSignatureWithReply:^(NSString *sig, TQ3Status stat) {
            if (sig != nil && signature != nullptr && numChars > 0)
            {
                strlcpy(signature, [sig UTF8String], numChars);
            }
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_GetButtons
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_GetButtons(TQ3ControllerRef controllerRef, TQ3Uns32 *buttons)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] getButtonsWithReply:^(TQ3Uns32 btns, TQ3Status stat) {
            *buttons = btns;
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_SetButtons
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_SetButtons(TQ3ControllerRef controllerRef, TQ3Uns32 buttons)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] setButtons:buttons reply:^(TQ3Status stat) {
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_GetValues
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_GetValues(TQ3ControllerRef controllerRef, TQ3Uns32 valueCount, float *values,
                           TQ3Boolean *changed, TQ3Uns32 *serialNumber)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] getValuesWithReply:^(NSArray<NSNumber *> *vals,
                                                              TQ3Uns32 count,
                                                              TQ3Boolean active,
                                                              TQ3Uns32 serNum,
                                                              TQ3Status stat) {
            if (vals && active)
            {
                TQ3Uns32 maxCount = MIN(count, valueCount);
                for (TQ3Uns32 i = 0; i < maxCount; i++)
                {
                    values[i] = [vals[i] floatValue];
                }

                if (serialNumber)
                {
                    if (changed) *changed = (*serialNumber != serNum) ? kQ3True : kQ3False;
                    *serialNumber = serNum;
                }
                else
                {
                    if (changed) *changed = kQ3True;
                }
            }
            else
            {
                if (changed) *changed = kQ3False;
            }

            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_SetValues
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_SetValues(TQ3ControllerRef controllerRef, const float *values, TQ3Uns32 valueCount)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        NSMutableArray<NSNumber *> *valuesArray = [NSMutableArray arrayWithCapacity:valueCount];
        for (TQ3Uns32 i = 0; i < valueCount; i++)
        {
            [valuesArray addObject:@(values[i])];
        }

        [[connection remoteObjectProxy] setValues:valuesArray
                                          ofCount:valueCount
                                            reply:^(TQ3Status stat) {
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_New
//-----------------------------------------------------------------------------
TQ3ControllerRef
CC3OSXController_New(const TQ3ControllerData *controllerData)
{
    __block TQ3ControllerRef controllerRef = nullptr;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSXPCConnection *connection = nil;
    if (connectionToDeviceDB(&connection) == kQ3Success)
    {
        // Create data dictionary for controller
        NSString *signature = controllerData->signature ?
            [NSString stringWithUTF8String:controllerData->signature] : @"";

        NSDictionary *dataDict = @{
            @"signature": signature,
            @"valueCount": @(controllerData->valueCount),
            @"channelCount": @(controllerData->channelCount),
            @"channelSetMethod": @((uint64_t)(uintptr_t)controllerData->channelSetMethod),
            @"channelGetMethod": @((uint64_t)(uintptr_t)controllerData->channelGetMethod),
        };

        [[connection remoteObjectProxy] newController:dataDict
                                                reply:^(NSString *uuid) {
            controllerRef = (__bridge_retained TQ3ControllerRef)[uuid copy];
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return controllerRef;
}

//=============================================================================
//      CC3OSXController_Decommission
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_Decommission(TQ3ControllerRef controllerRef)
{
    NSString *controllerUUID = (__bridge NSString *)controllerRef;

    // Remove from connections cache
    if (controllerConnections)
    {
        NSXPCConnection *conn = controllerConnections[controllerUUID];
        if (conn)
        {
            [conn invalidate];
            [controllerConnections removeObjectForKey:controllerUUID];
        }
    }

    // Notify device DB and wait for completion
    NSXPCConnection *connection = nil;
    if (connectionToDeviceDB(&connection) == kQ3Success)
    {
        dispatch_semaphore_t sema = dispatch_semaphore_create(0);
        [[connection remoteObjectProxy] decommissionController:controllerUUID
                                                         reply:^{
            dispatch_semaphore_signal(sema);
        }];
        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);

        // Note: we do NOT CFRelease controllerRef here. The caller (driver) retains ownership
        // of the ref and may continue using it for post-decommission queries (GetActivation,
        // HasTracker, etc.). The ref is released when the process exits or the driver disposes
        // its own reference.

        return kQ3Success;
    }

    return kQ3Failure;
}

//=============================================================================
//      CC3OSXController_GetValueCount
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_GetValueCount(TQ3ControllerRef controllerRef, TQ3Uns32 *valueCount)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] getValueCountWithReply:^(TQ3Uns32 count, TQ3Status stat) {
            *valueCount = count;
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_SetChannel
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_SetChannel(TQ3ControllerRef controllerRef, TQ3Uns32 channel, const void *data, TQ3Uns32 dataSize)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        NSData *channelData = [NSData dataWithBytes:data length:dataSize];

        [[connection remoteObjectProxy] setChannel:channel
                                           withData:channelData
                                             ofSize:dataSize
                                              reply:^(TQ3Status stat) {
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_GetChannel
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_GetChannel(TQ3ControllerRef controllerRef, TQ3Uns32 channel, void *data, TQ3Uns32 *dataSize)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] getChannel:channel
                                             reply:^(NSData *channelData, TQ3Uns32 size, TQ3Status stat) {
            if (channelData && data && dataSize)
            {
                TQ3Uns32 copySize = (TQ3Uns32)[channelData length];
                [channelData getBytes:data length:copySize];
                *dataSize = copySize;
            }
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_SetTracker
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_SetTracker(TQ3ControllerRef controllerRef, TC3TrackerInstanceDataPtr tracker)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)tracker;
        NSString *trackerUUID = trackerXPC ? trackerXPC->trackerUUID : nil;
        TQ3Boolean attachToSysCursor = (tracker == nullptr) ? kQ3True : kQ3False;

        [[connection remoteObjectProxy] setTracker:trackerUUID
                                 attachToSysCursor:attachToSysCursor
                                             reply:^(TQ3Status stat) {
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_HasTracker
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_HasTracker(TQ3ControllerRef controllerRef, TQ3Boolean *hasTracker)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] hasTrackerWithReply:^(TQ3Boolean has, TQ3Status stat) {
            *hasTracker = has;
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_Track2DCursor
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_Track2DCursor(TQ3ControllerRef controllerRef, TQ3Boolean *track2DCursor)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] track2DCursorWithReply:^(TQ3Boolean track, TQ3Status stat) {
            *track2DCursor = track;
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_Track3DCursor
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_Track3DCursor(TQ3ControllerRef controllerRef, TQ3Boolean *track3DCursor)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] track3DCursorWithReply:^(TQ3Boolean track, TQ3Status stat) {
            *track3DCursor = track;
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_GetTrackerPosition
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_GetTrackerPosition(TQ3ControllerRef controllerRef, TQ3Point3D *position)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] getTrackerPositionWithReply:^(TQ3Point3D pos, TQ3Status stat) {
            *position = pos;
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_SetTrackerPosition
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_SetTrackerPosition(TQ3ControllerRef controllerRef, const TQ3Point3D *position)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] setTrackerPosition:*position
                                                     reply:^(TQ3Status stat) {
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_MoveTrackerPosition
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_MoveTrackerPosition(TQ3ControllerRef controllerRef, const TQ3Vector3D *delta)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] moveTrackerPosition:*delta
                                                      reply:^(TQ3Status stat) {
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_GetTrackerOrientation
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_GetTrackerOrientation(TQ3ControllerRef controllerRef, TQ3Quaternion *orientation)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] getTrackerOrientationWithReply:^(TQ3Quaternion orient, TQ3Status stat) {
            *orientation = orient;
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_SetTrackerOrientation
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_SetTrackerOrientation(TQ3ControllerRef controllerRef, const TQ3Quaternion *orientation)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] setTrackerOrientation:*orientation
                                                        reply:^(TQ3Status stat) {
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

//=============================================================================
//      CC3OSXController_MoveTrackerOrientation
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXController_MoveTrackerOrientation(TQ3ControllerRef controllerRef, const TQ3Quaternion *delta)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        [[connection remoteObjectProxy] moveTrackerOrientation:*delta
                                                         reply:^(TQ3Status stat) {
            status = stat;
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return status;
}

#pragma mark - Controller State Functions

//=============================================================================
//      controllerObjectForUUID  (internal helper)
//-----------------------------------------------------------------------------
static Q3DcontrollerXPC *controllerObjectForUUID(NSString *uuid)
{
    if (!sharedDeviceDB || !uuid) return nil;
    for (Q3DcontrollerXPC *c in sharedDeviceDB.controllerObjects)
    {
        if ([c.UUID isEqualToString:uuid])
            return c;
    }
    return nil;
}

//=============================================================================
//      CC3OSXControllerState_New
//-----------------------------------------------------------------------------
TC3ControllerStateInstanceDataPtr
CC3OSXControllerState_New(TQ3Object theObject, TQ3ControllerRef theController)
{
    // Controller state is managed locally, not via XPC
    // Allocate a state instance
    TC3ControllerStateInstanceDataPtr stateData =
        (TC3ControllerStateInstanceDataPtr)calloc(1, sizeof(TC3ControllerStateInstanceData));

    if (stateData)
    {
        stateData->myController = theController;
        stateData->ctrlStateUUIDString = [[NSUUID UUID] UUIDString];
    }

    return stateData;
}

//=============================================================================
//      CC3OSXControllerState_Delete
//-----------------------------------------------------------------------------
TC3ControllerStateInstanceDataPtr
CC3OSXControllerState_Delete(TC3ControllerStateInstanceDataPtr controllerState)
{
    if (controllerState)
    {
        if (controllerState->ctrlStateUUIDString)
        {
            controllerState->ctrlStateUUIDString = nil;
        }
        free(controllerState);
    }
    return nullptr;
}

//=============================================================================
//      CC3OSXControllerState_SaveAndReset
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXControllerState_SaveAndReset(TC3ControllerStateInstanceDataPtr controllerStateObject)
{
    if (!controllerStateObject) return kQ3Failure;

    NSString *uuid = (__bridge NSString *)controllerStateObject->myController;
    Q3DcontrollerXPC *ctrl = controllerObjectForUUID(uuid);
    if (!ctrl) return kQ3Failure;

    TQ3Uns32 channelCount = ctrl.channelCount;
    if (channelCount > TC3_MAX_SAVED_CHANNELS) channelCount = TC3_MAX_SAVED_CHANNELS;
    controllerStateObject->savedChannelCount = channelCount;

    TQ3ChannelGetMethod getMethod = ctrl.channelGetMethod;
    TQ3ChannelSetMethod setMethod = ctrl.channelSetMethod;

    TQ3Uns32 zero = 0;
    for (TQ3Uns32 i = 0; i < channelCount; i++)
    {
        TQ3Uns32 val = 0;
        TQ3Uns32 size = sizeof(TQ3Uns32);
        if (getMethod)
            getMethod(ctrl.controllerRef, i, &val, &size);
        controllerStateObject->savedChannelData[i] = val;

        // Reset channel to 0
        if (setMethod)
            setMethod(ctrl.controllerRef, i, &zero, sizeof(TQ3Uns32));
    }

    return kQ3Success;
}

//=============================================================================
//      CC3OSXControllerState_Restore
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXControllerState_Restore(TC3ControllerStateInstanceDataPtr controllerStateObject)
{
    if (!controllerStateObject) return kQ3Failure;

    NSString *uuid = (__bridge NSString *)controllerStateObject->myController;
    Q3DcontrollerXPC *ctrl = controllerObjectForUUID(uuid);
    if (!ctrl) return kQ3Failure;

    TQ3ChannelSetMethod setMethod = ctrl.channelSetMethod;
    if (!setMethod) return kQ3Success;

    TQ3Uns32 channelCount = controllerStateObject->savedChannelCount;
    for (TQ3Uns32 i = 0; i < channelCount; i++)
    {
        TQ3Uns32 val = controllerStateObject->savedChannelData[i];
        setMethod(ctrl.controllerRef, i, &val, sizeof(TQ3Uns32));
    }

    return kQ3Success;
}

#pragma mark - Tracker Functions

//=============================================================================
//      CC3OSXTracker_New
//-----------------------------------------------------------------------------
TC3TrackerInstanceDataPtr
CC3OSXTracker_New(TQ3Object theObject, TQ3TrackerNotifyFunc notifyFunc)
{
    // Create a tracker object
    __block TC3TrackerInstanceDataXPC *trackerData = nullptr;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSXPCConnection *connection = nil;
    if (connectionToDeviceDB(&connection) == kQ3Success)
    {
        TQ3Object capturedObj = theObject;
        TQ3TrackerNotifyFunc capturedFunc = notifyFunc;
        [[connection remoteObjectProxy] newTrackerWithReply:^(NSString *uuid) {
            if (uuid)
            {
                trackerData = (TC3TrackerInstanceDataXPC *)calloc(1, sizeof(TC3TrackerInstanceDataXPC));
                if (trackerData)
                {
                    trackerData->trackerUUID = [uuid copy];
                    Q3TrackerXPC *xpcObj = [[Q3TrackerXPC alloc] initWithUUID:uuid
                                                                   notifyFunc:capturedFunc
                                                                trackerObject:capturedObj];
                    Q3TrackerXPC_Register(uuid, xpcObj);
                    trackerData->trackerXPCObject = (__bridge void *)xpcObj;
                }
            }
            dispatch_semaphore_signal(sema);
        }];

        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    }

    return (TC3TrackerInstanceDataPtr)trackerData;
}

//=============================================================================
//      CC3OSXTracker_Delete
//-----------------------------------------------------------------------------
TC3TrackerInstanceDataPtr
CC3OSXTracker_Delete(TC3TrackerInstanceDataPtr trackerObject)
{
    if (trackerObject)
    {
        TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)trackerObject;

        if (trackerXPC->trackerUUID)
        {
            Q3TrackerXPC_Unregister(trackerXPC->trackerUUID);
            trackerXPC->trackerXPCObject = nullptr;

            NSXPCConnection *connection = nil;
            if (connectionToDeviceDB(&connection) == kQ3Success)
            {
                [[connection remoteObjectProxy] deleteTracker:trackerXPC->trackerUUID reply:^{}];
            }

            trackerXPC->trackerUUID = nil; // ARC releases the NSString
        }
        free(trackerXPC);
    }
    return nullptr;
}

//=============================================================================
//      CC3OSXTracker_SetNotifyThresholds
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_SetNotifyThresholds(TC3TrackerInstanceDataPtr trackerObject, float positionThresh, float orientationThresh)
{
    if (trackerObject)
    {
        TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)trackerObject;
        Q3TrackerXPC *xpcObj = (__bridge Q3TrackerXPC *)trackerXPC->trackerXPCObject;
        if (xpcObj)
        {
            xpcObj.positionThreshold = positionThresh;
            xpcObj.orientationThreshold = orientationThresh;
            return kQ3Success;
        }
    }
    return kQ3Failure;
}

//=============================================================================
//      CC3OSXTracker_GetNotifyThresholds
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_GetNotifyThresholds(TC3TrackerInstanceDataPtr trackerObject, float *positionThresh, float *orientationThresh)
{
    if (trackerObject)
    {
        TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)trackerObject;
        Q3TrackerXPC *xpcObj = (__bridge Q3TrackerXPC *)trackerXPC->trackerXPCObject;
        if (xpcObj)
        {
            if (positionThresh)    *positionThresh    = xpcObj.positionThreshold;
            if (orientationThresh) *orientationThresh = xpcObj.orientationThreshold;
            return kQ3Success;
        }
    }
    if (positionThresh) *positionThresh = 0.0f;
    if (orientationThresh) *orientationThresh = 0.0f;
    return kQ3Failure;
}

//=============================================================================
//      CC3OSXTracker_SetActivation
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_SetActivation(TC3TrackerInstanceDataPtr trackerObject, TQ3Boolean active)
{
    if (trackerObject)
    {
        TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)trackerObject;
        Q3TrackerXPC *xpcObj = (__bridge Q3TrackerXPC *)trackerXPC->trackerXPCObject;
        if (xpcObj) { xpcObj.isActive = active; return kQ3Success; }
    }
    return kQ3Failure;
}

//=============================================================================
//      CC3OSXTracker_GetActivation
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_GetActivation(TC3TrackerInstanceDataPtr trackerObject, TQ3Boolean *active)
{
    if (trackerObject)
    {
        TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)trackerObject;
        Q3TrackerXPC *xpcObj = (__bridge Q3TrackerXPC *)trackerXPC->trackerXPCObject;
        if (xpcObj) { if (active) *active = xpcObj.isActive; return kQ3Success; }
    }
    if (active) *active = kQ3False;
    return kQ3Failure;
}

//=============================================================================
//      CC3OSXTracker_GetButtons
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_GetButtons(TC3TrackerInstanceDataPtr trackerObject, TQ3Uns32 *buttons)
{
    if (trackerObject && buttons)
    {
        TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)trackerObject;
        Q3TrackerXPC *xpcObj = (__bridge Q3TrackerXPC *)trackerXPC->trackerXPCObject;
        if (xpcObj)
        {
            *buttons = (xpcObj.isActive == kQ3True) ? xpcObj.theButtons : 0;
            return kQ3Success;
        }
    }
    if (buttons) *buttons = 0;
    return kQ3Failure;
}

//=============================================================================
//      CC3OSXTracker_ChangeButtons
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_ChangeButtons(TC3TrackerInstanceDataPtr trackerObject, TQ3ControllerRef controllerRef, TQ3Uns32 buttons, TQ3Uns32 buttonMask)
{
    if (trackerObject)
    {
        NSString *controllerUUID = (__bridge NSString *)controllerRef;
        NSXPCConnection *connection = connectionForController(controllerUUID);
        if (connection)
        {
            // Read-modify-write: apply buttonMask to current value
            __block TQ3Uns32 currentButtons = 0;
            __block TQ3Status getStatus = kQ3Failure;
            dispatch_semaphore_t getSema = dispatch_semaphore_create(0);
            [[connection remoteObjectProxy] getButtonsWithReply:^(TQ3Uns32 btns, TQ3Status stat) {
                currentButtons = btns;
                getStatus = stat;
                dispatch_semaphore_signal(getSema);
            }];
            dispatch_semaphore_wait(getSema, DISPATCH_TIME_FOREVER);

            if (getStatus == kQ3Success)
            {
                TQ3Uns32 newButtons = (currentButtons & ~buttonMask) | (buttons & buttonMask);
                __block TQ3Status setStatus = kQ3Failure;
                dispatch_semaphore_t setSema = dispatch_semaphore_create(0);
                [[connection remoteObjectProxy] setButtons:newButtons reply:^(TQ3Status stat) {
                    setStatus = stat;
                    dispatch_semaphore_signal(setSema);
                }];
                dispatch_semaphore_wait(setSema, DISPATCH_TIME_FOREVER);
                return setStatus;
            }
        }
    }
    return kQ3Failure;
}

//=============================================================================
//      CC3OSXTracker_GetPosition
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_GetPosition(TC3TrackerInstanceDataPtr trackerObject, TQ3Point3D *position, TQ3Vector3D *delta, TQ3Boolean *changed, TQ3Uns32 *serialNumber)
{
    if (trackerObject)
    {
        TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)trackerObject;
        Q3TrackerXPC *xpcObj = (__bridge Q3TrackerXPC *)trackerXPC->trackerXPCObject;
        if (xpcObj)
        {
            if (xpcObj.isActive == kQ3True)
            {
                TQ3Uns32 currentSerial = xpcObj.positionSerialNumber;
                if (position) *position = xpcObj.position;
                if (delta)    { delta->x = delta->y = delta->z = 0.0f; }
                if (changed)  *changed = (!serialNumber || *serialNumber != currentSerial) ? kQ3True : kQ3False;
                if (serialNumber) *serialNumber = currentSerial;
            }
            else
            {
                // Inactive tracker: return (0,0,0), do NOT update serialNumber
                if (position) { position->x = position->y = position->z = 0.0f; }
                if (delta)    { delta->x = delta->y = delta->z = 0.0f; }
                if (changed)  *changed = kQ3False;
                // serialNumber intentionally NOT updated
            }
            return kQ3Success;
        }
    }
    if (position) { position->x = position->y = position->z = 0.0f; }
    if (delta)    { delta->x = delta->y = delta->z = 0.0f; }
    if (changed)  *changed = kQ3False;
    return kQ3Failure;
}

//=============================================================================
//      CC3OSXTracker_SetPosition
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_SetPosition(TC3TrackerInstanceDataPtr trackerObject, TQ3ControllerRef controllerRef, const TQ3Point3D *position)
{
    if (trackerObject && position)
    {
        NSString *controllerUUID = (__bridge NSString *)controllerRef;
        NSXPCConnection *connection = connectionForController(controllerUUID);
        if (connection)
        {
            __block TQ3Status status = kQ3Failure;
            dispatch_semaphore_t sema = dispatch_semaphore_create(0);
            [[connection remoteObjectProxy] setTrackerPosition:*position reply:^(TQ3Status stat) {
                status = stat;
                dispatch_semaphore_signal(sema);
            }];
            dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
            return status;
        }
    }
    return kQ3Failure;
}

//=============================================================================
//      CC3OSXTracker_MovePosition
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_MovePosition(TC3TrackerInstanceDataPtr trackerObject, TQ3ControllerRef controllerRef, const TQ3Vector3D *delta)
{
    if (trackerObject && delta)
    {
        NSString *controllerUUID = (__bridge NSString *)controllerRef;
        NSXPCConnection *connection = connectionForController(controllerUUID);
        if (connection)
        {
            __block TQ3Status status = kQ3Failure;
            dispatch_semaphore_t sema = dispatch_semaphore_create(0);
            [[connection remoteObjectProxy] moveTrackerPosition:*delta reply:^(TQ3Status stat) {
                status = stat;
                dispatch_semaphore_signal(sema);
            }];
            dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
            return status;
        }
    }
    return kQ3Failure;
}

//=============================================================================
//      CC3OSXTracker_GetOrientation
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_GetOrientation(TC3TrackerInstanceDataPtr trackerObject, TQ3Quaternion *orientation, TQ3Quaternion *delta, TQ3Boolean *changed, TQ3Uns32 *serialNumber)
{
    if (trackerObject)
    {
        TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)trackerObject;
        Q3TrackerXPC *xpcObj = (__bridge Q3TrackerXPC *)trackerXPC->trackerXPCObject;
        if (xpcObj)
        {
            if (xpcObj.isActive == kQ3True)
            {
                TQ3Uns32 currentSerial = xpcObj.orientationSerialNumber;
                if (orientation) *orientation = xpcObj.orientation;
                if (delta)       { delta->w = 1.0f; delta->x = delta->y = delta->z = 0.0f; }
                if (changed)     *changed = (!serialNumber || *serialNumber != currentSerial) ? kQ3True : kQ3False;
                if (serialNumber) *serialNumber = currentSerial;
            }
            else
            {
                // Inactive tracker: return identity, do NOT update serialNumber
                if (orientation) { orientation->w = 1.0f; orientation->x = orientation->y = orientation->z = 0.0f; }
                if (delta)       { delta->w = 1.0f; delta->x = delta->y = delta->z = 0.0f; }
                if (changed)     *changed = kQ3False;
                // serialNumber intentionally NOT updated
            }
            return kQ3Success;
        }
    }
    if (orientation) { orientation->w = 1.0f; orientation->x = orientation->y = orientation->z = 0.0f; }
    if (delta)       { delta->w = 1.0f; delta->x = delta->y = delta->z = 0.0f; }
    if (changed) *changed = kQ3False;
    return kQ3Failure;
}

//=============================================================================
//      CC3OSXTracker_SetOrientation
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_SetOrientation(TC3TrackerInstanceDataPtr trackerObject, TQ3ControllerRef controllerRef, const TQ3Quaternion *orientation)
{
    if (trackerObject && orientation)
    {
        NSString *controllerUUID = (__bridge NSString *)controllerRef;
        NSXPCConnection *connection = connectionForController(controllerUUID);
        if (connection)
        {
            __block TQ3Status status = kQ3Failure;
            dispatch_semaphore_t sema = dispatch_semaphore_create(0);
            [[connection remoteObjectProxy] setTrackerOrientation:*orientation reply:^(TQ3Status stat) {
                status = stat;
                dispatch_semaphore_signal(sema);
            }];
            dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
            return status;
        }
    }
    return kQ3Failure;
}

//=============================================================================
//      CC3OSXTracker_MoveOrientation
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_MoveOrientation(TC3TrackerInstanceDataPtr trackerObject, TQ3ControllerRef controllerRef, const TQ3Quaternion *delta)
{
    if (trackerObject && delta)
    {
        NSString *controllerUUID = (__bridge NSString *)controllerRef;
        NSXPCConnection *connection = connectionForController(controllerUUID);
        if (connection)
        {
            __block TQ3Status status = kQ3Failure;
            dispatch_semaphore_t sema = dispatch_semaphore_create(0);
            [[connection remoteObjectProxy] moveTrackerOrientation:*delta reply:^(TQ3Status stat) {
                status = stat;
                dispatch_semaphore_signal(sema);
            }];
            dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
            return status;
        }
    }
    return kQ3Failure;
}

//=============================================================================
//      CC3OSXTracker_SetEventCoordinates
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_SetEventCoordinates(TC3TrackerInstanceDataPtr trackerObject, TQ3Uns32 timeStamp, TQ3Uns32 buttons, const TQ3Point3D *position, const TQ3Quaternion *orientation)
{
    if (trackerObject)
    {
        TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)trackerObject;

        __block TQ3Status status = kQ3Failure;
        dispatch_semaphore_t sema = dispatch_semaphore_create(0);

        NSXPCConnection *connection = nil;
        if (connectionToDeviceDB(&connection) == kQ3Success)
        {
            TQ3Point3D pos = position ? *position : (TQ3Point3D){0, 0, 0};
            TQ3Quaternion orient = orientation ? *orientation : (TQ3Quaternion){1, 0, 0, 0};

            [[connection remoteObjectProxy] setEventCoordinatesForTracker:trackerXPC->trackerUUID
                                                                timestamp:timeStamp
                                                                  buttons:buttons
                                                                 position:pos
                                                              orientation:orient
                                                                    reply:^(TQ3Status stat) {
                status = stat;
                dispatch_semaphore_signal(sema);
            }];

            dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
            return status;
        }
    }
    return kQ3Failure;
}

//=============================================================================
//      CC3OSXTracker_GetEventCoordinates
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_GetEventCoordinates(TC3TrackerInstanceDataPtr trackerObject, TQ3Uns32 timeStamp, TQ3Uns32 *buttons, TQ3Point3D *position, TQ3Quaternion *orientation)
{
    if (trackerObject)
    {
        TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)trackerObject;

        __block TQ3Status status = kQ3Failure;
        dispatch_semaphore_t sema = dispatch_semaphore_create(0);

        NSXPCConnection *connection = nil;
        if (connectionToDeviceDB(&connection) == kQ3Success)
        {
            [[connection remoteObjectProxy] getEventCoordinatesForTracker:trackerXPC->trackerUUID
                                                                timestamp:timeStamp
                                                                    reply:^(TQ3Uns32 btns,
                                                                           TQ3Point3D pos,
                                                                           TQ3Quaternion orient,
                                                                           TQ3Status stat) {
                if (buttons) *buttons = btns;
                if (position) *position = pos;
                if (orientation) *orientation = orient;
                status = stat;
                dispatch_semaphore_signal(sema);
            }];

            dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
            return status;
        }
    }
    if (buttons) *buttons = 0;
    if (position) {
        position->x = position->y = position->z = 0.0f;
    }
    if (orientation) {
        orientation->w = 1.0f;
        orientation->x = orientation->y = orientation->z = 0.0f;
    }
    return kQ3Failure;
}

#pragma mark - Cleanup

static void CC3OSX_CleanupXPCConnections(void)
{
    // Invalidate all controller connections
    [controllerConnections enumerateKeysAndObjectsUsingBlock:^(NSString *key, NSXPCConnection *conn, BOOL *stop) {
        [conn invalidate];
    }];
    [controllerConnections removeAllObjects];

    // Release controller listeners (kept alive to support connections)
    [controllerListeners removeAllObjects];

    // Stop device DB listener
    [deviceDBListener invalidate];
    deviceDBListener = nil;

    // Release device DB
    sharedDeviceDB = nil;

#if Q3_DEBUG
    NSLog(@"Cleaned up in-process XPC connections");
#endif
}
