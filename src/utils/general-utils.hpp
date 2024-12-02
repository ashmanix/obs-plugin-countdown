#ifndef GENERALUTILS_H
#define GENERALUTILS_H


#include <obs.h>
#include "plugin-support.h"

// Hotkeys function
// static void SaveHotkey(obs_data_t *sv_data, obs_hotkey_id id, const char *name) {
// 	obs_log(LOG_INFO, "Hotkey ID: %i, Value: %s", (int)id, name);
// 	if ((int)id == -1)
// 		return;
// 	OBSDataArrayAutoRelease array = obs_hotkey_save(id);
// 	obs_data_set_array(sv_data, name, array);
// };

#endif // GENERALUTILS_H
