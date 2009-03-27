/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#ifndef _COLOR_SLIDER_H
#define _COLOR_SLIDER_H

#include <Control.h>

#include "selected_color_mode.h"

#define MSG_UPDATE			'Updt'
#define	MSG_COLOR_SLIDER	'ColS'

class BBitmap;

class ColorSlider : public BControl {

public:
				ColorSlider( BPoint offset_point, selected_color_mode mode, float value1, float value2);
				~ColorSlider();
				
void			AttachedToWindow();

status_t		Invoke(BMessage *msg = NULL);

void			Draw( BRect updateRect );
void			Update( int depth );
inline void		DrawColorLine( float y, int r, int g, int b);

void			MouseDown(BPoint where);
void			MouseUp(BPoint where);
void			MouseMoved( BPoint where, uint32 code, const BMessage *a_message);

void			SetModeAndValues(selected_color_mode mode, float value1, float value2);
void			SetOtherValues(float value1, float value2);

void			SetMarkerToColor( rgb_color color );
void			SetValue(int32 value);

private:

static int32		UpdateThread( void *data );
static inline void	DrawColorLine( BView *view, float y, int r, int g, int b);

selected_color_mode	fMode;
float				fFixedValue1;
float				fFixedValue2;

bool				fMouseDown;

BBitmap				*fBgBitmap;
BView				*fBgView;

thread_id			fUpdateThread;
port_id				fUpdatePort;
};

#endif
