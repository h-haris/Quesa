# XPC Integration Troubleshooting Guide

## Quick Diagnostics

Run these commands to quickly diagnose common issues:

```bash
# 1. Verify XPC service is embedded
ls -la YourApp.app/Contents/XPCServices/

# 2. Check bundle identifier
/usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" \
    YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc/Contents/Info.plist

# 3. Validate executable
file YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc/Contents/MacOS/Quesa3DDeviceServer

# 4. Check if service is running
launchctl list | grep quesa

# 5. View real-time logs
log stream --predicate 'subsystem contains "quesa"' --level debug
```

---

## Build Time Issues

### Problem: XPC service target not found in Xcode

**Symptoms:**
- Can't select XPC service target
- Build fails with "target not found"

**Solutions:**
1. Verify target was created:
   - Project Navigator → Select project → Targets list
   - Should see "Quesa3DDeviceServer"

2. If missing, recreate:
   ```
   1. File → New → Target
   2. macOS → XPC Service
   3. Name: Quesa3DDeviceServer
   ```

3. Check project file:
   ```bash
   # Backup first!
   grep -r "Quesa3DDeviceServer" YourProject.xcodeproj/
   ```

---

### Problem: Compiler can't find headers in XPC service

**Symptoms:**
```
'IPCprotocolXPC.h' file not found
'E3Prefix.h' file not found
```

**Solutions:**

1. **Add Header Search Paths:**
   - XPC target → Build Settings → Header Search Paths
   - Add:
     ```
     $(SRCROOT)
     $(SRCROOT)/Include
     $(SRCROOT)/Source
     ```

2. **Verify file locations:**
   ```bash
   find . -name "IPCprotocolXPC.h"
   find . -name "E3Prefix.h"
   ```

3. **Add files to target:**
   - Select header file → File Inspector → Target Membership
   - Check "Quesa3DDeviceServer"

---

### Problem: Linker errors in XPC service

**Symptoms:**
```
Undefined symbols for architecture x86_64:
  "_OBJC_CLASS_$_Q3Ddb", referenced from...
```

**Solutions:**

1. **Check source file compilation:**
   - XPC target → Build Phases → Compile Sources
   - Ensure `Q3DdbXPC.mm` and `Q3DcontrollerXPC.mm` are listed

2. **Verify file includes:**
   - Open `Q3DdbXPC.mm`
   - Check it includes `Q3DcontrollerXPC.mm`:
     ```objc
     #import "Q3DcontrollerXPC.mm"
     ```

3. **Add missing frameworks:**
   - XPC target → Build Phases → Link Binary With Libraries
   - Add: Foundation.framework, CoreFoundation.framework

---

### Problem: Code signing fails for XPC service

**Symptoms:**
```
Code Signing Error: No signing certificate found
```

**Solutions:**

1. **Set automatic signing:**
   - XPC target → Signing & Capabilities
   - Automatically manage signing: ☑️

2. **Or use local signing:**
   ```
   Build Settings:
   Code Sign Identity: Sign to Run Locally
   Code Sign Style: Automatic
   ```

3. **For distribution:**
   ```bash
   # Sign XPC service manually
   codesign -s "Developer ID Application: Your Name" \
       YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc
   ```

---

## Runtime Issues

### Problem: XPC service not launching

**Symptoms:**
- App runs but XPC calls fail
- Console shows: "XPC connection invalidated"
- `launchctl list | grep quesa` returns nothing

**Diagnosis:**
```bash
# Check if XPC service exists
ls YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc/Contents/MacOS/

# Try to launch manually (for testing)
YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc/Contents/MacOS/Quesa3DDeviceServer
```

**Solutions:**

1. **Verify Copy Files phase:**
   - Main app target → Build Phases → Embed XPC Services
   - Destination: XPC Services
   - Contains: Quesa3DDeviceServer.xpc
   - "Copy only when installing" should be UNCHECKED

2. **Check target dependency:**
   - Main app target → Build Phases → Target Dependencies
   - Should list: Quesa3DDeviceServer

3. **Rebuild with clean:**
   ```bash
   xcodebuild clean
   rm -rf ~/Library/Developer/Xcode/DerivedData/*
   xcodebuild -scheme YourApp -configuration Debug
   ```

---

### Problem: XPC connection immediately invalidates

**Symptoms:**
```
XPC connection invalidated
Error Domain=NSCocoaErrorDomain Code=4099
```

**Diagnosis:**
```bash
# Check Console.app for crash logs
open -a Console

# Filter by process
# Look for: Quesa3DDeviceServer crash reports
```

**Solutions:**

1. **Verify bundle identifier matches:**
   ```objc
   // Client code must use exact string:
   @kQuesa3DDeviceServerXPC  // "com.quesa.osx.3ddevice.server.xpc"
   ```

2. **Check Info.plist:**
   ```bash
   plutil -lint YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc/Contents/Info.plist
   ```
   
   Should contain:
   ```xml
   <key>XPCService</key>
   <dict>
       <key>ServiceType</key>
       <string>Application</string>
   </dict>
   ```

3. **Look for crashes:**
   ```bash
   # View crash logs
   ls ~/Library/Logs/DiagnosticReports/ | grep Quesa
   
   # Or use Console.app:
   # 1. Open Console.app
   # 2. Select "Crash Reports"
   # 3. Filter by "Quesa"
   ```

---

### Problem: XPC service crashes on launch

**Symptoms:**
- Service appears briefly then disappears
- Crash report in Console.app

**Diagnosis:**
```bash
# Get crash log
cat ~/Library/Logs/DiagnosticReports/Quesa3DDeviceServer*.crash

# Common crash locations:
# - Initialization
# - Missing frameworks
# - Undefined symbols
```

**Solutions:**

1. **Debug with lldb:**
   ```bash
   # Launch XPC service under debugger
   lldb YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc/Contents/MacOS/Quesa3DDeviceServer
   
   (lldb) run
   # Watch where it crashes
   ```

2. **Add crash logging:**
   ```objc
   // In Q3DdbXPC.mm main():
   int main(int argc, const char *argv[])
   {
       @autoreleasepool {
           NSLog(@"XPC Service starting...");
           Q3DdbXPC *service = [[Q3DdbXPC alloc] init];
           NSLog(@"XPC Service initialized");
           [service run];
       }
       return 0;
   }
   ```

3. **Check for missing dependencies:**
   ```bash
   # List dynamic libraries
   otool -L YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc/Contents/MacOS/Quesa3DDeviceServer
   
   # Verify all paths are valid
   ```

---

### Problem: Protocol method mismatch

**Symptoms:**
```
*** NSInvalidArgumentException: -[proxy methodName:reply:]: unrecognized selector
```

**Diagnosis:**
Protocol mismatch between client and service

**Solutions:**

1. **Verify protocol declarations match:**
   ```objc
   // Client side (ControllerCoreOSX_XPC.mm):
   [NSXPCInterface interfaceWithProtocol:@protocol(Q3XPCDeviceDB)]
   
   // Service side (Q3DdbXPC.mm):
   @interface Q3DdbXPC : NSObject <Q3XPCDeviceDB>
   ```

2. **Check method signatures:**
   - All methods must have `reply:` parameter
   - Reply block types must match exactly
   - Protocol must be in shared header (IPCprotocolXPC.h)

3. **Rebuild both targets:**
   ```bash
   # Clean and rebuild everything
   xcodebuild -target Quesa3DDeviceServer clean build
   xcodebuild -target YourApp clean build
   ```

---

### Problem: Semaphore deadlock

**Symptoms:**
- App hangs when calling XPC method
- No crash, just freeze

**Diagnosis:**
```bash
# Get sample of hanging process
sample YourApp 10
```

**Solutions:**

1. **Check timeout is reasonable:**
   ```objc
   // Don't use DISPATCH_TIME_FOREVER for production!
   dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC);
   if (dispatch_semaphore_wait(sema, timeout) != 0) {
       NSLog(@"XPC call timed out!");
       return kQ3Failure;
   }
   ```

2. **Verify reply is always called:**
   ```objc
   - (void)someMethodWithReply:(void (^)(TQ3Status))reply {
       // ALWAYS call reply, even on error!
       if (error) {
           reply(kQ3Failure);  // Don't forget this!
           return;
       }
       // ... do work
       reply(kQ3Success);
   }
   ```

3. **Use error handler:**
   ```objc
   [[connection remoteObjectProxyWithErrorHandler:^(NSError *error) {
       NSLog(@"XPC error: %@", error);
       status = kQ3Failure;
       dispatch_semaphore_signal(sema);
   }] someMethod:... reply:^(TQ3Status stat) {
       status = stat;
       dispatch_semaphore_signal(sema);
   }];
   ```

---

### Problem: Data corruption across XPC

**Symptoms:**
- Values come back incorrect
- Structs have garbage data

**Diagnosis:**
XPC serialization issue

**Solutions:**

1. **Pass structs by value, not pointer:**
   ```objc
   // WRONG:
   - (void)setPosition:(TQ3Point3D *)position reply:(void (^)(TQ3Status))reply;
   
   // CORRECT:
   - (void)setPosition:(TQ3Point3D)position reply:(void (^)(TQ3Status))reply;
   ```

2. **For complex types, use NSValue:**
   ```objc
   // Encode
   NSValue *value = [NSValue valueWithBytes:&position objCType:@encode(TQ3Point3D)];
   
   // Decode
   TQ3Point3D position;
   [value getValue:&position];
   ```

3. **Verify array encoding:**
   ```objc
   // Float arrays must be wrapped in NSArray of NSNumber
   NSMutableArray *array = [NSMutableArray array];
   for (int i = 0; i < count; i++) {
       [array addObject:@(values[i])];
   }
   ```

---

## Performance Issues

### Problem: XPC calls are slow

**Symptoms:**
- App feels sluggish
- High latency on device operations

**Diagnosis:**
```bash
# Profile XPC calls
instruments -t "Time Profiler" YourApp.app

# Check for excessive calls
log stream --predicate 'subsystem contains "quesa"' | grep -c "XPC"
```

**Solutions:**

1. **Batch operations:**
   ```objc
   // Instead of 100 individual calls:
   for (int i = 0; i < 100; i++) {
       [controller setValue:values[i] atIndex:i];
   }
   
   // Make one call:
   [controller setValues:arrayOfValues];
   ```

2. **Use synchronous proxy for repeated calls:**
   ```objc
   // Get synchronous proxy
   id proxy = [connection synchronousRemoteObjectProxyWithErrorHandler:...];
   
   // Multiple calls without semaphore overhead
   [proxy method1];
   [proxy method2];
   [proxy method3];
   ```

3. **Cache results:**
   ```objc
   // Don't query every frame
   static TQ3Uns32 cachedSerialNumber = 0;
   if (currentSerialNumber != cachedSerialNumber) {
       // Only update when changed
       [controller getValues:...];
       cachedSerialNumber = currentSerialNumber;
   }
   ```

---

## Security Issues

### Problem: Sandbox violations

**Symptoms:**
```
Sandbox: deny mach-lookup com.quesa.osx.3ddevice.server.xpc
```

**Solutions:**

1. **Add entitlements to XPC service:**
   ```xml
   <!-- Quesa3DDeviceServer.entitlements -->
   <key>com.apple.security.app-sandbox</key>
   <false/>
   ```

2. **For sandboxed apps, add exception:**
   ```xml
   <!-- Main app entitlements -->
   <key>com.apple.security.temporary-exception.mach-lookup.global-name</key>
   <array>
       <string>com.quesa.osx.3ddevice.server.xpc</string>
   </array>
   ```

---

## Diagnostic Tools

### Enable Detailed XPC Logging

```bash
# Set environment variable
export CFNETWORK_DIAGNOSTICS=3
export XPC_DEBUG=1

# Run app
./YourApp.app/Contents/MacOS/YourApp
```

### XPC Activity Monitor

```bash
# Watch XPC connections
lsmp -p $(pgrep YourApp)

# Show XPC services
launchctl list | grep quesa
```

### Console Filters

In Console.app, use these filters:
```
subsystem:com.quesa.osx.3ddevice.server.xpc
category:XPC
```

---

## Getting Help

If none of these solutions work:

1. **Collect diagnostic info:**
   ```bash
   # Create bug report bundle
   ./collect_diagnostics.sh > diagnostics.txt
   ```

2. **Include:**
   - macOS version
   - Xcode version
   - Full error messages from Console.app
   - Crash logs if any
   - Build log
   - Output of validation script

3. **File issue:**
   - GitHub: https://github.com/jwwalker/Quesa/issues
   - Include diagnostics.txt
   - Describe steps to reproduce

---

## Prevention Checklist

Before reporting an issue, verify:

- ✅ XPC service target exists
- ✅ Bundle identifier is exact: `com.quesa.osx.3ddevice.server.xpc`
- ✅ XPC service is embedded in app bundle
- ✅ Target dependency is set
- ✅ Protocol definitions match exactly
- ✅ All reply blocks are called
- ✅ Semaphores have timeouts
- ✅ Structs passed by value
- ✅ Clean build performed
- ✅ Console.app checked for errors
- ✅ Validation script ran successfully

---

*Last updated: April 19, 2026*
