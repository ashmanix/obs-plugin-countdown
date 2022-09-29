#include "countdown-widget.hpp"

CountdownDockWidget::CountdownDockWidget(QWidget *parent)
	: QDockWidget("Countdown Timer", parent)
{
	blog(LOG_INFO, "Starting Loading of widget!");
	countdownTimerData = new CountdownWidgetStruct;
	countdownTimerData->countdownTimerUI = new QWidget();
	countdownTimerData->countdownTimerUI->setLayout(
		SetupCountdownWidgetUI(countdownTimerData));

	// countdownTimerUI->setBaseSize(200, 200);
	countdownTimerData->countdownTimerUI->setMinimumSize(200, 200);
	countdownTimerData->countdownTimerUI->setVisible(true);

	setWidget(countdownTimerData->countdownTimerUI);
	// layout = nullptr;

	obs_frontend_add_event_callback(ObsEventCallback, countdownTimerData);

	ConnectObsSignalHandlers(countdownTimerData);

	InitialiseTimerTime(countdownTimerData);
	// this->dockLocationChanged();
}

CountdownDockWidget::~CountdownDockWidget()
{
	obs_frontend_remove_event_callback(ObsEventCallback,
					   countdownTimerData);
	this->destroy();
}

QVBoxLayout *CountdownDockWidget::SetupCountdownWidgetUI(
	CountdownWidgetStruct *countdownStruct)
{

	CountdownWidgetStruct *context = countdownStruct;
	context->timerDisplay = new QLCDNumber(8);
	context->timerDisplay->display("00:00:00");

	context->timerHours = new QLineEdit("0");
	context->timerHours->setAlignment(Qt::AlignCenter);
	context->timerHours->setMaxLength(2);
	context->timerHours->setValidator(new QRegularExpressionValidator(
		QRegularExpression("[0-9]{1,2}"), this));
	QObject::connect(context->timerHours, SIGNAL(textEdited(QString text)),
			 SLOT(updateHours()));

	context->timerMinutes = new QLineEdit("0");
	context->timerMinutes->setAlignment(Qt::AlignCenter);
	context->timerMinutes->setMaxLength(2);
	context->timerMinutes->setValidator(new QRegularExpressionValidator(
		QRegularExpression("^[1-5]?[0-9]"), this));
	QObject::connect(context->timerMinutes,
			 SIGNAL(textEdited(QString text)),
			 SLOT(updateMinutes()));

	context->timerSeconds = new QLineEdit("0");
	context->timerSeconds->setAlignment(Qt::AlignCenter);
	context->timerSeconds->setMaxLength(2);
	context->timerSeconds->setValidator(new QRegularExpressionValidator(
		QRegularExpression("^[1-5]?[0-9]"), this));
	QObject::connect(context->timerSeconds,
			 SIGNAL(textEdited(QString text)), SLOT(updateTimer()));

	context->textSourceDropdownList = new QComboBox();
	// context->textSourceDropdownList->addItem("Text 1");
	// context->textSourceDropdownList->addItem("Text 2");
	// context->textSourceDropdownList->addItem("Text 3");
	// context->textSourceDropdownList->addItem("Text 4");

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

void CountdownDockWidget::ChangeEvent(QEvent *event)
{
	std::cout << event << "Event Happened!" << std::endl;
	// blog(LOG_INFO, "Callback function called!");
}

void CountdownDockWidget::PlayButtonClicked()
{
	CountdownWidgetStruct *context = countdownTimerData;

	if (IsSetTimeZero(context))
		return;

	context->timerDisplay->display(
		ConvertTimeToDisplayString(context->time));
	StartTimerCounting(context);

	blog(LOG_INFO, "Play Button Clicked");
	context = nullptr;
}

void CountdownDockWidget::PauseButtonClicked()
{
	CountdownWidgetStruct *context = countdownTimerData;
	StopTimerCounting(context);
	blog(LOG_INFO, "Pause Button Clicked");

	context = nullptr;
}

void CountdownDockWidget::ResetButtonClicked()
{
	CountdownWidgetStruct *context = countdownTimerData;

	QTime *time = context->time;
	int hours = context->timerHours->text().toInt();
	int minutes = context->timerMinutes->text().toInt();
	int seconds = context->timerSeconds->text().toInt();

	StopTimerCounting(context);

	context->time->setHMS(hours, minutes, seconds, 0);
	context->timerDisplay->display(ConvertTimeToDisplayString(time));

	blog(LOG_INFO, "Reset Button Clicked");
	context = nullptr;
}

void CountdownDockWidget::StartTimerCounting(CountdownWidgetStruct *context)
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

void CountdownDockWidget::StopTimerCounting(CountdownWidgetStruct *context)
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

void CountdownDockWidget::InitialiseTimerTime(CountdownWidgetStruct *context)
{
	context->timer = new QTimer();
	QObject::connect(context->timer, SIGNAL(timeout()),
			 SLOT(timerDecrement()));
	context->time = new QTime(context->timerHours->text().toInt(),
				  context->timerMinutes->text().toInt(),
				  context->timerSeconds->text().toInt());
}

void CountdownDockWidget::TimerDecrement()
{
	CountdownWidgetStruct *context = countdownTimerData;

	QTime *time = context->time;
	QLCDNumber *timerDisplay = context->timerDisplay;

	time->setHMS(time->addMSecs(-COUNTDOWNPERIOD).hour(),
		     time->addMSecs(-COUNTDOWNPERIOD).minute(),
		     time->addMSecs(-COUNTDOWNPERIOD).second());

	timerDisplay->display(ConvertTimeToDisplayString(time));

	if (time->hour() == 0 && time->minute() == 0 && time->second() == 0) {
		timerDisplay->display("00:00:00");
		time->setHMS(0, 0, 0, 0);
		StopTimerCounting(context);
		blog(LOG_INFO, "Timer reached zero");
		return;
	}

	blog(LOG_INFO, "One second down!");

	blog(LOG_INFO, "%s", qPrintable(time->toString()));

	time = nullptr;
	timerDisplay = nullptr;
	context = nullptr;
}

QString CountdownDockWidget::ConvertTimeToDisplayString(QTime *timeToConvert)
{
	return timeToConvert->toString("hh:mm:ss");
}

void CountdownDockWidget::UpdateTimer(CountdownWidgetStruct *context)
{
	UNUSED_PARAMETER(context);
}

bool CountdownDockWidget::IsSetTimeZero(CountdownWidgetStruct *context)
{
	bool isZero = false;

	if (context->time->hour() == 0 && context->time->minute() == 0 &&
	    context->time->second() == 0) {
		isZero = true;
	} else if (context->timerHours->text().toInt() == 0 &&
		   context->timerMinutes->text().toInt() == 0 &&
		   context->timerSeconds->text().toInt() == 0) {
		isZero = true;
	}

	return isZero;
}

void CountdownDockWidget::GetSourceList()
{
	// void *sourceList;
	blog(LOG_INFO, "Getting Source List");
	obs_enum_sources(EnumSources, nullptr);
	blog(LOG_INFO, "Source checking finished!");
}

bool CountdownDockWidget::EnumSources(void *data, obs_source_t *source)
{
	CountdownWidgetStruct *context = (CountdownWidgetStruct *)data;

	// enum obs_source_type type  = obs_source_get_type(source);

	const char *source_id = obs_source_get_unversioned_id(source);

	if (CountdownDockWidget::CheckIfTextSource(source)) {

		const char *name = obs_source_get_name(source);
		const char *id = obs_source_get_id(source);

		SourceListItem listItem = {name, id};

		context->textSourcesList.push_back(listItem);
		// context->textSourceDropdownList->addItem(name, id);
		blog(LOG_INFO, "Found Text Type: %s", source_id);
	}

	return true;
}

void CountdownDockWidget::ObsEventCallback(enum obs_frontend_event event,
					   void *private_data)
{
	UNUSED_PARAMETER(private_data);

	CountdownWidgetStruct *context = (CountdownWidgetStruct *)private_data;
	UNUSED_PARAMETER(context);
	blog(LOG_INFO, "Event Triggered!");

	if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
		// newCounterDock.getSourceList();
		// blog(LOG_INFO, "Getting Source List");
		// obs_enum_sources(EnumSources, context);
		// blog(LOG_INFO, "Source checking finished!");
		// context->textSourceAddedSignals.Connect(obs_get_signal_handler(),
		// 				   "source_create",
		// 				   OBSSourceAdded, context);
		// context->textSourceRemovedSignals.Connect(obs_get_signal_handler(),
		// 				   "source_destroy",
		// 				   OBSSourceDeleted, context);
		// context->textSourcesList.sort();

		// for (const auto &sourceItem : context->textSourcesList) {
		// 	context->textSourceDropdownList->addItem(
		// 		sourceItem.name, sourceItem.id);
		// }
	}
}

void CountdownDockWidget::ConnectObsSignalHandlers(
	CountdownWidgetStruct *context)
{
	signal_handler_t *signalHandler = obs_get_signal_handler();
	if (!signalHandler)
		return;
	signal_handler_connect(signalHandler, "source_create", OBSSourceAdded,
			       context);
	signal_handler_connect(signalHandler, "source_load", OBSSourceAdded,
			       context);
	signal_handler_connect(signalHandler, "source_remove", OBSSourceDeleted,
			       context);
	signal_handler_connect(signalHandler, "source_destroy",
			       OBSSourceDeleted, context);
}

void CountdownDockWidget::OBSSourceAdded(void *param, calldata_t *calldata)
{
	auto context =
		static_cast<CountdownDockWidget::CountdownWidgetStruct *>(
			param);

	OBSSource source((obs_source_t *)calldata_ptr(calldata, "source"));
	blog(LOG_INFO, "Source Add Triggered!");

	if (!source || !CheckIfTextSource(source))
		return;

	CountdownDockWidget::AddTextSourceToList(context, source);

	// if (!obs_source_get_output_flags(source))
	// 	return;
	// obs_enum_sources(EnumSources, context);
};

void CountdownDockWidget::OBSSourceDeleted(void *param, calldata_t *calldata)
{
	auto context =
		static_cast<CountdownDockWidget::CountdownWidgetStruct *>(
			param);

	OBSSource source((obs_source_t *)calldata_ptr(calldata, "source"));

	blog(LOG_INFO, "Source Delete Triggered!");

	const char *source_id = obs_source_get_unversioned_id(source);

	UNUSED_PARAMETER(context);
	UNUSED_PARAMETER(source_id);

	// CountdownDockWidget::RemoveTextSourceFromList(context, source);

	// if (!obs_source_get_output_flags(source))
	// 	return;
	// obs_enum_sources(EnumSources, context);
};

bool CountdownDockWidget::CheckIfTextSource(obs_source_t *source)
{
	blog(LOG_INFO, "Checking if Text Source");
	const char *source_id = obs_source_get_unversioned_id(source);
	blog(LOG_INFO, "source_id: %s", source_id);
	if (strcmp(source_id, "text_ft2_source") == 0 ||
	    strcmp(source_id, "text_gdiplus") == 0) {
		return true;
	}
	return false;
}

void CountdownDockWidget::AddTextSourceToList(CountdownWidgetStruct *context,
					      obs_source_t *source)
{
	const char *name = obs_source_get_name(source);
	const char *id = obs_source_get_id(source);

	SourceListItem listItem = {name, id};

	context->textSourcesList.push_back(listItem);
	context->textSourceDropdownList->addItem(listItem.name, listItem.id);
	context->textSourcesList.sort();
}

void CountdownDockWidget::RemoveTextSourceFromList(
	CountdownWidgetStruct *context, const char *sourceId)
{
	int indexToRemove = context->textSourceDropdownList->findData(sourceId);
	context->textSourceDropdownList->removeItem(indexToRemove);
	context->textSourcesList.sort();

	blog(LOG_INFO, "ID to remove: %s", sourceId);
	blog(LOG_INFO, "Dropdown Index to remove: %i", indexToRemove);
	// blog(LOG_INFO, );
}

void CountdownDockWidget::ObsSourceSignalHandler() {}