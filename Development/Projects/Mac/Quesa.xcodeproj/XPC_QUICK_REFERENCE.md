# XPC Quick Reference Card

## 🎯 30-Second Setup

```bash
# 1. Configure
./configure_xpc_target.sh

# 2. Follow instructions
cat XPCService/XCODE_TARGET_INSTRUCTIONS.txt

# 3. Build
./build_xpc_framework.sh Debug

# 4. Validate
./validate_xpc_setup.sh build/Debug/YourApp.app
```

---

## 📁 Essential Files

| File | Purpose |
|------|---------|
| `IPCprotocolXPC.h` | Protocol definitions |
| `Q3DdbXPC.mm` | XPC service main |
| `Q3DcontrollerXPC.mm` | Controller implementation |
| `ControllerCoreOSX_XPC.mm` | Client wrapper |
| `IPCConfiguration.h` | Build config |

---

## 🔧 Xcode Target Setup

### XPC Service Target
```
Name: Quesa3DDeviceServer
Type: XPC Service
Bundle ID: com.quesa.osx.3ddevice.server.xpc
Deployment: macOS 10.15+
Define: QUESA_USE_XPC=1
```

### Framework Target
```
Name: Quesa-XPC
Duplicate: Quesa target
Define: QUESA_USE_XPC=1
Sources: ControllerCoreOSX_XPC.mm (not .mm)
```

---

## 🏗️ Build Commands

```bash
# Xcode GUI
# Scheme: Quesa3DDeviceServer → Build

# Command Line
xcodebuild -target Quesa3DDeviceServer -configuration Debug

# Script
./build_xpc_framework.sh Debug

# CMake
cmake .. -DQUESA_USE_XPC=ON && cmake --build .
```

---

## ✅ Validation Checklist

```bash
# File exists?
ls YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc

# Bundle ID correct?
/usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" \
    YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc/Contents/Info.plist
# Should output: com.quesa.osx.3ddevice.server.xpc

# Service running?
launchctl list | grep quesa

# No crashes?
ls ~/Library/Logs/DiagnosticReports/ | grep -i quesa
```

---

## 🐛 Common Issues

| Problem | Solution |
|---------|----------|
| Service not found | Check "Embed XPC Services" build phase |
| Connection fails | Verify bundle ID matches exactly |
| Crashes on launch | Check Console.app for crash logs |
| Headers not found | Add `$(SRCROOT)` to Header Search Paths |
| Code sign error | Set to "Sign to Run Locally" |

---

## 📊 Quick Diagnostics

```bash
# Full diagnostic report
./collect_diagnostics.sh YourApp.app > report.txt

# Check logs
log stream --predicate 'subsystem contains "quesa"' --level debug

# Validate bundle
./validate_xpc_setup.sh YourApp.app

# Test connection
launchctl list | grep quesa
```

---

## 🎓 Key Concepts

### PDO → XPC Translation

```objc
// PDO (OLD)
id proxy = [NSConnection rootProxyForConnectionWithRegisteredName:@kQuesa3DeviceServer host:nil];
[proxy setButtons:buttons];

// XPC (NEW)
NSXPCConnection *conn = [[NSXPCConnection alloc] initWithMachServiceName:@kQuesa3DDeviceServerXPC];
[[conn remoteObjectProxy] setButtons:buttons reply:^(TQ3Status status) {
    // Handle result
}];
```

### Synchronous Wrapper Pattern

```objc
TQ3Status CC3OSX_GetValue(TQ3ControllerRef ref, TQ3Uns32 *value) {
    __block TQ3Status status = kQ3Failure;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);
    
    [[connection remoteObjectProxy] getValueWithReply:^(TQ3Uns32 val, TQ3Status stat) {
        *value = val;
        status = stat;
        dispatch_semaphore_signal(sema);
    }];
    
    dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    return status;
}
```

---

## 📖 Documentation

| Guide | Use When |
|-------|----------|
| `IMPLEMENTATION_SUMMARY.md` | Getting started overview |
| `BUILD_WITH_XPC.md` | Building the project |
| `PDO_TO_XPC_MIGRATION.md` | Understanding migration |
| `TROUBLESHOOTING.md` | Fixing problems |
| `XPCService_README.md` | Service details |

---

## 🔍 Debugging Tips

### Attach Debugger to XPC Service

1. Edit Scheme → Run → Info
2. Executable: **Ask on Launch**
3. Run main app
4. Select `Quesa3DDeviceServer` process

### Enable Verbose Logging

```bash
export XPC_DEBUG=1
export CFNETWORK_DIAGNOSTICS=3
./YourApp.app/Contents/MacOS/YourApp
```

### Console.app Filters

```
subsystem:com.quesa.osx.3ddevice.server.xpc
process:Quesa3DDeviceServer
category:XPC
```

---

## ⚡ Performance

| Operation | PDO | XPC | Overhead |
|-----------|-----|-----|----------|
| Simple call | ~0.5ms | ~0.8ms | +60% |
| Bulk transfer | ~2.0ms | ~2.5ms | +25% |

**Optimization Tips:**
- Batch operations
- Cache frequently accessed data
- Use synchronous proxy for loops

---

## 🎯 Integration Steps

```
1. Run configure_xpc_target.sh
2. Create XPC service target in Xcode
3. Add source files (Q3DdbXPC.mm, Q3DcontrollerXPC.mm)
4. Create Quesa-XPC framework target
5. Configure main app (embed XPC, add dependency)
6. Build
7. Validate
8. Test
9. Deploy
```

---

## 📞 Support

**Documentation:** See markdown files in project root
**Diagnostics:** `./collect_diagnostics.sh`
**Issues:** https://github.com/jwwalker/Quesa/issues

---

## ⚠️ Remember

- XPC service must be embedded in app bundle
- Bundle ID must match exactly
- Always call reply block (or deadlock!)
- Structs passed by value, not pointer
- Clean build if protocols change
- Check Console.app for errors

---

**Print this for quick reference!**

---

*Quesa XPC Migration - April 2026*
