#include "countdown-widget.hpp"
#include "plugin-macros.generated.h"
#include <string>
#include <iostream>

#include <QDockWidget>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

CountdownDockWidget::CountdownDockWidget(QWidget *parent)
	: QDockWidget("Countdown Timer", parent)
{
	QWidget *countdown_timer_widget = new QWidget();
	// setStyleSheet(
	// "QWidget{ background-color : rgba( 160, 160, 160, 255); border-radius : 7px;  }");

	QLabel *label = new QLabel(this);
	QPushButton *button1 = new QPushButton("Press Me!", this);
	QHBoxLayout *layout = new QHBoxLayout();
	label->setText("This is the countdown timer widget!");
	layout->addWidget(label);

	layout->addWidget(button1);
	countdown_timer_widget->setLayout(layout);
	// countdown_timer_widget->setBaseSize(200, 200);
	countdown_timer_widget->setMinimumSize(200, 200);
	countdown_timer_widget->setVisible(true);

	setWidget(countdown_timer_widget);

	// this->dockLocationChanged();
}

void CountdownDockWidget::changeEvent(QEvent *event)
{
	std::cout << event << "Event Happened!" << std::endl;
	blog("LOG_INFO", "Callback function called!");
}