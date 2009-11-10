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


#ifndef __APPLET_FIREWORK__
#define  __APPLET_FIREWORK__


#include <cairo-dock.h>
#include "applet-struct.h"
#include "applet-fire.h"


#define cd_icon_effect_load_firework_texture cd_icon_effect_load_fire_texture


void cd_icon_effect_init_firework (Icon *pIcon, CairoDock *pDock, double dt, CDIconEffectData *pData);


gboolean cd_icon_effect_update_fireworks (Icon *pIcon, CairoDock *pDock, CDIconEffectData *pData, gboolean bWillContinue);


void cd_icon_effect_render_fireworks (CDIconEffectData *pData);


void cd_icon_effect_free_fireworks (CDIconEffectData *pData);


#endif