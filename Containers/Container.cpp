/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#include "Container.h"

#include <Application.h>
#include <Bitmap.h>
#include <iostream>
#include <MessageRunner.h>
#include <Resources.h>
#include <stdio.h>
#include <String.h>
#include <stdio.h>
#include <Window.h>

Container::Container( BRect frame )
:	BView( frame, "Container", B_FOLLOW_TOP|B_FOLLOW_LEFT, B_WILL_DRAW ),
	fMouseDown( false ),
	fMessageRunner( 0 ),
	fGotFirstClick( false )
{

	BResources	*Resourcen = be_app->AppResources();
	
	size_t		groesse;
	BMessage	msg;
	char		*buf;

	buf = (char *)Resourcen->LoadResource('BBMP', "container", &groesse);
	if (buf) {
		msg.Unflatten(buf);
		fBackgroundBmp = new BBitmap( &msg );
	}
	
	SetViewColor( B_TRANSPARENT_COLOR );
}

void  Container::Draw( BRect updateRect ) {
		
	SetDrawingMode( B_OP_ALPHA );
	DrawBitmap( fBackgroundBmp, BPoint(0,0) );
	SetDrawingMode( B_OP_COPY );

	SetHighColor( fColor );
	FillEllipse( Bounds().InsetByCopy(2.0, 2.0) );

}

void  Container::MessageReceived( BMessage *message ) {

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
			BView::MessageReceived( message );
			return;
		}
		
	   	rgb_color	*color;
		ssize_t		numBytes;
		message->FindData(nameFound, typeFound, (const void **)&color, &numBytes);
		SetColor( *color );
	}
}

void  Container::MouseDown( BPoint where ) {

	Window()->Activate();
	
	long clicks = Window()->CurrentMessage()->FindInt32( "clicks" );
	
	if (clicks==2 && fGotFirstClick) {

		BMessage msg;
		msg.AddData("RGBColor", B_RGB_COLOR_TYPE, &fColor, sizeof(fColor));
		
		BHandler *colorpreview;
		if ( (colorpreview = (BHandler *)Window()->FindView("colorpreview") )) {
			Window()->PostMessage( &msg, colorpreview );
		}
	}
	
	else {
	
		fGotFirstClick = (clicks==1);
	
		fMouseDown = true;

		fMessageRunner = new BMessageRunner(this, new BMessage(MSG_MESSAGERUNNER), 300000, 1);

		SetMouseEventMask(B_POINTER_EVENTS, B_SUSPEND_VIEW_FOCUS|B_LOCK_WINDOW_FOCUS );
	}
}
			
void  Container::MouseUp( BPoint where ) {

	delete fMessageRunner;
	fMessageRunner = 0;

	fMouseDown = false;
	BView::MouseUp( where );

}

void Container::MouseMoved( BPoint where, uint32 code, const BMessage *a_message) {

	if (fMouseDown) DragColor( where );

}

void  Container::DragColor( BPoint where ) {

	fGotFirstClick = false;

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

void  Container::SetColor( long int c ) {

	rgb_color color;
	color.red	= (c>>16) & 255;
	color.green	= (c>>8) & 255;
	color.blue	= c & 255;
	SetColor( color );
}


void  Container::SetColor( rgb_color color ) {
	color.alpha	= 255;
	fColor = color;
	if (Window()) Draw(Bounds());
}

rgb_color Container::GetColor() {
	return fColor;
}
