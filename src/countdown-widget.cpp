#include "countdown-widget.hpp"

const char CountdownDockWidget::ZEROSTRING[] = "00:00:00:00";

CountdownDockWidget::CountdownDockWidget(QWidget *parent)
	: QWidget(parent),
	  ui(new Ui::CountdownTimer)
{
	countdownTimerData = new CountdownWidgetStruct;

	ui->setupUi(this);

	SetupCountdownWidgetUI(countdownTimerData);

	// setVisible(false);
	// setFloating(true);
	resize(300, 380);

	obs_frontend_add_event_callback(OBSFrontendEventHandler, ui);

	ConnectUISignalHandlers();

	ConnectObsSignalHandlers();

	InitialiseTimerTime(countdownTimerData);

	RegisterHotkeys(countdownTimerData);
}

CountdownDockWidget::~CountdownDockWidget()
{
	SaveSettings();
	UnregisterHotkeys();
}

void CountdownDockWidget::SetupCountdownWidgetUI(
	CountdownWidgetStruct *countdownStruct)
{

	CountdownWidgetStruct *context = countdownStruct;
	ui->timeDisplay->display(CountdownDockWidget::ZEROSTRING);

	ui->dateTimeEdit->setMinimumDate(QDate::currentDate());
	ui->dateTimeEdit->setMaximumDate(QDate::currentDate().addDays(999));

	ui->daysCheckBox->setText(obs_module_text("DaysCheckboxLabel"));
	ui->daysCheckBox->setCheckState(Qt::Checked);
	ui->daysCheckBox->setToolTip(obs_module_text("DaysCheckBoxTip"));
	ui->timerDays->setValidator(new QRegularExpressionValidator(
		QRegularExpression("^(0|[1-9]\\d{0,2})$"), this));

	ui->hoursCheckBox->setText(obs_module_text("HoursCheckboxLabel"));
	ui->hoursCheckBox->setCheckState(Qt::Checked);
	ui->hoursCheckBox->setToolTip(obs_module_text("HoursCheckBoxTip"));
	ui->timerHours->setMaxLength(2);
	ui->timerHours->setValidator(new QRegularExpressionValidator(
		QRegularExpression("^(0?[0-9]|1[0-9]|2[0-3])$"), this));

	ui->minutesCheckBox->setText(obs_module_text("MinutesCheckboxLabel"));
	ui->minutesCheckBox->setCheckState(Qt::Checked);
	ui->minutesCheckBox->setToolTip(obs_module_text("MinutesCheckBoxTip"));
	ui->timerMinutes->setMaxLength(2);
	ui->timerMinutes->setValidator(new QRegularExpressionValidator(
		QRegularExpression("^[1-5]?[0-9]"), this));

	ui->secondsCheckBox->setText(obs_module_text("SecondsCheckboxLabel"));
	ui->secondsCheckBox->setCheckState(Qt::Checked);
	ui->secondsCheckBox->setToolTip(obs_module_text("SecondsCheckBoxTip"));
	ui->timerSeconds->setAlignment(Qt::AlignCenter);
	ui->timerSeconds->setMaxLength(2);
	ui->timerSeconds->setValidator(new QRegularExpressionValidator(
		QRegularExpression("^[1-5]?[0-9]"), this));

	ui->leadZeroCheckBox->setText(obs_module_text("LeadZeroCheckboxLabel"));
	ui->leadZeroCheckBox->setCheckState(Qt::Checked);
	ui->leadZeroCheckBox->setToolTip(
		obs_module_text("LeadZeroCheckBoxTip"));

	ui->countdownTypeTabWidget->setTabText(
		0, obs_module_text("SetPeriodTabLabel"));
	ui->countdownTypeTabWidget->setTabText(
		1, obs_module_text("SetDatetimeTabLabel"));
	ui->countdownTypeTabWidget->setToolTip(
		obs_module_text("SetCountdownTypeTip"));

	ui->textSourceDropdownList->setToolTip(
		obs_module_text("TextSourceDropdownTip"));
	ui->textSourceDropdownLabel->setText(
		obs_module_text("TextSourceLabel"));

	ui->endMessageCheckBox->setCheckState(Qt::Unchecked);
	ui->endMessageCheckBox->setToolTip(
		obs_module_text("EndMessageCheckBoxTip"));
	ui->endMessageCheckBox->setText(obs_module_text("EndMessageLabel"));
	// ui->timerEndLabel->setEnabled(false);
	ui->endMessageLineEdit->setEnabled(false);
	ui->endMessageLineEdit->setToolTip(
		obs_module_text("EndMessageLineEditTip"));

	ui->switchSceneCheckBox->setCheckState(Qt::Unchecked);
	ui->switchSceneCheckBox->setToolTip(
		obs_module_text("SwitchSceneCheckBoxTip"));
	ui->switchSceneCheckBox->setText(obs_module_text("SwitchScene"));
	// ui->sceneSwitchLabel->setEnabled(false);
	ui->sceneSourceDropdownList->setEnabled(false);
	ui->sceneSourceDropdownList->setToolTip(
		obs_module_text("SceneSourceDropdownTip"));

	ui->playButton->setProperty("themeID", "playIcon");
	ui->playButton->setEnabled(true);
	ui->playButton->setToolTip(obs_module_text("PlayButtonTip"));
	ui->pauseButton->setProperty("themeID", "pauseIcon");
	ui->pauseButton->setEnabled(false);
	ui->pauseButton->setToolTip(obs_module_text("PauseButtonTip"));
	ui->resetButton->setProperty("themeID", "restartIcon");
	ui->resetButton->setToolTip(obs_module_text("ResetButtonTip"));

	ui->toTimePlayButton->setProperty("themeID", "playIcon");
	ui->toTimePlayButton->setEnabled(true);
	ui->toTimePlayButton->setToolTip(
		obs_module_text("ToTimePlayButtonTip"));
	ui->toTimeStopButton->setProperty("themeID", "stopIcon");
	ui->toTimeStopButton->setEnabled(false);
	ui->toTimeStopButton->setToolTip(
		obs_module_text("ToTimeStopButtonTip"));

	context->isPlaying = false;
}

void CountdownDockWidget::ConnectUISignalHandlers()
{
	QObject::connect(ui->switchSceneCheckBox, SIGNAL(stateChanged(int)),
			 SLOT(SceneSwitchCheckBoxSelected(int)));

	QObject::connect(ui->playButton, SIGNAL(clicked()),
			 SLOT(PlayButtonClicked()));

	QObject::connect(ui->pauseButton, SIGNAL(clicked()),
			 SLOT(PauseButtonClicked()));

	QObject::connect(ui->resetButton, SIGNAL(clicked()),
			 SLOT(ResetButtonClicked()));

	QObject::connect(ui->toTimePlayButton, SIGNAL(clicked()),
			 SLOT(ToTimePlayButtonClicked()));

	QObject::connect(ui->toTimeStopButton, SIGNAL(clicked()),
			 SLOT(ToTimeStopButtonClicked()));

	QObject::connect(ui->endMessageCheckBox, SIGNAL(stateChanged(int)),
			 SLOT(EndMessageCheckBoxSelected(int)));

	QObject::connect(ui->textSourceDropdownList,
			 SIGNAL(currentTextChanged(QString)),
			 SLOT(HandleTextSourceChange(QString)));

	QObject::connect(ui->sceneSourceDropdownList,
			 SIGNAL(currentTextChanged(QString)),
			 SLOT(HandleSceneSourceChange(QString)));
}

void CountdownDockWidget::RegisterHotkeys(CountdownWidgetStruct *context)
{
	auto LoadHotkey = [](obs_data_t *s_data, obs_hotkey_id id,
			     const char *name) {
		if ((int)id == -1)
			return;

		OBSDataArrayAutoRelease array =
			obs_data_get_array(s_data, name);

		obs_hotkey_load(id, array);
		// obs_data_array_release(array);
	};

	char *file = obs_module_config_path(CONFIG);
	obs_data_t *saved_data = nullptr;
	if (file) {
		saved_data = obs_data_create_from_json_file(file);
		bfree(file);
	}

#define HOTKEY_CALLBACK(pred, method, log_action)                              \
	[](void *incoming_data, obs_hotkey_id, obs_hotkey_t *, bool pressed) { \
		Ui::CountdownTimer &countdownUi =                              \
			*static_cast<Ui::CountdownTimer *>(incoming_data);     \
		if ((pred) && pressed) {                                       \
			obs_log(LOG_INFO, log_action " due to hotkey");        \
			method();                                              \
		}                                                              \
	}
	// Register Play Hotkey
	context->startCountdownHotkeyId = (int)obs_hotkey_register_frontend(
		"Ashmanix_Countdown_Timer_Start",
		obs_module_text("StartCountdownHotkeyDescription"),
		HOTKEY_CALLBACK(true, countdownUi.playButton->animateClick,
				"Play Button Pressed"),
		ui);
	if (saved_data)
		LoadHotkey(saved_data, context->startCountdownHotkeyId,
			   "Ashmanix_Countdown_Timer_Start");

	// Register Pause Hotkey
	context->pauseCountdownHotkeyId = (int)obs_hotkey_register_frontend(
		"Ashmanix_Countdown_Timer_Pause",
		obs_module_text("PauseCountdownHotkeyDescription"),
		HOTKEY_CALLBACK(true, countdownUi.pauseButton->animateClick,
				"Pause Button Pressed"),
		ui);
	if (saved_data)
		LoadHotkey(saved_data, context->pauseCountdownHotkeyId,
			   "Ashmanix_Countdown_Timer_Pause");

	// Register Reset Hotkey
	context->setCountdownHotkeyId = (int)obs_hotkey_register_frontend(
		"Ashmanix_Countdown_Timer_Set",
		obs_module_text("SetCountdownHotkeyDescription"),
		HOTKEY_CALLBACK(true, countdownUi.resetButton->animateClick,
				"Set Button Pressed"),
		ui);
	if (saved_data)
		LoadHotkey(saved_data, context->setCountdownHotkeyId,
			   "Ashmanix_Countdown_Timer_Set");

	// Register To Time Start Hotkey
	context->startCountdownToTimeHotkeyId =
		(int)obs_hotkey_register_frontend(
			"Ashmanix_Countdown_Timer_To_Time_Start",
			obs_module_text(
				"StartCountdownToTimeHotkeyDescription"),
			HOTKEY_CALLBACK(
				true,
				countdownUi.toTimePlayButton->animateClick,
				"To Time Start Button Pressed"),
			ui);
	if (saved_data)
		LoadHotkey(saved_data, context->startCountdownToTimeHotkeyId,
			   "Ashmanix_Countdown_Timer_To_Time_Start");

	// Register To Time Stop Hotkey
	context->stopCountdownToTimeHotkeyId = (int)obs_hotkey_register_frontend(
		"Ashmanix_Countdown_Timer_To_Time_Stop",
		obs_module_text("StopCountdownToTimeHotkeyDescription"),
		HOTKEY_CALLBACK(true,
				countdownUi.toTimeStopButton->animateClick,
				"To Time Stop Button Pressed"),
		ui);
	if (saved_data)
		LoadHotkey(saved_data, context->stopCountdownToTimeHotkeyId,
			   "Ashmanix_Countdown_Timer_To_Time_Stop");

	obs_data_release(saved_data);
#undef HOTKEY_CALLBACK
}

void CountdownDockWidget::ConfigureWebSocketConnection()
{
	vendor = obs_websocket_register_vendor("ashmanix-countdown-timer");

	if (!vendor) {
		obs_log(LOG_ERROR, "Error registering vendor to websocket!");
		return;
	}

#define WEBSOCKET_CALLBACK(method, log_action)                              \
	[](obs_data_t *request_data, obs_data_t *response_data,             \
	   void *incoming_data) {                                           \
		UNUSED_PARAMETER(request_data);                             \
		CountdownDockWidget &cdWidget =                             \
			*static_cast<CountdownDockWidget *>(incoming_data); \
		obs_log(LOG_INFO, log_action " due to websocket call");     \
		method();                                                   \
		obs_data_set_bool(response_data, "success", true);          \
	}

	obs_websocket_vendor_register_request(
		vendor, "period_play",
		WEBSOCKET_CALLBACK(cdWidget.ui->playButton->click,
				   "Period play button pressed"),
		this);
	obs_websocket_vendor_register_request(
		vendor, "period_pause",
		WEBSOCKET_CALLBACK(cdWidget.ui->pauseButton->click,
				   "Period pause button pressed"),
		this);
	obs_websocket_vendor_register_request(
		vendor, "period_set",
		WEBSOCKET_CALLBACK(cdWidget.ui->resetButton->click,
				   "Period Set button pressed"),
		this);

	obs_websocket_vendor_register_request(
		vendor, "to_time_play",
		WEBSOCKET_CALLBACK(cdWidget.ui->toTimePlayButton->click,
				   "To time play button pressed"),
		this);
	obs_websocket_vendor_register_request(
		vendor, "to_time_stop",
		WEBSOCKET_CALLBACK(cdWidget.ui->toTimeStopButton->click,
				   "To time stop button pressed"),
		this);

	obs_websocket_vendor_register_request(
		vendor, "get_timer_state",
		[](obs_data_t *request_data, obs_data_t *response_data,
		   void *priv_data) {
			UNUSED_PARAMETER(request_data);
			CountdownDockWidget *self =
				static_cast<CountdownDockWidget *>(priv_data);

			obs_data_set_bool(response_data, "is_running",
					  self->countdownTimerData->isPlaying);
			obs_data_set_int(
				response_data, "time_left_ms",
				self->countdownTimerData->timeLeftInMillis);

			obs_data_set_bool(response_data, "success", true);
		},
		this);

#undef WEBSOCKET_CALLBACK
}

void CountdownDockWidget::UnregisterHotkeys()
{
	if (countdownTimerData->startCountdownHotkeyId)
		obs_hotkey_unregister(
			countdownTimerData->startCountdownHotkeyId);
	if (countdownTimerData->pauseCountdownHotkeyId)
		obs_hotkey_unregister(
			countdownTimerData->pauseCountdownHotkeyId);
	if (countdownTimerData->setCountdownHotkeyId)
		obs_hotkey_unregister(countdownTimerData->setCountdownHotkeyId);

	if (countdownTimerData->startCountdownToTimeHotkeyId)
		obs_hotkey_unregister(
			countdownTimerData->startCountdownToTimeHotkeyId);
	if (countdownTimerData->stopCountdownToTimeHotkeyId)
		obs_hotkey_unregister(
			countdownTimerData->stopCountdownToTimeHotkeyId);
}

void CountdownDockWidget::PlayButtonClicked()
{
	CountdownWidgetStruct *context = countdownTimerData;

	if (ui->countdownTypeTabWidget->currentIndex() == 1) {
		ui->countdownTypeTabWidget->setCurrentIndex(0);
	}

	if (IsSetTimeZero(context))
		return;

	ui->timeDisplay->display(
		ConvertMillisToDateTimeString(context->timeLeftInMillis));
	StartTimerCounting(context);
}

void CountdownDockWidget::PauseButtonClicked()
{
	CountdownWidgetStruct *context = countdownTimerData;

	if (ui->countdownTypeTabWidget->currentIndex() == 1) {
		ui->countdownTypeTabWidget->setCurrentIndex(0);
	}

	StopTimerCounting(context);
	SendTimerStateEvent("paused");
}

void CountdownDockWidget::ResetButtonClicked()
{
	CountdownWidgetStruct *context = countdownTimerData;

	if (ui->countdownTypeTabWidget->currentIndex() == 1) {
		ui->countdownTypeTabWidget->setCurrentIndex(0);
	}

	StopTimerCounting(context);

	context->timeLeftInMillis = GetMillisFromPeriodUI();
	UpdateDateTimeDisplay(context->timeLeftInMillis);
}

long long CountdownDockWidget::GetMillisFromPeriodUI()
{
	long long days_ms =
		static_cast<long long>(ui->timerDays->text().toInt()) * 24 *
		60 * 60 * 1000;
	long long hours_ms =
		static_cast<long long>(ui->timerHours->text().toInt()) * 60 *
		60 * 1000;
	long long minutes_ms =
		static_cast<long long>(ui->timerMinutes->text().toInt()) * 60 *
		1000;
	long long seconds_ms =
		static_cast<long long>(ui->timerSeconds->text().toInt()) * 1000;

	return days_ms + hours_ms + minutes_ms + seconds_ms;
}

void CountdownDockWidget::ToTimeStopButtonClicked()
{
	CountdownWidgetStruct *context = countdownTimerData;

	if (ui->countdownTypeTabWidget->currentIndex() == 0) {
		ui->countdownTypeTabWidget->setCurrentIndex(1);
	}

	StopTimerCounting(context);
}

void CountdownDockWidget::ToTimePlayButtonClicked()
{
	CountdownWidgetStruct *context = countdownTimerData;

	if (ui->countdownTypeTabWidget->currentIndex() == 0) {
		ui->countdownTypeTabWidget->setCurrentIndex(1);
	}

	context->timeLeftInMillis =
		CalculateDateTimeDifference(ui->dateTimeEdit->dateTime());

	ui->timeDisplay->display(
		ConvertMillisToDateTimeString(context->timeLeftInMillis));
	StartTimerCounting(context);
}

void CountdownDockWidget::StartTimerCounting(CountdownWidgetStruct *context)
{
	context->isPlaying = true;
	context->timer->start(COUNTDOWNPERIOD);
	ui->playButton->setEnabled(false);
	ui->pauseButton->setEnabled(true);
	ui->resetButton->setEnabled(false);

	ui->toTimePlayButton->setEnabled(false);
	ui->toTimeStopButton->setEnabled(true);

	ui->timerDays->setEnabled(false);
	ui->daysCheckBox->setEnabled(false);
	ui->timerHours->setEnabled(false);
	ui->hoursCheckBox->setEnabled(false);
	ui->timerMinutes->setEnabled(false);
	ui->minutesCheckBox->setEnabled(false);
	ui->timerSeconds->setEnabled(false);
	ui->secondsCheckBox->setEnabled(false);
	ui->leadZeroCheckBox->setEnabled(false);

	ui->textSourceDropdownList->setEnabled(false);
	ui->textSourceDropdownLabel->setEnabled(false);
	ui->endMessageLineEdit->setEnabled(false);
	ui->sceneSourceDropdownList->setEnabled(false);
	ui->endMessageCheckBox->setEnabled(false);
	ui->switchSceneCheckBox->setEnabled(false);

	ui->countdownTypeTabWidget->tabBar()->setEnabled(false);
	ui->dateTimeEdit->setEnabled(false);

	SendTimerStateEvent("started");
}

void CountdownDockWidget::StopTimerCounting(CountdownWidgetStruct *context)
{
	context->isPlaying = false;
	context->timer->stop();
	ui->playButton->setEnabled(true);
	ui->pauseButton->setEnabled(false);
	ui->resetButton->setEnabled(true);

	ui->toTimePlayButton->setEnabled(true);
	ui->toTimeStopButton->setEnabled(false);

	ui->timerDays->setEnabled(true);
	ui->daysCheckBox->setEnabled(true);
	ui->timerHours->setEnabled(true);
	ui->hoursCheckBox->setEnabled(true);
	ui->timerMinutes->setEnabled(true);
	ui->minutesCheckBox->setEnabled(true);
	ui->timerSeconds->setEnabled(true);
	ui->secondsCheckBox->setEnabled(true);
	ui->leadZeroCheckBox->setEnabled(true);

	ui->textSourceDropdownList->setEnabled(true);
	ui->textSourceDropdownLabel->setEnabled(true);

	ui->endMessageCheckBox->setEnabled(true);
	if (ui->endMessageCheckBox->isChecked()) {
		ui->endMessageLineEdit->setEnabled(true);
	}
	ui->switchSceneCheckBox->setEnabled(true);
	if (ui->switchSceneCheckBox->isChecked()) {
		ui->sceneSourceDropdownList->setEnabled(true);
	}

	ui->countdownTypeTabWidget->tabBar()->setEnabled(true);
	ui->dateTimeEdit->setEnabled(true);

	SendTimerStateEvent("stopped");
}

void CountdownDockWidget::InitialiseTimerTime(CountdownWidgetStruct *context)
{
	context->timer = new QTimer();
	QObject::connect(context->timer, SIGNAL(timeout()),
			 SLOT(TimerDecrement()));

	context->timeLeftInMillis = GetMillisFromPeriodUI();
}

void CountdownDockWidget::TimerDecrement()
{
	CountdownWidgetStruct *context = countdownTimerData;

	// If selected tab is period
	if (ui->countdownTypeTabWidget->currentIndex() == 0) {
		context->timeLeftInMillis -= COUNTDOWNPERIOD;
	} else {
		// We get the current time and compare it to the set time to countdown to
		context->timeLeftInMillis = CalculateDateTimeDifference(
			ui->dateTimeEdit->dateTime());
	}

	UpdateDateTimeDisplay(context->timeLeftInMillis);

	// Send tick event
	SendTimerTickEvent(context->timeLeftInMillis);

	if (context->timeLeftInMillis < COUNTDOWNPERIOD) {
		QString endMessageText = ui->endMessageLineEdit->text();
		if (ui->endMessageCheckBox->isChecked()) {
			SetSourceText(endMessageText.toStdString().c_str());
		}
		if (ui->switchSceneCheckBox->isChecked()) {
			SetCurrentScene();
		}
		ui->timeDisplay->display(CountdownDockWidget::ZEROSTRING);
		context->timeLeftInMillis = 0;
		// Send completion event
		SendTimerStateEvent("completed");
		StopTimerCounting(context);
		return;
	}
}

long long
CountdownDockWidget::CalculateDateTimeDifference(QDateTime timeToCountdownTo)
{
	QDateTime systemTime = QDateTime::currentDateTime().toUTC();
	long long millisecondsDifference =
		systemTime.msecsTo(timeToCountdownTo.toUTC());
	long long millisResult = 0;

	millisecondsDifference =
		millisecondsDifference + 1000; // Add 1 second for countdown

	obs_log(LOG_INFO, "System Time: %s",
		systemTime.toString().toUtf8().constData());
	obs_log(LOG_INFO, "System Time: %lld", systemTime.toMSecsSinceEpoch());
	obs_log(LOG_INFO, "Time To Count To: %s",
		timeToCountdownTo.toString().toUtf8().constData());
	obs_log(LOG_INFO, "Time To Count To: %lld",
		timeToCountdownTo.toMSecsSinceEpoch());
	obs_log(LOG_INFO, "Time Difference: %lld", millisecondsDifference);

	if (millisecondsDifference > 0) {
		millisResult = millisecondsDifference;
	}

	return millisResult;
}

QString CountdownDockWidget::ConvertDateTimeToFormattedDisplayString(
	long long timeInMillis, bool showLeadingZero)
{
	int daysState = ui->daysCheckBox->checkState();
	int hoursState = ui->hoursCheckBox->checkState();
	int minutesState = ui->minutesCheckBox->checkState();
	int secondsState = ui->secondsCheckBox->checkState();

	long long days = timeInMillis / (24 * 60 * 60 * 1000);
	long long remainingMilliseconds = timeInMillis % (24 * 60 * 60 * 1000);

	QTime time = QTime::fromMSecsSinceStartOfDay(
		static_cast<int>(remainingMilliseconds));

	QString formattedDateTimeString = "";

	bool isFirstField = true;

	auto appendField = [&](long long value, int state) {
		if (state) {
			if (isFirstField && !showLeadingZero) {
				// Append without leading zero
				formattedDateTimeString +=
					QString::number(value);
			} else {
				// Append with leading zero
				formattedDateTimeString += QString("%1").arg(
					value, 2, 10, QChar('0'));
			}
			isFirstField = false;
		}
	};

	appendField(days, daysState);
	if (!formattedDateTimeString.isEmpty() && hoursState)
		formattedDateTimeString += ":";
	appendField(time.hour(), hoursState);
	if (!formattedDateTimeString.isEmpty() && minutesState)
		formattedDateTimeString += ":";
	appendField(time.minute(), minutesState);
	if (!formattedDateTimeString.isEmpty() && secondsState)
		formattedDateTimeString += ":";
	appendField(time.second(), secondsState);

	return (formattedDateTimeString == "") ? "Nothing selected!"
					       : formattedDateTimeString;
}

QString
CountdownDockWidget::ConvertMillisToDateTimeString(long long timeInMillis)
{
	long long days = timeInMillis / (24 * 60 * 60 * 1000);
	long long remainingMilliseconds = timeInMillis % (24 * 60 * 60 * 1000);

	QTime time = QTime::fromMSecsSinceStartOfDay(
		static_cast<int>(remainingMilliseconds));

	return QString("%1:%2:%3:%4")
		.arg(days, 2, 10, QChar('0'))        // Days with leading zeros
		.arg(time.hour(), 2, 10, QChar('0')) // Hours with leading zeros
		.arg(time.minute(), 2, 10,
		     QChar('0')) // Minutes with leading zeros
		.arg(time.second(), 2, 10,
		     QChar('0')); // Seconds with leading zeros
}

void CountdownDockWidget::UpdateDateTimeDisplay(long long timeInMillis)
{
	ui->timeDisplay->display(ConvertMillisToDateTimeString(timeInMillis));
	QString formattedDisplayTime = ConvertDateTimeToFormattedDisplayString(
		timeInMillis, ui->leadZeroCheckBox->checkState());
	SetSourceText(formattedDisplayTime);
}

void CountdownDockWidget::SetSourceText(QString newText)
{

	QString currentSourceNameString =
		ui->textSourceDropdownList->currentText();

	obs_source_t *selectedSource = obs_get_source_by_name(
		currentSourceNameString.toStdString().c_str());

	if (selectedSource != NULL) {
		obs_data_t *sourceSettings =
			obs_source_get_settings(selectedSource);
		obs_data_set_string(sourceSettings, "text",
				    newText.toStdString().c_str());
		obs_source_update(selectedSource, sourceSettings);
		obs_data_release(sourceSettings);
		obs_source_release(selectedSource);
	}
}

bool CountdownDockWidget::IsSetTimeZero(CountdownWidgetStruct *context)
{
	bool isZero = false;

	if (context->timeLeftInMillis == 0) {
		isZero = true;
	} else if (ui->timerDays->text().toInt() == 0 &&
		   ui->timerHours->text().toInt() == 0 &&
		   ui->timerMinutes->text().toInt() == 0 &&
		   ui->timerSeconds->text().toInt() == 0) {
		isZero = true;
	}

	return isZero;
}

void CountdownDockWidget::OBSFrontendEventHandler(enum obs_frontend_event event,
						  void *private_data)
{

	Ui::CountdownTimer *ui = (Ui::CountdownTimer *)private_data;

	switch (event) {
	case OBS_FRONTEND_EVENT_FINISHED_LOADING: {
		// CountdownDockWidget::ConnectUISignalHandlers(context);
		CountdownDockWidget::LoadSavedSettings(ui);
	} break;
	default:
		break;
	}
}

void CountdownDockWidget::ConnectObsSignalHandlers()
{
	// Source Signals
	signal_handler_connect(obs_get_signal_handler(), "source_create",
			       OBSSourceCreated, ui);

	signal_handler_connect(obs_get_signal_handler(), "source_destroy",
			       OBSSourceDeleted, ui);

	signal_handler_connect(obs_get_signal_handler(), "source_rename",
			       OBSSourceRenamed, ui);
}

void CountdownDockWidget::OBSSourceCreated(void *param, calldata_t *calldata)
{
	auto ui = static_cast<Ui::CountdownTimer *>(param);
	obs_source_t *source;
	calldata_get_ptr(calldata, "source", &source);

	if (!source)
		return;
	int sourceType = CheckSourceType(source);
	// If not sourceType we need;
	if (!sourceType)
		return;

	const char *name = obs_source_get_name(source);

	if (sourceType == TEXT_SOURCE) {
		ui->textSourceDropdownList->addItem(name);
	} else if (sourceType == SCENE_SOURCE) {
		ui->sceneSourceDropdownList->addItem(name);
	}
};

void CountdownDockWidget::OBSSourceDeleted(void *param, calldata_t *calldata)
{
	auto ui = static_cast<Ui::CountdownTimer *>(param);

	obs_source_t *source;

	calldata_get_ptr(calldata, "source", &source);

	if (!source)
		return;
	int sourceType = CheckSourceType(source);
	// If not sourceType we need;
	if (!sourceType)
		return;

	const char *name = obs_source_get_name(source);

	if (sourceType == TEXT_SOURCE) {
		int textIndexToRemove =
			ui->textSourceDropdownList->findText(name);
		ui->textSourceDropdownList->removeItem(textIndexToRemove);
	} else if (sourceType == SCENE_SOURCE) {
		int sceneIndexToRemove =
			ui->sceneSourceDropdownList->findText(name);
		ui->sceneSourceDropdownList->removeItem(sceneIndexToRemove);
	}
};

void CountdownDockWidget::OBSSourceRenamed(void *param, calldata_t *calldata)
{
	auto ui = static_cast<Ui::CountdownTimer *>(param);

	obs_source_t *source;
	calldata_get_ptr(calldata, "source", &source);

	if (!source)
		return;
	int sourceType = CheckSourceType(source);
	// If not sourceType we need;
	if (!sourceType)
		return;

	const char *newName = calldata_string(calldata, "new_name");
	const char *oldName = calldata_string(calldata, "prev_name");

	if (sourceType == TEXT_SOURCE) {
		int textListIndex =
			ui->textSourceDropdownList->findText(oldName);
		if (textListIndex == -1)
			return;
		ui->textSourceDropdownList->setItemText(textListIndex, newName);
	} else if (sourceType == SCENE_SOURCE) {
		int sceneListIndex =
			ui->sceneSourceDropdownList->findText(oldName);
		if (sceneListIndex == -1)
			return;
		ui->sceneSourceDropdownList->setItemText(sceneListIndex,
							 newName);
	}
};

int CountdownDockWidget::CheckSourceType(obs_source_t *source)
{
	const char *source_id = obs_source_get_unversioned_id(source);
	if (strcmp(source_id, "text_ft2_source") == 0 ||
	    strcmp(source_id, "text_gdiplus") == 0 ||
	    strcmp(source_id, "text_pango_source") == 0) {
		return TEXT_SOURCE;
	} else if (strcmp(source_id, "scene") == 0) {
		return SCENE_SOURCE;
	}
	return 0;
}

void CountdownDockWidget::EndMessageCheckBoxSelected(int state)
{
	if (state) {
		ui->endMessageLineEdit->setEnabled(true);
	} else {
		ui->endMessageLineEdit->setEnabled(false);
	}
}

void CountdownDockWidget::SceneSwitchCheckBoxSelected(int state)
{
	if (state) {
		ui->sceneSourceDropdownList->setEnabled(true);
	} else {
		ui->sceneSourceDropdownList->setEnabled(false);
	}
}

void CountdownDockWidget::SetCurrentScene()
{
	QString selectedScene = ui->sceneSourceDropdownList->currentText();
	if (selectedScene.length()) {
		obs_source_t *source = obs_get_source_by_name(
			selectedScene.toStdString().c_str());
		if (source != NULL) {
			obs_frontend_set_current_scene(source);
			obs_source_release(source);
		}
	}
}

void CountdownDockWidget::LoadSavedSettings(Ui::CountdownTimer *ui)
{
	char *file = obs_module_config_path(CONFIG);
	obs_data_t *data = nullptr;
	if (file) {
		data = obs_data_create_from_json_file(file);
		bfree(file);
	}
	if (data) {
		// Get Save Data

		// Time
		int days = (int)obs_data_get_int(data, "days");
		int daysCheckBoxStatus =
			(int)obs_data_get_int(data, "daysCheckBoxStatus");

		int hours = (int)obs_data_get_int(data, "hours");
		int hoursCheckBoxStatus =
			(int)obs_data_get_int(data, "hoursCheckBoxStatus");

		int minutes = (int)obs_data_get_int(data, "minutes");
		int minutesCheckBoxStatus =
			(int)obs_data_get_int(data, "minutesCheckBoxStatus");

		int seconds = (int)obs_data_get_int(data, "seconds");

		int secondsCheckBoxStatus =
			(int)obs_data_get_int(data, "secondsCheckBoxStatus");

		int leadZeroCheckBoxStatus =
			(int)obs_data_get_int(data, "leadZeroCheckBoxStatus");

		// Selections
		const char *selectedTextSource =
			obs_data_get_string(data, "selectedTextSource");

		int endMessageCheckBoxStatus =
			(int)obs_data_get_int(data, "endMessageCheckBoxStatus");

		const char *endMessageText =
			obs_data_get_string(data, "endMessageText");

		int switchSceneCheckBoxStatus = (int)obs_data_get_int(
			data, "switchSceneCheckBoxStatus");

		const char *selectedSceneSource =
			obs_data_get_string(data, "selectedSceneSource");

		const char *countdownToTime =
			obs_data_get_string(data, "countdownToTime");

		int selectedTimerTabIndex =
			(int)obs_data_get_int(data, "selectedTimerTabIndex");

		UNUSED_PARAMETER(selectedTextSource);
		UNUSED_PARAMETER(selectedSceneSource);

		// Apply saved data to plugin
		ui->timerDays->setText(QString::number(days));
		ui->daysCheckBox->setCheckState(
			(Qt::CheckState)daysCheckBoxStatus);

		ui->timerHours->setText(QString::number(hours));
		ui->hoursCheckBox->setCheckState(
			(Qt::CheckState)hoursCheckBoxStatus);

		ui->timerMinutes->setText(QString::number(minutes));
		ui->minutesCheckBox->setCheckState(
			(Qt::CheckState)minutesCheckBoxStatus);

		ui->timerSeconds->setText(QString::number(seconds));
		ui->secondsCheckBox->setCheckState(
			(Qt::CheckState)secondsCheckBoxStatus);

		ui->leadZeroCheckBox->setCheckState(
			(Qt::CheckState)leadZeroCheckBoxStatus);

		ui->endMessageLineEdit->setText(endMessageText);

		ui->endMessageCheckBox->setCheckState(
			(Qt::CheckState)endMessageCheckBoxStatus);

		ui->switchSceneCheckBox->setCheckState(
			(Qt::CheckState)switchSceneCheckBoxStatus);

		QDateTime savedTime = QDateTime::fromString(countdownToTime);
		ui->dateTimeEdit->setDateTime(savedTime);

		int textSelectIndex = ui->textSourceDropdownList->findText(
			selectedTextSource);
		if (textSelectIndex != -1)
			ui->textSourceDropdownList->setCurrentIndex(
				textSelectIndex);

		int sceneSelectIndex = ui->sceneSourceDropdownList->findText(
			selectedSceneSource);
		if (sceneSelectIndex != -1)
			ui->sceneSourceDropdownList->setCurrentIndex(
				sceneSelectIndex);
		if (selectedTimerTabIndex != -1)
			ui->countdownTypeTabWidget->setCurrentIndex(
				selectedTimerTabIndex);

		obs_data_release(data);
	}
}

void CountdownDockWidget::SaveSettings()
{
	CountdownWidgetStruct *context = countdownTimerData;

	obs_data_t *obsData = obs_data_create();

	int days = ui->timerDays->text().toInt();
	obs_data_set_int(obsData, "days", days);
	int daysCheckBoxStatus = ui->daysCheckBox->checkState();
	obs_data_set_int(obsData, "daysCheckBoxStatus", daysCheckBoxStatus);

	int hours = ui->timerHours->text().toInt();
	obs_data_set_int(obsData, "hours", hours);
	int hoursCheckBoxStatus = ui->hoursCheckBox->checkState();
	obs_data_set_int(obsData, "hoursCheckBoxStatus", hoursCheckBoxStatus);

	int minutes = ui->timerMinutes->text().toInt();
	obs_data_set_int(obsData, "minutes", minutes);
	int minutesCheckBoxStatus = ui->minutesCheckBox->checkState();
	obs_data_set_int(obsData, "minutesCheckBoxStatus",
			 minutesCheckBoxStatus);

	int seconds = ui->timerSeconds->text().toInt();
	obs_data_set_int(obsData, "seconds", seconds);
	int secondsCheckBoxStatus = ui->secondsCheckBox->checkState();
	obs_data_set_int(obsData, "secondsCheckBoxStatus",
			 secondsCheckBoxStatus);

	int leadZeroCheckBoxStatus = ui->leadZeroCheckBox->checkState();
	obs_data_set_int(obsData, "leadZeroCheckBoxStatus",
			 leadZeroCheckBoxStatus);

	obs_data_set_string(obsData, "selectedTextSource",
			    context->textSourceNameText.c_str());

	int endMessageCheckBoxStatus = ui->endMessageCheckBox->checkState();
	obs_data_set_int(obsData, "endMessageCheckBoxStatus",
			 endMessageCheckBoxStatus);

	QString endMessageLineEdit = ui->endMessageLineEdit->text();
	obs_data_set_string(obsData, "endMessageText",
			    endMessageLineEdit.toStdString().c_str());

	int switchSceneCheckBoxStatus = ui->switchSceneCheckBox->checkState();
	obs_data_set_int(obsData, "switchSceneCheckBoxStatus",
			 switchSceneCheckBoxStatus);

	obs_data_set_string(obsData, "selectedSceneSource",
			    context->sceneSourceNameText.c_str());

	QString countdownToTime = ui->dateTimeEdit->dateTime().toString();
	obs_data_set_string(obsData, "countdownToTime",
			    countdownToTime.toStdString().c_str());

	int selectedTimerTabIndex = ui->countdownTypeTabWidget->currentIndex();
	if (selectedTimerTabIndex != -1) {
		obs_data_set_int(obsData, "selectedTimerTabIndex",
				 selectedTimerTabIndex);
	};

	obs_data_set_int(obsData, "endMessageCheckBoxStatus",
			 endMessageCheckBoxStatus);

	// Hotkeys
	auto SaveHotkey = [](obs_data_t *sv_data, obs_hotkey_id id,
			     const char *name) {
		obs_log(LOG_INFO, "Hotkey ID: %i, Value: %s", (int)id, name);
		if ((int)id == -1)
			return;
		OBSDataArrayAutoRelease array = obs_hotkey_save(id);
		obs_data_set_array(sv_data, name, array);
	};

	obs_data_array_t *start_countdown_hotkey_save_array =
		obs_hotkey_save(context->startCountdownHotkeyId);
	obs_data_set_array(obsData, "Ashmanix_Countdown_Timer_Start",
			   start_countdown_hotkey_save_array);
	obs_data_array_release(start_countdown_hotkey_save_array);

	obs_data_array_t *pause_countdown_hotkey_save_array =
		obs_hotkey_save(context->pauseCountdownHotkeyId);
	obs_data_set_array(obsData, "Ashmanix_Countdown_Timer_Pause",
			   pause_countdown_hotkey_save_array);
	obs_data_array_release(pause_countdown_hotkey_save_array);

	obs_data_array_t *set_countdown_hotkey_save_array =
		obs_hotkey_save(context->setCountdownHotkeyId);
	obs_data_set_array(obsData, "Ashmanix_Countdown_Timer_Set",
			   set_countdown_hotkey_save_array);
	obs_data_array_release(set_countdown_hotkey_save_array);

	SaveHotkey(obsData, context->startCountdownToTimeHotkeyId,
		   "Ashmanix_Countdown_Timer_To_Time_Start");
	SaveHotkey(obsData, context->stopCountdownToTimeHotkeyId,
		   "Ashmanix_Countdown_Timer_To_Time_Stop");

	// obs_data_array_t *start_to_time_countdown_hotkey_save_array =
	// 	obs_hotkey_save(context->startCountdownToTimeHotkeyId);
	// obs_data_set_array(obsData, "Ashmanix_Countdown_Timer_To_Time_Start",
	// 		   start_to_time_countdown_hotkey_save_array);
	// obs_data_array_release(start_to_time_countdown_hotkey_save_array);

	// obs_data_array_t *stop_to_time_countdown_hotkey_save_array =
	// 	obs_hotkey_save(context->stopCountdownToTimeHotkeyId);
	// obs_data_set_array(obsData, "Ashmanix_Countdown_Timer_To_Time_Stop",
	// 		   stop_to_time_countdown_hotkey_save_array);
	// obs_data_array_release(stop_to_time_countdown_hotkey_save_array);

	char *file = obs_module_config_path(CONFIG);
	if (!obs_data_save_json(obsData, file)) {
		char *path = obs_module_config_path("");
		if (path) {
			os_mkdirs(path);
			bfree(path);
		}
		obs_data_save_json(obsData, file);
	}
	obs_data_release(obsData);
	bfree(file);
	deleteLater();
}

const char *CountdownDockWidget::ConvertToConstChar(QString value)
{
	QByteArray ba = value.toLocal8Bit();
	const char *cString = ba.data();
	return cString;
}

void CountdownDockWidget::HandleTextSourceChange(QString newText)
{
	std::string textSourceSelected = newText.toStdString();
	countdownTimerData->textSourceNameText = textSourceSelected;
}

void CountdownDockWidget::HandleSceneSourceChange(QString newText)
{
	std::string sceneSourceSelected = newText.toStdString();
	countdownTimerData->sceneSourceNameText = sceneSourceSelected;
}

void CountdownDockWidget::SendWebsocketEvent(const char *eventName,
					     obs_data_t *eventData)
{
	if (!vendor)
		return;

	obs_websocket_vendor_emit_event(vendor, eventName, eventData);
}

void CountdownDockWidget::SendTimerTickEvent(long long timeLeftInMillis)
{
	obs_data_t *eventData = obs_data_create();

	// Convert milliseconds to readable format
	QString timeString = ConvertDateTimeToFormattedDisplayString(
		timeLeftInMillis, ui->leadZeroCheckBox->checkState());

	obs_data_set_string(eventData, "time_display",
			    timeString.toStdString().c_str());
	obs_data_set_int(eventData, "time_left_ms", timeLeftInMillis);

	SendWebsocketEvent("timer_tick", eventData);
	obs_data_release(eventData);
}

void CountdownDockWidget::SendTimerStateEvent(const char *state)
{
	obs_data_t *eventData = obs_data_create();
	obs_data_set_string(eventData, "state", state);

	if (ui->textSourceDropdownList->currentText().length() > 0) {
		obs_data_set_string(eventData, "text_source",
				    ui->textSourceDropdownList->currentText()
					    .toStdString()
					    .c_str());
	}

	SendWebsocketEvent("timer_state_changed", eventData);
	obs_data_release(eventData);
}
