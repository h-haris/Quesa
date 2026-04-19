#!/bin/bash
#
# install_templates.sh
#
# Installs Quesa XPC Xcode project templates
#

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TEMPLATES_DIR="${SCRIPT_DIR}/XPCServiceTemplate"
XCODE_TEMPLATES_DIR="$HOME/Library/Developer/Xcode/Templates/Project Templates/Quesa"

echo "=========================================="
echo "Quesa XPC Template Installer"
echo "=========================================="
echo ""

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

# Check if templates directory exists
if [ ! -d "$TEMPLATES_DIR" ]; then
    print_error "Templates directory not found at: $TEMPLATES_DIR"
    echo "Please run this script from the project root directory."
    exit 1
fi

print_success "Found templates directory"

# Check if Xcode is installed
if ! command -v xcodebuild &> /dev/null; then
    print_error "Xcode is not installed or not in PATH"
    exit 1
fi

XCODE_VERSION=$(xcodebuild -version | head -1)
print_success "Found $XCODE_VERSION"

# Create Xcode templates directory
echo ""
echo "Creating Xcode templates directory..."
mkdir -p "$XCODE_TEMPLATES_DIR"
print_success "Created: $XCODE_TEMPLATES_DIR"

# Check if templates already exist
if [ -d "$XCODE_TEMPLATES_DIR/XPCServiceTemplate" ]; then
    print_warning "Templates already installed"
    echo ""
    read -p "Overwrite existing templates? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Installation cancelled."
        exit 0
    fi
    rm -rf "$XCODE_TEMPLATES_DIR/XPCServiceTemplate"
    rm -rf "$XCODE_TEMPLATES_DIR/FrameworkTemplate"
    print_success "Removed old templates"
fi

# Install XPC Service Template
echo ""
echo "Installing XPC Service Template..."
cp -R "$TEMPLATES_DIR" "$XCODE_TEMPLATES_DIR/"
print_success "Installed XPC Service Template"

# Create Framework Template
echo ""
echo "Creating Framework Template..."
FRAMEWORK_TEMPLATE_DIR="$XCODE_TEMPLATES_DIR/FrameworkTemplate"
mkdir -p "$FRAMEWORK_TEMPLATE_DIR"

# Create TemplateInfo.plist for framework
cat > "$FRAMEWORK_TEMPLATE_DIR/TemplateInfo.plist" << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>Identifier</key>
	<string>com.quesa.xpcframework</string>
	<key>Kind</key>
	<string>Xcode.Xcode3.ProjectTemplateUnitKind</string>
	<key>Ancestors</key>
	<array>
		<string>com.apple.dt.unit.macFramework</string>
	</array>
	<key>Concrete</key>
	<true/>
	<key>Description</key>
	<string>Creates a framework target for Quesa with XPC support enabled.</string>
	<key>Options</key>
	<array>
		<dict>
			<key>Identifier</key>
			<string>productName</string>
			<key>Required</key>
			<true/>
			<key>Name</key>
			<string>Product Name:</string>
			<key>Type</key>
			<string>text</string>
			<key>Default</key>
			<string>Quesa-XPC</string>
		</dict>
	</array>
	<key>Project</key>
	<dict>
		<key>SharedSettings</key>
		<dict>
			<key>PRODUCT_NAME</key>
			<string>$(TARGET_NAME)</string>
			<key>MACOSX_DEPLOYMENT_TARGET</key>
			<string>10.15</string>
			<key>GCC_PREPROCESSOR_DEFINITIONS</key>
			<array>
				<string>$(inherited)</string>
				<string>QUESA_USE_XPC=1</string>
			</array>
			<key>CLANG_ENABLE_OBJC_ARC</key>
			<string>YES</string>
			<key>CLANG_CXX_LANGUAGE_STANDARD</key>
			<string>c++14</string>
			<key>HEADER_SEARCH_PATHS</key>
			<array>
				<string>$(inherited)</string>
				<string>$(SRCROOT)</string>
				<string>$(SRCROOT)/Include</string>
			</array>
		</dict>
	</dict>
</dict>
</plist>
EOF

print_success "Installed Framework Template"

# Verify installation
echo ""
echo "Verifying installation..."

if [ -d "$XCODE_TEMPLATES_DIR/XPCServiceTemplate" ]; then
    print_success "XPC Service Template installed"
else
    print_error "XPC Service Template installation failed"
    exit 1
fi

if [ -d "$XCODE_TEMPLATES_DIR/FrameworkTemplate" ]; then
    print_success "Framework Template installed"
else
    print_error "Framework Template installation failed"
    exit 1
fi

# Check for running Xcode instances
echo ""
if pgrep -x "Xcode" > /dev/null; then
    print_warning "Xcode is currently running"
    echo ""
    echo "You must restart Xcode for templates to appear."
    echo ""
    read -p "Quit Xcode now? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        osascript -e 'quit app "Xcode"'
        print_success "Xcode quit successfully"
        echo ""
        echo "You can now restart Xcode to use the templates."
    else
        echo ""
        echo "Please restart Xcode manually to use the templates."
    fi
else
    print_success "Ready to use (start Xcode)"
fi

echo ""
echo "=========================================="
echo "Installation Complete!"
echo "=========================================="
echo ""
echo "Templates installed to:"
echo "  $XCODE_TEMPLATES_DIR"
echo ""
echo "To use the templates:"
echo "  1. (Re)start Xcode"
echo "  2. File → New → Target"
echo "  3. Scroll to bottom and select 'Quesa' category"
echo "  4. Choose 'Quesa 3D Device Server XPC Service'"
echo "     or 'Quesa XPC Framework'"
echo ""
echo "See XPCServiceTemplate/README.md for usage guide"
echo ""
