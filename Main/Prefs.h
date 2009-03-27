/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#ifndef _PREFS_H
#define _PREFS_H

#include <Path.h>

#define	PREFS_FILENAME	"Colors!_settings"

class Prefs : public BMessage {

public:
					Prefs();
					~Prefs();

private:

BPath				fSavePath;

};

extern Prefs prefs;
	
#endif
