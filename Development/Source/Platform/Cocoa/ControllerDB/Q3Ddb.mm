/*  NAME:
 Q3Ddb.mm

 DESCRIPTION:
    QuesaOSXDeviceServer - acts as registrar of 3D Controller Devices.
    Every 3D Controller Device shall be registered here

    COPYRIGHT:
        Copyright (c) 2011-2021, Quesa Developers. All rights reserved.

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

#import "Q3Ddb.h"
#import "Q3DcontrollerPDO.h"
#import "IPCprotocolPDO.h"
#include "E3MacDeviceDbStart.h"

#if Q3_DEBUG
#import <Foundation/Foundation.h>
#endif

@implementation Q3Ddb : NSObject

- (id)init {
    if (self = [super init]) {
        //init my own stuff
        controllerListSerialNumber = 0;
    }
    return self;
}


- (void)dealloc {
    [super dealloc];
}

- (void)registerVendConnection
{
    theConnection = [[NSConnection new] autorelease];
    theConnection.rootObject = self;

    //make name of ControllerDB public
    [theConnection registerName:@kQuesa3DeviceServer];
    [theConnection retain];

    _controllerPDOs = [NSMutableArray arrayWithCapacity:2];
    [_controllerPDOs retain];

#if Q3_DEBUG
    NSLog(@"-registerVendConnection vended: %@\n",@kQuesa3DeviceServer);
#endif
}

//vend 3D-device database out of startup code
- (void)applicationDidFinishLaunching:(NSNotification *)notification /*delegation protocol*/
{
    [self registerVendConnection];
#if Q3_DEBUG
    NSLog(@"-applicationDidFinishLaunching called registerVendConnection\n");
#endif
}


- (void)incControllerListSerialNumber {
    controllerListSerialNumber++;
}


- (NSUInteger) dbIndexOfTrackerUUID:(NSString *) aTrackerUUID
{
    NSUInteger index = [_controllerPDOs indexOfObjectPassingTest:
                        ^ BOOL (Q3DcontrollerPDO *obj, NSUInteger idx, BOOL *stop)
                        {
                            return NSOrderedSame==[aTrackerUUID compare:[obj trackerUUID]];
                        }];
    return index;
};


- (NSUInteger) dbIndexOfSignature:(NSString *) aDriverSignature
{
    NSUInteger index = [_controllerPDOs indexOfObjectPassingTest:
                        ^ BOOL (Q3DcontrollerPDO *obj, NSUInteger idx, BOOL *stop)
                        {
                            return NSOrderedSame==[aDriverSignature compare:[obj signature]];
                        }];
    return index;
};


- (NSUInteger) dbIndexOfSControllerRef:(TQ3ControllerRef) aControllerRef
{
    NSUInteger index = [_controllerPDOs indexOfObjectPassingTest:
                        ^ BOOL (Q3DcontrollerPDO *obj, NSUInteger idx, BOOL *stop)
                        {
                            return (aControllerRef==[obj controllerRef]);
                        }];
    return index;
};


- (TQ3Boolean) isKnownSignature:(NSString *) aDriverSignature
{
    if (NSNotFound==[self dbIndexOfSignature:aDriverSignature])
        return kQ3False;
    else
        return kQ3True;
};


/* newCC3Controller: called when controller is created
 * allocate, init and vend Q3DcontrollerPDO object
 * return autogenerated server name (including UUID) as key
 */
//create new Controller in db; returns string with key (UUID)
//-----------------------------------------------------------------------------
// used on Server/Driver Side
//-----------------------------------------------------------------------------
- (out NSString *)reNewCC3ControllerWithUUID:(NSString *) aControllerUUID
                              ctrlDriverUUID:(NSString *) aControllerDriverUUID
                               controllerRef:(TQ3ControllerRefCast) aControllerRef
                                  valueCount:(TQ3Uns32) valCnt
                                channelCount:(TQ3Uns32) chanCnt
                                   signature:(NSString *) sig
                         hasSetChannelMethod:(TQ3Boolean) hasSCMthd
                         hasGetChannelMethod:(TQ3Boolean) hasGCMthd
{
    Q3DcontrollerPDO    *theControllerObject;

    NSUInteger          foundSignatureAt = [self dbIndexOfSignature:sig];

    if (foundSignatureAt < NSNotFound)
    {
        //fetch Controller at recently found index
        //and assign to theControllerObject
        theControllerObject = _controllerPDOs[foundSignatureAt];
    }
    else
    {
        theControllerObject = [[[Q3DcontrollerPDO alloc] initWithParametersDB:self
                                                               controllerUUID:aControllerUUID
                                                              driverStateUUID:aControllerDriverUUID
                                                                controllerRef:(TQ3ControllerRef)aControllerRef
                                                                   valueCount:valCnt
                                                                 channelCount:chanCnt
                                                                    signature:sig
                                                          hasSetChannelMethod:hasSCMthd
                                                          hasGetChannelMethod:hasGCMthd] autorelease];

        //insert at db list end. Peserve order of insertion!
        [_controllerPDOs addObject:theControllerObject];
    }

    [theControllerObject recommissionController];
    [theControllerObject setActivation:kQ3True];//controllerListSerialNumber++;

    return [theControllerObject UUID];
}


//next Controller in db; returns string with key (controller UUID); passing NULL returns first in list; returning NULL indicates end of list
- (out TQ3ControllerRefCast)nextCC3Controller: (in TQ3ControllerRefCast) currentControllerRef
{
    NSUInteger idx;

    if ((TQ3ControllerRefCast)NULL==currentControllerRef)
    {
        idx = 0;
    }
    else
    {
        idx = [self dbIndexOfSControllerRef:(TQ3ControllerRef)currentControllerRef];
        if (NSNotFound==idx)
            return NULL;
        ++idx;
    }

    if (idx>=_controllerPDOs.count)
        return NULL;

    return (TQ3ControllerRefCast)[_controllerPDOs[idx] UUID];
};


- (TQ3Status) getListChanged:(inout TQ3Boolean*)listChanged
                SerialNumber:(inout TQ3Uns32*)serNum
{
    TQ3Status status = kQ3Success;

    if (controllerListSerialNumber!=*serNum)
    {
        *listChanged=kQ3True;
        *serNum=controllerListSerialNumber;
    }
    else
    {
        *listChanged=kQ3False;
    }
    return(status);
}


- (TQ3Status) trackerDeleted:(NSString *) deletedTrackerUUID
{
    TQ3Status status = kQ3Failure;

    Q3DcontrollerPDO    *theControllerObject;

    NSUInteger foundOldTrackerAt = [self dbIndexOfTrackerUUID:deletedTrackerUUID];

    if (NSNotFound!=foundOldTrackerAt)
    {
        //fetch Controller at recently found index
        //and delete the references to its tracker object
        theControllerObject = _controllerPDOs[foundOldTrackerAt];
        status = [theControllerObject deleteTracker];
    }
    return status;
}

@end

#pragma mark -

Q3Ddb* Q3DDeviceDb;

void startDeviceDB(void)
{
    bool foundVendedDB = false;

    //check if a device server is allready vended
    id   privateProxyDB = nil;
    privateProxyDB = [NSConnection
                      rootProxyForConnectionWithRegisteredName:@kQuesa3DeviceServer
                      host:nil];

    //if not, instantiate DB
    foundVendedDB = (privateProxyDB != nil);
    if (!foundVendedDB)
    {
        Q3DDeviceDb = [[[Q3Ddb alloc] init] autorelease];
        [Q3DDeviceDb registerVendConnection];
    }
}

