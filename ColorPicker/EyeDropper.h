/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#ifndef _EYE_DROPPER_H
#define _EYE_DROPPER_H

#include <Control.h>

#define MSG_EYEDROPPER	'EyeD'

class BBitmap;

class EyeDropper : public BControl {

public:
			EyeDropper( BRect frame );
			~EyeDropper();
			
void		Draw( BRect updateRect );

void		MouseDown(BPoint where);
void		MouseUp(BPoint where);
void		MouseMoved(	BPoint where, uint32 code, const BMessage *a_message);

private:

bool		fMouseDown;
bool		fButtonDown;
BBitmap		*fImage[2];

};

#endif
