# Quesa XPC Service Project Templates

This directory contains Xcode project templates that dramatically speed up XPC integration by pre-configuring targets with all necessary settings.

## What's Included

1. **XPC Service Template** - Pre-configured XPC service target
2. **Framework Template** - Pre-configured XPC-enabled framework target
3. **Installation Script** - Automated template installation

## Quick Start

### Option 1: Automated Installation (Recommended)

```bash
# Run the installation script
./install_templates.sh

# Restart Xcode
# Templates will appear in File → New → Target
```

### Option 2: Manual Installation

```bash
# Copy templates to Xcode
mkdir -p ~/Library/Developer/Xcode/Templates/Project\ Templates/Quesa
cp -R XPCServiceTemplate ~/Library/Developer/Xcode/Templates/Project\ Templates/Quesa/
cp -R FrameworkTemplate ~/Library/Developer/Xcode/Templates/Project\ Templates/Quesa/

# Restart Xcode
```

## Using the Templates

### Create XPC Service Target (5 minutes instead of 15!)

1. **In Xcode, select your project**
2. **File → New → Target**
3. **Select "Quesa" category** (at bottom of template list)
4. **Choose "Quesa 3D Device Server XPC Service"**
5. **Click Next**

Template automatically configures:
- ✅ Bundle ID: `com.quesa.osx.3ddevice.server.xpc`
- ✅ Deployment target: macOS 10.15
- ✅ Preprocessor: `QUESA_USE_XPC=1`
- ✅ Header search paths
- ✅ Info.plist with XPCService dictionary
- ✅ Frameworks (Foundation, CoreFoundation)

6. **Add your source files:**
   - XPCService/Q3DdbXPC.mm
   - XPCService/Q3DcontrollerXPC.mm
   - IPCprotocolXPC.h
   - Required Quesa headers

### Create XPC Framework Target (5 minutes instead of 10!)

1. **File → New → Target**
2. **Select "Quesa" category**
3. **Choose "Quesa XPC Framework"**
4. **Click Next**

Template automatically configures:
- ✅ Product name: Quesa-XPC
- ✅ Preprocessor: `QUESA_USE_XPC=1`
- ✅ Framework structure
- ✅ Build settings for XPC

5. **Add your source files:**
   - Include ControllerCoreOSX_XPC.mm (not ControllerCoreOSX.mm)
   - Other Quesa framework sources

## Time Savings

| Task | Manual | With Template | Savings |
|------|--------|---------------|---------|
| XPC Service Target | 15 min | 5 min | 10 min |
| Framework Target | 10 min | 5 min | 5 min |
| **Total** | **25 min** | **10 min** | **15 min** |

Plus fewer errors from typos or missed settings!

## What Gets Pre-Configured

### XPC Service Target Template

```
✓ Product Bundle Identifier: com.quesa.osx.3ddevice.server.xpc
✓ macOS Deployment Target: 10.15
✓ Preprocessor Macros: QUESA_USE_XPC=1
✓ Header Search Paths: $(SRCROOT), $(SRCROOT)/Include, $(SRCROOT)/Source
✓ C++ Language Standard: C++14
✓ C++ Library: libc++
✓ Enable Objective-C ARC: YES
✓ Info.plist location: $(SRCROOT)/XPCService/Info.plist
✓ XPCService dictionary: ServiceType=Application, RunLoopType=NSRunLoop
```

### Framework Target Template

```
✓ Product Name: Quesa-XPC
✓ Preprocessor Macros: QUESA_USE_XPC=1
✓ Framework structure
✓ Public/Private headers configuration
✓ Install path: @rpath
✓ Deployment target: macOS 10.15
✓ C++14 support
```

## Template Files

```
XPCServiceTemplate/
├── TemplateInfo.plist          # Template metadata and settings
├── main.mm                      # Placeholder main file
├── Info.plist                   # Pre-configured XPC service plist
└── README.md                    # This file

FrameworkTemplate/
├── TemplateInfo.plist          # Template metadata and settings
└── Quesa-XPC.h                  # Framework header

install_templates.sh            # Automated installation script
uninstall_templates.sh          # Removal script
```

## Customization

After creating target from template, you only need to:

1. **XPC Service:**
   - Add Q3DdbXPC.mm to target
   - Add Q3DcontrollerXPC.mm to target
   - Add required headers

2. **Framework:**
   - Add all Quesa source files
   - Ensure ControllerCoreOSX_XPC.mm is included (not ControllerCoreOSX.mm)

3. **Main App:**
   - Still need to manually add:
     - Target dependency on XPC service
     - Copy Files phase for embedding

## Verification

After using templates, verify with:

```bash
# Check XPC service target settings
xcodebuild -target Quesa3DDeviceServer -showBuildSettings | grep -E "PRODUCT_BUNDLE_IDENTIFIER|GCC_PREPROCESSOR_DEFINITIONS|MACOSX_DEPLOYMENT_TARGET"

# Expected output:
# PRODUCT_BUNDLE_IDENTIFIER = com.quesa.osx.3ddevice.server.xpc
# GCC_PREPROCESSOR_DEFINITIONS = QUESA_USE_XPC=1 ...
# MACOSX_DEPLOYMENT_TARGET = 10.15
```

## Troubleshooting

### Template doesn't appear in Xcode

**Solution:**
```bash
# Verify installation
ls ~/Library/Developer/Xcode/Templates/Project\ Templates/Quesa/

# Should see:
# XPCServiceTemplate
# FrameworkTemplate

# Restart Xcode completely (Quit, not just close window)
```

### Template creates target but settings are wrong

**Solution:**
The template sets base settings, but:
1. Check Build Settings in Xcode UI
2. Ensure no xcconfig overrides
3. Verify "Levels" view shows template values

### Can't find "Quesa" category

**Solution:**
Look at the **bottom** of the template list in File → New → Target.
Custom templates appear after all Apple templates.

## Updating Templates

To modify templates after installation:

```bash
# Edit in repository
nano XPCServiceTemplate/TemplateInfo.plist

# Reinstall
./install_templates.sh

# Restart Xcode
```

## Uninstallation

```bash
# Run uninstall script
./uninstall_templates.sh

# Or manually
rm -rf ~/Library/Developer/Xcode/Templates/Project\ Templates/Quesa/
```

## Integration with Setup Script

The `configure_xpc_target.sh` script now offers to install templates:

```bash
./configure_xpc_target.sh

# Will prompt:
# "Install Xcode templates? (y/n)"
# Answer 'y' for automatic installation
```

## Benefits

1. **Speed:** 15 minutes saved on setup
2. **Accuracy:** No typos in bundle IDs or settings
3. **Consistency:** Same configuration every time
4. **Confidence:** Know settings are correct from start

## Limitations

Templates provide target **configuration**, but you still need to:

- Add source files manually
- Configure main app embedding
- Set up target dependencies
- Verify build phases

These steps are inherently project-specific and can't be templated.

## Advanced: Creating Custom Templates

Based on your project needs, you can create additional templates:

```bash
# Copy existing template
cp -R XPCServiceTemplate MyCustomTemplate

# Edit TemplateInfo.plist
# Customize settings

# Install
cp -R MyCustomTemplate ~/Library/Developer/Xcode/Templates/Project\ Templates/Quesa/
```

## Support

If templates don't work as expected:

1. Check template installation: `ls ~/Library/Developer/Xcode/Templates/Project\ Templates/Quesa/`
2. Verify Xcode version: Templates require Xcode 10+
3. Review build settings after creation
4. Compare with manual configuration steps

## Next Steps

After using templates:

1. ✅ Created XPC service target (5 min)
2. ✅ Created framework target (5 min)
3. → Add source files to both targets
4. → Configure main app embedding
5. → Build and validate

See `BUILD_WITH_XPC.md` for complete instructions.

---

**Time investment:** 2 minutes to install templates
**Time saved:** 15 minutes per project
**ROI:** 750% ⚡

---

*Quesa XPC Project Templates - April 2026*
