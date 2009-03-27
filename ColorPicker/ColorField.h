/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#ifndef _COLOR_FIELD_H
#define _COLOR_FIELD_H

#include <Control.h>

#include "selected_color_mode.h"

#define MSG_COLOR_FIELD 'ColF'
#define MSG_UPDATE		'Updt'

class BBitmap;

class ColorField : public BControl {

public:
				ColorField( BPoint offset_point, selected_color_mode mode, float fixed_value);
				~ColorField();
				
status_t		Invoke(BMessage *msg = NULL);

void			Draw( BRect updateRect );
void			Update( int depth );

void			MouseDown(BPoint where);
void			MouseUp(BPoint where);
void			MouseMoved( BPoint where, uint32 code, const BMessage *a_message);

void			SetModeAndValue(selected_color_mode mode, float fixed_value);
void			SetFixedValue(float fixed_value);

void			SetMarkerToColor( rgb_color color );
void			PositionMarkerAt( BPoint where );

private:

static int32	UpdateThread( void *data );

inline void		DrawColorPoint( BPoint pt, int r, int g, int b );

selected_color_mode	fMode;
float				fFixedValue;

BPoint			fMarkerPosition;
BPoint			fLastMarkerPosition;

bool			fMouseDown;

BBitmap			*fBgBitmap[2];
BView			*fBgView[2];

thread_id		fUpdateThread;
port_id			fUpdatePort;
};

#endif
