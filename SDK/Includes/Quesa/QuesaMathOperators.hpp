/*! @header QuesaMathOperators.hpp

        Declares C++ operators for convenience in Quesa math.
*/
/*  NAME:
        QuesaMathOperators.hpp

    DESCRIPTION:
        Quesa public header.

    COPYRIGHT:
        Copyright (c) 1999-2022, Quesa Developers. All rights reserved.

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
#ifndef QUESA_MATH_OPS_HDR
#define QUESA_MATH_OPS_HDR

//=============================================================================
//      Include files
//-----------------------------------------------------------------------------
#include "QuesaMath.h"


//=============================================================================
//      Unary minus
//-----------------------------------------------------------------------------

// vector = - vector (unary minus)
inline TQ3Vector3D operator-( const TQ3Vector3D& inVec )
{
	TQ3Vector3D result;
	Q3FastVector3D_Negate( &inVec, &result );
	return result;
}

// vector = - vector (unary minus)
inline TQ3Vector2D operator-( const TQ3Vector2D& inVec )
{
	TQ3Vector2D result;
	Q3FastVector2D_Negate( &inVec, &result );
	return result;
}

//=============================================================================
//      Scalar Multiplication
//-----------------------------------------------------------------------------

// vector = scalar * vector
inline TQ3Vector3D operator*( float inScalar, const TQ3Vector3D& inVec )
{
	TQ3Vector3D result;
	Q3FastVector3D_Scale( &inVec, inScalar, &result );
	return result;
}

// vector = scalar * vector
inline TQ3Vector2D operator*( float inScalar, const TQ3Vector2D& inVec )
{
	TQ3Vector2D result;
	Q3FastVector2D_Scale( &inVec, inScalar, &result );
	return result;
}

// vector *= scalar
inline TQ3Vector3D& operator*=( TQ3Vector3D& ioA, float inScalar )
{
	Q3FastVector3D_Scale( &ioA, inScalar, &ioA );
	return ioA;
}

// vector *= scalar
inline TQ3Vector2D& operator*=( TQ3Vector2D& ioA, float inScalar )
{
	Q3FastVector2D_Scale( &ioA, inScalar, &ioA );
	return ioA;
}

// uv = scalar * uv
inline TQ3Param2D operator*( float inScalar, const TQ3Param2D& inUV )
{
	TQ3Param2D result = {
		inScalar * inUV.u, inScalar * inUV.v
	};
	return result;
}

// uv = uv + uv
inline TQ3Param2D operator+( const TQ3Param2D& inA, const TQ3Param2D& inB )
{
	TQ3Param2D result = {
		inA.u + inB.u, inA.v + inB.v
	};
	return result;
}


// point = scalar * point (not usual in math, but useful in 3D computing)
inline TQ3Point3D operator*( float inScalar, const TQ3Point3D& inVec )
{
	TQ3Point3D result;
	Q3FastVector3D_Scale( (const TQ3Vector3D*)&inVec, inScalar, (TQ3Vector3D*)&result );
	return result;
}

// point = scalar * point (not usual in math, but useful in 3D computing)
inline TQ3Point2D operator*( float inScalar, const TQ3Point2D& inVec )
{
	TQ3Point2D result;
	Q3FastVector2D_Scale( (const TQ3Vector2D*)&inVec, inScalar, (TQ3Vector2D*)&result );
	return result;
}


//=============================================================================
//      Additive Operations
//-----------------------------------------------------------------------------

// pt = pt + vector
inline TQ3Point3D operator+( const TQ3Point3D& inPt, const TQ3Vector3D& inVec )
{
	TQ3Point3D theSum;
	Q3FastPoint3D_Vector3D_Add( &inPt, &inVec, &theSum );
	return theSum;
}

// pt = pt + vector [2D]
inline TQ3Point2D operator+( const TQ3Point2D& inPt, const TQ3Vector2D& inVec )
{
	TQ3Point2D theSum;
	Q3FastPoint2D_Vector2D_Add( &inPt, &inVec, &theSum );
	return theSum;
}

// pt = pt - vector
inline TQ3Point3D operator-( const TQ3Point3D& inPt, const TQ3Vector3D& inVec )
{
	TQ3Point3D theSum;
	Q3FastPoint3D_Vector3D_Subtract( &inPt, &inVec, &theSum );
	return theSum;
}

// pt = pt - vector [2D]
inline TQ3Point2D operator-( const TQ3Point2D& inPt, const TQ3Vector2D& inVec )
{
	TQ3Point2D theSum;
	Q3FastPoint2D_Vector2D_Subtract( &inPt, &inVec, &theSum );
	return theSum;
}

// vector = pt - pt
inline TQ3Vector3D operator-( const TQ3Point3D& inA, const TQ3Point3D& inB )
{
	TQ3Vector3D result;
	Q3FastPoint3D_Subtract( &inA, &inB, &result );
	return result;
}

// vector = pt - pt [2D]
inline TQ3Vector2D operator-( const TQ3Point2D& inA, const TQ3Point2D& inB )
{
	TQ3Vector2D result;
	Q3FastPoint2D_Subtract( &inA, &inB, &result );
	return result;
}

// vector = vector + vector
inline TQ3Vector3D operator+( const TQ3Vector3D& inA, const TQ3Vector3D& inB )
{
	TQ3Vector3D result;
	Q3FastVector3D_Add( &inA, &inB, &result );
	return result;
}

// vector = vector + vector [2D]
inline TQ3Vector2D operator+( const TQ3Vector2D& inA, const TQ3Vector2D& inB )
{
	TQ3Vector2D result;
	Q3FastVector2D_Add( &inA, &inB, &result );
	return result;
}

// vector = vector - vector
inline TQ3Vector3D operator-( const TQ3Vector3D& inA, const TQ3Vector3D& inB )
{
	TQ3Vector3D result;
	Q3FastVector3D_Subtract( &inA, &inB, &result );
	return result;
}

// vector = vector - vector [2D]
inline TQ3Vector2D operator-( const TQ3Vector2D& inA, const TQ3Vector2D& inB )
{
	TQ3Vector2D result;
	Q3FastVector2D_Subtract( &inA, &inB, &result );
	return result;
}

// vector += vector
inline TQ3Vector3D& operator+=( TQ3Vector3D& ioA, const TQ3Vector3D& inB )
{
	Q3FastVector3D_Add( &ioA, &inB, &ioA );
	return ioA;
}

// vector -= vector
inline TQ3Vector3D& operator-=( TQ3Vector3D& ioA, const TQ3Vector3D& inB )
{
	Q3FastVector3D_Subtract( &ioA, &inB, &ioA );
	return ioA;
}

// pt += vector
inline TQ3Point3D& operator+=( TQ3Point3D& ioA, const TQ3Vector3D& inB )
{
	Q3FastPoint3D_Vector3D_Add( &ioA, &inB, &ioA );
	return ioA;
}

// pt -= vector
inline TQ3Point3D& operator-=( TQ3Point3D& ioA, const TQ3Vector3D& inB )
{
	Q3FastPoint3D_Vector3D_Subtract( &ioA, &inB, &ioA );
	return ioA;
}

// pt = pt + pt (useful for weighted averages)
inline TQ3Point3D operator+( const TQ3Point3D& inA, const TQ3Point3D& inB )
{
	TQ3Point3D result;
	Q3FastVector3D_Add( (const TQ3Vector3D*)&inA, (const TQ3Vector3D*)&inB, (TQ3Vector3D*)&result );
	return result;
}

// pt = pt + pt (useful for weighted averages) [2D]
inline TQ3Point2D operator+( const TQ3Point2D& inA, const TQ3Point2D& inB )
{
	TQ3Point2D result;
	Q3FastVector2D_Add( (const TQ3Vector2D*)&inA, (const TQ3Vector2D*)&inB, (TQ3Vector2D*)&result );
	return result;
}


//=============================================================================
//      Matrix Operations
//-----------------------------------------------------------------------------

// matrix * matrix
inline TQ3Matrix4x4 operator*( const TQ3Matrix4x4& inMat1, const TQ3Matrix4x4& inMat2 )
{
	TQ3Matrix4x4 result;
	Q3Matrix4x4_Multiply( &inMat1, &inMat2, &result );
	return result;
}

// matrix *= matrix;
inline TQ3Matrix4x4& operator*=( TQ3Matrix4x4& ioMat, const TQ3Matrix4x4& inMat2 )
{
	Q3Matrix4x4_Multiply( &ioMat, &inMat2, &ioMat );
	return ioMat;
}

// pt * matrix (transform point)
inline TQ3Point3D operator*( const TQ3Point3D& inPt, const TQ3Matrix4x4& inMat )
{
	TQ3Point3D result;
	Q3Point3D_Transform( &inPt, &inMat, &result );
	return result;
}

// pt * matrix (transform rational point)
inline TQ3RationalPoint4D operator*( const TQ3RationalPoint4D& inPt, const TQ3Matrix4x4& inMat )
{
	TQ3RationalPoint4D result;
	Q3RationalPoint4D_Transform( &inPt, &inMat, &result );
	return result;
}

// pt *= matrix (transform point)
inline TQ3Point3D& operator*=( TQ3Point3D& ioPt, const TQ3Matrix4x4& inMat )
{
	Q3Point3D_Transform( &ioPt, &inMat, &ioPt );
	return ioPt;
}

// vector * matrix (transform vector)
inline TQ3Vector3D operator*( const TQ3Vector3D& inVec, const TQ3Matrix4x4& inMat )
{
	TQ3Vector3D result;
	Q3Vector3D_Transform( &inVec, &inMat, &result );
	return result;
}

// vector *= matrix (transform vector)
inline TQ3Vector3D& operator*=( TQ3Vector3D& ioVec, const TQ3Matrix4x4& inMat )
{
	Q3Vector3D_Transform( &ioVec, &inMat, &ioVec );
	return ioVec;
}

// Exact equality (is this what you really want?) of 4x4 matrices
inline bool operator==( const TQ3Matrix4x4& one, const TQ3Matrix4x4 two )
{
	return
		(one.value[0][0] == two.value[0][0]) &&
		(one.value[0][1] == two.value[0][1]) &&
		(one.value[0][2] == two.value[0][2]) &&
		(one.value[0][3] == two.value[0][3]) &&
		(one.value[1][0] == two.value[1][0]) &&
		(one.value[1][1] == two.value[1][1]) &&
		(one.value[1][2] == two.value[1][2]) &&
		(one.value[1][3] == two.value[1][3]) &&
		(one.value[2][0] == two.value[2][0]) &&
		(one.value[2][1] == two.value[2][1]) &&
		(one.value[2][2] == two.value[2][2]) &&
		(one.value[2][3] == two.value[2][3]) &&
		(one.value[3][0] == two.value[3][0]) &&
		(one.value[3][1] == two.value[3][1]) &&
		(one.value[3][2] == two.value[3][2]) &&
		(one.value[3][3] == two.value[3][3]);
	;
}

// Exact inequality (is this what you really want?) of 4x4 matrices
inline bool operator!=( const TQ3Matrix4x4& one, const TQ3Matrix4x4 two )
{
	return !(one == two);
}

//=============================================================================
//      Common Functions
//-----------------------------------------------------------------------------

inline TQ3RationalPoint4D Q3ToRational4D( const TQ3Point3D& inPt )
{
	TQ3RationalPoint4D result = { inPt.x, inPt.y, inPt.z, 1.0f };
	return result;
}

inline TQ3RationalPoint4D Q3ToRational4D( const TQ3Vector3D& inVec )
{
	TQ3RationalPoint4D result = { inVec.x, inVec.y, inVec.z, 0.0f };
	return result;
}

inline TQ3Vector3D Q3Cross3D( const TQ3Vector3D& inA, const TQ3Vector3D& inB )
{
	TQ3Vector3D result;
	Q3FastVector3D_Cross( &inA, &inB, &result );
	return result;
}

inline float Q3Dot3D( const TQ3Vector3D& inA, const TQ3Vector3D& inB )
{
	return Q3FastVector3D_Dot( &inA, &inB );
}

inline float Q3Dot2D( const TQ3Vector2D& inA, const TQ3Vector2D& inB )
{
	return Q3FastVector2D_Dot( &inA, &inB );
}

inline TQ3Vector3D Q3Normalize3D( const TQ3Vector3D& inVec )
{
	TQ3Vector3D result;
	Q3FastVector3D_Normalize( &inVec, &result );
	return result;
}

inline TQ3Vector2D Q3Normalize2D( const TQ3Vector2D& inVec )
{
	TQ3Vector2D result;
	Q3FastVector2D_Normalize( &inVec, &result );
	return result;
}

inline float Q3Length3D( const TQ3Vector3D& inVec )
{
	return Q3FastVector3D_Length( &inVec );
}

inline float Q3LengthSquared3D( const TQ3Vector3D& inVec )
{
	return Q3FastVector3D_LengthSquared( &inVec );
}

inline float Q3Length2D( const TQ3Vector2D& inVec )
{
	return Q3FastVector2D_Length( &inVec );
}

inline float Q3LengthSquared2D( const TQ3Vector2D& inVec )
{
	return Q3FastVector2D_LengthSquared( &inVec );
}

inline TQ3Matrix4x4 Q3Invert( const TQ3Matrix4x4& inMtx )
{
	TQ3Matrix4x4 result;
	Q3Matrix4x4_Invert( &inMtx, &result );
	return result;
}

inline TQ3Point3D Q3VectorToPoint3D( const TQ3Vector3D& inVec )
{
	return TQ3Point3D{ inVec.x, inVec.y, inVec.z };
}

inline TQ3Vector3D Q3PointToVector3D( const TQ3Point3D& inPt )
{
	return TQ3Vector3D{ inPt.x, inPt.y, inPt.z };
}

#endif
