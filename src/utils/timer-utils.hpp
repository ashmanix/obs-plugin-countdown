#ifndef TIMERUTILS_H
#define TIMERUTILS_H

#include <QDateTime>
#include <QString>
#include <QTime>

const char *ConvertToConstChar(QString value);
long long ConvertStringPeriodToMillis(const char *time_string);
QString ConvertMillisToDateTimeString(long long timeInMillis);
QString GetFormattedTimerString(bool daysState, bool hoursState,
				bool minutesState, bool secondsState,
				bool showLeadingZero, long long timeInMillis);
long long CalcToCurrentDateTimeInMillis(QDateTime timeToCountdownTo,
					int countdownPeriod = 1000);

#endif // TIMERUTILS_H
