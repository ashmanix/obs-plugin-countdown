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
#include <QRegularExpression>
#include <QValidator>
#include <QTimer>
#include <QTime>
#include <QChar>
#include <QIcon>
#include <QGroupBox>
#include <QCheckbox>
#include <Qt>

#include <string>
#include <iostream>
#include <list>
#include <util/base.h>
#include <util/platform.h>
#include <vector>
#include <obs.h>
#include <obs.hpp>
#include <obs-frontend-api.h>
#include <obs-module.h>

#include "plugin-macros.generated.h"

#define CONFIG "config.json"

class CountdownDockWidget : public QDockWidget {
	Q_OBJECT
public:
	explicit CountdownDockWidget(QWidget *parent = nullptr);
	~CountdownDockWidget();
	// static bool SortSourceList (SourceListItem a, SourceListItem b) { return a.name < b.name; };
	// bool id_match(const SourceListItem &a, const char value){return (a.id) == value};
	

	struct CountdownWidgetStruct {
		bool isPlaying;

		QWidget *countdownTimerUI;
		QTimer *timer;
		QTime *time;
		QLCDNumber *timerDisplay;
		QLineEdit *timerHours;
		QLineEdit *timerMinutes;
		QLineEdit *timerSeconds;

		QLineEdit *timerEndMessage;
		QLabel *timerEndLabel;

		QPushButton *playButton;
		QPushButton *pauseButton;
		QPushButton *resetButton;

		QComboBox *textSourceDropdownList;
		QComboBox *sceneSourceDropdownList;
		QLabel *sceneSwitchLabel;

		QCheckBox *endMessageCheckBox;
		QCheckBox *switchSceneCheckBox;
		OBSSignal textSourceAddedSignals;
		OBSSignal textSourceRemovedSignals;

		std::string textSourceNameText;
		std::string sceneSourceNameText;
	};

private:
	enum SourceType { TEXT_SOURCE = 1, SCENE_SOURCE = 2};
	static const int COUNTDOWNPERIOD = 1000;

	CountdownWidgetStruct *countdownTimerData;

	QVBoxLayout *SetupCountdownWidgetUI(CountdownWidgetStruct *context);
	void StartTimerCounting(CountdownWidgetStruct *context);
	void StopTimerCounting(CountdownWidgetStruct *context);
	void InitialiseTimerTime(CountdownWidgetStruct *context);
	QString ConvertTimeToDisplayString(QTime *timeToConvert);
	bool IsSetTimeZero(CountdownWidgetStruct *context);
	void ConnectObsSignalHandlers(CountdownWidgetStruct* context);
	void ConnectUISignalHandlers(CountdownWidgetStruct *context);
	void UpdateTimeDisplay(CountdownWidgetStruct* context, QTime *time);
	// void SetSelectedSource(const QString &sourceName);
	void SetSourceText(CountdownWidgetStruct* context, QString newText);
	void SetCurrentScene();
	void SaveSettings();

	const char* ConvertToConstChar(QString value);

	static void ObsSourceSignalHandler();

	static void OBSSourceCreated(void *param, calldata_t *calldata);
	static void OBSSourceDeleted(void *param, calldata_t *calldata);
	static void OBSSourceRenamed(void *param, calldata_t *calldata);

	static void OBSFrontendEventHandler(enum obs_frontend_event event, void *private_data);
	// static void ConnectUISignalHandlers(CountdownWidgetStruct *context);
	// static void UpdateSceneList(CountdownWidgetStruct *context);

	static int CheckSourceType(obs_source_t *source);
	static void LoadSavedSettings(CountdownWidgetStruct* context);
	// static void AddSourceToList(CountdownWidgetStruct *context, obs_source_t *source, int sourceType);
	// static void RemoveSourceFromList(CountdownWidgetStruct *context, obs_source_t *source, int sourceType);
	// static void RenameSource(CountdownWidgetStruct *context, obs_source_t *source, int sourceType);


private slots:

	void PlayButtonClicked();
	void PauseButtonClicked();
	void ResetButtonClicked();
	void EndMessageCheckBoxSelected(int state);
	void SceneSwitchCheckBoxSelected(int state);
	void HandleTextSourceChange(QString newText);
	void HandleSceneSourceChange(QString newText);
	void TimerDecrement();
};

#endif // COUNTDOWNWIDGET_H