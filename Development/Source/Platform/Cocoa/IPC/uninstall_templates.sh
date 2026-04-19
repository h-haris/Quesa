#!/bin/bash
#
# uninstall_templates.sh
#
# Removes Quesa XPC Xcode project templates
#

set -e

XCODE_TEMPLATES_DIR="$HOME/Library/Developer/Xcode/Templates/Project Templates/Quesa"

echo "=========================================="
echo "Quesa XPC Template Uninstaller"
echo "=========================================="
echo ""

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

# Check if templates are installed
if [ ! -d "$XCODE_TEMPLATES_DIR" ]; then
    print_warning "Templates not found - nothing to uninstall"
    echo "Location checked: $XCODE_TEMPLATES_DIR"
    exit 0
fi

echo "Found templates at:"
echo "  $XCODE_TEMPLATES_DIR"
echo ""
ls -la "$XCODE_TEMPLATES_DIR"
echo ""

read -p "Remove these templates? (y/n) " -n 1 -r
echo

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Uninstallation cancelled."
    exit 0
fi

# Remove templates
rm -rf "$XCODE_TEMPLATES_DIR"
print_success "Templates removed"

# Check for running Xcode
echo ""
if pgrep -x "Xcode" > /dev/null; then
    print_warning "Xcode is currently running"
    echo ""
    echo "Restart Xcode to complete uninstallation."
    echo ""
    read -p "Quit Xcode now? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        osascript -e 'quit app "Xcode"'
        print_success "Xcode quit successfully"
    fi
fi

echo ""
echo "=========================================="
echo "Uninstallation Complete!"
echo "=========================================="
echo ""
