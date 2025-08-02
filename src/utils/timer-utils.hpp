#ifndef TIMERUTILS_H
#define TIMERUTILS_H

#include <QDateTime>
#include <QString>
#include <QTimer>
#include <QTabWidget>
#include <QTime>
#include <QColor>

const int TIMERPERIOD = 1000;

enum class TimerAction { PLAY, PAUSE, RESET, TO_TIME_PLAY, TO_TIME_STOP };

enum class CountdownType { PERIOD = 0, DATETIME = 1 };

enum class Direction { UP, DOWN };

enum class TimerCommand { START, STOP };

enum class WebsocketRequestType {
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

struct TimerDuration {
	int days = 0;
	int hours = 0;
	int minutes = 0;
	int seconds = 0;
};

struct ColourRuleData {
	TimerDuration minTime;
	TimerDuration maxTime;
	QColor colour;
};

struct DisplayOptions {
	bool showDays = true;
	bool showHours = true;
	bool showMinutes = true;
	bool showSeconds = true;
	bool showLeadingZero = true;
	bool showEndMessage = false;
	bool showEndScene = false;
	bool useFormattedOutput = false;
	QString outputStringFormat = "Will be back in %time% see you soon!";
	QString endMessage;

	bool useTextColour = false;
	QColor mainTextColour;
	QList<ColourRuleData> colourRuleList;
};

struct SourceConfig {
	QString selectedSource;
	QString selectedScene;
};

struct HotkeyBindings {
	int startCountdownHotkeyId = -1;
	int pauseCountdownHotkeyId = -1;
	int setCountdownHotkeyId = -1;
	int startCountdownToTimeHotkeyId = -1;
	int stopCountdownToTimeHotkeyId = -1;
};

struct TimerWidgetStruct {
	QString timerId;
	bool isPlaying;
	bool shouldCountUp = false;
	bool startOnStreamStart = false;
	bool resetTimerOnStreamStart = false;
	CountdownType selectedCountdownType = CountdownType::PERIOD;

	QTimer *timer = nullptr;
	QDateTime dateTime;
	QDateTime timeAtTimerStart;
	long long timeLeftInMillis = 0;
	bool smoothenPeriodTimer = false;

	TimerDuration periodDuration;
	DisplayOptions display;
	SourceConfig source;
	HotkeyBindings hotkeys;

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
QColor GetTextColourFromRulesList(QList<ColourRuleData> &colourRuleList, long long compareTimeMilli);
qint64 ConvertTimerDurationToMilliSeconds(const TimerDuration &timeToConvert);
long long ColourToInt(QColor color);

#endif // TIMERUTILS_H
