#ifndef SINGLECOLOURRULEWIDGET_H
#define SINGLECOLOURRULEWIDGET_H

#include <QWidget>
#include <QString>
#include <QPushButton>
#include <QSharedPointer>
#include <QToolButton>
#include <QFont>
#include <QString>
#include <QColor>
#include <QColorDialog>

#include <obs-frontend-api.h>

#include "../../classes/colour-rule.hpp"
#include "../../ui/ui_SingleColourRule.h"

class SingleColourRuleWidget : public QWidget {
	Q_OBJECT

public:
	explicit SingleColourRuleWidget(QWidget *parent = nullptr, QSharedPointer<ColourRule> colourRule = nullptr);

	QString GetID() const;
	QSharedPointer<ColourRule> GetColourRule() const;

	void SetData(ColourRule *colourRule);
	void SetID(QString newId);
	void SetLabel(QString labelValue);
	void SetMaxTime(PeriodData newTime);
	void SetMinTime(PeriodData newTime);
	void UpdateStyledUIComponents();
	void SetEnabled(bool isEnabled);
	void ValidateColourRuleTimes(ColourRule::TimerType changeType, PeriodData newTime);

signals:
	void RemoveColoureRule(QString id);
	void ChangeDetected();
	void ChangeTimeDetected(SingleColourRuleWidget *rule, ColourRule::TimerType type, PeriodData timeDuration);

private slots:
	void HandleColourButtonPushed();

private:
	Ui::SingleColourRuleWidget *m_ui = nullptr;
	QSharedPointer<ColourRule> m_colourRule;

	void ConnectUISignalHandlers();
	void SetupWidgetUI();
	void ToggleBlockAllUISignals(bool shouldBlock);
	void HandleTimerChange(ColourRule::TimerType timerType, ColourRule::DurationType durationType, int value);
	void SetTextColour(QColor colour);
};

#endif // SINGLECOLOURRULEWIDGET_H
