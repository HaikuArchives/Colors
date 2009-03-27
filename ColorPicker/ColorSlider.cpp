/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#include "ColorSlider.h"

#include <Bitmap.h>
#include <iostream>
#include <OS.h>
#include <Window.h>
#include <math.h>

#include "Convert_RGB_HSV.h"
#include "selected_color_mode.h"

#define round(x) (int)(x+.5)

ColorSlider::ColorSlider( BPoint offset_point, selected_color_mode mode, float value1, float value2)
:	BControl(BRect(0.0, 0.0, 35.0, 265.0).OffsetToCopy(offset_point), "ColorSlider",
		"", new BMessage(MSG_COLOR_SLIDER), B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW),
	fMode( mode ),
	fFixedValue1( value1 ),
	fFixedValue2( value2 ),
	fMouseDown( false ),
	fUpdateThread( 0 ),
	fUpdatePort( 0 )

{

	fBgBitmap = new BBitmap(Bounds(), B_RGB32, true);

	fBgBitmap->Lock();
	fBgView = new BView(Bounds(), "", B_FOLLOW_NONE, B_WILL_DRAW);
	fBgBitmap->AddChild(fBgView);
	fBgView->SetOrigin(8.0, 2.0);
	fBgBitmap->Unlock();

	fUpdatePort = create_port(100, "color slider update port");

	fUpdateThread = spawn_thread(ColorSlider::UpdateThread, "color slider update thread", 10, this);
	resume_thread( fUpdateThread );

	Update(2);
}

void ColorSlider::AttachedToWindow() {

	BControl::AttachedToWindow();

	if (Parent()) SetViewColor( Parent()->ViewColor() );
	else SetViewColor(216, 216, 216);
	
	fBgBitmap->Lock();
	fBgView->SetHighColor( ViewColor() );
	fBgView->FillRect( Bounds().OffsetToCopy(-3.0, -2.0) );
	fBgBitmap->Unlock();

	Update(2);
}

ColorSlider::~ColorSlider() {
	
	if (fUpdatePort) delete_port(fUpdatePort);
	if (fUpdateThread) kill_thread(fUpdateThread);
	
}

status_t ColorSlider::Invoke(BMessage *msg) {
	
	if (!msg) msg = Message();
	
	msg->RemoveName("value");
	
	switch (fMode) {
				
		case R_SELECTED:
		case G_SELECTED:
		case B_SELECTED: {
			msg->AddFloat("value", 1.0 - (float)Value()/255);
		} break;
		
		case H_SELECTED: {
			msg->AddFloat("value", (1.0 - (float)Value()/255)*6);
		} break;
		
		case S_SELECTED:
		case V_SELECTED: {
			msg->AddFloat("value", 1.0 - (float)Value()/255);
		} break;
		
	}

	return BControl::Invoke(msg);
}

void ColorSlider::Draw( BRect updateRect ) {
	Update(0);
}

int32 ColorSlider::UpdateThread( void *data ) {
	
	// -- Initialisierung
	ColorSlider *colorSlider = (ColorSlider *)data;
	
	BLooper	*looper = colorSlider->Looper();

	if (looper) looper->Lock();

	BBitmap *bitmap = colorSlider->fBgBitmap;
	BView	*view = colorSlider->fBgView;

	port_id	port = colorSlider->fUpdatePort;

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
	
		uint 	colormode = colorSlider->fMode;
		float	fixedvalue1 = colorSlider->fFixedValue1;
		float	fixedvalue2 = colorSlider->fFixedValue2;
	    
		if (looper) looper->Unlock();

		bitmap->Lock();

		view->BeginLineArray(256);
		
		switch (colormode) {
			
			case R_SELECTED: {
				G = round(fixedvalue1 * 255);
				B = round(fixedvalue2 * 255);
				for (int R=0; R<256; ++R) {
					DrawColorLine( view, R, R, G, B );
				}
			}; break;
			
			case G_SELECTED: {
				R = round(fixedvalue1 * 255);
				B = round(fixedvalue2 * 255);
				for (int G=0; G<256; ++G) {
					DrawColorLine( view, G, R, G, B );
				}
			}; break;
			
			case B_SELECTED: {
				R = round(fixedvalue1 * 255);
				G = round(fixedvalue2 * 255);
				for (int B=0; B<256; ++B) {
					DrawColorLine( view, B, R, G, B );
				}
			}; break;
			
			case H_SELECTED: {
				s = fixedvalue1;
				v = fixedvalue2;
				for (int y=0; y<256; ++y) {
					HSV_to_RGB( (float)y*6.0/255.0, s, v, r, g, b );
					DrawColorLine( view, y, r*255, g*255, b*255 );
				}
			}; break;
			
			case S_SELECTED: {
				h = fixedvalue1;
				v = fixedvalue2;
				for (int y=0; y<256; ++y) {
					HSV_to_RGB( h, (float)y/255, v, r, g, b );
					DrawColorLine( view, y, r*255, g*255, b*255 );
				}
			}; break;
			
			case V_SELECTED: {
				h = fixedvalue1;
				s = fixedvalue2;
				for (int y=0; y<256; ++y) {
					HSV_to_RGB( h, s, (float)y/255, r, g, b );
					DrawColorLine( view, y, r*255, g*255, b*255 );
				}
			}; break;
		}
	
		view->EndLineArray();
		view->Sync();
		bitmap->Unlock();

		looper = colorSlider->Looper();
		if (looper && looper->Lock()) {
			colorSlider->Update(1);
			looper->Unlock();
		}
	}
}

void ColorSlider::Update( int depth ) {

	// depth = Tiefe: 0 = nur onscreen, 1 = bitmap 1, 2 = bitmap 0
	if (depth==2) {
		write_port(fUpdatePort, MSG_UPDATE, NULL, 0);
		return;
	}
	
	if (depth>=1) {

		fBgBitmap->Lock();
	
		BRect bounds(-8.0, -2.0, fBgView->Bounds().right-8.0, fBgView->Bounds().bottom-2.0 );

		// Begrenzung
		BRect rect = bounds.InsetByCopy(6.0, 3.0);
		fBgView->SetHighColor( 255, 255, 255 );
		fBgView->StrokeRect( BRect(rect.left, rect.top, rect.right, rect.bottom));
		fBgView->SetHighColor( 128, 128, 128 );
		fBgView->StrokeRect( BRect(rect.left, rect.top, rect.right-1, rect.bottom-1));
		fBgView->SetHighColor( 0, 0, 0 );
		fBgView->StrokeRect( rect.InsetByCopy(1.0, 1.0));

		// Marker
		fBgView->SetHighColor( ViewColor() );
		fBgView->FillRect( BRect(bounds.left, bounds.top, bounds.left+5.0, bounds.bottom) );
		fBgView->FillRect( BRect(bounds.right - 5.0, bounds.top, bounds.right, bounds.bottom) );
		
		fBgView->SetHighColor( 0, 0, 0 );
		float value = Value();
		fBgView->StrokeLine( BPoint(bounds.left, value - 2.0), BPoint(bounds.left + 5.0, value + 3.0));
		fBgView->StrokeLine( BPoint(bounds.left, value + 8.0));
		fBgView->StrokeLine( BPoint(bounds.left, value - 2.0));

		fBgView->StrokeLine( BPoint(bounds.right, value - 2.0), BPoint(bounds.right - 5.0, value + 3.0));
		fBgView->StrokeLine( BPoint(bounds.right, value + 8.0));
		fBgView->StrokeLine( BPoint(bounds.right, value - 2.0));

		fBgView->Sync();

		fBgBitmap->Unlock();
	}

	DrawBitmap( fBgBitmap );

}

void ColorSlider::DrawColorLine( BView *view, float y, int r, int g, int b) {
	rgb_color color = {r, g, b, 255};
	y = 255.0 - y;
	view->AddLine( BPoint(0.0, y+3.0), BPoint(19.0, y+3.0), color );
}


void ColorSlider::MouseDown(BPoint where) {

	Window()->Activate();
	
	fMouseDown = true;
	SetMouseEventMask(B_POINTER_EVENTS, B_SUSPEND_VIEW_FOCUS|B_LOCK_WINDOW_FOCUS );
	SetValue( (int)where.y - 2 );
	Invoke();
}

void ColorSlider::MouseUp(BPoint where) {
	fMouseDown = false;
}

void ColorSlider::MouseMoved( BPoint where, uint32 code, const BMessage *a_message) {
	
	if (a_message || !fMouseDown ) {
		BView::MouseMoved( where, code, a_message);
		return;
	}
	
	SetValue( (int)where.y - 2 );
	Invoke();
}

void ColorSlider::SetValue( int32 value ) {

	value = max_c( min_c(value, 255), 0);
	BControl::SetValue( value );
	
	Update(2);

}

void ColorSlider::SetModeAndValues(selected_color_mode mode, float value1, float value2) {

	float R(0), G(0), B(0);
	float h(0), s(0), v(0);
	
	fBgBitmap->Lock();
	
	switch (fMode) {
		
		case R_SELECTED: {
			R = 255 - Value();
			G = round(fFixedValue1 * 255.0);
			B = round(fFixedValue2 * 255.0);
		}; break;
		
		case G_SELECTED: {
			R = round(fFixedValue1 * 255.0);
			G = 255 - Value();
			B = round(fFixedValue2 * 255.0);
		}; break;
		
		case B_SELECTED: {
			R = round(fFixedValue1 * 255.0);
			G = round(fFixedValue2 * 255.0);
			B = 255 - Value();
		}; break;
		
		case H_SELECTED: {
			h = (1.0 - (float)Value()/255.0)*6.0;
			s = fFixedValue1;
			v = fFixedValue2;
		}; break;
		
		case S_SELECTED: {
			h = fFixedValue1;
			s = 1.0 - (float)Value()/255.0;
			v = fFixedValue2;
		}; break;
		
		case V_SELECTED: {
			h = fFixedValue1;
			s = fFixedValue2;
			v = 1.0 - (float)Value()/255.0;
		}; break;
	}

	if (fMode & (H_SELECTED|S_SELECTED|V_SELECTED) ) {
		HSV_to_RGB(h, s, v, R, G, B);
		R*=255.0; G*=255.0; B*=255.0;
	}
	
	rgb_color color = { round(R), round(G), round(B), 255 };
	
	fMode = mode;
	SetOtherValues(value1, value2);
	fBgBitmap->Unlock();

	SetMarkerToColor( color );
	Update(2);
}

void ColorSlider::SetOtherValues(float value1, float value2) {
	
	fFixedValue1 = value1;
	fFixedValue2 = value2;

	Update(2);
}

void ColorSlider::SetMarkerToColor( rgb_color color ) {

	float h, s, v;
	if (fMode & (H_SELECTED|S_SELECTED|V_SELECTED) ) {
		RGB_to_HSV( (float)color.red / 255.0, (float)color.green / 255.0, (float)color.blue / 255.0, h, s, v );
	}
	
	switch (fMode) {
				
		case R_SELECTED: {
			SetValue( 255 - color.red );
		} break;
		
		case G_SELECTED: {
			SetValue( 255 - color.green );
		} break;

		case B_SELECTED: {
			SetValue( 255 - color.blue );
		} break;
		
		case H_SELECTED: {
			SetValue( 255.0 - round(h / 6.0 * 255.0) );
		} break;
		
		case S_SELECTED: {
			SetValue( 255.0 - round(s * 255.0) );
		} break;
		
		case V_SELECTED: {
			SetValue( 255.0 - round(v * 255.0) );
		} break;
		
	}

	Update(1);
}
