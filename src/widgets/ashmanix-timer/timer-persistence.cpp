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

	// ------------------------- Text Colour -------------------------
	obs_data_set_bool(dataObject, "useTextColour", timerData->display.useTextColour);
	// Save the main text colour as an object
	obs_data_t *mainColourObject = obs_data_create();
	SaveColour(mainColourObject, timerData->display.mainTextColour);
	obs_data_set_obj(dataObject, "mainTextColour", mainColourObject);
	obs_data_release(mainColourObject);
	// Save colour rules
	SaveColourRules(dataObject, timerData->display.colourRuleList);

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

	// ------------------------- Text Colour -------------------------
	timerData->display.useTextColour = (bool)obs_data_get_bool(dataObject, "useTextColour");
	// Save the main text colour as an object
	obs_data_t *mainColourObject = obs_data_get_obj(dataObject, "mainTextColour");
	if (mainColourObject) {
		timerData->display.mainTextColour = LoadColour(mainColourObject);
		obs_data_release(mainColourObject);
	}

	timerData->display.colourRuleList = LoadColourRules(dataObject);
}

PeriodData TimerPersistence::GetDurationObject(obs_data_t *object, const char *key)
{
	PeriodData timeDuration;
	obs_data_t *dur = obs_data_get_obj(object, key); // returns a ref you must release (if non-null)
	if (dur) {
		timeDuration.days = (int)obs_data_get_int(dur, "days");
		timeDuration.hours = (int)obs_data_get_int(dur, "hours");
		timeDuration.minutes = (int)obs_data_get_int(dur, "minutes");
		timeDuration.seconds = (int)obs_data_get_int(dur, "seconds");
		obs_data_release(dur);
	}
	return timeDuration;
}

void TimerPersistence::SetDurationObject(obs_data_t *object, const char *key, const PeriodData &d)
{
	obs_data_t *durationObject = obs_data_create();
	obs_data_set_int(durationObject, "days", d.days);
	obs_data_set_int(durationObject, "hours", d.hours);
	obs_data_set_int(durationObject, "minutes", d.minutes);
	obs_data_set_int(durationObject, "seconds", d.seconds);
	obs_data_set_obj(object, key, durationObject);
	obs_data_release(durationObject);
}

void TimerPersistence::SaveColourRules(obs_data_t *settings, const QList<ColourRuleData> &rules)
{
	obs_data_array_t *array = obs_data_array_create();

	for (const ColourRuleData &r : rules) {
		obs_data_t *object = obs_data_create();

		SetDurationObject(object, "minTime", r.minTime);
		SetDurationObject(object, "maxTime", r.maxTime);

		SaveColour(object, r.colour);

		obs_data_array_push_back(array, object);
		obs_data_release(object);
	}

	obs_data_set_array(settings, "colourRules", array);
	obs_data_array_release(array);
}

void TimerPersistence::SaveColour(obs_data_t *object, QColor colour)
{
	obs_data_set_int(object, "r", colour.red());
	obs_data_set_int(object, "g", colour.green());
	obs_data_set_int(object, "b", colour.blue());
	obs_data_set_int(object, "a", colour.alpha());
}

QList<ColourRuleData> TimerPersistence::LoadColourRules(obs_data_t *settings)
{
	QList<ColourRuleData> colourRules;

	obs_data_array_t *dataArray = obs_data_get_array(settings, "colourRules"); // gets a ref
	if (!dataArray)
		return colourRules;

	const size_t count = obs_data_array_count(dataArray);
	colourRules.reserve((int)count);

	for (size_t i = 0; i < count; ++i) {
		obs_data_t *obj = obs_data_array_item(dataArray, i); // returns a ref; must release

		ColourRuleData r;
		r.minTime = GetDurationObject(obj, "minTime");
		r.maxTime = GetDurationObject(obj, "maxTime");
		r.colour = LoadColour(obj);

		colourRules.push_back(r);
		obs_data_release(obj);
	}

	obs_data_array_release(dataArray);
	return colourRules;
}

QColor TimerPersistence::LoadColour(obs_data_t *object)
{
	const auto rr = (int)obs_data_get_int(object, "r");
	const auto gg = (int)obs_data_get_int(object, "g");
	const auto bb = (int)obs_data_get_int(object, "b");
	const auto aa = (int)obs_data_get_int(object, "a");

	return QColor(rr, gg, bb, aa);
}
