# In-Process XPC Implementation (MachXPC Style)

## Overview

This document describes how to use **in-process XPC** using anonymous listeners, eliminating the need for an external XPC service. This approach is inspired by the MachXPC library pattern.

## ✅ Benefits of In-Process XPC

1. **No External Service Required** - No need for separate XPC service bundle
2. **No Code Signing Issues** - Eliminates bundle identifier and entitlement problems
3. **Simpler Deployment** - Everything runs within your main app process
4. **Easier Debugging** - Debug all code in one process
5. **Protocol Benefits Retained** - Still get type-safe XPC protocol communication
6. **Cleaner Architecture** - Clear separation of concerns without IPC overhead

## 🏗️ Architecture

### Before (External XPC Service):
```
Main App Process                    XPC Service Process
┌──────────────────────┐           ┌─────────────────────┐
│ ControllerCoreOSX_XPC├──────────►│ Q3DdbXPC            │
│                      │   Mach    │  (Separate Process) │
│ NSXPCConnection      │   IPC     │ NSXPCListener       │
└──────────────────────┘           └─────────────────────┘
```

### After (In-Process XPC):
```
Main App Process Only
┌──────────────────────────────────────────────┐
│ ControllerCoreOSX_XPC ──┐                    │
│                         │                    │
│                         ▼                    │
│ NSXPCConnection ──► Q3DdbXPC                 │
│    (anonymous       (in-process)             │
│     endpoint)       NSXPCListener            │
│                     (anonymous)              │
└──────────────────────────────────────────────┘
```

## 🔑 Key Implementation Details

### 1. Anonymous Listeners

Instead of using a Mach service name:
```objc
// OLD (External Service):
NSXPCConnection *conn = [[NSXPCConnection alloc] 
    initWithMachServiceName:@"com.quesa.osx.3ddevice.server.xpc"];

// NEW (In-Process):
NSXPCListener *listener = [NSXPCListener anonymousListener];
listener.delegate = self;
[listener resume];

NSXPCConnection *conn = [[NSXPCConnection alloc]
    initWithListenerEndpoint:listener.endpoint];
```

### 2. Device Database Initialization

The `Q3DdbXPC` class now supports two modes:

```objc
// External XPC Service mode
- (instancetype)init {
    _listener = [[NSXPCListener alloc] 
        initWithMachServiceName:@kQuesa3DDeviceServerXPC];
    // ...
}

// In-Process mode (MachXPC-style)
- (instancetype)initForInProcess {
    _listener = nil;  // Caller provides anonymous listener
    // ...
}
```

### 3. Connection Management

In `ControllerCoreOSX_XPC.mm`:

```objc
static Q3DdbXPC *sharedDeviceDB = nil;
static NSXPCListener *deviceDBListener = nil;

static void initializeInProcessDeviceDB(void)
{
    sharedDeviceDB = [[Q3DdbXPC alloc] initForInProcess];
    
    // Anonymous listener - no external service!
    deviceDBListener = [NSXPCListener anonymousListener];
    deviceDBListener.delegate = sharedDeviceDB;
    [deviceDBListener resume];
}
```

### 4. NSXPCListenerDelegate Implementation

Both `Q3DdbXPC` and `Q3DcontrollerXPC` implement `NSXPCListenerDelegate`:

```objc
- (BOOL)listener:(NSXPCListener *)listener 
    shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    newConnection.exportedInterface = 
        [NSXPCInterface interfaceWithProtocol:@protocol(Q3XPCDeviceDB)];
    newConnection.exportedObject = self;
    [newConnection resume];
    return YES;
}
```

## 📝 Modified Files

### New Files:
- ✅ **Q3DdbXPC.h** - Header exposing in-process initialization

### Modified Files:
- ✅ **ControllerCoreOSX_XPC.mm** - Uses anonymous listeners instead of Mach services
- ✅ **Q3DdbXPC.mm** - Added `initForInProcess` method
- ✅ **Q3DcontrollerXPC.h** - Added NSXPCListenerDelegate conformance
- ✅ **Q3DcontrollerXPC.mm** - Implemented delegate method

## 🚀 Usage

### Initialization

The in-process XPC system initializes automatically on first use:

```objc
TQ3ControllerRef controller = CC3OSXController_New(&controllerData);
// On first call, initializeInProcessDeviceDB() runs automatically
```

### Cleanup

```objc
static void CC3OSX_CleanupXPCConnections(void)
{
    [controllerConnections enumerateKeysAndObjectsUsingBlock:^(...) {
        [conn invalidate];
    }];
    
    [deviceDBListener invalidate];
    deviceDBListener = nil;
    
    sharedDeviceDB = nil;
}
```

## ⚙️ Build Configuration

### No XPC Service Target Needed!

Simply ensure `QUESA_USE_XPC=1` is defined:

```bash
# In Xcode Build Settings:
Preprocessor Macros = QUESA_USE_XPC=1
```

### Removed Build Dependencies

You can now **remove**:
- ❌ XPC Service target from Xcode
- ❌ "Embed XPC Services" build phase
- ❌ Info.plist for XPC service
- ❌ Code signing configuration for XPC service
- ❌ Mach service name registration

## 🐛 Debugging

### Single-Process Debugging

Since everything runs in one process:

```objc
// Set breakpoints anywhere:
- (void)getButtonsWithReply:(void (^)(TQ3Uns32, TQ3Status))reply {
    // Breakpoint here works directly!
    reply(_theButtons, kQ3Success);
}
```

### Console Logging

```objc
#if Q3_DEBUG
    NSLog(@"✅ Initialized in-process device DB (MachXPC-style)");
#endif
```

## 🔍 Comparison: External vs In-Process

| Feature | External XPC Service | In-Process XPC |
|---------|---------------------|----------------|
| **Bundle Structure** | Separate .xpc bundle | Single app bundle |
| **Code Signing** | Complex (two bundles) | Simple (one bundle) |
| **Debugging** | Multi-process attach | Single process |
| **Deployment** | Embed service in app | Just the app |
| **Performance** | IPC overhead | Direct calls |
| **Process Isolation** | Separate processes | Same process |
| **Protocol Safety** | ✅ Yes | ✅ Yes |
| **Async Calls** | ✅ Yes | ✅ Yes |
| **Setup Complexity** | High | Low |

## 🎯 When to Use Each Approach

### Use **In-Process XPC** (MachXPC-style) when:
- ✅ You want protocol-based architecture
- ✅ Code signing is causing issues
- ✅ Debugging complexity is a concern
- ✅ You don't need process isolation
- ✅ Simpler deployment is preferred

### Use **External XPC Service** when:
- ✅ Process isolation is required for security
- ✅ Sandboxing different privilege levels
- ✅ Service needs to run independently
- ✅ Service shared by multiple apps

## 💡 Migration from External to In-Process

If you're currently using an external XPC service:

1. **Keep Protocol Definitions** - No changes to `IPCprotocolXPC.h`
2. **Update Initialization** - Use `initForInProcess` instead of `init`
3. **Use Anonymous Listeners** - Replace Mach service connections
4. **Remove XPC Service Target** - Clean up Xcode project
5. **Test Thoroughly** - Same API, different implementation

## 🔐 Security Considerations

### In-Process Limitations

⚠️ **Important**: In-process XPC does NOT provide:
- Process isolation
- Sandboxing enforcement
- Privilege separation
- Protection from crashes in one component

If your 3D device drivers are unstable or potentially malicious, stick with external XPC service.

### When In-Process is Safe

✅ Use in-process XPC when:
- You control all the code
- Drivers are well-tested and stable
- Security boundary is the app itself
- Simpler architecture outweighs isolation benefits

## 📚 Additional Resources

### MachXPC Inspiration
This implementation is inspired by projects that use XPC for internal architecture:
- Anonymous listeners for modular design
- Protocol-based boundaries
- Type-safe communication

### Apple Documentation
- NSXPCListener Class Reference
- NSXPCConnection Programming Guide
- "Daemons and Services Programming Guide"

## ✅ Verification Checklist

After implementing in-process XPC:

- [ ] No external XPC service bundle required
- [ ] Code signing succeeds with single bundle
- [ ] All controller operations work
- [ ] Debugging works in single process
- [ ] No errors in Console.app
- [ ] Performance is acceptable
- [ ] Memory management is correct (no leaks)
- [ ] Cleanup properly invalidates connections

## 🎉 Summary

**You've successfully eliminated the external XPC service requirement!**

Your code now:
- ✅ Uses protocol-based XPC architecture
- ✅ Runs entirely in-process
- ✅ Avoids code signing complexity
- ✅ Maintains clean separation of concerns
- ✅ Is easier to debug and deploy

This is the **MachXPC approach** - getting XPC benefits without external service overhead.

---

*Implementation completed: April 19, 2026*
*Based on MachXPC pattern: Anonymous XPC listeners for in-process communication*
