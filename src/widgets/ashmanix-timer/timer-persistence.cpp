#include "timer-persistence.hpp"
#include "../../utils/obs-utils.hpp"
#include "./hotkey-manager.hpp"

void TimerPersistence::SaveTimerWidgetDataToOBSSaveData(TimerWidgetStruct *timerData, obs_data_t *dataObject)
{
	obs_data_set_string(dataObject, "timerId", timerData->timerId.toStdString().c_str());
	obs_data_set_bool(dataObject, "startOnStreamStart", timerData->startOnStreamStart);
	obs_data_set_bool(dataObject, "resetTimerOnStreamStart", timerData->resetTimerOnStreamStart);
	obs_data_set_bool(dataObject, "shouldCountUp", timerData->shouldCountUp);
	obs_data_set_bool(dataObject, "showLeadingZero", timerData->display.showLeadingZero);

	obs_data_set_string(dataObject, "selectedSource", timerData->source.selectedSource.toStdString().c_str());
	obs_data_set_string(dataObject, "selectedScene", timerData->source.selectedScene.toStdString().c_str());
	obs_data_set_string(dataObject, "endMessage", timerData->display.endMessage.toStdString().c_str());
	obs_data_set_string(dataObject, "dateTime", timerData->dateTime.toString().toStdString().c_str());

	obs_data_set_int(dataObject, "periodDays", timerData->periodDuration.days);
	obs_data_set_int(dataObject, "periodHours", timerData->periodDuration.hours);
	obs_data_set_int(dataObject, "periodMinutes", timerData->periodDuration.minutes);
	obs_data_set_int(dataObject, "periodSeconds", timerData->periodDuration.seconds);

	obs_data_set_bool(dataObject, "showDays", timerData->display.showDays);
	obs_data_set_bool(dataObject, "showHours", timerData->display.showHours);
	obs_data_set_bool(dataObject, "showMinutes", timerData->display.showMinutes);
	obs_data_set_bool(dataObject, "showSeconds", timerData->display.showSeconds);
	obs_data_set_bool(dataObject, "showEndMessage", timerData->display.showEndMessage);
	obs_data_set_bool(dataObject, "showEndScene", timerData->display.showEndScene);

	obs_data_set_bool(dataObject, "useFormattedOutput", timerData->display.useFormattedOutput);
	obs_data_set_string(dataObject, "outputStringFormat",
			    timerData->display.outputStringFormat.toStdString().c_str());

	obs_data_set_bool(dataObject, "smoothenPeriodTimer", timerData->smoothenPeriodTimer);

	obs_data_set_int(dataObject, "selectedCountdownType", static_cast<int>(timerData->selectedCountdownType));

	obs_data_set_int(dataObject, "timeLeftInMillis", timerData->timeLeftInMillis);

	// ------------------------- Hotkeys -------------------------
	SaveHotkey(dataObject, timerData->hotkeys.startCountdownHotkeyId, HotkeyManager::TIMERSTARTHOTKEYNAME);

	SaveHotkey(dataObject, timerData->hotkeys.pauseCountdownHotkeyId, HotkeyManager::TIMERPAUSEHOTKEYNAME);

	SaveHotkey(dataObject, timerData->hotkeys.setCountdownHotkeyId, HotkeyManager::TIMERSETHOTKEYNAME);

	SaveHotkey(dataObject, timerData->hotkeys.startCountdownToTimeHotkeyId,
		   HotkeyManager::TIMERTOTIMESTARTHOTKEYNAME);

	SaveHotkey(dataObject, timerData->hotkeys.stopCountdownToTimeHotkeyId,
		   HotkeyManager::TIMERTOTIMESTOPHOTKEYNAME);
}

void TimerPersistence::LoadTimerWidgetDataFromOBSSaveData(TimerWidgetStruct *timerData, obs_data_t *dataObject)
{

	timerData->timerId = (char *)obs_data_get_string(dataObject, "timerId");
	timerData->startOnStreamStart = (bool)obs_data_get_bool(dataObject, "startOnStreamStart");
	timerData->resetTimerOnStreamStart = (bool)obs_data_get_bool(dataObject, "resetTimerOnStreamStart");
	timerData->shouldCountUp = (bool)obs_data_get_bool(dataObject, "shouldCountUp");
	timerData->display.showLeadingZero = (bool)obs_data_get_bool(dataObject, "showLeadingZero");
	timerData->source.selectedSource = (char *)obs_data_get_string(dataObject, "selectedSource");
	timerData->source.selectedScene = (char *)obs_data_get_string(dataObject, "selectedScene");
	timerData->display.endMessage = (char *)obs_data_get_string(dataObject, "endMessage");

	QDateTime savedTime = QDateTime::fromString((char *)obs_data_get_string(dataObject, "dateTime"));
	QDateTime currentTime = QDateTime::currentDateTime();
	if (currentTime > savedTime) {
		savedTime = savedTime.addDays(1);
		if (currentTime > savedTime)
			savedTime = savedTime.addDays(1);
	}
	timerData->dateTime = savedTime;

	timerData->periodDuration.days = (int)obs_data_get_int(dataObject, "periodDays");
	timerData->periodDuration.hours = (int)obs_data_get_int(dataObject, "periodHours");
	timerData->periodDuration.minutes = (int)obs_data_get_int(dataObject, "periodMinutes");
	timerData->periodDuration.seconds = (int)obs_data_get_int(dataObject, "periodSeconds");

	timerData->display.showDays = (bool)obs_data_get_bool(dataObject, "showDays");
	timerData->display.showHours = (bool)obs_data_get_bool(dataObject, "showHours");
	timerData->display.showMinutes = (bool)obs_data_get_bool(dataObject, "showMinutes");
	timerData->display.showSeconds = (bool)obs_data_get_bool(dataObject, "showSeconds");
	timerData->display.showEndMessage = (bool)obs_data_get_bool(dataObject, "showEndMessage");
	timerData->display.showEndScene = (bool)obs_data_get_bool(dataObject, "showEndScene");

	timerData->display.useFormattedOutput = (bool)obs_data_get_bool(dataObject, "useFormattedOutput");
	timerData->display.outputStringFormat = (char *)obs_data_get_string(dataObject, "outputStringFormat");

	timerData->smoothenPeriodTimer = (bool)obs_data_get_bool(dataObject, "smoothenPeriodTimer");

	timerData->selectedCountdownType = (CountdownType)obs_data_get_int(dataObject, "selectedCountdownType");

	timerData->timeLeftInMillis = (long long)obs_data_get_int(dataObject, "timeLeftInMillis");
	timerData->hotkeys.startCountdownHotkeyId = (int)obs_data_get_int(dataObject, "startCountdownHotkeyId");
	timerData->hotkeys.pauseCountdownHotkeyId = (int)obs_data_get_int(dataObject, "pauseCountdownHotkeyId");
	timerData->hotkeys.setCountdownHotkeyId = (int)obs_data_get_int(dataObject, "setCountdownHotkeyId");
	timerData->hotkeys.startCountdownToTimeHotkeyId =
		(int)obs_data_get_int(dataObject, "startCountdownToTimeHotkeyId");
	timerData->hotkeys.stopCountdownToTimeHotkeyId =
		(int)obs_data_get_int(dataObject, "stopCountdownToTimeHotkeyId");

	SetTimerData();
}
