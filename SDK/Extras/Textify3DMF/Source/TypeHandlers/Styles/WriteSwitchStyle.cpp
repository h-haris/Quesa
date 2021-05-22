/*
 *  WriteSwitchStyle.h
 *  Textify3DMF
 *
 *  Created by James Walker on 2021/05/21.
 *  Copyright (c) 2021 James W. Walker.
 *
 *  This software is provided 'as-is', without any express or implied warranty.
 *  In no event will the authors be held liable for any damages arising from the
 *  use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in a
 *    product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source distribution.
 *
 */

#include "WriteSwitchStyle.h"

enum TQ3WriteSwitchMasks {
	kQ3WriteSwitchMaskDepth                     = (1 << 0),
	kQ3WriteSwitchMaskColor                     = (1 << 1)
};


WriteSwitchStyle::WriteSwitchStyle()
	: TypeHandler( 'wrsw', "WriteSwitchStyle" )
{
}

void	WriteSwitchStyle::Process( size_t inStartOffset,
									size_t inEndOffset )
{
	if (inEndOffset != inStartOffset + 4)
	{
		throw DataLengthException( Name(), inStartOffset, inEndOffset, 4 );
	}
	
	uint32_t mask = FetchUInt32( inStartOffset );
	
	Out() << Indent() << Name() << " ( " <<
		mask << " )  # ";
		
	if ( (mask & kQ3WriteSwitchMaskDepth) != 0 )
	{
		Out() << "depth, ";
	}
	else
	{
		Out() << "no depth, ";
	}
	
	if ( (mask & kQ3WriteSwitchMaskColor) != 0 )
	{
		Out() << "color";
	}
	else
	{
		Out() << "no color";
	}
	Out() << "\n";
}
