#include "countdown-widget.hpp"


CountdownDockWidget::CountdownDockWidget(QWidget *parent)
	: QDockWidget("Countdown Timer", parent)
{
	QWidget *countdown_timer_widget = new QWidget();

	this->timeDisplay = new QLCDNumber(6);
	timeDisplay->display("00:00:00");

	QLabel *timerHoursLabel = new QLabel("h");
	this->timerHours = new QLineEdit("");
	timerHours->setAlignment(Qt::AlignVCenter);
	timerHours->setMaxLength(2);
	// timerHours->setPlaceholderText("H");
	timerHours->setInputMask("00");

	QLabel *timerMinutesLabel = new QLabel("m");
	this->timerMinutes = new QLineEdit("");
	timerMinutes->setAlignment(Qt::AlignVCenter);
	timerMinutes->setMaxLength(2);
	// timerMinutes->setPlaceholderText("M");
	timerMinutes->setInputMask("00");

	QLabel *timerSecondsLabel = new QLabel("s");
	this->timerSeconds = new QLineEdit("");
	timerSeconds->setAlignment(Qt::AlignVCenter);
	timerSeconds->setMaxLength(2);
	// timerSeconds->setPlaceholderText("S");
	timerSeconds->setInputMask("00");

	
	QLabel *sourceDropdownLabel = new QLabel();
	sourceDropdownLabel->setText("Text Source: ");

	this->textSourceDropdownList= new QComboBox();
	textSourceDropdownList->addItem("Text 1");
	textSourceDropdownList->addItem("Text 2");
	textSourceDropdownList->addItem("Text 3");
	textSourceDropdownList->addItem("Text 4");

	// playButton = new QPushButton(QIcon("../data/images/media_play.svg"),"Start", this);
	this->playButton = new QPushButton(this);
	this->playButton->setProperty("themeID", "playIcon");
	this->playButton->setEnabled(true);
	QObject::connect(playButton, SIGNAL(clicked()), SLOT(playButtonClicked()));

	this->pauseButton = new QPushButton(this);
	this->pauseButton->setProperty("themeID", "pauseIcon");
	this->pauseButton->setEnabled(false);
	QObject::connect(pauseButton, SIGNAL(clicked()), SLOT(pauseButtonClicked()));

	this->resetButton = new QPushButton(this);
	this->resetButton->setProperty("themeID", "restartIcon");
	QObject::connect(resetButton, SIGNAL(clicked()), SLOT(resetButtonClicked()));

	this ->isPlaying = false;	

	QHBoxLayout *timerLayout = new QHBoxLayout();

	timerLayout->addWidget(timerHours);
	timerLayout->addWidget(timerHoursLabel);
	timerLayout->addWidget(timerMinutes);
	timerLayout->addWidget(timerMinutesLabel);
	timerLayout->addWidget(timerSeconds);
	timerLayout->addWidget(timerSecondsLabel);

	QHBoxLayout *buttonLayout = new QHBoxLayout();

	buttonLayout->addWidget(resetButton);
	buttonLayout->addWidget(pauseButton);
	buttonLayout->addWidget(playButton);

	QHBoxLayout *dropDownLayout = new QHBoxLayout();
	dropDownLayout->addWidget(sourceDropdownLabel);
	dropDownLayout->addWidget(textSourceDropdownList);

	QVBoxLayout *mainLayout = new QVBoxLayout();
	
	mainLayout->addWidget(timeDisplay);
	mainLayout->addLayout(timerLayout);
	mainLayout->addLayout(dropDownLayout);
	mainLayout->addLayout(buttonLayout);

	countdown_timer_widget->setLayout(mainLayout);
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

void CountdownDockWidget::playButtonClicked() {
	this->isPlaying = false;
	this->playButton->setEnabled(false);
	this->pauseButton->setEnabled(true);
	// this->playButton->setIcon(SP_MediaPlay);
	
	blog(LOG_INFO, this->isPlaying ? "true" : "false");
	blog(LOG_INFO, "Play Button Clicked");
}

void CountdownDockWidget::pauseButtonClicked() {
	this->isPlaying = true;
	// Play button should be enabled
	this->playButton->setEnabled(true);
	this->pauseButton->setEnabled(false);

	blog(LOG_INFO, this->isPlaying ? "true" : "false");
	blog(LOG_INFO, "Pause Button Clicked");
}

void CountdownDockWidget::resetButtonClicked() {
	this->isPlaying = false;

	this->playButton->setEnabled(true);
	this->pauseButton->setEnabled(false);

	blog(LOG_INFO, "Reset Button Clicked");
}


