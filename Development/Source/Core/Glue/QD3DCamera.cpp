/*  NAME:
        QD3DCamera.c

    DESCRIPTION:
        Entry point for Quesa API calls. Performs parameter checking and
        then forwards each API call to the equivalent E3xxxxx routine.

    COPYRIGHT:
        Copyright (c) 1999-2020, Quesa Developers. All rights reserved.

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
#include "E3Prefix.h"
#include "E3Camera.h"
#include "E3Math_Intersect.h"
#include <cmath>



//=============================================================================
//      Internal constants
//-----------------------------------------------------------------------------
// Internal constants go here





//=============================================================================
//      Internal types
//-----------------------------------------------------------------------------
// Internal types go here





//=============================================================================
//      Internal functions
//-----------------------------------------------------------------------------

static float e3fisheyeCamera_maxRadius( const TQ3Vector2D& inSensorSize,
										TQ3FisheyeCroppingFormat inCropping )
{
	float radius = 0.0f;
	switch (inCropping)
	{
		default:
		case kQ3FisheyeCroppingFormatCircular:
			radius = 0.5f * E3Num_Min( inSensorSize.x, inSensorSize.y );
			break;
		
		case kQ3FisheyeCroppingFormatCroppedCircle:
			radius = 0.5f * E3Num_Max( inSensorSize.x, inSensorSize.y );
			break;
		
		case kQ3FisheyeCroppingFormatFullFrame:
			radius = 0.5f * hypotf( inSensorSize.x, inSensorSize.y );
			break;
	}
	return radius;
}




//=============================================================================
//      Public functions
//-----------------------------------------------------------------------------
//      Q3Camera_GetType : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3ObjectType
Q3Camera_GetType(TQ3CameraObject camera)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3Camera::IsOfMyClass ( camera ), kQ3ObjectTypeInvalid ) ;



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3Camera*) camera )->GetType () ;
}





//=============================================================================
//      Q3Camera_SetData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Camera_SetData(TQ3CameraObject camera, const TQ3CameraData *cameraData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3Camera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(cameraData), kQ3Failure);



	// Debug build checks
#if Q3_DEBUG
	// Further checks on cameraData
	if (fabs( Q3FastVector3D_Length( &cameraData->placement.upVector ) - 1.0f ) > kQ3RealZero)
	{
		E3ErrorManager_PostWarning( kQ3WarningVector3DNotUnitLength );
	}
	
	if (kQ3RealZero > Q3FastPoint3D_DistanceSquared(
		&cameraData->placement.cameraLocation,
		&cameraData->placement.pointOfInterest))
	{
		E3ErrorManager_PostError( kQ3ErrorVector3DZeroLength, kQ3False );
	}
#endif



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3Camera*) camera )->SetData ( cameraData ) ;
}





//=============================================================================
//      Q3Camera_GetData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Camera_GetData(TQ3CameraObject camera, TQ3CameraData *cameraData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3Camera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(cameraData), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3Camera*) camera )->GetData ( cameraData ) ;
}





//=============================================================================
//      Q3Camera_SetPlacement : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Camera_SetPlacement(TQ3CameraObject camera, const TQ3CameraPlacement *placement)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3Camera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(placement), kQ3Failure);



	// Debug build checks
#if Q3_DEBUG
	// Further checks on placement
	if (fabs( Q3FastVector3D_Length( &placement->upVector ) - 1.0f ) > kQ3RealZero)
	{
		E3ErrorManager_PostWarning( kQ3WarningVector3DNotUnitLength );
	}
	
	if (kQ3RealZero > Q3FastPoint3D_DistanceSquared( &placement->cameraLocation,
		&placement->pointOfInterest))
	{
		E3ErrorManager_PostError( kQ3ErrorVector3DZeroLength, kQ3False );
	}
#endif



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3Camera*) camera )->SetPlacement ( placement ) ;
}





//=============================================================================
//      Q3Camera_GetPlacement : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Camera_GetPlacement(TQ3CameraObject camera, TQ3CameraPlacement *placement)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3Camera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(placement), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3Camera*) camera )->GetPlacement ( placement ) ;
}





//=============================================================================
//      Q3Camera_SetRange : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Camera_SetRange(TQ3CameraObject camera, const TQ3CameraRange *range)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3Camera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(range), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3Camera*) camera )->SetRange ( range ) ;
}





//=============================================================================
//      Q3Camera_GetRange : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Camera_GetRange(TQ3CameraObject camera, TQ3CameraRange *range)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3Camera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(range), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3Camera*) camera )->GetRange ( range ) ;
}





//=============================================================================
//      Q3Camera_SetViewPort : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Camera_SetViewPort(TQ3CameraObject camera, const TQ3CameraViewPort *viewPort)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3Camera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(viewPort), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3Camera*) camera )->SetViewPort ( viewPort ) ;
}





//=============================================================================
//      Q3Camera_GetViewPort : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Camera_GetViewPort(TQ3CameraObject camera, TQ3CameraViewPort *viewPort)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3Camera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(viewPort), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3Camera*) camera )->GetViewPort ( viewPort ) ;
}





//=============================================================================
//      Q3Camera_GetWorldToView : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Camera_GetWorldToView(TQ3CameraObject camera, TQ3Matrix4x4 *worldToView)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3Camera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(worldToView), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3Camera*) camera )->GetWorldToView ( worldToView ) ;
}





//=============================================================================
//      Q3Camera_GetWorldToFrustum : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Camera_GetWorldToFrustum(TQ3CameraObject camera, TQ3Matrix4x4 *worldToFrustum)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3Camera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(worldToFrustum), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3Camera*) camera )->GetWorldToFrustum ( worldToFrustum ) ;
}





//=============================================================================
//      Q3Camera_GetViewToFrustum : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Camera_GetViewToFrustum(TQ3CameraObject camera, TQ3Matrix4x4 *viewToFrustum)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3Camera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(viewToFrustum), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3Camera*) camera )->GetViewToFrustum ( viewToFrustum ) ;
}





/*!
	@function	Q3Camera_TransformViewToFrustum
	@abstract	Transform a point from view coordinates to frustum coordinates.
	@discussion	Whereas Q3Camera_GetViewToFrustum does not work for certain kinds
				of cameras, this function should work for all cameras.
	@param		camera		The camera.
	@param		viewPt		A point in view coordinates.
	@param		frustumPt	Receives a point in frustum coordinates.
*/
void
Q3Camera_TransformViewToFrustum( TQ3CameraObject _Nonnull camera,
								const TQ3Point3D* _Nonnull viewPt,
								TQ3Point3D* _Nonnull frustumPt )
{
	Q3_REQUIRE( E3Camera::IsOfMyClass ( camera ) );
	Q3_REQUIRE( Q3_VALID_PTR( viewPt ) );
	Q3_REQUIRE( Q3_VALID_PTR( frustumPt ) );
	
	*frustumPt = E3Camera_ViewToFrustum( camera, *viewPt );
}





/*!
	@function	Q3Camera_TransformFrustumToView
	@abstract	Transform a point from frustum coordinates to view coordinates.
	@discussion	Whereas Q3Camera_GetViewToFrustum does not work for certain kinds
				of cameras, this function should work for all cameras.
	@param		camera		The camera.
	@param		frustumPt	A point in frustum coordinates.
	@param		viewPt		Receives a point in view coordinates.
*/
void
Q3Camera_TransformFrustumToView( TQ3CameraObject _Nonnull camera,
								const TQ3Point3D* _Nonnull frustumPt,
								TQ3Point3D* _Nonnull viewPt )
{
	Q3_REQUIRE( E3Camera::IsOfMyClass ( camera ) );
	Q3_REQUIRE( Q3_VALID_PTR( viewPt ) );
	Q3_REQUIRE( Q3_VALID_PTR( frustumPt ) );
	
	*viewPt = E3Camera_FrustumToView( camera, *frustumPt );
}




/*!
 *  @function
 *      Q3Camera_IsBoundingBoxVisible
 *  @abstract
 *      Test a bounding box for visibility.
 *
 *  @discussion
 *      The bounding box (assumed to be in world coordinates) is tested for
 *		intersection with the view frustum of the camera.
 *
 *  @param camera           The view to check the bounding box against.
 *  @param bbox             The world bounding box to test.
 *  @result                 True or false as the bounding box is visible.
 */
TQ3Boolean
Q3Camera_IsBoundingBoxVisible (
    TQ3CameraObject               camera,
    const TQ3BoundingBox          *bbox
)
{
	return (TQ3Boolean) E3BoundingBox_IntersectCameraFrustum( camera, *bbox );
}





//=============================================================================
//      Q3OrthographicCamera_New : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3CameraObject
Q3OrthographicCamera_New(const TQ3OrthographicCameraData *orthographicData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(orthographicData), nullptr);



	// Debug build checks
#if Q3_DEBUG
	// Further checks on orthographicData
	if (fabs( Q3FastVector3D_Length( &orthographicData->cameraData.placement.upVector ) -
		1.0f ) > kQ3RealZero)
	{
		E3ErrorManager_PostWarning( kQ3WarningVector3DNotUnitLength );
	}
#endif



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3OrthographicCamera_New(orthographicData));
}





//=============================================================================
//      Q3OrthographicCamera_GetData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3OrthographicCamera_GetData(TQ3CameraObject camera, TQ3OrthographicCameraData *cameraData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3OrthographicCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(cameraData), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3OrthographicCamera*) camera )->GetData ( cameraData ) ;
}





//=============================================================================
//      Q3OrthographicCamera_SetData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3OrthographicCamera_SetData(TQ3CameraObject camera, const TQ3OrthographicCameraData *cameraData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3OrthographicCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(cameraData), kQ3Failure);



	// Debug build checks
#if Q3_DEBUG
	// Further checks on cameraData
	if (fabs( Q3FastVector3D_Length( &cameraData->cameraData.placement.upVector ) -
		1.0f ) > kQ3RealZero)
	{
		E3ErrorManager_PostWarning( kQ3WarningVector3DNotUnitLength );
	}
#endif



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3OrthographicCamera*) camera )->SetData ( cameraData ) ;
}





//=============================================================================
//      Q3OrthographicCamera_SetLeft : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3OrthographicCamera_SetLeft(TQ3CameraObject camera, float left)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3OrthographicCamera::IsOfMyClass ( camera ), kQ3Failure ) ;



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3OrthographicCamera*) camera )->SetLeft ( left ) ;
}





//=============================================================================
//      Q3OrthographicCamera_GetLeft : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3OrthographicCamera_GetLeft(TQ3CameraObject camera, float *left)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3OrthographicCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(left), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3OrthographicCamera*) camera )->GetLeft ( left ) ;
}





//=============================================================================
//      Q3OrthographicCamera_SetTop : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3OrthographicCamera_SetTop(TQ3CameraObject camera, float top)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3OrthographicCamera::IsOfMyClass ( camera ), kQ3Failure ) ;



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3OrthographicCamera*) camera )->SetTop ( top ) ;
}





//=============================================================================
//      Q3OrthographicCamera_GetTop : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3OrthographicCamera_GetTop(TQ3CameraObject camera, float *top)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3OrthographicCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(top), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3OrthographicCamera*) camera )->GetTop ( top ) ;
}





//=============================================================================
//      Q3OrthographicCamera_SetRight : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3OrthographicCamera_SetRight(TQ3CameraObject camera, float right)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3OrthographicCamera::IsOfMyClass ( camera ), kQ3Failure ) ;



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3OrthographicCamera*) camera )->SetRight ( right ) ;
}





//=============================================================================
//      Q3OrthographicCamera_GetRight : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3OrthographicCamera_GetRight(TQ3CameraObject camera, float *right)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3OrthographicCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(right), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3OrthographicCamera*) camera )->GetRight ( right ) ;
}





//=============================================================================
//      Q3OrthographicCamera_SetBottom : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3OrthographicCamera_SetBottom(TQ3CameraObject camera, float bottom)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3OrthographicCamera::IsOfMyClass ( camera ), kQ3Failure ) ;



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3OrthographicCamera*) camera )->SetBottom ( bottom ) ;
}





//=============================================================================
//      Q3OrthographicCamera_GetBottom : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3OrthographicCamera_GetBottom(TQ3CameraObject camera, float *bottom)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3OrthographicCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(bottom), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3OrthographicCamera*) camera )->GetBottom ( bottom ) ;
}





//=============================================================================
//      Q3ViewPlaneCamera_New : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3CameraObject
Q3ViewPlaneCamera_New(const TQ3ViewPlaneCameraData *cameraData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(cameraData), nullptr);



	// Debug build checks
#if Q3_DEBUG
	// Further checks on cameraData
	if (fabs( Q3FastVector3D_Length( &cameraData->cameraData.placement.upVector ) -
		1.0f ) > kQ3RealZero)
	{
		E3ErrorManager_PostWarning( kQ3WarningVector3DNotUnitLength );
	}
#endif



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3ViewPlaneCamera_New(cameraData));
}





//=============================================================================
//      Q3ViewPlaneCamera_GetData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewPlaneCamera_GetData(TQ3CameraObject camera, TQ3ViewPlaneCameraData *cameraData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewPlaneCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(cameraData), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewPlaneCamera*) camera )->GetData ( cameraData ) ;
}





//=============================================================================
//      Q3ViewPlaneCamera_SetData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewPlaneCamera_SetData(TQ3CameraObject camera, const TQ3ViewPlaneCameraData *cameraData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewPlaneCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(cameraData), kQ3Failure);



	// Debug build checks
#if Q3_DEBUG
	// Further checks on cameraData
	if (fabs( Q3FastVector3D_Length( &cameraData->cameraData.placement.upVector ) -
		1.0f ) > kQ3RealZero)
	{
		E3ErrorManager_PostWarning( kQ3WarningVector3DNotUnitLength );
	}
#endif



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewPlaneCamera*) camera )->SetData ( cameraData ) ;
}





//=============================================================================
//      Q3ViewPlaneCamera_SetViewPlane : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewPlaneCamera_SetViewPlane(TQ3CameraObject camera, float viewPlane)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewPlaneCamera::IsOfMyClass ( camera ), kQ3Failure ) ;



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewPlaneCamera*) camera )->SetViewPlane ( viewPlane ) ;
}





//=============================================================================
//      Q3ViewPlaneCamera_GetViewPlane : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewPlaneCamera_GetViewPlane(TQ3CameraObject camera, float *viewPlane)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewPlaneCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(viewPlane), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewPlaneCamera*) camera )->GetViewPlane ( viewPlane ) ;
}





//=============================================================================
//      Q3ViewPlaneCamera_SetHalfWidth : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewPlaneCamera_SetHalfWidth(TQ3CameraObject camera, float halfWidthAtViewPlane)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewPlaneCamera::IsOfMyClass ( camera ), kQ3Failure ) ;



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewPlaneCamera*) camera )->SetHalfWidth ( halfWidthAtViewPlane ) ;
}





//=============================================================================
//      Q3ViewPlaneCamera_GetHalfWidth : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewPlaneCamera_GetHalfWidth(TQ3CameraObject camera, float *halfWidthAtViewPlane)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewPlaneCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(halfWidthAtViewPlane), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewPlaneCamera*) camera )->GetHalfWidth ( halfWidthAtViewPlane ) ;
}





//=============================================================================
//      Q3ViewPlaneCamera_SetHalfHeight : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewPlaneCamera_SetHalfHeight(TQ3CameraObject camera, float halfHeightAtViewPlane)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewPlaneCamera::IsOfMyClass ( camera ), kQ3Failure ) ;



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewPlaneCamera*) camera )->SetHalfHeight ( halfHeightAtViewPlane ) ;
}





//=============================================================================
//      Q3ViewPlaneCamera_GetHalfHeight : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewPlaneCamera_GetHalfHeight(TQ3CameraObject camera, float *halfHeightAtViewPlane)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewPlaneCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(halfHeightAtViewPlane), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewPlaneCamera*) camera )->GetHalfHeight ( halfHeightAtViewPlane ) ;
}





//=============================================================================
//      Q3ViewPlaneCamera_SetCenterX : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewPlaneCamera_SetCenterX(TQ3CameraObject camera, float centerXOnViewPlane)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewPlaneCamera::IsOfMyClass ( camera ), kQ3Failure ) ;



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewPlaneCamera*) camera )->SetCenterX ( centerXOnViewPlane ) ;
}





//=============================================================================
//      Q3ViewPlaneCamera_GetCenterX : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewPlaneCamera_GetCenterX(TQ3CameraObject camera, float *centerXOnViewPlane)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewPlaneCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(centerXOnViewPlane), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewPlaneCamera*) camera )->GetCenterX ( centerXOnViewPlane ) ;
}





//=============================================================================
//      Q3ViewPlaneCamera_SetCenterY : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewPlaneCamera_SetCenterY(TQ3CameraObject camera, float centerYOnViewPlane)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewPlaneCamera::IsOfMyClass ( camera ), kQ3Failure ) ;



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewPlaneCamera*) camera )->SetCenterY ( centerYOnViewPlane ) ;
}





//=============================================================================
//      Q3ViewPlaneCamera_GetCenterY : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewPlaneCamera_GetCenterY(TQ3CameraObject camera, float *centerYOnViewPlane)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewPlaneCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(centerYOnViewPlane), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewPlaneCamera*) camera )->GetCenterY ( centerYOnViewPlane ) ;
}





//=============================================================================
//      Q3ViewAngleAspectCamera_New : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3CameraObject
Q3ViewAngleAspectCamera_New(const TQ3ViewAngleAspectCameraData *cameraData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(cameraData), nullptr);



	// Debug build checks
#if Q3_DEBUG
	// Further checks on cameraData
	if (fabs( Q3FastVector3D_Length( &cameraData->cameraData.placement.upVector ) -
		1.0f ) > kQ3RealZero)
	{
		E3ErrorManager_PostWarning( kQ3WarningVector3DNotUnitLength );
	}
#endif



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3ViewAngleAspectCamera_New(cameraData));
}





//=============================================================================
//      Q3ViewAngleAspectCamera_SetData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewAngleAspectCamera_SetData(TQ3CameraObject camera, const TQ3ViewAngleAspectCameraData *cameraData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewAngleAspectCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(cameraData), kQ3Failure);



	// Debug build checks
#if Q3_DEBUG
	// Further checks on cameraData
	if (fabs( Q3FastVector3D_Length( &cameraData->cameraData.placement.upVector ) -
		1.0f ) > kQ3RealZero)
	{
		E3ErrorManager_PostWarning( kQ3WarningVector3DNotUnitLength );
	}
#endif



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewAngleAspectCamera*) camera )->SetData ( cameraData ) ;
}





//=============================================================================
//      Q3ViewAngleAspectCamera_GetData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewAngleAspectCamera_GetData(TQ3CameraObject camera, TQ3ViewAngleAspectCameraData *cameraData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewAngleAspectCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(cameraData), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewAngleAspectCamera*) camera )->GetData ( cameraData ) ;
}





//=============================================================================
//      Q3ViewAngleAspectCamera_SetFOV : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewAngleAspectCamera_SetFOV(TQ3CameraObject camera, float fov)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewAngleAspectCamera::IsOfMyClass ( camera ), kQ3Failure ) ;



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewAngleAspectCamera*) camera )->SetFOV ( fov ) ;
}





//=============================================================================
//      Q3ViewAngleAspectCamera_GetFOV : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewAngleAspectCamera_GetFOV(TQ3CameraObject camera, float *fov)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewAngleAspectCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(fov), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewAngleAspectCamera*) camera )->GetFOV ( fov ) ;
}





//=============================================================================
//      Q3ViewAngleAspectCamera_SetAspectRatio : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewAngleAspectCamera_SetAspectRatio(TQ3CameraObject camera, float aspectRatioXToY)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewAngleAspectCamera::IsOfMyClass ( camera ), kQ3Failure ) ;



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewAngleAspectCamera*) camera )->SetAspectRatio ( aspectRatioXToY ) ;
}





//=============================================================================
//      Q3ViewAngleAspectCamera_GetAspectRatio : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewAngleAspectCamera_GetAspectRatio(TQ3CameraObject camera, float *aspectRatioXToY)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3ViewAngleAspectCamera::IsOfMyClass ( camera ), kQ3Failure ) ;
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(aspectRatioXToY), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewAngleAspectCamera*) camera )->GetAspectRatio ( aspectRatioXToY ) ;
}




/*!
 *  @function
 *      Q3AllSeeingCamera_New
 *  @discussion
 *      Create a new all-seeing camera object.
 *
 *  	This kind of camera sees in all directions, essentially using spherical coordinates.
 *
 *  @param cameraData       The data for the camera object.
 *  @result                 The new camera object.
 */
TQ3CameraObject
Q3AllSeeingCamera_New( const TQ3CameraData * cameraData )
{

	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3AllSeeingCamera_New(cameraData));
}


/*!
	@function	Q3FisheyeCamera_New
	@abstract	Create a fisheye camera.
	@discussion	Rendering with this camera will experience some inaccuracy, due to the fact that
				OpenGL works by linearly interpolating across triangles, while the fisheye
				projection is very much nonlinear.  Naturally, the greater the angular diameter of a
				triangle as seen from the camera, the greater the inaccuracy.  Subdividing a
				triangulation can reduce the nonlinearity artifacts.
	@param cameraData       The data for the camera object.
	@result		The new camera object.
*/
TQ3CameraObject
Q3FisheyeCamera_New(
    const TQ3FisheyeCameraData * _Nonnull cameraData
)
{
	static_assert( sizeof(TQ3FisheyeCroppingFormat) == 4,
		"size of TQ3FisheyeCroppingFormat" );
	static_assert( sizeof(TQ3FisheyeMappingFunction) == 4,
		"size of TQ3FisheyeMappingFunction" );


	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FisheyeCamera_New(cameraData));
}

/*!
	@function	Q3FisheyeCamera_GetData
	@abstract	Get the data for a fisheye camera.
	@param		camera		The camera.
	@param		data		Receives the camera data.
	@result		Success or failure of the operation.
*/
TQ3Status
Q3FisheyeCamera_GetData( TQ3CameraObject _Nonnull camera,
						TQ3FisheyeCameraData* _Nonnull data )
{
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT( E3FisheyeCamera::IsOfMyClass ( camera ), kQ3Failure ) ;


	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3FisheyeCamera*) camera )->GetData( data );
}


/*!
	@function	Q3FisheyeCamera_SetData
	@abstract	Set the data for a fisheye camera.
	@param		camera		The camera.
	@param		cameraData	The new data.
	@result		Success or failure of the operation.
*/
TQ3Status
Q3FisheyeCamera_SetData( TQ3CameraObject _Nonnull camera,
						const TQ3FisheyeCameraData * _Nonnull cameraData )
{
	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3FisheyeCamera::IsOfMyClass( camera ), kQ3Failure );
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(cameraData), kQ3Failure);



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3FisheyeCamera*) camera )->SetData( cameraData );
}


/*!
	@function	Q3FisheyeCamera_CalcAngleOfView
	@abstract	Compute the angle of view of a fisheye camera from other parameters.
	@discussion	In the full frame cropping format, the maximum angle of view is achieved only
				on the diagonals, whereas in the cropped circle format the maximum angle of view
				is achieved horizontally.
	@param			inSensorSize			The dimensions of the sensor or film in mm.
	@param			inMappingFunc			The mapping function.
	@param			inCropping				The cropping format.
	@param			inFocalLength			The focal length in mm.
	@result			The maximum angle of view in radians.
*/
float
Q3FisheyeCamera_CalcAngleOfView( const TQ3Vector2D* inSensorSize,
								TQ3FisheyeMappingFunction inMappingFunc,
								TQ3FisheyeCroppingFormat inCropping,
								float inFocalLength )
{
	float rmax = e3fisheyeCamera_maxRadius( *inSensorSize, inCropping );
	float aov;
	switch (inMappingFunc)
	{
		default:
		case kQ3FisheyeMappingFunctionOrthographic:
			aov = 2.0f * asinf( rmax / inFocalLength );
			break;
		
		case kQ3FisheyeMappingFunctionStereographic:
			aov = 4.0f * atan2f( rmax, 2.0f * inFocalLength );
			break;
		
		case kQ3FisheyeMappingFunctionEquidistant:
			aov = 2.0 * rmax / inFocalLength;
			break;
		
		case kQ3FisheyeMappingFunctionEquisolidAngle:
			aov = 4.0f * asinf( rmax / (2.0f * inFocalLength) );
			break;
	}
	return aov;
}


/*!
	@function	Q3FisheyeCamera_CalcFocalLength
	@abstract	Compute the focal length of a fisheye camera from other parameters.
	@param			inSensorSize			The dimensions of the sensor or film in mm.
	@param			inMappingFunc			The mapping function.
	@param			inCropping				The cropping format.
	@param			inAngleOfView			The maximum angle of view in radians.
	@result			The focal length in mm.
*/
float
Q3FisheyeCamera_CalcFocalLength( const TQ3Vector2D* inSensorSize,
								TQ3FisheyeMappingFunction inMappingFunc,
								TQ3FisheyeCroppingFormat inCropping,
								float inAngleOfView )
{
	float rmax = e3fisheyeCamera_maxRadius( *inSensorSize, inCropping );
	float f;
	switch (inMappingFunc)
	{
		default:
		case kQ3FisheyeMappingFunctionOrthographic:
			f = rmax / sinf( inAngleOfView / 2.0f );
			break;
		
		case kQ3FisheyeMappingFunctionStereographic:
			f = rmax / (2.0f * tanf( inAngleOfView/4.0f ) );
			break;
			
		case kQ3FisheyeMappingFunctionEquidistant:
			f = 2.0f * rmax / inAngleOfView;
			break;
			
		case kQ3FisheyeMappingFunctionEquisolidAngle:
			f = rmax / (2.0f * sinf( inAngleOfView / 4.0f ) );
			break;
	}
	return f;
}

