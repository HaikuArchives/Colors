/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#include "ColorPickerView.h"

#include <Application.h>
#include <Bitmap.h>
#include <Beep.h>
#include <iostream.h>
#include <Font.h>
#include <Message.h>
#include <RadioButton.h>
#include <Resources.h>
#include <Screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <TextControl.h>
#include <Window.h>

#include "BackgroundView.h"
#include "ColorField.h"
#include "ColorPreview.h"
#include "ColorSlider.h"
#include "ColorWindow.h"
#include "Convert_RGB_HSV.h"
#include "EyeDropper.h"
#include "Prefs.h"

#define round(x) (int)(x+.5)
#define hexdec(str, offset) (int)(((str[offset]<60?str[offset]-48:(str[offset]|32)-87)<<4)|(str[offset+1]<60?str[offset+1]-48:(str[offset+1]|32)-87))

ColorPickerView::ColorPickerView( BRect rect )

:	BView( rect, "", B_FOLLOW_TOP|B_FOLLOW_LEFT, B_WILL_DRAW|B_PULSE_NEEDED),
	fMouseDown( false ),
	fRequiresUpdate( false )

{
	SetViewColor(230, 230, 230);

	h = 0.0; s = 1.0; v = 1.0;
	r = 1.0; g = 0.0; b = 0.0;
	
	int32	int_color;
	if (prefs.FindInt32("color", &int_color)==B_OK) {
	
		r	= (float)(int_color >> 16) / 255;
		g	= (float)((int_color >> 8) & 255) / 255;
		b	= (float)(int_color & 255) / 255;

		RGB_to_HSV( r, g, b, h, s, v );
	}

	SetColorMode( S_SELECTED, false );
	
}

ColorPickerView::~ColorPickerView() {

	prefs.RemoveName("color");

	int32	int_color = ((int)(r*255)<<16) + ((int)(g*255)<<8) + (int)(b*255);
	prefs.AddInt32("color", int_color);

}

void ColorPickerView::AttachedToWindow() {

	rgb_color	color = { (int)(r * 255), (int)(g * 255), (int)(b * 255), 255 };

	BView::AttachedToWindow();

	BResources	*Resourcen = be_app->AppResources();

	size_t		groesse;
	BMessage	msg;
	char		*buf = (char *)Resourcen->LoadResource('BBMP', "backgroundpattern", &groesse);
	BBitmap		*background = 0;
	if (buf) {
		msg.Unflatten(buf);
		background = new BBitmap( &msg );
		SetViewBitmap(background);
	}
	else {
		cerr << "*** Resource not found" << endl;
	}

	fColorField = new ColorField( BPoint(10.0, 10.0), fSelectedColorMode, *p );
	fColorField->SetMarkerToColor( color );
	AddChild( fColorField );
	fColorField->SetTarget( this );
	
	fColorSlider = new ColorSlider( BPoint(278.0, 7.0), fSelectedColorMode, *p1, *p2 );
	fColorSlider->SetMarkerToColor( color );
	AddChild( fColorSlider );
	fColorSlider->SetTarget( this );

	fColorPreview = new ColorPreview( BRect( 0.0, 0.0, 56.0, 50.0).OffsetToCopy(326.0, 10.0), color );
	AddChild( fColorPreview );
	fColorPreview->SetTarget( this );

	EyeDropper *eyeDropper = new EyeDropper( BRect( 0.0, 0.0, 56.0, 19.0).OffsetToCopy(326.0, 62.0) );
	AddChild( eyeDropper );
	eyeDropper->SetTarget( this );

	BFont font(be_plain_font);
	font.SetSize(10.0);
	SetFont( &font );
	
	const char *title[] = { "H", "S", "V", "R", "G", "B" };
	
	BTextView	*textView;
	
	for (int i=0; i<6; ++i) {
	
		fRadioButton[i] = new BRadioButton(BRect(0.0, 0.0, 30.0, 10.0).OffsetToCopy(320.0, 92.0 + 24.0 * i + (int)i/3 * 8),
			NULL, title[i], new BMessage(MSG_RADIOBUTTON + i));
		fRadioButton[i]->SetFont( &font );
		AddChild( fRadioButton[i] );

		fRadioButton[i]->SetTarget( this );

		if (i==1) fRadioButton[i]->SetValue(1);

		fRadioButton[i]->SetViewBitmap(background);

	}
	 
	for (int i=0; i<6; ++i) {
	
		fTextControl[i] = new BTextControl(BRect(0.0, 0.0, 32.0, 19.0).OffsetToCopy(350.0, 90.0 + 24.0 * i + (int)i/3 * 8),
			NULL, NULL, NULL, new BMessage(MSG_TEXTCONTROL + i));

		textView = fTextControl[i]->TextView();
		textView->SetMaxBytes(3);
		for (int j=32; j<255; ++j) {
			if (j<'0'||j>'9') textView->DisallowChar(j);
		}
		
		fTextControl[i]->SetFont( &font );
		fTextControl[i]->SetDivider(0.0);
		AddChild( fTextControl[i] );

		fTextControl[i]->SetTarget( this );
	}
	 
	fHexTextControl = new BTextControl(BRect(0.0, 0.0, 69.0, 19.0).OffsetToCopy(320.0, 248.0),
			NULL, "#", NULL, new BMessage(MSG_HEXTEXTCONTROL));

	textView = fHexTextControl->TextView();
	textView->SetMaxBytes(6);
	for (int j=32; j<255; ++j) {
		if (!( (j>='0' && j<='9') || (j>='a' && j<='f') || (j>='A' && j<='F') ) ) textView->DisallowChar(j);
	}
	
	fHexTextControl->SetFont( &font );
	fHexTextControl->SetDivider(12.0);
	AddChild( fHexTextControl );

	fHexTextControl->SetViewBitmap(background);
	fHexTextControl->SetTarget( this );
	
	UpdateTextControls();
	
	delete background;

}

void ColorPickerView::MessageReceived(BMessage *message) {
	
	switch (message->what) {

		case MSG_COLOR_FIELD: {
			float	value1, value2;
			value1 = message->FindFloat( "value" );
			value2 = message->FindFloat( "value", 1);
			UpdateColor( -1, value1, value2 );
			fRequiresUpdate = true;
		} break;
		
		case MSG_COLOR_SLIDER: {
			float	value;
			message->FindFloat( "value", &value);
			UpdateColor( value, -1, -1 );
			fRequiresUpdate = true;
		} break;
		
		case MSG_COLOR_PREVIEW: {
			rgb_color	*color;
			ssize_t		numBytes;
			if (message->FindData("color", B_RGB_COLOR_TYPE, (const void **)&color, &numBytes)==B_OK) {
				color->alpha = 255;
				SetColor( *color );
			}
		} break;
		
		case MSG_EYEDROPPER: {
			GrabColor();
		} break;
		
		case MSG_RADIOBUTTON: {
			SetColorMode( H_SELECTED );
		} break;

		case MSG_RADIOBUTTON + 1: {
			SetColorMode( S_SELECTED );
		} break;

		case MSG_RADIOBUTTON + 2: {
			SetColorMode( V_SELECTED );
		} break;

		case MSG_RADIOBUTTON + 3: {
			SetColorMode( R_SELECTED );
		} break;

		case MSG_RADIOBUTTON + 4: {
			SetColorMode( G_SELECTED );
		} break;

		case MSG_RADIOBUTTON + 5: {
			SetColorMode( B_SELECTED );
		} break;
		
		case MSG_TEXTCONTROL:
		case MSG_TEXTCONTROL + 1:
		case MSG_TEXTCONTROL + 2:
		case MSG_TEXTCONTROL + 3:
		case MSG_TEXTCONTROL + 4:
		case MSG_TEXTCONTROL + 5: {

			int nr = message->what - MSG_TEXTCONTROL;
			int value = atoi(fTextControl[nr]->Text());
			
			char string[4];

			switch (nr) {
				case 0: {
					value %= 360;
					sprintf(string, "%d", value);
					h = (float)value / 60;
				} break;

				case 1: {
					value = min_c(value, 100);
					sprintf(string, "%d", value);
					s = (float)value / 100;
				} break;

				case 2: {
					value = min_c(value, 100);
					sprintf(string, "%d", value);
					v = (float)value / 100;
				} break;

				case 3: {
					value = min_c(value, 255);
					sprintf(string, "%d", value);
					r = (float)value / 255;
				} break;

				case 4: {
					value = min_c(value, 255);
					sprintf(string, "%d", value);
					g = (float)value / 255;
				} break;

				case 5: {
					value = min_c(value, 255);
					sprintf(string, "%d", value);
					b = (float)value / 255;
				} break;
			}
			
			if (nr<3) { // hsv-mode
				HSV_to_RGB( h, s, v, r, g, b );
			}

			rgb_color color = { round(r*255), round(g*255), round(b*255), 255 };

			SetColor( color );
			
		} break;
		
		case MSG_HEXTEXTCONTROL: {
			if (fHexTextControl->TextView()->TextLength()==6) {
				const char *string = fHexTextControl->TextView()->Text();
				rgb_color color = { hexdec(string, 0), hexdec(string, 2), hexdec(string, 4), 255 };
				SetColor( color );
			}
		} break;

		default:
			BView::MessageReceived( message );
	}
}

void ColorPickerView::MouseDown(BPoint where) {
	
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

void ColorPickerView::MouseUp(BPoint where) {
	fMouseDown = false;
	BView::MouseUp( where );
}

void ColorPickerView::MouseMoved( BPoint where, uint32 code, const BMessage *a_message) {
	
	if (fMouseDown) {

		BPoint win_pos = Window()->Frame().LeftTop();
		Window()->MoveTo( win_pos.x + where.x - fMouseOffset.x, win_pos.y + where.y - fMouseOffset.y );

	}
	else {
		BView::MouseMoved( where, code, a_message);
	}
}

void ColorPickerView::UpdateColor(float value, float value1, float value2) {

	if (value!=-1) {
		fColorField->SetFixedValue(value);
		*p = value;
	}
	else if (value1!=-1 && value2!=-1) {
		fColorSlider->SetOtherValues(value1, value2);
		*p1 = value1; *p2 = value2;
	}

	if (fSelectedColorMode & (R_SELECTED|G_SELECTED|B_SELECTED) ) RGB_to_HSV( r, g, b, h, s, v );
	else HSV_to_RGB( h, s, v, r, g, b );

	rgb_color color = { (int)(r*255), (int)(g*255), (int)(b*255), 255 };
	fColorPreview->SetColor( color );
	
}

void ColorPickerView::SetColor( rgb_color color ) {

	r = (float)color.red/255; g = (float)color.green/255; b = (float)color.blue/255;
	RGB_to_HSV( r, g, b, h, s, v );
	
	fColorSlider->SetModeAndValues(fSelectedColorMode, *p1, *p2);
	fColorSlider->SetMarkerToColor( color );

	fColorField->SetModeAndValue(fSelectedColorMode, *p);
	fColorField->SetMarkerToColor( color );

	fColorPreview->SetColor( color );
	
	fRequiresUpdate = true;
}

void ColorPickerView::SetColorMode( selected_color_mode mode, bool update ) {

	fSelectedColorMode = mode;
	switch (mode) {
		case R_SELECTED:
			p = &r; p1 = &g; p2 = &b;
		break;

		case G_SELECTED:
			p = &g; p1 = &r; p2 = &b;
		break;

		case B_SELECTED:
			p = &b; p1 = &r; p2 = &g;
		break;

		case H_SELECTED:
			p = &h; p1 = &s; p2 = &v;
		break;

		case S_SELECTED:
			p = &s; p1 = &h; p2 = &v;
		break;

		case V_SELECTED:
			p = &v; p1 = &h; p2 = &s;
		break;
	}
	
	if (!update) return;

	fColorSlider->SetModeAndValues(fSelectedColorMode, *p1, *p2);
	fColorField->SetModeAndValue(fSelectedColorMode, *p);
	
}

void ColorPickerView::Pulse() {
	if (fRequiresUpdate) UpdateTextControls();
}

void ColorPickerView::UpdateTextControls() {

	Window()->DisableUpdates();

	char	string[5];

	sprintf(string, "%d", round(h*60) );
	fTextControl[0]->TextView()->SetText(string);
	
	sprintf(string, "%d", round(s*100) );
	fTextControl[1]->TextView()->SetText(string);

	sprintf(string, "%d", round(v*100) );
	fTextControl[2]->TextView()->SetText(string);

	sprintf(string, "%d", round(r*255) );
	fTextControl[3]->TextView()->SetText(string);
	
	sprintf(string, "%d", round(g*255) );
	fTextControl[4]->TextView()->SetText(string);

	sprintf(string, "%d", round(b*255) );
	fTextControl[5]->TextView()->SetText(string);

	sprintf(string, "%.6X", (round(r*255)<<16)|(round(g*255)<<8)|round(b*255) );
	fHexTextControl->TextView()->SetText(string);

	Window()->EnableUpdates();

	fRequiresUpdate = false;
	
}

void ColorPickerView::Draw( BRect updateRect ) {
	
	BView::Draw( updateRect );

	SetDrawingMode( B_OP_ALPHA );
	SetHighColor(255, 255, 255, 200);
	StrokeLine( Bounds().RightTop(), Bounds().LeftTop() );
	StrokeLine( Bounds().LeftBottom() );
	SetHighColor(0, 0, 0, 100);
	StrokeLine( Bounds().RightBottom() );
	StrokeLine( Bounds().RightTop() );
	SetDrawingMode( B_OP_COPY );

	font_height fontHeight;
	BFont font;
	GetFont( &font );
	
	font.GetHeight(&fontHeight);
	
	const char *title[] = { "°", "%", "%" };
	for (int i=0; i<3; ++i) {
		DrawString(title[i], BPoint(385.0, 93.0 + 24.0 * i + (int)i/3 * 8 + fontHeight.ascent));
	}

}

void ColorPickerView::GrabColor() {

	BScreen	screen( Window() );
	BWindow *win = new BWindow( screen.Frame(), "", B_NO_BORDER_WINDOW_LOOK, B_FLOATING_ALL_WINDOW_FEEL, 0 );
	BView *view = new BackgroundView( screen.Frame().OffsetToCopy(0.0, 0.0) );
	win->AddChild( view );
	view->MakeFocus();

	win->Show();
	
}
