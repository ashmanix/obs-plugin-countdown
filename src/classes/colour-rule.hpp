#ifndef COLOURRULE_H
#define COLOURRULE_H

#include <QString>
#include <QColor>

#include "../utils/timer-utils.hpp"

class ColourRule {

public:
	explicit ColourRule(QString id = {}, TimerDuration startTime = {0, 0, 0, 0},
			    TimerDuration endTime = {0, 0, 0, 0}, QColor colour = QColor("white"));

	QString GetID() const;
	TimerDuration GetStartTime() const;
	TimerDuration GetEndTime() const;
	QColor GetColour() const;

	void SetID(QString newId);
	void SetStartTime(TimerDuration newTime);
	void SetEndTime(TimerDuration newTime);
	void SetColour(QColor newColour);

	bool IsTimeWithinRule(TimerDuration timetToCompare);

private:
	QString m_id;
	TimerDuration m_startTime;
	TimerDuration m_endTime;
	QColor m_colour;

	qint64 ConvertToMilliSeconds(const TimerDuration &timeToConvert);
};

#endif // COLOURRULE_H
