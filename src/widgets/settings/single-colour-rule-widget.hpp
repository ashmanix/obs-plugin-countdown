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

#include "plugin-support.h"
#include "../../classes/colour-rule.hpp"
#include "../../ui/ui_SingleColourRule.h"

class SingleColourRuleWidget : public QWidget {
	Q_OBJECT

public:
	explicit SingleColourRuleWidget(QWidget *parent = nullptr, QSharedPointer<ColourRule> colourRule = nullptr);
	// ~SingleColourRuleWidget() override;

	QString GetID() const;
	QSharedPointer<ColourRule> GetColourRule() const;

	void SetData(ColourRuleData colourRuleData);
	void SetID(QString newId);
	void SetLabel(QString labelValue);
	void UpdateStyledUIComponents();
	void SetEnabled(bool isEnabled);

signals:
	void RemoveColoureRule(QString id);
	void ChangeDetected();

private slots:
	void HandleColourButtonPushed();

private:
	Ui::SingleColourRuleWidget *m_ui = nullptr;
	QSharedPointer<ColourRule> m_colourRule;
	enum TimerType { START = 1, END = 2 };

	void ConnectUISignalHandlers();
	void SetupWidgetUI();
	void ToggleBlockAllUISignals(bool shouldBlock);
	void HandleTimerChange(TimerType type, TimerDuration time);
	void SetTextColour(QColor colour);
};

#endif // SINGLECOLOURRULEWIDGET_H
