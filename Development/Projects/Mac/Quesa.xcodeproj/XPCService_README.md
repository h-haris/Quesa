# Quesa 3D Device Server XPC Service

## Overview
This directory contains the XPC service implementation for the Quesa 3D device server.

## XPC Service Target Setup

### Creating the XPC Service Target in Xcode

1. **Open your Xcode project**
2. **Add New Target:**
   - Select your project in the Project Navigator
   - Click the "+" button at the bottom of the targets list
   - Choose "XPC Service" from macOS templates
   - Click "Next"

3. **Configure the target:**
   - **Product Name:** `Quesa3DDeviceServer`
   - **Organization Identifier:** `com.quesa.osx`
   - **Bundle Identifier:** `com.quesa.osx.3ddevice.server.xpc`
   - **Language:** Objective-C
   - Click "Finish"

4. **Add Source Files to Target:**
   - Select the following files and add them to the XPC service target:
     - `Q3DdbXPC.mm`
     - `Q3DcontrollerXPC.mm`
     - `IPCprotocolXPC.h`
     - Any required Quesa headers (E3Prefix.h, etc.)

5. **Update Info.plist:**
   - Replace the generated Info.plist with `Quesa3DDeviceServer-Info.plist`
   - Or manually update these keys:
     - `CFBundleIdentifier`: `com.quesa.osx.3ddevice.server.xpc`
     - `XPCService` → `ServiceType`: `Application`
     - `XPCService` → `RunLoopType`: `NSRunLoop`

6. **Build Settings:**
   - **Base SDK:** macOS
   - **Deployment Target:** macOS 10.15 or later
   - **Enable Objective-C ARC:** Yes (for modern code)
   - **C++ Language Dialect:** C++14 or later
   - **Preprocessor Macros:** Add `QUESA_USE_XPC=1`

7. **Link Frameworks:**
   - Foundation.framework (automatically linked)
   - Any other Quesa dependencies

8. **Build Phases:**
   - Ensure "Copy Files" phase exists with:
     - **Destination:** XPC Services
     - **Subpath:** (leave empty)

## File Structure

```
XPCService/
├── Q3DdbXPC.mm                    # Main XPC service implementation
├── Q3DcontrollerXPC.mm            # Controller XPC implementation
├── IPCprotocolXPC.h                # XPC protocol definitions
├── Quesa3DDeviceServer-Info.plist # Service configuration
└── main.mm (optional)              # If you need a separate main file
```

## Dependencies

The XPC service requires:
- Foundation framework
- Quesa type definitions (TQ3Status, TQ3Uns32, etc.)
- Core Quesa headers for types

## Testing

1. Build the XPC service target
2. Build your main application
3. Run the application - the XPC service should launch automatically
4. Check Console.app for XPC service logs
5. Use `launchctl list | grep quesa` to verify service is running

## Debugging

To debug the XPC service:
1. In Xcode, select the XPC service scheme
2. Product → Scheme → Edit Scheme
3. Run → Info → Executable: Ask on Launch
4. Run the main application
5. When prompted, select the XPC service process
6. Breakpoints in XPC service code will now work

## Notes

- The XPC service is automatically embedded in the main application bundle
- It launches on-demand when the application makes its first XPC connection
- The service terminates automatically when no connections remain
- Check Console.app for service crashes or errors
