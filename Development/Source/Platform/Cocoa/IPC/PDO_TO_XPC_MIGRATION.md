# PDO to XPC Migration Guide

## Overview

This document describes the migration from PDO (Portable Distributed Objects) to XPC (Cross-Process Communication) in the Quesa 3D device framework.

## Why Migrate?

- **PDO is deprecated**: NSConnection and NSDistantObject have been deprecated by Apple
- **Modern macOS compatibility**: XPC is the recommended IPC mechanism for macOS 10.7+
- **Better security**: XPC provides sandboxing and privilege separation
- **More reliable**: Better error handling and connection management

## Architecture Changes

### Before (PDO):
```
Client App → NSConnection → Device Server (NSConnection vends objects)
                           ↓
                      Controllers/Trackers (vended as distributed objects)
```

### After (XPC):
```
Client App → NSXPCConnection → XPC Service (Mach service)
                              ↓
                         Device DB (exports Q3XPCDeviceDB protocol)
                              ↓
                         Controllers/Trackers (via anonymous endpoints)
```

## Key Differences

### 1. Object References
**PDO**: Objects could be passed by reference (pointers)
**XPC**: Must use serializable data (NSString UUIDs instead of TQ3ControllerRef pointers)

### 2. Synchronous vs Asynchronous
**PDO**: Synchronous calls were natural
**XPC**: All calls are asynchronous with reply blocks

### 3. Connection Management
**PDO**: `NSConnection` with registered names
**XPC**: `NSXPCConnection` with Mach service names or listener endpoints

## Migration Steps

### Step 1: Add XPC Service Target to Xcode Project

1. In Xcode, select File → New → Target
2. Choose "XPC Service" template
3. Name it "Quesa3DDeviceServer"
4. Set Product Bundle Identifier to `com.quesa.osx.3ddevice.server.xpc`

### Step 2: Configure XPC Service

1. Copy `Q3DdbXPC.mm` to the XPC service target
2. Copy `Q3DcontrollerXPC.mm` to the XPC service target
3. Copy `IPCprotocolXPC.h` to shared location
4. Use the provided `Quesa3DDeviceServer-Info.plist`

### Step 3: Update Main Application

1. Add `IPCConfiguration.h` to your project
2. Add `ControllerCoreOSX_XPC.mm` to your project
3. Update include statements to use `IPCConfiguration.h`
4. Link against XPC framework

### Step 4: Conditional Compilation

Use the `QUESA_USE_XPC` macro to maintain both paths during transition:

```objc
#if QUESA_USE_XPC
    #include "ControllerCoreOSX_XPC.mm"
#else
    #include "ControllerCoreOSX.mm"  // PDO version
#endif
```

### Step 5: Testing

1. Build the XPC service target
2. Build the main application
3. Set `QUESA_USE_XPC=1` in build settings
4. Test all controller/tracker functionality

## Code Changes Required

### In Client Code

**Before (PDO):**
```objc
id proxyDB = [NSConnection rootProxyForConnectionWithRegisteredName:@kQuesa3DeviceServer host:nil];
[proxyDB setProtocolForProxy:@protocol(Q3DODeviceDB)];
NSString *uuid = [proxyDB reNewCC3Controller:...];
```

**After (XPC):**
```objc
NSXPCConnection *conn = [[NSXPCConnection alloc] 
    initWithMachServiceName:@kQuesa3DDeviceServerXPC];
conn.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(Q3XPCDeviceDB)];
[conn resume];

[[conn remoteObjectProxy] reNewCC3Controller:... reply:^(NSString *uuid, TQ3Status status) {
    // Handle result
}];
```

### In Protocol Definitions

**Before (PDO):**
```objc
@protocol Q3DOController
- (TQ3Status) getButtons:(inout TQ3Uns32 *) buttons;
@end
```

**After (XPC):**
```objc
@protocol Q3XPCController
- (void)getButtonsWithReply:(void (^)(TQ3Uns32 buttons, TQ3Status status))reply;
@end
```

## Handling Asynchronous Calls

For APIs that must remain synchronous, use semaphores:

```objc
TQ3Status CC3OSXController_GetButtons(TQ3ControllerRef controllerRef, TQ3Uns32 *buttons)
{
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);
    
    NSXPCConnection *connection = connectionForController(controllerUUID);
    
    [[connection remoteObjectProxy] getButtonsWithReply:^(TQ3Uns32 btns, TQ3Status stat) {
        *buttons = btns;
        status = stat;
        dispatch_semaphore_signal(sema);
    }];
    
    dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    return status;
}
```

## Service Identifier Changes

All service identifiers have been updated from `3device` to `3ddevice`:

- `com.quesa.osx.3device.server` → `com.quesa.osx.3ddevice.server.xpc`
- `com.quesa.osx.3device.controller` → `com.quesa.osx.3ddevice.controller.xpc`
- `com.quesa.osx.3device.tracker` → `com.quesa.osx.3ddevice.tracker.xpc`

## Files Created/Modified

### New Files:
- ✅ `IPCprotocolXPC.h` - XPC protocol definitions
- ✅ `Q3DcontrollerXPC.mm` - XPC controller implementation
- ✅ `Q3DdbXPC.mm` - XPC database service
- ✅ `ControllerCoreOSX_XPC.mm` - XPC client wrapper
- ✅ `IPCConfiguration.h` - Build configuration
- ✅ `Quesa3DDeviceServer-Info.plist` - XPC service plist

### Modified Files:
- ✅ `IPCprotocolPDO.h` - Added deprecation notice

### Unchanged Files (for reference):
- `ControllerCoreOSX.mm` - PDO implementation (deprecated)
- `Q3DcontrollerPDO.mm` - PDO implementation (deprecated)
- `Q3Ddb.mm` - PDO implementation (deprecated)

## Troubleshooting

### XPC Service Not Found
- Ensure XPC service is embedded in main app bundle under `Contents/XPCServices/`
- Check that bundle identifier matches the Mach service name
- Verify Info.plist has correct XPCService dictionary

### Connection Interrupted
- Check for crashes in XPC service (use Console.app)
- Verify protocol methods match between client and service
- Ensure proper error handling in invalidation handlers

### Serialization Errors
- XPC only supports property list types, NSSecureCoding, or XPC objects
- Custom structs (TQ3Point3D, etc.) must be passed by value, not reference
- Use NSValue if needed for complex types

## Performance Considerations

- XPC has slightly more overhead than PDO
- Connection pooling can help with frequent operations
- Consider batching operations where possible
- Use `remoteObjectProxyWithErrorHandler:` for better error handling

## Security Benefits

- XPC services run in separate processes
- Can apply different entitlements and sandbox rules
- Better isolation between client and device drivers
- Audit trail via system logs

## Deployment

1. **Development**: Use conditional compilation to test both paths
2. **Beta**: Enable XPC for beta testers (macOS 10.15+)
3. **Release**: Set `QUESA_USE_XPC=1` as default
4. **Future**: Remove PDO code entirely

## Timeline

- **Phase 1** (Current): Implement XPC alongside PDO
- **Phase 2**: Test XPC implementation thoroughly
- **Phase 3**: Make XPC the default
- **Phase 4**: Remove PDO code

## Additional Resources

### Quick Start
- **`XPC_QUICK_REFERENCE.md`** - Quick reference card (print this!)
- **`IMPLEMENTATION_SUMMARY.md`** - Complete overview of what was created
- **`configure_xpc_target.sh`** - Automated setup script

### Build & Test
- **`BUILD_WITH_XPC.md`** - Comprehensive build guide
- **`build_xpc_framework.sh`** - Command-line build script
- **`validate_xpc_setup.sh`** - Installation validator
- **`CMakeLists.txt`** - CMake build support

### Debugging
- **`TROUBLESHOOTING.md`** - Complete troubleshooting guide
- **`collect_diagnostics.sh`** - Diagnostic information collector
- **`XPCService_README.md`** - XPC service details

### Integration
- **`XPCService/XCODE_TARGET_INSTRUCTIONS.txt`** - Step-by-step Xcode setup
- **`XPCService/BuildSettings.xcconfig`** - XPC service build settings
- **`Quesa-XPC.xcconfig`** - Framework build configuration

## Support

For questions or issues:
- GitHub: https://github.com/jwwalker/Quesa
- 3D Device Support: https://github.com/h-haris/Quesa

**Before reporting issues:**
1. Read TROUBLESHOOTING.md
2. Run `./collect_diagnostics.sh YourApp.app`
3. Check Console.app for errors
4. Include diagnostic output in issue report

---

*Last Updated: April 19, 2026*
