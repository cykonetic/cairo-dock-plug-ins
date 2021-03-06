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
#include <glib/gi18n.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-xgamma.h"
#include "applet-notifications.h"


CD_APPLET_ON_CLICK_BEGIN
	if (myDock)
	{
		xgamma_build_dialog_simple ();
	}
CD_APPLET_ON_CLICK_END


static void _cd_xgamma_set_up_gamma_correction (void)
{
	double fGamma = xgamma_get_gamma (&myData.Xgamma);
	if (fGamma >= 0)
	{
		//\___________________ On construit notre widget si c'est la 1ere fois.
		if (myData.pWidget == NULL)
		{
			xgamma_build_and_show_widget ();
		}
		else
		{
			//\___________________ On lui met les valeurs a jour, sans appeler les callbacks.
			g_signal_handler_block (myData.pGlobalScale, myData.iGloalScaleSignalID);
			g_signal_handler_block (myData.pRedScale, myData.iRedScaleSignalID);
			g_signal_handler_block (myData.pGreenScale, myData.iGreenScaleSignalID);
			g_signal_handler_block (myData.pBlueScale, myData.iBlueScaleSignalID);
			
			gtk_range_set_value (GTK_RANGE (myData.pGlobalScale), fGamma);
			gtk_range_set_value (GTK_RANGE (myData.pRedScale), myData.Xgamma.red);
			gtk_range_set_value (GTK_RANGE (myData.pGreenScale), myData.Xgamma.green);
			gtk_range_set_value (GTK_RANGE (myData.pBlueScale), myData.Xgamma.blue);
			myData.XoldGamma = myData.Xgamma;
			
			g_signal_handler_unblock (myData.pGlobalScale, myData.iGloalScaleSignalID);
			g_signal_handler_unblock (myData.pRedScale, myData.iRedScaleSignalID);
			g_signal_handler_unblock (myData.pGreenScale, myData.iGreenScaleSignalID);
			g_signal_handler_unblock (myData.pBlueScale, myData.iBlueScaleSignalID);
			
			if (myData.pDialog != NULL)
			{
				gldi_dialog_unhide (myData.pDialog);
			}
		}
	}
}

static void _cd_xgamma_remember_current_gamma (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	double fGamma = xgamma_get_gamma (&myData.Xgamma);
	g_return_if_fail (fGamma > 0);
	
	cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,
		G_TYPE_DOUBLE, "Configuration", "initial gamma", fGamma,
		G_TYPE_INVALID);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	if (myDock)
	{
		gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Set up gamma correction"), D_("middle-click"));
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GLDI_ICON_NAME_DIALOG_INFO, _cd_xgamma_set_up_gamma_correction, CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Apply current luminosity on startup"), GLDI_ICON_NAME_DIALOG_WARNING, _cd_xgamma_remember_current_gamma, CD_APPLET_MY_MENU);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myDock)
	{
		_cd_xgamma_set_up_gamma_correction ();
	}
CD_APPLET_ON_MIDDLE_CLICK_END


static gboolean _on_scroll (gpointer data)
{
	cd_debug ("%s (%d)", __func__, myData.iScrollCount);
	if (myData.iScrollCount != 0)
		xgamma_add_gamma (&myData.Xgamma, myData.iScrollCount);
	myData.iSidScrollAction = 0;
	myData.iScrollCount = 0;
	return FALSE;
}
CD_APPLET_ON_SCROLL_BEGIN
	myData.iScrollCount += (CD_APPLET_SCROLL_UP ? 1 : -1);
	if (myData.iSidScrollAction == 0)
		myData.iSidScrollAction = g_timeout_add (200, (GSourceFunc) _on_scroll, NULL);
CD_APPLET_ON_SCROLL_END


void cd_xgamma_on_keybinding_pull (const gchar *keystring, gpointer user_data)
{
	xgamma_add_gamma (&myData.Xgamma, +1);
}

void cd_xgamma_on_keybinding_pull2 (const gchar *keystring, gpointer user_data)
{
	xgamma_add_gamma (&myData.Xgamma, -1);
}
