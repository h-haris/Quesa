/*  NAME:
        GetCachedObject.cp

    DESCRIPTION:
       Functions for caching Quesa objects in a PolyValue node.

    COPYRIGHT:
        Copyright (c) 2005, Quesa Developers. All rights reserved.

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
#include "GetCachedObject.h"

#include "PolyValue.h"

namespace
{
	const char*	kCacheKey	= "[quesa]";
}

/*!
	@function	GetCachedObject
	
	@abstract	Get a cached Quesa object, if any, from a node.
	
	@param		inNode		A node.
	
	@result		An object reference or NULL.
*/
CQ3ObjectRef	GetCachedObject( const PolyValue& inNode )
{
	CQ3ObjectRef	theObject;
	const PolyValue::Dictionary&	theDict( inNode.GetDictionary() );
	PolyValue::Dictionary::const_iterator	found = theDict.find( kCacheKey );
	if (found != theDict.end())
	{
		const PolyValue&	theValue( found->second );
		if (theValue.GetType() == PolyValue::kDataTypeQuesaObject)
		{
			theObject = theValue.GetQuesaObject();
		}
	}
	return theObject;
}


/*!
	@function	SetCachedObject
	
	@abstract	Cache a Quesa object in a node.
	
	@param		ioNode		Node to modify.
	
	@param		inObject	An object.
*/
void	SetCachedObject( PolyValue& ioNode, const CQ3ObjectRef& inObject )
{
	PolyValue	theValue( inObject );
	ioNode.GetDictionary().insert(
		PolyValue::Dictionary::value_type( kCacheKey, theValue ) );
}
