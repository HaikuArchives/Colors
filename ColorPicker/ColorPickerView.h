/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#ifndef _COLORPICKER_VIEW_H
#define _COLORPICKER_VIEW_H

#include <View.h>

#include "selected_color_mode.h"

#define	MSG_RADIOBUTTON		'Rad0'
#define	MSG_TEXTCONTROL		'Txt0'
#define MSG_HEXTEXTCONTROL	'HTxt'

class ColorField;
class ColorSlider;
class ColorPreview;

class BRadioButton;
class BTextControl;

class ColorPickerView : public BView {

public:
			ColorPickerView( BRect rect );
			~ColorPickerView();
			
void		AttachedToWindow();
void		MessageReceived(BMessage *message);

void		MouseDown(BPoint where);
void		MouseUp(BPoint where);
void		MouseMoved( BPoint where, uint32 code, const BMessage *a_message);

void		SetColorMode( selected_color_mode mode, bool update = true );
void		SetColor( rgb_color color );

void		Draw( BRect updateRect );
void		Pulse();

private:

void		UpdateColor(float value, float value1, float value2);
void		UpdateTextControls();

void		GrabColor();

selected_color_mode	fSelectedColorMode;

float			h, s, v, r, g, b;
float			*p, *p1, *p2;

bool			fMouseDown;
BPoint			fMouseOffset;

bool			fRequiresUpdate;

ColorField		*fColorField;
ColorSlider		*fColorSlider;
ColorPreview	*fColorPreview;

BRadioButton	*fRadioButton[6];
BTextControl	*fTextControl[6];
BTextControl	*fHexTextControl;

};

#endif
