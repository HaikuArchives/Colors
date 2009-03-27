/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#include "TextContainer.h"

#include <Application.h>
#include <Bitmap.h>
#include <iostream>
#include <MessageRunner.h>
#include <Resources.h>
#include <Region.h>
#include <stdio.h>
#include <String.h>
#include <stdio.h>
#include <Window.h>

#define MSG_MESSAGERUNNER 'MsgR'

#define COLOR_RECT_1 BRect( 0.0, 0.0, 19.0, 11.0 ).OffsetToCopy(105.0, 6.0)
#define COLOR_RECT_2 BRect( 0.0, 0.0, 19.0, 11.0 ).OffsetToCopy(113.0, 12.0)

#define OUTER_RECT_1 BRect( 0.0, 0.0, 24.0, 17.0 ).OffsetToCopy(100.0, 0.0)
#define OUTER_RECT_2 BRect( 0.0, 0.0, 30.0, 30.0 ).OffsetToCopy(112.0, 0.0)

TextContainer::TextContainer( BRect frame )
:	BView( frame, "TextContainer", B_FOLLOW_TOP|B_FOLLOW_LEFT, B_WILL_DRAW ),
	fMouseDown( false ),
	fMessageRunner( 0 ),
	fGotFirstClick( false ),
	fActiveColor( 0 )
{

	BResources	*Resourcen = be_app->AppResources();
	
	size_t		groesse;
	BMessage	msg;
	char		*buf;

	buf = (char *)Resourcen->LoadResource('BBMP', "doublecontainer", &groesse);
	if (buf) {
		msg.Unflatten(buf);
		fBackgroundBmp = new BBitmap( &msg );
	}
	
	BRect rect = BRect( 0.0, 0.0, 100.0, 20.0 ).InsetByCopy( 2.0, 2.0 );
	fTextView = new BTextView( rect.OffsetByCopy(0.0, 5.0), "", rect.InsetByCopy(0.0, 2.0).OffsetByCopy(-2.0, -2.0), B_FOLLOW_ALL );
	fTextView->SetText("The quick brown fox jumps over the lazy dog.");
	fTextView->SetColorSpace(B_RGB32);
	AddChild( fTextView );

	SetViewColor( B_TRANSPARENT_COLOR );

}

void  TextContainer::Draw( BRect updateRect ) {
	
	// Textfield
	BRect rect( 0.0, 0.0, 100.0, 20.0 );
	rect.OffsetBy(0.0, 5.0);
	
	SetHighColor( 128, 128, 128 );
	StrokeLine( rect.RightTop(), rect.LeftTop() );
	StrokeLine( rect.LeftBottom() );
	SetHighColor( 255, 255, 255 );
	StrokeLine( rect.RightBottom() );
	StrokeLine( rect.RightTop() );

	rect.InsetBy( 1.0, 1.0 );
	SetHighColor( 0, 0, 0 );
	StrokeRect( rect );
	
	rect.InsetBy( 1.0, 1.0 );
	SetLowColor( fBgColor );
	FillRect( rect, B_SOLID_LOW );
	
	// Colorfield
	SetDrawingMode( B_OP_ALPHA );
	DrawBitmap( fBackgroundBmp, BPoint(104.0, 5.0));
	SetDrawingMode( B_OP_COPY );

	SetHighColor( fBgColor );
	FillRect( COLOR_RECT_2.InsetByCopy(1.0, 1.0) );

	SetHighColor( fColor );
	FillRect( COLOR_RECT_1.InsetByCopy(1.0, 1.0) );

	SetHighColor( 0, 0, 0 );
	StrokeRect( COLOR_RECT_1 );

}

void  TextContainer::MessageReceived( BMessage *message ) {

	fActiveColor = 0;
	
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
		
		BPoint where;
		if (message->FindPoint("_drop_point_", &where)==B_OK) {
			ConvertFromScreen(&where);
			
			if (OUTER_RECT_1.Contains(where) )
				SetColor( true, *color );
			else if (OUTER_RECT_2.Contains(where) )
				SetColor( false, *color );
	
		}
		else {
			SetColor ( true, *color );
		}
	}
}

void  TextContainer::MouseDown( BPoint where ) {

	Window()->Activate();
	
	long clicks = Window()->CurrentMessage()->FindInt32( "clicks" );
	
	if (clicks==2 && fGotFirstClick) {
		
		rgb_color color;

		if (COLOR_RECT_1.Contains(where)) color = fColor;
		else if (COLOR_RECT_2.Contains(where)) color = fBgColor;
		else return;
		
		BMessage msg;
		msg.AddData("RGBColor", B_RGB_COLOR_TYPE, &color, sizeof(color));
		
		BHandler *colorpreview;
		if ( (colorpreview = (BHandler *)Window()->FindView("colorpreview") )) {
			Window()->PostMessage( &msg, colorpreview );
		}
	}
	
	else {
	
		fGotFirstClick = (clicks==1);
	
		fMouseDown = true;

		fMessageRunner = new BMessageRunner(this, new BMessage(MSG_MESSAGERUNNER), 300000, 1);

//		SetMouseEventMask(B_POINTER_EVENTS, B_SUSPEND_VIEW_FOCUS|B_LOCK_WINDOW_FOCUS );
	}
}
			
void  TextContainer::MouseUp( BPoint where ) {

	delete fMessageRunner;
	fMessageRunner = 0;

	fMouseDown = false;
	BView::MouseUp( where );
}

void TextContainer::MouseMoved( BPoint where, uint32 code, const BMessage *a_message) {

	SetMouseEventMask(B_POINTER_EVENTS, B_SUSPEND_VIEW_FOCUS|B_LOCK_WINDOW_FOCUS);

	if (fMouseDown) {
		DragColor( where );
		return;
	}
	
	char *nameFound;
	type_code typeFound;

	if (!a_message || a_message->GetInfo(B_RGB_COLOR_TYPE, 0, &nameFound, &typeFound)!=B_OK) {
		BView::MouseMoved( where, code, a_message);
		return;
	}

  	rgb_color	*color;
	ssize_t		numBytes;
	a_message->FindData(nameFound, typeFound, (const void **)&color, &numBytes);
	
	if (OUTER_RECT_1.Contains(where) ) {
		if (fActiveColor!=1) {
			fActiveColor = 1;

			Draw( Bounds() );
			SetHighColor( *color );
			FillRect( COLOR_RECT_1.InsetByCopy(1.0, 1.0) );

			fTextView->SetFontAndColor( be_plain_font, B_FONT_ALL, color );
			fTextView->SetViewColor( fBgColor );
			fTextView->Invalidate();
		}
	}
	else if (OUTER_RECT_2.Contains(where) ) {
		if (fActiveColor!=2) {
			fActiveColor = 2;

			Draw( Bounds() );
			SetHighColor( *color );
			FillRect( COLOR_RECT_2.InsetByCopy(1.0, 1.0) );
			SetHighColor( fColor );
			FillRect( COLOR_RECT_1.InsetByCopy(1.0, 1.0) );
			SetHighColor( 0, 0, 0 );
			StrokeRect( COLOR_RECT_1 );

			fTextView->SetViewColor( *color );
			fTextView->Invalidate();
			fTextView->SetFontAndColor( be_plain_font, B_FONT_ALL, &fColor );
		}
	}
	else {
		if (fActiveColor!=0) {
			fActiveColor = 0;
			Draw( Bounds() );

			fTextView->SetFontAndColor( be_plain_font, B_FONT_ALL, &fColor );
			fTextView->SetViewColor( fBgColor );
			fTextView->Invalidate();
		}
	}

}

void  TextContainer::DragColor( BPoint where ) {

	fGotFirstClick = false;
	
	if (!COLOR_RECT_1.Contains(where) && !COLOR_RECT_2.Contains(where) ) return;
	
	rgb_color color = COLOR_RECT_1.Contains(where) ? fColor : fBgColor;
	
	char hexstr[7];
	sprintf(hexstr, "#%.2X%.2X%.2X", color.red, color.green, color.blue);

	BMessage msg(B_PASTE);
	msg.AddData("text/plain", B_MIME_TYPE, &hexstr, sizeof(hexstr));
	msg.AddData("RGBColor", B_RGB_COLOR_TYPE, &color, sizeof(color));

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

	view->SetHighColor(min_c(255, 1.2 * color.red + 40), min_c(255, 1.2 * color.green + 40), min_c(255, 1.2 * color.blue + 40));
	view->StrokeRect(rect);

	++rect.left; ++rect.top;
	view->SetHighColor(0.8 * color.red, 0.8 * color.green, 0.8 * color.blue);
	view->StrokeRect(rect);

	--rect.right; --rect.bottom;
	view->SetHighColor(color.red, color.green, color.blue);
	view->FillRect(rect);
	
	view->Flush();
	
	bmp->Unlock();

	DragMessage(&msg, bmp, B_OP_ALPHA, BPoint(14.0, 14.0) );

	MouseUp( where );

}

void  TextContainer::SetColor( bool foreground, long int c ) {

	rgb_color color;
	color.red	= (c>>16) & 255;
	color.green	= (c>>8) & 255;
	color.blue	= c & 255;
	
	SetColor( foreground, color );
}

void  TextContainer::SetColor( bool foreground, rgb_color color ) {
	
	color.alpha = 255;
	
	if (foreground) {
		fColor = color;
		fTextView->SetFontAndColor( be_plain_font, B_FONT_ALL, &color );
	}
	else {
		fBgColor = color;
		fTextView->SetViewColor( color );
		fTextView->Invalidate();
	}
	
	if (Window()) Draw(Bounds());
}

rgb_color TextContainer::GetColor( bool foreground ) {
	return foreground ? fColor : fBgColor;
}
