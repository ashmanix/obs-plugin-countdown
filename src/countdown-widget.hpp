#ifndef COUNTDOWNWIDGET_H
#define COUNTDOWNWIDGET_H

// #include <QWidget>
#include <QDockWidget>
#include <QEvent>
#include <QLabel>
#include <QPushButton>
#include <QDockWidget>
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
#include <vector>
#include <obs.h>
#include <obs.hpp>
#include <obs-frontend-api.h>

#include "plugin-macros.generated.h"

class CountdownDockWidget : public QDockWidget {
	Q_OBJECT
public:
	explicit CountdownDockWidget(QWidget *parent);
	~CountdownDockWidget();
	struct SourceListItem {
		const char *name;
		const char *id;
		obs_source_t *source;
		bool operator<(const SourceListItem& a) const {return name < a.name;}
	};

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

		QPushButton *playButton;
		QPushButton *pauseButton;
		QPushButton *resetButton;

		QComboBox *textSourceDropdownList;
		QComboBox *sceneSourceDropdownList;

		QCheckBox *endMessageCheckBox;
		QCheckBox *switchSceneCheckBox;

		// std::vector<SourcesList> sourcesList;
		std::list<SourceListItem> textSourcesList;
		std::list<obs_source_t*>::iterator it;
		OBSSignal textSourceAddedSignals;
		OBSSignal textSourceRemovedSignals;
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
	void UpdateTimeDisplay(CountdownWidgetStruct* context, QTime *time);
	// void SetSelectedSource(const QString &sourceName);
	void SetSourceText(CountdownWidgetStruct* context, QString newText);

	static void ObsSourceSignalHandler();

	static void OBSSourceCreated(void *param, calldata_t *calldata);
	static void OBSSourceDeleted(void *param, calldata_t *calldata);
	static void OBSSourceRenamed(void *param, calldata_t *calldata);

	// static void OBSFrontendEventHandler(enum obs_frontend_event event, void *private_data);
	// static void UpdateSceneList(CountdownWidgetStruct *context);

	static int CheckSourceType(obs_source_t *source);
	// static void AddSourceToList(CountdownWidgetStruct *context, obs_source_t *source, int sourceType);
	// static void RemoveSourceFromList(CountdownWidgetStruct *context, obs_source_t *source, int sourceType);
	// static void RenameSource(CountdownWidgetStruct *context, obs_source_t *source, int sourceType);


public slots:

	void PlayButtonClicked();
	void PauseButtonClicked();
	void ResetButtonClicked();
	void EndMessageCheckBoxSelected(int state);
	void SceneSwitchCheckBoxSelected(int state);
	void TimerDecrement();
};

#endif // COUNTDOWNWIDGET_H