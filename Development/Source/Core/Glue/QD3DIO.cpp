/*  NAME:
        QD3DIO.c

    DESCRIPTION:
        Entry point for Quesa API calls. Performs parameter checking and
        then forwards each API call to the equivalent E3xxxxx routine.

    COPYRIGHT:
        Copyright (c) 1999-2014, Quesa Developers. All rights reserved.

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
#include "E3IO.h"
#include "E3IOData.h"
#include "E3IOFileFormat.h"
#include "E3FFW_3DMFBin_Writer.h"
#include "E3View.h"





//=============================================================================
//      Internal constants
//-----------------------------------------------------------------------------
// Internal constants go here





//=============================================================================
//      Internal types
//-----------------------------------------------------------------------------
// Internal types go here





//=============================================================================
//      Internal macros
//-----------------------------------------------------------------------------
// Internal macros go here





//=============================================================================
//      Public functions
//-----------------------------------------------------------------------------
//      Q3XView_SubmitWriteData : Quesa API entry point.
//-----------------------------------------------------------------------------
#pragma mark -
TQ3Status
Q3XView_SubmitWriteData(TQ3ViewObject view, TQ3Size size, void *data, TQ3XDataDeleteMethod deleteData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3View_IsOfMyClass ( view ), kQ3Failure);



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3XView_SubmitWriteData(view, size, data, deleteData));
}





//=============================================================================
//      Q3XView_SubmitSubObjectData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3XView_SubmitSubObjectData(TQ3ViewObject view, TQ3XObjectClass objectClass, TQ3Uns32 size, void *data, TQ3XDataDeleteMethod deleteData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3View_IsOfMyClass ( view ), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3XView_SubmitSubObjectData(view, objectClass, size, data, deleteData));
}





//=============================================================================
//      Q3File_New : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3FileObject
Q3File_New(void)
{


	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3File_New());
}





//=============================================================================
//      Q3File_GetStorage : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3File_GetStorage(TQ3FileObject theFile, TQ3StorageObject *storage)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(storage), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->GetStorage ( storage ) ;
}





//=============================================================================
//      Q3File_SetStorage : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3File_SetStorage(TQ3FileObject theFile, TQ3StorageObject storage)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);
	// storage can be nullptr


	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->SetStorage ( storage ) ;
}





//=============================================================================
//      Q3File_OpenRead : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3File_OpenRead(TQ3FileObject theFile, TQ3FileMode *mode)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->OpenRead ( mode ) ;
}





//=============================================================================
//      Q3File_OpenWrite : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3File_OpenWrite(TQ3FileObject theFile, TQ3FileMode mode)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->OpenWrite ( mode ) ;
}





//=============================================================================
//      Q3File_IsOpen : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3File_IsOpen(TQ3FileObject theFile, TQ3Boolean *isOpen)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(isOpen), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->IsOpen ( isOpen ) ;
}





//=============================================================================
//      Q3File_GetMode : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3File_GetMode(TQ3FileObject theFile, TQ3FileMode *mode)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(mode), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->GetMode ( mode ) ;
}





//=============================================================================
//      Q3File_GetVersion : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3File_GetVersion(TQ3FileObject theFile, TQ3FileVersion *version)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(version), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->GetVersion ( version ) ;
}





//=============================================================================
//      Q3File_Close : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3File_Close(TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile)->Close () ;
}





//=============================================================================
//      Q3File_Cancel : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3File_Cancel(TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->Cancel () ;
}





//=============================================================================
//      Q3View_StartWriting : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3View_StartWriting(TQ3ViewObject view, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3View_IsOfMyClass ( view ), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3View_StartWriting(view, theFile));
}





//=============================================================================
//      Q3View_EndWriting : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3ViewStatus
Q3View_EndWriting(TQ3ViewObject view)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3View_IsOfMyClass ( view ), kQ3ViewStatusError);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3View_EndWriting(view));
}



#pragma mark -

//=============================================================================
//      Q3File_GetNextObjectType : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3ObjectType
Q3File_GetNextObjectType(TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, kQ3SharedTypeFile), kQ3ObjectTypeInvalid);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->GetNextObjectType () ;
}





//=============================================================================
//      Q3File_IsNextObjectOfType : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Boolean
Q3File_IsNextObjectOfType(TQ3FileObject theFile, TQ3ObjectType ofType)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, kQ3SharedTypeFile), kQ3False);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->IsNextObjectOfType ( ofType ) ;
}





//=============================================================================
//      Q3File_ReadObject : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Object
Q3File_ReadObject(TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, kQ3SharedTypeFile), nullptr);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->ReadObject () ;
}





//=============================================================================
//      Q3File_SkipObject : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3File_SkipObject(TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->SkipObject () ;
}





//=============================================================================
//      Q3File_IsEndOfData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Boolean
Q3File_IsEndOfData(TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, kQ3SharedTypeFile), kQ3False);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->IsEndOfData () ;
}





//=============================================================================
//      Q3File_IsEndOfContainer : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Boolean
Q3File_IsEndOfContainer(TQ3FileObject theFile, TQ3Object rootObject)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, kQ3SharedTypeFile), kQ3False);
	// rootObject can be nullptr


	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->IsEndOfContainer ( rootObject ) ;
}





//=============================================================================
//      Q3File_IsEndOfFile : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Boolean
Q3File_IsEndOfFile(TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, kQ3SharedTypeFile), kQ3False);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->IsEndOfFile () ;
}





//=============================================================================
//      Q3File_MarkAsExternalReference : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3File_MarkAsExternalReference(TQ3FileObject theFile, TQ3SharedObject sharedObject)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(sharedObject, (kQ3ObjectTypeShared)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3File_MarkAsExternalReference(theFile, sharedObject));
}





//=============================================================================
//      Q3File_GetExternalReferences : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3GroupObject
Q3File_GetExternalReferences(TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, kQ3SharedTypeFile), nullptr);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3File_GetExternalReferences(theFile));
}


#pragma mark -


//=============================================================================
//      Q3Shared_ClearEditTracking : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Shared_ClearEditTracking(TQ3SharedObject sharedObject)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(sharedObject, (kQ3ObjectTypeShared)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Shared_ClearEditTracking(sharedObject));
}





//=============================================================================
//      Q3Shared_GetEditTrackingState : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Boolean
Q3Shared_GetEditTrackingState(TQ3SharedObject sharedObject)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(sharedObject, kQ3ObjectTypeShared), kQ3False);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Shared_GetEditTrackingState(sharedObject));
}





//=============================================================================
//      Q3File_SetReadInGroup : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3File_SetReadInGroup(TQ3FileObject theFile, TQ3FileReadGroupState readGroupState)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->SetReadInGroup ( readGroupState ) ;
}





//=============================================================================
//      Q3File_GetReadInGroup : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3File_GetReadInGroup(TQ3FileObject theFile, TQ3FileReadGroupState *readGroupState)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(readGroupState), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->GetReadInGroup ( readGroupState ) ;
}





//=============================================================================
//      Q3File_SetIdleMethod : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3File_SetIdleMethod(TQ3FileObject theFile, TQ3FileIdleMethod idle, const void *idleData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->SetIdleMethod ( idle, idleData ) ;
}



#pragma mark -

//=============================================================================
//      Q3NewLine_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3NewLine_Write(TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3NewLine_Write( (E3File*) theFile ));
}





//=============================================================================
//      Q3Uns8_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Uns8_Read(TQ3Uns8 *data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Uns8_Read(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Uns8_ReadArray : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Uns8_ReadArray (
	TQ3Uns32					numNums,
	TQ3Uns8*					intArray,
	TQ3FileObject            	theFile
)
{
	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(intArray), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);

	

	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return (E3Uns8_ReadArray( numNums, intArray, (E3File*) theFile ));
}





//=============================================================================
//      Q3Uns8_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Uns8_Write(TQ3Uns8 data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Uns8_Write(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Uns16_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Uns16_Read(TQ3Uns16 *data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Uns16_Read(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Uns16_ReadArray : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Uns16_ReadArray (
	TQ3Uns32					numNums,
	TQ3Uns16*					intArray,
	TQ3FileObject            	theFile
)
{
	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(intArray), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);

	

	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return (E3Uns16_ReadArray( numNums, intArray, (E3File*) theFile ));
}





//=============================================================================
//      Q3Uns16_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Uns16_Write(TQ3Uns16 data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Uns16_Write(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Uns32_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Uns32_Read(TQ3Uns32 *data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Uns32_Read(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Uns32_ReadArray : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Uns32_ReadArray (
	TQ3Uns32					numNums,
	TQ3Uns32*					intArray,
	TQ3FileObject            	theFile
)
{
	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(intArray), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);

	

	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return (E3Uns32_ReadArray( numNums, intArray, (E3File*) theFile ));
}





//=============================================================================
//      Q3Uns32_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Uns32_Write(TQ3Uns32 data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Uns32_Write(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Int8_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Int8_Read(TQ3Int8 *data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Int8_Read(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Int8_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Int8_Write(TQ3Int8 data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Int8_Write(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Int16_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Int16_Read(TQ3Int16 *data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Int16_Read(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Int16_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Int16_Write(TQ3Int16 data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Int16_Write(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Int32_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Int32_Read(TQ3Int32 *data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Int32_Read(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Int32_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Int32_Write(TQ3Int32 data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Int32_Write(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Uns64_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Uns64_Read(TQ3Uns64 *data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Uns64_Read(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Uns64_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Uns64_Write(TQ3Uns64 data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Uns64_Write(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Int64_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Int64_Read(TQ3Int64 *data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Int64_Read(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Int64_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Int64_Write(TQ3Int64 data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Int64_Write(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Float32_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Float32_Read(TQ3Float32 *data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Float32_Read(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Float32_ReadArray : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Float32_ReadArray (
	TQ3Uns32					numFloats,
	TQ3Float32*					floatArray,
	TQ3FileObject            	theFile
)
{
	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(floatArray), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);

	

	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return (E3Float32_ReadArray( numFloats, floatArray, (E3File*) theFile ));
}





//=============================================================================
//      Q3Float32_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Float32_Write(TQ3Float32 data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Float32_Write(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Float64_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Float64_Read(TQ3Float64 *data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Float64_Read(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Float64_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Float64_Write(TQ3Float64 data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Float64_Write(data, (E3File*) theFile ));
}





//=============================================================================
//      Q3Size_Pad : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Size
Q3Size_Pad(TQ3Size size)
{


	// Release build checks



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Size_Pad(size));
}





//=============================================================================
//      Q3String_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3String_Read(char *data, TQ3Uns32 *length, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(length), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3String_Read(data, length, (E3File*) theFile ));
}





//=============================================================================
//      Q3String_ReadUnlimited : Quesa API entry point.
//-----------------------------------------------------------------------------
#if QUESA_ALLOW_QD3D_EXTENSIONS
TQ3Status
Q3String_ReadUnlimited(char *data, TQ3Uns32 *ioLength, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(ioLength), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3String_ReadUnlimited(data, ioLength, (E3File*) theFile ));
}
#endif





//=============================================================================
//      Q3String_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3String_Write(const char *data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3String_Write(data, (E3File*) theFile ));
}




//=============================================================================
//      Q3String_WriteUnlimited : Quesa API entry point.
//-----------------------------------------------------------------------------
#if QUESA_ALLOW_QD3D_EXTENSIONS
TQ3Status
Q3String_WriteUnlimited(const char *data, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return (E3String_WriteUnlimited(data, (E3File*) theFile ));
}
#endif




//=============================================================================
//      Q3RawData_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3RawData_Read(unsigned char *data, TQ3Uns32 size, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3RawData_Read(data, size, (E3File*) theFile ));
}





//=============================================================================
//      Q3RawData_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3RawData_Write(const unsigned char *data, TQ3Uns32 size, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3RawData_Write(data, size, (E3File*) theFile ));
}





//=============================================================================
//      Q3Point2D_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Point2D_Read(TQ3Point2D *point2D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(point2D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Point2D_Read(point2D, (E3File*) theFile ));
}





//=============================================================================
//      Q3Point2D_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Point2D_Write(const TQ3Point2D *point2D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(point2D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Point2D_Write(point2D, (E3File*) theFile ));
}





//=============================================================================
//      Q3Point3D_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Point3D_Read(TQ3Point3D *point3D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(point3D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Point3D_Read(point3D, (E3File*) theFile ));
}





//=============================================================================
//      Q3Point3D_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Point3D_Write(const TQ3Point3D *point3D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(point3D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Point3D_Write(point3D, (E3File*) theFile ));
}





//=============================================================================
//      Q3RationalPoint3D_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3RationalPoint3D_Read(TQ3RationalPoint3D *point3D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(point3D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3RationalPoint3D_Read(point3D, (E3File*) theFile ));
}





//=============================================================================
//      Q3RationalPoint3D_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3RationalPoint3D_Write(const TQ3RationalPoint3D *point3D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(point3D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3RationalPoint3D_Write(point3D, (E3File*) theFile ));
}





//=============================================================================
//      Q3RationalPoint4D_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3RationalPoint4D_Read(TQ3RationalPoint4D *point4D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(point4D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3RationalPoint4D_Read(point4D, (E3File*) theFile ));
}





//=============================================================================
//      Q3RationalPoint4D_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3RationalPoint4D_Write(const TQ3RationalPoint4D *point4D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(point4D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3RationalPoint4D_Write(point4D, (E3File*) theFile ));
}





//=============================================================================
//      Q3Vector2D_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Vector2D_Read(TQ3Vector2D *vector2D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(vector2D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Vector2D_Read(vector2D, (E3File*) theFile ));
}





//=============================================================================
//      Q3Vector2D_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Vector2D_Write(const TQ3Vector2D *vector2D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(vector2D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Vector2D_Write(vector2D, (E3File*) theFile ));
}





//=============================================================================
//      Q3Vector3D_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Vector3D_Read(TQ3Vector3D *vector3D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(vector3D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Vector3D_Read(vector3D, (E3File*) theFile ));
}





//=============================================================================
//      Q3Vector3D_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Vector3D_Write(const TQ3Vector3D *vector3D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(vector3D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Vector3D_Write(vector3D, (E3File*) theFile ));
}





//=============================================================================
//      Q3Matrix4x4_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Matrix4x4_Read(TQ3Matrix4x4 *matrix4x4, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(matrix4x4), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Matrix4x4_Read(matrix4x4, (E3File*) theFile ));
}





//=============================================================================
//      Q3Matrix4x4_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Matrix4x4_Write(const TQ3Matrix4x4 *matrix4x4, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(matrix4x4), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Matrix4x4_Write(matrix4x4, (E3File*) theFile ));
}





//=============================================================================
//      Q3Tangent2D_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Tangent2D_Read(TQ3Tangent2D *tangent2D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(tangent2D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Tangent2D_Read(tangent2D, (E3File*) theFile ));
}





//=============================================================================
//      Q3Tangent2D_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Tangent2D_Write(const TQ3Tangent2D *tangent2D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(tangent2D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Tangent2D_Write(tangent2D, (E3File*) theFile ));
}





//=============================================================================
//      Q3Tangent3D_Read : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Tangent3D_Read(TQ3Tangent3D *tangent3D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(tangent3D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Tangent3D_Read(tangent3D, (E3File*) theFile ));
}





//=============================================================================
//      Q3Tangent3D_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Tangent3D_Write(const TQ3Tangent3D *tangent3D, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(tangent3D), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Tangent3D_Write(tangent3D, (E3File*) theFile ));
}





//=============================================================================
//      Q3Comment_Write : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Comment_Write(char *comment, TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(comment), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, (kQ3SharedTypeFile)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Comment_Write(comment, (E3File*) theFile ));
}



#pragma mark -

//=============================================================================
//      Q3Unknown_GetType : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3ObjectType
Q3Unknown_GetType(TQ3UnknownObject unknownObject)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(unknownObject, kQ3ShapeTypeUnknown), kQ3ObjectTypeInvalid);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Unknown_GetType(unknownObject));
}





//=============================================================================
//      Q3Unknown_GetDirtyState : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Unknown_GetDirtyState(TQ3UnknownObject unknownObject, TQ3Boolean *isDirty)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(unknownObject, (kQ3ShapeTypeUnknown)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(isDirty), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Unknown_GetDirtyState(unknownObject, isDirty));
}





//=============================================================================
//      Q3Unknown_SetDirtyState : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3Unknown_SetDirtyState(TQ3UnknownObject unknownObject, TQ3Boolean isDirty)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(unknownObject, (kQ3ShapeTypeUnknown)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3Unknown_SetDirtyState(unknownObject, isDirty));
}





//=============================================================================
//      Q3UnknownText_GetData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3UnknownText_GetData(TQ3UnknownObject unknownObject, TQ3UnknownTextData *unknownTextData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(unknownObject, (kQ3ShapeTypeUnknown)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(unknownTextData), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3UnknownText_GetData(unknownObject, unknownTextData));
}





//=============================================================================
//      Q3UnknownText_EmptyData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3UnknownText_EmptyData(TQ3UnknownTextData *unknownTextData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(unknownTextData), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3UnknownText_EmptyData(unknownTextData));
}


#pragma mark -


//=============================================================================
//      Q3UnknownBinary_GetData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3UnknownBinary_GetData(TQ3UnknownObject unknownObject, TQ3UnknownBinaryData *unknownBinaryData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(unknownObject, (kQ3ShapeTypeUnknown)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(unknownBinaryData), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3UnknownBinary_GetData(unknownObject, unknownBinaryData));
}





//=============================================================================
//      Q3UnknownBinary_EmptyData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3UnknownBinary_EmptyData(TQ3UnknownBinaryData *unknownBinaryData)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(unknownBinaryData), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3UnknownBinary_EmptyData(unknownBinaryData));
}





//=============================================================================
//      Q3UnknownBinary_GetTypeString : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3UnknownBinary_GetTypeString(TQ3UnknownObject unknownObject, char **typeString)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(unknownObject, (kQ3ShapeTypeUnknown)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(typeString), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3UnknownBinary_GetTypeString(unknownObject, typeString));
}





//=============================================================================
//      Q3UnknownBinary_EmptyTypeString : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3UnknownBinary_EmptyTypeString(char **typeString)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(typeString), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3UnknownBinary_EmptyTypeString(typeString));
}



#pragma mark -

//=============================================================================
//      Q3ViewHints_New : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3ViewHintsObject
Q3ViewHints_New(TQ3ViewObject view)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT( E3View_IsOfMyClass ( view ), nullptr);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3ViewHints_New(view));
}





//=============================================================================
//      Q3ViewHints_SetRenderer : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_SetRenderer(TQ3ViewHintsObject viewHints, TQ3RendererObject renderer)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(renderer, (kQ3SharedTypeRenderer)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->SetRenderer ( renderer ) ;
}





//=============================================================================
//      Q3ViewHints_GetRenderer : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_GetRenderer(TQ3ViewHintsObject viewHints, TQ3RendererObject *renderer)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(renderer), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->GetRenderer ( renderer ) ;
}





//=============================================================================
//      Q3ViewHints_SetCamera : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_SetCamera(TQ3ViewHintsObject viewHints, TQ3CameraObject camera)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(camera, (kQ3ShapeTypeCamera)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->SetCamera ( camera ) ;
}





//=============================================================================
//      Q3ViewHints_GetCamera : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_GetCamera(TQ3ViewHintsObject viewHints, TQ3CameraObject *camera)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(camera), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->GetCamera ( camera ) ;
}





//=============================================================================
//      Q3ViewHints_SetLightGroup : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_SetLightGroup(TQ3ViewHintsObject viewHints, TQ3GroupObject lightGroup)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(lightGroup, (kQ3ShapeTypeGroup)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->SetLightGroup ( lightGroup ) ;
}





//=============================================================================
//      Q3ViewHints_GetLightGroup : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_GetLightGroup(TQ3ViewHintsObject viewHints, TQ3GroupObject *lightGroup)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(lightGroup), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->GetLightGroup ( lightGroup ) ;
}





//=============================================================================
//      Q3ViewHints_SetAttributeSet : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_SetAttributeSet(TQ3ViewHintsObject viewHints, TQ3AttributeSet attributeSet)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->SetAttributeSet ( attributeSet ) ;
}





//=============================================================================
//      Q3ViewHints_GetAttributeSet : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_GetAttributeSet(TQ3ViewHintsObject viewHints, TQ3AttributeSet *attributeSet)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(attributeSet), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->GetAttributeSet ( attributeSet ) ;
}





//=============================================================================
//      Q3ViewHints_SetDimensionsState : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_SetDimensionsState(TQ3ViewHintsObject viewHints, TQ3Boolean isValid)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->SetDimensionsState ( isValid ) ;
}





//=============================================================================
//      Q3ViewHints_GetDimensionsState : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_GetDimensionsState(TQ3ViewHintsObject viewHints, TQ3Boolean *isValid)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(isValid), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->GetDimensionsState ( isValid ) ;
}





//=============================================================================
//      Q3ViewHints_SetDimensions : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_SetDimensions(TQ3ViewHintsObject viewHints, TQ3Uns32 width, TQ3Uns32 height)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->SetDimensions ( width, height ) ;
}





//=============================================================================
//      Q3ViewHints_GetDimensions : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_GetDimensions(TQ3ViewHintsObject viewHints, TQ3Uns32 *width, TQ3Uns32 *height)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(width), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(height), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->GetDimensions ( width, height ) ;
}





//=============================================================================
//      Q3ViewHints_SetMaskState : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_SetMaskState(TQ3ViewHintsObject viewHints, TQ3Boolean isValid)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->SetMaskState ( isValid ) ;
}





//=============================================================================
//      Q3ViewHints_GetMaskState : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_GetMaskState(TQ3ViewHintsObject viewHints, TQ3Boolean *isValid)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(isValid), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->GetMaskState ( isValid ) ;
}





//=============================================================================
//      Q3ViewHints_SetMask : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_SetMask(TQ3ViewHintsObject viewHints, const TQ3Bitmap *mask)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(mask), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->SetMask ( mask ) ;
}





//=============================================================================
//      Q3ViewHints_GetMask : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_GetMask(TQ3ViewHintsObject viewHints, TQ3Bitmap *mask)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(mask), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->GetMask ( mask ) ;
}





//=============================================================================
//      Q3ViewHints_SetClearImageMethod : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_SetClearImageMethod(TQ3ViewHintsObject viewHints, TQ3DrawContextClearImageMethod clearMethod)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->SetClearImageMethod ( clearMethod ) ;
}





//=============================================================================
//      Q3ViewHints_GetClearImageMethod : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_GetClearImageMethod(TQ3ViewHintsObject viewHints, TQ3DrawContextClearImageMethod *clearMethod)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(clearMethod), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->GetClearImageMethod ( clearMethod ) ;
}





//=============================================================================
//      Q3ViewHints_SetClearImageColor : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_SetClearImageColor(TQ3ViewHintsObject viewHints, const TQ3ColorARGB *color)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(color), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->SetClearImageColor ( color ) ;
}





//=============================================================================
//      Q3ViewHints_GetClearImageColor : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3ViewHints_GetClearImageColor(TQ3ViewHintsObject viewHints, TQ3ColorARGB *color)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(viewHints, (kQ3SharedTypeViewHints)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(color), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3ViewHints*) viewHints )->GetClearImageColor ( color ) ;
}





//=============================================================================
//      Q3File_GetFileFormat : Quesa API entry point.
//-----------------------------------------------------------------------------
#pragma mark -
TQ3FileFormatObject
Q3File_GetFileFormat(TQ3FileObject theFile)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(theFile, kQ3SharedTypeFile), nullptr);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return ( (E3File*) theFile )->GetFileFormat () ;

}





//=============================================================================
//      Q3FileFormat_NewFromType : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3FileFormatObject
Q3FileFormat_NewFromType(TQ3ObjectType fformatObjectType)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(fformatObjectType != kQ3ObjectTypeInvalid, nullptr);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_NewFromType(fformatObjectType));
}


//=============================================================================
//      Q3FileFormat_GetType : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3ObjectType
Q3FileFormat_GetType(TQ3FileFormatObject format)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, kQ3ObjectTypeFileFormat), kQ3ObjectTypeInvalid);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GetType(format));
}





//=============================================================================
//      Q3FileFormat_HasModalConfigure : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Boolean
Q3FileFormat_HasModalConfigure(TQ3FileFormatObject format)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, kQ3ObjectTypeFileFormat), kQ3False);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_HasModalConfigure(format));
}





//=============================================================================
//      Q3FileFormat_ModalConfigure : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_ModalConfigure(TQ3FileFormatObject format, TQ3DialogAnchor dialogAnchor, TQ3Boolean *canceled)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(canceled), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_ModalConfigure(format, dialogAnchor, canceled));
}





//=============================================================================
//      Q3FileFormatClass_GetFormatNameString : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormatClass_GetFormatNameString(TQ3ObjectType formatClassType, TQ3ObjectClassNameString formatClassString)
{


	// Release build checks



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormatClass_GetFormatNameString(formatClassType, formatClassString));
}





//=============================================================================
//      Q3FileFormat_GetConfigurationData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GetConfigurationData(TQ3FileFormatObject format, unsigned char *dataBuffer, TQ3Uns32 bufferSize, TQ3Uns32 *actualDataSize)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(dataBuffer), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(actualDataSize), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GetConfigurationData(format, dataBuffer, bufferSize, actualDataSize));
}





//=============================================================================
//      Q3FileFormat_SetConfigurationData : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_SetConfigurationData(TQ3RendererObject format, unsigned char *dataBuffer, TQ3Uns32 bufferSize)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(dataBuffer), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_SetConfigurationData(format, dataBuffer, bufferSize));
}





//=============================================================================
//      Q3FileFormat_GenericReadBinary_8 : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericReadBinary_8(TQ3FileFormatObject format, TQ3Int8 *data)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericReadBinary_8(format, data));
}





//=============================================================================
//      Q3FileFormat_GenericReadBinary_16 : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericReadBinary_16(TQ3FileFormatObject format, TQ3Int16 *data)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericReadBinary_16(format, data));
}





//=============================================================================
//      Q3FileFormat_GenericReadBinary_32 : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericReadBinary_32(TQ3FileFormatObject format, TQ3Int32 *data)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericReadBinary_32(format, data));
}





//=============================================================================
//      Q3FileFormat_GenericReadBinary_64 : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericReadBinary_64(TQ3FileFormatObject format, TQ3Int64 *data)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericReadBinary_64(format, data));
}





//=============================================================================
//      Q3FileFormat_GenericReadBinary_String : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericReadBinary_String(TQ3FileFormatObject format, char *data, TQ3Uns32 *length)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(length), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericReadBinary_String(format, data, length));
}





//=============================================================================
//      Q3FileFormat_GenericReadBinary_Raw : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericReadBinary_Raw(TQ3FileFormatObject format, unsigned char *data, TQ3Uns32 length)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(length != 0, kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericReadBinary_Raw(format, data, length));
}





//=============================================================================
//      Q3FileFormat_GenericReadBinSwap_16 : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericReadBinSwap_16(TQ3FileFormatObject format, TQ3Int16 *data)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericReadBinSwap_16(format, data));
}





//=============================================================================
//      Q3FileFormat_GenericReadBinSwap_32 : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericReadBinSwap_32(TQ3FileFormatObject format, TQ3Int32 *data)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericReadBinSwap_32(format, data));
}





//=============================================================================
//      Q3FileFormat_GenericReadBinSwap_64 : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericReadBinSwap_64(TQ3FileFormatObject format, TQ3Int64 *data)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericReadBinSwap_64(format, data));
}





//=============================================================================
//      Q3FileFormat_GenericReadText_SkipBlanks : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericReadText_SkipBlanks(TQ3FileFormatObject format)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericReadText_SkipBlanks(format));
}





//=============================================================================
//      Q3FileFormat_GenericReadText_ReadUntilChars : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericReadText_ReadUntilChars(TQ3FileFormatObject		format,
											 char					*buffer,
											 char					*chars,
											 TQ3Uns32				numChars,
											 TQ3Boolean				blanks,
											 TQ3Int32				*foundChar,
											 TQ3Uns32				maxLen,
											 TQ3Uns32				*charsRead)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(buffer), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(chars), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(foundChar), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(charsRead), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericReadText_ReadUntilChars(format, buffer, chars, numChars, blanks, foundChar, maxLen, charsRead));
}



//=============================================================================
//      Q3FileFormat_GenericWriteBinary_8 : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericWriteBinary_8(TQ3FileFormatObject format, const TQ3Int8* data)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericWriteBinary_8(format, data));
}





//=============================================================================
//      Q3FileFormat_GenericWriteBinary_16 : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericWriteBinary_16(TQ3FileFormatObject format, const TQ3Int16* data)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericWriteBinary_16(format, data));
}





//=============================================================================
//      Q3FileFormat_GenericWriteBinary_32 : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericWriteBinary_32(TQ3FileFormatObject format, const TQ3Int32* data)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericWriteBinary_32(format, data));
}





//=============================================================================
//      Q3FileFormat_GenericWriteBinary_64 : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericWriteBinary_64(TQ3FileFormatObject format, const TQ3Int64* data)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericWriteBinary_64(format, data));
}





//=============================================================================
//      Q3FileFormat_GenericWriteBinary_String : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericWriteBinary_String(TQ3FileFormatObject format, const char *data, TQ3Uns32 *length)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(length), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericWriteBinary_String(format, data, length));
}





//=============================================================================
//      Q3FileFormat_GenericWriteBinary_Raw : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericWriteBinary_Raw(TQ3FileFormatObject format, const unsigned char *data, TQ3Uns32 length)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(Q3_VALID_PTR(data), kQ3Failure);
	Q3_REQUIRE_OR_RESULT(length != 0, kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericWriteBinary_Raw(format, data, length));
}





//=============================================================================
//      Q3FileFormat_GenericWriteBinSwap_16 : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericWriteBinSwap_16(TQ3FileFormatObject format, const TQ3Int16* data)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericWriteBinSwap_16(format, data));
}





//=============================================================================
//      Q3FileFormat_GenericWriteBinSwap_32 : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericWriteBinSwap_32(TQ3FileFormatObject format, const TQ3Int32* data)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericWriteBinSwap_32(format, data));
}





//=============================================================================
//      Q3FileFormat_GenericWriteBinSwap_64 : Quesa API entry point.
//-----------------------------------------------------------------------------
TQ3Status
Q3FileFormat_GenericWriteBinSwap_64(TQ3FileFormatObject format, const TQ3Int64* data)
{


	// Release build checks
	Q3_REQUIRE_OR_RESULT(Q3Object_IsType(format, (kQ3ObjectTypeFileFormat)), kQ3Failure);



	// Debug build checks



	// Call the bottleneck
	E3System_Bottleneck();



	// Call our implementation
	return(E3FileFormat_GenericWriteBinSwap_64(format, data));
}




