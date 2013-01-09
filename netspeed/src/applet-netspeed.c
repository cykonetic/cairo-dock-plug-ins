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
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-netspeed.h"
#include "cairo-dock.h"

#define NETSPEED_DATA_PIPE "/proc/net/dev"


// Prend un debit en octet par seconde et le transforme en une chaine de la forme : xxx yB/s
static void cd_netspeed_formatRate (unsigned long long rate, gchar* debit, int iBufferSize, gboolean bLong)
{
	int smallRate;
	if (rate <= 0)
	{
		if (bLong)
			snprintf (debit, iBufferSize, "0 %s/s", D_("B"));
		else
			snprintf (debit, iBufferSize, "0");
	}
	else if (rate < 1024)
	{
		smallRate = rate;
		if (bLong)
			snprintf (debit, iBufferSize, "%i %s/s", smallRate, D_("B"));
		else
			snprintf (debit, iBufferSize, "%iB", smallRate);
	}
	else if (rate < (1<<20))
	{
		smallRate = rate >> 10;
		if (bLong)
			snprintf (debit, iBufferSize, "%i %s/s", smallRate, D_("KB"));
		else
			snprintf (debit, iBufferSize, "%iK", smallRate);
	}
	else if (rate < (1<<30))
	{
		smallRate = rate >> 20;
		if (bLong)
			snprintf (debit, iBufferSize, "%i %s/s", smallRate, D_("MB"));
		else
			snprintf (debit, iBufferSize, "%iM", smallRate);
	}
	else if (rate < ((unsigned long long)1<<40))
	{
		smallRate = rate >> 30;
		if (bLong)
			snprintf (debit, iBufferSize, "%i %s/s", smallRate, D_("GB"));
		else
			snprintf (debit, iBufferSize, "%iG", smallRate);
	}
	else  // c'est vraiment pour dire qu'on est exhaustif :-)
	{
		smallRate = rate >> 40;
		if (bLong)
			snprintf (debit, iBufferSize, "%i %s/s", smallRate, D_("TB"));
		else
			snprintf (debit, iBufferSize, "%iT", smallRate);
	}
}


void cd_netspeed_format_value (CairoDataRenderer *pRenderer, int iNumValue, gchar *cFormatBuffer, int iBufferLength, CairoDockModuleInstance *myApplet)
{
	static gchar s_upRateFormatted[11];
	cd_netspeed_formatRate (iNumValue == 0 ? myData.iDownloadSpeed : myData.iUploadSpeed, s_upRateFormatted, 11, FALSE);
	snprintf (cFormatBuffer, iBufferLength,
		"%s%s",
		cairo_data_renderer_can_write_values (pRenderer) ? (iNumValue == 0 ?"↓" : "↑") : "",
		s_upRateFormatted);
}

static gboolean _cd_is_a_good_interface (gchar *cPointer, CairoDockModuleInstance *myApplet)
{
	if (myConfig.cInterface == NULL) // we monitor all interfaces except 'lo'
	{
		gchar *cColon = cPointer; // yeah, it's an English word and not a French one :D
		int iStringLen = 0;
		while (*cColon != ':') // || *cColon != '\0')
		{
			cColon ++;
			iStringLen ++;
		}
		if (strncmp (cPointer, "lo", iStringLen) != 0) // skip 'lo'
			return iStringLen;
	}
	else if (strncmp (cPointer, myConfig.cInterface, myConfig.iStringLen) == 0
		&& *(cPointer + myConfig.iStringLen) == ':')
		return myConfig.iStringLen; // the right interface followed by ':'

	return 0;
}

void cd_netspeed_get_data (CairoDockModuleInstance *myApplet)
{
	g_timer_stop (myData.pClock);
	double fTimeElapsed = g_timer_elapsed (myData.pClock, NULL);
	g_timer_start (myData.pClock);
	g_return_if_fail (fTimeElapsed > 0.1 || !myData.bInitialized);
	
	myData.bAcquisitionOK = FALSE;
	gchar *cContent = NULL;
	gsize length=0;
	GError *erreur = NULL;
	g_file_get_contents (NETSPEED_DATA_PIPE, &cContent, &length, &erreur);
	if (erreur != NULL)
	{
		cd_warning("NetSpeed : %s", erreur->message);
		g_error_free(erreur);
		erreur = NULL;
	}
	else if (cContent && *cContent != '\0')
	{
		int iNumLine = 1, iStringLen;
		gchar *tmp = cContent;
		long long int iReceivedBytes = 0, iTransmittedBytes = 0;
		do
		{
			if (iNumLine > 2)  // first 2 lines are the names of the fields (next lines are the various interfaces, in any order, the loopback is not always the first one).
			{
				while (*tmp == ' ')  // drop spaces
					tmp ++;
				
				if ((iStringLen = _cd_is_a_good_interface (tmp, myApplet)) != 0)
				{
					myData.bAcquisitionOK = TRUE;
					tmp += iStringLen+1;  // drop ':'
					while (*tmp == ' ')  // drop spaces
						tmp ++;
					iReceivedBytes += atoll (tmp);
					
					int i = 0;
					for (i = 0; i < 8; i ++)  // drop 8 next values
					{
						while (*tmp != ' ')  // drop numbers
							tmp ++;
						while (*tmp == ' ')  // drop spaces
							tmp ++;
					}
					iTransmittedBytes += atoll (tmp);

					if (myConfig.cInterface != NULL) // only one interface
						break ;
				}
			}
			tmp = strchr (tmp, '\n');
			if (tmp == NULL || *(++tmp) == '\0') // EOF
				break;
			iNumLine ++;
		}
		while (1);
		if (myData.bInitialized)  // first iteration, we can compute the speed
		{
			myData.iDownloadSpeed = (iReceivedBytes - myData.iReceivedBytes) / fTimeElapsed;
			myData.iUploadSpeed = (iTransmittedBytes - myData.iTransmittedBytes) / fTimeElapsed;
		}
		myData.iReceivedBytes = iReceivedBytes;
		myData.iTransmittedBytes = iTransmittedBytes;

		if (! myData.bInitialized)
			myData.bInitialized = TRUE;
	}
	g_free (cContent);
}

gboolean cd_netspeed_update_from_data (CairoDockModuleInstance *myApplet)
{
	static double s_fValues[CD_NETSPEED_NB_MAX_VALUES];
	static gchar s_upRateFormatted[11];
	static gchar s_downRateFormatted[11];
	CD_APPLET_ENTER;
	if ( ! myData.bAcquisitionOK)
	{
		if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
		{
			if (myConfig.defaultTitle) // has another default name
				CD_APPLET_SET_NAME_FOR_MY_ICON (myConfig.defaultTitle);
			else
				CD_APPLET_SET_NAME_FOR_MY_ICON (myApplet->pModule->pVisitCard->cTitle);
		}
		else if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("N/A");
		
		memset (s_fValues, 0, sizeof (s_fValues));
		CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (s_fValues);
		
		cairo_dock_downgrade_task_frequency (myData.pPeriodicTask);
	}
	else
	{
		cairo_dock_set_normal_task_frequency (myData.pPeriodicTask);
		
		if (! myData.bInitialized)
		{
			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_ICON)
				CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (myDock ? "..." : D_("Loading"));
			memset (s_fValues, 0, sizeof (s_fValues));
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (s_fValues);
		}
		else
		{
			if (myConfig.iInfoDisplay == CAIRO_DOCK_INFO_ON_LABEL)
			{
				cd_netspeed_formatRate (myData.iUploadSpeed, s_upRateFormatted, 11, myDesklet != NULL);
				cd_netspeed_formatRate (myData.iDownloadSpeed, s_downRateFormatted, 11, myDesklet != NULL);
				CD_APPLET_SET_NAME_FOR_MY_ICON_PRINTF ("↓%s - ↑%s", s_downRateFormatted, s_upRateFormatted);
			}
			
			if(myData.iUploadSpeed > myData.iMaxUpRate) {
				myData.iMaxUpRate = myData.iUploadSpeed;
			}
			if(myData.iDownloadSpeed > myData.iMaxDownRate) {
				myData.iMaxDownRate = myData.iDownloadSpeed;
			}
			
			double fUpValue, fDownValue;
			if (myData.iMaxUpRate != 0)
				fUpValue = (double) myData.iUploadSpeed / myData.iMaxUpRate;
			else
				fUpValue = 0.;
			if (myData.iMaxDownRate != 0)
				fDownValue = (double) myData.iDownloadSpeed / myData.iMaxDownRate;
			else
				fDownValue = 0.;
			
			s_fValues[0] = fDownValue;
			s_fValues[1] = fUpValue;
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (s_fValues);
		}
	}
	CD_APPLET_LEAVE (TRUE);
}
