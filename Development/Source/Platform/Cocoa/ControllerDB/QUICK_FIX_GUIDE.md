# Quick Fix Guide - Undefined symbol: _startDeviceDB

## The Problem
Both Framework and Framework(XPC) targets can't find `startDeviceDB`.

## The Root Cause
Both targets were getting `QUESA_USE_XPC=1` due to auto-detection, so neither compiled the function.

## The Solution (3 Steps)

### Step 1: Set Framework Target Preprocessor
1. Select **Framework** target in Xcode
2. Go to **Build Settings** tab
3. Search for "Preprocessor Macros"
4. Add to **both Debug and Release**:
   ```
   QUESA_USE_XPC=0
   ```

### Step 2: Set Framework(XPC) Target Preprocessor
1. Select **Framework(XPC)** target in Xcode
2. Go to **Build Settings** tab
3. Search for "Preprocessor Macros"
4. Add to **both Debug and Release**:
   ```
   QUESA_USE_XPC=1
   ```

### Step 3: Verify Target Membership
1. Select `Q3Ddb.mm` in Project Navigator
2. Open File Inspector (⌥⌘1)
3. In **Target Membership**:
   - ✅ Framework - **CHECKED**
   - ❌ Framework(XPC) - **NOT CHECKED**

### Step 4: Clean and Build
```
⇧⌘K - Clean Build Folder
⌘B - Build
```

## Expected Results

### Framework Target Build Log:
```
✅ Using PDO for inter-process communication (DEPRECATED)
✅ Build succeeds
```

### Framework(XPC) Target Build Log:
```
✅ Using XPC for inter-process communication
✅ Build succeeds
```

## Still Having Issues?

See **BUILD_SETTINGS_CONFIGURATION.md** for detailed troubleshooting.

---

**TL;DR:** Add `QUESA_USE_XPC=0` to Framework target and `QUESA_USE_XPC=1` to Framework(XPC) target in Build Settings → Preprocessor Macros.
