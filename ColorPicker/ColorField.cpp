/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#include "ColorField.h"

#include <Bitmap.h>
#include <iostream>
#include <OS.h>
#include <Region.h>
#include <Window.h>

#include "Convert_RGB_HSV.h"
#include "selected_color_mode.h"

#define DrawColorPoint( pt, r, g, b) { rgb_color color = {r, g, b, 255}; view->AddLine( pt, pt, color ); }
#define round(x) (int)(x+.5)

ColorField::ColorField( BPoint offset_point, selected_color_mode mode, float fixed_value)
:	BControl(BRect(0.0, 0.0, 259.0, 259.0).OffsetToCopy(offset_point), "ColorField",
		"", new BMessage(MSG_COLOR_FIELD), B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW|B_FRAME_EVENTS),
	fMode( mode ),
	fFixedValue( fixed_value ),
	fMarkerPosition( 0.0, 0.0 ),
	fLastMarkerPosition( -1.0, -1.0 ),
	fMouseDown( false ),
	fUpdateThread( 0 ),
	fUpdatePort( 0 )

{

	for (int i=0; i<2; ++i) {
		fBgBitmap[i] = new BBitmap(Bounds(), B_RGB32, true, true);

		fBgBitmap[i]->Lock();
		fBgView[i] = new BView(Bounds(), "", B_FOLLOW_NONE, B_WILL_DRAW);
		fBgBitmap[i]->AddChild(fBgView[i]);
		fBgView[i]->SetOrigin(2.0, 2.0);
		fBgBitmap[i]->Unlock();
	}

	fBgBitmap[1]->Lock();

	fBgView[1]->SetHighColor( 128, 128, 128 );
	fBgView[1]->StrokeRect( Bounds().InsetByCopy(-1.0, -1.0).OffsetToCopy(-2.0, -2.0));
	fBgView[1]->SetHighColor( 255.0, 255.0, 255.0 );
	fBgView[1]->StrokeRect( Bounds().InsetByCopy(-1.0, -1.0).OffsetToCopy(-4.0, -4.0));
	fBgView[1]->SetHighColor( 0, 0, 0 );
	fBgView[1]->StrokeRect( Bounds().InsetByCopy(1.0, 1.0).OffsetToCopy(-1.0, -1.0));
	
	BRegion region( fBgView[1]->Bounds().InsetByCopy(2.0, 2.0).OffsetToCopy(0.0, 0.0) );
	fBgView[1]->ConstrainClippingRegion( &region );

	fBgBitmap[1]->Unlock();

	fUpdatePort = create_port(100, "color field update port");

	fUpdateThread = spawn_thread(ColorField::UpdateThread, "color field update thread", 10, this);
	resume_thread( fUpdateThread );

	Update(3);
}

ColorField::~ColorField() {
	
	if (fUpdatePort) delete_port(fUpdatePort);
	if (fUpdateThread) kill_thread(fUpdateThread);
	
}

status_t ColorField::Invoke(BMessage *msg) {
	
	if (!msg) msg = Message();

	msg->RemoveName("value");

	float v1 = 0, v2 = 0;
	
	switch (fMode) {
				
		case R_SELECTED:
		case G_SELECTED:
		case B_SELECTED:

		case H_SELECTED: {
			v1 = fMarkerPosition.x / 255.0; v2 = 1.0 - fMarkerPosition.y / 255.0;
		} break;
		
		case S_SELECTED:
		case V_SELECTED: {
			v1 = fMarkerPosition.x / 255.0 * 6.0; v2 = 1.0 - fMarkerPosition.y / 255;
		} break;

	}
	
	msg->AddFloat("value", v1);
	msg->AddFloat("value", v2);
	
	return BControl::Invoke(msg);
}

void ColorField::Draw( BRect updateRect ) {
	Update(0);
}

int32 ColorField::UpdateThread( void *data ) {
	
	// -- Initialisierung
	ColorField *colorField = (ColorField *)data;
	
	BLooper	*looper = colorField->Looper();

	if (looper) looper->Lock();

	BBitmap *bitmap = colorField->fBgBitmap[0];
	BView	*view = colorField->fBgView[0];

	port_id	port = colorField->fUpdatePort;

	if (looper) looper->Unlock();
	
	float h, s, v, r, g, b;
	int R, G, B;
		
	// zeichnen

    int32	msg_code;
    char	msg_buffer;

	while (true) {

		port_info info;

		do {

			read_port(port, &msg_code, &msg_buffer, sizeof(msg_buffer));
			get_port_info(port, &info);
			
		} while (info.queue_count);
		
		if (looper) looper->Lock();
	
		uint 	colormode = colorField->fMode;
		float	fixedvalue = colorField->fFixedValue;
	     
		if (looper) looper->Unlock();

		bitmap->Lock();
		view->BeginLineArray(256*256);
		
		switch (colormode) {
			
			case R_SELECTED: {
				R = round(fixedvalue * 255);
				for (int G=0; G<256; ++G) {
					for (int B = 0; B<256; ++B) {
						DrawColorPoint( BPoint(G, 255.0 - B), R, G, B );
					}
				}
			}; break;
			
			case G_SELECTED: {
				G = round(fixedvalue * 255);
				for (int R=0; R<256; ++R) {
					for (int B = 0; B<256; ++B) {
						DrawColorPoint( BPoint(R, 255.0 - B), R, G, B );
					}
				}
			}; break;
			
			case B_SELECTED: {
				B = round(fixedvalue * 255);
				for (int R=0; R<256; ++R) {
					for (int G = 0; G<256; ++G) {
						DrawColorPoint( BPoint(R, 255.0 - G), R, G, B );
					}
				}
			}; break;
			
			case H_SELECTED: {
				h = fixedvalue;
				for (int x=0; x<256; ++x) {
					s = (float)x / 255.0;
					for (int y = 0; y<256; ++y) {
						v = (float)y / 255.0;
						HSV_to_RGB( h, s, v, r, g, b );
						DrawColorPoint( BPoint(x, 255.0 - y), round(r * 255.0), round(g * 255.0), round(b * 255.0) );
					}
				}
			}; break;
			
			case S_SELECTED: {
				s = fixedvalue;
				for (int x=0; x<256; ++x) {
					h = 6.0 / 255 * x;
					for (int y = 0; y<256; ++y) {
						v = (float)y / 255.0;
						HSV_to_RGB( h, s, v, r, g, b );
						DrawColorPoint( BPoint(x, 255.0 - y), round(r * 255.0), round(g * 255.0), round(b * 255.0) );
					}
				}

			}; break;
			
			case V_SELECTED: {
				v = fixedvalue;
				for (int x=0; x<256; ++x) {
					h = 6.0 / 255 * x;
					for (int y = 0; y<256; ++y) {
						s = (float)y / 255.0;
						HSV_to_RGB( h, s, v, r, g, b );
						DrawColorPoint( BPoint(x, 255.0 - y), round(r * 255.0), round(g * 255.0), round(b * 255.0) );
					}
				}
			}; break;
		}

		view->EndLineArray();
		view->Sync();

		bitmap->Unlock();

		looper = colorField->Looper();
		if (looper && looper->Lock()) {
			colorField->Update(2);
			looper->Unlock();
		}
	}
}

void ColorField::Update( int depth ) {

	// depth: 0 = only onscreen redraw, 1 = only cursor 1, 2 = full update part 2, 3 = full

	if (depth==3) {
		write_port(fUpdatePort, MSG_UPDATE, NULL, 0);
		return;
	}

	if (depth>=1) {

		fBgBitmap[1]->Lock();
		
		fBgView[1]->DrawBitmap( fBgBitmap[0], BPoint(-2.0, -2.0) );
	
		fBgView[1]->SetHighColor( 0, 0, 0 );
		fBgView[1]->StrokeEllipse( fMarkerPosition, 5.0, 5.0 );
		fBgView[1]->SetHighColor( 255.0, 255.0, 255.0 );
		fBgView[1]->StrokeEllipse( fMarkerPosition, 4.0, 4.0 );
			
		fBgView[1]->Sync();

		fBgBitmap[1]->Unlock();
	}
	
	if ( depth!=0 && depth!=2 && fMarkerPosition != fLastMarkerPosition ) {
	
		fBgBitmap[1]->Lock();

		DrawBitmap( fBgBitmap[1],
			BRect(-3.0, -3.0, 7.0, 7.0).OffsetByCopy(fMarkerPosition),
			BRect(-3.0, -3.0, 7.0, 7.0).OffsetByCopy(fMarkerPosition) );
		DrawBitmap( fBgBitmap[1],
			BRect(-3.0, -3.0, 7.0, 7.0).OffsetByCopy(fLastMarkerPosition),
			BRect(-3.0, -3.0, 7.0, 7.0).OffsetByCopy(fLastMarkerPosition) );

		fLastMarkerPosition = fMarkerPosition;

		fBgBitmap[1]->Unlock();

	}
	else

		DrawBitmap( fBgBitmap[1] );

}

void ColorField::MouseDown(BPoint where) {

	Window()->Activate();
	
	fMouseDown = true;
	SetMouseEventMask(B_POINTER_EVENTS, B_SUSPEND_VIEW_FOCUS|B_LOCK_WINDOW_FOCUS );
	PositionMarkerAt( where );
	Invoke();
}

void ColorField::MouseUp(BPoint where) {
	fMouseDown = false;
}

void ColorField::MouseMoved( BPoint where, uint32 code, const BMessage *a_message) {
	
	if (a_message || !fMouseDown ) {
		BView::MouseMoved( where, code, a_message);
		return;
	}
	
	PositionMarkerAt( where );
	Invoke();
}

void ColorField::PositionMarkerAt( BPoint where ) {

	BRect rect = Bounds().InsetByCopy( 2.0, 2.0 ).OffsetToCopy(0.0, 0.0);
	where = BPoint(	max_c( min_c(where.x - 2.0, rect.right), 0.0),
					max_c( min_c(where.y - 2.0, rect.bottom), 0.0) );
	
	fLastMarkerPosition = fMarkerPosition;
	fMarkerPosition = where;
	Update(1);

}

void ColorField::SetModeAndValue(selected_color_mode mode, float fixed_value) {

	float R(0), G(0), B(0);
	float H(0), S(0), V(0);
	
	fBgBitmap[0]->Lock();
	
	switch (fMode) {
		
		case R_SELECTED: {
			R = fFixedValue*255;
			G = round(fMarkerPosition.x);
			B = round(255.0 - fMarkerPosition.y);
		}; break;
		
		case G_SELECTED: {
			R = round(fMarkerPosition.x);
			G = fFixedValue*255;
			B = round(255.0 - fMarkerPosition.y);
		}; break;

		case B_SELECTED: {
			R = round(fMarkerPosition.x);
			G = round(255.0 - fMarkerPosition.y);
			B = fFixedValue*255;
		}; break;

		case H_SELECTED: {
			H = fFixedValue;
			S = fMarkerPosition.x/255.0;
			V = 1.0 - fMarkerPosition.y/255.0;
		}; break;

		case S_SELECTED: {
			H = fMarkerPosition.x/255.0*6.0;
			S = fFixedValue;
			V = 1.0 - fMarkerPosition.y/255.0;
		}; break;

		case V_SELECTED: {
			H = fMarkerPosition.x/255.0*6.0;
			S = 1.0 - fMarkerPosition.y/255.0;
			V = fFixedValue;
		}; break;
	}

	if (fMode & (H_SELECTED|S_SELECTED|V_SELECTED) ) {
		HSV_to_RGB(H, S, V, R, G, B);
		R*=255.0; G*=255.0; B*=255.0;
	}
	
	rgb_color color = { round(R), round(G), round(B), 255 };
	
	fFixedValue = fixed_value;
	fMode = mode;

	fBgBitmap[0]->Unlock();
	Update(3);

	SetMarkerToColor( color );

}

void ColorField::SetFixedValue(float fixed_value) {
	fFixedValue = fixed_value;
	Update(3);
}

void ColorField::SetMarkerToColor( rgb_color color ) {

	float h, s, v;
	RGB_to_HSV( (float)color.red / 255.0, (float)color.green / 255.0, (float)color.blue / 255.0, h, s, v );
	
	fLastMarkerPosition = fMarkerPosition;

	switch (fMode) {
				
		case R_SELECTED: {
			fMarkerPosition = BPoint( color.green, 255.0 - color.blue);
		} break;
		
		case G_SELECTED: {
			fMarkerPosition = BPoint( color.red, 255.0 - color.blue);
		} break;

		case B_SELECTED: {
			fMarkerPosition = BPoint( color.red, 255.0 - color.green);
		} break;
		
		case H_SELECTED: {
			fMarkerPosition = BPoint( s * 255.0, 255.0 - v * 255.0);
		} break;
		
		case S_SELECTED: {
			fMarkerPosition = BPoint( h / 6.0 * 255.0, 255.0 - v * 255.0);
		} break;

		case V_SELECTED: {
			fMarkerPosition = BPoint( h / 6.0 * 255.0, 255.0 - s * 255.0);
		} break;
		
	}

}
