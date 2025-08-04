#ifndef COLOURCHANGEWIDGET_H
#define COLOURCHANGEWIDGET_H

#include <QObject>
#include <QString>
#include <QSpacerItem>
#include <QDateTime>
#include <QSharedPointer>
#include <QHash>
#include <QColor>
#include <QColorDialog>
#include <QList>

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
	void SetData(TimerWidgetStruct *countdownTimerData);
	QList<ColourRuleData> GetColourRuleList();
	QColor GetMainTextColour();
	bool GetShouldUseColourChange();
	void ClearSelection();

signals:
	void ColourRuleChanged();

private slots:
	void HandleAddButtonClicked();
	void HandleColourRuleDeletion(QString id);
	void HandleMainColourButtonClick();
	void HandleRuleChange(SingleColourRuleWidget *ruleWidget, ColourRule::TimerType type, PeriodData timeDuration);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	void HandleEnableCheckBoxSelected(Qt::CheckState state);
#else
	void HandleEnableCheckBoxSelected(int state);
#endif

private:
	Ui::TextColourSettingsWidget *m_ui;
	QVector<QSharedPointer<ColourRule>> m_colourRules;
	QHash<QString, SingleColourRuleWidget *> m_colourRuleWidgetIds;
	QColor m_mainTextColour;
	QSharedPointer<TimerWidgetStruct> m_data;

	void ConnectUISignalHandlers();
	void ConnectColourRuleSignalHandlers(SingleColourRuleWidget *colourRuleWidget);
	void AddColourRule(QSharedPointer<ColourRule> in_colourRule = nullptr);
	void SetEnabled(bool isEnabled);
	void UpdateAllRuleLabels();
	void SetMainTextColour(QColor color);
};

#endif // COLOURCHANGEWIDGET_H
