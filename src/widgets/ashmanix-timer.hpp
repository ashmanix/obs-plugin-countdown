#ifndef ASHMANIXTIMER_H
#define ASHMANIXTIMER_H

#include <QWidget>
#include <QDateTime>
#include <QTabWidget>
#include <QRegularExpression>
#include <QValidator>
#include <QTimer>
#include <QUuid>
#include <QCryptographicHash>

#include <obs.h>
#include <obs.hpp>
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs-websocket-api.h>

#include "../plugin-support.h"
#include "../ui/ui_AshmanixTimer.h"
#include "../utils/timer-utils.hpp"
#include "settings-dialog.hpp"

class AshmanixTimer : public QWidget {
	Q_OBJECT

public:
	explicit AshmanixTimer(QWidget *parent = nullptr,
			       obs_websocket_vendor vendor = nullptr,
			       obs_data_t *savedData = nullptr);
	~AshmanixTimer();

	void SaveTimerWidgetDataToOBSSaveData(obs_data_t *dataObject);
	void LoadTimerWidgetDataFromOBSSaveData(obs_data_t *dataObject);

	QString GetTimerID();
	QPushButton *GetDeleteButton();
	TimerWidgetStruct *GetTimerData();
	void SetTimerData();
	bool AlterTime(WebsocketRequestType requestType,
		       long long timeInMillis);

	void PressPlayButton();
	void PressResetButton();
	void PressStopButton();
	void PressToTimePlayButton();
	void PressToTimeStopButton();

private:
	struct RegisterHotKeyCallbackData {
		std::function<void()>
			function; // Function pointer to callback function
		const char *
			hotkeyLogMessage; // Message to log when hotkey is triggered
					  // Ui::AshmanixTimer *ui;
	};
	enum SourceType { TEXT_SOURCE = 1, SCENE_SOURCE = 2 };
	static const int COUNTDOWNPERIOD = 1000;
	static inline const char *ZEROSTRING = "00:00:00:00";
	obs_websocket_vendor vendor = nullptr;

	TimerWidgetStruct countdownTimerData;
	Ui::AshmanixTimer *ui;
	SettingsDialog *settingsDialogUi = nullptr;

	void SetupTimerWidgetUI();
	void ConnectUISignalHandlers();

	QString ConvertDateTimeToFormattedDisplayString(long long timeInMillis,
							bool showLeadingZero);
	void StartTimerCounting();
	void StopTimerCounting();
	void InitialiseTimerTime();
	bool IsSetTimeZero();

	void UpdateDateTimeDisplay(long long timeInMillis);
	void SetSourceText(QString newText);
	void SetCurrentScene();
	long long GetMillisFromPeriodUI();

	void SendTimerTickEvent(QString timerId, long long timeLeftInMillis);
	void SendTimerStateEvent(QString timerId, const char *state);

	void RegisterAllHotKeys(obs_data_t *saved_data);
	void UnregisterAllHotKeys();

	void RegisterHotKey(int &id, const char *name, const char *description,
			    std::function<void()> function,
			    const char *buttonLogMessage,
			    obs_data_t *savedData);
	static void SaveHotKey(obs_data_t *sv_data, obs_hotkey_id id,
			       const char *name);
	static void *HotKeyCallback(void *incoming_data);

	const char *GetFullHotKeyName(const char *nameString, const char* joinText = "_");

signals:
	void RequestTimerReset();
	void RequestDelete(QString id);
	void RequestSendWebsocketEvent(const char *eventName,
				       obs_data_t *eventData);

public slots:
	void PlayButtonClicked();
	void PauseButtonClicked();
	void ResetButtonClicked();

	void ToTimePlayButtonClicked();
	void ToTimeStopButtonClicked();

private slots:
	void CountdownTypeTabChanged(int index);

	void SettingsButtonClicked();
	void DeleteButtonClicked();

	void TimerAdjust();
	void HandleTimerReset();

	void DaysChanged(QString newText);
	void HoursChanged(QString newText);
	void MinutesChanged(QString newText);
	void SecondsChanged(QString newText);
	void DateTimeChanged(QDateTime newDateTime);
};

#endif // ASHMANIXTIMER_H
