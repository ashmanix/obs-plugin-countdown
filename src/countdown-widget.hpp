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

		// std::vector<SourcesList> sourcesList;
		std::list<SourceListItem> textSourcesList;
		std::list<obs_source_t*>::iterator it;
		OBSSignal textSourceAddedSignals;
		OBSSignal textSourceRemovedSignals;
	};

private:
	enum MediaButtonType { play, pause, restart };
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
	void SetSelectedSource(const QString &text);
	void SetSourceText(CountdownWidgetStruct* context, const char* text);

	static void ObsSourceSignalHandler();
	static void OBSSourceCreated(void *param, calldata_t *calldata);
	static void OBSSourceLoaded(void *param, calldata_t *calldata);
	static void OBSSourceDeleted(void *param, calldata_t *calldata);
	static void OBSSourceRemoved(void *param, calldata_t *calldata);
	static void OBSSourceRenamed(void *param, calldata_t *calldata);

	static bool CheckIfTextSource(obs_source_t *source);
	static void AddTextSourceToList(CountdownWidgetStruct *context, obs_source_t *source);
	static void RemoveTextSourceFromList(CountdownWidgetStruct *context, obs_source_t *source);
	static void RenameTextSource(CountdownWidgetStruct *context, obs_source_t *source);

	

public slots:

	void PlayButtonClicked();
	void PauseButtonClicked();
	void ResetButtonClicked();
	void TimerDecrement();
};

#endif // COUNTDOWNWIDGET_H