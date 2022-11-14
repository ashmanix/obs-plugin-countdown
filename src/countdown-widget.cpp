#include "countdown-widget.hpp"

CountdownDockWidget::CountdownDockWidget(QWidget *parent)
	: QDockWidget("Countdown Timer", parent), ui(new Ui::CountdownTimer)
{
	countdownTimerData = new CountdownWidgetStruct;
	countdownTimerData->countdownTimerUI = new QWidget();
	countdownTimerData->countdownTimerUI->setLayout(
		SetupCountdownWidgetUI(countdownTimerData));

	setWidget(countdownTimerData->countdownTimerUI);
	setFeatures(QDockWidget::DockWidgetClosable |
		    QDockWidget::DockWidgetMovable |
		    QDockWidget::DockWidgetFloatable);

	ui->setupUi(this);

	setVisible(false);
	setFloating(true);
	resize(300, 380);

	obs_frontend_add_event_callback(OBSFrontendEventHandler,
					countdownTimerData);

	ConnectUISignalHandlers(countdownTimerData);

	ConnectObsSignalHandlers(countdownTimerData);

	InitialiseTimerTime(countdownTimerData);

	RegisterHotkeys(countdownTimerData);
}

CountdownDockWidget::~CountdownDockWidget()
{
	SaveSettings();
	UnregisterHotkeys();
}

QVBoxLayout *CountdownDockWidget::SetupCountdownWidgetUI(
	CountdownWidgetStruct *countdownStruct)
{

	CountdownWidgetStruct *context = countdownStruct;
	context->timeDisplay = new QLCDNumber(8);
	context->timeDisplay->display("00:00:00");

	context->hoursCheckBox = new QCheckBox();
	context->hoursCheckBox->setCheckState(Qt::Checked);
	context->hoursCheckBox->setToolTip(obs_module_text("HoursCheckBoxTip"));
	context->timerHours = new QLineEdit("0");
	context->timerHours->setAlignment(Qt::AlignCenter);
	context->timerHours->setMaxLength(2);
	context->timerHours->setValidator(new QRegularExpressionValidator(
		QRegularExpression("^[0-2]?[0-3]"), this));

	context->minutesCheckBox = new QCheckBox();
	context->minutesCheckBox->setCheckState(Qt::Checked);
	context->minutesCheckBox->setToolTip(
		obs_module_text("MinutesCheckBoxTip"));
	context->timerMinutes = new QLineEdit("0");
	context->timerMinutes->setAlignment(Qt::AlignCenter);
	context->timerMinutes->setMaxLength(2);
	context->timerMinutes->setValidator(new QRegularExpressionValidator(
		QRegularExpression("^[1-5]?[0-9]"), this));

	context->secondsCheckBox = new QCheckBox();
	context->secondsCheckBox->setCheckState(Qt::Checked);
	context->secondsCheckBox->setToolTip(
		obs_module_text("SecondsCheckBoxTip"));
	context->timerSeconds = new QLineEdit("0");
	context->timerSeconds->setAlignment(Qt::AlignCenter);
	context->timerSeconds->setMaxLength(2);
	context->timerSeconds->setValidator(new QRegularExpressionValidator(
		QRegularExpression("^[1-5]?[0-9]"), this));

	QHBoxLayout *timerLayout = new QHBoxLayout();
	timerLayout->addWidget(context->hoursCheckBox);
	timerLayout->addWidget(context->timerHours);
	timerLayout->addWidget(new QLabel("h"));
	timerLayout->addWidget(context->minutesCheckBox);
	timerLayout->addWidget(context->timerMinutes);
	timerLayout->addWidget(new QLabel("m"));
	timerLayout->addWidget(context->secondsCheckBox);
	timerLayout->addWidget(context->timerSeconds);
	timerLayout->addWidget(new QLabel("s"));

	QVBoxLayout *timeLayout = new QVBoxLayout();
	timeLayout->addWidget(context->timeDisplay);
	timeLayout->addLayout(timerLayout);

	context->textSourceDropdownList = new QComboBox();
	context->textSourceDropdownList->setToolTip(
		obs_module_text("TextSourceDropdownTip"));

	QHBoxLayout *sourceDropDownLayout = new QHBoxLayout();
	sourceDropDownLayout->addWidget(
		new QLabel(obs_module_text("TextSourceLabel")));
	sourceDropDownLayout->addWidget(context->textSourceDropdownList);
	sourceDropDownLayout->setStretch(1, 1);

	context->endMessageCheckBox = new QCheckBox();
	context->endMessageCheckBox->setCheckState(Qt::Unchecked);
	context->endMessageCheckBox->setToolTip(
		obs_module_text("EndMessageCheckBoxTip"));
	context->timerEndLabel = new QLabel(obs_module_text("EndMessageLabel"));
	context->timerEndLabel->setEnabled(false);
	context->endMessageLineEdit = new QLineEdit();
	context->endMessageLineEdit->setEnabled(false);
	context->endMessageLineEdit->setToolTip(
		obs_module_text("EndMessageLineEditTip"));

	QHBoxLayout *endMessageLayout = new QHBoxLayout();
	endMessageLayout->addWidget(context->endMessageCheckBox);
	endMessageLayout->addWidget(context->timerEndLabel);
	endMessageLayout->addWidget(context->endMessageLineEdit);

	context->switchSceneCheckBox = new QCheckBox();
	context->switchSceneCheckBox->setCheckState(Qt::Unchecked);
	context->switchSceneCheckBox->setToolTip(
		obs_module_text("SwitchSceneCheckBoxTip"));
	context->sceneSwitchLabel = new QLabel(obs_module_text("SwitchScene"));
	context->sceneSwitchLabel->setEnabled(false);
	context->sceneSourceDropdownList = new QComboBox();
	context->sceneSourceDropdownList->setEnabled(false);
	context->sceneSourceDropdownList->setToolTip(
		obs_module_text("SceneSourceDropdownTip"));

	QHBoxLayout *sceneDropDownLayout = new QHBoxLayout();
	sceneDropDownLayout->addWidget(context->switchSceneCheckBox);
	sceneDropDownLayout->addWidget(context->sceneSwitchLabel);
	sceneDropDownLayout->addWidget(context->sceneSourceDropdownList);
	sceneDropDownLayout->setEnabled(false);
	sceneDropDownLayout->setStretch(2, 1);

	context->playButton = new QPushButton(this);
	context->playButton->setProperty("themeID", "playIcon");
	context->playButton->setEnabled(true);
	context->playButton->setToolTip(obs_module_text("PlayButtonTip"));
	context->pauseButton = new QPushButton(this);
	context->pauseButton->setProperty("themeID", "pauseIcon");
	context->pauseButton->setEnabled(false);
	context->pauseButton->setToolTip(obs_module_text("PauseButtonTip"));
	context->resetButton = new QPushButton(this);
	context->resetButton->setProperty("themeID", "restartIcon");
	context->resetButton->setToolTip(obs_module_text("ResetButtonTip"));

	QHBoxLayout *buttonLayout = new QHBoxLayout();
	buttonLayout->addWidget(context->resetButton);
	buttonLayout->addWidget(context->pauseButton);
	buttonLayout->addWidget(context->playButton);

	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->addLayout(timeLayout);
	mainLayout->addLayout(sourceDropDownLayout);
	mainLayout->addLayout(endMessageLayout);
	mainLayout->addLayout(sceneDropDownLayout);
	mainLayout->addLayout(buttonLayout);

	context->isPlaying = false;

	return mainLayout;
}

void CountdownDockWidget::ConnectUISignalHandlers(CountdownWidgetStruct *context)
{
	QObject::connect(context->switchSceneCheckBox,
			 SIGNAL(stateChanged(int)),
			 SLOT(SceneSwitchCheckBoxSelected(int)));

	QObject::connect(context->playButton, SIGNAL(clicked()),
			 SLOT(PlayButtonClicked()));

	QObject::connect(context->pauseButton, SIGNAL(clicked()),
			 SLOT(PauseButtonClicked()));

	QObject::connect(context->resetButton, SIGNAL(clicked()),
			 SLOT(ResetButtonClicked()));

	QObject::connect(context->endMessageCheckBox, SIGNAL(stateChanged(int)),
			 SLOT(EndMessageCheckBoxSelected(int)));

	QObject::connect(context->textSourceDropdownList,
			 SIGNAL(currentTextChanged(QString)),
			 SLOT(HandleTextSourceChange(QString)));

	QObject::connect(context->sceneSourceDropdownList,
			 SIGNAL(currentTextChanged(QString)),
			 SLOT(HandleSceneSourceChange(QString)));

	QObject::connect(context->hoursCheckBox, SIGNAL(stateChanged(int)),
			 SLOT(ToggleHoursCheckBoxSelected(int)));

	QObject::connect(context->minutesCheckBox, SIGNAL(stateChanged(int)),
			 SLOT(ToggleMinutesCheckBoxSelected(int)));

	QObject::connect(context->secondsCheckBox, SIGNAL(stateChanged(int)),
			 SLOT(ToggleSecondsCheckBoxSelected(int)));
}

void CountdownDockWidget::RegisterHotkeys(CountdownWidgetStruct *context)
{
	auto LoadHotkey = [](obs_data_t *s_data, obs_hotkey_id id,
			     const char *name) {
		OBSDataArrayAutoRelease array =
			obs_data_get_array(s_data, name);

		obs_hotkey_load(id, array);
		obs_data_array_release(array);
	};

	char *file = obs_module_config_path(CONFIG);
	obs_data_t *saved_data = nullptr;
	if (file) {
		saved_data = obs_data_create_from_json_file(file);
		bfree(file);
	}

#define HOTKEY_CALLBACK(pred, method, log_action)                     \
	[](void *incoming_data, obs_hotkey_id, obs_hotkey_t *, bool pressed) { \
		CountdownWidgetStruct &countdownData =                \
			*static_cast<CountdownWidgetStruct *>(incoming_data);  \
		if ((pred) && pressed) {                              \
			blog(LOG_INFO, log_action " due to hotkey");  \
			method();                                     \
		}                                                     \
	}

	// Register Play Hotkey
	context->startCountdownHotkeyId = (int)obs_hotkey_register_frontend(
		"Ashmanix_Countdown_Timer_Start",
		obs_module_text("StartCountdownHotkeyDecription"),
		HOTKEY_CALLBACK(true, countdownData.playButton->animateClick,
				"Play Button Pressed"),
		context);
	if (saved_data)
		LoadHotkey(saved_data, context->startCountdownHotkeyId,
			   "Ashmanix_Countdown_Timer_Start");

	// Register Pause Hotkey
	context->pauseCountdownHotkeyId = (int)obs_hotkey_register_frontend(
		"Ashmanix_Countdown_Timer_Pause",
		obs_module_text("PauseCountdownHotkeyDecription"),
		HOTKEY_CALLBACK(true, countdownData.pauseButton->animateClick,
				"Pause Button Pressed"),
		context);
	if (saved_data)
		LoadHotkey(saved_data, context->pauseCountdownHotkeyId,
			   "Ashmanix_Countdown_Timer_Pause");

	// Register Reset Hotkey
	context->setCountdownHotkeyId = (int)obs_hotkey_register_frontend(
		"Ashmanix_Countdown_Timer_Set",
		obs_module_text("SetCountdownHotkeyDecription"),
		HOTKEY_CALLBACK(true, countdownData.resetButton->animateClick,
				"Set Button Pressed"),
		context);
	if (saved_data)
		LoadHotkey(saved_data, context->setCountdownHotkeyId,
			   "Ashmanix_Countdown_Timer_Set");

	obs_data_release(saved_data);
#undef HOTKEY_CALLBACK
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
}

void CountdownDockWidget::PlayButtonClicked()
{
	CountdownWidgetStruct *context = countdownTimerData;
	if (IsSetTimeZero(context))
		return;

	context->timeDisplay->display(context->time->toString("hh:mm:ss"));
	StartTimerCounting(context);
}

void CountdownDockWidget::PauseButtonClicked()
{
	CountdownWidgetStruct *context = countdownTimerData;
	StopTimerCounting(context);
}

void CountdownDockWidget::ResetButtonClicked()
{
	CountdownWidgetStruct *context = countdownTimerData;
	int hours = context->timerHours->text().toInt();
	int minutes = context->timerMinutes->text().toInt();
	int seconds = context->timerSeconds->text().toInt();

	StopTimerCounting(context);

	context->time->setHMS(hours, minutes, seconds, 0);

	UpdateTimeDisplay(context, context->time);
}

void CountdownDockWidget::StartTimerCounting(CountdownWidgetStruct *context)
{
	context->isPlaying = true;
	context->timer->start(COUNTDOWNPERIOD);
	context->playButton->setEnabled(false);
	context->pauseButton->setEnabled(true);
	context->resetButton->setEnabled(false);

	context->timerHours->setEnabled(false);
	context->hoursCheckBox->setEnabled(false);
	context->timerMinutes->setEnabled(false);
	context->minutesCheckBox->setEnabled(false);
	context->timerSeconds->setEnabled(false);
	context->secondsCheckBox->setEnabled(false);

	context->textSourceDropdownList->setEnabled(false);
	context->endMessageLineEdit->setEnabled(false);
	context->sceneSourceDropdownList->setEnabled(false);
	context->endMessageCheckBox->setEnabled(false);
	context->switchSceneCheckBox->setEnabled(false);
}

void CountdownDockWidget::StopTimerCounting(CountdownWidgetStruct *context)
{
	context->isPlaying = false;
	context->timer->stop();
	context->playButton->setEnabled(true);
	context->pauseButton->setEnabled(false);
	context->resetButton->setEnabled(true);

	context->timerHours->setEnabled(true);
	context->hoursCheckBox->setEnabled(true);
	context->timerMinutes->setEnabled(true);
	context->minutesCheckBox->setEnabled(true);
	context->timerSeconds->setEnabled(true);
	context->secondsCheckBox->setEnabled(true);

	context->textSourceDropdownList->setEnabled(true);

	context->endMessageCheckBox->setEnabled(true);
	if (context->endMessageCheckBox->isChecked()) {
		context->endMessageLineEdit->setEnabled(true);
	}
	context->switchSceneCheckBox->setEnabled(true);
	if (context->switchSceneCheckBox->isChecked()) {
		context->sceneSourceDropdownList->setEnabled(true);
	}
}

void CountdownDockWidget::InitialiseTimerTime(CountdownWidgetStruct *context)
{
	context->timer = new QTimer();
	QObject::connect(context->timer, SIGNAL(timeout()),
			 SLOT(TimerDecrement()));
	context->time = new QTime(context->timerHours->text().toInt(),
				  context->timerMinutes->text().toInt(),
				  context->timerSeconds->text().toInt());
}

void CountdownDockWidget::TimerDecrement()
{
	CountdownWidgetStruct *context = countdownTimerData;

	QTime *currentTime = context->time;

	currentTime->setHMS(currentTime->addMSecs(-COUNTDOWNPERIOD).hour(),
			    currentTime->addMSecs(-COUNTDOWNPERIOD).minute(),
			    currentTime->addMSecs(-COUNTDOWNPERIOD).second());

	UpdateTimeDisplay(context, currentTime);

	if (currentTime->hour() == 0 && currentTime->minute() == 0 &&
	    currentTime->second() == 0) {
		QString endMessageText = context->endMessageLineEdit->text();
		if (context->endMessageCheckBox->isChecked()) {
			SetSourceText(context,
				      endMessageText.toStdString().c_str());
		}
		if (context->switchSceneCheckBox->isChecked()) {
			SetCurrentScene();
		}
		context->timeDisplay->display("00:00:00");
		currentTime->setHMS(0, 0, 0, 0);
		StopTimerCounting(context);
		return;
	}
}

QString CountdownDockWidget::ConvertTimeToDisplayString(QTime *timeToConvert)
{
	int hoursState = countdownTimerData->hoursCheckBox->checkState();
	int minutesState = countdownTimerData->minutesCheckBox->checkState();
	int secondsState = countdownTimerData->secondsCheckBox->checkState();

	QString stringTime = "";

	if (hoursState && minutesState & secondsState) {
		stringTime = timeToConvert->toString("hh:mm:ss");
	} else if (!hoursState && minutesState && secondsState) {
		stringTime = timeToConvert->toString("mm:ss");
	} else if (!hoursState && !minutesState && secondsState) {
		stringTime = timeToConvert->toString("ss");
	} else if (!hoursState && minutesState && !secondsState) {
		stringTime = timeToConvert->toString("mm");
	} else if (hoursState && !minutesState && !secondsState) {
		stringTime = timeToConvert->toString("hh");
	} else if (hoursState && !minutesState && secondsState) {
		stringTime = timeToConvert->toString("hh:ss");
	} else if (hoursState && minutesState && !secondsState) {
		stringTime = timeToConvert->toString("hh:mm");
	} else if (!hoursState && !minutesState && !secondsState) {
		stringTime = "Nothing selected!";
	}

	return stringTime;
}

void CountdownDockWidget::UpdateTimeDisplay(CountdownWidgetStruct *context,
					    QTime *time)
{
	context->timeDisplay->display(time->toString("hh:mm:ss"));
	QString formattedDisplayTime = ConvertTimeToDisplayString(time);
	// const char *timeToShow = ConvertToConstChar(formattedDisplayTime);
	// blog(LOG_INFO, "Formatted time is: %s", timeToShow);
	SetSourceText(context, formattedDisplayTime);
}

void CountdownDockWidget::SetSourceText(CountdownWidgetStruct *context,
					QString newText)
{

	QString currentSourceNameString =
		context->textSourceDropdownList->currentText();

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
	} else if (context->timerHours->text().toInt() == 0 &&
		   context->timerMinutes->text().toInt() == 0 &&
		   context->timerSeconds->text().toInt() == 0) {
		isZero = true;
	}

	return isZero;
}

void CountdownDockWidget::OBSFrontendEventHandler(enum obs_frontend_event event,
						  void *private_data)
{

	CountdownWidgetStruct *context = (CountdownWidgetStruct *)private_data;

	switch (event) {
	case OBS_FRONTEND_EVENT_FINISHED_LOADING: {
		// CountdownDockWidget::ConnectUISignalHandlers(context);
		CountdownDockWidget::LoadSavedSettings(context);
	} break;
	default:
		break;
	}
}

void CountdownDockWidget::ConnectObsSignalHandlers(
	CountdownWidgetStruct *context)
{
	// Source Signals
	signal_handler_connect(obs_get_signal_handler(), "source_create",
			       OBSSourceCreated, context);

	signal_handler_connect(obs_get_signal_handler(), "source_destroy",
			       OBSSourceDeleted, context);

	signal_handler_connect(obs_get_signal_handler(), "source_rename",
			       OBSSourceRenamed, context);
}

void CountdownDockWidget::OBSSourceCreated(void *param, calldata_t *calldata)
{
	auto context =
		static_cast<CountdownDockWidget::CountdownWidgetStruct *>(
			param);
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
		context->textSourceDropdownList->addItem(name);
	} else if (sourceType == SCENE_SOURCE) {
		context->sceneSourceDropdownList->addItem(name);
	}
};

void CountdownDockWidget::OBSSourceDeleted(void *param, calldata_t *calldata)
{
	auto context =
		static_cast<CountdownDockWidget::CountdownWidgetStruct *>(
			param);

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
			context->textSourceDropdownList->findText(name);
		context->textSourceDropdownList->removeItem(textIndexToRemove);
	} else if (sourceType == SCENE_SOURCE) {
		int sceneIndexToRemove =
			context->sceneSourceDropdownList->findText(name);
		context->sceneSourceDropdownList->removeItem(
			sceneIndexToRemove);
	}
};

void CountdownDockWidget::OBSSourceRenamed(void *param, calldata_t *calldata)
{
	auto context =
		static_cast<CountdownDockWidget::CountdownWidgetStruct *>(
			param);

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
			context->textSourceDropdownList->findText(oldName);
		if (textListIndex == -1)
			return;
		context->textSourceDropdownList->setItemText(textListIndex,
							     newName);
	} else if (sourceType == SCENE_SOURCE) {
		int sceneListIndex =
			context->sceneSourceDropdownList->findText(oldName);
		if (sceneListIndex == -1)
			return;
		context->sceneSourceDropdownList->setItemText(sceneListIndex,
							      newName);
	}
};

int CountdownDockWidget::CheckSourceType(obs_source_t *source)
{
	const char *source_id = obs_source_get_unversioned_id(source);
	if (strcmp(source_id, "text_ft2_source") == 0 ||
	    strcmp(source_id, "text_gdiplus") == 0) {
		return TEXT_SOURCE;
	} else if (strcmp(source_id, "scene") == 0) {
		return SCENE_SOURCE;
	}
	return 0;
}

void CountdownDockWidget::EndMessageCheckBoxSelected(int state)
{
	if (state) {
		countdownTimerData->endMessageLineEdit->setEnabled(true);
		countdownTimerData->timerEndLabel->setEnabled(true);
	} else {
		countdownTimerData->endMessageLineEdit->setEnabled(false);
		countdownTimerData->timerEndLabel->setEnabled(false);
	}
}

void CountdownDockWidget::SceneSwitchCheckBoxSelected(int state)
{
	if (state) {
		countdownTimerData->sceneSourceDropdownList->setEnabled(true);
		countdownTimerData->sceneSwitchLabel->setEnabled(true);
	} else {
		countdownTimerData->sceneSourceDropdownList->setEnabled(false);
		countdownTimerData->sceneSwitchLabel->setEnabled(false);
	}
}

void CountdownDockWidget::SetCurrentScene()
{
	QString selectedScene =
		countdownTimerData->sceneSourceDropdownList->currentText();
	if (selectedScene.length()) {
		obs_source_t *source = obs_get_source_by_name(
			selectedScene.toStdString().c_str());
		if (source != NULL) {
			obs_frontend_set_current_scene(source);
			obs_source_release(source);
		}
	}
}

void CountdownDockWidget::LoadSavedSettings(CountdownWidgetStruct *context)
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

		UNUSED_PARAMETER(selectedTextSource);
		UNUSED_PARAMETER(selectedSceneSource);

		// Apply saved data to plugin
		context->timerHours->setText(QString::number(hours));
		context->hoursCheckBox->setCheckState(
			(Qt::CheckState)hoursCheckBoxStatus);

		context->timerMinutes->setText(QString::number(minutes));
		context->minutesCheckBox->setCheckState(
			(Qt::CheckState)minutesCheckBoxStatus);

		context->timerSeconds->setText(QString::number(seconds));
		context->secondsCheckBox->setCheckState(
			(Qt::CheckState)secondsCheckBoxStatus);

		context->endMessageLineEdit->setText(endMessageText);

		context->endMessageCheckBox->setCheckState(
			(Qt::CheckState)endMessageCheckBoxStatus);

		context->switchSceneCheckBox->setCheckState(
			(Qt::CheckState)switchSceneCheckBoxStatus);

		int textSelectIndex = context->textSourceDropdownList->findText(
			selectedTextSource);
		if (textSelectIndex != -1)
			context->textSourceDropdownList->setCurrentIndex(
				textSelectIndex);

		int sceneSelectIndex =
			context->sceneSourceDropdownList->findText(
				selectedSceneSource);
		if (sceneSelectIndex != -1)
			context->sceneSourceDropdownList->setCurrentIndex(
				sceneSelectIndex);

		obs_data_release(data);
	}
}

void CountdownDockWidget::SaveSettings()
{
	CountdownWidgetStruct *context = countdownTimerData;

	obs_data_t *obsData = obs_data_create();

	int hours = context->timerHours->text().toInt();
	obs_data_set_int(obsData, "hours", hours);
	int hoursCheckBoxStatus = context->hoursCheckBox->checkState();
	obs_data_set_int(obsData, "hoursCheckBoxStatus", hoursCheckBoxStatus);

	int minutes = context->timerMinutes->text().toInt();
	obs_data_set_int(obsData, "minutes", minutes);
	int minutesCheckBoxStatus = context->minutesCheckBox->checkState();
	obs_data_set_int(obsData, "minutesCheckBoxStatus",
			 minutesCheckBoxStatus);

	int seconds = context->timerSeconds->text().toInt();
	obs_data_set_int(obsData, "seconds", seconds);
	int secondsCheckBoxStatus = context->secondsCheckBox->checkState();
	obs_data_set_int(obsData, "secondsCheckBoxStatus",
			 secondsCheckBoxStatus);

	obs_data_set_string(obsData, "selectedTextSource",
			    context->textSourceNameText.c_str());

	int endMessageCheckBoxStatus =
		context->endMessageCheckBox->checkState();
	obs_data_set_int(obsData, "endMessageCheckBoxStatus",
			 endMessageCheckBoxStatus);

	QString endMessageLineEdit = context->endMessageLineEdit->text();
	obs_data_set_string(obsData, "endMessageText",
			    endMessageLineEdit.toStdString().c_str());

	int switchSceneCheckBoxStatus =
		context->switchSceneCheckBox->checkState();
	obs_data_set_int(obsData, "switchSceneCheckBoxStatus",
			 switchSceneCheckBoxStatus);

	obs_data_set_string(obsData, "selectedSceneSource",
			    context->sceneSourceNameText.c_str());

	// Hotkeys
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

	char *file = obs_module_config_path(CONFIG);
	obs_data_save_json(obsData, file);
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
