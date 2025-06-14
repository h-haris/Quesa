/*  NAME:
        E3GeometryTriMesh.cpp

    DESCRIPTION:
        Implementation of Quesa TriMesh geometry class.

    COPYRIGHT:
        Copyright (c) 1999-2023, Quesa Developers. All rights reserved.

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
#include "E3FastArray.h"
#include "E3View.h"
#include "E3Pick.h"
#include "E3Set.h"
#include "E3Math.h"
#include "E3Math_Intersect.h"
#include "E3Geometry.h"
#include "E3GeometryTriMesh.h"
#include "E3ErrorManager.h"
#include "QuesaMathOperators.hpp"

#include <cstring>
#include <utility>




//=============================================================================
//      Internal constants
//-----------------------------------------------------------------------------
const TQ3Uns32 kTriMeshNone											= 0;
const TQ3Uns32 kTriMeshLocked										= (1 << 0);
const TQ3Uns32 kTriMeshLockedReadOnly								= (1 << 1);





//=============================================================================
//      Internal types
//-----------------------------------------------------------------------------
// TriMesh instance data
typedef struct {
	TQ3Uns32			theFlags;
	TQ3Uns32			lockCount;
	TQ3TriMeshData		geomData;
} TQ3TriMeshInstanceData;


class E3NakedTriMesh : public E3Geometry // This is a leaf class so no other classes use this,
								// so it can be here in the .c file rather than in
								// the .h file, hence all the fields can be public
								// as nobody should be including this file
{
Q3_CLASS_ENUMS ( kQ3GeometryTypeNakedTriMesh, E3NakedTriMesh, E3Geometry )
public :

	TQ3TriMeshInstanceData			instanceData ;

} ;



struct TQ3TriMeshOuterData
{
	TQ3AttributeSet		geomAttributeSet;
	E3NakedTriMesh*		nakedTriMesh;
};


class E3TriMesh : public E3Geometry // This is a leaf class so no other classes use this,
								// so it can be here in the .c file rather than in
								// the .h file, hence all the fields can be public
								// as nobody should be including this file
{
Q3_CLASS_ENUMS ( kQ3GeometryTypeTriMesh, E3TriMesh, E3Geometry )
public :

	TQ3TriMeshOuterData			instanceData ;

} ;



//=============================================================================
//      Internal functions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//      e3geom_trimesh_clone : Clone a block of memory.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_trimesh_clone(void *srcPtr, void **dstPtr, TQ3Uns32 theSize)
{


	// Validate our parameters
	Q3_REQUIRE_OR_RESULT(srcPtr  != nullptr, kQ3Failure);
	Q3_REQUIRE_OR_RESULT(dstPtr  != nullptr, kQ3Failure);
	Q3_REQUIRE_OR_RESULT(theSize != 0,    kQ3Failure);



	// Alocate the memory
	*dstPtr = Q3Memory_Allocate(theSize);
	if (*dstPtr == nullptr)
		return(kQ3Failure);



	// Copy it over
	Q3Memory_Copy(srcPtr, *dstPtr, theSize);

	return(kQ3Success);
}





//=============================================================================
//      e3geom_trimesh_attribute_find : Find a TriMesh attribute.
//-----------------------------------------------------------------------------
static TQ3TriMeshAttributeData *
e3geom_trimesh_attribute_find(TQ3Uns32						numAttributeTypes,
								TQ3TriMeshAttributeData		*attributeTypes,
								TQ3AttributeType			theType)
{	TQ3Uns32		n;



	// Find the attribute type
	for (n = 0; n < numAttributeTypes; n++)
		{
		if (attributeTypes[n].attributeType == theType)
			return(&attributeTypes[n]);
		}

	return(nullptr);
}





//=============================================================================
//		e3geom_trimesh_disposeattributes: Free a TQ3TriMeshAttributeData array.
//-----------------------------------------------------------------------------
static void
e3geom_trimesh_disposeattributes(TQ3Uns32						numAttributeTypes,
								TQ3Uns32						numArrayMembers,
									TQ3TriMeshAttributeData		**attributeTypes)
{
	TQ3Uns32 i, j;

	if (*attributeTypes != nullptr)
		{
		for (i=0; i<numAttributeTypes; i++)
			{
			if ((*attributeTypes)[i].attributeType == kQ3AttributeTypeSurfaceShader)
				{
				TQ3Object*	obArray = (TQ3Object*) (*attributeTypes)[i].data;

				for (j = 0; j < numArrayMembers; ++j)
					{
					Q3Object_CleanDispose( &obArray[j] );
					}
				}

			Q3Memory_Free( &((*attributeTypes)[i].data) );
			Q3Memory_Free( &((*attributeTypes)[i].attributeUseArray) );
			}
		}

	Q3Memory_Free( attributeTypes );
}





//=============================================================================
//      e3geom_trimesh_copyattributes : Copy a TQ3TriMeshAttributeData array.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_trimesh_copyattributes(  TQ3Uns32					numAttributeTypes,
								TQ3Uns32					numElements,
								TQ3TriMeshAttributeData		*srcAttributeTypes,
								TQ3TriMeshAttributeData		**destAttributeTypes)
	{
	if ( numAttributeTypes < 1 )
		{
		*destAttributeTypes = nullptr ;
		return kQ3Success ;
		}

	TQ3Status qd3dStatus = e3geom_trimesh_clone(srcAttributeTypes,
									  (void **) destAttributeTypes,
									  static_cast<TQ3Uns32>(numAttributeTypes * sizeof(TQ3TriMeshAttributeData)));
	if (qd3dStatus != kQ3Success)
		return(qd3dStatus);

	for ( TQ3Uns32 i = 0 ; i < numAttributeTypes && qd3dStatus == kQ3Success ; ++i )
		{
		// We must make a copy of the data elements; but first, we must figure
		// out how big they are (depends on the attribute type)
		TQ3AttributeType attrType = srcAttributeTypes[i].attributeType;

		if (attrType == kQ3AttributeTypeSurfaceShader)
			{
			(*destAttributeTypes)[i].data = Q3Memory_Allocate( static_cast<TQ3Uns32>(numElements * sizeof(TQ3Object)) );
			TQ3Object*	dstObArray = (TQ3Object*) (*destAttributeTypes)[i].data;
			if (dstObArray != nullptr)
				{
				TQ3Object*	srcObArray = (TQ3Object*) srcAttributeTypes[i].data;

				for (TQ3Uns32 j = 0; j < numElements; ++j)
					{
					E3Shared_Acquire( &dstObArray[j], srcObArray[j] );
					}
				}
			}
		else
			{
			attrType = E3Attribute_AttributeToClassType(attrType);
			E3ClassInfoPtr theClass = E3ClassTree::GetClass ( attrType ) ;
			if ( theClass != nullptr )
				{
				// Copy the attribute data
				TQ3Uns32 attrSize = theClass->GetInstanceSize () ;
				TQ3Uns32 bytes    = numElements * attrSize ;
				if ( bytes != 0 )
					qd3dStatus = e3geom_trimesh_clone(srcAttributeTypes[i].data,
													  &(*destAttributeTypes)[i].data,
													  bytes);
				else
					(*destAttributeTypes)[i].data = nullptr;


				// Copy the custom attribute useage flags
				bytes = static_cast<TQ3Uns32>(numElements * sizeof(char));
				if (bytes != 0 && srcAttributeTypes[i].attributeUseArray != nullptr)
					qd3dStatus = e3geom_trimesh_clone(srcAttributeTypes[i].attributeUseArray,
													  (void **) &(*destAttributeTypes)[i].attributeUseArray,
													  bytes);
				else
					(*destAttributeTypes)[i].attributeUseArray = nullptr;
				}
			}
		}

	return qd3dStatus ;
	}





//=============================================================================
//      e3geom_trimesh_disposedata : Dispose of a TQ3TriMeshData.
//-----------------------------------------------------------------------------
static void
e3geom_trimesh_disposedata(TQ3TriMeshData *theTriMesh)
{


	// Dispose of the TriMesh
	Q3Object_CleanDispose(&theTriMesh->triMeshAttributeSet );

	Q3Memory_Free( &theTriMesh->triangles );
	e3geom_trimesh_disposeattributes( theTriMesh->numTriangleAttributeTypes,
									  theTriMesh->numTriangles,
									  &theTriMesh->triangleAttributeTypes );

	Q3Memory_Free( &theTriMesh->edges );
	e3geom_trimesh_disposeattributes( theTriMesh->numEdgeAttributeTypes,
									  theTriMesh->numEdges,
									  &theTriMesh->edgeAttributeTypes );

	Q3Memory_Free( &theTriMesh->points );
	e3geom_trimesh_disposeattributes( theTriMesh->numVertexAttributeTypes,
									  theTriMesh->numPoints,
									  &theTriMesh->vertexAttributeTypes );
}





//=============================================================================
//      e3geom_nakedtrimesh_copydata : Copy TQ3TriMeshData from one to another.
//-----------------------------------------------------------------------------
//		Note :	Since this will be used for NakedTriMesh objects, we leave the
//				triMeshAttributeSet member of the destination as nullptr.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_nakedtrimesh_copydata(const TQ3TriMeshData *src, TQ3TriMeshData *dst )
{	TQ3Status		qd3dStatus = kQ3Success;
	TQ3Uns32		n = 0;



	// 0. Initialise ourselves
	Q3Memory_Clear(dst, sizeof(TQ3TriMeshData));



	// 2. triangles
	if (qd3dStatus == kQ3Success)
		{
		n = src->numTriangles;
		if (n)
			{
			qd3dStatus = e3geom_trimesh_clone( src->triangles, (void **) &dst->triangles,
										       static_cast<TQ3Uns32>(n * sizeof(TQ3TriMeshTriangleData)) );
			if (qd3dStatus == kQ3Success)
				dst->numTriangles = n;
			}
		}



	// 3. triangleAttributeTypes
	if (qd3dStatus == kQ3Success)
		{
		qd3dStatus = e3geom_trimesh_copyattributes( src->numTriangleAttributeTypes, n,
							 					    src->triangleAttributeTypes,
												   &dst->triangleAttributeTypes );
		if (qd3dStatus == kQ3Success)
			dst->numTriangleAttributeTypes = src->numTriangleAttributeTypes;
		}



	// 4. edges
	if (qd3dStatus == kQ3Success)
		{
		n = src->numEdges;
		if (n)
			{
			qd3dStatus = e3geom_trimesh_clone( src->edges, (void **) &dst->edges,
										       static_cast<TQ3Uns32>(n * sizeof(TQ3TriMeshEdgeData)) );
			if (qd3dStatus == kQ3Success)
				dst->numEdges = n;
			}
		}



	// 5. edgeAttributesTypes
	if (qd3dStatus == kQ3Success)
		{
		qd3dStatus = e3geom_trimesh_copyattributes( src->numEdgeAttributeTypes, n,
												    src->edgeAttributeTypes,
												   &dst->edgeAttributeTypes );
		if (qd3dStatus == kQ3Success)
			dst->numEdgeAttributeTypes = src->numEdgeAttributeTypes;
		}



	// 6. points (vertices)
	if (qd3dStatus == kQ3Success)
		{
		n = src->numPoints;
		if (n)
			{
			qd3dStatus = e3geom_trimesh_clone( src->points, (void **) &dst->points,
				static_cast<TQ3Uns32>(n * sizeof(TQ3Point3D)) );
			if (qd3dStatus == kQ3Success)
				dst->numPoints = n;
			}
		}



	// 7. vertexAttributeTypes
	if (qd3dStatus == kQ3Success)
		{
		qd3dStatus = e3geom_trimesh_copyattributes( src->numVertexAttributeTypes, n,
												    src->vertexAttributeTypes,
												   &dst->vertexAttributeTypes );
		if (qd3dStatus == kQ3Success)
			dst->numVertexAttributeTypes = src->numVertexAttributeTypes;
		}



	// 8. bounding box
	if (qd3dStatus == kQ3Success)
		{
        if (src->bBox.isEmpty && (dst->numPoints > 0))
            Q3BoundingBox_SetFromPoints3D(&dst->bBox,
                                           dst->points,
                                           dst->numPoints,
                                           sizeof(TQ3Point3D));
        else
            Q3BoundingBox_Copy(&src->bBox, &dst->bBox);
		}



	// Handle failure
	if (qd3dStatus != kQ3Success)
		e3geom_trimesh_disposedata(dst);

	return(qd3dStatus);
}





//=============================================================================
//      e3geom_trimesh_get_geom_data : Get the TQ3TriMeshData for a TriMesh.
//-----------------------------------------------------------------------------
static const TQ3TriMeshData *
e3geom_trimesh_get_geom_data(TQ3Object theObject, const void *objectData)
{	const TQ3TriMeshOuterData		*instanceData;
	const TQ3TriMeshData				*geomData;



	// Obtain the geometry data from a submitted object/data pair
	//
	// If the app has submitted a retained mode object, theObject will be non-nullptr
	// and objectData will be the instance data for that object - i.e., a pointer
	// to a TQ3TriMeshOuterData structure.
	//
	// If the app has submitted the data for a TriMesh in immediate mode, theObject
	// will be nullptr and objectData is the pointer submitted by the app - i.e., a
	// pointer to the public TQ3TriMeshData structure.
	// pointer they submitted to the public geometry data (i.e., a TQ3TriMeshData).
	if (theObject != nullptr)
		{
		instanceData = (const TQ3TriMeshOuterData *) objectData;
		geomData     = &instanceData->nakedTriMesh->instanceData.geomData;
		}
	else
		geomData = (const TQ3TriMeshData *) objectData;

	return(geomData);
}





//=============================================================================
//      e3geom_trimesh_optimize_normals : Optimise TriMesh normals.
//-----------------------------------------------------------------------------
static void
e3geom_trimesh_optimize_normals(TQ3Uns32 numNormals, TQ3TriMeshAttributeData *attributeData)
{	TQ3Vector3D		*theNormals;
	TQ3Uns32		n;



	// Validate our parameters
	Q3_ASSERT_VALID_PTR(attributeData);



	// Normalize the normal vectors
	theNormals = (TQ3Vector3D *) attributeData->data;

	if (attributeData->attributeUseArray != nullptr)
		{
		// Process normals which exist
		for (n = 0; n < numNormals; n++)
			{
			if (attributeData->attributeUseArray[n])
				Q3FastVector3D_Normalize(&theNormals[n], &theNormals[n]);
			}
		}
	else
		{
		// Process every normal
		for (n = 0; n < numNormals; n++)
			Q3FastVector3D_Normalize(&theNormals[n], &theNormals[n]);
		}
}





//=============================================================================
//      e3geom_trimesh_optimize : Optimise a TriMesh.
//-----------------------------------------------------------------------------
#if QUESA_NORMALIZE_NORMALS
static void
e3geom_trimesh_optimize(TQ3TriMeshData *theTriMesh)
{	TQ3TriMeshAttributeData		*attributeData;



	// Validate our parameters
	Q3_ASSERT_VALID_PTR(theTriMesh);



	// Normalize triangle normals
	attributeData = e3geom_trimesh_attribute_find(theTriMesh->numTriangleAttributeTypes,
												  theTriMesh->triangleAttributeTypes,
												  kQ3AttributeTypeNormal);
	if (attributeData != nullptr)
		e3geom_trimesh_optimize_normals(theTriMesh->numTriangles, attributeData);



	// Normalize vertex normals
	attributeData = e3geom_trimesh_attribute_find(theTriMesh->numVertexAttributeTypes,
												  theTriMesh->vertexAttributeTypes,
												  kQ3AttributeTypeNormal);
	if (attributeData != nullptr)
		e3geom_trimesh_optimize_normals(theTriMesh->numPoints, attributeData);
}
#else
	#define		e3geom_trimesh_optimize(x)
#endif





//=============================================================================
//      e3geom_trimesh_validate : Check for bad indices.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_trimesh_validate( TQ3TriMeshData *theTriMesh )
{
	TQ3Status	theStatus = kQ3Success;
	bool	reportedFaceIndexWarning = false;

	// Check faces
	TQ3Uns32	i;
	const TQ3Uns32	faceCount = theTriMesh->numTriangles;
	const TQ3Uns32	pointCount = theTriMesh->numPoints;
	for (i = 0; i < faceCount; ++i)
	{
		if ( (theTriMesh->triangles[i].pointIndices[0] >= pointCount) ||
			(theTriMesh->triangles[i].pointIndices[1] >= pointCount) ||
			(theTriMesh->triangles[i].pointIndices[2] >= pointCount) )
		{
			E3ErrorManager_PostError( kQ3ErrorTriMeshPointIndexOutOfRange,
				kQ3False );
			theStatus = kQ3Failure;
			break;
		}
	}

	// Check edges
	const TQ3Uns32	edgeCount = theTriMesh->numEdges;
	for (i = 0; i < edgeCount; ++i)
	{
		if ( (theTriMesh->edges[i].pointIndices[0] >= pointCount) ||
			(theTriMesh->edges[i].pointIndices[1] >= pointCount) )
		{
			E3ErrorManager_PostError( kQ3ErrorTriMeshPointIndexOutOfRange,
				kQ3False );
			theStatus = kQ3Failure;
			break;
		}

		if ( (theTriMesh->edges[i].triangleIndices[0] >= faceCount) &&
			(theTriMesh->edges[i].triangleIndices[0] != kQ3ArrayIndexNULL) )
		{
			if (! reportedFaceIndexWarning)
			{
				E3ErrorManager_PostWarning(
					kQ3WarningTriMeshTriangleIndexOutOfBounds );
				reportedFaceIndexWarning = true;
			}
			theTriMesh->edges[i].triangleIndices[0] = kQ3ArrayIndexNULL;
		}

		if ( (theTriMesh->edges[i].triangleIndices[1] >= faceCount) &&
			(theTriMesh->edges[i].triangleIndices[1] != kQ3ArrayIndexNULL) )
		{
			if (! reportedFaceIndexWarning)
			{
				E3ErrorManager_PostWarning(
					kQ3WarningTriMeshTriangleIndexOutOfBounds );
				reportedFaceIndexWarning = true;
			}
			theTriMesh->edges[i].triangleIndices[1] = kQ3ArrayIndexNULL;
		}
	}

	return theStatus;
}


//=============================================================================
//      e3geom_nakedtrimesh_new : TriMesh new method.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_nakedtrimesh_new(TQ3Object theObject, void *privateData, const void *paramData)
{	TQ3TriMeshInstanceData		*instanceData = (TQ3TriMeshInstanceData *) privateData;
	const TQ3TriMeshData		*trimeshData  = (const TQ3TriMeshData   *) paramData;
	TQ3Status					qd3dStatus;
#pragma unused(theObject)



	// Initialise the TriMesh, then optimise it
	instanceData->theFlags = kTriMeshNone;
	qd3dStatus = e3geom_nakedtrimesh_copydata( trimeshData, &instanceData->geomData );

	if (qd3dStatus == kQ3Success)
	{
		qd3dStatus = e3geom_trimesh_validate( &instanceData->geomData );
	}

	if (qd3dStatus == kQ3Success)
	{
		e3geom_trimesh_optimize(&instanceData->geomData);
	}

	return qd3dStatus;
}




//=============================================================================
//      e3geom_trimesh_new : TriMesh new method.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_trimesh_new(TQ3Object theObject, void *privateData, const void *paramData)
{
	TQ3TriMeshOuterData		*instanceData = (TQ3TriMeshOuterData *) privateData;
	const TQ3TriMeshData		*trimeshData  = (const TQ3TriMeshData   *) paramData;
#pragma unused(theObject)


	E3Shared_Acquire(&instanceData->geomAttributeSet, trimeshData->triMeshAttributeSet);

	instanceData->nakedTriMesh = (E3NakedTriMesh*) E3ClassTree::CreateInstance(
		kQ3GeometryTypeNakedTriMesh, kQ3False, trimeshData );

	return (instanceData->nakedTriMesh != nullptr)? kQ3Success : kQ3Failure;
}





//=============================================================================
//      e3geom_nakedtrimesh_new_nocopy : TriMesh new method.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_nakedtrimesh_new_nocopy(TQ3Object theObject, void *privateData, const void *paramData)
{	TQ3TriMeshInstanceData		*instanceData = (TQ3TriMeshInstanceData *) privateData;
	const TQ3TriMeshData		*trimeshData  = (const TQ3TriMeshData   *) paramData;
#pragma unused(theObject)



	// Initialise the TriMesh, then optimise it
	instanceData->theFlags = kTriMeshNone;

	Q3Memory_Copy( trimeshData, &instanceData->geomData, sizeof(TQ3TriMeshData) );

	instanceData->geomData.triMeshAttributeSet = nullptr;

	if (instanceData->geomData.bBox.isEmpty)
            Q3BoundingBox_SetFromPoints3D(&instanceData->geomData.bBox,
                                           instanceData->geomData.points,
                                           instanceData->geomData.numPoints,
                                           sizeof(TQ3Point3D));


	e3geom_trimesh_optimize(&instanceData->geomData);

	return kQ3Success;
}





//=============================================================================
//      e3geom_trimesh_delete : TriMesh delete method.
//-----------------------------------------------------------------------------
static void
e3geom_trimesh_delete(TQ3Object theObject, void *privateData)
{	TQ3TriMeshOuterData		*instanceData = (TQ3TriMeshOuterData *) privateData;
#pragma unused(theObject)



	// Dispose of our instance data
	Q3Object_CleanDispose( &instanceData->geomAttributeSet );
	Q3Object_CleanDispose( (TQ3Object*) &instanceData->nakedTriMesh );
}





//=============================================================================
//      e3geom_nakedtrimesh_delete : TriMesh delete method.
//-----------------------------------------------------------------------------
static void
e3geom_nakedtrimesh_delete(TQ3Object theObject, void *privateData)
{	TQ3TriMeshInstanceData		*instanceData = (TQ3TriMeshInstanceData *) privateData;
#pragma unused(theObject)



	// Dispose of our instance data
	e3geom_trimesh_disposedata(&instanceData->geomData);
}





//=============================================================================
//      e3geom_nakedtrimesh_duplicate : TriMesh duplicate method.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_nakedtrimesh_duplicate(TQ3Object fromObject, const void *fromPrivateData,
						  TQ3Object toObject,  void       *toPrivateData)
{	const TQ3TriMeshInstanceData		*fromData = (const TQ3TriMeshInstanceData *) fromPrivateData;
	TQ3TriMeshInstanceData				*toData   = (TQ3TriMeshInstanceData *)       toPrivateData;
	TQ3Status							qd3dStatus;



	// Validate our parameters
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(fromObject),      kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(fromPrivateData), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(toObject),        kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(toPrivateData),   kQ3Failure);



	// Initialise the instance data of the new object
	toData->theFlags = fromData->theFlags;
	qd3dStatus       = e3geom_nakedtrimesh_copydata( &fromData->geomData, &toData->geomData );

	return(qd3dStatus);
}





//=============================================================================
//      e3geom_trimesh_duplicate : TriMesh duplicate method.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_trimesh_duplicate(TQ3Object fromObject, const void *fromPrivateData,
						  TQ3Object toObject,  void       *toPrivateData)
{	const TQ3TriMeshOuterData		*fromData = (const TQ3TriMeshOuterData *) fromPrivateData;
	TQ3TriMeshOuterData				*toData   = (TQ3TriMeshOuterData *)       toPrivateData;
	TQ3Status							qd3dStatus;



	// Validate our parameters
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(fromObject),      kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(fromPrivateData), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(toObject),        kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(toPrivateData),   kQ3Failure);



	// Initialise the instance data of the new object
	qd3dStatus = kQ3Success;
	if (fromData->geomAttributeSet == nullptr)
	{
		toData->geomAttributeSet = nullptr;
	}
	else
	{
		toData->geomAttributeSet = Q3Object_Duplicate( fromData->geomAttributeSet );
		if (toData->geomAttributeSet == nullptr)
		{
			qd3dStatus = kQ3Failure;
		}
	}

	if (fromData->nakedTriMesh == nullptr)
	{
		toData->nakedTriMesh = nullptr;
	}
	else
	{
		toData->nakedTriMesh = (E3NakedTriMesh*) Q3Object_Duplicate( fromData->nakedTriMesh );
		if (toData->nakedTriMesh == nullptr)
		{
			qd3dStatus = kQ3Failure;
		}
	}


	return(qd3dStatus);
}





//=============================================================================
//      e3geom_trimesh_triangle_new : Retrieve a triangle from the TriMesh.
//-----------------------------------------------------------------------------
static void
e3geom_trimesh_triangle_new(TQ3ViewObject theView, const TQ3TriMeshData *theTriMesh, TQ3Uns32 theIndex, TQ3TriangleData *theTriangle)
{	TQ3Uns32				n, m, i0, i1, i2, vertIndex, attrSize;
	TQ3OrientationStyle		theOrientation;
	TQ3Vector3D				theNormal;
	TQ3ObjectType			attrType;
	E3ClassInfoPtr			theClass;


	// Validate our parameters
	Q3_REQUIRE(Q3_VALID_PTR(theTriMesh));
	Q3_REQUIRE(theIndex < theTriMesh->numTriangles);
	Q3_REQUIRE(Q3_VALID_PTR(theTriangle));



	// Initialise the data
	Q3Memory_Clear(theTriangle, sizeof(TQ3TriangleData));



	// Set up the triangle
	//
	// We always create an attribute set, since our triangles will always have at least
	// a triangle normal for better performance when backface culling is on.
	theTriangle->triangleAttributeSet = Q3AttributeSet_New();
	TQ3AttributeSet			triAtts = theTriangle->triangleAttributeSet;

	if (triAtts != nullptr)
		{
		// Copy overall TriMesh attributes to the triangle.
		// If the TriMesh has a texture shader, the triangle will get a
		// reference to that, not a duplicate.
		TQ3AttributeSet tmAtts = theTriMesh->triMeshAttributeSet;
		if (tmAtts != nullptr)
			{
			Q3AttributeSet_Inherit( tmAtts,
				triAtts,
				triAtts );
			}

		// Add the attributes
		for (n = 0; n < theTriMesh->numTriangleAttributeTypes; n++)
			{
			attrType = theTriMesh->triangleAttributeTypes[n].attributeType;
			attrType = E3Attribute_AttributeToClassType(attrType);
			theClass = E3ClassTree::GetClass ( attrType ) ;
			if (theClass != nullptr)
				{
				attrSize = theClass->GetInstanceSize () ;
				Q3AttributeSet_Add(triAtts, attrType,
									(TQ3Uns8 *) theTriMesh->triangleAttributeTypes[n].data + (theIndex * attrSize));
				}
			}



		// Add the triangle normal
		if (!Q3AttributeSet_Contains(triAtts, kQ3AttributeTypeNormal))
			{
			// Calculate the triangle normal
			//
			// We can find the normal for a CCW triangle with Q3Point3D_CrossProductTri.
			i0 = theTriMesh->triangles[theIndex].pointIndices[0];
			i1 = theTriMesh->triangles[theIndex].pointIndices[1];
			i2 = theTriMesh->triangles[theIndex].pointIndices[2];

			Q3Point3D_CrossProductTri(&theTriMesh->points[i0],
									  &theTriMesh->points[i1],
									  &theTriMesh->points[i2],
									  &theNormal);
			Q3Vector3D_Normalize(&theNormal, &theNormal);


			// Reverse the normal if required
			//
			// Since the default normal for a triangle depends on the current orientation
			// style, we need to reverse the normal if the triangle is actually CW.
			theOrientation = E3View_State_GetStyleOrientation(theView);
			if (theOrientation == kQ3OrientationStyleClockwise)
				Q3Vector3D_Negate(&theNormal, &theNormal);


			// Add the normal
			Q3AttributeSet_Add(triAtts, kQ3AttributeTypeNormal, &theNormal);
			}
		}



	// Set up the vertices
	for (n = 0; n < 3; n++)
		{
		// Set up the point
		vertIndex = theTriMesh->triangles[theIndex].pointIndices[n];
		theTriangle->vertices[n].point = theTriMesh->points[vertIndex];


		// Set up the attributes
		if (theTriMesh->numVertexAttributeTypes != 0)
			{
			Q3_ASSERT(Q3_VALID_PTR(theTriMesh->vertexAttributeTypes));
			theTriangle->vertices[n].attributeSet = Q3AttributeSet_New();
			TQ3AttributeSet vertAtts = theTriangle->vertices[n].attributeSet;
			if (vertAtts != nullptr)
				{
				for (m = 0; m < theTriMesh->numVertexAttributeTypes; m++)
					{
					attrType = theTriMesh->vertexAttributeTypes[m].attributeType;
					attrType = E3Attribute_AttributeToClassType(attrType);
					theClass = E3ClassTree::GetClass ( attrType ) ;
					if (theClass != nullptr)
						{
						attrSize = theClass->GetInstanceSize () ;
						Q3AttributeSet_Add(vertAtts, attrType,
											(TQ3Uns8 *) theTriMesh->vertexAttributeTypes[m].data + (vertIndex * attrSize));
						}
					}
				}
			}
		}
}





//=============================================================================
//      e3geom_trimesh_triangle_delete : Delete the data for a single triangle.
//-----------------------------------------------------------------------------
static void
e3geom_trimesh_triangle_delete(TQ3TriangleData *theTriangle)
{	TQ3Uns32		n;



	// Validate our parameters
	Q3_REQUIRE(Q3_VALID_PTR(theTriangle));



	// Dispose of the triangle
	for (n = 0; n < 3; n++)
		Q3Object_CleanDispose(&theTriangle->vertices[n].attributeSet);

	Q3Object_CleanDispose(&theTriangle->triangleAttributeSet);
}





//=============================================================================
//      e3geom_trimesh_cache_new : TriMesh cache new method.
//-----------------------------------------------------------------------------
static TQ3Object
e3geom_trimesh_cache_new(TQ3ViewObject theView, TQ3GeometryObject theGeom,
						const void *dataParam)
{
	const TQ3TriMeshOuterData* instanceData = (const TQ3TriMeshOuterData*) dataParam;
	TQ3TriangleData			triangleData;
	TQ3GeometryObject		theTriangle;
	TQ3GroupObject			theGroup;
	TQ3Uns32				n;
	const TQ3TriMeshData*	geomData;


	// Create a group to hold the cached geometry
	theGroup = Q3DisplayGroup_New();
	if (theGroup == nullptr)
		return(nullptr);


	// Access the TriMesh data
	geomData = e3geom_trimesh_get_geom_data( theGeom, instanceData );


	// Add the cached form to the group
	for (n = 0; n < geomData->numTriangles; n++)
		{
		// Extract the triangle
		e3geom_trimesh_triangle_new(theView, geomData, n, &triangleData);


		// Create it
		theTriangle = Q3Triangle_New(&triangleData);
		if (theTriangle != nullptr)
			Q3Group_AddObjectAndDispose(theGroup, &theTriangle);


		// Clean up
		e3geom_trimesh_triangle_delete(&triangleData);
		}



	// Finish off the group state (in-line, since we don't make any view state changes)
	Q3DisplayGroup_SetState(theGroup, kQ3DisplayGroupStateMaskIsInline  |
									  kQ3DisplayGroupStateMaskIsDrawn   |
									  kQ3DisplayGroupStateMaskIsWritten |
									  kQ3DisplayGroupStateMaskIsPicked);

	return(theGroup);
}



//=============================================================================
//      e3geom_trimesh_pick_with_ray : TriMesh ray picking method.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_trimesh_pick_with_ray( TQ3ViewObject				theView,
								TQ3PickObject			thePick,
								const TQ3Ray3D			*theRay,
								const TQ3TriMeshData	*geomData )
{	TQ3Uns32						n, numPoints, v0, v1, v2;
	TQ3Boolean						haveUV, cullBackface;
	TQ3Param2D						hitUV, *resultUV;
	TQ3BackfacingStyle				backfacingStyle;
	TQ3TriangleData					worldTriangle;
	TQ3Point3D						*worldPoints;
	TQ3Status						qd3dStatus;
	TQ3Vector3D						hitNormal;
	TQ3Point3D						hitXYZ;
	TQ3Param3D						theHit;
	TQ3BoundingBox					worldBounds;




	// Check for face tolerance
	float faceTolerance;
	E3Pick_GetFaceTolerance( thePick, &faceTolerance );
	float toleranceSquared = faceTolerance * faceTolerance;
	bool useTolerance = toleranceSquared > kQ3RealZero;


	// In case we are using face tolerance, find out whether we are doing a
	// window point pick.
	bool isWindowPointPick = (E3Pick_GetType( thePick ) == kQ3PickTypeWindowPoint);


	// If using tolerance in window space, we will need to transform from
	// world to window space.
	TQ3Matrix4x4 worldToView;
	if ( useTolerance && isWindowPointPick )
	{
		Q3Camera_GetWorldToView( E3View_AccessCamera(theView), &worldToView );
	}


	// Test whether the ray hits the bounding box, as a first approximation.
	const TQ3Matrix4x4* localToWorld = E3View_State_GetMatrixLocalToWorld(theView);
	if (useTolerance)
	{
		if (isWindowPointPick)
		{
			// If part of the TriMesh is behind the camera, then due to the
			// non-affine nature of the world to window transformation,
			// E3BoundingBox_Transform will not give us a correct window-space
			// bounding box.  I don't know how to fix that, so I will skip it.
			TQ3Matrix4x4 localToView = *localToWorld * worldToView;
			TQ3BoundingBox viewBounds;
			E3BoundingBox_Transform( &geomData->bBox, &localToView, &viewBounds );

			if (viewBounds.min.z >= 0.0f) // all behind the camera, definitely no hit
			{
				return kQ3Success;
			}
			else if (viewBounds.max.z < 0.0f) // all ahead of the camera
			{
				TQ3Point3D		localBoundCorners[8];
				E3BoundingBox_GetCorners( &geomData->bBox, localBoundCorners );
				TQ3Point2D 	windowBoundCorners[8];
				int i;
				for (i = 0; i < 8; ++i)
				{
					E3View_TransformLocalToWindow( theView, &localBoundCorners[i],
						&windowBoundCorners[i] );
				}
				TQ3Area windowBounds = E3Area_SetFromPoints2D( 8, windowBoundCorners );

				// Expand window bounds by tolerance in x and y directions.
				windowBounds.min.x -= faceTolerance;
				windowBounds.min.y -= faceTolerance;
				windowBounds.max.x += faceTolerance;
				windowBounds.max.y += faceTolerance;
				// Get the original window point from the pick.
				TQ3Point2D pickPt;
				E3WindowPointPick_GetPoint( thePick, &pickPt );
				// If the pick point is outside the window bounds, we know it is a miss.
				if ( (pickPt.x < windowBounds.min.x) ||
					(pickPt.x > windowBounds.max.x) ||
					(pickPt.y < windowBounds.min.y) ||
					(pickPt.y > windowBounds.max.y) )
				{
					return kQ3Success;
				}
			}
		}
		else	// world space test
		{
			E3BoundingBox_Transform( &geomData->bBox, localToWorld, &worldBounds );
			// expand world bounds by tolerance in each direction.
			worldBounds.min.x -= faceTolerance;
			worldBounds.min.y -= faceTolerance;
			worldBounds.min.z -= faceTolerance;
			worldBounds.max.x += faceTolerance;
			worldBounds.max.y += faceTolerance;
			worldBounds.max.z += faceTolerance;
			if (! E3Ray3D_IntersectBoundingBox( theRay, &worldBounds, nullptr ))
			{
				// The ray misses the bounds, so it misses the mesh.
				return kQ3Success;
			}
		}
	}
	else // no tolerance to worry about, look for exact hit in world space
	{
		E3BoundingBox_Transform( &geomData->bBox, localToWorld, &worldBounds );
		if (! E3Ray3D_IntersectBoundingBox( theRay, &worldBounds, nullptr ))
		{
			// The ray misses the bounds, so it misses the mesh.
			return kQ3Success;
		}
	}


	// Transform our points from local to world coordinates
	numPoints   = geomData->numPoints;
	worldPoints = (TQ3Point3D *) Q3Memory_Allocate(static_cast<TQ3Uns32>(numPoints * sizeof(TQ3Point3D)));
	if (worldPoints == nullptr)
		return(kQ3Failure);

	Q3Point3D_To3DTransformArray(geomData->points,
								 E3View_State_GetMatrixLocalToWorld(theView),
								 worldPoints,
								 numPoints,
								 sizeof(TQ3Point3D),
								 sizeof(TQ3Point3D));



	// Determine if we should cull back-facing triangles or not
	qd3dStatus   = E3View_GetBackfacingStyleState(theView, &backfacingStyle);
	cullBackface = (TQ3Boolean)(qd3dStatus == kQ3Success && backfacingStyle == kQ3BackfacingStyleRemove);



	// See if we fall within the pick
	//
	// Note we do not use any vertex/edge tolerances supplied for the pick, since
	// QD3D's blue book appears to suggest neither are used for triangles.
	bool isOrientationReversing = E3Matrix4x4_Determinant( localToWorld ) < 0.0f;
	for (n = 0; n < geomData->numTriangles && qd3dStatus == kQ3Success; ++n)
	{
		// Grab the vertex indices
		v0 = geomData->triangles[n].pointIndices[0];
		v1 = geomData->triangles[n].pointIndices[1];
		v2 = geomData->triangles[n].pointIndices[2];
		Q3_ASSERT(v0 >= 0 && v0 < geomData->numPoints);
		Q3_ASSERT(v1 >= 0 && v1 < geomData->numPoints);
		Q3_ASSERT(v2 >= 0 && v2 < geomData->numPoints);

		// An oriention-reversing transformation can interfere with backface
		// culling, so maybe flip the triangle winding to compensate.
		if (cullBackface && isOrientationReversing)
		{
			std::swap( v1, v2 );
		}

		// For convenience, name the 3 world-space corners of the triangle
		const TQ3Point3D& p0( worldPoints[v0] );
		const TQ3Point3D& p1( worldPoints[v1] );
		const TQ3Point3D& p2( worldPoints[v2] );

		// Pick the triangle
		TQ3Boolean didHit = kQ3False;
		if (useTolerance)
		{
			if (E3Ray3D_NearTriangle( *theRay,
				p0, p1, p2, cullBackface, theHit ))
			{
				TQ3Point3D triNearPt = (1.0f - theHit.u - theHit.v) * p0 +
					theHit.u * p1 + theHit.v * p2;
				TQ3Point3D rayNearPt = theRay->origin + theHit.w * theRay->direction;

				if (isWindowPointPick)
				{
					TQ3Point2D	triNearWin, rayNearWin;
					E3View_TransformWorldToWindow( theView, &triNearPt, &triNearWin );
					E3View_TransformWorldToWindow( theView, &rayNearPt, &rayNearWin );
					float winDistSq = Q3LengthSquared2D( triNearWin - rayNearWin );
					didHit = (winDistSq < toleranceSquared)? kQ3True : kQ3False;
				}
				else
				{
					float worldDistSq = Q3LengthSquared3D( rayNearPt - triNearPt );
					didHit = (worldDistSq < toleranceSquared)? kQ3True : kQ3False;
				}
			}
		}
		else // require exact hits
		{
			didHit = E3Ray3D_IntersectTriangle( *theRay,
				p0, p1, p2, cullBackface, theHit );
		}

		if (didHit)
		{
			// Create the triangle, and update the vertices to the transformed coordinates
			e3geom_trimesh_triangle_new(theView, geomData, n, &worldTriangle);
			worldTriangle.vertices[0].point = p0;
			worldTriangle.vertices[1].point = p1;
			worldTriangle.vertices[2].point = p2;


			// Obtain the XYZ, normal, and UV for the hit point. We always return an
			// XYZ and normal for the hit, however we need to cope with missing UVs.
			E3Triangle_InterpolateHit(theView,&worldTriangle, &theHit,
				&hitXYZ, &hitNormal, &hitUV, &haveUV);
			resultUV = (haveUV ? &hitUV : nullptr);


			// Record the hit
			qd3dStatus = E3Pick_RecordHit(thePick, theView, &hitXYZ, &hitNormal,
				resultUV, nullptr, &theHit, n );


			// Clean up
			e3geom_trimesh_triangle_delete(&worldTriangle);
		}
	}


	// Clean up
	Q3Memory_Free(&worldPoints);

	return(qd3dStatus);
}





//=============================================================================
//      e3geom_trimesh_find_line_point_in_area : Given two points in window
//				coordinates and an area in screen space, try to find a point
//				that is both in the line segment and the area.
//-----------------------------------------------------------------------------
static TQ3Boolean
e3geom_trimesh_find_line_point_in_area( const TQ3Area& inRect,
										const TQ3Point2D& inPtOne,
										const TQ3Point2D& inPtTwo,
										TQ3Point2D& outInArea )
{
	TQ3Point2D	startPt = inPtOne;
	TQ3Point2D	endPt = inPtTwo;
	TQ3Point2D	clipStart = startPt;
	TQ3Point2D	clipEnd = endPt;

	TQ3Boolean	doIntersect = E3Rect_ClipLine( &inRect, &clipStart, &clipEnd );

	if (doIntersect == kQ3True)
	{
		TQ3Point2D	midHit;
		Q3FastPoint2D_RRatio( &clipStart, &clipEnd, 1.0f, 1.0f, &midHit );

		// Find a number t such that midHit = t*startPt + (1-t)*endPt.
		// Equivalently, midHit = t*(startPt - endPt) + endPt,
		// or midHit - endPt = t*(startPt - endPt).
		float	t;
		TQ3Vector2D	endToMid, endToStart;
		Q3FastPoint2D_Subtract( &midHit, &endPt, &endToMid );
		Q3FastPoint2D_Subtract( &startPt, &endPt, &endToStart );
		float	wholeLen = Q3FastVector2D_Length( &endToStart );
		if (wholeLen < kQ3RealZero)
		{
			t = 0.5f;
		}
		else
		{
			t = Q3FastVector2D_Length( &endToMid ) / wholeLen;
		}

		Q3FastPoint2D_RRatio( &inPtOne, &inPtTwo, t, 1.0f - t, &outInArea );
	}

	return doIntersect;
}





//=============================================================================
//      e3geom_trimesh_find_triangle_point_in_area : Given 3 points in window
//				coordinates and an area in window space, try to find a point
//				in both the triangle and the area.
//-----------------------------------------------------------------------------
static TQ3Boolean
e3geom_trimesh_find_triangle_point_in_area( const TQ3Area& inRect,
											const TQ3Point2D& inVert1,
											const TQ3Point2D& inVert2,
											const TQ3Point2D& inVert3,
											TQ3Point2D& outInArea )
{
	TQ3Boolean	foundPoint = kQ3False;

	if (
		e3geom_trimesh_find_line_point_in_area( inRect, inVert1, inVert2, outInArea ) ||
		e3geom_trimesh_find_line_point_in_area( inRect, inVert1, inVert3, outInArea ) ||
		e3geom_trimesh_find_line_point_in_area( inRect, inVert2, inVert3, outInArea )
	)
	{
		foundPoint = kQ3True;
	}

	return foundPoint;
}





//=============================================================================
//      e3geom_trimesh_pick_with_rect : TriMesh rect picking method.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_trimesh_pick_with_rect(TQ3ViewObject				theView,
								TQ3PickObject			thePick,
								const TQ3Area			*theRect,
								const TQ3TriMeshData	*geomData)
{
	TQ3Uns32			n, numPoints, v0, v1, v2;
	TQ3Point2D			triVertices[3];
	TQ3Status			qd3dStatus;



	// Transform our points from local coordinates to window coordinates
	numPoints    = geomData->numPoints;
	if (numPoints == 0)
	{
		return kQ3Success;
	}
	E3FastArray<TQ3Point2D>	windowPoints( numPoints );

	E3View_TransformArrayLocalToWindow( theView, numPoints, geomData->points, &windowPoints[0] );



	// See if we fall within the pick
	qd3dStatus = kQ3Success;

	for (n = 0; n < geomData->numTriangles && qd3dStatus == kQ3Success; n++)
	{
		// Grab the vertex indices
		v0 = geomData->triangles[n].pointIndices[0];
		v1 = geomData->triangles[n].pointIndices[1];
		v2 = geomData->triangles[n].pointIndices[2];
		Q3_ASSERT(v0 >= 0 && v0 < geomData->numPoints);
		Q3_ASSERT(v1 >= 0 && v1 < geomData->numPoints);
		Q3_ASSERT(v2 >= 0 && v2 < geomData->numPoints);


		// Set up the 2D component of the triangle
		triVertices[0].x = windowPoints[v0].x;
		triVertices[0].y = windowPoints[v0].y;

		triVertices[1].x = windowPoints[v1].x;
		triVertices[1].y = windowPoints[v1].y;

		triVertices[2].x = windowPoints[v2].x;
		triVertices[2].y = windowPoints[v2].y;


		// See if this triangle falls within the pick
		TQ3Point2D	windowHitPt;
		if (e3geom_trimesh_find_triangle_point_in_area( *theRect, windowPoints[v0],
			windowPoints[v1], windowPoints[v2], windowHitPt ))
		{
			TQ3Point3D		worldHitPt;
			E3View_TransformWindowToWorld( theView, &windowHitPt, &worldHitPt );
			qd3dStatus = E3Pick_RecordHit(thePick, theView, &worldHitPt, nullptr,
				nullptr, nullptr, nullptr, n);
			break;
		}
	}


	return(qd3dStatus);
}



//=============================================================================
//      e3geom_trimesh_pick_screen_bounds : Get our screen bounding rect.
//-----------------------------------------------------------------------------
//		Note :	Returns a 2D bounding rect that encloses the eight vertices of
//				our bounding box when projected to the screen.
//-----------------------------------------------------------------------------
static void
e3geom_trimesh_pick_screen_bounds(TQ3ViewObject theView, const TQ3TriMeshData *geomData, TQ3Area *windowBounds)
{
	// Get the corners of the bounds.
	TQ3Point3D	theCorners[8];
	E3BoundingBox_GetCorners( &geomData->bBox, theCorners );

	// Convert the corners from local to window space.
	TQ3Point2D windowCorners[8];
	E3View_TransformArrayLocalToWindow( theView, 8, theCorners, windowCorners );

	*windowBounds = E3Area_SetFromPoints2D( 8, windowCorners );
}





//=============================================================================
//      e3geom_trimesh_pick_window_point : TriMesh window-point picking method.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_trimesh_pick_window_point(TQ3ViewObject theView, TQ3PickObject thePick, const TQ3TriMeshData *geomData)
{
	TQ3Status					qd3dStatus;
	TQ3Ray3D					theRay;



	E3View_GetRayThroughPickPoint(theView, &theRay);

	qd3dStatus = e3geom_trimesh_pick_with_ray( theView, thePick, &theRay,
			geomData );

	return(qd3dStatus);
}





//=============================================================================
//      e3geom_trimesh_record_any_xyz : Record a hit on any point in the TriMesh, in world coords.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_trimesh_record_any_xyz( TQ3ViewObject theView, TQ3PickObject thePick,
								const TQ3TriMeshData& geomData )
{
	TQ3Point3D	worldHit;
	Q3Point3D_Transform( &geomData.points[0], E3View_State_GetMatrixLocalToWorld(theView),
		&worldHit );
	TQ3Status	qd3dStatus = E3Pick_RecordHit(thePick, theView, &worldHit, nullptr,
		nullptr, nullptr, nullptr );
	return qd3dStatus;
}





//=============================================================================
//      e3geom_trimesh_pick_window_rect : TriMesh window-rect picking method.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_trimesh_pick_window_rect(TQ3ViewObject theView, TQ3PickObject thePick, const TQ3TriMeshData *geomData)
{	TQ3Area						windowBounds;
	TQ3Status					qd3dStatus = kQ3Success;
	TQ3WindowRectPickData		pickData;



	// Get the pick data
	Q3WindowRectPick_GetData(thePick, &pickData);



	// Obtain our window bounding rectangle
	e3geom_trimesh_pick_screen_bounds(theView, geomData, &windowBounds);



	// See if we fall within the pick - identifying trivial matches if we can
	if (E3Rect_ContainsRect(&windowBounds, &pickData.rect))
		e3geom_trimesh_record_any_xyz( theView, thePick, *geomData );

	else if (E3Rect_IntersectRect(&windowBounds, &pickData.rect))
		qd3dStatus = e3geom_trimesh_pick_with_rect(theView, thePick, &pickData.rect, geomData);

	return(qd3dStatus);
}





//=============================================================================
//      e3geom_trimesh_pick_world_ray : TriMesh world-ray picking method.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_trimesh_pick_world_ray(TQ3ViewObject theView, TQ3PickObject thePick, const TQ3TriMeshData *geomData)
{
	TQ3Status					qd3dStatus;
	TQ3Ray3D					pickRay;



	// Get the pick data
	E3WorldRayPick_GetRay(thePick, &pickRay);



	qd3dStatus = e3geom_trimesh_pick_with_ray( theView, thePick,
			&pickRay, geomData );


	return(qd3dStatus);
}





//=============================================================================
//      e3geom_trimesh_pick : TriMesh picking method.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_trimesh_pick(TQ3ViewObject theView, TQ3ObjectType objectType, TQ3Object theObject, const void *objectData)
{
#pragma unused( objectType )
	TQ3Status				qd3dStatus;
	const TQ3TriMeshData	*geomData;
	TQ3PickObject			thePick;



	// Get the geometry data
	geomData = e3geom_trimesh_get_geom_data(theObject, objectData);
	Q3_ASSERT(geomData->bBox.isEmpty == kQ3False);



	// Handle the pick
	thePick = E3View_AccessPick(theView);
	switch (Q3Pick_GetType(thePick)) {
		case kQ3PickTypeWindowPoint:
			qd3dStatus = e3geom_trimesh_pick_window_point(theView, thePick, geomData);
			break;

		case kQ3PickTypeWindowRect:
			qd3dStatus = e3geom_trimesh_pick_window_rect(theView, thePick, geomData);
			break;

		case kQ3PickTypeWorldRay:
			qd3dStatus = e3geom_trimesh_pick_world_ray(theView, thePick, geomData);
			break;

		default:
			qd3dStatus = kQ3Failure;
			break;
		}

	return(qd3dStatus);
}





//=============================================================================
//      e3geom_trimesh_bounds : TriMesh bounds method.
//-----------------------------------------------------------------------------
static TQ3Status
e3geom_trimesh_bounds(TQ3ViewObject theView, TQ3ObjectType objectType,
					TQ3Object theObject, const void *objectData)
{	TQ3Point3D					boundCorners[8];
	TQ3BoundingMethod			boundingMethod;
	const TQ3TriMeshData		*geomData;
#pragma unused(objectType)



	// Get the geometry data
	geomData = e3geom_trimesh_get_geom_data(theObject, objectData);
	Q3_ASSERT(geomData->bBox.isEmpty == kQ3False);



	// Calculate the exact bounds from our points
	boundingMethod = E3View_GetBoundingMethod(theView);
	if (boundingMethod == kQ3BoxBoundsExact || boundingMethod == kQ3SphereBoundsExact)
		E3View_UpdateBounds(theView, geomData->numPoints, sizeof(TQ3Point3D), geomData->points);



	// And our approximate bounds from our bounding box
	//
	// In local coordinates, taking the bounding box of the min and max points does recreate
	// the bounding box. However, even in "approximate" mode, E3View_UpdateBounds uses the
	// exact method for a bounding box of a small numbers of points, hence the min and max
	// points would be converted to world coordinates before taking a bounding box.
	//
	// This can give a bad result, even an empty bounding box. To prevent this, we take
	// the bounds of our eight corners and not just the two min/max points.
	else
		{
		E3BoundingBox_GetCorners( &geomData->bBox, boundCorners );
		E3View_UpdateBounds(theView, 8, sizeof(TQ3Point3D), boundCorners);
		}

	return(kQ3Success);
}





//=============================================================================
//      e3geom_trimesh_get_attribute : TriMesh get attribute set pointer.
//-----------------------------------------------------------------------------
static TQ3AttributeSet *
e3geom_trimesh_get_attribute ( TQ3Object triMesh )
{
	// Return the address of the geometry attribute set
	return & ((E3TriMesh*)triMesh)->instanceData.geomAttributeSet ;
}





//=============================================================================
//      e3geom_trimesh_get_public_data : TriMesh get public data pointer.
//-----------------------------------------------------------------------------
static const void *
e3geom_trimesh_get_public_data ( TQ3Object inTriMesh )
{
	E3TriMesh* triMesh = (E3TriMesh*) inTriMesh;
	E3NakedTriMesh* nakedTriMesh = triMesh->instanceData.nakedTriMesh;

	// For compatibility with code that does not know about the outer/naked
	// division, we want to return a pointer to TQ3TriMeshData, but containing
	// the attribute set from the outer level.
	E3Shared_Replace( &nakedTriMesh->instanceData.geomData.triMeshAttributeSet,
		triMesh->instanceData.geomAttributeSet );

	// Return the address of the geometry public data
	return & triMesh->instanceData.nakedTriMesh->instanceData.geomData;
}





//=============================================================================
//      e3geom_trimesh_metahandler : TriMesh metahandler.
//-----------------------------------------------------------------------------
static TQ3XFunctionPointer
e3geom_trimesh_metahandler(TQ3XMethodType methodType)
{	TQ3XFunctionPointer		theMethod = nullptr;



	// Return our methods
	switch (methodType) {
		case kQ3XMethodTypeObjectNew:
			theMethod = (TQ3XFunctionPointer) e3geom_trimesh_new;
			break;

		case kQ3XMethodTypeObjectDelete:
			theMethod = (TQ3XFunctionPointer) e3geom_trimesh_delete;
			break;

		case kQ3XMethodTypeObjectDuplicate:
			theMethod = (TQ3XFunctionPointer) e3geom_trimesh_duplicate;
			break;

		case kQ3XMethodTypeGeomCacheNew:
			theMethod = (TQ3XFunctionPointer) e3geom_trimesh_cache_new;
			break;

		case kQ3XMethodTypeObjectSubmitPick:
			theMethod = (TQ3XFunctionPointer) e3geom_trimesh_pick;
			break;

		case kQ3XMethodTypeObjectSubmitBounds:
			theMethod = (TQ3XFunctionPointer) e3geom_trimesh_bounds;
			break;

		case kQ3XMethodTypeGeomGetAttribute:
			theMethod = (TQ3XFunctionPointer) e3geom_trimesh_get_attribute;
			break;

		case kQ3XMethodTypeGeomGetPublicData:
			theMethod = (TQ3XFunctionPointer) e3geom_trimesh_get_public_data;
			break;

		case kQ3XMethodTypeGeomUsesOrientation:
			theMethod = (TQ3XFunctionPointer) kQ3True;
			break;
		}

	return(theMethod);
}




//=============================================================================
//      e3geom_nakedtrimesh_metahandler : TriMesh metahandler.
//-----------------------------------------------------------------------------
static TQ3XFunctionPointer
e3geom_nakedtrimesh_metahandler(TQ3XMethodType methodType)
{
	TQ3XFunctionPointer		theMethod = nullptr;

	switch (methodType)
	{
		case kQ3XMethodTypeObjectNew:
			theMethod = (TQ3XFunctionPointer) e3geom_nakedtrimesh_new;
			break;

		case kQ3XMethodTypeObjectDelete:
			theMethod = (TQ3XFunctionPointer) e3geom_nakedtrimesh_delete;
			break;

		case kQ3XMethodTypeObjectDuplicate:
			theMethod = (TQ3XFunctionPointer) e3geom_nakedtrimesh_duplicate;
			break;
	}

	return(theMethod);
}




//=============================================================================
//      Public functions
//-----------------------------------------------------------------------------
//      E3GeometryTriMesh_RegisterClass : Register the class.
//-----------------------------------------------------------------------------
#pragma mark -
TQ3Status
E3GeometryTriMesh_RegisterClass(void)
{
	// Register the class
	TQ3Status status = Q3_REGISTER_CLASS (	kQ3ClassNameGeometryTriMesh,
								e3geom_trimesh_metahandler,
								E3TriMesh );

	if (status == kQ3Success)
	{
		status = Q3_REGISTER_CLASS (	kQ3ClassNameGeometryNakedTriMesh,
								e3geom_nakedtrimesh_metahandler,
								E3NakedTriMesh );
	}

	return status;
}





//=============================================================================
//      E3GeometryTriMesh_UnregisterClass : Unregister the class.
//-----------------------------------------------------------------------------
TQ3Status
E3GeometryTriMesh_UnregisterClass(void)
{	TQ3Status		qd3dStatus;



	// Unregister the class
	qd3dStatus = E3ClassTree::UnregisterClass(kQ3GeometryTypeTriMesh, kQ3True);
	E3ClassTree::UnregisterClass(kQ3GeometryTypeNakedTriMesh, kQ3True);

	return(qd3dStatus);
}





//=============================================================================
//      E3TriMesh_New : Create a TriMesh object.
//-----------------------------------------------------------------------------
#pragma mark -
TQ3GeometryObject
E3TriMesh_New(const TQ3TriMeshData *triMeshData)
{	TQ3Object		theObject;



	// Create the object
	theObject = E3ClassTree::CreateInstance ( kQ3GeometryTypeTriMesh, kQ3False, triMeshData);
	return(theObject);
}




TQ3GeometryObject
E3TriMesh_New_NoCopy(const TQ3TriMeshData *triMeshData)
{
	TQ3Object		theObject;

	E3Root* outerClass = (E3Root*) E3ClassTree::GetClass( kQ3GeometryTypeTriMesh );
	E3Root* innerClass = (E3Root*) E3ClassTree::GetClass( kQ3GeometryTypeNakedTriMesh );
	if ( (outerClass == nullptr) || (innerClass == nullptr) )
	{
		E3ErrorManager_PostWarning ( kQ3WarningTypeHasNotBeenRegistered ) ;

		if ( ! Q3IsInitialized () )
			E3ErrorManager_PostError ( kQ3ErrorNotInitialized, kQ3False ) ;

		return nullptr ;
	}

	innerClass->newMethod = (TQ3XObjectNewMethod) e3geom_nakedtrimesh_new_nocopy;

	theObject = outerClass->CreateInstance( kQ3False, triMeshData );

	innerClass->newMethod = (TQ3XObjectNewMethod) e3geom_nakedtrimesh_new;

	return theObject;
}



//=============================================================================
//      E3TriMesh_Submit : Submit a TriMesh.
//-----------------------------------------------------------------------------
TQ3Status
E3TriMesh_Submit(const TQ3TriMeshData *triMeshData, TQ3ViewObject theView)
{	TQ3Status		qd3dStatus;



	// Submit the geometry
	qd3dStatus = E3View_SubmitImmediate(theView, kQ3GeometryTypeTriMesh, triMeshData);
	return(qd3dStatus);
}





//=============================================================================
//      E3TriMesh_SetData : Set the data for a TriMesh object.
//-----------------------------------------------------------------------------
TQ3Status
E3TriMesh_SetData(TQ3GeometryObject theTriMesh, const TQ3TriMeshData *triMeshData)
{
	E3TriMesh* triMesh = (E3TriMesh*) theTriMesh;


	// If the naked TriMesh is referenced elsewhere, duplicate it, giving copy-on-write behavior.
	if (triMesh->instanceData.nakedTriMesh->IsReferenced())
	{
		E3NakedTriMesh* oldNakedTriMesh = triMesh->instanceData.nakedTriMesh;
		E3NakedTriMesh* newNakedTriMesh = (E3NakedTriMesh*) oldNakedTriMesh->DuplicateInstance();
		triMesh->instanceData.nakedTriMesh = newNakedTriMesh;
		E3Shared_Dispose( oldNakedTriMesh );
	}


	// Dispose of the existing data
	e3geom_trimesh_disposedata( & triMesh->instanceData.nakedTriMesh->instanceData.geomData );



	// Copy the new TriMesh data, then optimize it
	TQ3Status qd3dStatus = e3geom_nakedtrimesh_copydata( triMeshData,
		& triMesh->instanceData.nakedTriMesh->instanceData.geomData );

	E3Shared_Replace( &triMesh->instanceData.geomAttributeSet,
		triMeshData->triMeshAttributeSet );


	if (qd3dStatus == kQ3Success)
	{
		qd3dStatus = e3geom_trimesh_validate(
			&triMesh->instanceData.nakedTriMesh->instanceData.geomData );
	}

	if ( qd3dStatus != kQ3Failure )
	{
		e3geom_trimesh_optimize(
			& triMesh->instanceData.nakedTriMesh->instanceData.geomData );
	}

	Q3Shared_Edited ( triMesh );
	Q3Shared_Edited ( triMesh->instanceData.nakedTriMesh );

	return qd3dStatus;
}





//=============================================================================
//      E3TriMesh_GetData : Return the data for a TriMesh object.
//-----------------------------------------------------------------------------
TQ3Status
E3TriMesh_GetData(TQ3GeometryObject theTriMesh, TQ3TriMeshData *triMeshData)
{
	E3TriMesh* triMesh = (E3TriMesh*) theTriMesh ;

	// Copy the data out of the TriMesh
	TQ3Status qd3dStatus = e3geom_nakedtrimesh_copydata(
		& triMesh->instanceData.nakedTriMesh->instanceData.geomData, triMeshData );

	Q3_ASSERT( triMeshData->triMeshAttributeSet == nullptr );
	Q3_ASSERT( (triMesh->instanceData.geomAttributeSet == nullptr) ||
		Q3_OBJECT_IS_CLASS( triMesh->instanceData.geomAttributeSet, E3Shared ) );

	E3Shared_Replace( &triMeshData->triMeshAttributeSet,
		triMesh->instanceData.geomAttributeSet );

	return qd3dStatus ;
}





//=============================================================================
//      E3TriMesh_EmptyData : Empty the data for a TriMesh object.
//-----------------------------------------------------------------------------
TQ3Status
E3TriMesh_EmptyData(TQ3TriMeshData *triMeshData)
{


	// Dispose of the data
	e3geom_trimesh_disposedata(triMeshData);

	return(kQ3Success);
}





//=============================================================================
//      E3TriMesh_LockData : Lock a TriMesh for direct access.
//-----------------------------------------------------------------------------
//		Note :	Our current implementation does not require locking, and so we
//				can simply return a pointer to the TriMesh instance data.
//
//				We will eventually be able to move responsibility for geometry
//				data down to the renderer objects, at which time we will need
//				to keep a flag indicating if a TriMesh is locked.
//
//				We will then need to update the renderer object after a TriMesh
//				is unlocked after being locked without read-only access.
//-----------------------------------------------------------------------------
TQ3Status
E3TriMesh_LockData(TQ3GeometryObject theTriMesh, TQ3Boolean readOnly, TQ3TriMeshData **triMeshData)
{
	E3TriMesh* triMesh = (E3TriMesh*) theTriMesh ;
	E3NakedTriMesh* nakedTriMesh = triMesh->instanceData.nakedTriMesh;



	// If the TriMesh was already locked,
	// then this lock had better be read-only, lest the code that did the outer
	// lock get confused.
	Q3_ASSERT( (nakedTriMesh->instanceData.lockCount == 0) || readOnly );



	// If we are locking with write access, then the naked TriMesh had better not be
	// shared, so that naked TriMeshes will work in a copy-on-write way.
	if ( (! readOnly) and nakedTriMesh->IsReferenced() )
	{
		E3NakedTriMesh* newNakedTriMesh = (E3NakedTriMesh*) nakedTriMesh->DuplicateInstance();
		triMesh->instanceData.nakedTriMesh = newNakedTriMesh;
		E3Shared_Dispose( nakedTriMesh );
		Q3_ASSERT( newNakedTriMesh->GetReferenceCount() == 1 );
		nakedTriMesh = newNakedTriMesh;
	}



	// Lock the TriMesh
	if ( readOnly && (nakedTriMesh->instanceData.lockCount == 0) )
		E3Bit_Set( nakedTriMesh->instanceData.theFlags, kTriMeshLockedReadOnly ) ;
	nakedTriMesh->instanceData.lockCount += 1;



	// Return a pointer to the TriMesh data
	E3Shared_Replace( &nakedTriMesh->instanceData.geomData.triMeshAttributeSet,
		triMesh->instanceData.geomAttributeSet );

	*triMeshData = & nakedTriMesh->instanceData.geomData ;

	return kQ3Success ;
}





//=============================================================================
//      E3TriMesh_UnlockData : Unlock a TriMesh after direct access.
//-----------------------------------------------------------------------------
TQ3Status
E3TriMesh_UnlockData(TQ3GeometryObject theTriMesh)
{
	TQ3Status	theStatus = kQ3Success;
	E3TriMesh* triMesh = (E3TriMesh*) theTriMesh ;
	E3NakedTriMesh* nakedTriMesh = triMesh->instanceData.nakedTriMesh;



	// Unlock the TriMesh
	nakedTriMesh->instanceData.lockCount -= 1;
	if (nakedTriMesh->instanceData.lockCount == 0)
	{
		// If the TriMesh was mutable, assume it needs updating
		if ( ! E3Bit_IsSet( nakedTriMesh->instanceData.theFlags, kTriMeshLockedReadOnly ) )
		{
			theStatus = e3geom_trimesh_validate( &nakedTriMesh->instanceData.geomData );

			// Re-optimize the TriMesh
			e3geom_trimesh_optimize ( & nakedTriMesh->instanceData.geomData ) ;


			// Bump the edit index
			Q3Shared_Edited ( nakedTriMesh ) ;
			Q3Shared_Edited ( triMesh ) ;
		}


		E3Bit_Clear( nakedTriMesh->instanceData.theFlags, kTriMeshLockedReadOnly ) ;
	}

	return theStatus;
}





//=============================================================================
//      E3TriMesh_AddTriangleNormals : Add triangle normals to a TriMesh.
//-----------------------------------------------------------------------------
void
E3TriMesh_AddTriangleNormals(TQ3GeometryObject theTriMesh, TQ3OrientationStyle theOrientation)
	{
	E3TriMesh* triMesh = (E3TriMesh*) theTriMesh ;
	E3NakedTriMesh* nakedTriMesh = triMesh->instanceData.nakedTriMesh;

	// Validate our parameters
	Q3_ASSERT_VALID_PTR(triMesh);



	// Do nothing if we already have triangle normals
	TQ3TriMeshAttributeData* attributeData = e3geom_trimesh_attribute_find (
												  nakedTriMesh->instanceData.geomData.numTriangleAttributeTypes,
												  nakedTriMesh->instanceData.geomData.triangleAttributeTypes,
												  kQ3AttributeTypeNormal ) ;
	if ( attributeData != nullptr )
		return ;



	// Allocate the normals
	TQ3Uns32 theSize    = static_cast<TQ3Uns32>(
		nakedTriMesh->instanceData.geomData.numTriangles * sizeof ( TQ3Vector3D ));
	TQ3Vector3D* theNormals = (TQ3Vector3D *) Q3Memory_Allocate ( theSize ) ;
	TQ3Status qd3dStatus = ( theNormals != nullptr ? kQ3Success : kQ3Failure ) ;



	// Append another triangle attribute
	if ( qd3dStatus != kQ3Failure )
		{
		theSize    = static_cast<TQ3Uns32>(
			( nakedTriMesh->instanceData.geomData.numTriangleAttributeTypes + 1 ) *
			sizeof ( TQ3TriMeshAttributeData ));
		qd3dStatus = Q3Memory_Reallocate( & nakedTriMesh->instanceData.geomData.triangleAttributeTypes, theSize ) ;
		if ( qd3dStatus != kQ3Failure )
			{
			attributeData = & nakedTriMesh->instanceData.geomData.triangleAttributeTypes [ nakedTriMesh->instanceData.geomData.numTriangleAttributeTypes ] ;
			nakedTriMesh->instanceData.geomData.numTriangleAttributeTypes++ ;
			}
		}



	// Calculate the normals
	if ( qd3dStatus != kQ3Failure )
		{
		// Set up the attribute
		attributeData->data              = theNormals ;
		attributeData->attributeType     = kQ3AttributeTypeNormal ;
		attributeData->attributeUseArray = nullptr ;



		// Calculate the normals in CCW form
		TQ3Point3D* thePoints = nakedTriMesh->instanceData.geomData.points;
		if (thePoints != nullptr)
		{
			Q3Triangle_CrossProductArray ( nakedTriMesh->instanceData.geomData.numTriangles, nullptr,
										 nakedTriMesh->instanceData.geomData.triangles[0].pointIndices,
										 thePoints,
										 theNormals ) ;
		}


		// Reverse them if our orientation is CW
		if ( theOrientation == kQ3OrientationStyleClockwise )
			{
			for ( TQ3Uns32 n = 0 ; n < nakedTriMesh->instanceData.geomData.numTriangles ; ++n )
				Q3Vector3D_Negate ( & theNormals [ n ], &theNormals [ n ] ) ;
			}
		}



	// Clean up
	Q3Shared_Edited ( triMesh ) ;

	if ( qd3dStatus == kQ3Failure )
		Q3Memory_Free( &theNormals ) ;
	}


TQ3GeometryObject	E3TriMesh_GetNakedGeometry( TQ3GeometryObject inGeom )
{
	E3TriMesh* triMesh = (E3TriMesh*) inGeom;
	E3NakedTriMesh* nakedTriMesh = triMesh->instanceData.nakedTriMesh;
	return nakedTriMesh->GetReference();
}

void	E3TriMesh_SetNakedGeometry( TQ3GeometryObject inTriMesh,
									TQ3GeometryObject inNaked )
{
	E3TriMesh* triMesh = (E3TriMesh*) inTriMesh;
	E3Shared_Replace( (TQ3Object*) &triMesh->instanceData.nakedTriMesh, inNaked );
}
