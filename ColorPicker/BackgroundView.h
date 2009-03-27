/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#ifndef _BACKGROUND_VIEW_H
#define _BACKGROUND_VIEW_H

#include <View.h>

class	BBitmap;

class BackgroundView : public BView {

public:
			BackgroundView( BRect frame );
			~BackgroundView();
			
void		Draw( BRect bounds );
void		AttachedToWindow();

void		MouseDown(BPoint where);
void		MouseMoved(	BPoint where, uint32 code, const BMessage *a_message);

void		KeyDown(const char *bytes, int32 numBytes);

private:

rgb_color	ColorAt( BPoint pt );
void		Close();

BBitmap		*fBitmap;
uint32		fWorkspace;
BWindow		*fHiddenWindow;
};

#endif
