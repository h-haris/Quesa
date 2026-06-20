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

#import <Foundation/Foundation.h>

//=============================================================================
//      Internal variables
//-----------------------------------------------------------------------------

// Mach bootstrap service name used by both the device-DB host and clients.
static NSString * const kQ3DeviceDBServiceName = @"com.quesa.devicedb";

// Set when this process hosts the device database.
static Q3DdbXPC        *sharedDeviceDB      = nil;
static NSXPCListener   *deviceDBListener     = nil;  // anonymous — used for the in-process connection
static NSXPCListener   *deviceDBMachListener = nil;  // Mach-name listener — effective when this process is the QuesaDeviceDB LaunchAgent

// Single connection to the device database (may be in-process or cross-process).
static NSXPCConnection *sDeviceDBConnection = nil;

// Per-controller cached connections and their backing listeners.
static NSMutableDictionary<NSString *, NSXPCConnection *> *controllerConnections = nil;
static NSMutableDictionary<NSString *, NSXPCListener *>   *controllerListeners   = nil;

// Driver-side channel state: per-controller objects + anonymous listeners kept alive.
// Only populated in driver processes (when channel methods are present).
static NSMutableDictionary<NSString *, id>             *driverStateObjects   = nil;
static NSMutableDictionary<NSString *, NSXPCListener *> *driverStateListeners = nil;

// Channel count cache: populated at newController time for use in SaveAndReset.
static NSMutableDictionary<NSString *, NSNumber *> *sCachedChannelCounts = nil;

//=============================================================================
//      Internal function prototypes
//-----------------------------------------------------------------------------

static void CC3OSX_CleanupXPCConnections(void);

//=============================================================================
//      xpcSync / XPC_SYNC
//
//      The public Quesa controller API (Q3Controller_GetSignature, Q3Tracker_GetPosition,
//      etc.) is a synchronous C API — functions return TQ3Status directly, as declared
//      in QuesaController.h and specified in "3D Graphics Programming With QuickDraw 3D
//      1.5.4" (Apple, 1995).  Callers expect a return value immediately.
//      The underlying transport is XPC, which is inherently asynchronous — you send
//      a message and get the result in a reply block.
//
//      XPC_SYNC bridges that mismatch: it blocks the calling thread on a semaphore
//      until the XPC reply arrives, making the async call appear synchronous to the
//      caller.  The alternative would be redesigning the public Quesa API to be
//      callback-based, which would break all existing application code written against
//      the original synchronous QuickDraw 3D API.
//
//      Usage: call done() inside the reply block to unblock the caller.
//-----------------------------------------------------------------------------
static void xpcSync(const char *caller, void (^body)(dispatch_block_t done))
{
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);
    body(^{ dispatch_semaphore_signal(sema); });
    if (dispatch_semaphore_wait(sema, dispatch_time(DISPATCH_TIME_NOW, 30 * NSEC_PER_SEC)) != 0)
        NSLog(@"[XPC] Timeout in %s - reply never arrived", caller);
}
#define XPC_SYNC(body) xpcSync(__func__, body)

#pragma mark - Driver-State XPC (driver side)

// Implements Q3XPCControllerDriverState in the driver process.
// The LaunchAgent's Q3DcontrollerXPC connects back to this for SetChannel / GetChannel.
@interface Q3DriverStateXPC : NSObject <Q3XPCControllerDriverState, NSXPCListenerDelegate>
- (instancetype)initWithChannelSetMethod:(TQ3ChannelSetMethod)setMethod
                        channelGetMethod:(TQ3ChannelGetMethod)getMethod
                           controllerRef:(TQ3ControllerRef)ref;
@end

@implementation Q3DriverStateXPC {
    TQ3ChannelSetMethod _setMethod;
    TQ3ChannelGetMethod _getMethod;
    TQ3ControllerRef    _ref;
}

- (instancetype)initWithChannelSetMethod:(TQ3ChannelSetMethod)setMethod
                        channelGetMethod:(TQ3ChannelGetMethod)getMethod
                           controllerRef:(TQ3ControllerRef)ref
{
    if (self = [super init]) {
        _setMethod = setMethod;
        _getMethod = getMethod;
        _ref       = ref;
    }
    return self;
}

- (void)setChannel:(TQ3Uns32)channel
          withData:(NSData * _Nullable)data
            ofSize:(TQ3Uns32)dataSize
             reply:(void (^)(TQ3Status))reply
{
    if (_setMethod && data)
        _setMethod(_ref, channel, data.bytes, dataSize);
    reply(kQ3Success);
}

- (void)getChannel:(TQ3Uns32)channel
             reply:(void (^)(NSData * _Nullable, TQ3Uns32, TQ3Status))reply
{
    uint8_t buf[256] = {0};
    TQ3Uns32 size = sizeof(buf);
    if (_getMethod)
        _getMethod(_ref, channel, buf, &size);
    reply([NSData dataWithBytes:buf length:size], size, kQ3Success);
}

// State save/restore across processes is not supported — return failure so
// callers fall back to the in-process path when available.
- (void)newDrvStateWithUUID:(NSString *)stateUUID reply:(void (^)(TQ3Status))reply
    { reply(kQ3Failure); }
- (void)deleteDrvStateWithUUID:(NSString *)stateUUID reply:(void (^)(TQ3Status))reply
    { reply(kQ3Failure); }
- (void)saveDrvResetStateWithUUID:(NSString *)stateUUID reply:(void (^)(TQ3Status))reply
    { reply(kQ3Failure); }
- (void)restoreDrvStateWithUUID:(NSString *)stateUUID reply:(void (^)(TQ3Status))reply
    { reply(kQ3Failure); }

// NSXPCListenerDelegate — accept connections from the LaunchAgent.
- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)conn
{
    conn.exportedInterface = [NSXPCInterface interfaceWithProtocol:
                                @protocol(Q3XPCControllerDriverState)];
    conn.exportedObject = self;
    [conn resume];
    return YES;
}

@end

#pragma mark - In-Process XPC Setup

// Start hosting the device database in this process.
//
// An anonymous listener is created for the reliable in-process XPC connection.
// A Mach-service listener is also started under kQ3DeviceDBServiceName; this
// is only effective when the process IS the launchd-registered host for that
// name (i.e. the QuesaDeviceDB LaunchAgent).  In all other processes it starts
// silently without registering, which is harmless.
static void initializeInProcessDeviceDB(void)
{
    if (sharedDeviceDB != nil)
        return;

    sharedDeviceDB = [[Q3DdbXPC alloc] initForInProcess];

    // Anonymous listener — always works, used for the in-process XPC connection.
    deviceDBListener = [NSXPCListener anonymousListener];
    deviceDBListener.delegate = sharedDeviceDB;
    [deviceDBListener resume];

    // Mach-service listener — effective only when this process is registered
    // with launchd as the QuesaDeviceDB LaunchAgent.
    deviceDBMachListener = [[NSXPCListener alloc] initWithMachServiceName:kQ3DeviceDBServiceName];
    deviceDBMachListener.delegate = sharedDeviceDB;
    [deviceDBMachListener resume];

    if (controllerConnections == nil)
        controllerConnections = [NSMutableDictionary dictionary];
    if (controllerListeners == nil)
        controllerListeners = [NSMutableDictionary dictionary];

    static dispatch_once_t atexitToken;
    dispatch_once(&atexitToken, ^{ atexit(CC3OSX_CleanupXPCConnections); });

#if Q3_DEBUG
    NSLog(@"Hosting device DB (anonymous + Mach service: %@)", kQ3DeviceDBServiceName);
#endif
}

// Start the device database server and run the main run loop indefinitely.
// Intended to be called from the QuesaDeviceDB LaunchAgent's main().
// Does not return under normal operation.
__attribute__((visibility("default")))
void Q3XPC_StartDeviceDBServer(void)
{
    initializeInProcessDeviceDB();
    [[NSRunLoop mainRunLoop] run];
}

// Return a connection to the device database.
//
// Strategy:
//   1. Try to connect to an already-running device DB via the Mach bootstrap name.
//      This succeeds when a driver process in another address space has already
//      called Q3Controller_New (which registers the DB under kQ3DeviceDBServiceName).
//   2. If no external server responds within 200 ms, become the server ourselves:
//      start an in-process DB and register it under the same Mach name so future
//      cross-process clients (other apps) can reach it.
static TQ3Status connectionToDeviceDB(NSXPCConnection **outConnection)
{
    if (sDeviceDBConnection)
    {
        *outConnection = sDeviceDBConnection;
        return kQ3Success;
    }

    // --- probe for the QuesaDeviceDB LaunchAgent via the Mach service name ---
    //
    // launchd activates the LaunchAgent on the first connection attempt, which
    // can take up to ~1 second on a cold start.  Retry up to 15 times (×200 ms)
    // for a total wait of ~3 s before giving up and falling back to in-process.
    for (int attempt = 0; attempt < 15; attempt++)
    {
        NSXPCConnection *probe = [[NSXPCConnection alloc]
            initWithMachServiceName:kQ3DeviceDBServiceName options:0];
        probe.remoteObjectInterface =
            [NSXPCInterface interfaceWithProtocol:@protocol(Q3XPCDeviceDB)];

        __block BOOL probeOK = NO;
        dispatch_semaphore_t sem = dispatch_semaphore_create(0);

        __block NSString *probeFailReason = nil;
        probe.invalidationHandler = ^{
            if (!probeFailReason) probeFailReason = @"invalidated";
            dispatch_semaphore_signal(sem);
        };
        [probe resume];

        [[probe remoteObjectProxyWithErrorHandler:^(NSError *err) {
            probeFailReason = [NSString stringWithFormat:@"error: %@", err.localizedDescription];
            dispatch_semaphore_signal(sem);
        }] getListChangedWithReply:^(TQ3Boolean ch, TQ3Uns32 sn, TQ3Status st) {
            probeOK = (st == kQ3Success);
            dispatch_semaphore_signal(sem);
        }];

        dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 200 * NSEC_PER_MSEC));
        if (!probeOK) NSLog(@"[DB probe] attempt %d failed: %@", attempt+1, probeFailReason ?: @"timeout");

        if (probeOK)
        {
            probe.invalidationHandler = ^{
                NSLog(@"Device DB connection (LaunchAgent) invalidated");
                sDeviceDBConnection = nil;
            };
            sDeviceDBConnection = probe;

            static dispatch_once_t atexitToken;
            dispatch_once(&atexitToken, ^{ atexit(CC3OSX_CleanupXPCConnections); });

#if Q3_DEBUG
            NSLog(@"Connected to QuesaDeviceDB LaunchAgent via Mach service: %@ (attempt %d)",
                  kQ3DeviceDBServiceName, attempt + 1);
#endif
            *outConnection = sDeviceDBConnection;
            return kQ3Success;
        }

        [probe invalidate];
    }

    // --- no external server found — become the server ---
    initializeInProcessDeviceDB();

    // Connect via the anonymous listener's endpoint (always works; the Mach-service
    // listener is for cross-process clients and is started alongside it).
    sDeviceDBConnection = [[NSXPCConnection alloc]
        initWithListenerEndpoint:deviceDBListener.endpoint];
    sDeviceDBConnection.remoteObjectInterface =
        [NSXPCInterface interfaceWithProtocol:@protocol(Q3XPCDeviceDB)];
    sDeviceDBConnection.interruptionHandler = ^{
        NSLog(@"Device DB connection (in-process) interrupted");
    };
    sDeviceDBConnection.invalidationHandler = ^{
        NSLog(@"Device DB connection (in-process) invalidated");
        sDeviceDBConnection = nil;
    };
    [sDeviceDBConnection resume];

#if Q3_DEBUG
    NSLog(@"Connected to in-process device DB");
#endif

    *outConnection = sDeviceDBConnection;
    return kQ3Success;
}

// Return a connection to a specific controller.
//
// In-process path  (sharedDeviceDB != nil): create an anonymous listener backed
//   by the Q3DcontrollerXPC object and connect to its endpoint locally.
// Cross-process path (sharedDeviceDB == nil): ask the device DB for the
//   controller's endpoint via the Q3XPCDeviceDB protocol, then connect.
static NSXPCConnection *connectionForController(NSString *controllerUUID)
{
    if (!controllerConnections)
        controllerConnections = [NSMutableDictionary dictionary];

    NSXPCConnection *connection = controllerConnections[controllerUUID];
    if (connection)
        return connection;

    // --- obtain the controller's listener endpoint ---
    NSXPCListenerEndpoint *endpoint = nil;

    if (sharedDeviceDB)
    {
        // In-process: create (or reuse) an anonymous listener on the controller object.
        if (!controllerListeners)
            controllerListeners = [NSMutableDictionary dictionary];

        NSXPCListener *listener = controllerListeners[controllerUUID];
        if (!listener)
        {
            Q3DcontrollerXPC *controller = nil;
            for (Q3DcontrollerXPC *c in sharedDeviceDB.controllerObjects)
            {
                if ([c.UUID isEqualToString:controllerUUID]) { controller = c; break; }
            }
            if (!controller) return nil;

            // The listener must outlive the connection — store it persistently.
            listener = [NSXPCListener anonymousListener];
            listener.delegate = (id<NSXPCListenerDelegate>)controller;
            [listener resume];
            controllerListeners[controllerUUID] = listener;
        }
        endpoint = listener.endpoint;
    }
    else
    {
        // Cross-process: ask the device DB (running in another process) for the endpoint.
        NSXPCConnection *dbConn = nil;
        if (connectionToDeviceDB(&dbConn) != kQ3Success) return nil;

        __block NSXPCListenerEndpoint *ep = nil;
        XPC_SYNC(^(dispatch_block_t done) {
            [[dbConn remoteObjectProxyWithErrorHandler:^(NSError *err) {
                done();
            }] connectionForController:controllerUUID reply:^(NSXPCListenerEndpoint *e) {
                ep = e;
                done();
            }];
        });
        endpoint = ep;
    }

    if (!endpoint) return nil;

    // --- build and cache the connection ---
    connection = [[NSXPCConnection alloc] initWithListenerEndpoint:endpoint];
    connection.remoteObjectInterface =
        [NSXPCInterface interfaceWithProtocol:@protocol(Q3XPCController)];
    connection.interruptionHandler = ^{
        NSLog(@"Controller connection interrupted: %@", controllerUUID);
    };
    connection.invalidationHandler = ^{
        [controllerConnections removeObjectForKey:controllerUUID];
        [controllerListeners   removeObjectForKey:controllerUUID];
    };
    [connection resume];
    controllerConnections[controllerUUID] = connection;

#if Q3_DEBUG
    NSLog(@"Created connection to controller: %@", controllerUUID);
#endif
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

    NSXPCConnection *connection = nil;
    if (connectionToDeviceDB(&connection) == kQ3Success)
    {
        TQ3Uns32 lastKnownSerialNumber = *serialNumber;
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] getListChangedWithReply:^(TQ3Boolean changed,
                                                                       TQ3Uns32 serNum,
                                                                       TQ3Status stat) {
                // Mirror PDO behavior: changed if serial number differs from caller's last known
                *listChanged = (serNum != lastKnownSerialNumber) ? kQ3True : kQ3False;
                *serialNumber = serNum;
                status = stat;
                done();
            }];
        });
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

    NSXPCConnection *connection = nil;
    if (connectionToDeviceDB(&connection) == kQ3Success)
    {
        NSString *currentUUID = (__bridge NSString *)controllerRef;

        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] nextCC3Controller:currentUUID
                                                        reply:^(NSString *nextUUID) {
                // __bridge_retained transfers ownership to the void* caller (caller retains for lifetime of the ref)
                *nextControllerRef = nextUUID ? (__bridge_retained TQ3ControllerRef)[nextUUID copy] : nullptr;
                status = kQ3Success;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] setActivation:active reply:^(TQ3Status stat) {
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] getActivationWithReply:^(TQ3Boolean act, TQ3Status stat) {
                *active = act;
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] getSignatureWithReply:^(NSString *sig, TQ3Status stat) {
                if (sig != nil && signature != nullptr && numChars > 0)
                {
                    strlcpy(signature, [sig UTF8String], numChars);
                }
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] getButtonsWithReply:^(TQ3Uns32 btns, TQ3Status stat) {
                *buttons = btns;
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] setButtons:buttons reply:^(TQ3Status stat) {
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
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
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        NSMutableArray<NSNumber *> *valuesArray = [NSMutableArray arrayWithCapacity:valueCount];
        for (TQ3Uns32 i = 0; i < valueCount; i++)
        {
            [valuesArray addObject:@(values[i])];
        }

        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] setValues:valuesArray
                                              ofCount:valueCount
                                                reply:^(TQ3Status stat) {
                status = stat;
                done();
            }];
        });
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

    NSXPCConnection *connection = nil;
    if (connectionToDeviceDB(&connection) == kQ3Success)
    {
        NSString *signature = controllerData->signature ?
            [NSString stringWithUTF8String:controllerData->signature] : @"";

        // Function pointers are only valid in-process (sharedDeviceDB != nil).
        // For the cross-process case they are sent as 0; the driver state XPC
        // endpoint registered below is used for channel calls instead.
        BOOL crossProcess = (sharedDeviceDB == nil);
        uint64_t setPtr = crossProcess ? 0 : (uint64_t)(uintptr_t)controllerData->channelSetMethod;
        uint64_t getPtr = crossProcess ? 0 : (uint64_t)(uintptr_t)controllerData->channelGetMethod;

        NSDictionary *dataDict = @{
            @"signature":         signature,
            @"valueCount":        @(controllerData->valueCount),
            @"channelCount":      @(controllerData->channelCount),
            @"channelSetMethod":  @(setPtr),
            @"channelGetMethod":  @(getPtr),
        };

        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] newController:dataDict
                                                    reply:^(NSString *uuid) {
                controllerRef = (__bridge_retained TQ3ControllerRef)[uuid copy];
                done();
            }];
        });
    }

    if (!controllerRef)
        return nullptr;

    NSString *uuid = (__bridge NSString *)controllerRef;

    // Cache channel count for use in CC3OSXControllerState_SaveAndReset.
    if (!sCachedChannelCounts) sCachedChannelCounts = [NSMutableDictionary dictionary];
    sCachedChannelCounts[uuid] = @(controllerData->channelCount);

    // Cross-process: if this driver has channel methods, expose them via a
    // per-controller anonymous XPC listener and register the endpoint with the DB.
    if (sharedDeviceDB == nil &&
        (controllerData->channelSetMethod || controllerData->channelGetMethod))
    {
        if (!driverStateObjects)   driverStateObjects   = [NSMutableDictionary dictionary];
        if (!driverStateListeners) driverStateListeners = [NSMutableDictionary dictionary];

        Q3DriverStateXPC *state = [[Q3DriverStateXPC alloc]
            initWithChannelSetMethod:controllerData->channelSetMethod
                    channelGetMethod:controllerData->channelGetMethod
                       controllerRef:controllerRef];

        NSXPCListener *listener = [NSXPCListener anonymousListener];
        listener.delegate = state;
        [listener resume];

        driverStateObjects[uuid]   = state;
        driverStateListeners[uuid] = listener;

        // Register the endpoint with the DB (fire-and-forget is fine).
        NSXPCConnection *dbConn = nil;
        if (connectionToDeviceDB(&dbConn) == kQ3Success)
        {
            [[dbConn remoteObjectProxy] setDriverStateEndpoint:listener.endpoint
                                                 forController:uuid
                                                         reply:^{}];
        }
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

    // Tear down driver state (if this process was acting as driver for that controller)
    [driverStateListeners removeObjectForKey:controllerUUID];
    [driverStateObjects   removeObjectForKey:controllerUUID];
    [sCachedChannelCounts removeObjectForKey:controllerUUID];

    // Notify device DB and wait for completion
    NSXPCConnection *connection = nil;
    if (connectionToDeviceDB(&connection) == kQ3Success)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] decommissionController:controllerUUID
                                                             reply:^{
                done();
            }];
        });

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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] getValueCountWithReply:^(TQ3Uns32 count, TQ3Status stat) {
                *valueCount = count;
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        NSData *channelData = [NSData dataWithBytes:data length:dataSize];

        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] setChannel:channel
                                               withData:channelData
                                                 ofSize:dataSize
                                                  reply:^(TQ3Status stat) {
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] getChannel:channel
                                                 reply:^(NSData *channelData, TQ3Uns32 size, TQ3Status stat) {
                if (channelData && data && dataSize)
                {
                    TQ3Uns32 copySize = (TQ3Uns32)[channelData length];
                    [channelData getBytes:data length:copySize];
                    *dataSize = copySize;
                }
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)tracker;
        NSString *trackerUUID = trackerXPC ? trackerXPC->trackerUUID : nil;
        // Provide the tracker's listener endpoint so cross-process controllers can
        // reach back to the tracker living in this process.
        NSXPCListenerEndpoint *trackerEndpoint = trackerUUID
            ? Q3TrackerXPC_EndpointForUUID(trackerUUID) : nil;
        TQ3Boolean attachToSysCursor = (tracker == nullptr) ? kQ3True : kQ3False;

        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] setTracker:trackerUUID
                                       trackerEndpoint:trackerEndpoint
                                     attachToSysCursor:attachToSysCursor
                                                 reply:^(TQ3Status stat) {
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] hasTrackerWithReply:^(TQ3Boolean has, TQ3Status stat) {
                *hasTracker = has;
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] track2DCursorWithReply:^(TQ3Boolean track, TQ3Status stat) {
                *track2DCursor = track;
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] track3DCursorWithReply:^(TQ3Boolean track, TQ3Status stat) {
                *track3DCursor = track;
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] getTrackerPositionWithReply:^(TQ3Point3D pos, TQ3Status stat) {
                *position = pos;
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] setTrackerPosition:*position
                                                         reply:^(TQ3Status stat) {
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] moveTrackerPosition:*delta
                                                          reply:^(TQ3Status stat) {
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] getTrackerOrientationWithReply:^(TQ3Quaternion orient, TQ3Status stat) {
                *orientation = orient;
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] setTrackerOrientation:*orientation
                                                            reply:^(TQ3Status stat) {
                status = stat;
                done();
            }];
        });
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

    NSString *controllerUUID = (__bridge NSString *)controllerRef;
    NSXPCConnection *connection = connectionForController(controllerUUID);

    if (connection)
    {
        XPC_SYNC(^(dispatch_block_t done) {
            [[connection remoteObjectProxy] moveTrackerOrientation:*delta
                                                             reply:^(TQ3Status stat) {
                status = stat;
                done();
            }];
        });
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

    // --- In-process path: direct function pointer calls ---
    Q3DcontrollerXPC *ctrl = controllerObjectForUUID(uuid);
    if (ctrl)
    {
        TQ3Uns32 channelCount = ctrl.channelCount;
        if (channelCount > TC3_MAX_SAVED_CHANNELS) channelCount = TC3_MAX_SAVED_CHANNELS;
        controllerStateObject->savedChannelCount = channelCount;

        TQ3ChannelGetMethod getMethod = ctrl.channelGetMethod;
        TQ3ChannelSetMethod setMethod = ctrl.channelSetMethod;
        TQ3Uns32 zero = 0;
        for (TQ3Uns32 i = 0; i < channelCount; i++)
        {
            TQ3Uns32 val = 0, size = sizeof(TQ3Uns32);
            if (getMethod) getMethod(ctrl.controllerRef, i, &val, &size);
            controllerStateObject->savedChannelData[i] = val;
            if (setMethod) setMethod(ctrl.controllerRef, i, &zero, sizeof(TQ3Uns32));
        }
        return kQ3Success;
    }

    // --- Cross-process path: route through CC3OSXController_GetChannel/SetChannel ---
    // Those calls traverse: client → LaunchAgent → driver state XPC → driver.
    TQ3Uns32 channelCount = [sCachedChannelCounts[uuid] unsignedIntValue];
    if (channelCount > TC3_MAX_SAVED_CHANNELS) channelCount = TC3_MAX_SAVED_CHANNELS;
    controllerStateObject->savedChannelCount = channelCount;

    TQ3Uns32 zero = 0;
    for (TQ3Uns32 i = 0; i < channelCount; i++)
    {
        TQ3Uns32 val = 0, size = sizeof(TQ3Uns32);
        CC3OSXController_GetChannel(controllerStateObject->myController, i, &val, &size);
        controllerStateObject->savedChannelData[i] = val;
        CC3OSXController_SetChannel(controllerStateObject->myController, i, &zero, sizeof(TQ3Uns32));
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

    // --- In-process path ---
    Q3DcontrollerXPC *ctrl = controllerObjectForUUID(uuid);
    if (ctrl)
    {
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

    // --- Cross-process path ---
    TQ3Uns32 channelCount = controllerStateObject->savedChannelCount;
    for (TQ3Uns32 i = 0; i < channelCount; i++)
    {
        TQ3Uns32 val = controllerStateObject->savedChannelData[i];
        CC3OSXController_SetChannel(controllerStateObject->myController, i, &val, sizeof(TQ3Uns32));
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

    NSXPCConnection *connection = nil;
    if (connectionToDeviceDB(&connection) == kQ3Success)
    {
        TQ3Object capturedObj = theObject;
        TQ3TrackerNotifyFunc capturedFunc = notifyFunc;
        XPC_SYNC(^(dispatch_block_t done) {
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
                done();
            }];
        });
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
            XPC_SYNC(^(dispatch_block_t done) {
                [[connection remoteObjectProxy] getButtonsWithReply:^(TQ3Uns32 btns, TQ3Status stat) {
                    currentButtons = btns;
                    getStatus = stat;
                    done();
                }];
            });

            if (getStatus == kQ3Success)
            {
                TQ3Uns32 newButtons = (currentButtons & ~buttonMask) | (buttons & buttonMask);
                __block TQ3Status setStatus = kQ3Failure;
                XPC_SYNC(^(dispatch_block_t done) {
                    [[connection remoteObjectProxy] setButtons:newButtons reply:^(TQ3Status stat) {
                        setStatus = stat;
                        done();
                    }];
                });
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
        // NULL controllerRef means an app-initiated reset (no controller involved).
        // Update the local Q3TrackerXPC object directly — skip connectionForController
        // entirely to avoid a synchronous XPC round-trip on a nil UUID.
        if (!controllerRef)
        {
            TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)trackerObject;
            Q3TrackerXPC *xpcObj = (__bridge Q3TrackerXPC *)trackerXPC->trackerXPCObject;
            if (xpcObj)
            {
                xpcObj.position = *position;
                xpcObj.positionSerialNumber++;
                return kQ3Success;
            }
            return kQ3Failure;
        }
        NSString *controllerUUID = (__bridge NSString *)controllerRef;
        NSXPCConnection *connection = connectionForController(controllerUUID);
        if (connection)
        {
            __block TQ3Status status = kQ3Failure;
            XPC_SYNC(^(dispatch_block_t done) {
                [[connection remoteObjectProxy] setTrackerPosition:*position reply:^(TQ3Status stat) {
                    status = stat;
                    done();
                }];
            });
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
            XPC_SYNC(^(dispatch_block_t done) {
                [[connection remoteObjectProxy] moveTrackerPosition:*delta reply:^(TQ3Status stat) {
                    status = stat;
                    done();
                }];
            });
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
        // NULL controllerRef means an app-initiated reset (no controller involved).
        // Update the local Q3TrackerXPC object directly — skip connectionForController
        // entirely to avoid a synchronous XPC round-trip on a nil UUID.
        if (!controllerRef)
        {
            TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)trackerObject;
            Q3TrackerXPC *xpcObj = (__bridge Q3TrackerXPC *)trackerXPC->trackerXPCObject;
            if (xpcObj)
            {
                xpcObj.orientation = *orientation;
                xpcObj.orientationSerialNumber++;
                return kQ3Success;
            }
            return kQ3Failure;
        }
        NSString *controllerUUID = (__bridge NSString *)controllerRef;
        NSXPCConnection *connection = connectionForController(controllerUUID);
        if (connection)
        {
            __block TQ3Status status = kQ3Failure;
            XPC_SYNC(^(dispatch_block_t done) {
                [[connection remoteObjectProxy] setTrackerOrientation:*orientation reply:^(TQ3Status stat) {
                    status = stat;
                    done();
                }];
            });
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
            XPC_SYNC(^(dispatch_block_t done) {
                [[connection remoteObjectProxy] moveTrackerOrientation:*delta reply:^(TQ3Status stat) {
                    status = stat;
                    done();
                }];
            });
            return status;
        }
    }
    return kQ3Failure;
}

//=============================================================================
//      CC3OSXTracker_SetEventCoordinates
//
// Event coordinates are stored in the tracker object, which always lives in
// the client process.  Routing through the device DB (LaunchAgent) would fail
// cross-process because the DB calls Q3TrackerXPC_ForUUID in its own process
// where the tracker is not registered.  Use the local trackerXPCObject pointer
// directly — the same pattern used by SetActivation, GetPosition, etc.
//-----------------------------------------------------------------------------
TQ3Status
CC3OSXTracker_SetEventCoordinates(TC3TrackerInstanceDataPtr trackerObject, TQ3Uns32 timeStamp, TQ3Uns32 buttons, const TQ3Point3D *position, const TQ3Quaternion *orientation)
{
    if (trackerObject)
    {
        TC3TrackerInstanceDataXPC *trackerXPC = (TC3TrackerInstanceDataXPC *)trackerObject;
        Q3TrackerXPC *xpcObj = (__bridge Q3TrackerXPC *)trackerXPC->trackerXPCObject;
        if (xpcObj)
        {
            [xpcObj addEventTimestamp:timeStamp
                              buttons:buttons
                             position:position
                          orientation:orientation];
            return kQ3Success;
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
        Q3TrackerXPC *xpcObj = (__bridge Q3TrackerXPC *)trackerXPC->trackerXPCObject;
        if (xpcObj)
        {
            return [xpcObj getEventAtOrBeforeTimestamp:timeStamp
                                               buttons:buttons
                                              position:position
                                           orientation:orientation];
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
    [controllerConnections enumerateKeysAndObjectsUsingBlock:^(NSString *key,
                                                                NSXPCConnection *conn,
                                                                BOOL *stop) {
        [conn invalidate];
    }];
    [controllerConnections removeAllObjects];

    // Release controller listeners (kept alive to support connections)
    [controllerListeners removeAllObjects];

    // Tear down the device DB connection (client or in-process)
    [sDeviceDBConnection invalidate];
    sDeviceDBConnection = nil;

    // Stop device DB listeners and release DB (only set in the hosting process)
    [deviceDBMachListener invalidate];
    deviceDBMachListener = nil;
    [deviceDBListener invalidate];
    deviceDBListener = nil;
    sharedDeviceDB = nil;

#if Q3_DEBUG
    NSLog(@"Cleaned up XPC connections");
#endif
}
