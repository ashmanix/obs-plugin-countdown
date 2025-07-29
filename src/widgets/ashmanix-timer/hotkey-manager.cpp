#include "hotkey-manager.hpp"
#include "../../ui/ui_AshmanixTimer.h"
#include "countdown-widget.hpp"
#include "../../utils/obs-utils.hpp"

HotkeyManager::HotkeyManager(QWidget *parentWidget, Ui_AshmanixTimer *ui, TimerWidgetStruct *data)
	: parent(parentWidget),
	  ui(ui),
	  data(data)
{
}

void HotkeyManager::RegisterAllHotkeys(obs_data_t *savedData)
{
	LoadHotkey(
		data->hotkeys.startCountdownHotkeyId, TIMERSTARTHOTKEYNAME,
		GetFullHotkeyName(obs_module_text("StartCountdownHotkeyDescription"), " - ").c_str(),
		[this]() { ui->playButton->click(); }, GetFullHotkeyName("Play Hotkey Pressed", " "), savedData);

	LoadHotkey(
		data->hotkeys.pauseCountdownHotkeyId, TIMERPAUSEHOTKEYNAME,
		GetFullHotkeyName(obs_module_text("PauseCountdownHotkeyDescription"), " - ").c_str(),
		[this]() { ui->pauseButton->animateClick(); }, GetFullHotkeyName("Pause Hotkey Pressed", " "),
		savedData);

	LoadHotkey(
		data->hotkeys.setCountdownHotkeyId, TIMERSETHOTKEYNAME,
		GetFullHotkeyName(obs_module_text("SetCountdownHotkeyDescription"), " - ").c_str(),
		[this]() { ui->resetButton->animateClick(); }, GetFullHotkeyName("Set Hotkey Pressed", " "), savedData);

	LoadHotkey(
		data->hotkeys.startCountdownToTimeHotkeyId, TIMERTOTIMESTARTHOTKEYNAME,
		GetFullHotkeyName(obs_module_text("StartCountdownToTimeHotkeyDescription"), " - ").c_str(),
		[this]() { ui->toTimePlayButton->animateClick(); },
		GetFullHotkeyName("To Time Start Hotkey Pressed", " "), savedData);

	LoadHotkey(
		data->hotkeys.stopCountdownToTimeHotkeyId, TIMERTOTIMESTOPHOTKEYNAME,
		GetFullHotkeyName(obs_module_text("StopCountdownToTimeHotkeyDescription"), " - ").c_str(),
		[this]() { ui->toTimeStopButton->animateClick(); },
		GetFullHotkeyName("To Time Stop Hotkey Pressed", " "), savedData);
}

void HotkeyManager::UnregisterAllHotkeys()
{
	if (data->hotkeys.startCountdownHotkeyId)
		obs_hotkey_unregister(data->hotkeys.startCountdownHotkeyId);
	if (data->hotkeys.pauseCountdownHotkeyId)
		obs_hotkey_unregister(data->hotkeys.pauseCountdownHotkeyId);
	if (data->hotkeys.setCountdownHotkeyId)
		obs_hotkey_unregister(data->hotkeys.setCountdownHotkeyId);

	if (data->hotkeys.startCountdownToTimeHotkeyId)
		obs_hotkey_unregister(data->hotkeys.startCountdownToTimeHotkeyId);
	if (data->hotkeys.stopCountdownToTimeHotkeyId)
		obs_hotkey_unregister(data->hotkeys.stopCountdownToTimeHotkeyId);
}

std::string HotkeyManager::GetFullHotkeyName(const std::string &name, const char *joinText)
{
	return name + joinText + data->timerId.toStdString();
}
