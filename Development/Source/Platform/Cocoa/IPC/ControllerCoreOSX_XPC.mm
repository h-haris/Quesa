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

#import <Foundation/Foundation.h>

//=============================================================================
//      Internal variables
//-----------------------------------------------------------------------------

static NSXPCConnection *privateDeviceDBConnection = nil;
static NSMutableDictionary<NSString *, NSXPCConnection *> *controllerConnections = nil;

//=============================================================================
//      Internal function prototypes
//-----------------------------------------------------------------------------

#pragma mark - XPC Connection Management

static TQ3Status connectionToDeviceDB(NSXPCConnection **outConnection)
{
    if (privateDeviceDBConnection == nil)
    {
        privateDeviceDBConnection = [[NSXPCConnection alloc] 
            initWithMachServiceName:@kQuesa3DDeviceServerXPC
                            options:0];
        
        privateDeviceDBConnection.remoteObjectInterface = 
            [NSXPCInterface interfaceWithProtocol:@protocol(Q3XPCDeviceDB)];
        
        privateDeviceDBConnection.interruptionHandler = ^{
            NSLog(@"XPC connection to device database interrupted");
        };
        
        privateDeviceDBConnection.invalidationHandler = ^{
            NSLog(@"XPC connection to device database invalidated");
            privateDeviceDBConnection = nil;
        };
        
        [privateDeviceDBConnection resume];
        
#if Q3_DEBUG
        NSLog(@"Established XPC connection to device database");
#endif
    }
    
    *outConnection = privateDeviceDBConnection;
    return kQ3Success;
}

static NSXPCConnection *connectionForController(NSString *controllerUUID)
{
    if (controllerConnections == nil)
    {
        controllerConnections = [NSMutableDictionary dictionary];
    }
    
    NSXPCConnection *connection = controllerConnections[controllerUUID];
    
    if (connection == nil)
    {
        NSXPCConnection *dbConnection = nil;
        if (connectionToDeviceDB(&dbConnection) == kQ3Success)
        {
            // Request endpoint for this controller
            dispatch_semaphore_t sema = dispatch_semaphore_create(0);
            __block NSXPCListenerEndpoint *endpoint = nil;
            
            [[dbConnection remoteObjectProxy] connectionForController:controllerUUID
                                                                reply:^(NSXPCListenerEndpoint *ep) {
                endpoint = ep;
                dispatch_semaphore_signal(sema);
            }];
            
            dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
            
            if (endpoint)
            {
                connection = [[NSXPCConnection alloc] initWithListenerEndpoint:endpoint];
                connection.remoteObjectInterface = 
                    [NSXPCInterface interfaceWithProtocol:@protocol(Q3XPCController)];
                
                connection.interruptionHandler = ^{
                    NSLog(@"Controller connection interrupted: %@", controllerUUID);
                };
                
                connection.invalidationHandler = ^{
                    NSLog(@"Controller connection invalidated: %@", controllerUUID);
                    [controllerConnections removeObjectForKey:controllerUUID];
                };
                
                [connection resume];
                controllerConnections[controllerUUID] = connection;
            }
        }
    }
    
    return connection;
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
        [[connection remoteObjectProxy] getListChangedWithReply:^(TQ3Boolean changed, 
                                                                   TQ3Uns32 serNum,
                                                                   TQ3Status stat) {
            *listChanged = changed;
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
            *nextControllerRef = (__bridge TQ3ControllerRef)nextUUID;
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
            if (sig != nil)
            {
                [sig getCString:signature maxLength:numChars encoding:NSASCIIStringEncoding];
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
                    *changed = (*serialNumber != serNum) ? kQ3True : kQ3False;
                    *serialNumber = serNum;
                }
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

// Additional controller functions would follow the same pattern...
// GetTrackerPosition, SetTrackerPosition, GetChannel, SetChannel, etc.

#pragma mark - Cleanup

void CC3OSX_CleanupXPCConnections(void)
{
    [privateDeviceDBConnection invalidate];
    privateDeviceDBConnection = nil;
    
    [controllerConnections enumerateKeysAndObjectsUsingBlock:^(NSString *key, NSXPCConnection *conn, BOOL *stop) {
        [conn invalidate];
    }];
    [controllerConnections removeAllObjects];
}
