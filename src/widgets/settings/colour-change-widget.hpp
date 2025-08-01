#ifndef COLOURCHANGEWIDGET_H
#define COLOURCHANGEWIDGET_H

#include <QObject>
#include <QString>
#include <QSpacerItem>
#include <QDateTime>
#include <QSharedPointer>

#include "../../plugin-support.h"
#include "../../ui/ui_TextColourSettings.h"
#include "../../utils/timer-utils.hpp"

#include "single-colour-rule-widget.hpp"

// Forward declarations
class Ui_AshmanixTimer;
class AshmanixTimer;
class SettingsDialog;
class CountdownDockWidget;
struct TimerWidgetStruct;

class ColourChangeWidget : public QWidget {
	Q_OBJECT

public:
	explicit ColourChangeWidget(QWidget *parent, TimerWidgetStruct *countdownTimerData);
	void SetupWidgetUI();
	void UpdateStyledUIComponents();
	void SetData();
	void ClearSelection();

signals:
	void ColourRuleChanged();

private slots:
	void HandleEnableClick(bool isEnabled);
	void HandleAddButtonClicked();
	void HandleColourRuleDeleteButtonClicked(QString id);

private:
	Ui::TextColourSettingsWidget *m_ui;
	QMap<QString, SingleColourRuleWidget *> m_colourRulesWidgetMap;
	QSharedPointer<TimerWidgetStruct> data;

	void ConnectUISignalHandlers();
	void ConnectColourRuleSignalHandlers(SingleColourRuleWidget *colourRuleWidget);
	void AddColourRule(QSharedPointer<ColourRule> in_colourRule = nullptr);
	void SetEnabled(bool isEnabled);
};

#endif // COLOURCHANGEWIDGET_H
