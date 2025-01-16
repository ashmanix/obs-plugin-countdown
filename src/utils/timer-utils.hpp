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
	TO_TIME_STOP = 8,
	PLAY_ALL = 9,
	STOP_ALL = 10
};

struct TimerWidgetStruct {
	QString timerId;
	bool isPlaying;
	bool shouldCountUp = false;
	bool showLeadingZero = true;
	bool startOnStreamStart = false;
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

	bool useFormattedOutput = false;
	QString outputStringFormat = "Will be back in %time% see you soon!";

	bool smoothenPeriodTimer = false;

	long long timeLeftInMillis = 0;
	QDateTime timeAtTimerStart;
	CountdownType selectedCountdownType = PERIOD;

	int startCountdownHotkeyId = -1;
	int pauseCountdownHotkeyId = -1;
	int setCountdownHotkeyId = -1;
	int startCountdownToTimeHotkeyId = -1;
	int stopCountdownToTimeHotkeyId = -1;

	QWidget *periodVLayout;
	QWidget *datetimeVLayout;
};

struct PeriodData {
	int days;
	int hours;
	int minutes;
	int seconds;
};

struct Result {
	bool success;
	QString errorMessage;
};

const char *ConvertToConstChar(QString value);
PeriodData ConvertStringPeriodToPeriodData(const char *time_string);
PeriodData ConvertMillisToPeriodData(long long timeInMillis);
long long ConvertStringPeriodToMillis(const char *time_string);
QString ConvertMillisToDateTimeString(long long timeInMillis);
QString GetFormattedTimerString(bool daysState, bool hoursState, bool minutesState, bool secondsState,
				bool showLeadingZero, long long timeInMillis);
long long CalcToCurrentDateTimeInMillis(QDateTime timeToCountdownTo, int countdownPeriod = 1000);

#endif // TIMERUTILS_H
