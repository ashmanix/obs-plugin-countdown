#include "colour-rule.hpp"
#include <obs-module.h>
#include "../utils/obs-utils.hpp"

ColourRule::ColourRule(QString id, PeriodData minTime, PeriodData maxTime, QColor colour)
	: m_id(id),
	  m_maxTime(maxTime),
	  m_minTime(minTime),
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

PeriodData ColourRule::GetMaxTime() const
{
	return m_maxTime;
}

PeriodData ColourRule::GetMinTime() const
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

void ColourRule::SetTime(TimerType type, PeriodData newTime)
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
