#include "countdown-widget.hpp"

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
	countdown_timer_widget->setBaseSize(200, 200);

    setWidget(countdown_timer_widget);
}