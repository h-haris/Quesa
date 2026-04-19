# Building Quesa with XPC Support

This guide covers multiple build methods for the XPC-enabled Quesa framework.

## Quick Start

```bash
# 1. Run configuration script
chmod +x configure_xpc_target.sh
./configure_xpc_target.sh

# 2. Follow Xcode instructions
cat XPCService/XCODE_TARGET_INSTRUCTIONS.txt

# 3. Build with Xcode or use build script
./build_xpc_framework.sh Debug
```

## Method 1: Xcode (Recommended)

### Step 1: Create XPC Service Target

1. Open your `.xcodeproj` file in Xcode
2. Select the project in Project Navigator
3. Click "+" at the bottom of the Targets list
4. Choose **macOS → XPC Service**
5. Configure:
   - **Product Name:** `Quesa3DDeviceServer`
   - **Bundle Identifier:** `com.quesa.osx.3ddevice.server.xpc`
   - **Language:** Objective-C

### Step 2: Configure XPC Service Target

**Build Settings:**
```
Product Bundle Identifier: com.quesa.osx.3ddevice.server.xpc
macOS Deployment Target: 10.15
Preprocessor Macros: QUESA_USE_XPC=1
Header Search Paths: $(SRCROOT) $(SRCROOT)/Include
Info.plist File: XPCService/Info.plist
```

**Add to Target Membership:**
- `XPCService/Q3DdbXPC.mm` ✓
- `XPCService/Q3DcontrollerXPC.mm` ✓
- `IPCprotocolXPC.h` ✓
- Required Quesa headers

### Step 3: Create XPC-Enabled Framework Target

**Option A: Duplicate Existing Framework Target**

1. Right-click "Quesa" framework target → **Duplicate**
2. Rename to `Quesa-XPC`
3. **Build Settings:**
   ```
   Product Name: Quesa-XPC
   Preprocessor Macros: $(inherited) QUESA_USE_XPC=1
   ```
4. **Compile Sources Phase:**
   - Remove: `ControllerCoreOSX.mm`
   - Add: `ControllerCoreOSX_XPC.mm`

**Option B: Add Configuration File**

1. In project settings → **Info** tab
2. Under **Configurations**, duplicate "Debug" → "Debug-XPC"
3. Duplicate "Release" → "Release-XPC"  
4. Set `Quesa-XPC.xcconfig` as configuration file for both
5. Build using these configurations

### Step 4: Configure Main Application

**For applications using Quesa:**

1. Select your app target
2. **Build Phases → Target Dependencies:**
   - Add `Quesa3DDeviceServer`
3. **Build Phases → New Copy Files Phase:**
   - Name: "Embed XPC Services"
   - Destination: **XPC Services**
   - Add: `Quesa3DDeviceServer.xpc`
   - ⚠️ Uncheck "Copy only when installing"

### Step 5: Build and Validate

```bash
# Build in Xcode (Cmd+B) or via command line:
xcodebuild -scheme YourApp -configuration Debug

# Validate XPC service was embedded
./validate_xpc_setup.sh build/Debug/YourApp.app
```

## Method 2: Command Line Build Script

```bash
# Build XPC-enabled framework
./build_xpc_framework.sh Debug

# Or with custom settings
./build_xpc_framework.sh Release macosx
```

The script accepts:
- Argument 1: Configuration (Debug/Release) - default: Debug
- Argument 2: Platform (macosx/iphoneos) - default: macosx

## Method 3: CMake (Alternative)

```bash
# Create build directory
mkdir build && cd build

# Configure with XPC enabled (default)
cmake .. -DQUESA_USE_XPC=ON

# Or disable XPC to use PDO
cmake .. -DQUESA_USE_XPC=OFF

# Build
cmake --build .

# Install
sudo cmake --install .
```

## Method 4: Manual xcodebuild

### Build XPC Service
```bash
xcodebuild \
    -target Quesa3DDeviceServer \
    -configuration Debug \
    -sdk macosx \
    clean build
```

### Build XPC Framework
```bash
xcodebuild \
    -target Quesa \
    -configuration Debug \
    -sdk macosx \
    GCC_PREPROCESSOR_DEFINITIONS='$(inherited) QUESA_USE_XPC=1' \
    PRODUCT_NAME=Quesa-XPC \
    clean build
```

## Verification

### Check XPC Service Bundle Structure

```bash
# After building your app:
tree YourApp.app/Contents/XPCServices/

# Should show:
# Quesa3DDeviceServer.xpc/
#   Contents/
#     Info.plist
#     MacOS/
#       Quesa3DDeviceServer
```

### Validate Bundle Identifier

```bash
/usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" \
    YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc/Contents/Info.plist

# Output: com.quesa.osx.3ddevice.server.xpc
```

### Check XPC Service Type

```bash
/usr/libexec/PlistBuddy -c "Print :XPCService:ServiceType" \
    YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc/Contents/Info.plist

# Output: Application
```

### Test XPC Service Launch

```bash
# Run your app, then check if XPC service is running:
launchctl list | grep quesa

# Check system logs:
log stream --predicate 'subsystem == "com.quesa.osx.3ddevice.server.xpc"' --level debug
```

## Debugging

### Debug XPC Service in Xcode

1. **Create Debug Scheme:**
   - Product → Scheme → Manage Schemes
   - Create new scheme for `Quesa3DDeviceServer`
   
2. **Edit Scheme:**
   - Run → Info → Executable: **Ask on Launch**
   
3. **Start Debugging:**
   - Run your main application
   - When XPC service launches, Xcode will ask to attach
   - Select the `Quesa3DDeviceServer` process
   - Breakpoints in XPC code will now work

### Debug with Console.app

1. Open **Console.app**
2. Filter by: `com.quesa` or `3ddevice`
3. Watch for XPC connection/error messages
4. Look for crash reports

### Debug with lldb

```bash
# Attach to running XPC service
sudo lldb -p $(pgrep Quesa3DDeviceServer)

# Or launch and attach
lldb YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc/Contents/MacOS/Quesa3DDeviceServer
```

## Common Build Issues

### Issue: "XPC service not found"
**Solution:**
- Verify "Embed XPC Services" copy phase is configured
- Check target dependency is set
- Clean build folder and rebuild

### Issue: Code signing errors
**Solution:**
```
# In XPC target Build Settings:
Code Signing Identity: Sign to Run Locally
Code Signing Style: Automatic
```

### Issue: Headers not found during XPC build
**Solution:**
```
# Add to Header Search Paths:
$(SRCROOT)
$(SRCROOT)/Include
$(SRCROOT)/Source
```

### Issue: Linking errors in XPC service
**Solution:**
- Ensure all required Quesa implementation files are included
- Check that `Q3DdbXPC.mm` properly includes dependencies
- Verify framework links (Foundation, CoreFoundation)

### Issue: Runtime crash in XPC service
**Solution:**
1. Check Console.app for crash logs
2. Verify all protocols match between client and service
3. Ensure bundle identifier is exactly: `com.quesa.osx.3ddevice.server.xpc`
4. Check Info.plist XPCService dictionary is correct

## Build Configurations

### Debug Build
```bash
# With symbols and logging
xcodebuild -configuration Debug GCC_PREPROCESSOR_DEFINITIONS='$(inherited) QUESA_USE_XPC=1 Q3_DEBUG=1'
```

### Release Build
```bash
# Optimized
xcodebuild -configuration Release GCC_PREPROCESSOR_DEFINITIONS='$(inherited) QUESA_USE_XPC=1'
```

### Dual Build (PDO + XPC)
```bash
# Build both versions
xcodebuild -target Quesa -configuration Debug    # PDO version
xcodebuild -target Quesa -configuration Debug GCC_PREPROCESSOR_DEFINITIONS='QUESA_USE_XPC=1' PRODUCT_NAME=Quesa-XPC  # XPC version
```

## Distribution

When distributing your app with XPC service:

1. **XPC service must be in app bundle:**
   ```
   YourApp.app/
     Contents/
       MacOS/
         YourApp
       XPCServices/
         Quesa3DDeviceServer.xpc/
   ```

2. **Code signing:**
   - Sign the XPC service first
   - Then sign the main app bundle
   ```bash
   codesign -s "Developer ID" YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc
   codesign -s "Developer ID" YourApp.app
   ```

3. **Notarization:**
   - Include XPC service in notarization
   - XPC service will be scanned as part of app bundle

## Performance Comparison

### XPC vs PDO Benchmarks

```bash
# Run performance tests
./test_ipc_performance.sh

# Expected results:
# PDO:  ~0.5ms per call
# XPC:  ~0.8ms per call (slightly higher overhead)
```

XPC has minimal performance impact for most use cases. The benefits (security, reliability, modern API) outweigh the small overhead.

## Next Steps

1. ✅ Create XPC service target in Xcode
2. ✅ Configure build settings
3. ✅ Add source files to target
4. ✅ Create XPC-enabled framework target
5. ✅ Build and test
6. ✅ Validate with validation script
7. ✅ Debug if needed
8. ✅ Deploy

## Resources

- [Apple XPC Documentation](https://developer.apple.com/library/archive/documentation/MacOSX/Conceptual/BPSystemStartup/Chapters/CreatingXPCServices.html)
- [Migration Guide](PDO_TO_XPC_MIGRATION.md)
- [XPC Service README](XPCService_README.md)
- [Troubleshooting Guide](TROUBLESHOOTING.md)

## Support

If you encounter issues:
1. Check the troubleshooting section above
2. Review Console.app logs
3. Run validation script
4. Open issue on GitHub with logs and error messages
