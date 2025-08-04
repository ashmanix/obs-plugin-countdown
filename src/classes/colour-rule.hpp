#ifndef COLOURRULE_H
#define COLOURRULE_H

#include <QString>
#include <QColor>

#include "../utils/timer-utils.hpp"

class ColourRule {

public:
	enum class TimerType { MIN = 1, MAX = 2 };
	enum class DurationType { DAYS = 0, HOURS = 1, MINUTES = 2, SECONDS = 3 };

	explicit ColourRule(QString id = {}, PeriodData minTime = {0, 0, 0, 0}, PeriodData maxTime = {0, 0, 0, 0},
			    QColor colour = QColor("white"));

	QString GetID() const;
	PeriodData GetMinTime() const;
	PeriodData GetMaxTime() const;
	QColor GetColour() const;

	void SetID(QString newId);
	void SetTime(TimerType type, PeriodData newTime);
	void SetColour(QColor newColour);

private:
	QString m_id;
	PeriodData m_maxTime;
	PeriodData m_minTime;
	QColor m_colour;
};

#endif // COLOURRULE_H
