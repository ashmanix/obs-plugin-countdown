#include "ashmanix-timer.hpp"
#include "settings-dialog.hpp"

AshmanixTimer::AshmanixTimer(QWidget *parent, obs_websocket_vendor newVendor, obs_data_t *savedData,
			     CountdownDockWidget *mDockWidget)
	: QWidget(parent),
	  ui(new Ui::AshmanixTimer)
{
	// Register custom type for signals and slots
	qRegisterMetaType<obs_data_t *>("obs_data_t*");
	qRegisterMetaType<obs_data_t *>("CountdownDockWidget*");

	mainDockWidget = mDockWidget;

	countdownTimerData = TimerWidgetStruct();

	ui->setupUi(this);

	if (savedData) {
		LoadTimerWidgetDataFromOBSSaveData(savedData);
	}

	if (countdownTimerData.timerId.size() == 0) {
		// Create a unique ID for the timer
		QUuid uuid = QUuid::createUuid();
		QByteArray hash = QCryptographicHash::hash(uuid.toByteArray(), QCryptographicHash::Md5);
		countdownTimerData.timerId =
			QString(hash.toHex().left(8)); // We take the first 8 characters of the hash
	}

	this->setProperty("id", countdownTimerData.timerId);

	vendor = &newVendor;

	SetupTimerWidgetUI();

	ConnectUISignalHandlers();

	InitialiseTimerTime();

	RegisterAllHotkeys(savedData);
}

AshmanixTimer::~AshmanixTimer()
{
	UnregisterAllHotkeys();
	this->deleteLater();
}

void AshmanixTimer::SaveTimerWidgetDataToOBSSaveData(obs_data_t *dataObject)
{
	obs_data_set_string(dataObject, "timerId", countdownTimerData.timerId.toStdString().c_str());
	obs_data_set_bool(dataObject, "startOnStreamStart", countdownTimerData.startOnStreamStart);
	obs_data_set_bool(dataObject, "shouldCountUp", countdownTimerData.shouldCountUp);
	obs_data_set_bool(dataObject, "showLeadingZero", countdownTimerData.showLeadingZero);

	obs_data_set_string(dataObject, "selectedSource", countdownTimerData.selectedSource.toStdString().c_str());
	obs_data_set_string(dataObject, "selectedScene", countdownTimerData.selectedScene.toStdString().c_str());
	obs_data_set_string(dataObject, "endMessage", countdownTimerData.endMessage.toStdString().c_str());
	obs_data_set_string(dataObject, "dateTime", countdownTimerData.dateTime.toString().toStdString().c_str());

	obs_data_set_int(dataObject, "periodDays", countdownTimerData.periodDays);
	obs_data_set_int(dataObject, "periodHours", countdownTimerData.periodHours);
	obs_data_set_int(dataObject, "periodMinutes", countdownTimerData.periodMinutes);
	obs_data_set_int(dataObject, "periodSeconds", countdownTimerData.periodSeconds);

	obs_data_set_bool(dataObject, "showDays", countdownTimerData.showDays);
	obs_data_set_bool(dataObject, "showHours", countdownTimerData.showHours);
	obs_data_set_bool(dataObject, "showMinutes", countdownTimerData.showMinutes);
	obs_data_set_bool(dataObject, "showSeconds", countdownTimerData.showSeconds);
	obs_data_set_bool(dataObject, "showEndMessage", countdownTimerData.showEndMessage);
	obs_data_set_bool(dataObject, "showEndScene", countdownTimerData.showEndScene);

	obs_data_set_int(dataObject, "selectedCountdownType", countdownTimerData.selectedCountdownType);

	// ------------------------- Hotkeys -------------------------
	SaveHotkey(dataObject, countdownTimerData.startCountdownHotkeyId, TIMERSTARTHOTKEYNAME);

	SaveHotkey(dataObject, countdownTimerData.pauseCountdownHotkeyId, TIMERPAUSEHOTKEYNAME);

	SaveHotkey(dataObject, countdownTimerData.setCountdownHotkeyId, TIMERSETHOTKEYNAME);

	SaveHotkey(dataObject, countdownTimerData.startCountdownToTimeHotkeyId, TIMERTOTIMESTARTHOTKEYNAME);

	SaveHotkey(dataObject, countdownTimerData.stopCountdownToTimeHotkeyId, TIMERTOTIMESTOPHOTKEYNAME);
}

void AshmanixTimer::LoadTimerWidgetDataFromOBSSaveData(obs_data_t *dataObject)
{

	countdownTimerData.timerId = (char *)obs_data_get_string(dataObject, "timerId");
	countdownTimerData.startOnStreamStart = (bool)obs_data_get_bool(dataObject, "startOnStreamStart");
	countdownTimerData.shouldCountUp = (bool)obs_data_get_bool(dataObject, "shouldCountUp");
	countdownTimerData.showLeadingZero = (bool)obs_data_get_bool(dataObject, "showLeadingZero");
	countdownTimerData.selectedSource = (char *)obs_data_get_string(dataObject, "selectedSource");
	countdownTimerData.selectedScene = (char *)obs_data_get_string(dataObject, "selectedScene");
	countdownTimerData.endMessage = (char *)obs_data_get_string(dataObject, "endMessage");

	QDateTime savedTime = QDateTime::fromString((char *)obs_data_get_string(dataObject, "dateTime"));
	QDateTime currentTime = QDateTime::currentDateTime();
	if (currentTime > savedTime) {
		savedTime = savedTime.addDays(1);
		if (currentTime > savedTime)
			savedTime = savedTime.addDays(1);
	}
	countdownTimerData.dateTime = savedTime;

	countdownTimerData.periodDays = (int)obs_data_get_int(dataObject, "periodDays");
	countdownTimerData.periodHours = (int)obs_data_get_int(dataObject, "periodHours");
	countdownTimerData.periodMinutes = (int)obs_data_get_int(dataObject, "periodMinutes");
	countdownTimerData.periodSeconds = (int)obs_data_get_int(dataObject, "periodSeconds");

	countdownTimerData.showDays = (bool)obs_data_get_bool(dataObject, "showDays");
	countdownTimerData.showHours = (bool)obs_data_get_bool(dataObject, "showHours");
	countdownTimerData.showMinutes = (bool)obs_data_get_bool(dataObject, "showMinutes");
	countdownTimerData.showSeconds = (bool)obs_data_get_bool(dataObject, "showSeconds");
	countdownTimerData.showEndMessage = (bool)obs_data_get_bool(dataObject, "showEndMessage");
	countdownTimerData.showEndScene = (bool)obs_data_get_bool(dataObject, "showEndScene");
	countdownTimerData.selectedCountdownType = (CountdownType)obs_data_get_int(dataObject, "selectedCountdownType");
	countdownTimerData.startCountdownHotkeyId = (int)obs_data_get_int(dataObject, "startCountdownHotkeyId");
	countdownTimerData.pauseCountdownHotkeyId = (int)obs_data_get_int(dataObject, "pauseCountdownHotkeyId");
	countdownTimerData.setCountdownHotkeyId = (int)obs_data_get_int(dataObject, "setCountdownHotkeyId");
	countdownTimerData.startCountdownToTimeHotkeyId =
		(int)obs_data_get_int(dataObject, "startCountdownToTimeHotkeyId");
	countdownTimerData.stopCountdownToTimeHotkeyId =
		(int)obs_data_get_int(dataObject, "stopCountdownToTimeHotkeyId");

	SetTimerData();
}

QString AshmanixTimer::GetTimerID()
{
	return countdownTimerData.timerId;
}

void AshmanixTimer::SetTimerID(QString newId)
{
	countdownTimerData.timerId = newId;
	ui->timerNameLabel->setText(QString("Timer: %1").arg(newId));
}

TimerWidgetStruct *AshmanixTimer::GetTimerData()
{
	return &countdownTimerData;
}

void AshmanixTimer::SetHideMultiTimerUIButtons(bool shouldHide)
{
	if (shouldHide) {

		ui->deleteToolButton->setDisabled(true);
		ui->deleteToolButton->hide();
		ui->timerTitleHLayout->insertItem(0, deleteButtonSpacer);
		ui->moveUpToolButton->hide();
		ui->moveDownToolButton->hide();
	} else {
		ui->timerTitleHLayout->removeItem(deleteButtonSpacer);
		ui->deleteToolButton->setDisabled(false);
		ui->deleteToolButton->show();
		ui->moveUpToolButton->show();
		ui->moveDownToolButton->show();
	}
}

void AshmanixTimer::SetIsUpButtonDisabled(bool isDisabled)
{
	ui->moveUpToolButton->setDisabled(isDisabled);
}

void AshmanixTimer::SetIsDownButtonDisabled(bool isDisabled)
{
	ui->moveDownToolButton->setDisabled(isDisabled);
}

void AshmanixTimer::SetTimerData()
{
	ui->timerNameLabel->setText(QString("Timer: %1").arg(countdownTimerData.timerId));

	ui->dateTimeEdit->setDateTime(countdownTimerData.dateTime);

	ui->timerDays->setText(QString::number(countdownTimerData.periodDays));
	ui->timerHours->setText(QString::number(countdownTimerData.periodHours));
	ui->timerMinutes->setText(QString::number(countdownTimerData.periodMinutes));
	ui->timerSeconds->setText(QString::number(countdownTimerData.periodSeconds));

	ui->timerNameLabel->setText(QString("Timer: %1").arg(countdownTimerData.timerId));
	InitialiseTimerTime();
}

bool AshmanixTimer::AlterTime(WebsocketRequestType requestType, const char *stringTime)
{
	bool result = false;
	// QDateTime currentDateTime = QDateTime::currentDateTime();
	// long long timeToAdd = countdownTimerData.shouldCountUp ? -timeInMillis
	// 						       : timeInMillis;

	switch (requestType) {
	case ADD_TIME:
		result = AddTime(stringTime, countdownTimerData.shouldCountUp);
		break;
	case SET_TIME:
		result = SetTime(stringTime, countdownTimerData.shouldCountUp);
		break;
	default:
		return false;
		break;
	}

	// if (countdownTimerData.selectedCountdownType == PERIOD) {
	// 	switch (requestType) {
	// 	case ADD_TIME:
	// 		countdownTimerData.timeAtTimerStart =
	// 			countdownTimerData.timeAtTimerStart.addMSecs(
	// 				timeToAdd);
	// 		countdownTimerData.timeLeftInMillis =
	// 			countdownTimerData.timeLeftInMillis +
	// 			timeInMillis;
	// 		break;
	// 	case SET_TIME:
	// 		countdownTimerData.timeAtTimerStart =
	// 			currentDateTime.addMSecs(timeToAdd);
	// 		break;
	// 	default:
	// 		return false;
	// 		break;
	// 	}

	// 	UpdateDateTimeDisplay(countdownTimerData.timeLeftInMillis);
	// 	result = true;
	// } else if (countdownTimerData.selectedCountdownType == DATETIME) {
	// 	QDateTime updatedDateTime;

	// 	switch (requestType) {
	// 	case ADD_TIME:
	// 		updatedDateTime = ui->dateTimeEdit->dateTime().addMSecs(
	// 			timeInMillis);
	// 		ui->dateTimeEdit->setDateTime(updatedDateTime);
	// 		break;
	// 	case SET_TIME:
	// 		updatedDateTime = QDateTime::currentDateTime().addMSecs(
	// 			timeInMillis);
	// 		ui->dateTimeEdit->setDateTime(updatedDateTime);
	// 		break;
	// 	default:
	// 		return false;
	// 		break;
	// 	}
	// 	long long new_time = CalcToCurrentDateTimeInMillis(
	// 		ui->dateTimeEdit->dateTime(), TIMERPERIOD);
	// 	UpdateDateTimeDisplay(new_time);
	// 	result = true;
	// }
	// emit RequestTimerReset();
	return result;
}

bool AshmanixTimer::AddTime(const char *stringTime, bool isCountingUp)
{
	bool result = false;
	QDateTime currentDateTime = QDateTime::currentDateTime();
	long long timeInMillis = ConvertStringPeriodToMillis(stringTime);

	if (countdownTimerData.selectedCountdownType == PERIOD) {
		long long uiPeriodInMillis = GetMillisFromPeriodUI();
		uiPeriodInMillis = std::max((uiPeriodInMillis + timeInMillis), 0ll);

		PeriodData periodData = ConvertMillisToPeriodData(uiPeriodInMillis);
		UpdateTimerPeriod(periodData);

		if (!isCountingUp) {
			countdownTimerData.timeAtTimerStart =
				countdownTimerData.timeAtTimerStart.addMSecs(timeInMillis);
			UpdateDateTimeDisplay(countdownTimerData.timeLeftInMillis);
		} else {
			countdownTimerData.timeLeftInMillis =
				std::max((countdownTimerData.timeLeftInMillis + timeInMillis), 0ll);
		}

		result = true;
	} else if (countdownTimerData.selectedCountdownType == DATETIME) {
		QDateTime updatedDateTime;
		updatedDateTime = ui->dateTimeEdit->dateTime().addMSecs(timeInMillis);
		ui->dateTimeEdit->setDateTime(updatedDateTime);
		countdownTimerData.timeLeftInMillis =
			std::max((countdownTimerData.timeLeftInMillis + timeInMillis), 0ll);
		result = true;
	}
	return result;
}

bool AshmanixTimer::SetTime(const char *stringTime, bool isCountingUp)
{
	UNUSED_PARAMETER(isCountingUp);
	long long timeInMillis = ConvertStringPeriodToMillis(stringTime);

	if (timeInMillis < 0)
		return false;

	bool result = false;
	QDateTime currentDateTime = QDateTime::currentDateTime();

	if (countdownTimerData.selectedCountdownType == PERIOD) {
		PeriodData periodData = ConvertMillisToPeriodData(timeInMillis);
		UpdateTimerPeriod(periodData);
		result = true;
	} else if (countdownTimerData.selectedCountdownType == DATETIME) {
		QDateTime updatedDateTime;
		updatedDateTime = currentDateTime.addMSecs(timeInMillis + TIMERPERIOD);
		ui->dateTimeEdit->setDateTime(updatedDateTime);
		result = true;
	}

	emit RequestTimerReset();

	return result;
}

void AshmanixTimer::UpdateTimerPeriod(PeriodData periodData)
{
	countdownTimerData.periodDays = periodData.days;
	countdownTimerData.periodHours = periodData.hours;
	countdownTimerData.periodMinutes = periodData.minutes;
	countdownTimerData.periodSeconds = periodData.seconds;

	ui->timerDays->setText(QString::number(countdownTimerData.periodDays));
	ui->timerHours->setText(QString::number(countdownTimerData.periodHours));
	ui->timerMinutes->setText(QString::number(countdownTimerData.periodMinutes));
	ui->timerSeconds->setText(QString::number(countdownTimerData.periodSeconds));
}

void AshmanixTimer::PressPlayButton()
{
	ui->playButton->click();
}

void AshmanixTimer::PressResetButton()
{
	ui->resetButton->click();
}

void AshmanixTimer::PressStopButton()
{
	ui->pauseButton->click();
}

void AshmanixTimer::PressToTimePlayButton()
{
	ui->toTimePlayButton->click();
}

void AshmanixTimer::PressToTimeStopButton()
{
	ui->toTimeStopButton->click();
}

void AshmanixTimer::UpdateStyles()
{
	// Set toolbutton colour checked to darkened colour
	QColor bgColor = ui->periodToolButton->palette().color(QPalette::Button);
	QColor darkenedColor = bgColor.darker(150);
	this->setStyleSheet(QString("QToolButton:checked { background-color: %1; } ").arg(darkenedColor.name()));
}

// --------------------------------- Private ----------------------------------

void AshmanixTimer::SetupTimerWidgetUI()
{
	ui->timerNameLabel->setText(QString("Timer: %1").arg(countdownTimerData.timerId));

	ui->settingsToolButton->setProperty("themeID", "propertiesIconSmall");
	ui->settingsToolButton->setProperty("class", "icon-gear");
	ui->settingsToolButton->setText("");
	ui->settingsToolButton->setEnabled(true);
	ui->settingsToolButton->setToolTip(obs_module_text("SettingsButtonTip"));

	ui->deleteToolButton->setProperty("themeID", "removeIconSmall");
	ui->deleteToolButton->setProperty("class", "icon-trash");
	ui->deleteToolButton->setText("");
	ui->deleteToolButton->setEnabled(true);
	ui->deleteToolButton->setToolTip(obs_module_text("DeleteTimerButtonTip"));

	ui->moveUpToolButton->setProperty("themeID", "upArrowIconSmall");
	ui->moveUpToolButton->setProperty("class", "icon-up");
	ui->moveUpToolButton->setText("");
	ui->moveUpToolButton->setEnabled(true);
	ui->moveUpToolButton->setToolTip(obs_module_text("MoveTimerUpButtonTip"));

	ui->moveDownToolButton->setProperty("themeID", "downArrowIconSmall");
	ui->moveDownToolButton->setProperty("class", "icon-down");
	ui->moveDownToolButton->setText("");
	ui->moveDownToolButton->setEnabled(true);
	ui->moveDownToolButton->setToolTip(obs_module_text("MoveTimerDownButtonTip"));

	ui->timeDisplay->display(AshmanixTimer::ZEROSTRING);

	ui->dateTimeEdit->setMinimumDate(QDate::currentDate());
	ui->dateTimeEdit->setMaximumDate(QDate::currentDate().addDays(999));

	ui->timerDays->setMaxLength(3);
	ui->timerDays->setValidator(new QRegularExpressionValidator(QRegularExpression("^(0|[1-9]\\d{0,2})$"), this));
	ui->timerDays->setToolTip(obs_module_text("DaysCheckboxLabel"));

	ui->timerHours->setMaxLength(2);
	ui->timerHours->setValidator(
		new QRegularExpressionValidator(QRegularExpression("^(0?[0-9]|1[0-9]|2[0-3])$"), this));
	ui->timerHours->setToolTip(obs_module_text("HoursCheckboxLabel"));

	ui->timerMinutes->setMaxLength(2);
	ui->timerMinutes->setValidator(new QRegularExpressionValidator(QRegularExpression("^[1-5]?[0-9]"), this));
	ui->timerMinutes->setToolTip(obs_module_text("MinutesCheckboxLabel"));

	ui->timerSeconds->setAlignment(Qt::AlignCenter);
	ui->timerSeconds->setMaxLength(2);
	ui->timerSeconds->setValidator(new QRegularExpressionValidator(QRegularExpression("^[1-5]?[0-9]"), this));
	ui->timerSeconds->setToolTip(obs_module_text("SecondsCheckboxLabel"));

	countdownTimerData.periodVLayout = ui->periodWidget;
	ui->periodToolButton->setText(obs_module_text("SetPeriodTabLabel"));
	ui->periodToolButton->setToolTip(obs_module_text("SetPeriodTabTip"));
	countdownTimerData.datetimeVLayout = ui->datetimeWidget;
	ui->datetimeToolButton->setText(obs_module_text("SetDatetimeTabLabel"));
	ui->datetimeToolButton->setToolTip(obs_module_text("SetDatetimeTabTip"));
	UpdateStyles();
	ToggleTimeType(countdownTimerData.selectedCountdownType);

	ui->playButton->setProperty("themeID", "playIcon");
	ui->playButton->setProperty("class", "icon-media-play");
	ui->playButton->setEnabled(true);
	ui->playButton->setToolTip(obs_module_text("PlayButtonTip"));

	ui->pauseButton->setProperty("themeID", "pauseIcon");
	ui->pauseButton->setProperty("class", "icon-media-pause");
	ui->pauseButton->setEnabled(false);
	ui->pauseButton->setToolTip(obs_module_text("PauseButtonTip"));

	ui->resetButton->setProperty("themeID", "restartIcon");
	ui->resetButton->setProperty("class", "icon-media-restart");
	ui->resetButton->setToolTip(obs_module_text("ResetButtonTip"));

	ui->toTimePlayButton->setProperty("themeID", "playIcon");
	ui->toTimePlayButton->setProperty("class", "icon-media-play");
	ui->toTimePlayButton->setEnabled(true);
	ui->toTimePlayButton->setToolTip(

		obs_module_text("ToTimePlayButtonTip"));
	ui->toTimeStopButton->setProperty("themeID", "stopIcon");
	ui->toTimeStopButton->setProperty("class", "icon-media-stop");
	ui->toTimeStopButton->setEnabled(false);
	ui->toTimeStopButton->setToolTip(obs_module_text("ToTimeStopButtonTip"));

	ui->frame->setProperty("class", "bg-base");

	deleteButtonSpacer = new QSpacerItem(ui->deleteToolButton->sizeHint().width(),
					     ui->deleteToolButton->sizeHint().height(), QSizePolicy::Fixed,
					     QSizePolicy::Fixed);

	UpdateTimeDisplayTooltip();

	countdownTimerData.isPlaying = false;
}

void AshmanixTimer::ConnectUISignalHandlers()
{
	QObject::connect(ui->playButton, &QPushButton::clicked, this, &AshmanixTimer::PlayButtonClicked);

	QObject::connect(ui->pauseButton, &QPushButton::clicked, this, &AshmanixTimer::PauseButtonClicked);

	QObject::connect(ui->resetButton, &QPushButton::clicked, this, &AshmanixTimer::ResetButtonClicked);

	QObject::connect(ui->toTimePlayButton, &QPushButton::clicked, this, &AshmanixTimer::ToTimePlayButtonClicked);

	QObject::connect(ui->toTimeStopButton, &QPushButton::clicked, this, &AshmanixTimer::ToTimeStopButtonClicked);

	QObject::connect(ui->deleteToolButton, &QPushButton::clicked, this, &AshmanixTimer::DeleteButtonClicked);

	QObject::connect(ui->settingsToolButton, &QPushButton::clicked, this, &AshmanixTimer::SettingsButtonClicked);

	QObject::connect(ui->timerDays, &QLineEdit::textChanged, this, &AshmanixTimer::DaysChanged);

	QObject::connect(ui->timerHours, &QLineEdit::textChanged, this, &AshmanixTimer::HoursChanged);

	QObject::connect(ui->timerMinutes, &QLineEdit::textChanged, this, &AshmanixTimer::MinutesChanged);

	QObject::connect(ui->timerSeconds, &QLineEdit::textChanged, this, &AshmanixTimer::SecondsChanged);

	QObject::connect(ui->dateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &AshmanixTimer::DateTimeChanged);

	QObject::connect(ui->moveUpToolButton, &QPushButton::clicked, this, &AshmanixTimer::EmitMoveTimerUpSignal);

	QObject::connect(ui->moveDownToolButton, &QPushButton::clicked, this, &AshmanixTimer::EmitMoveTimerDownSignal);

	QObject::connect(ui->periodToolButton, &QPushButton::clicked, this, [this]() { ToggleTimeType(PERIOD); });

	QObject::connect(ui->datetimeToolButton, &QPushButton::clicked, this, [this]() { ToggleTimeType(DATETIME); });
}

QString AshmanixTimer::ConvertDateTimeToFormattedDisplayString(long long timeInMillis, bool showLeadingZero)
{
	QString formattedDateTimeString = GetFormattedTimerString(
		countdownTimerData.showDays, countdownTimerData.showHours, countdownTimerData.showMinutes,
		countdownTimerData.showSeconds, showLeadingZero, timeInMillis);

	return (formattedDateTimeString == "") ? "Nothing selected!" : formattedDateTimeString;
}

void AshmanixTimer::StartTimerCounting()
{
	countdownTimerData.isPlaying = true;
	countdownTimerData.timer->start(TIMERPERIOD);
	ui->playButton->setEnabled(false);
	ui->pauseButton->setEnabled(true);
	ui->resetButton->setEnabled(false);

	ui->toTimePlayButton->setEnabled(false);
	ui->toTimeStopButton->setEnabled(true);

	ui->timerDays->setEnabled(false);
	ui->timerHours->setEnabled(false);
	ui->timerMinutes->setEnabled(false);
	ui->timerSeconds->setEnabled(false);

	ui->periodToolButton->setEnabled(false);
	ui->datetimeToolButton->setEnabled(false);
	ui->dateTimeEdit->setEnabled(false);

	if (settingsDialogUi)
		settingsDialogUi->SetCountUpCheckBoxEnabled(false);

	SendTimerStateEvent(countdownTimerData.timerId, "started");
}

void AshmanixTimer::StopTimerCounting()
{
	countdownTimerData.isPlaying = false;
	countdownTimerData.timer->stop();
	ui->playButton->setEnabled(true);
	ui->pauseButton->setEnabled(false);
	ui->resetButton->setEnabled(true);

	ui->toTimePlayButton->setEnabled(true);
	ui->toTimeStopButton->setEnabled(false);

	ui->timerDays->setEnabled(true);
	ui->timerHours->setEnabled(true);
	ui->timerMinutes->setEnabled(true);
	ui->timerSeconds->setEnabled(true);

	ui->periodToolButton->setEnabled(true);
	ui->datetimeToolButton->setEnabled(true);
	ui->dateTimeEdit->setEnabled(true);

	if (settingsDialogUi)
		settingsDialogUi->SetCountUpCheckBoxEnabled(true);

	SendTimerStateEvent(countdownTimerData.timerId, "stopped");
}

void AshmanixTimer::InitialiseTimerTime()
{
	countdownTimerData.timer = new QTimer();
	QObject::connect(countdownTimerData.timer, SIGNAL(timeout()), SLOT(TimerAdjust()));
	QObject::connect(this, &AshmanixTimer::RequestTimerReset, this, &AshmanixTimer::HandleTimerReset);

	countdownTimerData.timeLeftInMillis = GetMillisFromPeriodUI();
}

bool AshmanixTimer::IsSetTimeZero()
{
	bool isZero = false;

	if (countdownTimerData.timeLeftInMillis == 0) {
		isZero = true;
	} else if (ui->timerDays->text().toInt() == 0 && ui->timerHours->text().toInt() == 0 &&
		   ui->timerMinutes->text().toInt() == 0 && ui->timerSeconds->text().toInt() == 0) {
		isZero = true;
	}

	return isZero;
}

void AshmanixTimer::UpdateDateTimeDisplay(long long timeInMillis)
{
	long long timeToUpdateInMillis = std::max(timeInMillis, 0ll);
	ui->timeDisplay->display(ConvertMillisToDateTimeString(timeToUpdateInMillis));
	QString formattedDisplayTime =
		ConvertDateTimeToFormattedDisplayString(timeToUpdateInMillis, countdownTimerData.showLeadingZero);
	SetSourceText(formattedDisplayTime);
}

void AshmanixTimer::SetSourceText(QString newText)
{
	obs_source_t *selectedSource = obs_get_source_by_name(countdownTimerData.selectedSource.toStdString().c_str());

	if (selectedSource != NULL) {
		obs_data_t *sourceSettings = obs_source_get_settings(selectedSource);
		obs_data_set_string(sourceSettings, "text", newText.toStdString().c_str());
		obs_source_update(selectedSource, sourceSettings);
		obs_data_release(sourceSettings);
		obs_source_release(selectedSource);
	}
}

void AshmanixTimer::SetCurrentScene()
{
	if (countdownTimerData.selectedScene.length()) {
		obs_source_t *source = obs_get_source_by_name(countdownTimerData.selectedScene.toStdString().c_str());
		if (source != NULL) {
			obs_frontend_set_current_scene(source);
			obs_source_release(source);
		}
	}
}

long long AshmanixTimer::GetMillisFromPeriodUI()
{
	long long days_ms = static_cast<long long>(ui->timerDays->text().toInt()) * 24 * 60 * 60 * 1000;
	long long hours_ms = static_cast<long long>(ui->timerHours->text().toInt()) * 60 * 60 * 1000;
	long long minutes_ms = static_cast<long long>(ui->timerMinutes->text().toInt()) * 60 * 1000;
	long long seconds_ms = static_cast<long long>(ui->timerSeconds->text().toInt()) * 1000;

	return days_ms + hours_ms + minutes_ms + seconds_ms;
}

void AshmanixTimer::SendTimerTickEvent(QString timerId, long long timeLeftInMillis)
{
	obs_data_t *eventData = obs_data_create();

	// Convert milliseconds to readable format
	QString timeString =
		ConvertDateTimeToFormattedDisplayString(timeLeftInMillis, countdownTimerData.showLeadingZero);

	obs_data_set_string(eventData, "timer_id", timerId.toStdString().c_str());
	obs_data_set_string(eventData, "time_display", timeString.toStdString().c_str());
	obs_data_set_int(eventData, "time_left_ms", timeLeftInMillis);

	emit RequestSendWebsocketEvent("const char *eventName", eventData);
	obs_data_release(eventData);
}

void AshmanixTimer::SendTimerStateEvent(QString timerId, const char *state)
{
	obs_data_t *eventData = obs_data_create();
	obs_data_set_string(eventData, "timer_id", timerId.toStdString().c_str());
	obs_data_set_string(eventData, "state", state);

	if (countdownTimerData.selectedSource.length() > 0) {
		obs_data_set_string(eventData, "text_source", countdownTimerData.selectedSource.toStdString().c_str());
	}

	emit RequestSendWebsocketEvent("timer_state_changed", eventData);
	obs_data_release(eventData);
}

void AshmanixTimer::RegisterAllHotkeys(obs_data_t *savedData)
{
	LoadHotkey(
		countdownTimerData.startCountdownHotkeyId, TIMERSTARTHOTKEYNAME,
		GetFullHotkeyName(obs_module_text("StartCountdownHotkeyDescription"), " - ").c_str(),
		[this]() { ui->playButton->click(); }, GetFullHotkeyName("Play Hotkey Pressed", " "), savedData);

	LoadHotkey(
		countdownTimerData.pauseCountdownHotkeyId, TIMERPAUSEHOTKEYNAME,
		GetFullHotkeyName(obs_module_text("PauseCountdownHotkeyDescription"), " - ").c_str(),
		[this]() { ui->pauseButton->animateClick(); }, GetFullHotkeyName("Pause Hotkey Pressed", " "),
		savedData);

	LoadHotkey(
		countdownTimerData.setCountdownHotkeyId, TIMERSETHOTKEYNAME,
		GetFullHotkeyName(obs_module_text("SetCountdownHotkeyDescription"), " - ").c_str(),
		[this]() { ui->resetButton->animateClick(); }, GetFullHotkeyName("Set Hotkey Pressed", " "), savedData);

	LoadHotkey(
		countdownTimerData.startCountdownToTimeHotkeyId, TIMERTOTIMESTARTHOTKEYNAME,
		GetFullHotkeyName(obs_module_text("StartCountdownToTimeHotkeyDescription"), " - ").c_str(),
		[this]() { ui->toTimePlayButton->animateClick(); },
		GetFullHotkeyName("To Time Start Hotkey Pressed", " "), savedData);

	LoadHotkey(
		countdownTimerData.stopCountdownToTimeHotkeyId, TIMERTOTIMESTOPHOTKEYNAME,
		GetFullHotkeyName(obs_module_text("StopCountdownToTimeHotkeyDescription"), " - ").c_str(),
		[this]() { ui->toTimeStopButton->animateClick(); },
		GetFullHotkeyName("To Time Stop Hotkey Pressed", " "), savedData);
}

void AshmanixTimer::UnregisterAllHotkeys()
{
	if (countdownTimerData.startCountdownHotkeyId)
		obs_hotkey_unregister(countdownTimerData.startCountdownHotkeyId);
	if (countdownTimerData.pauseCountdownHotkeyId)
		obs_hotkey_unregister(countdownTimerData.pauseCountdownHotkeyId);
	if (countdownTimerData.setCountdownHotkeyId)
		obs_hotkey_unregister(countdownTimerData.setCountdownHotkeyId);

	if (countdownTimerData.startCountdownToTimeHotkeyId)
		obs_hotkey_unregister(countdownTimerData.startCountdownToTimeHotkeyId);
	if (countdownTimerData.stopCountdownToTimeHotkeyId)
		obs_hotkey_unregister(countdownTimerData.stopCountdownToTimeHotkeyId);
}

std::string AshmanixTimer::GetFullHotkeyName(std::string name, const char *joinText)
{
	static std::string fullName;
	fullName = std::string(name) + std::string(joinText) + countdownTimerData.timerId.toStdString();
	return fullName;
}

// --------------------------------- Public Slots ----------------------------------

void AshmanixTimer::PlayButtonClicked()
{
	if (countdownTimerData.selectedCountdownType == DATETIME) {
		ToggleTimeType(PERIOD);
	}
	UpdateDateTimeDisplay(countdownTimerData.timeLeftInMillis);

	long long periodUIInMillis = GetMillisFromPeriodUI();
	QDateTime currentDateTime = QDateTime::currentDateTime();

	if ((!countdownTimerData.shouldCountUp && IsSetTimeZero()) ||
	    (countdownTimerData.shouldCountUp && countdownTimerData.timeLeftInMillis >= periodUIInMillis))
		return;

	if (countdownTimerData.shouldCountUp) {
		countdownTimerData.timeAtTimerStart = currentDateTime.addMSecs(-(countdownTimerData.timeLeftInMillis));
	} else {
		countdownTimerData.timeAtTimerStart = currentDateTime.addMSecs(countdownTimerData.timeLeftInMillis);
	}

	StartTimerCounting();
}

void AshmanixTimer::PauseButtonClicked()
{
	if (countdownTimerData.selectedCountdownType == DATETIME) {
		ToggleTimeType(PERIOD);
	}

	StopTimerCounting();
	SendTimerStateEvent(countdownTimerData.timerId, "paused");
}

void AshmanixTimer::ResetButtonClicked()
{
	if (countdownTimerData.selectedCountdownType == DATETIME) {
		ToggleTimeType(PERIOD);
	}

	StopTimerCounting();
	countdownTimerData.shouldCountUp ? countdownTimerData.timeLeftInMillis = 0
					 : countdownTimerData.timeLeftInMillis = GetMillisFromPeriodUI();

	UpdateDateTimeDisplay(countdownTimerData.timeLeftInMillis);
}

void AshmanixTimer::ToTimePlayButtonClicked()
{
	if (countdownTimerData.selectedCountdownType == PERIOD) {
		ToggleTimeType(DATETIME);
	}

	countdownTimerData.timeAtTimerStart = QDateTime::currentDateTime();

	if (countdownTimerData.shouldCountUp) {
		countdownTimerData.timeLeftInMillis = 0;
	} else {
		countdownTimerData.timeLeftInMillis =
			countdownTimerData.timeAtTimerStart.msecsTo(ui->dateTimeEdit->dateTime());
		if (countdownTimerData.timeLeftInMillis < 0)
			countdownTimerData.timeLeftInMillis = 0;
	}

	UpdateDateTimeDisplay(countdownTimerData.timeLeftInMillis);
	StartTimerCounting();
}

void AshmanixTimer::ToTimeStopButtonClicked()
{
	if (countdownTimerData.selectedCountdownType == PERIOD) {
		ToggleTimeType(DATETIME);
	}

	StopTimerCounting();
}

// ------------------------------- Private Slots ----------------------------------

void AshmanixTimer::SettingsButtonClicked()
{
	if (!settingsDialogUi) {
		settingsDialogUi = new SettingsDialog(this, &countdownTimerData, mainDockWidget);

		QObject::connect(settingsDialogUi, &SettingsDialog::SettingsUpdated, this,
				 [this]() { UpdateTimeDisplayTooltip(); });
	}
	if (settingsDialogUi->isVisible()) {
		settingsDialogUi->raise();
		settingsDialogUi->activateWindow();
	} else {
		settingsDialogUi->setVisible(true);
	}
}

void AshmanixTimer::DeleteButtonClicked()
{
	emit RequestDelete(countdownTimerData.timerId);
}

void AshmanixTimer::TimerAdjust()
{
	// Flag for ending timer
	bool endTimer = false;
	bool isCountingDown = !countdownTimerData.shouldCountUp;
	long long timerPeriodMillis = countdownTimerData.timeLeftInMillis;

	if (isCountingDown) {
		// Counting down
		if (countdownTimerData.selectedCountdownType == PERIOD) {
			// If selected tab is period
			timerPeriodMillis = static_cast<long long>(
				QDateTime::currentDateTime().msecsTo(countdownTimerData.timeAtTimerStart));
		} else {
			// If selected tab is datetime
			timerPeriodMillis = static_cast<long long>(
				QDateTime::currentDateTime().msecsTo(ui->dateTimeEdit->dateTime()));
		}
		if (timerPeriodMillis < TIMERPERIOD)
			endTimer = true;
	} else {
		// When counting up always add to current timer

		// Check if we need to end timer
		if (countdownTimerData.selectedCountdownType == PERIOD) {
			timerPeriodMillis = static_cast<long long>(
				countdownTimerData.timeAtTimerStart.msecsTo(QDateTime::currentDateTime()));
			// If selected tab is period
			if (timerPeriodMillis >= GetMillisFromPeriodUI())
				endTimer = true;
		} else {
			timerPeriodMillis = static_cast<long long>(
				countdownTimerData.timeAtTimerStart.msecsTo(QDateTime::currentDateTime()));
			// If selected tab is datetime
			if ((countdownTimerData.timeAtTimerStart.msecsTo(ui->dateTimeEdit->dateTime())) -
				    timerPeriodMillis <=
			    TIMERPERIOD)
				endTimer = true;
		}
	}

	countdownTimerData.timeLeftInMillis = timerPeriodMillis;

	if (countdownTimerData.timeLeftInMillis < 0)
		countdownTimerData.timeLeftInMillis = 0;

	// We only update the time and send a tick event if the seconds have changed from last time
	if (lastDisplayedSeconds != (countdownTimerData.timeLeftInMillis / 1000)) {
		lastDisplayedSeconds = countdownTimerData.timeLeftInMillis / 1000;

		UpdateDateTimeDisplay(countdownTimerData.timeLeftInMillis);

		// Send tick event
		SendTimerTickEvent(countdownTimerData.timerId, countdownTimerData.timeLeftInMillis);
	}

	if (endTimer == true) {
		if (countdownTimerData.showEndMessage) {
			SetSourceText(countdownTimerData.endMessage.toStdString().c_str());
		}
		if (countdownTimerData.showEndScene) {
			SetCurrentScene();
		}
		if (isCountingDown) {
			ui->timeDisplay->display(AshmanixTimer::ZEROSTRING);
			countdownTimerData.timeLeftInMillis = 0;
		} else {
			if (countdownTimerData.selectedCountdownType == PERIOD) {
				countdownTimerData.timeLeftInMillis = GetMillisFromPeriodUI();
			} else {
				countdownTimerData.timeLeftInMillis =
					countdownTimerData.timeAtTimerStart.msecsTo(ui->dateTimeEdit->dateTime());
			}
			UpdateDateTimeDisplay(countdownTimerData.timeLeftInMillis);
		}
		// Send completion event
		SendTimerStateEvent(countdownTimerData.timerId, "completed");
		StopTimerCounting();
		return;
	}
}

void AshmanixTimer::HandleTimerReset()
{
	if (countdownTimerData.timer && countdownTimerData.timer->isActive()) {
		switch (countdownTimerData.selectedCountdownType) {
		case PERIOD:
			ResetButtonClicked();
			PlayButtonClicked();
			break;

		case DATETIME:
			ToTimePlayButtonClicked();
			break;

		default:
			break;
		}
	}
}

void AshmanixTimer::DaysChanged(QString newText)
{
	countdownTimerData.periodDays = newText.toInt();
}

void AshmanixTimer::HoursChanged(QString newText)
{
	countdownTimerData.periodHours = newText.toInt();
}

void AshmanixTimer::MinutesChanged(QString newText)
{
	countdownTimerData.periodMinutes = newText.toInt();
}

void AshmanixTimer::SecondsChanged(QString newText)
{
	countdownTimerData.periodSeconds = newText.toInt();
}

void AshmanixTimer::DateTimeChanged(QDateTime newDateTime)
{
	countdownTimerData.dateTime = newDateTime;
}

void AshmanixTimer::EmitMoveTimerDownSignal()
{
	emit MoveTimer(QString("down"), countdownTimerData.timerId);
}

void AshmanixTimer::EmitMoveTimerUpSignal()
{
	emit MoveTimer(QString("up"), countdownTimerData.timerId);
}

void AshmanixTimer::ToggleTimeType(CountdownType type)
{
	if (countdownTimerData.periodVLayout && countdownTimerData.datetimeVLayout) {
		countdownTimerData.datetimeVLayout->hide();
		countdownTimerData.periodVLayout->hide();
		ui->periodToolButton->setChecked(false);
		ui->datetimeToolButton->setChecked(false);

		if (type == PERIOD) {
			countdownTimerData.periodVLayout->show();
			countdownTimerData.selectedCountdownType = type;
			ui->periodToolButton->setChecked(true);
		} else if (type == DATETIME) {
			countdownTimerData.datetimeVLayout->show();
			countdownTimerData.selectedCountdownType = type;
			ui->datetimeToolButton->setChecked(true);
		}
	} else {
		obs_log(LOG_WARNING, "Period and/or Datetime layouts not found!");
	}
}

void AshmanixTimer::UpdateTimeDisplayTooltip()
{
	QString detailsTooltip = "";

	detailsTooltip += obs_module_text("TextSourceLabel");
	detailsTooltip += " : ";
	detailsTooltip += countdownTimerData.selectedSource;
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("StartOnStreamCheckBoxLabel");
	detailsTooltip += " : ";
	detailsTooltip += countdownTimerData.startOnStreamStart ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("EndMessageLabel");
	detailsTooltip += " : ";
	if (countdownTimerData.showEndMessage) {
		detailsTooltip += "✓ ";
		detailsTooltip += countdownTimerData.endMessage;
	} else {
		detailsTooltip += "-";
	}
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("SwitchScene");
	detailsTooltip += " : ";
	if (countdownTimerData.showEndScene) {
		detailsTooltip += "✓ ";
		detailsTooltip += countdownTimerData.selectedScene;
	} else {
		detailsTooltip += "-";
	}
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("DaysCheckboxLabel");
	detailsTooltip += " : ";
	detailsTooltip += countdownTimerData.showDays ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("HoursCheckboxLabel");
	detailsTooltip += " : ";
	detailsTooltip += countdownTimerData.showHours ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("MinutesCheckboxLabel");
	detailsTooltip += " : ";
	detailsTooltip += countdownTimerData.showMinutes ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("SecondsCheckboxLabel");
	detailsTooltip += " : ";
	detailsTooltip += countdownTimerData.showSeconds ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("LeadZeroCheckboxLabel");
	detailsTooltip += " : ";
	detailsTooltip += countdownTimerData.showLeadingZero ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("CountUpCheckBoxLabel");
	detailsTooltip += " : ";
	detailsTooltip += countdownTimerData.shouldCountUp ? "✓" : "-";

	ui->timeDisplay->setToolTip(detailsTooltip);
}

void AshmanixTimer::StartTimer()
{
	switch (countdownTimerData.selectedCountdownType) {
	case PERIOD:
		PressPlayButton();
		break;

	case DATETIME:
		PressToTimePlayButton();
		break;
	}
}

void AshmanixTimer::StopTimer()
{
	switch (countdownTimerData.selectedCountdownType) {
	case PERIOD:
		PressStopButton();
		break;

	case DATETIME:
		PressToTimeStopButton();
		break;
	}
}
