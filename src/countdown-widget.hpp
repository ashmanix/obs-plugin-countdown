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

#include <string>
#include <iostream>
#include <sstream>
#include <util/base.h>
#include <vector>
#include <obs.h>
#include <obs-frontend-api.h>

class CountdownDockWidget : public QDockWidget {
	Q_OBJECT
public:
	explicit CountdownDockWidget(QWidget *parent);
	~CountdownDockWidget();

	virtual void changeEvent(QEvent *event);

private:
	enum MediaButtonType { play, pause, restart };
	static const int COUNTDOWNPERIOD = 1000;

	struct SourcesList
		{
			const char *name;
			const char *id;
		};
	struct CountdownWidgetStruct {
		bool isPlaying;

		QWidget *countdownTimerUI;
		QTimer *timer;
		QTime *time;
		QLCDNumber *timerDisplay;
		QLineEdit *timerHours;
		QLineEdit *timerMinutes;
		QLineEdit *timerSeconds;

		QPushButton *playButton;
		QPushButton *pauseButton;
		QPushButton *resetButton;

		QComboBox *textSourceDropdownList;
		std::vector<SourcesList> sourcesList;

		
	};

	CountdownWidgetStruct* countdownTimerData;
	

	QVBoxLayout* setupCountdownWidgetUI(CountdownWidgetStruct *context);
	void startTimerCounting(CountdownWidgetStruct* context);
	void stopTimerCounting(CountdownWidgetStruct* context);
	void initialiseTimerTime(CountdownWidgetStruct* context);
	QString convertTimeToDisplayString(QTime* timeToConvert);
	bool isSetTimeZero(CountdownWidgetStruct* context);
	static void getSourceList();
	static bool enumSources(void *data, obs_source_t *source);
	static void obsEventCallback(enum obs_frontend_event event, void *private_data);

public slots:
	// void mediaButtonClicked();
	void playButtonClicked();
	void pauseButtonClicked();
	void resetButtonClicked();
	void timerDecrement();
	void updateTimer(CountdownWidgetStruct* context);
};

#endif // COUNTDOWNWIDGET_H