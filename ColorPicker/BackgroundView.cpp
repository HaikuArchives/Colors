/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#include "BackgroundView.h"

#include <Application.h>
#include <Bitmap.h>
#include <Cursor.h>
#include <iostream>
#include <Region.h>
#include <Screen.h>
#include <Window.h>

#define ORIG_RECT	BRect( -8.0, -8.0, 8.0, 8.0)
#define FACTOR		6
#define COPY_RECT	BRect( ORIG_RECT.left * FACTOR, ORIG_RECT.top * FACTOR, \
							ORIG_RECT.right * FACTOR, ORIG_RECT.bottom * FACTOR )

BackgroundView::BackgroundView( BRect frame )
:	BView( frame, "", 0, B_WILL_DRAW )
{
	fHiddenWindow = be_app->WindowAt( 0 );
	fHiddenWindow->Hide();

	BScreen screen( Window() );
	screen.GetBitmap( &fBitmap, false );
}

BackgroundView::~BackgroundView() {
	delete fBitmap;
}

void BackgroundView::Close() {
	fHiddenWindow->Show();

	Window()->Lock();
	Window()->Close(); // here automatically this view quits!
}

void BackgroundView::AttachedToWindow() {
	fWorkspace = Window()->Workspaces();
	BView::AttachedToWindow();
}

void BackgroundView::Draw( BRect bounds ) {
	
	if ( fWorkspace != Window()->Workspaces() ) {
		Close();
		return;
	}
	
	const char cursor_data[] = {

		/* size, depth and hotspot y/x */
		16, 1, 15, -1,

		/* colors */
		0x00, 0x1C,
		0x00, 0x3E,
		0x00, 0x3E,
		0x00, 0xFE,
		
		0x00, 0xBC,
		0x01, 0x10,
		0x02, 0x30,
		0x04, 0x40,

		0x08, 0x80,
		0x11, 0x00,
		0x22, 0x00,
		0x44, 0x00,

		0x48, 0x00,
		0xB0, 0x00,
		0x40, 0x00,
		0x00, 0x00,

		/* alpha */
		0x00, 0x1C,
		0x00, 0x3E,
		0x00, 0x3E,
		0x00, 0xFE,
		
		0x00, 0xFC,
		0x01, 0xF0,
		0x03, 0xF0,
		0x07, 0xC0,

		0x0F, 0x80,
		0x1F, 0x00,
		0x3E, 0x00,
		0x7C, 0x00,

		0x78, 0x00,
		0xF0, 0x00,
		0x40, 0x00,
		0x00, 0x00,
	};
			
	BCursor	cursor(&cursor_data);
	SetViewCursor(&cursor);
	DrawBitmap( fBitmap );
}

void BackgroundView::MouseDown(BPoint where) {

	Window()->Activate();
	
	if (Window()->CurrentMessage()->FindInt32("buttons")==B_PRIMARY_MOUSE_BUTTON) {
	
		rgb_color	color = ColorAt( where );
		BWindow *win = be_app->WindowAt(0);
		if (win) {
			BHandler *colorpreview;
			if ( (colorpreview = (BHandler *)win->FindView("colorpreview") )) {
				BMessage msg;
				msg.AddData("RGBColor", B_RGB_COLOR_TYPE, &color, sizeof(color));
				win->PostMessage( &msg, colorpreview );
			}
		}
	
	}
	
	Close();
}

void BackgroundView::MouseMoved( BPoint where,
	uint32 code, const BMessage *a_message) {
	
	static BPoint lastPosition (-1.0, -1.0);
	
	BRect destRect( COPY_RECT.OffsetToCopy( where ).
		OffsetByCopy( -(COPY_RECT.IntegerWidth()>>1), -(COPY_RECT.IntegerHeight()>>1) ) );
		
	if (lastPosition != BPoint(-1.0, -1.0) ) {

		BRegion	region( Frame() );
		region.Exclude( destRect );
		ConstrainClippingRegion( &region );
		
		DrawBitmapAsync( fBitmap,

			COPY_RECT.
			OffsetToCopy( lastPosition ).
			OffsetByCopy( -(COPY_RECT.IntegerWidth()>>1), -(COPY_RECT.IntegerHeight()>>1) ),
			
			COPY_RECT.
			OffsetToCopy( lastPosition ).
			OffsetByCopy( -(COPY_RECT.IntegerWidth()>>1), -(COPY_RECT.IntegerHeight()>>1) )
		);

		region = Frame();
		ConstrainClippingRegion( &region );
	}

	lastPosition = where;

	DrawBitmap( fBitmap, ORIG_RECT.
		OffsetToCopy( where ).
		OffsetByCopy( -(ORIG_RECT.IntegerWidth()>>1), -(ORIG_RECT.IntegerHeight()>>1) ),
		destRect );
	
	SetDrawingMode( B_OP_INVERT );
	StrokeRect( destRect );
	StrokeRect( BRect(-.5*FACTOR, -.5*FACTOR, .5*FACTOR, .5*FACTOR).
		OffsetToCopy( where ).
		OffsetByCopy( -(FACTOR>>1), -(FACTOR>>1) ).
		InsetByCopy( 1.0, 1.0 )
	);
	SetDrawingMode( B_OP_COPY );
}

void BackgroundView::KeyDown(const char *bytes, int32 numBytes) {

	if ( (int)bytes[0] == 27 ) Close();

}

rgb_color BackgroundView::ColorAt( BPoint pt ) {

	rgb_color	color = { 0, 0, 0, 255 };
	
	if (!fBitmap->Bounds().Contains( pt ) ) return color;
	
	uchar	*bits = (uchar *)fBitmap->Bits();

	switch( fBitmap->ColorSpace() ) {

		case B_RGB32: // B[7:0]  G[7:0]  R[7:0]  -[7:0]
		case B_RGBA32: // B[7:0]  G[7:0]  R[7:0]  A[7:0]
		{
			uint64	pos = (uint64)pt.x * 4 + (uint64)pt.y * fBitmap->BytesPerRow();
			color.red 	= bits[pos+2];
			color.green	= bits[pos+1];
			color.blue	= bits[pos];
		} break;

		case B_RGB24: // B[7:0]  G[7:0]  R[7:0]
		{
			uint64	pos = (uint64)pt.x * 3 + (uint64)pt.y * fBitmap->BytesPerRow();
			color.red 	= bits[pos+2];
			color.green	= bits[pos+1];
			color.blue	= bits[pos];
		} break;
		
		case B_RGB16:	// G[2:0],B[4:0]  R[4:0],G[5:3]						
		{
			uint64	pos = (uint64)pt.x * 2 + (uint64)pt.y * fBitmap->BytesPerRow();
			color.red 	= bits[pos+1] & 0xF8;
			color.green	= ((bits[pos+1] & 0x07) << 5) | ((bits[pos] & 0xE0) >> 3);
			color.blue	= (bits[pos] & 0x1F) << 3;
		} break;

		case B_RGB15:	// G[2:0],B[4:0]  	   -[0],R[4:0],G[4:3]			
		case B_RGBA15:	// G[2:0],B[4:0]  	   A[0],R[4:0],G[4:3]			
		{
			uint64	pos = (uint64)pt.x * 2 + (uint64)pt.y * fBitmap->BytesPerRow();
			color.red 	= (bits[pos+1] & 0x7B) << 1;
			color.green	= ((bits[pos+1] & 0x03) << 6) | ((bits[pos] & 0xE0) >> 2);
			color.blue	= (bits[pos] & 0x1F) << 3;
		} break;

		case B_CMAP8:	// D[7:0]
		{
			uint64	pos = (uint64)pt.x + (uint64)pt.y * fBitmap->BytesPerRow();
			color = BScreen().ColorForIndex( bits[pos] );
		} break;

		case B_GRAY8:	// Y[7:0]
		{
			uint64	pos = (uint64)pt.x + (uint64)pt.y * fBitmap->BytesPerRow();
			color.red 	= bits[pos];
			color.green	= bits[pos];
			color.blue	= bits[pos];
		} break;

		default:
			cerr << "Sorry, unknown colorspace: 0x" << hex << (uint64)fBitmap->ColorSpace() << endl;
	}			
	return color;

}
