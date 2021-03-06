/*  NAME:
        GLCocoaContext.h

    DESCRIPTION:
        Header file for GLCocoaContext.m.

    COPYRIGHT:
        Copyright (c) 1999-2019, Quesa Developers. All rights reserved.

        For the current release of Quesa, please see:

            <https://github.com/jwwalker/Quesa>
        
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
#ifndef GLCOCOACONTEXT_HDR
#define GLCOCOACONTEXT_HDR
//=============================================================================
//      Include files
//-----------------------------------------------------------------------------
#import "GLPrefix.h"





//=============================================================================
//		C++ preamble
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif





//=============================================================================
//      Types
//-----------------------------------------------------------------------------
// Cocoa GL context type
#if QUESA_OS_COCOA

@class NSOpenGLContext;

class CocoaGLContext : public CQ3GLContext
{
public:
						CocoaGLContext(
								TQ3DrawContextObject theDrawContext,
								TQ3Boolean shareTextures );
						
	virtual				~CocoaGLContext();
	
	virtual void		SwapBuffers();
	
	virtual void		SetCurrentBase( TQ3Boolean inForceSet );

	virtual void		SetCurrent( TQ3Boolean inForceSet );
	
	virtual bool		UpdateWindowSize();

private:
	NSOpenGLContext		*glContext;
	BOOL				createdContext;
	// Strong so that in garbage collected (GC) mode the collector keeps track of these pointers. This is needed since they are not declared as 'id'.
	__strong void		*nsView;
	GLint				viewPort[4];
};

#endif





//=============================================================================
//		C++ postamble
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif

