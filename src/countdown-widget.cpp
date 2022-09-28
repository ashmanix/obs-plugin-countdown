#include "countdown-widget.hpp"

CountdownDockWidget::CountdownDockWidget(QWidget *parent)
	: QDockWidget("Countdown Timer", parent)
{
	countdownTimerUI = new QWidget();
	countdownTimerUI->setLayout(setupCountdownWidgetUI());

	// countdownTimerUI->setBaseSize(200, 200);
	countdownTimerUI->setMinimumSize(200, 200);
	countdownTimerUI->setVisible(true);

	setWidget(countdownTimerUI);
	countdownTimerUI = nullptr;
	// layout = nullptr;

	initialiseTimerTime();
	// this->dockLocationChanged();
}

CountdownDockWidget::~CountdownDockWidget()
{
	this->destroy();
}

QVBoxLayout* CountdownDockWidget::setupCountdownWidgetUI() {
	
	timerDisplay = new QLCDNumber(9);
	timerDisplay->display("00:00:00");

	timerHours = new QLineEdit("0");
	timerHours->setAlignment(Qt::AlignCenter);
	timerHours->setMaxLength(2);
	timerHours->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{1,2}"), this));
	QObject::connect(timerHours, SIGNAL(changeEvent()), SLOT(updateTimer()));

	timerMinutes = new QLineEdit("0");
	timerMinutes->setAlignment(Qt::AlignCenter);
	timerMinutes->setMaxLength(2);
	timerMinutes->setValidator(new QRegularExpressionValidator(QRegularExpression("^[1-5]?[0-9]"), this));
	QObject::connect(timerMinutes, SIGNAL(changeEvent()), SLOT(updateTimer()));

	timerSeconds = new QLineEdit("0");
	timerSeconds->setAlignment(Qt::AlignCenter);
	timerSeconds->setMaxLength(2);
	timerSeconds->setValidator(new QRegularExpressionValidator(QRegularExpression("^[1-5]?[0-9]"), this));
	QObject::connect(timerSeconds, SIGNAL(changeEvent()), SLOT(updateTimer()));

	textSourceDropdownList = new QComboBox();
	textSourceDropdownList->addItem("Text 1");
	textSourceDropdownList->addItem("Text 2");
	textSourceDropdownList->addItem("Text 3");
	textSourceDropdownList->addItem("Text 4");

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
	timerLayout->addWidget(new QLabel("h"));

	timerLayout->addWidget(timerMinutes);
	timerLayout->addWidget(new QLabel("m"));
	
	timerLayout->addWidget(timerSeconds);
	timerLayout->addWidget(new QLabel("s"));

	QHBoxLayout *buttonLayout = new QHBoxLayout();

	buttonLayout->addWidget(resetButton);
	buttonLayout->addWidget(pauseButton);
	buttonLayout->addWidget(playButton);

	QHBoxLayout *dropDownLayout = new QHBoxLayout();
	dropDownLayout->addWidget(new QLabel("Text Source: "));
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
	if(timer)
	startTimerCounting();
	// QString hours = timerHours->text().rightJustified(2, '0');
	// QString minutes = timerMinutes->text().rightJustified(2, '0');
	// QString seconds = timerSeconds->text().rightJustified(2, '0');
	// std::ostringstream timerTime;
	// timerTime << hours<< ":" << minutes << ":" << seconds;
	// std::string displayTime = timerTime.str();
	// timerDisplay->display(QString("%1:%2:%3").arg(hours).arg(minutes).arg(seconds));
	blog(LOG_INFO, isPlaying ? "true" : "false");
	blog(LOG_INFO, "Play Button Clicked");
}

void CountdownDockWidget::pauseButtonClicked()
{
	stopTimerCounting();

	blog(LOG_INFO, isPlaying ? "true" : "false");
	blog(LOG_INFO, "Pause Button Clicked");
}

void CountdownDockWidget::resetButtonClicked()
{
	stopTimerCounting();
	time->setHMS(timerHours->text().toInt(), timerMinutes->text().toInt(), timerSeconds->text().toInt(),0);
	timerDisplay->display(convertTimeToDisplayString(time));

	blog(LOG_INFO, isPlaying ? "true" : "false");
	blog(LOG_INFO, "Reset Button Clicked");
}

void CountdownDockWidget::startTimerCounting()
{
	isPlaying = true;
	timer->start(COUNTDOWNPERIOD);
	playButton->setEnabled(false);
	pauseButton->setEnabled(true);
	resetButton->setEnabled(false);

	timerHours->setEnabled(false);
	timerMinutes->setEnabled(false);
	timerSeconds->setEnabled(false);

	blog(LOG_INFO, "Timer STARTED counting!");
}

void CountdownDockWidget::stopTimerCounting()
{
	isPlaying = false;
	timer->stop();
	playButton->setEnabled(true);
	pauseButton->setEnabled(false);
	resetButton->setEnabled(true);

	timerHours->setEnabled(true);
	timerMinutes->setEnabled(true);
	timerSeconds->setEnabled(true);

	blog(LOG_INFO, "Timer STOPPED counting!");
}

void CountdownDockWidget::initialiseTimerTime() {
	timer = new QTimer();
	QObject::connect(timer, SIGNAL(timeout()), SLOT(timerDecrement()));
	time = new QTime(timerHours->text().toInt(), timerMinutes->text().toInt(), timerSeconds->text().toInt());

}

void CountdownDockWidget::timerDecrement() {

	if(time->hour() == 0 && time->minute() == 0 && time->second() == 0) {
		timerDisplay->display("00:00:00");
		time->setHMS(0,0,0,0);
		stopTimerCounting();
		blog(LOG_INFO, "Timer reached zero");
		return;
	}
	
	blog(LOG_INFO, "One second down!");

	time->setHMS(time->addMSecs(-COUNTDOWNPERIOD).hour(), time->addMSecs(-COUNTDOWNPERIOD).minute(), 
	time->addMSecs(-COUNTDOWNPERIOD).second());

	blog(LOG_INFO, "%s", qPrintable(time->toString()));
	timerDisplay->display(convertTimeToDisplayString(time));
}

QString CountdownDockWidget::convertTimeToDisplayString(QTime* timeToConvert) {
	return timeToConvert->toString("hh:mm:ss");
}