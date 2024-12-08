#ifndef TIMERUTILS_H
#define TIMERUTILS_H

#include <QDateTime>
#include <QString>
#include <QTimer>
#include <QTabWidget>
#include <QTime>

enum CountdownType { PERIOD = 0, DATETIME = 1 };

enum WebsocketRequestType {
	ADD_TIME = 1,
	SET_TIME = 2,
	GET_TIME = 3,
	PERIOD_PLAY = 4,
	PERIOD_PAUSE = 5,
	PERIOD_SET = 6,
	TO_TIME_PLAY = 7,
	TO_TIME_STOP = 8
};

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
	QDateTime timeToCountTo;
	CountdownType selectedCountdownType = PERIOD;

	int startCountdownHotkeyId = -1;
	int pauseCountdownHotkeyId = -1;
	int setCountdownHotkeyId = -1;
	int startCountdownToTimeHotkeyId = -1;
	int stopCountdownToTimeHotkeyId = -1;

	QWidget *periodVLayout;
	QWidget *datetimeVLayout;
};

const char *ConvertToConstChar(QString value);
long long ConvertStringPeriodToMillis(const char *time_string);
QString ConvertMillisToDateTimeString(long long timeInMillis);
QString GetFormattedTimerString(bool daysState, bool hoursState,
				bool minutesState, bool secondsState,
				bool showLeadingZero, long long timeInMillis);
long long CalcToCurrentDateTimeInMillis(QDateTime timeToCountdownTo,
					int countdownPeriod = 1000);

#endif // TIMERUTILS_H
