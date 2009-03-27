/*
 * Copyrights (c):
 *     2001 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#ifndef _CONVERT_RGB_HSV_H
#define _CONVERT_RGB_HSV_H

void RGB_to_HSV( float r, float g, float b, float &h, float &s, float &v );
void HSV_to_RGB( float h, float s, float v, float &r, float &g, float &b );

#endif
