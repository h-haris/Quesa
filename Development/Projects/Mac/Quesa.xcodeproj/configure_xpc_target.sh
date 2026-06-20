#!/bin/bash
#
# configure_xpc_target.sh
#
# Script to help configure XPC service target in Xcode project
# This script provides the necessary build settings and configuration
#

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="${SCRIPT_DIR}/.."

echo "=========================================="
echo "Quesa XPC Service Configuration Helper"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "${PROJECT_DIR}/IPCprotocolXPC.h" ]; then
    print_error "Cannot find IPCprotocolXPC.h. Please run this script from the project root."
    exit 1
fi

print_info "Checking required files..."

# List of required files
REQUIRED_FILES=(
    "IPCprotocolXPC.h"
    "Q3DdbXPC.mm"
    "Q3DcontrollerXPC.mm"
    "Quesa3DDeviceServer-Info.plist"
    "IPCConfiguration.h"
    "ControllerCoreOSX_XPC.mm"
)

MISSING_FILES=0
for file in "${REQUIRED_FILES[@]}"; do
    if [ -f "${PROJECT_DIR}/${file}" ]; then
        print_info "✓ Found: ${file}"
    else
        print_error "✗ Missing: ${file}"
        MISSING_FILES=$((MISSING_FILES + 1))
    fi
done

echo ""

if [ $MISSING_FILES -gt 0 ]; then
    print_error "Missing ${MISSING_FILES} required file(s). Please ensure all XPC migration files are present."
    exit 1
fi

print_info "All required files found!"
echo ""

# Create XPC service directory structure
XPC_SERVICE_DIR="${PROJECT_DIR}/XPCService"
print_info "Creating XPC service directory structure..."

mkdir -p "${XPC_SERVICE_DIR}"

# Copy XPC service files
print_info "Organizing XPC service files..."
cp "${PROJECT_DIR}/Q3DdbXPC.mm" "${XPC_SERVICE_DIR}/"
cp "${PROJECT_DIR}/Q3DcontrollerXPC.mm" "${XPC_SERVICE_DIR}/"
cp "${PROJECT_DIR}/Quesa3DDeviceServer-Info.plist" "${XPC_SERVICE_DIR}/Info.plist"

print_info "✓ XPC service files organized in ${XPC_SERVICE_DIR}"
echo ""

# Generate build settings file
print_info "Generating build settings configuration..."

cat > "${XPC_SERVICE_DIR}/BuildSettings.xcconfig" << 'EOF'
// Quesa 3D Device Server XPC Service Build Settings
// Auto-generated configuration file

// Product Settings
PRODUCT_NAME = Quesa3DDeviceServer
PRODUCT_BUNDLE_IDENTIFIER = com.quesa.osx.3ddevice.server.xpc
INFOPLIST_FILE = $(SRCROOT)/XPCService/Info.plist

// Deployment
MACOSX_DEPLOYMENT_TARGET = 10.15
SDKROOT = macosx

// Language Settings
CLANG_ENABLE_OBJC_ARC = YES
CLANG_CXX_LANGUAGE_STANDARD = c++14
CLANG_CXX_LIBRARY = libc++

// Preprocessor Macros
GCC_PREPROCESSOR_DEFINITIONS = $(inherited) QUESA_USE_XPC=1

// Build Options
ONLY_ACTIVE_ARCH = NO
SKIP_INSTALL = NO

// Search Paths
HEADER_SEARCH_PATHS = $(inherited) $(SRCROOT) $(SRCROOT)/Include

// Warnings
GCC_WARN_ABOUT_RETURN_TYPE = YES_ERROR
GCC_WARN_UNDECLARED_SELECTOR = YES
CLANG_WARN_UNREACHABLE_CODE = YES
CLANG_WARN__DUPLICATE_METHOD_MATCH = YES
CLANG_WARN_OBJC_LITERAL_CONVERSION = YES

// Code Signing (adjust as needed)
CODE_SIGN_IDENTITY = -
CODE_SIGN_STYLE = Automatic
EOF

print_info "✓ Created BuildSettings.xcconfig"
echo ""

# Generate scheme for XPC service
print_info "XPC Service scheme configuration:"
cat > "${XPC_SERVICE_DIR}/README_SCHEME.md" << 'EOF'
# XPC Service Scheme Setup

After creating the XPC service target in Xcode, create a scheme:

1. **Product → Scheme → Manage Schemes**
2. **Click "+" to add new scheme**
3. **Name:** Quesa3DDeviceServer
4. **Target:** Quesa3DDeviceServer
5. **Check "Shared"** (optional, for team)

## Debugging Configuration

Edit the scheme (Product → Scheme → Edit Scheme):

### Run Tab:
- **Build Configuration:** Debug
- **Executable:** Ask on Launch
- **Debug Process As:** root (only if needed for device access)

### Build Tab:
Ensure these are checked for "Run":
- ✓ Quesa3DDeviceServer
- ✓ Your main application target

### Info Tab:
- **Launch:** Wait for executable to be launched

This allows you to debug the XPC service when it's launched by your main app.
EOF

print_info "✓ Created scheme configuration guide"
echo ""

# Create framework target configuration
print_info "Creating XPC-enabled framework target configuration..."

cat > "${PROJECT_DIR}/Quesa-XPC.xcconfig" << 'EOF'
// Quesa Framework with XPC Enabled
// Build configuration for XPC-enabled version of Quesa framework

// Enable XPC
GCC_PREPROCESSOR_DEFINITIONS = $(inherited) QUESA_USE_XPC=1

// Product Name Override
PRODUCT_NAME = Quesa-XPC

// Ensure we link the XPC version of controller core
// Add to OTHER_CFLAGS or source file configuration
EOF

print_info "✓ Created Quesa-XPC.xcconfig"
echo ""

# Create aggregate target script
cat > "${PROJECT_DIR}/build_xpc_framework.sh" << 'EOF'
#!/bin/bash
#
# build_xpc_framework.sh
#
# Builds the XPC-enabled version of Quesa framework
#

set -e

echo "Building Quesa Framework with XPC Support..."

# Set XPC flag
export QUESA_USE_XPC=1

# Build configuration
CONFIGURATION="${1:-Debug}"
PLATFORM="${2:-macosx}"

echo "Configuration: ${CONFIGURATION}"
echo "Platform: ${PLATFORM}"

# Build the framework
xcodebuild -target Quesa \
    -configuration "${CONFIGURATION}" \
    -sdk "${PLATFORM}" \
    GCC_PREPROCESSOR_DEFINITIONS='$(inherited) QUESA_USE_XPC=1' \
    PRODUCT_NAME=Quesa-XPC \
    build

echo "✓ Build complete!"
echo "Framework: build/${CONFIGURATION}/Quesa-XPC.framework"
EOF

chmod +x "${PROJECT_DIR}/build_xpc_framework.sh"

print_info "✓ Created build_xpc_framework.sh"
echo ""

# Create validation script
cat > "${PROJECT_DIR}/validate_xpc_setup.sh" << 'EOF'
#!/bin/bash
#
# validate_xpc_setup.sh
#
# Validates XPC service installation and configuration
#

set -e

echo "=========================================="
echo "XPC Service Validation"
echo "=========================================="
echo ""

APP_PATH="${1}"

if [ -z "$APP_PATH" ]; then
    echo "Usage: $0 <path-to-app.app>"
    echo "Example: $0 build/Debug/YourApp.app"
    exit 1
fi

if [ ! -d "$APP_PATH" ]; then
    echo "Error: Application not found at: $APP_PATH"
    exit 1
fi

echo "Checking: $APP_PATH"
echo ""

# Check for XPC service
XPC_SERVICE_PATH="${APP_PATH}/Contents/XPCServices/Quesa3DDeviceServer.xpc"

if [ -d "$XPC_SERVICE_PATH" ]; then
    echo "✓ XPC service found at: $XPC_SERVICE_PATH"
else
    echo "✗ XPC service NOT found at: $XPC_SERVICE_PATH"
    exit 1
fi

# Check Info.plist
PLIST="${XPC_SERVICE_PATH}/Contents/Info.plist"
if [ -f "$PLIST" ]; then
    echo "✓ Info.plist found"
    
    # Validate bundle identifier
    BUNDLE_ID=$(/usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" "$PLIST" 2>/dev/null || echo "NOT_FOUND")
    if [ "$BUNDLE_ID" = "com.quesa.osx.3ddevice.server.xpc" ]; then
        echo "✓ Bundle identifier correct: $BUNDLE_ID"
    else
        echo "✗ Bundle identifier incorrect: $BUNDLE_ID"
        echo "  Expected: com.quesa.osx.3ddevice.server.xpc"
    fi
    
    # Check XPC service type
    SERVICE_TYPE=$(/usr/libexec/PlistBuddy -c "Print :XPCService:ServiceType" "$PLIST" 2>/dev/null || echo "NOT_FOUND")
    if [ "$SERVICE_TYPE" = "Application" ]; then
        echo "✓ Service type correct: $SERVICE_TYPE"
    else
        echo "✗ Service type incorrect: $SERVICE_TYPE"
    fi
else
    echo "✗ Info.plist NOT found"
    exit 1
fi

# Check executable
EXECUTABLE="${XPC_SERVICE_PATH}/Contents/MacOS/Quesa3DDeviceServer"
if [ -f "$EXECUTABLE" ]; then
    echo "✓ Executable found"
    
    # Check if executable is valid
    if file "$EXECUTABLE" | grep -q "Mach-O"; then
        echo "✓ Executable is valid Mach-O binary"
    else
        echo "✗ Executable is not a valid Mach-O binary"
    fi
else
    echo "✗ Executable NOT found at: $EXECUTABLE"
    exit 1
fi

echo ""
echo "=========================================="
echo "Validation Complete!"
echo "=========================================="
EOF

chmod +x "${PROJECT_DIR}/validate_xpc_setup.sh"

print_info "✓ Created validate_xpc_setup.sh"
echo ""

# Create Xcode project snippet for manual addition
cat > "${XPC_SERVICE_DIR}/XCODE_TARGET_INSTRUCTIONS.txt" << 'EOF'
========================================
MANUAL XCODE TARGET SETUP INSTRUCTIONS
========================================

Since we cannot automatically modify .xcodeproj files, follow these steps:

1. ADD XPC SERVICE TARGET
   ========================
   - Open your .xcodeproj in Xcode
   - Select the project in Project Navigator
   - Click "+" at bottom of Targets list
   - Choose: macOS > XPC Service
   - Click Next
   - Product Name: Quesa3DDeviceServer
   - Organization Identifier: com.quesa.osx
   - Bundle Identifier: com.quesa.osx.3ddevice.server.xpc
   - Language: Objective-C
   - Click Finish

2. CONFIGURE XPC SERVICE TARGET
   ==============================
   - Select "Quesa3DDeviceServer" target
   - Build Settings tab
   - Set these values:
     * Product Bundle Identifier: com.quesa.osx.3ddevice.server.xpc
     * macOS Deployment Target: 10.15
     * Preprocessor Macros: Add "QUESA_USE_XPC=1"
     * Header Search Paths: Add "$(SRCROOT)" and "$(SRCROOT)/Include"

3. ADD SOURCE FILES TO TARGET
   ============================
   Select these files and check "Quesa3DDeviceServer" in Target Membership:
   - XPCService/Q3DdbXPC.mm
   - XPCService/Q3DcontrollerXPC.mm
   - IPCprotocolXPC.h
   - Required Quesa headers (E3Prefix.h, etc.)

4. UPDATE INFO.PLIST
   ==================
   - Replace target's Info.plist with: XPCService/Info.plist
   - OR in Build Settings, set Info.plist File to: XPCService/Info.plist

5. CREATE QUESA-XPC FRAMEWORK TARGET (Optional)
   ============================================
   Option A: Duplicate existing Quesa target
   -----------------------------------------
   - Right-click Quesa framework target
   - Select "Duplicate"
   - Rename to "Quesa-XPC"
   - In Build Settings:
     * Product Name: Quesa-XPC
     * Preprocessor Macros: Add "QUESA_USE_XPC=1"
   - In Compile Sources build phase:
     * Remove: ControllerCoreOSX.mm
     * Add: ControllerCoreOSX_XPC.mm

   Option B: Add build configuration
   ----------------------------------
   - Project settings > Info tab
   - Under Configurations, add "Debug-XPC" and "Release-XPC"
   - Set Quesa-XPC.xcconfig as configuration file
   - Build using these configurations to get XPC version

6. ADD EMBED XPC SERVICE PHASE
   ============================
   - Select your main application target
   - Build Phases tab
   - Click "+" > New Copy Files Phase
   - Name it "Embed XPC Services"
   - Destination: XPC Services
   - Click "+" and add "Quesa3DDeviceServer.xpc"
   - Ensure "Copy only when installing" is UNCHECKED

7. SET UP DEPENDENCIES
   ====================
   - Select main app target
   - Build Phases > Target Dependencies
   - Click "+" and add "Quesa3DDeviceServer"
   - This ensures XPC service builds before the app

8. CREATE SCHEMES
   ==============
   For XPC Service debugging:
   - Product > Scheme > Manage Schemes
   - Click "+" to create new scheme
   - Name: Quesa3DDeviceServer (Debug)
   - Target: Quesa3DDeviceServer
   - Edit Scheme > Run > Info tab
     * Executable: Ask on Launch
   
   For XPC-enabled framework:
   - Create scheme for Quesa-XPC target
   - Or use build_xpc_framework.sh script

9. TEST THE SETUP
   ===============
   Terminal commands:
   
   # Build everything
   xcodebuild -scheme YourMainApp -configuration Debug
   
   # Validate XPC service installation
   ./validate_xpc_setup.sh build/Debug/YourApp.app
   
   # Build XPC-enabled framework
   ./build_xpc_framework.sh Debug

10. VERIFY IN XCODE
    ================
    - Build your main application
    - Check build log for "Quesa3DDeviceServer.xpc"
    - Product > Show Build Folder in Finder
    - Navigate to: YourApp.app/Contents/XPCServices/
    - Verify Quesa3DDeviceServer.xpc exists

TROUBLESHOOTING
===============

XPC Service not found:
- Check "Embed XPC Services" copy phase is configured
- Verify target dependency is set
- Clean build folder (Cmd+Shift+K) and rebuild

Code signing issues:
- In XPC target Build Settings:
  * Code Signing Identity: Sign to Run Locally
  * Code Signing Style: Automatic

Headers not found:
- Add to Header Search Paths:
  * $(SRCROOT)
  * $(SRCROOT)/Include
  * $(SRCROOT)/Source

Link errors:
- Ensure all required Quesa source files are in target
- Check that Q3DdbXPC.mm includes Q3DcontrollerXPC.mm correctly

Runtime errors:
- Check Console.app for crash logs
- Look for XPC service logs (filter by "Quesa")
- Ensure bundle identifier matches exactly
- Verify Info.plist has XPCService dictionary

========================================
EOF

print_info "✓ Created Xcode target instructions"
echo ""

# Summary
echo ""
echo "=========================================="
echo "Configuration Complete!"
echo "=========================================="
echo ""
print_info "Created files:"
echo "  - ${XPC_SERVICE_DIR}/"
echo "    ├── Q3DdbXPC.mm"
echo "    ├── Q3DcontrollerXPC.mm"
echo "    ├── Info.plist"
echo "    ├── BuildSettings.xcconfig"
echo "    ├── README_SCHEME.md"
echo "    └── XCODE_TARGET_INSTRUCTIONS.txt"
echo ""
echo "  - Quesa-XPC.xcconfig"
echo "  - build_xpc_framework.sh"
echo "  - validate_xpc_setup.sh"
echo ""

# Offer to install Xcode templates
echo ""
echo "=========================================="
print_info "Xcode Project Templates Available"
echo "=========================================="
echo ""
echo "Templates can speed up Xcode configuration:"
echo "  • Pre-configured XPC service target"
echo "  • Pre-configured framework target"
echo "  • Saves 15+ minutes of manual setup"
echo ""
read -p "Install Xcode templates now? (y/n) " -n 1 -r
echo
TEMPLATE_INSTALLED=0
if [[ $REPLY =~ ^[Yy]$ ]]; then
    if [ -f "${PROJECT_DIR}/install_templates.sh" ]; then
        print_info "Installing templates..."
        "${PROJECT_DIR}/install_templates.sh"
        TEMPLATE_INSTALLED=1
    else
        print_warning "Template installer not found"
        echo "Run ./install_templates.sh manually later"
    fi
else
    print_info "Skipped template installation"
    echo "You can install them later with: ./install_templates.sh"
fi

echo ""
echo "=========================================="
print_info "Next steps:"
echo "=========================================="
echo ""
if [ $TEMPLATE_INSTALLED -eq 1 ]; then
    echo "  1. Restart Xcode"
    echo "  2. File → New → Target → Quesa category"
    echo "  3. Select 'Quesa 3D Device Server XPC Service'"
    echo "  4. Add source files to target"
    echo "  5. Build and test"
else
    echo "  1. Read: ${XPC_SERVICE_DIR}/XCODE_TARGET_INSTRUCTIONS.txt"
    echo "  2. Open your Xcode project"
    echo "  3. Follow the instructions to create XPC service target"
    echo "  4. Build and test"
fi
echo ""
print_info "Documentation:"
echo "  - Quick start: README_XPC_MIGRATION.md"
echo "  - Templates: XPCServiceTemplate/README.md"
echo "  - Build guide: BUILD_WITH_XPC.md"
echo ""
if [ $TEMPLATE_INSTALLED -eq 0 ]; then
    print_warning "Manual Xcode configuration required!"
    echo "  The .xcodeproj file must be modified in Xcode."
    echo "  OR install templates to speed up the process."
fi
echo ""
echo "=========================================="
