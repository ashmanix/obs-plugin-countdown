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

class CountdownDockWidget : public QDockWidget {
	Q_OBJECT
public:
	explicit CountdownDockWidget(QWidget *parent);
	~CountdownDockWidget();

	virtual void changeEvent(QEvent *event);

private:
	bool isPlaying;

	static const int COUNTDOWNPERIOD = 1000;

	QWidget* countdownTimerUI = nullptr;
	QTimer* timer = nullptr;
	QTime* time = nullptr;
	QLCDNumber* timerDisplay = nullptr;
	QLineEdit* timerHours = nullptr;
	QLineEdit* timerMinutes = nullptr;
	QLineEdit* timerSeconds = nullptr;

	QPushButton* playButton = nullptr;
	QPushButton* pauseButton = nullptr;
	QPushButton* resetButton = nullptr;

	QComboBox *textSourceDropdownList = nullptr;

	QVBoxLayout* setupCountdownWidgetUI();
	void startTimerCounting();
	void stopTimerCounting();
	void initialiseTimerTime();
	QString convertTimeToDisplayString(QTime* timeToConvert);

public slots:
	void playButtonClicked();
	void pauseButtonClicked();
	void resetButtonClicked();
	void timerDecrement();
	void updateTimer();
};

#endif // COUNTDOWNWIDGET_H