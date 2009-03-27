/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#include "ColorPreview.h"

#include <Bitmap.h>
#include <iostream>
#include <MessageRunner.h>
#include <stdio.h>
#include <String.h>
#include <stdio.h>
#include <Window.h>

#include "Prefs.h"

ColorPreview::ColorPreview( BRect frame, rgb_color color )
:	BControl( frame, "colorpreview", "", new BMessage(MSG_COLOR_PREVIEW),
		B_FOLLOW_TOP|B_FOLLOW_LEFT,	B_WILL_DRAW ),
	fMouseDown( false ),
	fMessageRunner( 0 )
{
	fColor = color;
	fOldColor = color;
}

void ColorPreview::Draw( BRect updateRect ) {
		
	SetHighColor( 128, 128, 128 );
	StrokeRect( Bounds().InsetByCopy(-1.0, -1.0).OffsetToCopy(0.0, 0.0));
	SetHighColor( 255.0, 255.0, 255.0 );
	StrokeRect( Bounds().InsetByCopy(-1.0, -1.0).OffsetToCopy(-2.0, -2.0));
	SetHighColor( 0, 0, 0 );
	StrokeRect( Bounds().InsetByCopy(1.0, 1.0));
	
	BRect rect = Bounds().InsetByCopy(2.0, 2.0);
	rect.bottom /= 2;
	SetHighColor( fColor );
	FillRect( rect );

	rect.top = rect.bottom + 1;
	rect.bottom = Bounds().bottom - 2.0;
	SetHighColor( fOldColor );
	FillRect( rect );
}

void ColorPreview::MessageReceived( BMessage *message ) {

	if (message->what==MSG_MESSAGERUNNER) {

		BPoint	where;
		uint32	buttons;

		GetMouse( &where, &buttons );

		DragColor( where );

	}
	else {
		char *nameFound;
		type_code typeFound;
		
		if (message->GetInfo(B_RGB_COLOR_TYPE, 0, &nameFound, &typeFound)!=B_OK) {
			BControl::MessageReceived( message );
			return;
		}
		
	   	rgb_color	*color;
		ssize_t		numBytes;
		message->FindData(nameFound, typeFound, (const void **)&color, &numBytes);
		
		BPoint where;
		bool drop_new = false;
		if (message->FindPoint("_drop_point_", &where)==B_OK) {
			ConvertFromScreen(&where);
			if (where.y > Bounds().top + (Bounds().IntegerHeight()>>1) )
				drop_new = true;
		}
	
		if (drop_new) SetNewColor( *color ); else SetColor( *color );
		Invoke();
	}
}

void ColorPreview::MouseDown( BPoint where ) {

	Window()->Activate();
	
	fMouseDown = true;

	fMessageRunner = new BMessageRunner(this, new BMessage(MSG_MESSAGERUNNER), 300000, 1);

	SetMouseEventMask(B_POINTER_EVENTS, B_SUSPEND_VIEW_FOCUS|B_LOCK_WINDOW_FOCUS );

	BRect rect = Bounds().InsetByCopy(2.0, 2.0);
	rect.top = rect.bottom/2 + 1;
	
	if (rect.Contains( where ) ) {
		fColor = fOldColor;
		Draw( Bounds() );
		Invoke();
	}
	
}
			
void ColorPreview::MouseUp( BPoint where ) {

	delete fMessageRunner;
	fMessageRunner = 0;

	fMouseDown = false;
	BControl::MouseUp( where );

}

void ColorPreview::MouseMoved( BPoint where, uint32 code, const BMessage *a_message) {

	if (fMouseDown) DragColor( where );

}

void ColorPreview::DragColor( BPoint where ) {

	char hexstr[7];
	sprintf(hexstr, "#%.2X%.2X%.2X", fColor.red, fColor.green, fColor.blue);
	
	BMessage msg(B_PASTE);
	msg.AddData("text/plain", B_MIME_TYPE, &hexstr, sizeof(hexstr));
	msg.AddData("RGBColor", B_RGB_COLOR_TYPE, &fColor, sizeof(fColor));

	BRect rect( 0.0, 0.0, 20.0, 20.0 );

	BBitmap *bmp = new BBitmap(rect, B_RGB32, true);
	bmp->Lock();
	
	BView *view = new BView( rect, "", B_FOLLOW_NONE, B_WILL_DRAW );
	bmp->AddChild(view);

	view->SetHighColor(B_TRANSPARENT_COLOR);
	view->FillRect( view->Bounds() );
	
	++rect.top; ++rect.left;
	view->SetHighColor(0, 0, 0, 100);
	view->FillRect( rect );
	rect.OffsetBy(-1.0, -1.0);

	view->SetHighColor(min_c(255, 1.2 * fColor.red + 40), min_c(255, 1.2 * fColor.green + 40), min_c(255, 1.2 * fColor.blue + 40));
	view->StrokeRect(rect);

	++rect.left; ++rect.top;
	view->SetHighColor(0.8 * fColor.red, 0.8 * fColor.green, 0.8 * fColor.blue);
	view->StrokeRect(rect);

	--rect.right; --rect.bottom;
	view->SetHighColor(fColor.red, fColor.green, fColor.blue);
	view->FillRect(rect);
	
	view->Flush();
	
	bmp->Unlock();

	DragMessage(&msg, bmp, B_OP_ALPHA, BPoint(14.0, 14.0) );

	MouseUp( where );

}

status_t ColorPreview::Invoke( BMessage *msg ) {
	if (!msg) msg = Message();
	
	msg->RemoveName("color");
	msg->AddData("color", B_RGB_COLOR_TYPE, &fColor, sizeof(fColor));
	
	return BControl::Invoke( msg );
}

void ColorPreview::SetColor( rgb_color color ) {
	color.alpha = 255;
	fColor = color;
	Draw(Bounds());
}

void ColorPreview::SetNewColor( rgb_color color ) {
	fColor = color;
	fOldColor = color;
	Draw(Bounds());
}
