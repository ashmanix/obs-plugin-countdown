#ifndef TIMERUIMANAGER_H
#define TIMERUIMANAGER_H

#include <QObject>
#include <QString>
#include <QSpacerItem>
#include <QDateTime>

#include "../../ui/ui_AshmanixTimer.h"
#include "../../utils/timer-utils.hpp"

// Forward declarations
class Ui_AshmanixTimer;
class TimerWidgetStruct;
class AshmanixTimer;
class SettingsDialog;
class CountdownDockWidget;

class TimerUIManager : public QWidget {
	Q_OBJECT

public:
	explicit TimerUIManager(QWidget *parent, Ui::AshmanixTimer *ui, TimerWidgetStruct *countdownTimerData,
				CountdownDockWidget *countdownDockWidget);
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
	void UpdateDateTimeDisplay(long long timeInMillis);
	bool IsSetTimeZero();
	void UpdateTimerPeriod(PeriodData periodData);
	void SetHideMultiTimerUIButtons(bool shouldHide);
	void SetIsUpButtonDisabled(bool isDisabled);
	void SetIsDownButtonDisabled(bool isDisabled);
	QString ConvertDateTimeToFormattedDisplayString(long long timeInMillis, bool showLeadingZero);

signals:
	void RequestDelete(QString id);
	void MoveTimer(Direction direction);
	void UpdateSource(QString sourceString);
	void TimerChange(TimerCommand command);

public slots:
	void HandleTimerAction(TimerAction action);

private slots:
	void SettingsButtonClicked();
	void DeleteButtonClicked();
	void HandleTimerValueChange(const QString &type, int value);
	void DateTimeChanged(QDateTime newDateTime);

private:
	static inline const char *ZEROSTRING = "00:00:00:00";
	static inline const char *TIMETEMPLATECODE = "%time%";

	QWidget *parent;
	Ui_AshmanixTimer *ui;
	SettingsDialog *settingsDialogUi = nullptr;
	TimerWidgetStruct *data;
	CountdownDockWidget *countdownDockWidget;
	QSpacerItem *deleteButtonSpacer;

	void TimerStateChange(TimerCommand command);
};

#endif // TIMERUIMANAGER_H
