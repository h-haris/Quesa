/*!
	@header		QOClientStates.h
	
	This is the header for the ClientStates class of the Quesa OpenGL renderer.
*/

/*  NAME:
       QOClientStates.h

    DESCRIPTION:
        Header for Quesa OpenGL renderer.
		    
    COPYRIGHT:
        Copyright (c) 2007-2019, Quesa Developers. All rights reserved.

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

//=============================================================================
//      Include files
//-----------------------------------------------------------------------------
#include "QOPrefix.h"



//=============================================================================
//      Class Declaration
//-----------------------------------------------------------------------------
namespace QORenderer
{
	struct GLSLFuncs;
	class PerPixelLighting;
	
/*!
	@class		ClientStates
	
	@abstract	Helper to set OpenGL client states.
	
	@discussion	This object remembers the settings of the OpenGL client state
				flags, so that we can make as few calls to OpenGL as possible
				to set those flags.
*/
class ClientStates
{
public:
					ClientStates( const GLSLFuncs& inFuncs,
								const PerPixelLighting& inShader )
						: mFuncs( inFuncs )
						, mShader( inShader )
						, mGLClientStateNormal( false )
						, mGLClientStateColor( false )
						, mGLClientStateUV( false )
						, mGLClientStateLayers( false )
					{}

	void			StartProgram();
	
	void			EnableNormalArray( bool inEnable = true );
	void			DisableNormalArray();
	
	void			EnableTextureArray( bool inEnable = true );
	void			DisableTextureArray();
	
	void			EnableColorArray( bool inEnable = true );
	void			DisableColorArray();
	
	void			EnableLayerShiftArray( bool inEnable = true );
	

private:
	const GLSLFuncs&			mFuncs;
	const PerPixelLighting&		mShader;

	bool		mGLClientStateNormal;
	bool		mGLClientStateColor;
	bool		mGLClientStateUV;
	bool		mGLClientStateLayers;
};
	
}
