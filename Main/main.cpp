/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#include "App.h"

#include "ColorWindow.h"

App::App()
:	BApplication("application/x-vnd.pecora-colors") {

	ColorWindow *aColorWin=new ColorWindow();
	aColorWin->Show();

	
}

int main() {
	App *aApp=new App();
	aApp->Run();
	return 0;
}
