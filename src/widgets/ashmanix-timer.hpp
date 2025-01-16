#ifndef ASHMANIXTIMER_H
#define ASHMANIXTIMER_H

#include <algorithm>

#include <QWidget>
#include <QDateTime>
#include <QTabWidget>
#include <QRegularExpression>
#include <QValidator>
#include <QTimer>
#include <QUuid>
#include <QCryptographicHash>
#include <QSpinBox>

#include <obs.h>
#include <obs.hpp>
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs-websocket-api.h>
#include <obs-data.h>
#include <QMetaType>

#include "../plugin-support.h"
#include "../ui/ui_AshmanixTimer.h"
#include "../utils/timer-utils.hpp"
#include "../utils/obs-utils.hpp"

// Forward declarations
class SettingsDialog;
class CountdownDockWidget;

class AshmanixTimer : public QWidget {
	Q_OBJECT

public:
	explicit AshmanixTimer(QWidget *parent = nullptr, obs_websocket_vendor vendor = nullptr,
			       obs_data_t *savedData = nullptr, CountdownDockWidget *mDockWidget = nullptr);
	~AshmanixTimer();

	void SaveTimerWidgetDataToOBSSaveData(obs_data_t *dataObject);
	void LoadTimerWidgetDataFromOBSSaveData(obs_data_t *dataObject);

	QString GetTimerID();
	void SetTimerID(QString newId);
	TimerWidgetStruct *GetTimerData();
	void SetHideMultiTimerUIButtons(bool shouldHide);
	void SetIsUpButtonDisabled(bool isDisabled);
	void SetIsDownButtonDisabled(bool isDisabled);
	void SetTimerData();
	bool AlterTime(WebsocketRequestType requestType, const char *stringTime);

	void PressPlayButton();
	void PressResetButton();
	void PressStopButton();
	void PressToTimePlayButton();
	void PressToTimeStopButton();
	void UpdateStyles();
	void StartTimer();
	void StopTimer();

private:
	enum SourceType { TEXT_SOURCE = 1, SCENE_SOURCE = 2 };
	static const int TIMERPERIOD = 1000;
	static inline const char *ZEROSTRING = "00:00:00:00";
	static inline const char *TIMETEMPLATECODE = "%time%";
	obs_websocket_vendor vendor = nullptr;
	long long lastDisplayedSeconds = -1;
	QSpacerItem *deleteButtonSpacer;
	CountdownDockWidget *mainDockWidget;

	static inline const char *TIMERSTARTHOTKEYNAME = "Ashmanix_Countdown_Timer_Start";
	static inline const char *TIMERPAUSEHOTKEYNAME = "Ashmanix_Countdown_Timer_Pause";
	static inline const char *TIMERSETHOTKEYNAME = "Ashmanix_Countdown_Timer_Set";
	static inline const char *TIMERTOTIMESTARTHOTKEYNAME = "Ashmanix_Countdown_Timer_To_Time_Start";
	static inline const char *TIMERTOTIMESTOPHOTKEYNAME = "Ashmanix_Countdown_Timer_To_Time_Stop";

	TimerWidgetStruct countdownTimerData;
	Ui::AshmanixTimer *ui;
	SettingsDialog *settingsDialogUi = nullptr;

	void SetupTimerWidgetUI();
	void ConnectUISignalHandlers();

	QString ConvertDateTimeToFormattedDisplayString(long long timeInMillis, bool showLeadingZero);
	void StartTimerCounting();
	void StopTimerCounting();
	void InitialiseTimerTime(bool setTimeLeftToUI = true);
	bool IsSetTimeZero();

	void UpdateDateTimeDisplay(long long timeInMillis);
	void SetSourceText(QString newText);
	void SetCurrentScene();
	long long GetMillisFromPeriodUI();

	void SendTimerTickEvent(QString timerId, long long timeLeftInMillis);
	void SendTimerStateEvent(QString timerId, const char *state);

	void RegisterAllHotkeys(obs_data_t *saved_data);
	void UnregisterAllHotkeys();

	std::string GetFullHotkeyName(std::string nameString, const char *joinText = "_");

	void UpdateTimeDisplayTooltip();
	bool AddTime(const char *stringTime, bool isCountingUp);
	bool SetTime(const char *stringTime, bool isCountingUp);
	void UpdateTimerPeriod(PeriodData periodData);

signals:
	void RequestTimerReset(bool restartOnly = false);
	void RequestDelete(QString id);
	void RequestSendWebsocketEvent(const char *eventName, obs_data_t *eventData);
	void MoveTimer(QString direction, QString timerId);

public slots:
	void PlayButtonClicked();
	void PauseButtonClicked();
	void ResetButtonClicked();

	void ToTimePlayButtonClicked();
	void ToTimeStopButtonClicked();

private slots:
	void SettingsButtonClicked();
	void DeleteButtonClicked();

	void TimerAdjust();
	void HandleTimerReset(bool restartOnly = false);

	void DaysChanged(int newValue);
	void HoursChanged(int newValue);
	void MinutesChanged(int newValue);
	void SecondsChanged(int newValue);
	void DateTimeChanged(QDateTime newDateTime);
	void EmitMoveTimerDownSignal();
	void EmitMoveTimerUpSignal();
	void ToggleTimeType(CountdownType type);
};

#endif // ASHMANIXTIMER_H
