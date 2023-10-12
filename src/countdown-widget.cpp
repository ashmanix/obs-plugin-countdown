#include "countdown-widget.hpp"

CountdownDockWidget::CountdownDockWidget(QWidget *parent)
	: QDockWidget("Countdown Timer", parent), ui(new Ui::CountdownTimer)
{
	countdownTimerData = new CountdownWidgetStruct;

	ui->setupUi(this);

	SetupCountdownWidgetUI(countdownTimerData);

	setVisible(false);
	setFloating(true);
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
	ui->timeDisplay->display("00:00:00");

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

	ui->countdownTypeTabWidget->setTabText(
		0, obs_module_text("SetPeriodTabLabel"));
	ui->countdownTypeTabWidget->setTabText(
		1, obs_module_text("SetTimeTabLabel"));
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
			blog(LOG_INFO, log_action " due to hotkey");           \
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
		blog(LOG_ERROR, "Error registering vendor to websocket!");
		return;
	}

#define WEBSOCKET_CALLBACK(method, log_action)                              \
	[](obs_data_t *request_data, obs_data_t *response_data,             \
	   void *incoming_data) {                                           \
		UNUSED_PARAMETER(request_data);                             \
		CountdownDockWidget &cdWidget =                             \
			*static_cast<CountdownDockWidget *>(incoming_data); \
		blog(LOG_INFO, log_action " due to websocket call");        \
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

	ui->timeDisplay->display(context->time->toString("hh:mm:ss"));
	StartTimerCounting(context);
}

void CountdownDockWidget::PauseButtonClicked()
{
	CountdownWidgetStruct *context = countdownTimerData;

	if (ui->countdownTypeTabWidget->currentIndex() == 1) {
		ui->countdownTypeTabWidget->setCurrentIndex(0);
	}

	StopTimerCounting(context);
}

void CountdownDockWidget::ResetButtonClicked()
{
	CountdownWidgetStruct *context = countdownTimerData;

	if (ui->countdownTypeTabWidget->currentIndex() == 1) {
		ui->countdownTypeTabWidget->setCurrentIndex(0);
	}

	int hours = ui->timerHours->text().toInt();
	int minutes = ui->timerMinutes->text().toInt();
	int seconds = ui->timerSeconds->text().toInt();

	StopTimerCounting(context);

	context->time->setHMS(hours, minutes, seconds);
	UpdateTimeDisplay(context->time);
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

	CountdownDockWidget::TimeIncrements timeDifference =
		CalculateTimeDifference(ui->timeEdit->time());
	context->time->setHMS(timeDifference.hours, timeDifference.minutes,
			      timeDifference.seconds,
			      timeDifference.milliseconds);

	ui->timeDisplay->display(context->time->toString("hh:mm:ss"));
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

	ui->timerHours->setEnabled(false);
	ui->hoursCheckBox->setEnabled(false);
	ui->timerMinutes->setEnabled(false);
	ui->minutesCheckBox->setEnabled(false);
	ui->timerSeconds->setEnabled(false);
	ui->secondsCheckBox->setEnabled(false);

	ui->textSourceDropdownList->setEnabled(false);
	ui->textSourceDropdownLabel->setEnabled(false);
	ui->endMessageLineEdit->setEnabled(false);
	ui->sceneSourceDropdownList->setEnabled(false);
	ui->endMessageCheckBox->setEnabled(false);
	ui->switchSceneCheckBox->setEnabled(false);

	ui->countdownTypeTabWidget->tabBar()->setEnabled(false);
	ui->timeEdit->setEnabled(false);
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

	ui->timerHours->setEnabled(true);
	ui->hoursCheckBox->setEnabled(true);
	ui->timerMinutes->setEnabled(true);
	ui->minutesCheckBox->setEnabled(true);
	ui->timerSeconds->setEnabled(true);
	ui->secondsCheckBox->setEnabled(true);

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
	ui->timeEdit->setEnabled(true);
}

void CountdownDockWidget::InitialiseTimerTime(CountdownWidgetStruct *context)
{
	context->timer = new QTimer();
	QObject::connect(context->timer, SIGNAL(timeout()),
			 SLOT(TimerDecrement()));
	context->time = new QTime(ui->timerHours->text().toInt(),
				  ui->timerMinutes->text().toInt(),
				  ui->timerSeconds->text().toInt());
}

void CountdownDockWidget::TimerDecrement()
{
	CountdownWidgetStruct *context = countdownTimerData;

	QTime *currentTime = new QTime();

	// If selected tab is h/m/s
	if (ui->countdownTypeTabWidget->currentIndex() == 0) {
		currentTime = context->time;

		currentTime->setHMS(
			currentTime->addMSecs(-COUNTDOWNPERIOD).hour(),
			currentTime->addMSecs(-COUNTDOWNPERIOD).minute(),
			currentTime->addMSecs(-COUNTDOWNPERIOD).second());
	} else {
		// We get the current time and compare it to the set time to countdown to
		CountdownDockWidget::TimeIncrements timeDifference =
			CalculateTimeDifference(ui->timeEdit->time());
		currentTime->setHMS(timeDifference.hours,
				    timeDifference.minutes,
				    timeDifference.seconds);
	}

	UpdateTimeDisplay(currentTime);

	if (currentTime->hour() == 0 && currentTime->minute() == 0 &&
	    currentTime->second() == 0) {
		QString endMessageText = ui->endMessageLineEdit->text();
		if (ui->endMessageCheckBox->isChecked()) {
			SetSourceText(endMessageText.toStdString().c_str());
		}
		if (ui->switchSceneCheckBox->isChecked()) {
			SetCurrentScene();
		}
		ui->timeDisplay->display("00:00:00");
		currentTime->setHMS(0, 0, 0, 0);
		StopTimerCounting(context);
		return;
	}
}

CountdownDockWidget::TimeIncrements
CountdownDockWidget::CalculateTimeDifference(QTime timeToCountdownTo)
{
	QTime systemTime = QTime::currentTime();
	int millisecondsDifference = systemTime.msecsTo(timeToCountdownTo);
	int hours = 0;
	int minutes = 0;
	int seconds = 0;
	int milliseconds = 0;

	millisecondsDifference =
		millisecondsDifference + 1000; // Add 1 second for countdown

	if (millisecondsDifference > 0) {
		milliseconds = (int)(millisecondsDifference % 1000);
		seconds = (int)((millisecondsDifference / 1000) % 60);
		minutes = (int)((millisecondsDifference / (1000 * 60)) % 60);
		hours = (int)((millisecondsDifference / (1000 * 60 * 60)) % 24);
	}
	return {hours, minutes, seconds, milliseconds};
}

QString CountdownDockWidget::ConvertTimeToDisplayString(QTime *timeToConvert)
{
	int hoursState = ui->hoursCheckBox->checkState();
	int minutesState = ui->minutesCheckBox->checkState();
	int secondsState = ui->secondsCheckBox->checkState();

	QString stringTime = "";
	QString timeFormat = "";

	timeFormat += hoursState ? "hh" : "";
	timeFormat += (timeFormat != "" && minutesState) ? ":" : "";
	timeFormat += minutesState ? "mm" : "";
	timeFormat += (timeFormat != "" && secondsState) ? ":" : "";
	timeFormat += secondsState ? "ss" : "";

	return (timeFormat == "") ? "Nothing selected!" : timeToConvert->toString(timeFormat);
}

void CountdownDockWidget::UpdateTimeDisplay(QTime *time)
{
	ui->timeDisplay->display(time->toString("hh:mm:ss"));
	QString formattedDisplayTime = ConvertTimeToDisplayString(time);
	// const char *timeToShow = ConvertToConstChar(formattedDisplayTime);
	// blog(LOG_INFO, "Formatted time is: %s", timeToShow);
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

	if (context->time->hour() == 0 && context->time->minute() == 0 &&
	    context->time->second() == 0) {
		isZero = true;
	} else if (ui->timerHours->text().toInt() == 0 &&
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
		// ui->timerEndLabel->setEnabled(true);
	} else {
		ui->endMessageLineEdit->setEnabled(false);
		// ui->timerEndLabel->setEnabled(false);
	}
}

void CountdownDockWidget::SceneSwitchCheckBoxSelected(int state)
{
	if (state) {
		ui->sceneSourceDropdownList->setEnabled(true);
		// ui->sceneSwitchLabel->setEnabled(true);
	} else {
		ui->sceneSourceDropdownList->setEnabled(false);
		// ui->sceneSwitchLabel->setEnabled(false);
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
		int hours = (int)obs_data_get_int(data, "hours");
		int hoursCheckBoxStatus =
			(int)obs_data_get_int(data, "hoursCheckBoxStatus");

		int minutes = (int)obs_data_get_int(data, "minutes");
		int minutesCheckBoxStatus =
			(int)obs_data_get_int(data, "minutesCheckBoxStatus");

		int seconds = (int)obs_data_get_int(data, "seconds");

		// Selections
		int secondsCheckBoxStatus =
			(int)obs_data_get_int(data, "secondsCheckBoxStatus");

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
		ui->timerHours->setText(QString::number(hours));
		ui->hoursCheckBox->setCheckState(
			(Qt::CheckState)hoursCheckBoxStatus);

		ui->timerMinutes->setText(QString::number(minutes));
		ui->minutesCheckBox->setCheckState(
			(Qt::CheckState)minutesCheckBoxStatus);

		ui->timerSeconds->setText(QString::number(seconds));
		ui->secondsCheckBox->setCheckState(
			(Qt::CheckState)secondsCheckBoxStatus);

		ui->endMessageLineEdit->setText(endMessageText);

		ui->endMessageCheckBox->setCheckState(
			(Qt::CheckState)endMessageCheckBoxStatus);

		ui->switchSceneCheckBox->setCheckState(
			(Qt::CheckState)switchSceneCheckBoxStatus);

		QTime savedTime = QTime::fromString(countdownToTime);
		ui->timeEdit->setTime(savedTime);

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

	QString countdownToTime = ui->timeEdit->time().toString();
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
		blog(LOG_INFO, "Hotkey ID: %i, Value: %s", (int)id, name);
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
