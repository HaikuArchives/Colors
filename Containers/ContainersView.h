/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#ifndef _CONTAINTERS_VIEW_H
#define _CONTAINTERS_VIEW_H

#include <View.h>

class Container;
class TextContainer;

class ContainersView : public BView {

public:
				ContainersView( BRect rect );
				~ContainersView();
				
void			MouseDown(BPoint where);
void			MouseUp(BPoint where);
void			MouseMoved( BPoint where, uint32 code, const BMessage *a_message);

void			Draw( BRect updateRect );

private:

bool			fMouseDown;
BPoint			fMouseOffset;

Container		*fColorContainer[10];
TextContainer 	*fTextContainer;
	

};

#endif
