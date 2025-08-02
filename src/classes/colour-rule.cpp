#include "colour-rule.hpp"
#include <obs-module.h>
#include "../utils/obs-utils.hpp"

ColourRule::ColourRule(QString id, TimerDuration minTime, TimerDuration maxTime, QColor colour)
	: m_id(id),
	  m_minTime(minTime),
	  m_maxTime(maxTime),
	  m_colour(colour)
{
	if (m_id.isEmpty()) {
		// Get a unique ID for blendshape rule
		m_id = GenerateUniqueID();
	}
}

QString ColourRule::GetID() const
{
	return m_id;
}

TimerDuration ColourRule::GetMaxTime() const
{
	return m_maxTime;
}

TimerDuration ColourRule::GetMinTime() const
{
	return m_minTime;
}

QColor ColourRule::GetColour() const
{
	return m_colour;
}

void ColourRule::SetID(QString new_id)
{
	m_id = new_id;
}

void ColourRule::SetTime(TimerType type, TimerDuration newTime)
{
	switch (type) {
	case TimerType::MIN:
		m_minTime = newTime;
		break;
	case TimerType::MAX:
		m_maxTime = newTime;
		break;
	}
}

void ColourRule::SetColour(QColor newColour)
{
	m_colour = newColour;
}

// bool ColourRule::IsTimeWithinRule(TimerDuration timeToCompare)
// {
// 	qint64 minTime = ConvertToMilliSeconds(m_minTime);
// 	qint64 maxTime = ConvertToMilliSeconds(m_maxTime);
// 	qint64 compareTime = ConvertToMilliSeconds(timeToCompare);

// 	return compareTime >= minTime && compareTime <= maxTime;
// }

// //  ----------------------------------------------- Private Functions ------------------------------------------------

// qint64 ColourRule::ConvertToMilliSeconds(const TimerDuration &timeToConvert)
// {
// 	const qint64 days = static_cast<qint64>(timeToConvert.days);
// 	const qint64 hours = static_cast<qint64>(timeToConvert.hours);
// 	const qint64 minutes = static_cast<qint64>(timeToConvert.minutes);
// 	const qint64 seconds = static_cast<qint64>(timeToConvert.seconds);

// 	return (((days * 24 + hours) * 60 + minutes) * 60 + seconds) * 1000;
// }
