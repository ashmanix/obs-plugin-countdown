#include "timer-utils.hpp"

const char *ConvertToConstChar(QString value)
{
	QByteArray ba = value.toLocal8Bit();
	const char *cString = ba.data();
	return cString;
}

PeriodData ConvertStringPeriodToPeriodData(const char *time_string)
{
	int days = 0;
	int hours = 0;
	int minutes = 0;
	int seconds = 0;

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
		sscanf(time_string, "%d:%d:%d:%d", &days, &hours, &minutes, &seconds);
		break;
	default:
		sscanf(time_string, "%d:%d:%d:%d", &days, &hours, &minutes, &seconds);
		break;
	}

	return {days, hours, minutes, seconds};
}

PeriodData ConvertMillisToPeriodData(long long timeInMillis)
{
	int days = 0;
	int hours = 0;
	int minutes = 0;
	int seconds = 0;

	long long daysFromMillis = timeInMillis / (24 * 60 * 60 * 1000);
	long long remainingMilliseconds = timeInMillis % (24 * 60 * 60 * 1000);

	QTime time = QTime::fromMSecsSinceStartOfDay(static_cast<int>(remainingMilliseconds));
	days = static_cast<int>(daysFromMillis);
	hours = time.hour();
	minutes = time.minute();
	seconds = time.second();

	return {days, hours, minutes, seconds};
}

long long ConvertStringPeriodToMillis(const char *time_string)
{
	PeriodData periodData = ConvertStringPeriodToPeriodData(time_string);

	// Convert each unit into milliseconds and sum them up
	long long totalMilliseconds = 0;
	totalMilliseconds += static_cast<long long>(periodData.days) * 86400000; // 24 * 60 * 60 * 1000
	totalMilliseconds += static_cast<long long>(periodData.hours) * 3600000; // 60 * 60 * 1000
	totalMilliseconds += static_cast<long long>(periodData.minutes) * 60000; // 60 * 1000
	totalMilliseconds += static_cast<long long>(periodData.seconds) * 1000;  // 1000
	return totalMilliseconds;
}

QString ConvertMillisToDateTimeString(long long timeInMillis)
{
	long long days = timeInMillis / (24 * 60 * 60 * 1000);
	long long remainingMilliseconds = timeInMillis % (24 * 60 * 60 * 1000);

	QTime time = QTime::fromMSecsSinceStartOfDay(static_cast<int>(remainingMilliseconds));

	return QString("%1:%2:%3:%4")
		.arg(days, 2, 10, QChar('0'))        // Days with leading zeros
		.arg(time.hour(), 2, 10, QChar('0')) // Hours with leading zeros
		.arg(time.minute(), 2, 10,
		     QChar('0')) // Minutes with leading zeros
		.arg(time.second(), 2, 10,
		     QChar('0')); // Seconds with leading zeros
}

QString GetFormattedTimerString(bool daysState, bool hoursState, bool minutesState, bool secondsState,
				bool showLeadingZero, long long timeInMillis)
{
	long long days = timeInMillis / (24 * 60 * 60 * 1000);
	long long remainingMilliseconds = timeInMillis % (24 * 60 * 60 * 1000);

	QTime time = QTime::fromMSecsSinceStartOfDay(static_cast<int>(remainingMilliseconds));

	QString formattedDateTimeString = "";

	bool isFirstField = true;

	auto appendField = [&](long long value, bool state) {
		if (state) {
			if (isFirstField && !showLeadingZero) {
				// Append without leading zero
				formattedDateTimeString += QString::number(value);
			} else {
				// Append with leading zero
				formattedDateTimeString += QString("%1").arg(value, 2, 10, QChar('0'));
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

long long CalcToCurrentDateTimeInMillis(QDateTime timeToCountdownTo, int countdownPeriod)
{
	QDateTime systemTime = QDateTime::currentDateTime();
	long long millisecondsDifference = systemTime.msecsTo(timeToCountdownTo);
	long long millisResult = 0;

	millisecondsDifference = millisecondsDifference + countdownPeriod; // Add 1 second for countdown

	if (millisecondsDifference > 0) {
		millisResult = millisecondsDifference;
	}

	return millisResult;
}

QColor GetTextColourFromRulesList(const QList<ColourRuleData> &colourRuleList, long long compareMillis)
{
	qint64 compareTimeMilli;

	if (compareMillis >= 0)
		compareTimeMilli = (compareMillis / 1000) * 1000;
	else {
		compareTimeMilli = -(((-compareMillis + 999) / 1000) * 1000);
	}

	for (const ColourRuleData &rule : colourRuleList) {
		auto minTimeMilli = ConvertTimerDurationToMilliSeconds(rule.minTime);
		auto maxTimeMilli = ConvertTimerDurationToMilliSeconds(rule.maxTime);

		if (maxTimeMilli < minTimeMilli)
			continue;

		if (compareTimeMilli >= minTimeMilli && compareTimeMilli <= maxTimeMilli)
			return rule.colour;
	}

	return QColor();
}

qint64 ConvertTimerDurationToMilliSeconds(const PeriodData &timeToConvert)
{
	const auto days = static_cast<qint64>(timeToConvert.days);
	const auto hours = static_cast<qint64>(timeToConvert.hours);
	const auto minutes = static_cast<qint64>(timeToConvert.minutes);
	const auto seconds = static_cast<qint64>(timeToConvert.seconds);

	return (((days * 24 + hours) * 60 + minutes) * 60 + seconds) * 1000;
}

bool IsPeriodDataBefore(PeriodData time, PeriodData timeToCompareAgainst)
{
	qint64 timeMilli = ConvertTimerDurationToMilliSeconds(time);
	qint64 timeToCompareAgainstMilli = ConvertTimerDurationToMilliSeconds(timeToCompareAgainst);

	return timeMilli < timeToCompareAgainstMilli;
}

bool IsPeriodDataAfter(PeriodData time, PeriodData timeToCompareAgainst)
{
	qint64 timeMilli = ConvertTimerDurationToMilliSeconds(time);
	qint64 timeToCompareAgainstMilli = ConvertTimerDurationToMilliSeconds(timeToCompareAgainst);

	return timeMilli > timeToCompareAgainstMilli;
}

PeriodData AddSecondsToTimerDuration(PeriodData time, int noOfSeconds)
{
	auto timeMilli = ConvertTimerDurationToMilliSeconds(time);
	timeMilli = std::max(timeMilli + (noOfSeconds * 1000), 0LL);
	return ConvertMillisToPeriodData(timeMilli);
}

long long ColourToInt(QColor color)
{
	auto shift = [&](unsigned val, int shift2) {
		return ((val & 0xff) << shift2);
	};

	return shift(color.red(), 0) | shift(color.green(), 8) | shift(color.blue(), 16) | shift(color.alpha(), 24);
}
