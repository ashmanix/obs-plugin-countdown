#ifndef TIMERPERSISTENCE_H
#define TIMERPERSISTENCE_H

#include <obs-data.h>
#include "../../utils/timer-utils.hpp"

// Forward declarations
struct TimerWidgetStruct;

class TimerPersistence {

public:
	void SaveTimerWidgetDataToOBSSaveData(TimerWidgetStruct *timerData, obs_data_t *dataObject);
	void LoadTimerWidgetDataFromOBSSaveData(TimerWidgetStruct *timerData, obs_data_t *dataObject);

private:
	static PeriodData GetDurationObject(obs_data_t *object, const char *key);
	static void SetDurationObject(obs_data_t *object, const char *key, const PeriodData &d);

	static void SaveColourRules(obs_data_t *settings, const QList<ColourRuleData> &rules);
	static void SaveColour(obs_data_t *settings, QColor colour);
	static QList<ColourRuleData> LoadColourRules(obs_data_t *settings);
	static QColor LoadColour(obs_data_t *settings);
};

#endif // TIMERPERSISTENCE_H
