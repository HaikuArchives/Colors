/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>

#include <iostream>

#include "App.h"
#include "Prefs.h"

Prefs prefs;

Prefs::Prefs()
{

	// FindPath
	find_directory(B_USER_SETTINGS_DIRECTORY, &fSavePath);
	fSavePath.SetTo( fSavePath.Path(), PREFS_FILENAME );
	
	// --- Einstellungen vornehmen
	BFile		file( fSavePath.Path(), B_READ_ONLY );
	Unflatten( &file );
	
}

Prefs::~Prefs() {

	BFile		file( fSavePath.Path(), B_WRITE_ONLY|B_CREATE_FILE );
	Flatten( &file );

}
