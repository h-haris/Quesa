# XPC Integration Checklist

Use this checklist to track your progress integrating XPC into your project.

---

## Phase 1: Initial Setup ✓

### Files Created
- [ ] `IPCprotocolXPC.h` - XPC protocol definitions
- [ ] `Q3DdbXPC.mm` - XPC database service implementation
- [ ] `Q3DcontrollerXPC.mm` - XPC controller implementation
- [ ] `ControllerCoreOSX_XPC.mm` - Client-side wrapper
- [ ] `IPCConfiguration.h` - Build configuration header
- [ ] `Quesa3DDeviceServer-Info.plist` - XPC service Info.plist
- [ ] `Quesa-XPC.xcconfig` - Framework build configuration

### Scripts Created
- [ ] `configure_xpc_target.sh` - Setup automation
- [ ] `build_xpc_framework.sh` - Build script
- [ ] `validate_xpc_setup.sh` - Validation script
- [ ] `collect_diagnostics.sh` - Diagnostic collector

### Documentation Created
- [ ] `PDO_TO_XPC_MIGRATION.md` - Migration guide
- [ ] `BUILD_WITH_XPC.md` - Build instructions
- [ ] `TROUBLESHOOTING.md` - Troubleshooting guide
- [ ] `XPCService_README.md` - XPC service documentation
- [ ] `IMPLEMENTATION_SUMMARY.md` - Overview
- [ ] `XPC_QUICK_REFERENCE.md` - Quick reference

### Configuration Run
- [ ] Made scripts executable: `chmod +x *.sh`
- [ ] Ran `./configure_xpc_target.sh`
- [ ] Verified `XPCService/` directory created
- [ ] Read `XPCService/XCODE_TARGET_INSTRUCTIONS.txt`

---

## Phase 2: Xcode Configuration

### XPC Service Target
- [ ] Created new XPC Service target
  - [ ] Name: `Quesa3DDeviceServer`
  - [ ] Type: XPC Service (macOS)
  - [ ] Language: Objective-C
  - [ ] Bundle ID: `com.quesa.osx.3ddevice.server.xpc`

- [ ] Configured Build Settings
  - [ ] Product Bundle Identifier: `com.quesa.osx.3ddevice.server.xpc`
  - [ ] macOS Deployment Target: 10.15 or later
  - [ ] Info.plist File: `XPCService/Info.plist`
  - [ ] Header Search Paths: `$(SRCROOT)` and `$(SRCROOT)/Include`
  - [ ] Preprocessor Macros: `QUESA_USE_XPC=1`

- [ ] Added Source Files to Target
  - [ ] `XPCService/Q3DdbXPC.mm`
  - [ ] `XPCService/Q3DcontrollerXPC.mm`
  - [ ] `IPCprotocolXPC.h`
  - [ ] Required Quesa headers (E3Prefix.h, etc.)

- [ ] Verified Info.plist
  - [ ] XPCService → ServiceType: `Application`
  - [ ] XPCService → RunLoopType: `NSRunLoop`
  - [ ] Bundle identifier matches

- [ ] Linked Frameworks
  - [ ] Foundation.framework
  - [ ] CoreFoundation.framework

### XPC-Enabled Framework Target
- [ ] Created Quesa-XPC framework target
  - **Option A:** Duplicated existing Quesa target
    - [ ] Renamed to `Quesa-XPC`
    - [ ] Set Product Name: `Quesa-XPC`
  - **Option B:** Added build configuration
    - [ ] Created Debug-XPC configuration
    - [ ] Created Release-XPC configuration
    - [ ] Applied `Quesa-XPC.xcconfig`

- [ ] Configured Build Settings
  - [ ] Preprocessor Macros: `QUESA_USE_XPC=1`
  - [ ] Product Name: `Quesa-XPC`

- [ ] Updated Compile Sources
  - [ ] Removed: `ControllerCoreOSX.mm` (PDO version)
  - [ ] Added: `ControllerCoreOSX_XPC.mm` (XPC version)

### Main Application Target
- [ ] Added Target Dependency
  - [ ] Build Phases → Target Dependencies
  - [ ] Added `Quesa3DDeviceServer`

- [ ] Added Copy Files Phase
  - [ ] Created "Embed XPC Services" phase
  - [ ] Destination: XPC Services
  - [ ] Added `Quesa3DDeviceServer.xpc`
  - [ ] **Unchecked** "Copy only when installing"

- [ ] Updated Framework Link
  - [ ] Link against `Quesa-XPC.framework` (or set QUESA_USE_XPC=1)

---

## Phase 3: Building

### Initial Build
- [ ] Selected Quesa3DDeviceServer scheme
- [ ] Built XPC service (Cmd+B)
  - [ ] Build succeeded
  - [ ] No warnings or errors

- [ ] Selected Quesa-XPC scheme (or main app)
- [ ] Built framework/app (Cmd+B)
  - [ ] Build succeeded
  - [ ] XPC service embedded in bundle

### Command-Line Build (Optional)
- [ ] Ran `./build_xpc_framework.sh Debug`
- [ ] Build completed successfully
- [ ] Output: `build/Debug/Quesa-XPC.framework`

### CMake Build (Optional)
- [ ] Created build directory: `mkdir build && cd build`
- [ ] Configured: `cmake .. -DQUESA_USE_XPC=ON`
- [ ] Built: `cmake --build .`
- [ ] Build succeeded

---

## Phase 4: Validation

### Automated Validation
- [ ] Ran validation script:
  ```bash
  ./validate_xpc_setup.sh build/Debug/YourApp.app
  ```
- [ ] All checks passed:
  - [ ] ✓ XPC service bundle found
  - [ ] ✓ Info.plist valid
  - [ ] ✓ Bundle identifier correct
  - [ ] ✓ Service type correct
  - [ ] ✓ Executable found
  - [ ] ✓ Executable is valid Mach-O

### Manual Validation
- [ ] Verified bundle structure:
  ```
  YourApp.app/
    Contents/
      XPCServices/
        Quesa3DDeviceServer.xpc/
          Contents/
            Info.plist
            MacOS/
              Quesa3DDeviceServer
  ```

- [ ] Checked bundle identifier:
  ```bash
  /usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" \
      YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc/Contents/Info.plist
  # Output: com.quesa.osx.3ddevice.server.xpc
  ```

- [ ] Verified executable type:
  ```bash
  file YourApp.app/Contents/XPCServices/Quesa3DDeviceServer.xpc/Contents/MacOS/Quesa3DDeviceServer
  # Output: Mach-O 64-bit executable
  ```

---

## Phase 5: Testing

### Basic Functionality
- [ ] Launched application
- [ ] No immediate crashes
- [ ] XPC service launched (check with `launchctl list | grep quesa`)

### Controller Operations
- [ ] Controller enumeration works
- [ ] Controller activation works
- [ ] Button state reads correctly
- [ ] Values read/write correctly
- [ ] Tracker operations work
- [ ] Channel set/get works
- [ ] Controller state save/restore works

### Error Handling
- [ ] Connection interruption handled gracefully
- [ ] Invalid controller UUID handled
- [ ] Timeout scenarios work
- [ ] Error messages are clear

### Performance
- [ ] No noticeable lag
- [ ] Controller response time acceptable
- [ ] No memory leaks (check Instruments)
- [ ] CPU usage reasonable

---

## Phase 6: Debugging

### Console Monitoring
- [ ] Opened Console.app
- [ ] Filtered by "quesa" or "3ddevice"
- [ ] No error messages
- [ ] No crash reports

### XPC Service Debugging
- [ ] Configured debug scheme
  - [ ] Edit Scheme → Run → Executable: Ask on Launch
- [ ] Set breakpoints in XPC service code
- [ ] Launched app
- [ ] Attached to XPC service process
- [ ] Breakpoints hit correctly

### Diagnostic Collection
- [ ] Ran diagnostic script:
  ```bash
  ./collect_diagnostics.sh YourApp.app > diagnostics.txt
  ```
- [ ] Reviewed output
- [ ] All sections show expected values

---

## Phase 7: Comparison Testing (If Migrating from PDO)

### Dual Build
- [ ] Built PDO version (QUESA_USE_XPC=0)
- [ ] Built XPC version (QUESA_USE_XPC=1)
- [ ] Both build successfully

### Functionality Comparison
- [ ] PDO version works as before
- [ ] XPC version has same functionality
- [ ] No regressions in XPC version

### Performance Comparison
- [ ] Measured PDO call latency
- [ ] Measured XPC call latency
- [ ] Overhead is acceptable (< 1ms difference)

---

## Phase 8: Documentation

### Code Documentation
- [ ] Added comments to XPC-specific code
- [ ] Documented any project-specific changes
- [ ] Updated README if needed

### Internal Documentation
- [ ] Documented build procedure
- [ ] Created troubleshooting notes for team
- [ ] Updated deployment docs

---

## Phase 9: Deployment Preparation

### Code Signing
- [ ] XPC service signs correctly
- [ ] Main app signs with embedded service
- [ ] No signing warnings

### Packaging
- [ ] Created distribution build
- [ ] Verified XPC service included
- [ ] Archive validates correctly

### Testing on Clean System
- [ ] Tested on machine without Xcode
- [ ] XPC service launches correctly
- [ ] All functionality works

---

## Phase 10: Rollout

### Beta Testing
- [ ] Deployed to beta testers
- [ ] Collected feedback
- [ ] Fixed any issues found

### Production Release
- [ ] Updated version number
- [ ] Created release notes mentioning XPC
- [ ] Deployed to production
- [ ] Monitored for issues

### Post-Release
- [ ] No XPC-related crash reports
- [ ] Performance is good
- [ ] User feedback positive

---

## Phase 11: Cleanup (After XPC Proven Stable)

### PDO Code Removal
- [ ] Marked PDO code as deprecated
- [ ] Added warnings to PDO headers
- [ ] Set removal date

### Future Cleanup
- [ ] Remove conditional compilation (`#if QUESA_USE_XPC`)
- [ ] Delete PDO files:
  - [ ] `ControllerCoreOSX.mm`
  - [ ] `Q3DcontrollerPDO.mm`
  - [ ] `Q3Ddb.mm` (PDO version)
  - [ ] `IPCprotocolPDO.h`
- [ ] Update documentation
- [ ] Rename `Quesa-XPC` to `Quesa`

---

## Troubleshooting Checklist

If something doesn't work:

- [ ] Checked TROUBLESHOOTING.md
- [ ] Ran collect_diagnostics.sh
- [ ] Checked Console.app for errors
- [ ] Verified bundle structure
- [ ] Confirmed bundle identifier matches
- [ ] Checked for crash logs
- [ ] Verified all build phases configured
- [ ] Cleaned build folder and rebuilt
- [ ] Reviewed Xcode build log
- [ ] Asked for help (with diagnostics output)

---

## Success Criteria

✅ **Integration is successful when:**

- [ ] XPC service builds without errors
- [ ] XPC service embedded in app bundle
- [ ] App launches without crashes
- [ ] XPC service process appears in Activity Monitor
- [ ] All controller operations work
- [ ] No errors in Console.app
- [ ] Performance is acceptable
- [ ] Validation script passes
- [ ] Beta testers approve
- [ ] Ready for production

---

## Notes

Use this space for project-specific notes:

```
Date Started: __________________
Target Completion: __________________
Current Phase: __________________

Issues Encountered:
- 
- 
- 

Lessons Learned:
- 
- 
- 

Next Steps:
- 
- 
- 
```

---

## Sign-off

- [ ] **Developer:** Code complete, builds successfully
  - Name: __________________ Date: __________

- [ ] **QA:** Testing complete, all tests pass
  - Name: __________________ Date: __________

- [ ] **Tech Lead:** Code reviewed, approved for merge
  - Name: __________________ Date: __________

- [ ] **Release Manager:** Approved for deployment
  - Name: __________________ Date: __________

---

**Print this checklist and track your progress!**

---

*Quesa XPC Integration Checklist - April 2026*
