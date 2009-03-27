/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#include "ColorWindow.h"

#include <Alert.h>
#include <Application.h>
#include <iostream.h>
#include <MenuItem.h>
#include <Message.h>
#include <PopUpMenu.h>

#include "ColorPickerView.h"
#include "ContainersView.h"
#include "Prefs.h"

ColorWindow::ColorWindow()

:	BWindow(
		BRect(0.0, 0.0, 400.0, 278.0),
 		"Colors!", B_TITLED_WINDOW,
		B_QUIT_ON_WINDOW_CLOSE|B_ASYNCHRONOUS_CONTROLS)
{

	AddChild( new ColorPickerView( BRect(0.0, 1.0, 400.0, 278.0) ) );
	AddChild( new ContainersView( BRect(0.0, 279.0, 400.0, 310.0) ) );

	SetSizeLimits( Bounds().Width(), Bounds().Width(), Bounds().Height(), 310.0 );
	SetWindowAlignment( B_PIXEL_ALIGNMENT, 0, 0, 0, 0, 0, 0, 32, 23 );	
	
	BPoint pt;
	if (prefs.FindPoint("window_position", &pt)!=B_OK) pt = BPoint(10.0, 30.0);
	
	MoveTo(pt);

	if (prefs.FindBool("window_floating")) {
		SetFeel( B_FLOATING_ALL_WINDOW_FEEL );
	}
	if (prefs.FindBool("window_accept_first_click")) {
		SetFlags( Flags()|B_WILL_ACCEPT_FIRST_CLICK );
	}
	if (prefs.FindBool("window_all_workspaces")) {
		SetWorkspaces( B_ALL_WORKSPACES );
	}

}

ColorWindow::~ColorWindow() {

	prefs.RemoveName("window_zoomed");
	prefs.AddBool("window_zoomed", Bounds().Height() != 310.0);

	prefs.RemoveName("window_position");
	prefs.AddPoint("window_position", Frame().LeftTop());
	
	prefs.RemoveName("window_floating");
	prefs.AddBool("window_floating", Feel()==B_FLOATING_ALL_WINDOW_FEEL);

	prefs.RemoveName("window_accept_first_click");
	prefs.AddBool("window_accept_first_click", Flags()&B_WILL_ACCEPT_FIRST_CLICK);

	prefs.RemoveName("window_all_workspaces");
	prefs.AddBool("window_all_workspaces", Workspaces()==B_ALL_WORKSPACES);

}

void ColorWindow::Show() {

	if (!prefs.FindBool("window_zoomed")) {
		Zoom();
		// Avoid unwanted zoom when window is temporary hidden
		prefs.RemoveName("window_zoomed"); 
		prefs.AddBool("window_zoomed", 1);
	}

	BWindow::Show();

}

void ColorWindow::MessageReceived(BMessage *message) {
	
	switch( message->what ) {

		case MSG_POPUP_MENU: {

			BPoint	pt;
			if (message->FindPoint("where", &pt)!=B_OK) return;
			
			BPopUpMenu	*menu = new BPopUpMenu("", false, false);

			BMenuItem	*item[3];

			item[0] = new BMenuItem("Floating window", NULL);
			item[0]->SetMarked( Feel()==B_FLOATING_ALL_WINDOW_FEEL );
			menu->AddItem(item[0]);
			
			item[1] = new BMenuItem("Accept first mouse click", NULL);
			item[1]->SetMarked( Flags()&B_WILL_ACCEPT_FIRST_CLICK );
			menu->AddItem(item[1]);

			item[2] = new BMenuItem("Follow to all workspaces", NULL);
			item[2]->SetMarked( Workspaces()==B_ALL_WORKSPACES );
			menu->AddItem(item[2]);
			
			menu->ResizeToPreferred();
			
			BMenuItem	*selected_item = menu->Go( pt );
			
			if (selected_item == item[0]) {

				if (!prefs.FindBool("floating_msg_showed")) {
					(new BAlert( "Information",
						"Please be aware of the following restrictions, when using "
						"the 'Floating window' mode:\n"
						"\n"
						"· You cannot minimize the window and...\n"
						"· ... the window doesn't appear in the Deskbar.\n"
						"\nUnfortunately there is no way to change this behaviour.",
						"Thanks for the information"
						))->Go();
					prefs.AddBool("floating_msg_showed", true);
				}

				SetFeel( 
					Feel()==B_FLOATING_ALL_WINDOW_FEEL ?
					B_NORMAL_WINDOW_FEEL :
					B_FLOATING_ALL_WINDOW_FEEL
				);
			}

			else if (selected_item == item[1] ) {
				SetFlags( Flags() ^ B_WILL_ACCEPT_FIRST_CLICK );
			}
			
			else if (selected_item == item[2] ) {
				SetWorkspaces(
					Workspaces()==B_ALL_WORKSPACES ?
					B_CURRENT_WORKSPACE :
					B_ALL_WORKSPACES
				);
			}

			delete menu;
			
		} break;
		
		default:
			BWindow::MessageReceived( message );
	}
}
