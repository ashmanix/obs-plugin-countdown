#include "timer-utils.hpp"

const char *ConvertToConstChar(QString value)
{
	QByteArray ba = value.toLocal8Bit();
	const char *cString = ba.data();
	return cString;
}

long long ConvertStringPeriodToMillis(const char *time_string)
{
	int days = 0, hours = 0, minutes = 0, seconds = 0;

	// Count the number of colons in the string
	int colonCount = 0;
	for (const char *c = time_string; *c != '\0'; ++c) {
		if (*c == ':')
			++colonCount;
	}

	switch (colonCount) {
	case 0:
		sscanf(time_string, "%d", &seconds);
		break;
	case 1:
		sscanf(time_string, "%d:%d", &minutes, &seconds);
		break;
	case 2:
		sscanf(time_string, "%d:%d:%d", &hours, &minutes, &seconds);
		break;
	case 4:
		sscanf(time_string, "%d:%d:%d:%d", &days, &hours, &minutes,
		       &seconds);
		break;
	default:
		sscanf(time_string, "%d:%d:%d:%d", &days, &hours, &minutes,
		       &seconds);
		break;
	}

	// Convert each unit into milliseconds and sum them up
	long long totalMilliseconds = 0;
	totalMilliseconds +=
		static_cast<long long>(days) * 86400000; // 24 * 60 * 60 * 1000
	totalMilliseconds +=
		static_cast<long long>(hours) * 3600000; // 60 * 60 * 1000
	totalMilliseconds +=
		static_cast<long long>(minutes) * 60000; // 60 * 1000
	totalMilliseconds += static_cast<long long>(seconds) * 1000; // 1000
	return totalMilliseconds;
}

QString ConvertMillisToDateTimeString(long long timeInMillis)
{
	long long days = timeInMillis / (24 * 60 * 60 * 1000);
	long long remainingMilliseconds = timeInMillis % (24 * 60 * 60 * 1000);

	QTime time = QTime::fromMSecsSinceStartOfDay(
		static_cast<int>(remainingMilliseconds));

	return QString("%1:%2:%3:%4")
		.arg(days, 2, 10, QChar('0'))        // Days with leading zeros
		.arg(time.hour(), 2, 10, QChar('0')) // Hours with leading zeros
		.arg(time.minute(), 2, 10,
		     QChar('0')) // Minutes with leading zeros
		.arg(time.second(), 2, 10,
		     QChar('0')); // Seconds with leading zeros
}

QString GetFormattedTimerString(bool daysState, bool hoursState,
				bool minutesState, bool secondsState,
				bool showLeadingZero, long long timeInMillis)
{
	long long days = timeInMillis / (24 * 60 * 60 * 1000);
	long long remainingMilliseconds = timeInMillis % (24 * 60 * 60 * 1000);

	QTime time = QTime::fromMSecsSinceStartOfDay(
		static_cast<int>(remainingMilliseconds));

	QString formattedDateTimeString = "";

	bool isFirstField = true;

	auto appendField = [&](long long value, bool state) {
		if (state) {
			if (isFirstField && !showLeadingZero) {
				// Append without leading zero
				formattedDateTimeString +=
					QString::number(value);
			} else {
				// Append with leading zero
				formattedDateTimeString += QString("%1").arg(
					value, 2, 10, QChar('0'));
			}
			isFirstField = false;
		}
	};

	appendField(days, daysState);
	if (!formattedDateTimeString.isEmpty() && hoursState)
		formattedDateTimeString += ":";
	appendField(time.hour(), hoursState);
	if (!formattedDateTimeString.isEmpty() && minutesState)
		formattedDateTimeString += ":";
	appendField(time.minute(), minutesState);
	if (!formattedDateTimeString.isEmpty() && secondsState)
		formattedDateTimeString += ":";
	appendField(time.second(), secondsState);

	return formattedDateTimeString;
}

long long CalcToCurrentDateTimeInMillis(QDateTime timeToCountdownTo,
					int countdownPeriod)
{
	QDateTime systemTime = QDateTime::currentDateTime();
	long long millisecondsDifference =
		systemTime.msecsTo(timeToCountdownTo);
	long long millisResult = 0;

	millisecondsDifference = millisecondsDifference +
				 countdownPeriod; // Add 1 second for countdown

	if (millisecondsDifference > 0) {
		millisResult = millisecondsDifference;
	}

	return millisResult;
}
