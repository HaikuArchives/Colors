/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#ifndef _CONTAINER_H
#define _CONTAINER_H

#include <View.h>

#define MSG_MESSAGERUNNER 'MsgR'

class BMessageRunner;

class Container : public BView {

public:
				Container( BRect rect );
			
void			MessageReceived(BMessage *message);

void			MouseDown(BPoint where);
void			MouseUp(BPoint where);
void			MouseMoved( BPoint where, uint32 code, const BMessage *a_message);

void			Draw( BRect updateRect );

void			SetColor( rgb_color color );
void			SetColor( long int color );

rgb_color		GetColor();

private:

void			DragColor( BPoint where );

bool			fMouseDown;
BPoint			fMouseOffset;

rgb_color		fColor;

BBitmap			*fBackgroundBmp;
BMessageRunner	*fMessageRunner;

bool			fGotFirstClick;

};

#endif
