/*  NAME:
        GLTextureLoader.h

    DESCRIPTION:
        Header file for GLTextureLoader.cpp.

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
#ifndef GLTEXTURELOADER_HDR
#define GLTEXTURELOADER_HDR
//=============================================================================
//      Include files
//-----------------------------------------------------------------------------
#include "GLPrefix.h"

namespace QORenderer
{
	struct GLFuncs;
}





//=============================================================================
//      Function prototypes
//-----------------------------------------------------------------------------

/*!
	@function	GLTextureLoader
	
	@abstract	Load a Quesa texture object as an OpenGL texture object.
	@param		inTexture			A texture object.
	@param		inPremultiplyAlpha	If true, the loader will multiply each color
									value by its alpha value.  Use this if your
									texture data has an alpha channel and is NOT
									set up with premultiplied alpha.
	@param		inFuncs				OpenGL function pointers.
	@result		An OpenGL texture "name", or 0 on failure.
*/
GLuint	GLTextureLoader( TQ3TextureObject inTexture,
						TQ3Boolean inPremultiplyAlpha,
						const QORenderer::GLFuncs& inFuncs );



#endif
