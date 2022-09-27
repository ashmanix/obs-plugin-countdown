#include "countdown-widget.hpp"

CountdownDockWidget::CountdownDockWidget(QWidget *parent)
	: QDockWidget("Countdown Timer", parent)
{
	countdownTimerUI = new QWidget();

	QVBoxLayout* layout = setupCountdownWidgetUI();
	countdownTimerUI->setLayout(layout);

	// countdownTimerUI->setBaseSize(200, 200);
	countdownTimerUI->setMinimumSize(200, 200);
	countdownTimerUI->setVisible(true);

	setWidget(countdownTimerUI);
	
	layout = nullptr;
	// this->dockLocationChanged();
}

CountdownDockWidget::~CountdownDockWidget()
{
	delete (timerDisplay);
	delete (timerHours);
	delete (timerMinutes);
	delete (timerSeconds);

	delete (playButton);
	delete (pauseButton);
	delete (resetButton);

	delete (textSourceDropdownList);
	delete (countdownTimerUI);

}

QVBoxLayout* CountdownDockWidget::setupCountdownWidgetUI() {
	
	timerDisplay = new QLCDNumber(9);
	timerDisplay->display("00:00");

	QRegularExpression rx("[0-9]{1,2}");
	QValidator *timerValidator = new QRegularExpressionValidator(rx, this);

	QLabel *timerHoursLabel = new QLabel("h");
	timerHours = new QLineEdit("0");
	timerHours->setAlignment(Qt::AlignVCenter);
	timerHours->setMaxLength(2);
	timerHours->setValidator(timerValidator);

	QLabel *timerMinutesLabel = new QLabel("m");
	timerMinutes = new QLineEdit("0");
	timerMinutes->setAlignment(Qt::AlignVCenter);
	timerMinutes->setMaxLength(2);
	timerMinutes->setValidator(timerValidator);

	QLabel *timerSecondsLabel = new QLabel("s");
	timerSeconds = new QLineEdit("0");
	timerSeconds->setAlignment(Qt::AlignVCenter);
	timerSeconds->setMaxLength(2);
	timerSeconds->setValidator(timerValidator);

	QLabel *sourceDropdownLabel = new QLabel();
	sourceDropdownLabel->setText("Text Source: ");

	textSourceDropdownList = new QComboBox();
	textSourceDropdownList->addItem("Text 1");
	textSourceDropdownList->addItem("Text 2");
	textSourceDropdownList->addItem("Text 3");
	textSourceDropdownList->addItem("Text 4");

	// playButton = new QPushButton(QIcon("../data/images/media_play.svg"),"Start", this);
	playButton = new QPushButton(this);
	playButton->setProperty("themeID", "playIcon");
	playButton->setEnabled(true);
	QObject::connect(playButton, SIGNAL(clicked()),
			 SLOT(playButtonClicked()));

	pauseButton = new QPushButton(this);
	pauseButton->setProperty("themeID", "pauseIcon");
	pauseButton->setEnabled(false);
	QObject::connect(pauseButton, SIGNAL(clicked()),
			 SLOT(pauseButtonClicked()));

	resetButton = new QPushButton(this);
	resetButton->setProperty("themeID", "restartIcon");
	QObject::connect(resetButton, SIGNAL(clicked()),
			 SLOT(resetButtonClicked()));

	isPlaying = false;

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

	mainLayout->addWidget(timerDisplay);
	mainLayout->addLayout(timerLayout);
	mainLayout->addLayout(dropDownLayout);
	mainLayout->addLayout(buttonLayout);
	return mainLayout;
}

void CountdownDockWidget::changeEvent(QEvent *event)
{
	std::cout << event << "Event Happened!" << std::endl;
	// blog(LOG_INFO, "Callback function called!");
}

void CountdownDockWidget::playButtonClicked()
{
	setTimerIsCounting();
	QString hours = timerHours->text();
	QString minutes = timerMinutes->text();
	QString seconds = timerSeconds->text();

	QString displayTime = QString("%1:%2:%3").arg(hours, minutes, seconds);
	// std::ostringstream timerTime;
	// timerTime << hours<< ":" << minutes << ":" << seconds;
	// std::string displayTime = timerTime.str();
	timerDisplay->display(displayTime);
	blog(LOG_INFO, isPlaying ? "true" : "false");
	blog(LOG_INFO, "Play Button Clicked");
}

void CountdownDockWidget::pauseButtonClicked()
{
	setTimerNotCounting();

	blog(LOG_INFO, isPlaying ? "true" : "false");
	blog(LOG_INFO, "Pause Button Clicked");
}

void CountdownDockWidget::resetButtonClicked()
{
	setTimerNotCounting();

	blog(LOG_INFO, isPlaying ? "true" : "false");
	blog(LOG_INFO, "Reset Button Clicked");
}

void CountdownDockWidget::setTimerIsCounting()
{
	isPlaying = true;
	playButton->setEnabled(false);
	pauseButton->setEnabled(true);
	blog(LOG_INFO, "Timer STARTED counting!");
}

void CountdownDockWidget::setTimerNotCounting()
{
	isPlaying = false;
	playButton->setEnabled(true);
	pauseButton->setEnabled(false);
	blog(LOG_INFO, "Timer STOPPED counting!");
}
