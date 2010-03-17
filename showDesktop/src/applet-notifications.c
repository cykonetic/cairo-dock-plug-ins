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
#include<X11/Xlib.h>
#ifdef HAVE_XRANDR
#include<X11/extensions/Xrandr.h>
#endif

#include "applet-struct.h"
#include "applet-notifications.h"


static gboolean _cd_allow_minimize (CairoDesklet *pDesklet, gpointer data)
{
	pDesklet->bAllowMinimize = TRUE;
	return FALSE;
}

static void _cd_show_hide_desktop (gboolean bShowDesklets)
{
	if (! myData.bDesktopVisible && ! bShowDesklets)  // on autorise chaque desklet a etre minimise. l'autorisation est annulee lors de leur cachage, donc on n'a pas besoin de faire le contraire apres avoir montre le bureau.
	{
		cairo_dock_foreach_desklet ((CairoDockForeachDeskletFunc) _cd_allow_minimize, NULL);
	}
	
	cairo_dock_show_hide_desktop (! myData.bDesktopVisible);
}

static void _cd_show_hide_desklet (void)
{
	if (!myData.bDeskletsVisible)
	{
		//myData.xLastActiveWindow = cairo_dock_get_current_active_window ();
		cairo_dock_set_all_desklets_visible (TRUE);  // TRUE <=> les desklets de la couche widget aussi.
	}
	else
	{
		cairo_dock_set_desklets_visibility_to_default ();
		//cairo_dock_show_xwindow (myData.xLastActiveWindow);
	}
	myData.bDeskletsVisible = ! myData.bDeskletsVisible;
	
	if (myConfig.cVisibleImage)
	{
		if (myData.bDesktopVisible || myData.bDeskletsVisible)
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cVisibleImage);
		else
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cHiddenImage);
	}
}

static void _compiz_dbus_action (const gchar *cCommand)  // taken from the Compiz-Icon applet, thanks ChangFu !
{
	if (! cairo_dock_dbus_detect_application ("org.freedesktop.compiz"))
	{
		cd_warning  ("Dbus plug-in must be activated in Compiz !");
		cairo_dock_show_temporary_dialog_with_icon (D_("You need to run Compiz and activate its 'DBus' plug-in."), myIcon, myContainer, 6000, "same icon");
	}
	
	GError *erreur = NULL;
	gchar *cDbusCommand = g_strdup_printf ("dbus-send --type=method_call --dest=org.freedesktop.compiz /org/freedesktop/compiz/%s org.freedesktop.compiz.activate string:'root' int32:%d", cCommand, cairo_dock_get_root_id ());
	g_spawn_command_line_async (cDbusCommand, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("ShowDesktop : when trying to send '%s' : %s", cDbusCommand, erreur->message);
		g_error_free (erreur);
	}
	g_free (cDbusCommand);
}

static void _cd_show_widget_layer (void)
{
	_compiz_dbus_action ("widget/allscreens/toggle_button");  // toggle avant la 0.7
}

static void _cd_expose (void)
{
	_compiz_dbus_action ("expo/allscreens/expo_button");  // expo avant la 0.7
}

static void _cd_action (CDActionOnClick iAction)
{
	switch (iAction)
	{
		case CD_SHOW_DESKTOP :
			_cd_show_hide_desktop (FALSE);
		break ;
		case CD_SHOW_DESKLETS :
			_cd_show_hide_desklet ();
		break ;
		case CD_SHOW_DESKTOP_AND_DESKLETS :
			_cd_show_hide_desktop (TRUE);  // TRUE <=> show the desklets
		break ;
		case CD_SHOW_WIDGET_LAYER :
			_cd_show_widget_layer ();
		break ;
		case CD_EXPOSE :
			_cd_expose ();
		break ;
		default:
		break;
	}
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	_cd_action (myConfig.iActionOnLeftClick);
CD_APPLET_ON_CLICK_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
#ifdef HAVE_XRANDR
static void _on_select_resolution (GtkMenuItem *menu_item, gpointer data)
{
	CD_APPLET_ENTER;
	int 				    iNumRes = GPOINTER_TO_INT (data);
	Display                 *dpy;
	Window                  root;
	XRRScreenConfiguration  *conf;
	short                   *rates;
	int                     num_rates;
	dpy = gdk_x11_get_default_xdisplay ();
	root = RootWindow(dpy, 0);
	
	conf = XRRGetScreenInfo(dpy, root);
	CD_APPLET_LEAVE_IF_FAIL (conf != NULL);
	//g_return_if_fail (conf != NULL);
	
	rates = XRRRates(dpy, 0, iNumRes, &num_rates);
	CD_APPLET_LEAVE_IF_FAIL (num_rates > 0);
	//g_return_if_fail (num_rates > 0);
	g_print ("available rates : from %d to %d Hz\n", rates[0], rates[num_rates-1]);
	
	XRRSetScreenConfigAndRate(dpy, conf, root, iNumRes, RR_Rotate_0, rates[num_rates-1], CurrentTime);
	XRRFreeScreenConfigInfo (conf);
	
	// restore original conf :  XRRSetScreenConfigAndRate(dpy, conf, root, original_size_id, original_rotation, original_rate, CurrentTime);
	CD_APPLET_LEAVE();
}
#endif
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
	
	#ifdef HAVE_XRANDR
	pSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (D_("Change screen resolution"), CD_APPLET_MY_MENU, GTK_STOCK_FULLSCREEN);
	
	Display                 *dpy;
	Window                  root;
	int                     num_sizes;
	XRRScreenSize           *xrrs;
	XRRScreenConfiguration  *conf;
	short                   original_rate;
	Rotation                original_rotation;
	SizeID                  original_size_id;
	
	dpy = gdk_x11_get_default_xdisplay ();
	root = RootWindow(dpy, 0);
	
	conf = XRRGetScreenInfo(dpy, root);  // config  courante.
	if (conf != NULL)
	{
		original_rate = XRRConfigCurrentRate(conf);
		original_size_id = XRRConfigCurrentConfiguration(conf, &original_rotation);
		
		// resolutions possibles.
		int num_sizes = 0;
		XRRScreenSize *xrrs = XRRSizes(dpy, 0, &num_sizes);
		// add to the menu
		GString *pResString = g_string_new ("");
		int i;
		for (i = 0; i < num_sizes; i ++)
		{
			g_string_printf (pResString, "%s%dx%d", (i == original_size_id ? "=>" : ""), xrrs[i].width, xrrs[i].height);
			CD_APPLET_ADD_IN_MENU_WITH_DATA (pResString->str, _on_select_resolution, pSubMenu, GINT_TO_POINTER (i));
			/*short   *rates;
			int     num_rates;
			rates = XRRRates(dpy, 0, i, &num_rates);
			for(int j = 0; j < num_rates; j ++) {
				possible_frequencies[i][j] = rates[j];
				printf("%4i ", rates[j]); }*/
		}
		g_string_free (pResString, TRUE);
		XRRFreeScreenConfigInfo (conf);
	}
	#endif
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	_cd_action (myConfig.iActionOnMiddleClick);
CD_APPLET_ON_MIDDLE_CLICK_END


void on_keybinding_pull (const char *keystring, gpointer user_data)
{
	CD_APPLET_ENTER;
	_cd_action (myConfig.iActionOnMiddleClick);
	CD_APPLET_LEAVE();
}


gboolean on_show_desktop (CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	myData.bDesktopVisible = cairo_dock_desktop_is_visible ();
	g_print ("bDesktopVisible <- %d\n", myData.bDesktopVisible);
	
	if (myConfig.cVisibleImage)
	{
		if (myData.bDesktopVisible || myData.bDeskletsVisible)
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cVisibleImage);
		else
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cHiddenImage);
	}
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}
