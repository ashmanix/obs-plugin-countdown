#ifndef TIMERPERSISTENCE_H
#define TIMERPERSISTENCE_H

#include <obs-data.h>
#include "../../utils/timer-utils.hpp"

// Forward declarations
class TimerWidgetStruct;

class TimerPersistence {

public:
	// TimerPersistence();

	void SaveTimerWidgetDataToOBSSaveData(TimerWidgetStruct *timerData, obs_data_t *dataObject);
	void LoadTimerWidgetDataFromOBSSaveData(TimerWidgetStruct *timerData, obs_data_t *dataObject);
};

#endif // TIMERPERSISTENCE_H
