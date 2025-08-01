#ifndef SINGLECOLOURRULEWIDGET_H
#define SINGLECOLOURRULEWIDGET_H

#include <QWidget>
#include <QString>
#include <QPushButton>
#include <QSharedPointer>
#include <QToolButton>

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

	void SetData(QSharedPointer<ColourRule> in_colourRule);
	void UpdateStyledUIComponents();

signals:
	void RemoveBlendshapeRule(QString id);
	void Change();

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
};

#endif // SINGLECOLOURRULEWIDGET_H
