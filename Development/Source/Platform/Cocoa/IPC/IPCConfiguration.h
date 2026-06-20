/*  NAME:
 IPCConfiguration.h
 
 DESCRIPTION:
 Configuration header to switch between PDO and XPC implementations.
 
 Define QUESA_USE_XPC to use the modern XPC implementation.
 Otherwise, the deprecated PDO implementation will be used.
 
    COPYRIGHT:
        Copyright (c) 2013-2026, Quesa Developers. All rights reserved.

        For the current release of Quesa, please see:

            <https://github.com/jwwalker/Quesa>

        For the current release of Quesa including 3D device support,
        please see: <https://github.com/h-haris/Quesa>

        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions
        are met:
        
            o Redistributions of source code must retain the above copyright
              notice, this list of conditions and the following disclaimer.
        
            o Redistributions in binary form must reproduce the above
              copyright notice, this list of conditions and the following
              disclaimer in the documentation and/or other materials provided
              with the distribution.
        
            o Neither the name of Quesa nor the names of its contributors
              may be used to endorse or promote products derived from this
              software without specific prior written permission.
        
        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
        "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
        LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
        A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
        OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
        SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
        TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
        PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
        LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
        NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
        SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    ___________________________________________________________________________
 */

#ifndef IPCConfiguration_h
#define IPCConfiguration_h

//=============================================================================
// IPC Implementation Selection
//=============================================================================

// IMPORTANT: QUESA_USE_XPC must be defined in build settings for each target:
//   - Framework (PDO) target: Set QUESA_USE_XPC=0 in build settings
//   - Framework(XPC) target: Set QUESA_USE_XPC=1 in build settings

#if !defined(QUESA_USE_XPC)
    #error "QUESA_USE_XPC must be explicitly defined in build settings! Use QUESA_USE_XPC=0 for PDO or QUESA_USE_XPC=1 for XPC"
#endif

//=============================================================================
// Include appropriate protocol header
//=============================================================================

#if (QUESA_USE_XPC!=1)
  #import "IPCprotocolPDO.h"
  #pragma message "Using PDO for inter-process communication (DEPRECATED)"
#else
  #import "IPCprotocolXPC.h"
  #pragma message "Using XPC for inter-process communication"
#endif

#endif /* IPCConfiguration_h */
