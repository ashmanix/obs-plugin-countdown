#include "colour-rule.hpp"
#include <obs-module.h>
#include "../utils/obs-utils.hpp"

ColourRule::ColourRule(QString id, TimerDuration startTime, TimerDuration endTime, QColor colour)
	: m_id(id),
	  m_startTime(startTime),
	  m_endTime(endTime),
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

TimerDuration ColourRule::GetStartTime() const
{
	return m_startTime;
}

TimerDuration ColourRule::GetEndTime() const
{
	return m_endTime;
}

QColor ColourRule::GetColour() const
{
	return m_colour;
}

void ColourRule::SetID(QString new_id)
{
	m_id = new_id;
}

void ColourRule::SetStartTime(TimerDuration m_newTime)
{
	m_startTime = m_newTime;
}

void ColourRule::SetEndTime(TimerDuration m_newTime)
{
	m_endTime = m_newTime;
}

void ColourRule::SetColour(QColor newColour)
{
	m_colour = newColour;
}

bool ColourRule::IsTimeWithinRule(TimerDuration timeToCompare)
{
	qint64 startTime = ConvertToMilliSeconds(m_startTime);
	qint64 endTime = ConvertToMilliSeconds(m_endTime);
	qint64 compareTime = ConvertToMilliSeconds(timeToCompare);

	return compareTime >= startTime && compareTime <= endTime;
}

//  ----------------------------------------------- Private Functions ------------------------------------------------

qint64 ColourRule::ConvertToMilliSeconds(const TimerDuration &timeToConvert)
{
	const qint64 days = static_cast<qint64>(timeToConvert.days);
	const qint64 hours = static_cast<qint64>(timeToConvert.hours);
	const qint64 minutes = static_cast<qint64>(timeToConvert.minutes);
	const qint64 seconds = static_cast<qint64>(timeToConvert.seconds);

	return (((days * 24 + hours) * 60 + minutes) * 60 + seconds) * 1000;
}
