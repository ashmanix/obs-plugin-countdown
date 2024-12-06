#include "obs-utils.hpp"

void LoadHotkey(int &id, const char *name,
				   const char *description,
				   std::function<void()> function,
				   std::string buttonLogMessage,
				   obs_data_t *savedData = nullptr)
{

	id = (int)obs_hotkey_register_frontend(
		name, description,
		(obs_hotkey_func)&HotkeyCallback,
		new RegisterHotkeyCallbackData{function, buttonLogMessage});

	if (savedData) {
		if ((int)id == -1)
			return;

		OBSDataArrayAutoRelease array =
			obs_data_get_array(savedData, name);

		obs_hotkey_load(id, array);
	}
}

void SaveHotkey(obs_data_t *sv_data, obs_hotkey_id id,
			       const char *name)
{
	// obs_log(LOG_INFO, "Hotkey ID: %i, Value: %s", (int)id, name);
	if ((int)id == -1)
		return;
	OBSDataArrayAutoRelease array = obs_hotkey_save(id);
	obs_data_set_array(sv_data, name, array);
};

void *HotkeyCallback(void *incoming_data, obs_hotkey_id,
				    obs_hotkey_t *, bool pressed)
{
	if (pressed) {
		RegisterHotkeyCallbackData *hotkey_callback_data =
			static_cast<RegisterHotkeyCallbackData *>(
				incoming_data);
		obs_log(LOG_INFO,
			hotkey_callback_data->hotkeyLogMessage.c_str(),
			" due to hotkey");
		hotkey_callback_data->function();
	}
	return incoming_data;
}
