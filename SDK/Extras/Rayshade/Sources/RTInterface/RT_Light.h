/*  NAME:
        RS_APISurface.h

    DESCRIPTION:
        A procedural api for RayShade.

    COPYRIGHT:
        Quesa Copyright � 1999-2000, Quesa Developers.
        
        For the list of Quesa Developers, and contact details, see:
        
            Documentation/contributors.html

        For the current version of Quesa, see:

        	<http://www.quesa.org/>

		This library is free software; you can redistribute it and/or
		modify it under the terms of the GNU Lesser General Public
		License as published by the Free Software Foundation; either
		version 2 of the License, or (at your option) any later version.

		This library is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
		Lesser General Public License for more details.

		You should have received a copy of the GNU Lesser General Public
		License along with this library; if not, write to the Free Software
		Foundation Inc, 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#ifndef _RT_LIGHT_H_
#define _RT_LIGHT_H_

#include "RT.h"

#if USE_QUESA_INCLUDES
	#include <QuesaLight.h>
#else
	#include <QD3DLight.h>
#endif
/******************************************************************************
 **																			 **
 **								Functions								     **
 **																			 **
 *****************************************************************************/
extern TQ3Status
RT_AddAmbientLight(
		TRTDrawContext			 *inContext,
        const TQ3ColorRGB        *inColor,
        float                    inBrightness);
                
extern TQ3Status
RT_AddDirectionalLight(
			TRTDrawContext			 *inContext,
            TQ3Vector3D              *inDirection,
            TQ3Boolean                  inCastShadows,
            const TQ3ColorRGB        *inColor,
            float                    inBrightness);
            
extern TQ3Status
RT_AddSpotLight(
			TRTDrawContext		 	 *inContext,
            TQ3Point3D               *inLocation,
            TQ3Vector3D              *inDirection,
            float                    hotAngle,
            float                    outerAngle,
            TQ3FallOffType           inFallOffType,
            TQ3Boolean                  inCastShadows,
            const TQ3ColorRGB        *inColor,
            float                    inBrightness);

extern TQ3Status
RT_AddPointLight(
			TRTDrawContext			 *inContext,
            TQ3Point3D               *inLocation,
            TQ3Boolean                  inCastShadows,
            const TQ3ColorRGB        *inColor,
            float                    inBrightness);
       
extern TQ3Status
RT_ResetLights(
			TRTDrawContext			 *inContext);

#endif /* _RT_LIGHT_H__ */