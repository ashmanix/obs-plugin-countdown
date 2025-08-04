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
#include "./ashmanix-timer/timer-ui-manager.hpp"
#include "./ashmanix-timer/timer-persistence.hpp"
#include "./ashmanix-timer/hotkey-manager.hpp"

// Forward declarations
class SettingsDialog;
class CountdownDockWidget;
class WebsocketNotifier;

class AshmanixTimer : public QWidget {
	Q_OBJECT

public:
	explicit AshmanixTimer(QWidget *parent = nullptr, WebsocketNotifier *websocketNotifier = nullptr,
			       obs_data_t *savedData = nullptr, CountdownDockWidget *mDockWidget = nullptr);
	~AshmanixTimer() override;

	QString GetTimerID();
	void SetTimerID(QString newId);
	TimerWidgetStruct *GetTimerData();
	void SetHideMultiTimerUIButtons(bool shouldHide);
	void SetIsUpButtonDisabled(bool isDisabled);
	void SetIsDownButtonDisabled(bool isDisabled);
	bool AlterTime(WebsocketRequestType requestType, const char *stringTime);
	void UpdateStyles();
	void StartTimer(bool shouldReset = false);
	void StopTimer();
	void ActivateTimerAction(TimerAction action);
	void SaveData(obs_data_t *dataObject);

private:
	enum SourceType { TEXT_SOURCE = 1, SCENE_SOURCE = 2 };
	static const int TIMERPERIOD = 1000;
	static inline const char *ZEROSTRING = "00:00:00:00";
	static inline const char *TIMETEMPLATECODE = "%time%";
	obs_websocket_vendor vendor = nullptr;
	long long lastDisplayedSeconds = -1;
	QSpacerItem *deleteButtonSpacer;
	CountdownDockWidget *mainDockWidget;

	TimerWidgetStruct countdownTimerData;
	Ui::AshmanixTimer *ui;

	TimerUIManager *uiManager = nullptr;
	TimerPersistence *timerPersistence = nullptr;
	HotkeyManager *hotkeyManager = nullptr;
	WebsocketNotifier *websocketNotifier = nullptr;

	void ConnectSignalHandlers();

	void StartTimerCounting();
	void StopTimerCounting();
	void InitialiseTimerTime(bool setTimeLeftToUI = true);

	void UpdateDateTimeDisplay(long long millis);
	QString ConvertDateTimeToFormattedDisplayString(long long timeInMillis, bool showLeadingZero);
	void SetSourceText(QString newText, QColor textColour = QColor());
	void SetCurrentScene();

	std::string GetFullHotkeyName(std::string nameString, const char *joinText = "_");

	bool AddTime(const char *stringTime, bool isCountingUp);
	bool SetTime(const char *stringTime);

signals:
	void RequestTimerReset(bool restartOnly = false);
	void RequestDelete(QString id);
	void RequestSendWebsocketEvent(const char *eventName, obs_data_t *eventData);
	void MoveTimer(Direction direction, QString timerId);

private slots:

	void TimerAdjust();
	void HandleTimerReset(bool restartOnly = false);
};

#endif // ASHMANIXTIMER_H
