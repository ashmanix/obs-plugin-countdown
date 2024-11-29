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
	explicit AshmanixTimer(QWidget *parent = nullptr, QString id = "",
			       obs_websocket_vendor vendor = nullptr);
	~AshmanixTimer();

	struct TimerWidgetStruct {
		QString timerId;
		bool isPlaying;
		bool shouldCountUp = false;
		bool showLeadingZero = true;
		QString selectedSource = "";
		QString selectedScene = "";
		QString endMessage = "";
		QTimer *timer;
		QDateTime dateTime;

		bool showDays = true;
		bool showHours = true;
		bool showMinutes = true;
		bool showSeconds = true;
		bool showEndMessage = false;
		bool showEndSource = false;

		long long timeLeftInMillis = 0;
		QDateTime timeToCountUpToStart;

		QTabWidget *countdownTypeTabWidget;

		std::string textSourceNameText;
		std::string sceneSourceNameText;

		int startCountdownHotkeyId = -1;
		int pauseCountdownHotkeyId = -1;
		int setCountdownHotkeyId = -1;
		int startCountdownToTimeHotkeyId = -1;
		int stopCountdownToTimeHotkeyId = -1;
	};

	enum WebsocketRequestType { ADD_TIME = 1, SET_TIME = 2 };
	QString GetTimerID();
	QPushButton *GetDeleteButton();

private:
	enum SourceType { TEXT_SOURCE = 1, SCENE_SOURCE = 2 };
	static const int COUNTDOWNPERIOD = 1000;
	static inline const char *ZEROSTRING = "00:00:00:00";
	obs_websocket_vendor vendor = nullptr;

	TimerWidgetStruct *countdownTimerData;
	Ui::AshmanixTimer *ui;
	SettingsDialog *settingsDialogUi = nullptr;

	void SetupTimerWidgetUI(TimerWidgetStruct *context);
	void StartTimerCounting(TimerWidgetStruct *context);
	void StopTimerCounting(TimerWidgetStruct *context);
	void InitialiseTimerTime(TimerWidgetStruct *context);
	QString ConvertDateTimeToFormattedDisplayString(long long timeInMillis,
							bool showLeadingZero);
	bool IsSetTimeZero(TimerWidgetStruct *context);
	void UpdateDateTimeDisplay(long long timeInMillis);
	void SetSourceText(QString newText);
	void SetCurrentScene();
	long long GetMillisFromPeriodUI();

	void SendWebsocketEvent(const char *eventName, obs_data_t *eventData);
	void SendTimerTickEvent(QString timerId, long long timeLeftInMillis);
	void SendTimerStateEvent(QString timerId, const char *state);

	void ConnectUISignalHandlers();

signals:
	void RequestTimerReset();
	void RequestDelete(QString id);

private slots:
	// void DeleteRequested();
	void PlayButtonClicked();
	void PauseButtonClicked();
	void ResetButtonClicked();

	void ToTimePlayButtonClicked();
	void ToTimeStopButtonClicked();

	void SettingsButtonClicked();
	void DeleteButtonClicked();

	void TimerAdjust();
	void HandleTimerReset();
};

#endif // ASHMANIXTIMER_H
