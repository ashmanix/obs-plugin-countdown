#include "countdown-widget.hpp"


CountdownDockWidget::CountdownDockWidget(QWidget *parent)
	: QDockWidget("Countdown Timer", parent)
{
	QWidget *countdown_timer_widget = new QWidget();
	// setStyleSheet(
	// "QWidget{ background-color : rgba( 160, 160, 160, 255); border-radius : 7px;  }");

	QLabel *label = new QLabel(this);
	playButton = new QPushButton("Start", this);
	QObject::connect(playButton, SIGNAL(clicked()), SLOT(clicked()));

	this ->isPlaying = false;

	QHBoxLayout *layout = new QHBoxLayout();
	label->setText("This is the countdown timer widget!");
	layout->addWidget(label);

	layout->addWidget(playButton);
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
	// blog(LOG_INFO, "Callback function called!");
}

void CountdownDockWidget::clicked() {
	std::cout << "Button Clicked!" << std::endl;
	this->isPlaying = !isPlaying;
	if(!isPlaying){
		this->playButton->setText("Pause");
		// this->playButton->setIcon(SP_MediaPlay);
	} else {
		this->playButton->setText("Play");
	}
	
	blog(LOG_INFO, "Something");
}