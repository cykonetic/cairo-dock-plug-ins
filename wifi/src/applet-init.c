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

#include "stdlib.h"

#include "applet-config.h"
#include "applet-notifications.h"
#include "applet-struct.h"
#include "applet-wifi.h"
#include "applet-draw.h"
#include "applet-init.h"


CD_APPLET_DEFINITION (N_("wifi"),
	2, 0, 7,
	CAIRO_DOCK_CATEGORY_APPLET_INTERNET,
	N_("This applet shows you the signal strength of the first active wifi connection\n"
	"Left-click to pop-up some info,"
	"Middle-click to re-check immediately."),
	"ChAnGFu (Rémy Robertson)");


static void _set_data_renderer (CairoDockModuleInstance *myApplet, gboolean bReload)
{
	if (myConfig.iDisplayType == CD_WIFI_BAR)
		return; /// TODO

	CairoDataRendererAttribute *pRenderAttr = NULL;  // attributes for the global data-renderer.
	CairoGaugeAttribute aGaugeAttr;  // gauge attributes.
	CairoGraphAttribute aGraphAttr;  // graph attributes.
	if (myConfig.iDisplayType == CD_WIFI_GAUGE)
	{
		memset (&aGaugeAttr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&aGaugeAttr);
		pRenderAttr->cModelName = "gauge";
		aGaugeAttr.cThemePath = myConfig.cGThemePath;
	}
	else if (myConfig.iDisplayType == CD_WIFI_GRAPH)
	{
		memset (&aGraphAttr, 0, sizeof (CairoGraphAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&aGraphAttr);
		pRenderAttr->cModelName = "graph";
		int w, h;
		CD_APPLET_GET_MY_ICON_EXTENT (&w, &h);
		pRenderAttr->iMemorySize = (w > 1 ? w : 32);  // fWidth peut etre <= 1 en mode desklet au chargement.  // fWidht peut etre <= 1 en mode desklet au chargement.
		aGraphAttr.iType = myConfig.iGraphType;
		aGraphAttr.fHighColor = myConfig.fHigholor;
		aGraphAttr.fLowColor = myConfig.fLowColor;
		memcpy (aGraphAttr.fBackGroundColor, myConfig.fBgColor, 4*sizeof (double));
	}
	pRenderAttr->iLatencyTime = myConfig.iCheckInterval * 1000 * myConfig.fSmoothFactor;
	//pRenderAttr->bWriteValues = TRUE;
	if (! bReload)
		CD_APPLET_ADD_DATA_RENDERER_ON_MY_ICON (pRenderAttr);
	else
		CD_APPLET_RELOAD_MY_DATA_RENDERER (pRenderAttr);
}

CD_APPLET_INIT_BEGIN
	if (myDesklet != NULL)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
		CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
	}
	
	// Initialisation du rendu.
	_set_data_renderer (myApplet, FALSE);
	
	// Initialisation de la tache periodique de mesure.
	myData.iPreviousQuality = -2;  // force le dessin.
	myData.pTask = cairo_dock_new_task (myConfig.iCheckInterval,
		(CairoDockGetDataAsyncFunc) cd_wifi_get_data,
		(CairoDockUpdateSyncFunc) cd_wifi_update_from_data,
		myApplet);
	if (cairo_dock_is_loading ())
		cairo_dock_launch_task_delayed (myData.pTask, 2000);
	else
		cairo_dock_launch_task (myData.pTask);
	
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	//\_______________ On recharge les donnees qui ont pu changer.
	int i; // reset surfaces utilisateurs.
	for (i = 0; i < WIFI_NB_QUALITY; i ++) {
		if (myData.pSurfaces[i] != NULL) {
			cairo_surface_destroy (myData.pSurfaces[i]);
			myData.pSurfaces[i] = NULL;
		}
	}
	
	//\_______________ On relance avec la nouvelle config ou on redessine.
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		if (myDesklet && CD_APPLET_MY_CONTAINER_TYPE_CHANGED)  // we are now in a desklet, set a renderer.
		{
			CD_APPLET_SET_DESKLET_RENDERER ("Simple");
			CD_APPLET_ALLOW_NO_CLICKABLE_DESKLET;
		}
		
		_set_data_renderer (myApplet, TRUE);
		
		myData.iQuality = -2;  // force le redessin.
		myData.iPercent = -2;
		myData.iSignalLevel = -2;
		
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		cairo_dock_relaunch_task_immediately (myData.pTask, myConfig.iCheckInterval);
	}
	else  // on redessine juste l'icone.
	{
		CD_APPLET_RELOAD_MY_DATA_RENDERER (NULL);
		if (myConfig.iDisplayType == CD_WIFI_GRAPH)
			CD_APPLET_SET_MY_DATA_RENDERER_HISTORY_TO_MAX;
		
		myData.iQuality = -2;  // force le redessin.
		if (! cairo_dock_task_is_running (myData.pTask))
		{
			if (myData.bWirelessExt)
			{
				cd_wifi_draw_icon ();
			}
			else
			{
				cd_wifi_draw_no_wireless_extension ();
			}
		}
	}
CD_APPLET_RELOAD_END
