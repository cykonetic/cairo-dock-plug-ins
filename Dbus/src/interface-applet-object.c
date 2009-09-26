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


#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "interface-applet-signals.h"
#include "interface-applet-methods.h"
#include "dbus-applet-spec.h"
#include "dbus-sub-applet-spec.h"
#include "interface-applet-object.h"

G_DEFINE_TYPE(dbusApplet, cd_dbus_applet, G_TYPE_OBJECT);

G_DEFINE_TYPE(dbusSubApplet, cd_dbus_sub_applet, G_TYPE_OBJECT);


static void cd_dbus_applet_class_init(dbusAppletClass *klass)
{
	cd_message("");
	
	cd_dbus_applet_init_signals_once (klass);
	
	dbus_g_object_type_install_info (cd_dbus_applet_get_type(), &dbus_glib_cd_dbus_applet_object_info);
}
static void cd_dbus_applet_init (dbusApplet *pDbusApplet)
{
	cd_message("");
	
	pDbusApplet->connection = cairo_dock_get_session_connection ();
	pDbusApplet->proxy = cairo_dock_get_main_proxy ();
	pDbusApplet->pSubApplet = g_object_new (cd_dbus_sub_applet_get_type(), NULL);
	pDbusApplet->pSubApplet->pApplet = pDbusApplet;
}

static void cd_dbus_sub_applet_class_init(dbusSubAppletClass *klass)
{
	cd_message("");
	
	cd_dbus_sub_applet_init_signals_once (klass);
	
	dbus_g_object_type_install_info (cd_dbus_sub_applet_get_type(), &dbus_glib_cd_dbus_sub_applet_object_info);
}
static void cd_dbus_sub_applet_init (dbusSubApplet *pDbusSubApplet)
{
	cd_message("");
}



dbusApplet * cd_dbus_get_dbus_applet_from_instance (CairoDockModuleInstance *pModuleInstance)
{
	const gchar *cModuleName = pModuleInstance->pModule->pVisitCard->cModuleName;
	dbusApplet *pDbusApplet = NULL;
	GList *a;
	for (a = myData.pAppletList; a != NULL; a = a->next)
	{
		pDbusApplet = a->data;
		if (strcmp (cModuleName, pDbusApplet->cModuleName) == 0)
			break ;
	}
	return (a ? pDbusApplet : NULL);
}

static dbusApplet * _remove_dbus_applet_from_list (CairoDockModuleInstance *pModuleInstance)
{
	const gchar *cModuleName = pModuleInstance->pModule->pVisitCard->cModuleName;
	dbusApplet *pDbusApplet;
	GList *a;
	for (a = myData.pAppletList; a != NULL; a = a->next)
	{
		pDbusApplet = a->data;
		if (strcmp (cModuleName, pDbusApplet->cModuleName) == 0)
			break ;
	}
	myData.pAppletList = g_list_delete_link (myData.pAppletList, a);
	return (a ? pDbusApplet : NULL);
}

#define _applet_list_is_empty() (myData.pAppletList == NULL)

dbusApplet *cd_dbus_create_remote_applet_object (CairoDockModuleInstance *pModuleInstance)
{
	g_return_val_if_fail (pModuleInstance != NULL && myData.pMainObject != NULL, NULL);  // l'interface principale a deja enregistre notre domaine, etc.
	const gchar *cModuleName = pModuleInstance->pModule->pVisitCard->cModuleName;
	g_print ("%s (%s)\n", __func__, cModuleName);
	
	// on assure l'unicite.
	dbusApplet *pDbusApplet = cd_dbus_get_dbus_applet_from_instance (pModuleInstance);
	if (pDbusApplet != NULL)
	{
		cd_warning ("this applet (%s) already has a remote object on the bus", cModuleName);
		return pDbusApplet;
	}
	
	// on cree l'objet DBus correspondant a l'applet.
	pDbusApplet = g_object_new (cd_dbus_applet_get_type(), NULL);  // appelle cd_dbus_applet_class_init() et cd_dbus_applet_init().
	pDbusApplet->cModuleName = g_strdup (cModuleName);
	pDbusApplet->pModuleInstance = pModuleInstance;
	
	// on l'enregistre sous un chemin propre a l'applet.
	gchar *cPath = g_strconcat ("/org/cairodock/CairoDock/", cModuleName, NULL);
	dbus_g_connection_register_g_object (pDbusApplet->connection, cPath, G_OBJECT(pDbusApplet));
	g_free (cPath);
	cPath = g_strconcat ("/org/cairodock/CairoDock/", cModuleName, "/sub_icons", NULL);
	dbus_g_connection_register_g_object (pDbusApplet->connection, cPath, G_OBJECT(pDbusApplet->pSubApplet));
	g_free (cPath);
	
	// on s'abonne aux notifications qu'on voudra propager sur le bus.
	if (pDbusApplet->proxy != NULL && _applet_list_is_empty ())  // 1ere applet Dbus.
	{
		cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_click_icon,
			CAIRO_DOCK_RUN_AFTER,
			NULL);
		cairo_dock_register_notification (CAIRO_DOCK_MIDDLE_CLICK_ICON,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_middle_click_icon,
			CAIRO_DOCK_RUN_AFTER,
			NULL);
		cairo_dock_register_notification (CAIRO_DOCK_SCROLL_ICON,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_scroll_icon,
			CAIRO_DOCK_RUN_AFTER,
			NULL);
		cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_build_menu,
			CAIRO_DOCK_RUN_FIRST,
			NULL);
		cairo_dock_register_notification (CAIRO_DOCK_DROP_DATA,
			(CairoDockNotificationFunc) cd_dbus_applet_emit_on_drop_data,
			CAIRO_DOCK_RUN_AFTER,
			NULL);
	}
	
	myData.pAppletList = g_list_prepend (myData.pAppletList, pDbusApplet);
	return pDbusApplet;
}

void cd_dbus_delete_remote_applet_object (CairoDockModuleInstance *pModuleInstance)
{
	dbusApplet *pDbusApplet = _remove_dbus_applet_from_list (pModuleInstance);
	
	if (_applet_list_is_empty ())
	{
		cd_dbus_unregister_notifications ();
	}
	
	if (pDbusApplet != NULL)
	{
		if (pDbusApplet->pSubApplet != NULL)
		{
			g_object_unref (pDbusApplet->pSubApplet);
			pDbusApplet->pSubApplet = NULL;
		}
		g_object_unref (pDbusApplet);
	}
}

void cd_dbus_unregister_notifications (void)
{
	cairo_dock_remove_notification_func (CAIRO_DOCK_CLICK_ICON,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_click_icon,
		NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_MIDDLE_CLICK_ICON,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_middle_click_icon,
		NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_SCROLL_ICON,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_scroll_icon,
		NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_BUILD_MENU,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_build_menu,
		NULL);
	cairo_dock_remove_notification_func (CAIRO_DOCK_DROP_DATA,
		(CairoDockNotificationFunc) cd_dbus_applet_emit_on_drop_data,
		NULL);
}


gboolean cd_dbus_applet_is_used (const gchar *cModuleName)
{
	//g_print ("%s (%s in %s)\n", __func__, cModuleName, myData.cActiveModules);
	if (myData.cActiveModules == NULL)
		return FALSE;
	gchar *str = g_strstr_len (myData.cActiveModules, -1, cModuleName);
	return (str && (str[strlen(cModuleName)] == ';' || str[strlen(cModuleName)] == '\0'));
}

void cd_dbus_launch_distant_applet_in_dir (const gchar *cModuleName, const gchar *cDirPath)
{
	gchar *cCommand = g_strdup_printf ("cd \"%s/%s/%s\" && ./\"%s\"", cDirPath, "third-party", cModuleName, cModuleName);
	g_print ("on lance une applet distante : '%s'\n", cCommand);
	cairo_dock_launch_command (cCommand);
	g_free (cCommand);
}

void cd_dbus_launch_distant_applet (const gchar *cModuleName)
{
	gchar *cFilePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR"/third-party", cModuleName);
	if (g_file_test (cFilePath, G_FILE_TEST_EXISTS))
	{
		cd_dbus_launch_distant_applet_in_dir (cModuleName, MY_APPLET_SHARE_DATA_DIR);
	}
	else
	{
		g_free (cFilePath);
		cFilePath = g_strdup_printf ("%s/%s/%s", g_cCairoDockDataDir, "third-party", cModuleName);
		if (g_file_test (cFilePath, G_FILE_TEST_EXISTS))
		{
			cd_dbus_launch_distant_applet_in_dir (cModuleName, g_cCairoDockDataDir);
		}
	}
	g_free (cFilePath);
}