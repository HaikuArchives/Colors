/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#ifndef _TEXT_CONTAINER_H
#define _TEXT_CONTAINER_H

#include <TextView.h>
#include <View.h>

class BMessageRunner;

class TextContainer : public BView {

public:
				TextContainer( BRect rect );
			
void			MessageReceived(BMessage *message);

void			MouseDown(BPoint where);
void			MouseUp(BPoint where);
void			MouseMoved( BPoint where, uint32 code, const BMessage *a_message);

void			Draw( BRect updateRect );

void			SetColor( bool foreground, rgb_color color );
void			SetColor( bool foreground, long int color );

rgb_color		GetColor( bool foreground );

private:

void			DragColor( BPoint where );

bool			fMouseDown;
BPoint			fMouseOffset;

BTextView		*fTextView;
rgb_color		fColor, fBgColor;

BBitmap			*fBackgroundBmp;
BMessageRunner	*fMessageRunner;

bool			fGotFirstClick;
char			fActiveColor;

};

#endif
