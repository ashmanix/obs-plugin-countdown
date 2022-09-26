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
#include <QIcon>

#include <string>
#include <iostream>
#include <util/base.h>

class CountdownDockWidget : public QDockWidget {
	Q_OBJECT
public:
	explicit CountdownDockWidget(QWidget *parent);
	virtual void changeEvent(QEvent *event);

private:
	bool isPlaying;
	QLCDNumber *timeDisplay;
	QLineEdit *timerHours;
	QLineEdit *timerMinutes;
	QLineEdit *timerSeconds;

	QPushButton *playButton;
	QPushButton *pauseButton;
	QPushButton *resetButton;

	QComboBox *textSourceDropdownList;

public slots:
	void playButtonClicked();
	void pauseButtonClicked();
	void resetButtonClicked();
};


#endif // COUNTDOWNWIDGET_H