# Build Settings Configuration for PDO and XPC Targets

## Required Build Settings

To fix the linker error and ensure both targets build correctly, you **must** set the `QUESA_USE_XPC` preprocessor macro in Xcode build settings for each target.

## Step-by-Step Instructions

### 1. Configure Framework (PDO) Target

This is your **legacy PDO target** that should call `startDeviceDB()`.

1. Select your project in Xcode
2. Select the **Framework** target (the PDO one)
3. Go to **Build Settings** tab
4. Search for "Preprocessor Macros" or scroll to find **GCC_PREPROCESSOR_DEFINITIONS**
5. Add the following to both Debug and Release configurations:
   ```
   QUESA_USE_XPC=0
   ```

**Visual Guide:**
```
Framework Target → Build Settings → Preprocessor Macros
├─ Debug
│  └─ QUESA_USE_XPC=0
└─ Release
   └─ QUESA_USE_XPC=0
```

### 2. Configure Framework(XPC) Target

This is your **modern XPC target** that should NOT call `startDeviceDB()`.

1. Select your project in Xcode
2. Select the **Framework(XPC)** target
3. Go to **Build Settings** tab
4. Search for "Preprocessor Macros" or scroll to find **GCC_PREPROCESSOR_DEFINITIONS**
5. Add the following to both Debug and Release configurations:
   ```
   QUESA_USE_XPC=1
   ```

**Visual Guide:**
```
Framework(XPC) Target → Build Settings → Preprocessor Macros
├─ Debug
│  └─ QUESA_USE_XPC=1
└─ Release
   └─ QUESA_USE_XPC=1
```

### 3. Ensure Q3Ddb.mm is in Framework (PDO) Target Only

**Critical:** The file `Q3Ddb.mm` should be compiled ONLY in the Framework (PDO) target.

To verify:
1. Select `Q3Ddb.mm` in the Project Navigator
2. Open the **File Inspector** (⌥⌘1)
3. Check **Target Membership** section
4. Ensure:
   - ✅ **Framework** is checked
   - ❌ **Framework(XPC)** is NOT checked

## What This Does

### Framework Target (QUESA_USE_XPC=0)

```c
// In Q3Ddb.mm:
#if (QUESA_USE_XPC != 1)  // ✅ TRUE (0 != 1)
void startDeviceDB(void) {
    // Implementation compiles
}
#endif

// In E3MacSystem.cpp:
#if (QUESA_USE_XPC != 1)  // ✅ TRUE (0 != 1)
    startDeviceDB();  // ✅ Function is called
#endif
```

**Result:** ✅ Function defined and called → **Linker success**

### Framework(XPC) Target (QUESA_USE_XPC=1)

```c
// In Q3Ddb.mm:
#if (QUESA_USE_XPC != 1)  // ❌ FALSE (1 != 1 is false)
void startDeviceDB(void) {
    // NOT compiled
}
#endif

// In E3MacSystem.cpp:
#if (QUESA_USE_XPC != 1)  // ❌ FALSE (1 != 1 is false)
    startDeviceDB();  // ❌ Function NOT called
#endif
```

**Result:** ✅ Function not defined and not called → **Linker success**

## Verification

After setting the preprocessor macros, clean and rebuild:

### Framework (PDO) Target:
```bash
# Clean
Product → Clean Build Folder (⇧⌘K)

# Build Framework target
Product → Build (⌘B)

# You should see in build log:
# "Using PDO for inter-process communication (DEPRECATED)"
```

### Framework(XPC) Target:
```bash
# Clean
Product → Clean Build Folder (⇧⌘K)

# Build Framework(XPC) target
Product → Build (⌘B)

# You should see in build log:
# "Using XPC for inter-process communication"
```

## Alternative: Using .xcconfig Files

If you prefer to use configuration files instead of build settings:

### Framework-PDO.xcconfig
```xcconfig
// Framework (PDO) Configuration
GCC_PREPROCESSOR_DEFINITIONS = $(inherited) QUESA_USE_XPC=0
```

### Framework-XPC.xcconfig
```xcconfig
// Framework (XPC) Configuration
GCC_PREPROCESSOR_DEFINITIONS = $(inherited) QUESA_USE_XPC=1
```

Then set these as the Base Configuration in your target settings.

## Troubleshooting

### "Undefined symbol: _startDeviceDB" in Framework target

**Cause:** `QUESA_USE_XPC` is not set to 0 (or is set to 1)

**Fix:** 
1. Check Build Settings → Preprocessor Macros
2. Ensure `QUESA_USE_XPC=0` is set
3. Verify `Q3Ddb.mm` is in target membership
4. Clean build folder and rebuild

### "Undefined symbol: _startDeviceDB" in Framework(XPC) target

**Cause:** `QUESA_USE_XPC` is not set to 1 (or is set to 0)

**Fix:**
1. Check Build Settings → Preprocessor Macros
2. Ensure `QUESA_USE_XPC=1` is set
3. Clean build folder and rebuild

### Build error: "QUESA_USE_XPC must be explicitly defined"

**Cause:** The preprocessor macro is not set in build settings

**Fix:** Follow steps 1 or 2 above to set the macro

### Both targets show same warning message

**Cause:** The preprocessor macro might be set at the project level instead of target level

**Fix:**
1. Check project-level build settings
2. Remove `QUESA_USE_XPC` from project settings
3. Add it only to target-level settings

## Command-Line Building

### Framework (PDO):
```bash
xcodebuild \
  -target Framework \
  -configuration Debug \
  GCC_PREPROCESSOR_DEFINITIONS='$(inherited) QUESA_USE_XPC=0'
```

### Framework(XPC):
```bash
xcodebuild \
  -target "Framework(XPC)" \
  -configuration Debug \
  GCC_PREPROCESSOR_DEFINITIONS='$(inherited) QUESA_USE_XPC=1'
```

## Summary Checklist

Before building, verify:

### Framework (PDO) Target:
- [ ] `QUESA_USE_XPC=0` in Preprocessor Macros (Debug)
- [ ] `QUESA_USE_XPC=0` in Preprocessor Macros (Release)
- [ ] `Q3Ddb.mm` has target membership checked
- [ ] Build log shows: "Using PDO for inter-process communication"

### Framework(XPC) Target:
- [ ] `QUESA_USE_XPC=1` in Preprocessor Macros (Debug)
- [ ] `QUESA_USE_XPC=1` in Preprocessor Macros (Release)
- [ ] `Q3Ddb.mm` does NOT have target membership checked
- [ ] Build log shows: "Using XPC for inter-process communication"

---

**Once these settings are configured correctly, both targets will build successfully!**
