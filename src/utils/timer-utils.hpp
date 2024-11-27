#ifndef TIMERUTILS_H
#define TIMERUTILS_H

#include <QDateTime>
#include <QString>
#include <QTime>

const char *ConvertToConstChar(QString value);
long long ConvertStringPeriodToMillis(const char *time_string);
QString ConvertMillisToDateTimeString(long long timeInMillis);
QString GetFormattedTimerString(int daysState, int hoursState, int minutesState,
				int secondsState, bool showLeadingZero,
				long long timeInMillis);
long long CalculateDateTimeDifference(QDateTime timeToCountdownTo);

#endif // TIMERUTILS_H
