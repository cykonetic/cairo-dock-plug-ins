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
#include <math.h>

#include "applet-struct.h"
#include "applet-evaporate.h"
#include "applet-fade-out.h"
#include "applet-explode.h"
#include "applet-break.h"
#include "applet-black-hole.h"
#include "applet-lightning.h"
#include "applet-notifications.h"


gboolean cd_illusion_on_remove_insert_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock)
{
	if (fabs (pIcon->fInsertRemoveFactor) < .1)  // useless or not needed animation.
	{
		cd_illusion_free_data (pUserData, pIcon);  // be sure we won't handle the animation, in case the previous appearance animation was still here
		return GLDI_NOTIFICATION_LET_PASS;
	}
	if (! CAIRO_CONTAINER_IS_OPENGL (CAIRO_CONTAINER (pDock)))  // gere le cas ou pDock est NULL.
		return GLDI_NOTIFICATION_LET_PASS;
	
	CDIllusionData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
	{
		pData = g_new0 (CDIllusionData, 1);
		pData->fDeltaT = (double) cairo_dock_get_animation_delta_t (CAIRO_CONTAINER (pDock));
		pData->sens = (pIcon->fInsertRemoveFactor > .05 ? 1 : -1);
		CD_APPLET_SET_MY_ICON_DATA (pIcon, pData);
		
		gboolean bSartAnimation = FALSE;
		CDIllusionEffect iEffect = (pData->sens == 1 ? myConfig.iDisappearanceEffect : myConfig.iAppearanceEffect);
		if (iEffect >= CD_ILLUSION_NB_EFFECTS)
			iEffect = g_random_int_range (0, CD_ILLUSION_NB_EFFECTS);
		switch (iEffect)
		{
			case CD_ILLUSION_EVAPORATE :
				pData->iEffectDuration = myConfig.iEvaporateDuration;
				pData->fTimeLimitPercent = .8;
				if (pData->sens == -1)
					pData->fTime = pData->iEffectDuration;  // on part a rebours.
				bSartAnimation = cd_illusion_init_evaporate (pIcon, pDock, pData);
			break ;
			case CD_ILLUSION_FADE_OUT :
				pData->iEffectDuration = myConfig.iFadeOutDuration;
				pData->fTimeLimitPercent = .75;
				if (pData->sens == -1)
					pData->fTime = pData->iEffectDuration;
				bSartAnimation = cd_illusion_init_fade_out (pIcon, pDock, pData);
			break ;
			case CD_ILLUSION_EXPLODE :
				pData->iEffectDuration = myConfig.iExplodeDuration;
				pData->fTimeLimitPercent = .9;
				if (pData->sens == -1)
					pData->fTime = pData->iEffectDuration;
				bSartAnimation = cd_illusion_init_explode (pIcon, pDock, pData);
			break ;
			case CD_ILLUSION_BREAK :
				pData->iEffectDuration = myConfig.iBreakDuration;
				pData->fTimeLimitPercent = 1.;
				if (pData->sens == -1)
					pData->fTime = pData->iEffectDuration;
				bSartAnimation = cd_illusion_init_break (pIcon, pDock, pData);
			break ;
			case CD_ILLUSION_BLACK_HOLE :
				pData->iEffectDuration = myConfig.iBlackHoleDuration;
				pData->fTimeLimitPercent = 1.;
				if (pData->sens == -1)
					pData->fTime = pData->iEffectDuration;
				bSartAnimation = cd_illusion_init_black_hole (pIcon, pDock, pData);
			break ;
			case CD_ILLUSION_LIGHTNING :
				pData->iEffectDuration = myConfig.iLightningDuration;
				pData->fTimeLimitPercent = 1.;
				if (pData->sens == -1)
					pData->fTime = pData->iEffectDuration;
				bSartAnimation = cd_illusion_init_lightning (pIcon, pDock, pData);
			break ;
			default :
			break ;
		}
		if (bSartAnimation)
		{
			pData->iCurrentEffect = iEffect;
		}
	}
	else  // si on a un pData, c'est qu'on etait deja en pleine animation, on garde la meme, qui partira en sens inverse a partir du temps actuel.
	{
		pData->sens = (pIcon->fInsertRemoveFactor > 0 ? 1 : -1);
	}
	
	return GLDI_NOTIFICATION_LET_PASS;
}


gboolean cd_illusion_render_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bHasBeenRendered, cairo_t *pCairoContext)
{
	if (pCairoContext != NULL || *bHasBeenRendered)
		return GLDI_NOTIFICATION_LET_PASS;
	CDIllusionData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	switch (pData->iCurrentEffect)
	{
		case CD_ILLUSION_EVAPORATE :
			cd_illusion_draw_evaporate_icon (pIcon, pDock, pData);
			*bHasBeenRendered = TRUE;
		break ;
		case CD_ILLUSION_FADE_OUT :
			cd_illusion_draw_fade_out_icon (pIcon, pDock, pData);
		break ;
		case CD_ILLUSION_EXPLODE :
			cd_illusion_draw_explode_icon (pIcon, pDock, pData);
			*bHasBeenRendered = TRUE;
		break ;
		case CD_ILLUSION_BREAK :
			cd_illusion_draw_break_icon (pIcon, pDock, pData);
			*bHasBeenRendered = TRUE;
		break ;
		case CD_ILLUSION_BLACK_HOLE :
			cd_illusion_draw_black_hole_icon (pIcon, pDock, pData);
			*bHasBeenRendered = TRUE;
		break ;
		case CD_ILLUSION_LIGHTNING :
			cd_illusion_draw_lightning_icon (pIcon, pDock, pData);
			*bHasBeenRendered = TRUE;
		break ;
		default :
		break ;
	}
	
	return GLDI_NOTIFICATION_LET_PASS;
}


gboolean cd_illusion_update_icon (gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bContinueAnimation)
{
	CDIllusionData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	pData->fTime += pData->sens * pData->fDeltaT;
	if (pData->fTime < 0)
		pData->fTime = 0;
	switch (pData->iCurrentEffect)
	{
		case CD_ILLUSION_EVAPORATE :
			cd_illusion_update_evaporate (pIcon, pDock, pData);
		break ;
		case CD_ILLUSION_FADE_OUT :
			cd_illusion_update_fade_out (pIcon, pDock, pData);
		break ;
		case CD_ILLUSION_EXPLODE :
			cd_illusion_update_explode (pIcon, pDock, pData);
		break ;
		case CD_ILLUSION_BREAK :
			cd_illusion_update_break (pIcon, pDock, pData);
		break ;
		case CD_ILLUSION_BLACK_HOLE :
			cd_illusion_update_black_hole (pIcon, pDock, pData);
		break ;
		case CD_ILLUSION_LIGHTNING :
			cd_illusion_update_lightning (pIcon, pDock, pData);
		break ;
		default :
		break ;
	}	
	
	if (pData->sens == 1 && pData->fTime < pData->fTimeLimitPercent * pData->iEffectDuration)  // if the icon is disappearing, we keep it at max size until x% of the animation is complete, so that we can see the animation (that's why we register after the core).
	{
		pIcon->fInsertRemoveFactor = 1.;
		*bContinueAnimation = TRUE;
	}
	
	if ((pData->sens == 1 && pData->fTime < pData->iEffectDuration)
	|| (pData->sens == -1 && pData->fTime > 0))  // the animation is not yet finished
	{
		*bContinueAnimation = TRUE;
	}
	else
	{
		cd_illusion_free_data (pUserData, pIcon);
	}
	return GLDI_NOTIFICATION_LET_PASS;
}


gboolean cd_illusion_free_data (gpointer pUserData, Icon *pIcon)
{
	cd_message ("");
	CDIllusionData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pData == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	cairo_dock_free_particle_system (pData->pEvaporateSystem);
	
	g_free (pData->pExplosionPart);
	
	g_free (pData->pBreakPart);
	
	g_free (pData->pBlackHolePoints);
	g_free (pData->pBlackHoleCoords);
	g_free (pData->pBlackHoleVertices);
	
	int i;
	for (i = 0; i < pData->iNbSources; i ++)
	{
		g_free (pData->pLightnings[i].pVertexTab);
	}
	g_free (pData->pLightnings);
	
	g_free (pData);
	CD_APPLET_SET_MY_ICON_DATA (pIcon, NULL);
	return GLDI_NOTIFICATION_LET_PASS;
}
