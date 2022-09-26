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
	QPushButton *playButton;

public slots:
	void clicked();
};


#endif // COUNTDOWNWIDGET_H