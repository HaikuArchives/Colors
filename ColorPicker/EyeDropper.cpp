/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#include "EyeDropper.h"

#include <Application.h>
#include <Bitmap.h>
#include <iostream>
#include <Resources.h>
#include <Window.h>

BBitmap	*FetchImage(const char *imgname);

EyeDropper::EyeDropper( BRect frame )
:	BControl( frame, "eyedropper", "", new BMessage(MSG_EYEDROPPER),
		B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW ),
	fMouseDown( false ),
	fButtonDown( false )

{
	fImage[0] = FetchImage("pipette0");
	fImage[1] = FetchImage("pipette1");

	SetDrawingMode( B_OP_ALPHA );
}

EyeDropper::~EyeDropper() {
	delete fImage[0];
	delete fImage[1];
}

void EyeDropper::Draw( BRect updateRect ) {
	DrawBitmap( fImage[fButtonDown] );
}

void EyeDropper::MouseDown(BPoint where) {

	Window()->Activate();
	
	fButtonDown = true;
	fMouseDown = true;
	Draw( Bounds() );

	SetMouseEventMask(B_POINTER_EVENTS, B_SUSPEND_VIEW_FOCUS|B_LOCK_WINDOW_FOCUS );
}

void EyeDropper::MouseUp(BPoint where) {
	fMouseDown = false;

	if (fButtonDown) {
		Invoke();
		fButtonDown = false;
	}

	Draw( Bounds() );
}

void EyeDropper::MouseMoved( BPoint where, uint32 code, const BMessage *a_message) {

	if (!fMouseDown || a_message) return;
	
	if (Bounds().Contains(where)) {

		if (fButtonDown) return;

		fButtonDown = true;		
	}
	else {

		if (!fButtonDown) return;

		fButtonDown = false;
	}
	
	Draw( Bounds() );
}

BBitmap	*FetchImage(const char *imgname) {

	BResources	*Resourcen = be_app->AppResources();

	size_t		groesse;
	BMessage	msg;
	char		*buf = (char *)Resourcen->LoadResource('BBMP', imgname, &groesse);

	if (buf) {
		msg.Unflatten(buf);
		return new BBitmap( &msg );
	}

	cerr << "*** Resource not found: " << imgname << endl;
	return 0;
}
