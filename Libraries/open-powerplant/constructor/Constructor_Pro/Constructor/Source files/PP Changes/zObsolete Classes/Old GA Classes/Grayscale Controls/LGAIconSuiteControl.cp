// Copyright �2005, 2006 Freescale Semiconductor, Inc.
// Please see the License for the specific language governing rights and
// limitations under the License.
/*	Copyright� 1995 - 1997 Metrowerks Corp. All rights reserved

---------------------------------------<< � >>-----------------------------------------

FILE:					LGAIconSuiteControl.cp

CLASSES:				LGAIconSuiteControl

DESCRIPTION:		Implementation file for LGAIconSuiteControl

AUTHOR:				Robin Mair

CREATION DATE :	96.06.07

CHANGE HISTORY :

		97.04.20		rtm	Removed the CreateFromStream method.
		97.04.04		rtm	Removed the call to DisposeIconSuite from the destructor as
								this has been moved into the LGAIconMixin class.
		97.02.11		rtm	Fixed a bug in ActivateSelf where the correct transform was not
								being calculated based on the enabled state of the control, this
								was causing an diabled control to be drawn enabled.
		97.02.08		rtm	Changed the rendering of the control such that is it drawn as
								disabled when it is deactivated, this is matches the approach
								Apple is taking with the Apearance Manager.
		97.01.24		rtm	Added a copy constructor.
		96.01.20		rtm	Added a parameterized constructor so that the pane can be built
								procedurally.
		96.12.16		rtm	Changed the constructors so that the mHiliteIcon flag is set
								based on the control mode setting instead of getting it from
								the stream.
		96.12.11		rtm	Got rid of the locally defined enum for the control modes this
								class now uses the ones defined in the UGraphicsUtilities.
		96.10.21		rtm	Removed override of AdaptToSuperFrameSize.
		96.10.19		rtm	Optimized the CalcIconLocation method so that the icon width
								was only being calculated once. Also added a FinishCreateSelf
								method in which the loading and setup of the icon location
								is handled.
		96.10.08		rtm	Fixed the CalcIconLocation method so that it now correctly
								handles the width for a mini icon. Also fixed CalcLocalIconRect
								for the same problem.
		96.10.08		rtm	Got rid of DrawIconSuiteControl by moving its contents to
								DrawSelf.
		96.10.01		rtm	Added an override to MoveBy so that we can recalulate the icons
								location.
		96.09.05		rtm	Changed the constructors so that they now use the function
								provided by the icon mixin for loading and icon suite handle.
		96.09.04		rtm	PowerPlant conformance changes.
		96.09.02		rtm	Fixed a problem where the transform was not being setup
								correctly when the control was hilited and the disabled.
		96.08.05		rtm	Fixed a bug where the hilite flag was not getting setup in the
								various constructors
		96.06.26		rtm	Added override of ResizeFrameBy so that we can call 
								CalcIconLocation() whenever the zize changes.
		96.06.20		rtm	Changed the SetHiliteState method so that it now handles the
								the redraw after a state change instead of all the other 
								places that were trying to take care of it.
		96.06.17		rtm	Changed the drawing of the icons so that they are now drawn
								from a cached icon suite handle instead of calling PlotIconID
								which was very slow.  By default we are jsut oading all of the
								icon data.
		96.06.07		rtm	Initial creation of file

---------------------------------------<< � >>-----------------------------------------
*/

#ifdef PowerPlant_PCH
	#include PowerPlant_PCH
#endif

// � GRAYSCALE HEADERS
#include <LGAIconSuiteControl.h>
#include <UGraphicsUtilities.h>

// � POWERPLANT HEADERS
#include <LStream.h>
#include <UDrawingState.h>
#include <UDrawingUtils.h>
#include <PP_Messages.h>

// � TOOLBOX HEADERS
#include <Icons.h>


//=====================================================================================
// CONSTANTS
//=====================================================================================

//=====================================================================================
// DEFINES
//=====================================================================================

#pragma mark === CLASS: LGAIconSuiteControl

//=====================================================================================
// CLASS:	LGAIconSuiteControl
//=====================================================================================

#pragma mark -
#pragma mark === INITIALIZATION

//=====================================================================================
// �� INITIALIZATION & DESTRUCTION
//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::LGAIconSuiteControl					---<<� CONSTRUCTOR �>>---
//-------------------------------------------------------------------------------------
//
//	Default constructor

LGAIconSuiteControl::LGAIconSuiteControl ()
{	

	mControlMode = controlMode_Button;
	mHilited = false;
	
}	//	LGAIconSuiteControl::LGAIconSuiteControl


//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::LGAIconSuiteControl					---<<� CONSTRUCTOR �>>---
//-------------------------------------------------------------------------------------
//
//	Copy constructor

LGAIconSuiteControl::LGAIconSuiteControl 	(	const LGAIconSuiteControl &inOriginal )
									:	LControl ( inOriginal ),
										LGAIconMixin ( inOriginal )
{	

	mControlMode = inOriginal.mControlMode;
	mHilited = inOriginal.mHilited;
	
}	//	LGAIconSuiteControl::LGAIconSuiteControl


//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::LGAIconSuiteControl					---<<� CONSTRUCTOR �>>---
//-------------------------------------------------------------------------------------
//
//	Stream constructor

LGAIconSuiteControl::LGAIconSuiteControl (	LStream	*inStream ) 
										: LControl ( inStream )
{
	SInt16 controlMode;
	SInt16 sizeSelector;
	SInt16	position;
	
	// � Setup some default values
	mEdgeOffset = 0;
		
	// � Read in the data from the stream
	*inStream >> controlMode;
	mControlMode = (EControlMode)controlMode;
	*inStream >> mIconSuiteID;
	*inStream >> sizeSelector;
	mSizeSelector = (ESizeSelector)sizeSelector;
	*inStream >> position;
	mIconPlacement = (EIconPosition)position;
	*inStream >> mHiliteIcon;
	mOffsetIconOnHilite = false;
	*inStream >> mClickInIcon;
	
	// � If the control mode is some form of button then we need to make
	// sure that the max value gets set to at least one
	if ( mControlMode > 0 && GetMaxValue () < 1 )
		mMaxValue = 1;
		
	// � If there is no control mode then we need to make sure that
	// the hilite icon flag is turned off
	if ( mControlMode == controlMode_None )
		mHiliteIcon = false;
	else
		mHiliteIcon = true;
		
	// � Get the hiliting correctly setup to match the state passed in
	mHilited = GetValue () != 0;	

}	//	LGAIconSuiteControl::LGAIconSuiteControl


//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::LGAIconSuiteControl					---<<� CONSTRUCTOR �>>---
//-------------------------------------------------------------------------------------
//
//	Parameterized constructor

LGAIconSuiteControl::LGAIconSuiteControl (	const SPaneInfo 			&inPaneInfo,
															const SControlInfo 		&inControlInfo,
															EControlMode				inControlMode,
															ResIDT						inIconSuiteID,		
															ESizeSelector				inSizeSelector,			
															EIconPosition				inIconPlacement,	
															Boolean						inHiliteIcon,	
															Boolean						inClickInIcon )
											: LControl ( 	inPaneInfo,
																inControlInfo.valueMessage,
																inControlInfo.value,
																inControlInfo.minValue,
																inControlInfo.maxValue )
{	

	// � Setup all the icon fields
	mControlMode = inControlMode;
	mIconSuiteID = inIconSuiteID;
	mSizeSelector = inSizeSelector;
	mIconPlacement = inIconPlacement;
	mIconLocation = gEmptyPoint;
	mHiliteIcon = inHiliteIcon;
	mOffsetIconOnHilite = false;
	mClickInIcon = inClickInIcon;
	
	// � If the control mode is some form of button then we need to make
	// sure that the max value gets set to at least one
	if ( mControlMode > 0 && GetMaxValue () < 1 )
		mMaxValue = 1;
		
	// � If there is no control mode then we need to make sure that
	// the hilite icon flag is turned off
	if ( mControlMode == controlMode_None )
		mHiliteIcon = false;
	else
		mHiliteIcon = true;
		
	// � Make sure the hilited flag gets setup correctly from any value
	// that is currently set
	mHilited = GetValue () != 0;	
		
}	//	LGAIconSuiteControl::LGAIconSuiteControl


//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::LGAIconSuiteControl					---<<� CONSTRUCTOR �>>---
//-------------------------------------------------------------------------------------
//
//	Parameterized constructor

LGAIconSuiteControl::LGAIconSuiteControl (	const SPaneInfo 			&inPaneInfo,
															const SControlInfo 		&inControlInfo,
															const SIconSuiteInfo 	&inIconInfo,
															EControlMode				inControlMode )
										: LControl ( 	inPaneInfo,
															inControlInfo.valueMessage,
															inControlInfo.value,
															inControlInfo.minValue,
															inControlInfo.maxValue )
{	

	// � Setup all the icon fields
	mControlMode = inControlMode;
	mIconSuiteID = inIconInfo.iconSuiteID;
	mTransform = inIconInfo.transform;
	mSizeSelector = inIconInfo.sizeSelector;
	mIconPlacement = inIconInfo.iconPlacement;
	mIconLocation = gEmptyPoint;
	mEdgeOffset = inIconInfo.edgeOffset;
	mHiliteIcon = inIconInfo.hiliteIcon;
	mOffsetIconOnHilite = false;
	mClickInIcon = inIconInfo.clickInIcon;
	
	// � If the control mode is some form of button then we need to make
	// sure that the max value gets set to at least one
	if ( mControlMode > 0 && GetMaxValue () < 1 )
		mMaxValue = 1;
		
	// � If there is no control mode then we need to make sure that
	// the hilite icon flag is turned off
	if ( mControlMode == controlMode_None )
		mHiliteIcon = false;
	else
		mHiliteIcon = true;
		
	// � Make sure the hilited flag gets setup correctly from any value
	// that is currently set
	mHilited = GetValue () != 0;	
		
}	//	LGAIconSuiteControl::LGAIconSuiteControl


//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::~LGAIconSuiteControl				---<<� DESTRUCTOR �>>---
//-------------------------------------------------------------------------------------

LGAIconSuiteControl::~LGAIconSuiteControl ()
{
}	//	LGAIconSuiteControl::~LGAIconSuiteControl


//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::FinishCreateSelf
//-------------------------------------------------------------------------------------

void LGAIconSuiteControl::FinishCreateSelf ()
{

	// � Load and cache the icon suite
	LoadIconSuiteHandle ();
		
	// � If the icon is not enabled we need to make sure that
	// the transform reflects that
	mTransform = mEnabled == triState_Off ? ttDisabled : ttNone;
			
	// � Finally, we need to get the icon location calculated
	CalcIconLocation ();	
	
}	//	LGAIconSuiteControl::FinishCreateSelf


#pragma mark -
#pragma mark === ACCESSORS

//=====================================================================================
// �� ACCESSORS
//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::CalcLocalIconRect
//-------------------------------------------------------------------------------------

void
LGAIconSuiteControl::CalcLocalIconRect ( Rect &outRect ) const
{
	// � As a safety measure we will always first make sure that the
	// icon location has been updated just in case something about our
	// position in the universe changed
	const void CalcIconLocation ();
	
	// � Setup the rectangle based on the location and size of the icon
	outRect.top = mIconLocation.v;
	outRect.left = mIconLocation.h;
	outRect.bottom = mIconLocation.v + mSizeSelector;
	outRect.right = mIconLocation.h + (mSizeSelector == 12 ? 16 : mSizeSelector);
	
	// � If the button is hilited and the offset flag is set then
	// we need to offset the rect byt one pixel to the bottom and right
	if ( mHilited && mOffsetIconOnHilite )
		::OffsetRect ( &outRect, 1, 1 );
	
}	//	LGAIconSuiteControl::CalcLocalIconRect


//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::SetHiliteState
//-------------------------------------------------------------------------------------

void			
LGAIconSuiteControl::SetHiliteState	(	Boolean	inHiliteState )
{
	
	// � We only change the state if we have to
	if ( mHilited != inHiliteState )
	{
		mHilited = inHiliteState;
		
		// � Things have changed so we need to make sure that the
		// transform is in synch with the change
		if ( mHiliteIcon )
			mTransform = inHiliteState && IsEnabled () ? ttSelected : 
										(IsEnabled () ? ttNone : 
											(inHiliteState ? ttSelected + ttDisabled : ttDisabled));
			
		// � Because the state has changed we need to make sure 
		// that things get redrawn so that the change can take effect
		Draw ( nil );
	}
			
}	//	LGAIconSuiteControl::SetHiliteState


//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::SetValue
//-------------------------------------------------------------------------------------
//

void
LGAIconSuiteControl::SetValue	(	SInt32	inValue )
{
	// �  Only change the value if we have too
	if ( mValue != inValue )
	{
		// � Get the value setup first
		LControl::SetValue ( inValue );
		
		// � And make sure we get the hilite state changed
		SetHiliteState ( inValue == Button_Off ? false : true );
		
		// � Broadcast the fact that we have turned the button on if that is the case
		if ( inValue == Button_On ) 
			BroadcastMessage ( msg_ControlClicked, (void*) this );
	}
	
}	//	LGAIconSuiteControl::SetValue


#pragma mark -
#pragma mark === ENABLING & DISABLING

//=====================================================================================
// �� ENABLING & DISABLING
//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::EnableSelf
//-------------------------------------------------------------------------------------
//
//	This has been overridden so that we can do the correct thing about getting the
//		transform to match the state of the control

void
LGAIconSuiteControl::EnableSelf ()
{

 	if ( mEnabled == triState_On )
	{
		SInt16 oldTransform = GetIconTransform ();
		
		if ( IsHilited () && mHiliteIcon )
			mTransform = ttSelected;
		else
			mTransform = ttNone;
			
		// � If the transform actually changed then make sure that
		// we get the button redrawn
		if ( oldTransform != mTransform )
			Draw ( nil );
	}

}	//	LGAIconSuiteControl::EnableSelf


//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::DisableSelf
//-------------------------------------------------------------------------------------
//
//	This has been overridden so that we can do the correct thing about getting the
//		transform to match the state of the control

void
LGAIconSuiteControl::DisableSelf ()
{

	// � Always just set the transform to be disabled
	if ( mEnabled == triState_Off || mEnabled == triState_Latent )
	{
		SInt16	oldTransform = GetIconTransform ();
		
		// � Add the disabled transform to the current one this will
		// allow us to display a selected icon as dimmed
		mTransform += ttDisabled;

		// � If the transform actually changed then make sure that
		// we get the button redrawn
		if ( oldTransform != mTransform )
			Draw ( nil );
	}
	
}	//	LGAIconSuiteControl::DisableSelf


#pragma mark -
#pragma mark === ACTIVATION

//=====================================================================================
// �� ACTIVATION
//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::ActivateSelf
//-------------------------------------------------------------------------------------
//
//	This has been overridden so that we can do the correct thing about getting the
//		transform to match the state of the control

void
LGAIconSuiteControl::ActivateSelf ()
{
	
	Rect	localFrame;
	CalcLocalFrameRect ( localFrame );
	
	// � Setup a device loop so that we can handle drawing at the correct bit depth
	StDeviceLoop	theLoop ( localFrame );
	SInt16				depth;
	while ( theLoop.NextDepth ( depth )) 
	{
		if ( depth > 4 )
		{
		 	if ( mActive == triState_On )
			{
				SInt16 oldTransform = GetIconTransform ();
				
				if ( mEnabled == triState_On )
				{
					if ( IsHilited () && mHiliteIcon )
						mTransform = ttSelected;
					else
						mTransform = ttNone;
				}
				else
					mTransform = ttDisabled;
					
				// � If the transform actually changed then make sure that
				// we get the button redrawn
				if ( oldTransform != mTransform )
					Refresh ();
			}
		}
	}
	
}	//	LGAIconSuiteControl::ActivateSelf


//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::DeactivateSelf
//-------------------------------------------------------------------------------------
//
//	This has been overridden so that we can do the correct thing about getting the
//		transform to match the state of the control

void
LGAIconSuiteControl::DeactivateSelf ()
{

	Rect	localFrame;
	CalcLocalFrameRect ( localFrame );
	
	// � Setup a device loop so that we can handle drawing at the correct bit depth
	StDeviceLoop	theLoop ( localFrame );
	SInt16				depth;
	while ( theLoop.NextDepth ( depth )) 
	{
		if ( depth > 4 )
		{
			// � Always just set the transform to be disabled
			if ( mActive == triState_Off || mActive == triState_Latent )
			{
				SInt16	oldTransform = GetIconTransform ();
				
				// � Add the disabled transform to the current one this will
				// allow us to display a selected icon as dimmed
				mTransform = ttDisabled;

				// � If the transform actually changed then make sure that
				// we get the button redrawn
				if ( oldTransform != mTransform )
					Refresh ();
			}
		}
	}
	
}	//	LGAIconSuiteControl::DeactivateSelf


#pragma mark -
#pragma mark === RESIZING

//=====================================================================================
// �� RESIZING
// ------------------------------------------------------------------------------------
//		LGAIconSuiteControl::ResizeFrameBy
// ------------------------------------------------------------------------------------
//

void
LGAIconSuiteControl::ResizeFrameBy ( 	SInt16		inWidthDelta,
										SInt16		inHeightDelta,
										Boolean		inRefresh )
{
	
	// � Pas the call on to our superclass
	LControl::ResizeFrameBy ( inWidthDelta, inHeightDelta, inRefresh );
	
	// � Any time we might undergo a size change we need to make sure that
	// we re-calculate the icon location
	CalcIconLocation ();
	
}	//	LGAIconSuiteControl::ResizeFrameBy


//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::MoveBy
//-------------------------------------------------------------------------------------

void
LGAIconSuiteControl::MoveBy ( 	SInt32		inHorizDelta,
											SInt32		inVertDelta,
											Boolean	inRefresh )
{
	
	// � Pass the call on to our superclass
	LControl::MoveBy ( inHorizDelta, inVertDelta, inRefresh );
	
	// � Any time we might undergo a size change we need to make sure that
	// we re-calculate the icon location
	CalcIconLocation ();
	
}	//	LGAIconSuiteControl::MoveBy


#pragma mark -
#pragma mark === MOUSE TRACKING

//=====================================================================================
// �� MOUSE TRACKING
//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::HotSpotAction
//-------------------------------------------------------------------------------------

void
LGAIconSuiteControl::HotSpotAction	(	SInt16		/* inHotSpot */,
											Boolean	inCurrInside,
											Boolean	inPrevInside )
{

	// � If the mouse moved in or out of the hot spot handle the
	// hiliting of the control which is slightly different for
	// each of the modes supported by the control
	if ( inCurrInside != inPrevInside )
	{
		switch ( GetControlMode ())
		{
			case controlMode_Button:
				// � For a straight button all we have to worry about
				// is switching the hiliting of the button
				SetHiliteState ( inCurrInside );
			break;
		
			case controlMode_RadioButton:
				// � Things are a little trickier if we ar a radio button
				// as we only need to toggle the state if the control is
				// not already selected, we also want to avoid doing any
				// unnecessary drawing
				if ( inCurrInside )
				{
					if ( !IsHilited ())
						SetHiliteState ( true );
				}
				else if ( !IsSelected ())
					SetHiliteState ( false );
			break;
			
			case controlMode_Switch:
				// � If the control is a switch we toggle the hiliting
				// from whatever its current setting is
				SetHiliteState ( !IsHilited ());
			break;
			
			default:
			break;
		}
	}
		
}	//	LGAIconSuiteControl::HotSpotAction


//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::HotSpotResult
//-------------------------------------------------------------------------------------

void
LGAIconSuiteControl::HotSpotResult	(	SInt16 inHotSpot )
{
	
	// � Make sure that by default we turn the hiliting off
	// if we are in button or radio mode, switch mode is handled
	// in the HotSpotAction method
	if ( mControlMode == controlMode_Button )
		HotSpotAction ( inHotSpot, false, true );
	else if ( mControlMode == controlMode_RadioButton )
		HotSpotAction ( inHotSpot, true, true );
		
	// � Do the right thing about getting the value set after a control has been
	// clicked in its hotspot, NOTE: this method assumes the entire control is
	// the hotspot
	switch ( GetControlMode ())
	{
		case controlMode_Button:
			BroadcastValueMessage ();	
		break;
		
		case controlMode_Switch:
			// � If the control is a switch we toggle
			// the value field of the control to indicate that
			// it is on or off
			SetValue ( 1 - GetValue ());
		break;
		
		case controlMode_RadioButton:
			// � If the control is not currently selected then we need to
			// ensure that the control is 'on', once it is on we never
			// change it back as it is a radio button and they only get
			// turned off by someone else when another radio button in
			// a group is selected
			if ( !IsSelected ())
			{
				SetValue ( Button_On );
			}
		break;
		
		default:
		break;
	}
	
}	//	LGAIconSuiteControl::HotSpotResult


#pragma mark -
#pragma mark === DRAWING

//=====================================================================================
// �� DRAWING
//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::DrawSelf
//-------------------------------------------------------------------------------------

void
LGAIconSuiteControl::DrawSelf	()
{

	StColorPenState::Normalize ();

	// � Get the icon rectangle
	Rect iconRect;
	CalcLocalIconRect ( iconRect );
	
	// �  Now we plot the icon by calling the toolbox passing it our cached
	// handle to the icon suite
	if ( GetIconSuiteH ())
		::PlotIconSuite ( &iconRect, atNone, mTransform, GetIconSuiteH () );
	
}	//	LGAIconSuiteControl::DrawSelf


#pragma mark -
#pragma mark === MISCELLANEOUS

//=====================================================================================
// �� MISCELLANEOUS
//-------------------------------------------------------------------------------------
// LGAIconSuiteControl::CalcIconLocation
//-------------------------------------------------------------------------------------

void
LGAIconSuiteControl::CalcIconLocation ()
{

	Rect	localFrame;
	SInt16	width, height;
	
	// � Get the local inset frame rect
	CalcLocalFrameRect ( localFrame );
	mIconLocation.h = localFrame.left;
	mIconLocation.v = localFrame.top;
	
	// � Setup the width and height values
	width = localFrame.right - localFrame.left;
	height = localFrame.bottom - localFrame.top;
	
	// � Calculate the icon width based on the size selector
	SInt16		iconWidth = mSizeSelector == 12 ? 16 : mSizeSelector;
	
	// � Now we calculate the location of the icon based on the icon placement enum
	switch ( mIconPlacement )
	{
		case iconPosition_None:
		case iconPosition_TopLeft:
			mIconLocation.h += mEdgeOffset;
			mIconLocation.v += mEdgeOffset;
		break;
		
		case iconPosition_TopCenter:
			mIconLocation.h += ( width / 2 ) - ( iconWidth / 2 );
			mIconLocation.v += mEdgeOffset;
		break;
		
		case iconPosition_TopRight:
			mIconLocation.h += width - ( iconWidth + mEdgeOffset );
			mIconLocation.v += mEdgeOffset;
		break;
		
		case iconPosition_RightCenter:
			mIconLocation.h += width - ( iconWidth + mEdgeOffset );
			mIconLocation.v += ( height / 2 ) - ( mSizeSelector / 2 );
		break;
		
		case iconPosition_BottomRight:
			mIconLocation.h += width - ( iconWidth + mEdgeOffset );
			mIconLocation.v += height - ( mSizeSelector + mEdgeOffset );
		break;
		
		case iconPosition_BottomCenter:
			mIconLocation.h += ( width / 2 ) - ( iconWidth / 2 );
			mIconLocation.v += height - ( mSizeSelector + mEdgeOffset );
		break;
		
		case iconPosition_BottomLeft:
			mIconLocation.h += mEdgeOffset;
			mIconLocation.v += height - ( mSizeSelector + mEdgeOffset );
		break;
		
		case iconPosition_LeftCenter:
			mIconLocation.h += mEdgeOffset;
			mIconLocation.v += ( height / 2 ) - ( mSizeSelector / 2 );
		break;
		
		case iconPosition_Center:
			mIconLocation.h += ( width / 2 ) - ( iconWidth / 2 );
			mIconLocation.v += ( height / 2 ) - ( mSizeSelector / 2 );
		break;
		
	};
	
}	//	LGAIconSuiteControl::CalcIconLocation


//=====================================================================================
// �� MISCELLANEOUS
// ---------------------------------------------------------------------------
//	LGAIconSuiteControl::PointIsInFrame
// ---------------------------------------------------------------------------

Boolean
LGAIconSuiteControl::PointIsInFrame ( 	SInt32	inHorizPort,
													SInt32	inVertPort ) const
{
	// � If the option to handle the click only in the region occupied
	// by the icon is on then handle that first otherwise we just go
	// to the standard version of this method which handles a click in
	// the pane's frame
	if ( mClickInIcon )
	{
		Point	portPt = { inVertPort, inHorizPort };
		PortToLocalPoint ( portPt );

		// � As a safety measure we will always first make sure that the
		// icon location has been updated just in case something about our
		// position in the universe changed
		const void CalcIconLocation ();
		
		// � Setup the rectangle based on the location and size of the icon
		Rect	iconRect;
		iconRect.top = mIconLocation.v;
		iconRect.left = mIconLocation.h;
		iconRect.bottom = mIconLocation.v + mSizeSelector;
		iconRect.right = mIconLocation.h + (mSizeSelector == 12 ? 16 : mSizeSelector);
		
		return PtInIconID ( portPt, &iconRect, atNone, mIconSuiteID );
	}
	else
		return LControl::PointIsInFrame ( inHorizPort, inVertPort );
	
}


