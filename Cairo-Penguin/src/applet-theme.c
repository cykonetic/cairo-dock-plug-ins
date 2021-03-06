/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-theme.h"


static gchar * _penguin_get_animation_properties (GKeyFile *pKeyFile, const gchar *cAnimationName, PenguinAnimation *pAnimation, PenguinAnimation *pDefaultAnimation)
{
	if (! g_key_file_has_group (pKeyFile, cAnimationName))
		return NULL;
	cd_debug ("%s (%s)", __func__, cAnimationName);
	
	gchar *cFileName = g_key_file_get_string (pKeyFile, cAnimationName, "file", NULL);
	if (cFileName != NULL && *cFileName == '\0')
	{
		g_free (cFileName);
		cFileName = NULL;
	}
	
	GError *erreur = NULL;
	pAnimation->iNbDirections = g_key_file_get_integer (pKeyFile, cAnimationName, "nb directions", &erreur);
	if (erreur != NULL)
	{
		pAnimation->iNbDirections = pDefaultAnimation->iNbDirections;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	pAnimation->iNbFrames = g_key_file_get_integer (pKeyFile, cAnimationName, "nb frames", &erreur);
	if (erreur != NULL)
	{
		pAnimation->iNbFrames = pDefaultAnimation->iNbFrames;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	pAnimation->iSpeed = g_key_file_get_integer (pKeyFile, cAnimationName, "speed", &erreur);
	if (erreur != NULL)
	{
		pAnimation->iSpeed = pDefaultAnimation->iSpeed;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	pAnimation->iAcceleration = g_key_file_get_integer (pKeyFile, cAnimationName, "acceleration", &erreur);
	if (erreur != NULL)
	{
		pAnimation->iAcceleration = pDefaultAnimation->iAcceleration;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	pAnimation->iTerminalVelocity = g_key_file_get_integer (pKeyFile, cAnimationName, "terminal velocity", &erreur);
	if (erreur != NULL)
	{
		pAnimation->iTerminalVelocity = pDefaultAnimation->iTerminalVelocity;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	pAnimation->bEnding = g_key_file_get_boolean (pKeyFile, cAnimationName, "ending", &erreur);
	if (erreur != NULL)
	{
		pAnimation->bEnding = pDefaultAnimation->bEnding;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	pAnimation->iDirection = g_key_file_get_integer (pKeyFile, cAnimationName, "direction", &erreur);
	if (erreur != NULL)
	{
		pAnimation->iDirection = pDefaultAnimation->iDirection;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	return cFileName;
}


void penguin_load_theme (GldiModuleInstance *myApplet, gchar *cThemePath)
{
	g_return_if_fail (cThemePath != NULL);
	cd_message ("%s (%s)", __func__, cThemePath);
	
	//\___________________ On ouvre le fichier de conf.
	gchar *cConfFilePath = g_strconcat (cThemePath, "/theme.conf", NULL);
	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("Cairo-Penguin : %s", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	myData.fFrameDelay = (double) g_key_file_get_integer (pKeyFile, "Theme", "delay", &erreur) * 1e-3;
	if (erreur != NULL)
	{
		cd_warning ("Cairo-Penguin : %s", erreur->message);
		myData.fFrameDelay = .1;
		g_error_free (erreur);
		erreur = NULL;
	}
	
	_penguin_get_animation_properties (pKeyFile, "Default", &myData.defaultAnimation, &myData.defaultAnimation);
	
	gsize length = 0;
	gchar **pGroupList = g_key_file_get_groups (pKeyFile, &length);
	g_return_if_fail (length > 0);
	
	g_free (myData.pAnimations);
	myData.iNbAnimations = 0;
	myData.pAnimations = g_new0 (PenguinAnimation, length - 1);
	
	g_free (myData.pBeginningAnimations);
	myData.iNbBeginningAnimations = 0;
	myData.pBeginningAnimations = g_new0 (int, length - 1);
	
	g_free (myData.pEndingAnimations);
	myData.iNbEndingAnimations = 0;
	myData.pEndingAnimations = g_new0 (int, length - 1);
	
	g_free (myData.pGoUpAnimations);
	myData.iNbGoUpAnimations = 0;
	myData.pGoUpAnimations = g_new0 (int, length - 1);
	
	g_free (myData.pMovmentAnimations);
	myData.iNbMovmentAnimations = 0;
	myData.pMovmentAnimations = g_new0 (int, length - 1);
	
	g_free (myData.pRestAnimations);
	myData.iNbRestAnimations = 0;
	myData.pRestAnimations = g_new0 (int, length - 1);
	
	PenguinAnimation *pAnimation;
	gchar *cFileName, *cGroupName;
	int i, iNumAnimation = 0;
	for (i = 0; pGroupList[i] != NULL; i++)
	{
		cGroupName = pGroupList[i];
		if (strcmp (cGroupName, "Theme") != 0 && strcmp (cGroupName, "Default") != 0)
		{
			pAnimation = &myData.pAnimations[iNumAnimation];
			
			cFileName = _penguin_get_animation_properties (pKeyFile, cGroupName, pAnimation, &myData.defaultAnimation);
			if (cFileName != NULL)
			{
				pAnimation->cFilePath = g_strconcat (cThemePath, "/", cFileName, NULL);
				g_free (cFileName);
			}
			if (pAnimation->bEnding)
			{
				myData.pEndingAnimations[myData.iNbEndingAnimations++] = iNumAnimation;
				cd_debug (" %s : ending", pAnimation->cFilePath);
			}
			else if (pAnimation->iDirection == PENGUIN_DOWN)  // descente.
			{
				myData.pBeginningAnimations[myData.iNbBeginningAnimations++] = iNumAnimation;
				cd_debug (" %s : beginning", pAnimation->cFilePath);
			}
			else if (pAnimation->iDirection == PENGUIN_UP)
			{
				myData.pGoUpAnimations[myData.iNbGoUpAnimations++] = iNumAnimation;
				cd_debug (" %s : go up", pAnimation->cFilePath);
			}
			else if (pAnimation->iSpeed == 0 && pAnimation->iAcceleration == 0 && pAnimation->iNbFrames == 1)
			{
				myData.pRestAnimations[myData.iNbRestAnimations++] = iNumAnimation;
				cd_debug (" %s : rest", pAnimation->cFilePath);
			}
			else
			{
				myData.pMovmentAnimations[myData.iNbMovmentAnimations++] = iNumAnimation;
				cd_debug (" %s : moving", pAnimation->cFilePath);
			}
			
			iNumAnimation ++;
		}
	}
	
	g_strfreev (pGroupList);
	g_free (cConfFilePath);
	g_key_file_free (pKeyFile);
}


void penguin_load_animation_buffer (PenguinAnimation *pAnimation, cairo_t *pSourceContext, double fAlpha, gboolean bLoadTexture)
{
	cd_debug ("%s (%s)", __func__, pAnimation->cFilePath);
	if (pAnimation->cFilePath == NULL)
		return;
	
	CairoDockImageBuffer *pImageBuffer = cairo_dock_create_image_buffer (pAnimation->cFilePath,
		0,
		0,
		0);  // on charge l'image a sa taille naturelle.
	
	/**double fImageWidth=0, fImageHeight=0;
	cairo_surface_t *pBigSurface = cairo_dock_load_image (
		pSourceContext,
		pAnimation->cFilePath,
		&fImageWidth,
		&fImageHeight,
		0.,
		fAlpha,
		FALSE);*/
	pAnimation->iFrameWidth = (int) pImageBuffer->iWidth / pAnimation->iNbFrames;
	pAnimation->iFrameHeight = (int) pImageBuffer->iHeight / pAnimation->iNbDirections;
	cd_debug ("  surface chargee (%dx%d)", pAnimation->iFrameWidth, pAnimation->iFrameHeight);
	if (bLoadTexture)
	{
		pAnimation->iTexture = pImageBuffer->iTexture;
		pImageBuffer->iTexture = 0;  // on ne veut pas detruire la texture.
	}
	else if (pImageBuffer->pSurface != NULL)
	{
		pAnimation->pSurfaces = g_new (cairo_surface_t **, pAnimation->iNbDirections);
		int i, j;
		for (i = 0; i < pAnimation->iNbDirections; i ++)
		{
			pAnimation->pSurfaces[i] = g_new (cairo_surface_t *, pAnimation->iNbFrames);
			for (j = 0; j < pAnimation->iNbFrames; j ++)
			{
				//cd_debug ("    dir %d, frame %d)", i, j);
				pAnimation->pSurfaces[i][j] = cairo_surface_create_similar (cairo_get_target (pSourceContext),
					CAIRO_CONTENT_COLOR_ALPHA,
					pAnimation->iFrameWidth,
					pAnimation->iFrameHeight);
				cairo_t *pCairoContext = cairo_create (pAnimation->pSurfaces[i][j]);
				
				cairo_set_source_surface (pCairoContext,
					pImageBuffer->pSurface,
					- j * pAnimation->iFrameWidth,
					- i * pAnimation->iFrameHeight);
				cairo_paint (pCairoContext);
				
				cairo_destroy (pCairoContext);
			}
		}
	}
	cairo_dock_free_image_buffer (pImageBuffer);
}
