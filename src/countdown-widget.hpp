#ifndef COUNTDOWNWIDGET_H
#define COUNTDOWNWIDGET_H

#include <QMainWindow>
#include <QDockWidget>
#include <QEvent>
#include <QLabel>
#include <QPushButton>
#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QLCDNumber>
#include <QDateTimeEdit>
#include <QRegularExpression>
#include <QValidator>
#include <QTimer>
#include <QDateTime>
#include <QChar>
#include <QIcon>
#include <QGroupBox>
#include <QCheckBox>
#include <QTabWidget>
#include <Qt>
#include <QMap>

#include <string>
#include <iostream>
#include <chrono>
#include <list>
#include <util/base.h>
#include <util/platform.h>
#include <util/config-file.h>
#include <vector>
#include <obs.h>
#include <obs.hpp>
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs-websocket-api.h>

#include "plugin-support.h"
#include "ui/ui_CountdownTimer.h"
#include "utils/timer-utils.hpp"
#include "widgets/ashmanix-timer.hpp"

#define CONFIG "config.json"

class CountdownDockWidget : public QWidget {
	Q_OBJECT
public:
	explicit CountdownDockWidget(QWidget *parent = nullptr);
	~CountdownDockWidget();
	void ConfigureWebSocketConnection();
	// struct CountdownWidgetStruct {
	// 	bool isPlaying;
	// 	bool shouldCountUp = false;
	// 	QTimer *timer;
	// 	QDateTime dateTime;
	// 	long long timeLeftInMillis = 0;
	// 	QDateTime timeToCountUpToStart;

	// 	QTabWidget *countdownTypeTabWidget;

	// 	std::string textSourceNameText;
	// 	std::string sceneSourceNameText;

	// 	int startCountdownHotkeyId = -1;
	// 	int pauseCountdownHotkeyId = -1;
	// 	int setCountdownHotkeyId = -1;
	// 	int startCountdownToTimeHotkeyId = -1;
	// 	int stopCountdownToTimeHotkeyId = -1;
	// };

	enum WebsocketRequestType { ADD_TIME = 1, SET_TIME = 2 };
	struct WebsocketCallbackData {
		CountdownDockWidget *instance;
		WebsocketRequestType requestType;
		const char *requestDataKey;
	};

private:
	QMap<QString, AshmanixTimer *> timerWidgetMap;
	QVBoxLayout *timerListLayout;

	// enum SourceType { TEXT_SOURCE = 1, SCENE_SOURCE = 2 };
	static const int COUNTDOWNPERIOD = 1000;
	static char const ZEROSTRING[];
	static inline const char *VENDORNAME = "ashmanix-countdown-timer";
	obs_websocket_vendor vendor = nullptr;

	// CountdownWidgetStruct *countdownTimerData;
	Ui::CountdownTimer *ui;

	void SetupCountdownWidgetUI();
	// void StartTimerCounting(CountdownWidgetStruct *context);
	// void StopTimerCounting(CountdownWidgetStruct *context);
	// void InitialiseTimerTime(CountdownWidgetStruct *context);
	// QString ConvertDateTimeToFormattedDisplayString(long long timeInMillis,
	// 						bool showLeadingZero);
	// bool IsSetTimeZero(CountdownWidgetStruct *context);
	void ConnectObsSignalHandlers();
	void ConnectUISignalHandlers();
	// void UpdateDateTimeDisplay(long long timeInMillis);
	// void SetSourceText(QString newText);
	void SetCurrentScene();
	void SaveSettings();
	void RegisterHotkeys();
	void UnregisterHotkeys();

	void SendWebsocketEvent(const char *eventName, obs_data_t *eventData);
	void SendTimerTickEvent(long long timeLeftInMillis);
	void SendTimerStateEvent(const char *state);

	long long GetMillisFromPeriodUI();

	static void OBSSourceCreated(void *param, calldata_t *calldata);
	static void OBSSourceDeleted(void *param, calldata_t *calldata);
	static void OBSSourceRenamed(void *param, calldata_t *calldata);
	static void OBSFrontendEventHandler(enum obs_frontend_event event,
					    void *private_data);
	static int CheckSourceType(obs_source_t *source);
	static void LoadSavedSettings(Ui::CountdownTimer *ui);
	// static void ChangeTimerTimeViaWebsocket(obs_data_t *request_data,
	// obs_data_t *response_data,
	// void *priv_data);
signals:
	void RequestTimerReset();

private slots:
	void AddTimerButtonClicked();
	void RemoveTimerButtonClicked(QString id);

	// void EndMessageCheckBoxSelected(int state);
	// void SceneSwitchCheckBoxSelected(int state);
	void HandleTextSourceChange(QString newText);
	void HandleSceneSourceChange(QString newText);
	// void TimerAdjust();
	void HandleTimerReset();
};

#endif // COUNTDOWNWIDGET_H
