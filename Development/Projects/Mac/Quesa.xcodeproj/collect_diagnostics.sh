#!/bin/bash
#
# collect_diagnostics.sh
#
# Collects diagnostic information for XPC setup troubleshooting
#

set -e

APP_PATH="${1}"

echo "=========================================="
echo "Quesa XPC Diagnostics Collection"
echo "=========================================="
echo ""
echo "Date: $(date)"
echo "macOS Version: $(sw_vers -productVersion)"
echo "Xcode Version: $(xcodebuild -version | head -1)"
echo ""

if [ -z "$APP_PATH" ]; then
    echo "WARNING: No app path provided"
    echo "Usage: $0 <path-to-app>"
    echo ""
    echo "Collecting project-level diagnostics only..."
    echo ""
else
    echo "Application: $APP_PATH"
    echo ""
fi

echo "=========================================="
echo "1. File Structure"
echo "=========================================="
echo ""

echo "Required XPC files:"
for file in IPCprotocolXPC.h Q3DdbXPC.mm Q3DcontrollerXPC.mm Quesa3DDeviceServer-Info.plist; do
    if [ -f "$file" ]; then
        echo "  ✓ $file ($(wc -l < "$file") lines)"
    else
        echo "  ✗ $file MISSING"
    fi
done

echo ""
echo "XPCService directory:"
if [ -d "XPCService" ]; then
    ls -la XPCService/
else
    echo "  ✗ XPCService directory not found"
fi

echo ""

if [ -n "$APP_PATH" ] && [ -d "$APP_PATH" ]; then
    echo "=========================================="
    echo "2. App Bundle Structure"
    echo "=========================================="
    echo ""
    
    XPC_SERVICE_PATH="${APP_PATH}/Contents/XPCServices/Quesa3DDeviceServer.xpc"
    
    if [ -d "$XPC_SERVICE_PATH" ]; then
        echo "✓ XPC service bundle found"
        echo ""
        echo "Bundle contents:"
        tree "$XPC_SERVICE_PATH" || find "$XPC_SERVICE_PATH" -type f
        echo ""
    else
        echo "✗ XPC service bundle NOT FOUND at:"
        echo "  $XPC_SERVICE_PATH"
        echo ""
        echo "Available XPC services:"
        ls -la "${APP_PATH}/Contents/XPCServices/" 2>/dev/null || echo "  No XPCServices directory"
        echo ""
    fi
    
    echo "=========================================="
    echo "3. Info.plist Validation"
    echo "=========================================="
    echo ""
    
    if [ -f "${XPC_SERVICE_PATH}/Contents/Info.plist" ]; then
        PLIST="${XPC_SERVICE_PATH}/Contents/Info.plist"
        
        echo "Bundle Identifier:"
        /usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" "$PLIST" 2>/dev/null || echo "  ERROR: Could not read"
        
        echo ""
        echo "XPC Service Type:"
        /usr/libexec/PlistBuddy -c "Print :XPCService:ServiceType" "$PLIST" 2>/dev/null || echo "  ERROR: Could not read"
        
        echo ""
        echo "Full plist contents:"
        plutil -p "$PLIST" || cat "$PLIST"
        echo ""
    else
        echo "✗ Info.plist not found"
        echo ""
    fi
    
    echo "=========================================="
    echo "4. Executable Validation"
    echo "=========================================="
    echo ""
    
    EXECUTABLE="${XPC_SERVICE_PATH}/Contents/MacOS/Quesa3DDeviceServer"
    
    if [ -f "$EXECUTABLE" ]; then
        echo "✓ Executable found"
        echo ""
        
        echo "File type:"
        file "$EXECUTABLE"
        echo ""
        
        echo "Architecture:"
        lipo -info "$EXECUTABLE" 2>/dev/null || echo "  Not a Mach-O file"
        echo ""
        
        echo "Linked libraries:"
        otool -L "$EXECUTABLE" 2>/dev/null || echo "  Could not read"
        echo ""
        
        echo "Code signature:"
        codesign -dv "$EXECUTABLE" 2>&1 || echo "  Not signed or invalid signature"
        echo ""
    else
        echo "✗ Executable not found at:"
        echo "  $EXECUTABLE"
        echo ""
    fi
fi

echo "=========================================="
echo "5. Build Configuration"
echo "=========================================="
echo ""

if [ -f "XPCService/BuildSettings.xcconfig" ]; then
    echo "XPC Build Settings:"
    cat XPCService/BuildSettings.xcconfig
    echo ""
else
    echo "BuildSettings.xcconfig not found"
    echo ""
fi

if [ -f "Quesa-XPC.xcconfig" ]; then
    echo "Framework XPC Configuration:"
    cat Quesa-XPC.xcconfig
    echo ""
else
    echo "Quesa-XPC.xcconfig not found"
    echo ""
fi

echo "=========================================="
echo "6. Runtime Status"
echo "=========================================="
echo ""

echo "Running XPC services:"
launchctl list | grep -i quesa || echo "  No Quesa XPC services running"
echo ""

echo "Recent crash reports:"
ls -lt ~/Library/Logs/DiagnosticReports/ | grep -i quesa | head -5 || echo "  No crash reports found"
echo ""

echo "=========================================="
echo "7. System Logs (last 50 lines)"
echo "=========================================="
echo ""

echo "XPC-related logs:"
log show --predicate 'subsystem contains "quesa" OR processImagePath contains "Quesa"' \
    --last 1h --info --debug 2>/dev/null | tail -50 || echo "  No recent logs (requires macOS 10.12+)"
echo ""

echo "=========================================="
echo "8. Protocol Validation"
echo "=========================================="
echo ""

if [ -f "IPCprotocolXPC.h" ]; then
    echo "Protocol definitions:"
    grep -A 2 "@protocol" IPCprotocolXPC.h || echo "  Could not extract protocols"
    echo ""
    
    echo "Method count per protocol:"
    for protocol in Q3XPCController Q3XPCDeviceDB Q3XPCControllerDriverState Q3XPCTracker; do
        count=$(grep -c "^- (void)" IPCprotocolXPC.h || echo 0)
        echo "  $protocol: $count methods"
    done
    echo ""
else
    echo "IPCprotocolXPC.h not found"
    echo ""
fi

echo "=========================================="
echo "9. Dependencies Check"
echo "=========================================="
echo ""

echo "Required headers:"
for header in E3Prefix.h QuesaController.h; do
    if find . -name "$header" | grep -q .; then
        echo "  ✓ $header"
    else
        echo "  ✗ $header not found"
    fi
done
echo ""

echo "=========================================="
echo "10. Git Status (if available)"
echo "=========================================="
echo ""

if [ -d ".git" ]; then
    echo "Current branch:"
    git branch --show-current 2>/dev/null || echo "  Not a git repository"
    echo ""
    
    echo "Recent commits:"
    git log --oneline -5 2>/dev/null || echo "  Could not read git log"
    echo ""
    
    echo "Modified files:"
    git status --short 2>/dev/null || echo "  Could not read git status"
    echo ""
else
    echo "Not a git repository"
    echo ""
fi

echo "=========================================="
echo "Diagnostics Collection Complete"
echo "=========================================="
echo ""
echo "To save this report:"
echo "  $0 $APP_PATH > diagnostics-$(date +%Y%m%d-%H%M%S).txt"
echo ""
echo "When reporting issues, include:"
echo "  - This diagnostics output"
echo "  - Error messages from Console.app"
echo "  - Steps to reproduce"
echo "  - Expected vs actual behavior"
echo ""
