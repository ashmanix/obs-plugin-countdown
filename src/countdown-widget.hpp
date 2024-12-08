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
#include <QJsonArray>
#include <QJsonDocument>

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
#include <obs-data.h>
#include <QMetaType>

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
	int GetNumberOfTimers();
	AshmanixTimer *GetFirstTimerWidget();

	struct WebsocketCallbackData {
		CountdownDockWidget *instance;
		WebsocketRequestType requestType;
		const char *requestDataKey;
		const char *requestTimerIdKey;
	};

private:
	QMap<QString, AshmanixTimer *> timerWidgetMap;
	QVBoxLayout *timerListLayout;
	int addTimerHotkeyId = -1;
	int startAllTimersHotkeyId = -1;
	int stopAllTimersHotkeyId = -1;

	static inline const char *addTimerHotkeyName =
		"Ashmanix_Countdown_Timer_Add_Timer";
	static inline const char *startAllTimersHotkeyName =
		"Ashmanix_Countdown_Timer_Start_All_Timers";
	static inline const char *stopAllTimersHotkeyName =
		"Ashmanix_Countdown_Timer_Stop_All_Timers";

	enum SourceType { TEXT_SOURCE = 1, SCENE_SOURCE = 2 };

	static const int COUNTDOWNPERIOD = 1000;
	static inline const char *VENDORNAME = "ashmanix-countdown-timer";
	static inline const char *TIMERIDKEY = "timer_id";

	obs_websocket_vendor vendor = nullptr;

	Ui::CountdownTimer *ui;

	void SetupCountdownWidgetUI();
	void ConnectUISignalHandlers();
	void ConnectTimerSignalHandlers(AshmanixTimer *timerWidget);
	void SaveSettings();
	void RegisterAllHotkeys(obs_data_t *savedData);
	void UnregisterAllHotkeys();
	void AddTimer(obs_data_t *savedData = nullptr);
	void UpdateTimerListMoveButtonState();
	static void
	UpdateWidgetStyles(CountdownDockWidget *countdownDockWidget);

	static void OBSFrontendEventHandler(enum obs_frontend_event event,
					    void *private_data);
	static void LoadSavedSettings(CountdownDockWidget *timerWidgetMap);
	static AshmanixTimer *
	AttemptToGetTimerWidgetById(CountdownDockWidget *countdownWidget,
				    const char *websocketTimerID);
	static void ChangeTimerTimeViaWebsocket(obs_data_t *request_data,
						obs_data_t *response_data,
						void *priv_data);
	static void GetTimerStateViaWebsocket(obs_data_t *request_data,
					      obs_data_t *response_data,
					      void *priv_data);
	static void HandleWebsocketButtonPressRequest(obs_data_t *request_data,
						      obs_data_t *response_data,
						      void *priv_data);

signals:
	void RequestTimerReset();

private slots:
	void RemoveTimerButtonClicked(QString id);
	void AddTimerButtonClicked();
	void HandleWebsocketSendEvent(const char *eventName,
				      obs_data_t *eventData);
	void MoveTimerInList(QString direction, QString id);
	void StartAllTimers();
	void StopAllTimers();
};

#endif // COUNTDOWNWIDGET_H
