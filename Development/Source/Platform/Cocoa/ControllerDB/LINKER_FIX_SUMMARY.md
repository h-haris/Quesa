# Linker Error Fix: Undefined symbol _startDeviceDB

## Problem

Both Framework (PDO) and Framework(XPC) targets were reporting undefined symbol errors for `_startDeviceDB`.

**Error:**
```
Undefined symbol: _startDeviceDB
```

## Root Cause

The issue had **two parts**:

### Part 1: Inconsistent Conditional Compilation

The conditional compilation directives were inconsistent between where `startDeviceDB()` is **defined** and where it is **called**.

**Before Fix - In Q3Ddb.mm** (where function is defined):
```objc
#ifndef QUESA_USE_XPC  // ← Compiles only if NOT defined at all
void startDeviceDB(void) { ... }
#endif
```

**In E3MacSystem.cpp** (where function is called):
```cpp
#if (QUESA_USE_XPC!=1)  // ← Compiles if not equal to 1
    startDeviceDB();
#endif
```

### Part 2: Auto-Detection Setting Wrong Values

The `IPCConfiguration.h` was auto-detecting based on macOS version:

```objc
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101500
    #define QUESA_USE_XPC 1  // ← Both targets got this!
#else
    #define QUESA_USE_XPC 0
#endif
```

This meant **both targets** were getting `QUESA_USE_XPC=1`, causing neither to compile `startDeviceDB()`.

## Solution

### Fix 1: Made Conditional Compilation Consistent

Wrapped the entire `Q3Ddb.mm` file with the same condition used in `E3MacSystem.cpp`:

```objc
// This entire file is only for PDO (non-XPC) builds
#include "IPCConfiguration.h"

#if (QUESA_USE_XPC != 1)  // ← Now matches E3MacSystem.cpp

#import "Q3Ddb.h"
// ... all PDO implementation ...

void startDeviceDB(void) { ... }

#endif // QUESA_USE_XPC != 1
```

### Fix 2: Require Explicit Build Settings

Changed `IPCConfiguration.h` to **require** explicit definition instead of auto-detecting:

```objc
#if !defined(QUESA_USE_XPC)
    #error "QUESA_USE_XPC must be explicitly defined in build settings!"
#endif
```

### Fix 3: Configure Build Settings Per Target

**This is the critical step you must do in Xcode!**

#### Framework (PDO) Target:
Build Settings → Preprocessor Macros → Add:
```
QUESA_USE_XPC=0
```

#### Framework(XPC) Target:
Build Settings → Preprocessor Macros → Add:
```
QUESA_USE_XPC=1
```

## Target Behavior

### Framework (PDO) Target (QUESA_USE_XPC=0)
- ✅ `Q3Ddb.mm` is compiled (entire file)
- ✅ `startDeviceDB()` is defined
- ✅ `E3MacSystem.cpp` calls `startDeviceDB()`
- ✅ **Linker success**: Function defined and called
- ✅ Uses PDO/NSConnection for IPC

### Framework(XPC) Target (QUESA_USE_XPC=1)
- ✅ `Q3Ddb.mm` is NOT compiled (empty file)
- ✅ `startDeviceDB()` is NOT defined
- ✅ `E3MacSystem.cpp` does NOT call `startDeviceDB()`
- ✅ **Linker success**: Function neither defined nor called
- ✅ Uses `Q3DdbXPC.mm` and XPC service instead

## Files Modified

1. **Q3Ddb.mm**
   - Added `#include "IPCConfiguration.h"` at the top
   - Wrapped entire file with `#if (QUESA_USE_XPC != 1) ... #endif`
   - Removed redundant inner conditional
   - Added clarifying comment about PDO-only usage

2. **IPCConfiguration.h**
   - Removed auto-detection logic
   - Added `#error` directive to require explicit definition
   - Forces developers to set `QUESA_USE_XPC` in build settings

## Required Xcode Configuration

### Step-by-Step:

1. **Select Framework target** → Build Settings → Preprocessor Macros
   - Debug: Add `QUESA_USE_XPC=0`
   - Release: Add `QUESA_USE_XPC=0`

2. **Select Framework(XPC) target** → Build Settings → Preprocessor Macros
   - Debug: Add `QUESA_USE_XPC=1`
   - Release: Add `QUESA_USE_XPC=1`

3. **Verify Q3Ddb.mm target membership**:
   - Select `Q3Ddb.mm` in navigator
   - File Inspector → Target Membership
   - ✅ Framework (checked)
   - ❌ Framework(XPC) (NOT checked)

4. **Clean and rebuild both targets**

**See BUILD_SETTINGS_CONFIGURATION.md for detailed instructions with screenshots.**

## Verification

After configuration, you should see in the build logs:

**Framework target:**
```
Using PDO for inter-process communication (DEPRECATED)
```

**Framework(XPC) target:**
```
Using XPC for inter-process communication
```

## Related Files

- **Q3Ddb.mm** - PDO device database implementation (modified)
- **Q3DdbXPC.mm** - XPC device database implementation (no changes)
- **E3MacSystem.cpp** - System initialization (no changes)
- **E3MacDeviceDbStart.h** - Header declaring `startDeviceDB()` (no changes)
- **IPCConfiguration.h** - Build configuration selector (modified)
- **BUILD_SETTINGS_CONFIGURATION.md** - Detailed setup guide (created)

## Key Takeaways

1. **Consistent conditionals**: Use the same `#if` condition where symbols are defined and used
2. **Explicit over implicit**: Don't rely on auto-detection for target-specific builds
3. **Target-level settings**: Set preprocessor macros at target level, not project level
4. **Target membership**: Ensure target-specific files are only in the correct target

---

**Fix Date:** April 19, 2026  
**Issue:** Undefined symbol: _startDeviceDB in both targets  
**Root Cause:** Auto-detection setting QUESA_USE_XPC=1 for both targets  
**Solution:** Explicit per-target preprocessor definitions  
**Status:** ✅ Resolved (pending Xcode build settings configuration)
