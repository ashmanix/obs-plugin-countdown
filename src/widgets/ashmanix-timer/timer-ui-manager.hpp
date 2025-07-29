#ifndef TIMERUIMANAGER_H
#define TIMERUIMANAGER_H

#include <QObject>
#include <QString>
#include <QSpacerItem>
#include <QDateTime>
#include <QSharedPointer>

#include "../../ui/ui_AshmanixTimer.h"
#include "../../utils/timer-utils.hpp"

// Forward declarations
class Ui_AshmanixTimer;
class AshmanixTimer;
class SettingsDialog;
class CountdownDockWidget;
struct TimerWidgetStruct;

class TimerUIManager : public QWidget {
	Q_OBJECT

public:
	explicit TimerUIManager(QWidget *parent, Ui::AshmanixTimer *ui, TimerWidgetStruct *countdownTimerData,
				CountdownDockWidget *countdownDockWidget, const char *zeroString);
	void SetupUI();
	void ConnectUISignalHandlers();
	void SetTimeUI();
	bool AddTime(const char *stringTime, bool isCountingUp);
	bool SetTime(const char *stringTime);
	void UpdateStyles();
	void ToggleTimeType(CountdownType type);
	long long GetMillisFromPeriodUI();
	QDateTime GetToTimeEditDateTime();
	void UpdateTimeDisplayTooltip();
	void UpdateDisplay(long long timeToUpdateInMillis);
	bool IsSetTimeZero();
	void UpdateTimerPeriod(PeriodData periodData);
	void SetHideMultiTimerUIButtons(bool shouldHide);
	void SetIsUpButtonDisabled(bool isDisabled);
	void SetIsDownButtonDisabled(bool isDisabled);
	void TimerStateChange(TimerCommand command);
	void SetZeroTimeDisplay();
	QDateTime GetToDateTimeValue();
	void SetTimerIDLabel(QString newId);

signals:
	void RequestDelete(QString id);
	void MoveTimer(Direction direction);
	void TimerChange(TimerCommand command);

public slots:
	void HandleTimerAction(TimerAction action);

private slots:
	void SettingsButtonClicked();
	void DeleteButtonClicked();
	void HandleTimerValueChange(const QString &type, int value);
	void DateTimeChanged(QDateTime newDateTime);

private:
	QWidget *parent;
	Ui_AshmanixTimer *ui;
	QSharedPointer<SettingsDialog> settingsDialogUi = nullptr;
	TimerWidgetStruct *data;
	CountdownDockWidget *countdownDockWidget;
	QSpacerItem *deleteButtonSpacer;
	const char *zeroString;
};

#endif // TIMERUIMANAGER_H
