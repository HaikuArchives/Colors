/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#ifndef _COLORWINDOW_H
#define _COLORWINDOW_H

#include <Window.h>

#define	MSG_POPUP_MENU		'PopU'
#define MSG_AUTOBORDERLESS	'AutB'

class ColorWindow : public BWindow {

public:

			ColorWindow();
virtual		~ColorWindow();

void		Show();

void		MessageReceived(BMessage *message);

};

#endif
