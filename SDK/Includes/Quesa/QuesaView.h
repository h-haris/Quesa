/*! @header QuesaView.h
        Declares the Quesa view object.

	@ignore	_Nullable
	@ignore _Nonnull
	@ignore	_Null_unspecified
 */
/*  NAME:
        QuesaView.h

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
#ifndef QUESA_VIEW_HDR
#define QUESA_VIEW_HDR
//=============================================================================
//      Include files
//-----------------------------------------------------------------------------
#include "Quesa.h"

#include "QuesaStyle.h"
#include "QuesaSet.h"

// Disable QD3D header
#ifdef __QD3DVIEW__
#error
#endif

#define __QD3DVIEW__





//=============================================================================
//      C++ preamble
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif





//=============================================================================
//      Constants
//-----------------------------------------------------------------------------
/*!
 *  @enum
 *      TQ3ViewStatus
 *  @discussion
 *      View submit loop status.
 *
 *  @constant kQ3ViewStatusDone                 Submit loop completed successfully.
 *  @constant kQ3ViewStatusRetraverse           Submit loop requires another pass.
 *  @constant kQ3ViewStatusError                Submit loop encountered an error.
 *  @constant kQ3ViewStatusCancelled            Submit loop was cancelled.
 */
typedef enum TQ3ViewStatus QUESA_ENUM_BASE(TQ3Uns32) {
    kQ3ViewStatusDone                           = 0,
    kQ3ViewStatusRetraverse                     = 1,
    kQ3ViewStatusError                          = 2,
    kQ3ViewStatusCancelled                      = 3,
    kQ3ViewStatusSize32                         = 0xFFFFFFFF
} TQ3ViewStatus;


// Default attribute values
#define kQ3ViewDefaultAmbientCoefficient        1.0f
#define kQ3ViewDefaultDiffuseColor              1.0f, 1.0f, 1.0f
#define kQ3ViewDefaultSpecularColor             0.5f, 0.5f, 0.5f
#define kQ3ViewDefaultSpecularControl           4.0f
#define kQ3ViewDefaultMetallic					0.0f
#define kQ3ViewDefaultTransparency              1.0f, 1.0f, 1.0f
#define kQ3ViewDefaultHighlightState            kQ3Off
#define kQ3ViewDefaultHighlightColor            1.0f, 0.0f, 0.0f
#define kQ3ViewDefaultSubdivisionMethod         kQ3SubdivisionMethodScreenSpace
#define kQ3ViewDefaultSubdivisionC1             20.0f
#define kQ3ViewDefaultSubdivisionC2             20.0f





//=============================================================================
//      Types
//-----------------------------------------------------------------------------
/*!
 *  @typedef
 *      TQ3ViewIdleMethod
 *  @discussion
 *      Application callback for Q3View_SetIdleMethod.
 *
 *  @param theView          The view being submitted to.
 *  @param idlerData        The application-specific data passed to Q3View_SetIdleMethod.
 *  @result                 Success or failure of the callback.
 */
typedef Q3_CALLBACK_API_C(TQ3Status,           TQ3ViewIdleMethod)(
                            TQ3ViewObject _Nonnull      theView,
                            const void          * _Nonnull idlerData);


/*!
 *  @typedef
 *      TQ3ViewIdleProgressMethod
 *  @discussion
 *      Application callback for Q3View_SetIdleProgressMethod.
 *
 *  @param theView              The view being submitted to.
 *  @param idlerData            The application-specific data passed to Q3View_SetIdleProgressMethod.
 *  @param progressCurrent      The number of work units completed.
 *  @param progressCompleted    The total number of work units which will be completed.
 *  @result                     Success or failure of the callback.
 */
typedef Q3_CALLBACK_API_C(TQ3Status,           TQ3ViewIdleProgressMethod)(
                            TQ3ViewObject _Nonnull      theView,
                            const void          * _Nonnull idlerData,
                            TQ3Uns32            progressCurrent,
                            TQ3Uns32            progressCompleted);


/*!
 *  @typedef
 *      TQ3ViewEndFrameMethod
 *  @discussion
 *      Application callback for Q3View_SetEndFrameMethod.
 *
 *  @param theView          The view being submitted to.
 *  @param endFrameData     The application-specific data passed to Q3View_SetEndFrameMethod.
 */
typedef Q3_CALLBACK_API_C(void,                TQ3ViewEndFrameMethod)(
                            TQ3ViewObject _Nonnull      theView,
                            void                * _Nonnull endFrameData);





//=============================================================================
//      Function prototypes
//-----------------------------------------------------------------------------
/*!
 *  @function
 *      Q3View_New
 *  @discussion
 *      Create a new view.
 *
 *  @result                 The new view object.
 */
Q3_EXTERN_API_C ( TQ3ViewObject _Nonnull )
Q3View_New (
    void
);



/*!
 *  @function
 *      Q3View_NewWithDefaults
 *  @discussion
 *      Create a view with a default camera, light group, renderer, and
 *		draw context for the specified draw context target.
 *
 *      The draw context type indicates the type of draw context target
 *      to create, and implies the following values for drawContextTarget.
 *
 *        kQ3DrawContextTypePixmap    => A TQ3Pixmap object.
 *        kQ3DrawContextTypeMacintosh => A WindowRef.
 *        kQ3DrawContextTypeCocoa     => An NSView.
 *        kQ3DrawContextTypeWin32DC   => An HDC.
 *        kQ3DrawContextTypeX11       => A Visual.
 *        kQ3DrawContextTypeBe        => A BView.
 *
 *      The exact contents of the returned view are not defined, however it
 *      should be suitable for interactively rendering objects placed at the
 *      origin and will fill the supplied window.
 *
 *      <em>This function is not available in QD3D.</em>
 *
 *  @param drawContextType     The type of data referenced by drawContextTarget
 *  @param drawContextTarget   The window/etc reference for the draw context.
 *  @result                    The new view object, or nullptr on failure.
 */
#if QUESA_ALLOW_QD3D_EXTENSIONS

Q3_EXTERN_API_C ( TQ3ViewObject _Nullable )
Q3View_NewWithDefaults (
    TQ3ObjectType                 drawContextType,
    void                          * _Nonnull drawContextTarget
);

#endif // QUESA_ALLOW_QD3D_EXTENSIONS



/*!
 *  @function
 *      Q3View_Cancel
 *  @discussion
 *      Cancel a submit loop.
 *
 *  @param view             The view being submitted to.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_Cancel (
    TQ3ViewObject _Nonnull                view
);



/*!
 *  @function
 *      Q3View_SetRendererByType
 *  @discussion
 *      Set the renderer for a view.
 *
 *      The renderer is specified by class type, e.g., kQ3RendererTypeInteractive
 *      for the interactive renderer or kQ3RendererTypeWireFrame for the wire-frame
 *      renderer.
 *
 *  @param view             The view to update.
 *  @param theType          The type of the renderer to assign to the view.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_SetRendererByType (
    TQ3ViewObject _Nonnull                view,
    TQ3ObjectType                 theType
);



/*!
 *  @function
 *      Q3View_SetRenderer
 *  @discussion
 *      Set the renderer for a view.
 *
 *      The reference count of the renderer will be incremented by the view.
 *
 *  @param view             The view to update.
 *  @param renderer         The renderer to assign to the view.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_SetRenderer (
    TQ3ViewObject _Nonnull                view,
    TQ3RendererObject _Nullable            renderer
);



/*!
 *  @function
 *      Q3View_GetRenderer
 *  @discussion
 *      Get the renderer from a view.
 *
 *      The reference count of the renderer is incremented.
 *
 *  @param view             The view to query.
 *  @param renderer         Receives the renderer associated with the view.
 *                          Dispose the reference when you are done with it.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetRenderer (
    TQ3ViewObject _Nonnull                view,
    TQ3RendererObject _Nullable           * _Nonnull renderer
);



/*!
 *  @function
 *      Q3View_StartRendering
 *  @discussion
 *      Start a rendering loop.
 *
 *  @param view             The view to start rendering with.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_StartRendering (
    TQ3ViewObject _Nonnull                view
);



/*!
 *  @function
 *      Q3View_EndRendering
 *  @discussion
 *      End a rendering loop.
 *
 *  @param view             The view being rendered to.
 *  @result                 Result of the rendering pass.
 *                          Note that the result is a TQ3ViewStatus, not a TQ3Status.
 *							Be sure to watch for kQ3ViewStatusRetraverse, and repeat
 *							the submit loop in that case.
 */
Q3_EXTERN_API_C ( TQ3ViewStatus  )
Q3View_EndRendering (
    TQ3ViewObject _Nonnull                view
);



/*!
 *  @function
 *      Q3View_Flush
 *  @discussion
 *      Flush the output from within a rendering loop.
 *
 *      May only be called between a Q3View_StartRendering/Q3View_EndRendering sequence.
 *      May or may not update the draw context - behaviour is renderer-dependent.
 *
 *  @param view             The view to flush.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_Flush (
    TQ3ViewObject _Nonnull                view
);



/*!
 *  @function
 *      Q3View_Sync
 *  @discussion
 *      Flush the previous rendering loop to the draw context.
 *
 *      Blocks until the previous rendering loop has updated the draw context. May only
 *      be called after a call to Q3View_EndRendering.
 *
 *  @param view             The view to sync.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_Sync (
    TQ3ViewObject _Nonnull                view
);



/*!
 *  @function
 *      Q3View_StartBoundingBox
 *  @discussion
 *      Start a bounding loop.
 *
 *  @param view             The view to start bounding with.
 *  @param computeBounds    The accuracy of the calculated bounds.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_StartBoundingBox (
    TQ3ViewObject _Nonnull                view,
    TQ3ComputeBounds              computeBounds
);



/*!
 *  @function
 *      Q3View_EndBoundingBox
 *  @discussion
 *      End a bounding loop.
 *
 *  @param view             The view being bounded to.
 *  @param theBounds        Bounding box to set to the computed bounds.
 *  @result                 Result of the bounding pass.
 *                          Note that the result is a TQ3ViewStatus, not a TQ3Status.
 *							Be sure to watch for kQ3ViewStatusRetraverse, and repeat
 *							the submit loop in that case.
 */
Q3_EXTERN_API_C ( TQ3ViewStatus  )
Q3View_EndBoundingBox (
    TQ3ViewObject _Nonnull                view,
    TQ3BoundingBox                * _Nonnull theBounds
);



/*!
 *  @function
 *      Q3View_StartBoundingSphere
 *  @discussion
 *      Start a bounding loop.
 *
 *  @param view             The view to start bounding with.
 *  @param computeBounds    The accuracy of the calculated bounds.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_StartBoundingSphere (
    TQ3ViewObject _Nonnull                view,
    TQ3ComputeBounds              computeBounds
);



/*!
 *  @function
 *      Q3View_EndBoundingSphere
 *  @discussion
 *      End a bounding loop.
 *
 *  @param view             The view being bounded to.
 *  @param theBounds        Bounding sphere to set to the computed bounds.
 *  @result                 Result of the bounding pass.
 *                          Note that the result is a TQ3ViewStatus, not a TQ3Status.
 *							Be sure to watch for kQ3ViewStatusRetraverse, and repeat
 *							the submit loop in that case.
 */
Q3_EXTERN_API_C ( TQ3ViewStatus  )
Q3View_EndBoundingSphere (
    TQ3ViewObject _Nonnull                view,
    TQ3BoundingSphere             * _Nonnull theBounds
);



/*!
 *  @function
 *      Q3View_StartPicking
 *  @discussion
 *      Start a picking loop.
 *
 *  @param view             The view to start picking with.
 *  @param pick             The pick object for the pick loop.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_StartPicking (
    TQ3ViewObject _Nonnull                view,
    TQ3PickObject _Nonnull                pick
);



/*!
 *  @function
 *      Q3View_EndPicking
 *  @discussion
 *      End a picking loop.
 *
 *  @param view             The view being picked to.
 *  @result                 Result of the picking pass.
 *                          Note that the result is a TQ3ViewStatus, not a TQ3Status.
 *							Be sure to watch for kQ3ViewStatusRetraverse, and repeat
 *							the submit loop in that case.
 */
Q3_EXTERN_API_C ( TQ3ViewStatus  )
Q3View_EndPicking (
    TQ3ViewObject _Nonnull                view
);



/*!
 *  @function
 *      Q3View_GetCamera
 *  @discussion
 *      Get the camera from a view.
 *
 *      The reference count of the camera is incremented.
 *
 *  @param view             The view to query.
 *  @param camera           Receives the camera associated with the view.
 *                          Dispose the reference when you are done with it.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetCamera (
    TQ3ViewObject _Nonnull                view,
    TQ3CameraObject _Nullable             * _Nonnull camera
);



/*!
 *  @function
 *      Q3View_SetCamera
 *  @discussion
 *      Set the camera for a view.
 *
 *      The reference count of the camera will be incremented by the view.
 *
 *  @param view             The view to update.
 *  @param camera           The camera to assign to the view.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_SetCamera (
    TQ3ViewObject _Nonnull                view,
    TQ3CameraObject _Nonnull              camera
);



/*!
 *  @function
 *      Q3View_SetLightGroup
 *  @discussion
 *      Set the light group for a view.
 *
 *      The reference count of the light group will be incremented by the view.
 *
 *  @param view             The view to update.
 *  @param lightGroup       The light group to assign to the view.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_SetLightGroup (
    TQ3ViewObject _Nonnull                view,
    TQ3GroupObject _Nonnull               lightGroup
);



/*!
 *  @function
 *      Q3View_GetLightGroup
 *  @discussion
 *      Get the light group from a view.
 *
 *      The reference count of the light group is incremented.
 *
 *  @param view             The view to query.
 *  @param lightGroup       Receives the light group associated with the view.
 *                          Dispose the reference when you are done with it.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetLightGroup (
    TQ3ViewObject _Nonnull                view,
    TQ3GroupObject _Nullable              * _Nonnull lightGroup
);



/*!
 *  @function
 *      Q3View_AddLight
 *  @discussion
 *      Add a light to a view's light group.
 *
 *      If the view does not currently posses a light group, a new group
 *      will be created.
 *
 *      lightData is assumed to point to the appropriate data structure
 *      for lightType. E.g., if lightType is kQ3LightTypeSpot, then
 *      lightData is assumed to be a pointer to a TQ3SpotLightData
 *      structure.
 *
 *      lightType may also be set to kQ3ShapeTypeLight. In this case
 *      lightData is assumed to point to an existing TQ3LightObject (of
 *      any type) rather than to the data structure for a specific
 *      type of light.
 *
 *      The reference count of the TQ3LightObject will be unaffected, and
 *      so the caller must dispose of it to release their reference.
 *
 *      <em>This function is not available in QD3D.</em>
 *
 *  @param theView          The view to add the light to.
 *  @param lightType        The type of data referenced by lightData.
 *  @param lightData        The data for the light.
 *  @result                 Success or failure of the operation.
 */
#if QUESA_ALLOW_QD3D_EXTENSIONS

Q3_EXTERN_API_C ( TQ3Status  )
Q3View_AddLight (
    TQ3ViewObject _Nonnull                theView,
    TQ3ObjectType                 lightType,
    void                          * _Nonnull lightData
);

#endif // QUESA_ALLOW_QD3D_EXTENSIONS



/*!
 *  @function
 *      Q3View_SetIdleMethod
 *  @discussion
 *      Set the idle method for view submit operations.
 *
 *      An idle method can be used to return control to the application
 *      periodically during a submit loop.
 *
 *  @param view             The view to update.
 *  @param idleMethod       The view idle callback.
 *  @param idleData         Application-specific data for the callback.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_SetIdleMethod (
    TQ3ViewObject _Nonnull                view,
    TQ3ViewIdleMethod _Nullable            idleMethod,
    const void                    * _Nonnull idleData
);



/*!
 *  @function
 *      Q3View_SetIdleProgressMethod
 *  @discussion
 *      Set the idle progress method for view submit operations.
 *
 *      An idle method can be used to return control to the application
 *      periodically during a submit loop.
 *
 *      The idle progress method allows renderers to pass progress information
 *      to the application, to update the user interface during a render.
 *
 *  @param view             The view to update.
 *  @param idleMethod       The view idle callback.
 *  @param idleData         Application-specific data for the callback.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_SetIdleProgressMethod (
    TQ3ViewObject _Nonnull                view,
    TQ3ViewIdleProgressMethod _Nullable     idleMethod,
    const void                    * _Nonnull idleData
);



/*!
 *  @function
 *      Q3View_SetEndFrameMethod
 *  @discussion
 *      Set the end frame method for view submit operations.
 *
 *  @param view             The view to update.
 *  @param endFrame         The view end frame callback.
 *  @param endFrameData     Application-specific data for the callback.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_SetEndFrameMethod (
    TQ3ViewObject _Nonnull                view,
    TQ3ViewEndFrameMethod _Nullable        endFrame,
    void                          * _Nonnull endFrameData
);



/*!
 *  @function
 *      Q3Push_Submit
 *  @discussion
 *      Submit a push state operator to a view.
 *
 *  @param view             The view to submit the state operator to.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3Push_Submit (
    TQ3ViewObject _Nonnull                view
);



/*!
 *  @function
 *      Q3Pop_Submit
 *  @discussion
 *      Submit a pop state operator to a view.
 *
 *  @param view             The view to submit the state operator to.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3Pop_Submit (
    TQ3ViewObject _Nonnull                view
);



/*!
 *  @function
 *      Q3Push_New
 *  @discussion
 *      Create a push state operator.
 *
 *  @result                 The new push state operator.
 */
Q3_EXTERN_API_C ( TQ3StateOperatorObject _Nonnull )
Q3Push_New (
    void
);



/*!
 *  @function
 *      Q3Pop_New
 *  @discussion
 *      Create a pop state operator.
 *
 *  @result                 The new pop state operator.
 */
Q3_EXTERN_API_C ( TQ3StateOperatorObject _Nonnull )
Q3Pop_New (
    void
);



/*!
 *  @function
 *      Q3StateOperator_Submit
 *  @discussion
 *      Submit a state operator to a view.
 *
 *  @param stateOperator    The state operator to submit.
 *  @param view             The view to submit the state operator to.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3StateOperator_Submit (
    TQ3StateOperatorObject _Nonnull       stateOperator,
    TQ3ViewObject _Nonnull                view
);



/*!
 *  @function
 *      Q3View_IsBoundingBoxVisible
 *  @abstract
 *      Test a bounding box for visibility.
 *
 *  @discussion
 *      The bounding box (assumed to be in local coordinates) is tested for
 *		intersection with the view frustum of the camera currently associated
 *		with the view.
 *
 *		This function could be used for visibility culling, either in a
 *		rendering loop of client code or within a renderer.
 *
 *  @param view             The view to check the bounding box against.
 *  @param bbox             The local bounding box to test.
 *  @result                 True or false as the bounding box is visible.
 */
Q3_EXTERN_API_C ( TQ3Boolean  )
Q3View_IsBoundingBoxVisible (
    TQ3ViewObject _Nonnull                view,
    const TQ3BoundingBox          * _Nonnull bbox
);



/*!
 *  @function
 *      Q3View_AllowAllGroupCulling
 *  @discussion
 *      Set the group culling state of a view.
 *
 *      If group culling is active, a view will skip groups whose bounding
 *      boxes are not visible.
 *
 *  @param view             The view to update.
 *  @param allowCulling     The new group culling state for the view.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_AllowAllGroupCulling (
    TQ3ViewObject _Nonnull                view,
    TQ3Boolean                    allowCulling
);



/*!
 *  @function
 *      Q3View_TransformLocalToWorld
 *  @discussion
 *      Transforms a point from local to world coordinates.
 *
 *      May only be called within a view submitting loop.
 *
 *      <em>This function is not available in QD3D.</em>
 *
 *  @param theView          The view currently being submitted to.
 *  @param localPoint       The point to transform, in local coordinates.
 *  @param worldPoint       The transformed point, in world coordinates.
 *  @result                 Success or failure of the operation.
 */
#if QUESA_ALLOW_QD3D_EXTENSIONS

Q3_EXTERN_API_C ( TQ3Status  )
Q3View_TransformLocalToWorld (
    TQ3ViewObject _Nonnull                theView,
    const TQ3Point3D              * _Nonnull localPoint,
    TQ3Point3D                    * _Nonnull worldPoint
);

#endif // QUESA_ALLOW_QD3D_EXTENSIONS



/*!
 *  @function
 *      Q3View_TransformLocalToWindow
 *  @discussion
 *      Transforms a point from local to window (pixel) coordinates.
 *
 *      May only be called within a view submitting loop.
 *
 *      <em>This function is not available in QD3D.</em>
 *
 *  @param theView          The view currently being submitted to.
 *  @param localPoint       The point to transform, in local coordinates.
 *  @param windowPoint      The transformed point, in window coordinates.
 *  @result                 Success or failure of the operation.
 */
#if QUESA_ALLOW_QD3D_EXTENSIONS

Q3_EXTERN_API_C ( TQ3Status  )
Q3View_TransformLocalToWindow (
    TQ3ViewObject _Nonnull                theView,
    const TQ3Point3D              * _Nonnull localPoint,
    TQ3Point2D                    * _Nonnull windowPoint
);

#endif // QUESA_ALLOW_QD3D_EXTENSIONS



/*!
 *  @function
 *      Q3View_TransformWorldToWindow
 *  @discussion
 *      Transforms a point from world to window (pixel) coordinates.
 *
 *      May only be called within a view submitting loop.
 *
 *      <em>This function is not available in QD3D.</em>
 *
 *  @param theView          The view currently being submitted to.
 *  @param worldPoint       The point to transform, in world coordinates.
 *  @param windowPoint      The transformed point, in window coordinates.
 *  @result                 Success or failure of the operation.
 */
#if QUESA_ALLOW_QD3D_EXTENSIONS

Q3_EXTERN_API_C ( TQ3Status  )
Q3View_TransformWorldToWindow (
    TQ3ViewObject _Nonnull                theView,
    const TQ3Point3D              * _Nonnull worldPoint,
    TQ3Point2D                    * _Nonnull windowPoint
);

#endif // QUESA_ALLOW_QD3D_EXTENSIONS



/*!
 *  @function
 *      Q3View_SetDrawContext
 *  @discussion
 *      Set the draw context for a view.
 *
 *      The reference count of the draw context will be incremented by the view.
 *
 *  @param view             The view to update.
 *  @param drawContext      The draw context to assign to the view.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_SetDrawContext (
    TQ3ViewObject _Nonnull                view,
    TQ3DrawContextObject _Nonnull         drawContext
);



/*!
 *  @function
 *      Q3View_GetDrawContext
 *  @discussion
 *      Get the draw context from a view.
 *
 *      The reference count of the draw context is incremented.
 *
 *  @param view             The view to query.
 *  @param drawContext      Receives the draw context associated with the view.
 *                          Dispose the reference when you are done with it.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetDrawContext (
    TQ3ViewObject _Nonnull                view,
    TQ3DrawContextObject _Nullable        * _Nonnull drawContext
);



/*!
 *  @function
 *      Q3View_GetLocalToWorldMatrixState
 *  @discussion
 *      Get the local-to-world matrix state from a view.
 *		Must be called within a submitting loop.
 *
 *  @param view             The view to query.
 *  @param matrix           Receives the local-to-world matrix.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetLocalToWorldMatrixState (
    TQ3ViewObject _Nonnull                view,
    TQ3Matrix4x4                  * _Nonnull matrix
);



/*!
 *  @function
 *      Q3View_GetWorldToFrustumMatrixState
 *  @discussion
 *      Get the world-to-frustum matrix state from a view.
 *
 *		This matrix can be used (via <code>Q3Point3D_Transform</code>) to transform points
 *		from world coordinates to frustum coordinates.  Frustum coordinates
 *		specify where a point falls within the viewing frustum.  In frustum
 *		space, the viewable area ranges in X from -1 (left) to 1 (right); in
 *		Y from -1 (bottom) to 1 (top), and in Z from 0 (near clipping plane)
 *		to -1 (far clipping plane).
 *
 *		Note that this matrix cannot sensibly transform a point which is at
 *		or behind the near clipping plane.
 *
 *		This matrix may be inverted for frustum-to-world transformations.
 *
 *		This function must be called within a submitting loop.  If you need
 *		this matrix when you are not in a submitting loop, use
 *		<code>Q3Camera_GetWorldToFrustum</code> instead.  However, this function may not return
 *		the same result as <code>Q3Camera_GetWorldToFrustum</code>, because
 *		<code>Q3Camera_GetWorldToFrustum</code> does not respect the action of a camera
 *		transform or a rasterize transform.
 *
 *		If the view's camera has a nonlinear projection (as in fisheye and all-seeing
 *		cameras), then the world to frustum transformation cannot be expressed
 *		as a matrix, hence this function will return <code>kQ3Failure</code>.
 *
 *  @param view             The view to query.
 *  @param matrix           Receives the world-to-frustum matrix.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetWorldToFrustumMatrixState (
    TQ3ViewObject _Nonnull                view,
    TQ3Matrix4x4                  * _Nonnull matrix
);



/*!
 *  @function
 *      Q3View_GetFrustumToWindowMatrixState
 *  @discussion
 *      Get the frustum-to-window matrix state from a view.
 *		Must be called within a submitting loop.
 *
 *		This matrix may be used (via Q3Point3D_Transform) to transform
 *		points from frustum coordinates into window coordinates.  See
 *		Q3View_GetWorldToFrustumMatrixState for a description of frustum
 *		coordinates.
 *
 *  @param view             The view to query.
 *  @param matrix           Receives the frustum-to-window matrix.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetFrustumToWindowMatrixState (
    TQ3ViewObject _Nonnull                view,
    TQ3Matrix4x4                  * _Nonnull matrix
);



/*!
	@function
		Q3View_GetIlluminationShaderState
	@discussion
		Get the current illumination shader state of a view.
		Must be called within a submitting loop.
	@param	theView		The view to query.
	@param	outType		Receives the type of the current illumination shader.
	@result             Success or failure of the operation.
*/
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetIlluminationShaderState(
		TQ3ViewObject _Nonnull theView,
		TQ3ObjectType* _Nonnull outType );



/*!
 *  @function
 *      Q3View_GetBackfacingStyleState
 *  @discussion
 *      Get the current backfacing style state from a view.
 *		Must be called within a submitting loop.
 *
 *  @param view             The view to query.
 *  @param backfacingStyle  Receives the current backfacing style state.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetBackfacingStyleState (
    TQ3ViewObject _Nonnull                view,
    TQ3BackfacingStyle            * _Nonnull backfacingStyle
);



/*!
 *  @function
 *      Q3View_GetInterpolationStyleState
 *  @discussion
 *      Get the current interpolation style state from a view.
 *		Must be called within a submitting loop.
 *
 *  @param view                 The view to query.
 *  @param interpolationType    Receives the current interpolation style state.
 *  @result                     Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetInterpolationStyleState (
    TQ3ViewObject _Nonnull                view,
    TQ3InterpolationStyle         * _Nonnull interpolationType
);



/*!
 *  @function
 *      Q3View_GetFillStyleState
 *  @discussion
 *      Get the current fill style state from a view.
 *		Must be called within a submitting loop.
 *
 *  @param view             The view to query.
 *  @param fillStyle        Receives the current fill style state.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetFillStyleState (
    TQ3ViewObject _Nonnull                view,
    TQ3FillStyle                  * _Nonnull fillStyle
);



/*!
 *  @function
 *      Q3View_GetHighlightStyleState
 *  @discussion
 *      Get the current highlight style state from a view.
 *		Must be called within a submitting loop.
 *
 *  @param view             The view to query.
 *  @param highlightStyle   Receives the current highlight style state (the attribute set, not the style
 *  						object).  If not NULL, this is a new
 *  						reference that you are responsible for disposing of.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetHighlightStyleState (
    TQ3ViewObject _Nonnull                view,
    TQ3AttributeSet _Nullable              * _Nonnull highlightStyle
);



/*!
 *  @function
 *      Q3View_GetSubdivisionStyleState
 *  @discussion
 *      Get the current subdivision style state from a view.
 *		Must be called within a submitting loop.
 *
 *  @param view             The view to query.
 *  @param subdivisionStyle Receives the current subdivision style state.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetSubdivisionStyleState (
    TQ3ViewObject _Nonnull                view,
    TQ3SubdivisionStyleData       * _Nonnull subdivisionStyle
);



/*!
 *  @function
 *      Q3View_GetOrientationStyleState
 *  @discussion
 *      Get the current orientation style state from a view.
 *		Must be called within a submitting loop.
 *
 *  @param view                        The view to query.
 *  @param fontFacingDirectionStyle    Receives the current orientation style state.
 *  @result                            Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetOrientationStyleState (
    TQ3ViewObject _Nonnull                view,
    TQ3OrientationStyle           * _Nonnull fontFacingDirectionStyle
);



/*!
 *  @function
 *      Q3View_GetCastShadowsStyleState
 *  @discussion
 *      Get the current cast shadows style state from a view.
 *		Must be called within a submitting loop.
 *
 *      <em>This function is not available in QD3D.</em>
 *
 *  @param view             The view to query.
 *  @param castShadows      Receives the current cast shadows style state.
 *  @result                 Success or failure of the operation.
 */
#if QUESA_ALLOW_QD3D_EXTENSIONS

Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetCastShadowsStyleState (
    TQ3ViewObject _Nonnull                view,
    TQ3Boolean                    * _Nonnull castShadows
);

#endif // QUESA_ALLOW_QD3D_EXTENSIONS



/*!
 *  @function
 *      Q3View_GetReceiveShadowsStyleState
 *  @discussion
 *      Get the current receive shadows style state from a view.
 *		Must be called within a submitting loop.
 *
 *  @param view             The view to query.
 *  @param receiveShadows   Receives the current receive shadows style state.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetReceiveShadowsStyleState (
    TQ3ViewObject _Nonnull                view,
    TQ3Boolean                    * _Nonnull receiveShadows
);



/*!
 *  @function
 *      Q3View_GetPickIDStyleState
 *  @discussion
 *      Get the current pick ID style state from a view.
 *		Must be called within a submitting loop.
 *
 *  @param view             The view to query.
 *  @param pickIDStyle      Receives the current pick ID style state.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetPickIDStyleState (
    TQ3ViewObject _Nonnull                view,
    TQ3Uns32                      * _Nonnull pickIDStyle
);



/*!
 *  @function
 *      Q3View_GetPickPartsStyleState
 *  @discussion
 *      Get the current pick parts style state from a view.
 *		Must be called within a submitting loop.
 *
 *  @param view             The view to query.
 *  @param pickPartsStyle   Receives the current pick parts style state.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetPickPartsStyleState (
    TQ3ViewObject _Nonnull                view,
    TQ3PickParts                  * _Nonnull pickPartsStyle
);



/*!
 *  @function
 *      Q3View_GetAntiAliasStyleState
 *  @discussion
 *      Get the current anti-alias style state from a view.
 *		Must be called within a submitting loop.
 *
 *  @param view             The view to query.
 *  @param antiAliasData    Receives the current anti-alias style state.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetAntiAliasStyleState (
    TQ3ViewObject _Nonnull                view,
    TQ3AntiAliasStyleData         * _Nonnull antiAliasData
);



/*!
 *  @function
 *      Q3View_GetFogStyleState
 *  @discussion
 *      Get the current fog style associated with a view.
 *		Must be called within a submitting loop.
 *
 *      <em>This function is not available in QD3D.</em>
 *
 *  @param theView          The view to query.
 *  @param fogData          Receives the current fog style state.
 *  @result                 Success or failure of the operation.
 */
#if QUESA_ALLOW_QD3D_EXTENSIONS

Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetFogStyleState (
    TQ3ViewObject _Nonnull                view,
    TQ3FogStyleData               * _Nonnull fogData
);

#endif // QUESA_ALLOW_QD3D_EXTENSIONS


#if QUESA_ALLOW_QD3D_EXTENSIONS

/*!
	@function	Q3View_GetDepthRangeStyleState
	@abstract	Get the current depth range state.
	@discussion	Must be called within a submitting loop.
	@param		view		The view to query.
	@param		outData		Receives the depth range data.
	@result     Success or failure of the operation.
*/
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetDepthRangeStyleState (
    TQ3ViewObject _Nonnull    view,
    TQ3DepthRangeStyleData* _Nonnull outData
);

#endif // QUESA_ALLOW_QD3D_EXTENSIONS


#if QUESA_ALLOW_QD3D_EXTENSIONS

/*!
	@function	Q3View_GetWriteSwitchStyleState
	@abstract	Get the current write switch state.
	@discussion	Must be called within a submitting loop.
	@param		view		The view to query.
	@param		outMask		Receives the write switch mask.  See TQ3WriteSwitchMasks.
	@result     Success or failure of the operation.
*/
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetWriteSwitchStyleState (
    TQ3ViewObject _Nonnull    view,
    TQ3Uns32* _Nonnull outMask
);

#endif // QUESA_ALLOW_QD3D_EXTENSIONS

#if QUESA_ALLOW_QD3D_EXTENSIONS

/*!
	@function	Q3View_GetDepthCompareStyleState
	@abstract	Get the current depth compare state.
	@discussion	Must be called within a submitting loop.
	@param		view		The view to query.
	@param		outFunc		Receives the depth compare function.
	@result     Success or failure of the operation.
*/
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetDepthCompareStyleState (
    TQ3ViewObject _Nonnull    view,
    TQ3DepthCompareFunc* _Nonnull outFunc
);

#endif // QUESA_ALLOW_QD3D_EXTENSIONS


/*!
 *  @function
 *      Q3View_GetDefaultAttributeSet
 *  @discussion
 *      Get the default attribute set from a view.
 *
 *      The reference count of the attribute set is incremented.
 *
 *  @param view             The view to query.
 *  @param attributeSet     Receives the default attribute set.
 *                          Dispose the reference when you are done with it.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetDefaultAttributeSet (
    TQ3ViewObject _Nonnull                view,
    TQ3AttributeSet _Nullable             * _Nonnull attributeSet
);



/*!
 *  @function
 *      Q3View_SetDefaultAttributeSet
 *  @discussion
 *      Set the default attribute set for a view.
 *
 *      The reference count of the attribute set will be incremented by the view.
 *
 *  @param view             The view to update.
 *  @param attributeSet     The attribute set to assign to the view.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_SetDefaultAttributeSet (
    TQ3ViewObject _Nonnull                view,
    TQ3AttributeSet _Nonnull              attributeSet
);



/*!
 *  @function
 *      Q3View_GetAttributeSetState
 *  @discussion
 *      Get the current attribute state set from a view.
 *
 *  @param view             The view to query.
 *  @param attributeSet     Receives the current attribute state set.
 *                          Dispose the reference when you are done with it.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetAttributeSetState (
    TQ3ViewObject _Nonnull                view,
    TQ3AttributeSet  _Nullable            * _Nonnull attributeSet
);



/*!
 *  @function
 *      Q3View_GetAttributeState
 *  @discussion
 *      Get the current state of an attribute from a view.
 *
 *		This function returns a pointer to data within an attribute state
 *		representing the current attributes in the view.  You should consider
 *		the pointer to be read-only and temporary.  It is safer and less
 *		error-prone to get the attribute set with <code>Q3View_GetAttributeSetState</code>
 *		and then extract data with <code>Q3AttributeSet_Get</code>.
 *
 *  @param view             The view to query.
 *  @param attributeType    The attribute type to query.
 *  @param data             Receives the address of the current value for the specified attribute.
 *  @result                 Success or failure of the operation.
 */
Q3_EXTERN_API_C ( TQ3Status  )
Q3View_GetAttributeState (
    TQ3ViewObject _Nonnull                view,
    TQ3AttributeType              attributeType,
    void                          * _Nonnull data
);





//=============================================================================
//      C++ postamble
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif


