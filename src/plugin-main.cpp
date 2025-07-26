/*
Plugin Name
Copyright (C) <Year> <Developer> <Email Address>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/
#include <QPointer>
#include <countdown-widget.hpp>

#include "plugin-support.h"

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("Ashmanix")
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-GB")

static QPointer<CountdownDockWidget> countdownWidget = nullptr;

bool obs_module_load(void)
{
	const auto main_window = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	obs_frontend_push_ui_translation(obs_module_get_string);
	countdownWidget = new CountdownDockWidget(main_window);

	obs_frontend_add_dock_by_id("ashmanixCountdownWidget", obs_module_text("CountdownTimer"), countdownWidget);
	obs_frontend_pop_ui_translation();

	obs_log(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_post_load(void)
{
	if (countdownWidget)
		countdownWidget->ConfigureWebSocketConnection();
}

void obs_module_unload()
{
	obs_log(LOG_INFO, "plugin unloaded");
}

const char *obs_module_name(void)
{
	return obs_module_text("CountdownTimer");
}

const char *obs_module_description(void)
{
	return obs_module_text("Description");
}
