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
#include <cairo-dock.h>

#include "applet-utils.h"


void env_backend_logout (void)
{
	// since Gnome 3, gnome-session-save has been replaced by gnome-session-quit
	gchar *cResult = cairo_dock_launch_command_sync ("which gnome-session-quit");
	if (cResult != NULL && *cResult == '/')
		cairo_dock_launch_command ("gnome-session-quit --logout");
	else
		cairo_dock_launch_command ("gnome-session-save --kill --gui");
	g_free (cResult);
}

void env_backend_shutdown (void)
{
	// since Gnome 3, gnome-session-save has been replaced by gnome-session-quit
	gchar *cResult = cairo_dock_launch_command_sync ("which gnome-session-quit");
	if (cResult != NULL && *cResult == '/')
		cairo_dock_launch_command ("gnome-session-quit --power-off");
	else
		cairo_dock_launch_command ("gnome-session-save --shutdown-dialog");
	g_free (cResult);
}

void env_backend_lock_screen (void)
{
	cairo_dock_launch_command (MY_APPLET_SHARE_DATA_DIR"/../shared-files/scripts/lock-screen.sh");
}

void env_backend_setup_time (void)
{
	cairo_dock_launch_command ("time-admin");  // it uses PolicyKit => no gksudo.
}

void env_backend_show_system_monitor (void)
{
	cairo_dock_launch_command ("gnome-system-monitor");
}
