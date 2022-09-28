#include "countdown-widget.hpp"

CountdownDockWidget::CountdownDockWidget(QWidget *parent)
	: QDockWidget("Countdown Timer", parent)
{
	countdownTimerData = new CountdownWidgetStruct;
	countdownTimerData->countdownTimerUI = new QWidget();
	countdownTimerData->countdownTimerUI->setLayout(setupCountdownWidgetUI(countdownTimerData));

	// countdownTimerUI->setBaseSize(200, 200);
	countdownTimerData->countdownTimerUI->setMinimumSize(200, 200);
	countdownTimerData->countdownTimerUI->setVisible(true);

	setWidget(countdownTimerData->countdownTimerUI);
	// layout = nullptr;

	initialiseTimerTime(countdownTimerData);
	// this->dockLocationChanged();
}

CountdownDockWidget::~CountdownDockWidget()
{
	this->destroy();
}

QVBoxLayout* CountdownDockWidget::setupCountdownWidgetUI(CountdownWidgetStruct* countdownStruct) {

	CountdownWidgetStruct* context = countdownStruct;
	context->timerDisplay = new QLCDNumber(8);
	context->timerDisplay->display("00:00:00");

	context->timerHours = new QLineEdit("0");
	context->timerHours->setAlignment(Qt::AlignCenter);
	context->timerHours->setMaxLength(2);
	context->timerHours->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{1,2}"), this));
	QObject::connect(context->timerHours, SIGNAL(textEdited(QString text)), SLOT(updateHours()));

	context->timerMinutes = new QLineEdit("0");
	context->timerMinutes->setAlignment(Qt::AlignCenter);
	context->timerMinutes->setMaxLength(2);
	context->timerMinutes->setValidator(new QRegularExpressionValidator(QRegularExpression("^[1-5]?[0-9]"), this));
	QObject::connect(context->timerMinutes, SIGNAL(textEdited(QString text)), SLOT(updateMinutes()));

	context->timerSeconds = new QLineEdit("0");
	context->timerSeconds->setAlignment(Qt::AlignCenter);
	context->timerSeconds->setMaxLength(2);
	context->timerSeconds->setValidator(new QRegularExpressionValidator(QRegularExpression("^[1-5]?[0-9]"), this));
	QObject::connect(context->timerSeconds, SIGNAL(textEdited(QString text)), SLOT(updateTimer()));

	context->textSourceDropdownList = new QComboBox();
	context->textSourceDropdownList->addItem("Text 1");
	context->textSourceDropdownList->addItem("Text 2");
	context->textSourceDropdownList->addItem("Text 3");
	context->textSourceDropdownList->addItem("Text 4");

	context->playButton = new QPushButton(this);
	context->playButton->setProperty("themeID", "playIcon");
	context->playButton->setEnabled(true);
	QObject::connect(context->playButton, SIGNAL(clicked()),
			 SLOT(playButtonClicked()));

	context->pauseButton = new QPushButton(this);
	context->pauseButton->setProperty("themeID", "pauseIcon");
	context->pauseButton->setEnabled(false);
	QObject::connect(context->pauseButton, SIGNAL(clicked()),
			 SLOT(pauseButtonClicked()));

	context->resetButton = new QPushButton(this);
	context->resetButton->setProperty("themeID", "restartIcon");
	QObject::connect(context->resetButton, SIGNAL(clicked()),
			 SLOT(resetButtonClicked()));

	context->isPlaying = false;

	QHBoxLayout *timerLayout = new QHBoxLayout();

	timerLayout->addWidget(context->timerHours);
	timerLayout->addWidget(new QLabel("h"));

	timerLayout->addWidget(context->timerMinutes);
	timerLayout->addWidget(new QLabel("m"));
	
	timerLayout->addWidget(context->timerSeconds);
	timerLayout->addWidget(new QLabel("s"));

	QHBoxLayout *buttonLayout = new QHBoxLayout();

	buttonLayout->addWidget(context->resetButton);
	buttonLayout->addWidget(context->pauseButton);
	buttonLayout->addWidget(context->playButton);

	QHBoxLayout *dropDownLayout = new QHBoxLayout();
	dropDownLayout->addWidget(new QLabel("Text Source: "));
	dropDownLayout->addWidget(context->textSourceDropdownList);

	QVBoxLayout *mainLayout = new QVBoxLayout();

	mainLayout->addWidget(context->timerDisplay);
	mainLayout->addLayout(timerLayout);
	mainLayout->addLayout(dropDownLayout);
	mainLayout->addLayout(buttonLayout);

	timerLayout = nullptr;
	dropDownLayout = nullptr;
	buttonLayout = nullptr;
	context = nullptr;

	return mainLayout;
}

void CountdownDockWidget::changeEvent(QEvent *event)
{
	std::cout << event << "Event Happened!" << std::endl;
	// blog(LOG_INFO, "Callback function called!");
}


void CountdownDockWidget::playButtonClicked()
{	
	CountdownWidgetStruct* context = countdownTimerData;

	if(isSetTimeZero(context)) return;

	context->timerDisplay->display(convertTimeToDisplayString(context->time));
	startTimerCounting(context);

	blog(LOG_INFO, "Play Button Clicked");
	context = nullptr;
}

void CountdownDockWidget::pauseButtonClicked()
{
	CountdownWidgetStruct* context = countdownTimerData;
	stopTimerCounting(context);
	blog(LOG_INFO, "Pause Button Clicked");

	context = nullptr;
}

void CountdownDockWidget::resetButtonClicked()
{
	CountdownWidgetStruct* context = countdownTimerData;

	QTime *time = context->time;
	int hours = context->timerHours->text().toInt();
	int minutes = context->timerMinutes->text().toInt();
	int seconds = context->timerSeconds->text().toInt();
	
	stopTimerCounting(context);

	context->time->setHMS(hours, minutes, seconds,0);
	context->timerDisplay->display(convertTimeToDisplayString(time));

	blog(LOG_INFO, "Reset Button Clicked");
	context = nullptr;
}

void CountdownDockWidget::startTimerCounting(CountdownWidgetStruct* context)
{
	context->isPlaying = true;
	context->timer->start(COUNTDOWNPERIOD);
	context->playButton->setEnabled(false);
	context->pauseButton->setEnabled(true);
	context->resetButton->setEnabled(false);

	context->timerHours->setEnabled(false);
	context->timerMinutes->setEnabled(false);
	context->timerSeconds->setEnabled(false);

	blog(LOG_INFO, "Timer STARTED counting!");
}

void CountdownDockWidget::stopTimerCounting(CountdownWidgetStruct* context)
{
	context->isPlaying = false;
	context->timer->stop();
	context->playButton->setEnabled(true);
	context->pauseButton->setEnabled(false);
	context->resetButton->setEnabled(true);

	context->timerHours->setEnabled(true);
	context->timerMinutes->setEnabled(true);
	context->timerSeconds->setEnabled(true);

	blog(LOG_INFO, "Timer STOPPED counting!");
}

void CountdownDockWidget::initialiseTimerTime(CountdownWidgetStruct* context) {
	context->timer = new QTimer();
	QObject::connect(context->timer, SIGNAL(timeout()), SLOT(timerDecrement()));
	context->time = new QTime(context->timerHours->text().toInt(), context->timerMinutes->text().toInt(), context->timerSeconds->text().toInt());

}

void CountdownDockWidget::timerDecrement() {
	CountdownWidgetStruct* context = countdownTimerData;

	QTime* time = context->time;
	QLCDNumber* timerDisplay = context->timerDisplay;

	time->setHMS(time->addMSecs(-COUNTDOWNPERIOD).hour(), time->addMSecs(-COUNTDOWNPERIOD).minute(), 
	time->addMSecs(-COUNTDOWNPERIOD).second());

	timerDisplay->display(convertTimeToDisplayString(time));

	if(time->hour() == 0 && time->minute() == 0 && time->second() == 0) {
		timerDisplay->display("00:00:00");
		time->setHMS(0,0,0,0);
		stopTimerCounting(context);
		blog(LOG_INFO, "Timer reached zero");
		return;
	}

	blog(LOG_INFO, "One second down!");

	blog(LOG_INFO, "%s", qPrintable(time->toString()));

	time = nullptr;
	timerDisplay = nullptr;
	context = nullptr;
}

QString CountdownDockWidget::convertTimeToDisplayString(QTime* timeToConvert) {
	return timeToConvert->toString("hh:mm:ss");
}

void CountdownDockWidget::updateTimer(CountdownWidgetStruct* context) {
	UNUSED_PARAMETER(context);
}

bool CountdownDockWidget::isSetTimeZero(CountdownWidgetStruct* context) {
	bool isZero = false;

	if(context->time->hour() == 0 && context->time->minute() == 0 && context->time->second() == 0) {
		isZero = true;
	} else if(context->timerHours->text().toInt() == 0 && context->timerMinutes->text().toInt() == 0 && context->timerSeconds->text().toInt() == 0) {
		isZero = true;
	}

	return isZero;
}
