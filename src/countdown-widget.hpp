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
#include <QTime>
#include <QChar>
#include <QIcon>
#include <QGroupBox>
#include <QCheckBox>
#include <QTabWidget>
#include <Qt>

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

#include "plugin-macros.generated.h"
#include "ui_CountdownTimer.h"

#define CONFIG "config.json"

class CountdownDockWidget : public QDockWidget {
	Q_OBJECT
public:
	explicit CountdownDockWidget(QWidget *parent = nullptr);
	~CountdownDockWidget();
	void ConfigureWebSocketConnection();
	struct CountdownWidgetStruct {
		bool isPlaying;
		QTimer *timer;
		QTime *time;

		QTabWidget *countdownTypeTabWidget;

		std::string textSourceNameText;
		std::string sceneSourceNameText;

		int startCountdownHotkeyId = -1;
		int pauseCountdownHotkeyId = -1;
		int setCountdownHotkeyId = -1;
		int startCountdownToTimeHotkeyId = -1;
		int stopCountdownToTimeHotkeyId = -1;
	};

	struct TimeIncrements {
		int hours;
		int minutes;
		int seconds;
		int milliseconds;
	};

private:
	enum SourceType { TEXT_SOURCE = 1, SCENE_SOURCE = 2 };
	static const int COUNTDOWNPERIOD = 1000;
	obs_websocket_vendor vendor = nullptr;

	CountdownWidgetStruct *countdownTimerData;
	Ui::CountdownTimer *ui;

	void SetupCountdownWidgetUI(CountdownWidgetStruct *context);
	void StartTimerCounting(CountdownWidgetStruct *context);
	void StopTimerCounting(CountdownWidgetStruct *context);
	void InitialiseTimerTime(CountdownWidgetStruct *context);
	QString ConvertTimeToDisplayString(QTime *timeToConvert);
	bool IsSetTimeZero(CountdownWidgetStruct *context);
	void ConnectObsSignalHandlers();
	void ConnectUISignalHandlers();
	void UpdateTimeDisplay(QTime *time);
	void SetSourceText(QString newText);
	void SetCurrentScene();
	void SaveSettings();
	void RegisterHotkeys(CountdownWidgetStruct *context);
	void UnregisterHotkeys();
	void ClickButton(CountdownWidgetStruct *context);
	TimeIncrements CalculateTimeDifference(QTime timeToCountdownTo);
	
	static void VendorRequestPlayButtonClicked(obs_data_t *request_data, obs_data_t *response_data,
			    void *);

	const char *ConvertToConstChar(QString value);

	static void ObsSourceSignalHandler();

	static void OBSSourceCreated(void *param, calldata_t *calldata);
	static void OBSSourceDeleted(void *param, calldata_t *calldata);
	static void OBSSourceRenamed(void *param, calldata_t *calldata);

	static void OBSFrontendEventHandler(enum obs_frontend_event event,
					    void *private_data);

	static int CheckSourceType(obs_source_t *source);
	static void LoadSavedSettings(Ui::CountdownTimer *ui);

private slots:

	void PlayButtonClicked();
	void PauseButtonClicked();
	void ResetButtonClicked();

	void ToTimePlayButtonClicked();
	void ToTimeStopButtonClicked();

	void EndMessageCheckBoxSelected(int state);
	void SceneSwitchCheckBoxSelected(int state);
	void HandleTextSourceChange(QString newText);
	void HandleSceneSourceChange(QString newText);
	void TimerDecrement();
};

#endif // COUNTDOWNWIDGET_H