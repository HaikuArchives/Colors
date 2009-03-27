/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#include "ContainersView.h"

#include <Application.h>
#include <Bitmap.h>
#include <iostream.h>
#include <Resources.h>
#include <Window.h>

#include "ColorWindow.h"
#include "Container.h"
#include "TextContainer.h"
#include "Prefs.h"

#define COLOR_TO_INT(c) (c.red<<16) + (c.green<<8) + (c.blue)

ContainersView::ContainersView( BRect rect )

:	BView( rect, "", B_FOLLOW_TOP|B_FOLLOW_LEFT, B_WILL_DRAW|B_DRAW_ON_CHILDREN),
	fMouseDown( false )

{

	SetViewColor(230, 230, 230);

	for (int i=0; i<10; ++i) {
		fColorContainer[i] = new Container( BRect(0.0, 0.0, 19.0, 19.0).OffsetToCopy(10.0 + 23.0 * i, 6.0));
		fColorContainer[i]->SetColor( prefs.FindInt32("color_containers", i) );
		AddChild( fColorContainer[i] );
	}

	long int prefs_color;
	
	fTextContainer = new TextContainer( BRect(0.0, 0.0, 150.0, 29.0).OffsetToCopy(260.0, 1.0) );

	if (prefs.FindInt32("foreground_container", &prefs_color)!=B_OK) prefs_color = 0x000000;
	fTextContainer->SetColor( true, prefs_color );
	
	if (prefs.FindInt32("background_container", &prefs_color)!=B_OK) prefs_color = 0xffffff;
	fTextContainer->SetColor( false, prefs_color );

	AddChild( fTextContainer );

}

ContainersView::~ContainersView() {

	prefs.RemoveName("color_containers");

	for (int i=0; i<10; ++i) {
		prefs.AddInt32("color_containers",
			COLOR_TO_INT(fColorContainer[i]->GetColor()));
	}
	
	prefs.RemoveName("foreground_container");
	prefs.AddInt32("foreground_container",
		COLOR_TO_INT(fTextContainer->GetColor(true)));

	prefs.RemoveName("background_container");
	prefs.AddInt32("background_container",
		COLOR_TO_INT(fTextContainer->GetColor(false)));

}

void ContainersView::MouseDown(BPoint where) {

	Window()->Activate();
	
	if (Window()->CurrentMessage()->FindInt32("buttons")==B_SECONDARY_MOUSE_BUTTON) {

		BPoint	pt;
		Window()->CurrentMessage()->FindPoint("where", &pt);
		ConvertToScreen( &pt );

		BMessage	msg(MSG_POPUP_MENU);
		msg.AddPoint("where", pt);
		Window()->PostMessage(&msg);
	}
	else {
		fMouseDown = true;
		fMouseOffset = where;
	
		SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY|B_SUSPEND_VIEW_FOCUS|B_LOCK_WINDOW_FOCUS );
	
		BView::MouseDown( where );
	}
}

void ContainersView::MouseUp(BPoint where) {
	fMouseDown = false;
	BView::MouseUp( where );
}

void ContainersView::MouseMoved( BPoint where, uint32 code, const BMessage *a_message) {
	
	if (fMouseDown) {

		BPoint win_pos = Window()->Frame().LeftTop();
		Window()->MoveTo( win_pos.x + where.x - fMouseOffset.x, win_pos.y + where.y - fMouseOffset.y );

	}
	else {
		BView::MouseMoved( where, code, a_message);
	}
}

void ContainersView::Draw( BRect updateRect ) {
	
	BView::Draw( updateRect );

	SetDrawingMode( B_OP_ALPHA );
	SetHighColor(0, 0, 0, 100);
	StrokeLine( Bounds().RightTop(), Bounds().LeftTop() );
	StrokeLine( Bounds().LeftBottom() );
	SetHighColor(255, 255, 255, 200);
	StrokeLine( Bounds().RightBottom() );
	StrokeLine( Bounds().RightTop() );
	SetDrawingMode( B_OP_COPY );
}
