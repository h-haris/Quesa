# Xcode Template Usage Guide

## Visual Step-by-Step Guide

### Installation

```
┌─────────────────────────────────────────┐
│ Terminal                                │
├─────────────────────────────────────────┤
│ $ ./install_templates.sh                │
│                                         │
│ ✓ Found templates directory             │
│ ✓ Found Xcode 15.2                      │
│ ✓ Created templates directory            │
│ ✓ Installed XPC Service Template        │
│ ✓ Installed Framework Template          │
│                                         │
│ Installation Complete!                  │
│                                         │
│ Restart Xcode to use templates          │
└─────────────────────────────────────────┘
```

### Using XPC Service Template

#### Step 1: File → New → Target

```
┌──────────────────────────────────────────────────────┐
│ Choose a template for your new target:               │
├──────────────────────────────────────────────────────┤
│                                                      │
│  iOS                                                 │
│  ├─ Application                                      │
│  ├─ Framework & Library                              │
│  └─ ...                                              │
│                                                      │
│  macOS                                               │
│  ├─ Application                                      │
│  ├─ Framework & Library                              │
│  ├─ XPC Service ← (Default template)                 │
│  └─ ...                                              │
│                                                      │
│  ▼ Quesa  ← (NEW! Scroll to bottom)                 │
│  ├─ ★ Quesa 3D Device Server XPC Service ← SELECT    │
│  └─ ★ Quesa XPC Framework                            │
│                                                      │
│                          [Cancel]  [Next]            │
└──────────────────────────────────────────────────────┘
```

#### Step 2: Configure Target

```
┌──────────────────────────────────────────────────────┐
│ Choose options for your new target:                  │
├──────────────────────────────────────────────────────┤
│                                                      │
│  Product Name:                                       │
│  ┌────────────────────────────────┐                 │
│  │ Quesa3DDeviceServer            │  ← Pre-filled   │
│  └────────────────────────────────┘                 │
│                                                      │
│  Bundle Identifier:                                  │
│  ┌────────────────────────────────┐                 │
│  │ com.quesa.osx.3ddevice.serv... │  ← Pre-filled   │
│  └────────────────────────────────┘                 │
│                                                      │
│  Language:         Objective-C                       │
│  Include Tests:    ☐                                 │
│                                                      │
│                          [Cancel]  [Finish]          │
└──────────────────────────────────────────────────────┘
```

#### Step 3: Template Creates Target

```
┌──────────────────────────────────────────────────────┐
│ PROJECT                                              │
│ ├─ YourApp                                           │
│                                                      │
│ TARGETS                                              │
│ ├─ YourApp                                           │
│ ├─ YourAppTests                                      │
│ └─ ★ Quesa3DDeviceServer  ← NEW TARGET CREATED!     │
│                                                      │
└──────────────────────────────────────────────────────┘

Template automatically configured:
✓ Bundle ID: com.quesa.osx.3ddevice.server.xpc
✓ Deployment: macOS 10.15+
✓ Preprocessor: QUESA_USE_XPC=1
✓ Header paths: $(SRCROOT), $(SRCROOT)/Include
✓ Info.plist: XPCService dictionary
✓ Frameworks: Foundation, CoreFoundation
```

#### Step 4: Add Source Files

```
┌──────────────────────────────────────────────────────┐
│ Select files to add:                                 │
├──────────────────────────────────────────────────────┤
│  ☑ XPCService/Q3DdbXPC.mm                            │
│  ☑ XPCService/Q3DcontrollerXPC.mm                    │
│  ☑ IPCprotocolXPC.h                                  │
│  ☑ E3Prefix.h                                        │
│  ☐ main.mm (remove template placeholder)            │
│                                                      │
│  Add to targets:                                     │
│  ☑ Quesa3DDeviceServer                               │
│  ☐ YourApp                                           │
│                                                      │
│                          [Cancel]  [Add]             │
└──────────────────────────────────────────────────────┘
```

### Time Comparison

#### WITHOUT Template (Manual Configuration)

```
Time: ████████████████████████████ 30 minutes

Steps:
1. Create XPC Service target               [5 min]  ████
2. Configure bundle identifier             [2 min]  ██
3. Set deployment target                   [1 min]  █
4. Add preprocessor macros                 [3 min]  ███
5. Configure header search paths           [4 min]  ████
6. Set up Info.plist                       [5 min]  ████
7. Link frameworks                         [2 min]  ██
8. Configure build settings                [5 min]  ████
9. Add source files                        [3 min]  ███
                                          ──────────
                                          30 minutes
```

#### WITH Template

```
Time: ████████ 8 minutes

Steps:
1. Install template (one-time)             [2 min]  ██
2. File → New → Target → Select template   [1 min]  █
3. Add source files                        [3 min]  ███
4. Verify and build                        [2 min]  ██
                                          ──────────
                                          8 minutes

SAVINGS: 22 minutes (73% faster!)
```

### What Template Pre-Configures

```
Build Settings (automatically set):
┌────────────────────────────────────────────────┐
│ Setting                          │ Value       │
├──────────────────────────────────┼─────────────┤
│ PRODUCT_BUNDLE_IDENTIFIER        │ ✓ Set       │
│ MACOSX_DEPLOYMENT_TARGET         │ ✓ 10.15     │
│ GCC_PREPROCESSOR_DEFINITIONS     │ ✓ XPC=1     │
│ HEADER_SEARCH_PATHS              │ ✓ 3 paths   │
│ CLANG_CXX_LANGUAGE_STANDARD      │ ✓ C++14     │
│ CLANG_CXX_LIBRARY                │ ✓ libc++    │
│ CLANG_ENABLE_OBJC_ARC            │ ✓ YES       │
│ INFOPLIST_FILE                   │ ✓ XPCService│
└────────────────────────────────────────────────┘

Info.plist (automatically configured):
┌────────────────────────────────────────────────┐
│ <dict>                                         │
│   <key>CFBundleIdentifier</key>                │
│   <string>com.quesa.osx.3ddevice.server.xpc</  │
│   ...                                          │
│   <key>XPCService</key>                        │
│   <dict>                                       │
│     <key>ServiceType</key>                     │
│     <string>Application</string>      ✓        │
│     <key>RunLoopType</key>                     │
│     <string>NSRunLoop</string>        ✓        │
│   </dict>                                      │
│ </dict>                                        │
└────────────────────────────────────────────────┘
```

### Verification After Template

```bash
# Check that template worked correctly
$ xcodebuild -target Quesa3DDeviceServer -showBuildSettings \
    | grep -E "PRODUCT_BUNDLE_IDENTIFIER|PREPROCESSOR|DEPLOYMENT"

PRODUCT_BUNDLE_IDENTIFIER = com.quesa.osx.3ddevice.server.xpc ✓
GCC_PREPROCESSOR_DEFINITIONS = QUESA_USE_XPC=1 $(inherited)    ✓
MACOSX_DEPLOYMENT_TARGET = 10.15                               ✓
```

### Common Workflow with Templates

```
Day 1: One-time setup
┌─────────────────────────────────────────────┐
│ 1. Run: ./install_templates.sh              │ [2 min]
│ 2. Restart Xcode                             │ [1 min]
└─────────────────────────────────────────────┘

Day 1: First project
┌─────────────────────────────────────────────┐
│ 1. File → New → Target → Quesa XPC Service  │ [1 min]
│ 2. Add source files                          │ [3 min]
│ 3. Create framework target (duplicate)       │ [5 min]
│ 4. Configure main app embedding              │ [5 min]
│ 5. Build and validate                        │ [2 min]
└─────────────────────────────────────────────┘
Total: 16 minutes (vs 45 minutes manually!)

Day 2+: Additional projects
┌─────────────────────────────────────────────┐
│ Templates already installed!                 │
│ Just File → New → Target → Select template  │
│ Same 15-minute savings every time!           │
└─────────────────────────────────────────────┘
```

### Template vs Manual Comparison

```
                    Manual    Template    Savings
─────────────────────────────────────────────────
First time setup     45 min    16 min    29 min (64%)
Additional projects  30 min    10 min    20 min (67%)
Per-project work     30 min    10 min    20 min (67%)
Error rate           Higher    Lower     Fewer bugs
Consistency          Varies    Same      Predictable
```

### What You Still Need to Do

Templates don't automate everything. You still need to:

```
Manual Steps (Cannot be templated):
┌─────────────────────────────────────────────┐
│ ☐ Add source files to target                │
│ ☐ Configure main app "Copy Files" phase     │
│ ☐ Set target dependency                     │
│ ☐ Verify build phases                       │
│ ☐ Test functionality                        │
└─────────────────────────────────────────────┘

These are project-specific and can't be pre-configured.
But templates eliminate all the repetitive, error-prone
configuration work!
```

### Troubleshooting

```
Template doesn't appear?
┌─────────────────────────────────────────────┐
│ 1. Check installation:                       │
│    $ ls ~/Library/Developer/Xcode/Templates/ │
│       Project\ Templates/Quesa/              │
│                                              │
│ 2. Restart Xcode completely                  │
│    (Quit, don't just close windows)          │
│                                              │
│ 3. Look at BOTTOM of template list           │
│    Custom templates appear last              │
└─────────────────────────────────────────────┘

Template creates wrong settings?
┌─────────────────────────────────────────────┐
│ 1. Check Build Settings "Levels" view        │
│ 2. Ensure no xcconfig overrides              │
│ 3. Reinstall templates if corrupted          │
│    $ ./uninstall_templates.sh                │
│    $ ./install_templates.sh                  │
└─────────────────────────────────────────────┘
```

### Summary

```
✓ Install once: 2 minutes
✓ Use many times: 10 minutes per project
✓ Save 20+ minutes each time
✓ Fewer errors, more consistency
✓ Pre-configured for XPC
✓ Ready to build immediately

Templates turn this:    Into this:
┌──────────────────┐   ┌──────────────────┐
│ 30 minutes of    │   │ 10 minutes of    │
│ manual clicking  │   │ adding files     │
│ and typing       │   │                  │
│                  │   │ Ready to build!  │
│ Lots of errors   │   │                  │
│ to fix           │   │                  │
└──────────────────┘   └──────────────────┘
```

---

**Ready to speed up your XPC integration?**

```bash
./install_templates.sh
```

Then restart Xcode and enjoy faster target creation!

---

*Quesa XPC Templates Visual Guide - April 2026*
