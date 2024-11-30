#ifndef TIMERUTILS_H
#define TIMERUTILS_H

#include <QDateTime>
#include <QString>
#include <QTimer>
#include <QTabWidget>
#include <QTime>
#include <obs.h>

struct TimerWidgetStruct {
	QString timerId;
	bool isPlaying;
	bool shouldCountUp = false;
	bool showLeadingZero = true;
	QString selectedSource = "";
	QString selectedScene = "";
	QString endMessage = "";
	QTimer *timer;
	QDateTime dateTime;

	int periodDays = 0;
	int periodHours = 0;
	int periodMinutes = 0;
	int periodSeconds = 0;

	bool showDays = true;
	bool showHours = true;
	bool showMinutes = true;
	bool showSeconds = true;
	bool showEndMessage = false;
	bool showEndScene = false;

	long long timeLeftInMillis = 0;
	QDateTime timeToCountUpToStart;
	int countdownTypeSelectedTab = 0;

	int startCountdownHotkeyId = -1;
	int pauseCountdownHotkeyId = -1;
	int setCountdownHotkeyId = -1;
	int startCountdownToTimeHotkeyId = -1;
	int stopCountdownToTimeHotkeyId = -1;
};

enum WebsocketRequestType { ADD_TIME = 1, SET_TIME = 2, GET_TIME = 3 };

const char *ConvertToConstChar(QString value);
long long ConvertStringPeriodToMillis(const char *time_string);
QString ConvertMillisToDateTimeString(long long timeInMillis);
QString GetFormattedTimerString(bool daysState, bool hoursState,
				bool minutesState, bool secondsState,
				bool showLeadingZero, long long timeInMillis);
long long CalcToCurrentDateTimeInMillis(QDateTime timeToCountdownTo,
					int countdownPeriod = 1000);
void SaveTimerWidgetDataToOBSSaveData(obs_data_t *dataObject,
				      TimerWidgetStruct *timerData);
void LoadTimerWidgetDataFromOBSSaveData(obs_data_t *dataObject,
					TimerWidgetStruct *timerData);

#endif // TIMERUTILS_H
