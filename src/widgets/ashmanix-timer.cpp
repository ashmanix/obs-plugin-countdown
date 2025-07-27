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
	vendor = &newVendor;

	countdownTimerData = TimerWidgetStruct();

	uiManager = new TimerUIManager(this, ui, &countdownTimerData, mDockWidget, settingsDialogUi);
	timerEngine = new TimerEngine(this, &countdownTimerData);
	timerPersistence = new TimerPersistence();
	hotkeyManager = new HotkeyManager(this, ui, &countdownTimerData);
	websocketNotifier = new WebsocketNotifier(vendor);

	this->setProperty("id", countdownTimerData.timerId);

	ConnectSignalHandlers();

	if (savedData) {
		timerPersistence->LoadTimerWidgetDataFromOBSSaveData(&countdownTimerData, savedData);
		SetTimerData();
	} else {
		InitialiseTimerTime();
	}

	if (countdownTimerData.timerId.size() == 0) {
		// Create a unique ID for the timer
		QUuid uuid = QUuid::createUuid();
		QByteArray hash = QCryptographicHash::hash(uuid.toByteArray(), QCryptographicHash::Md5);
		countdownTimerData.timerId =
			QString(hash.toHex().left(8)); // We take the first 8 characters of the hash
	}

	hotkeyManager->RegisterAllHotkeys(savedData);
}

AshmanixTimer::~AshmanixTimer()
{
	hotkeyManager->UnregisterAllHotkeys();
	this->deleteLater();
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
	uiManager->SetHideMultiTimerUIButtons(shouldHide);
}

void AshmanixTimer::SetIsUpButtonDisabled(bool isDisabled)
{
	uiManager->SetIsUpButtonDisabled(isDisabled);
}

void AshmanixTimer::SetIsDownButtonDisabled(bool isDisabled)
{
	uiManager->SetIsDownButtonDisabled(isDisabled);
}

void AshmanixTimer::SetTimerData()
{
	uiManager->SetTimeUI();
	InitialiseTimerTime(false);
}

bool AshmanixTimer::AlterTime(WebsocketRequestType requestType, const char *stringTime)
{
	bool result = false;

	switch (requestType) {
	case WebsocketRequestType::ADD_TIME:
		result = AddTime(stringTime, countdownTimerData.shouldCountUp);
		break;
	case WebsocketRequestType::SET_TIME:
		result = SetTime(stringTime);
		break;
	default:
		return false;
		break;
	}
	return result;
}

void AshmanixTimer::UpdateStyles()
{
	uiManager->UpdateStyles();
}

void AshmanixTimer::StartTimer(bool shouldReset)
{
	switch (countdownTimerData.selectedCountdownType) {
	case CountdownType::PERIOD:
		if (shouldReset)
			uiManager->HandleTimerAction(TimerAction::Reset);

		uiManager->HandleTimerAction(TimerAction::Play);
		break;

	case CountdownType::DATETIME:
		uiManager->HandleTimerAction(TimerAction::ToTimePlay);
		break;
	}
}

void AshmanixTimer::StopTimer()
{
	switch (countdownTimerData.selectedCountdownType) {
	case CountdownType::PERIOD:
		uiManager->HandleTimerAction(TimerAction::Reset);
		break;

	case CountdownType::DATETIME:
		uiManager->HandleTimerAction(TimerAction::ToTimeStop);
		break;
	}
}

void AshmanixTimer::ActivateTimerAction(TimerAction action)
{
	uiManager->HandleTimerAction(action);
}

void AshmanixTimer::SaveData(obs_data_t *dataObject)
{
	timerPersistence->SaveTimerWidgetDataToOBSSaveData(&countdownTimerData, dataObject);
}

// --------------------------------- Private ----------------------------------

void AshmanixTimer::ConnectSignalHandlers()
{
	// 	QObject::connect(ui->playButton, &QPushButton::clicked, this, &AshmanixTimer::PlayButtonClicked);

	// 	QObject::connect(ui->pauseButton, &QPushButton::clicked, this, &AshmanixTimer::PauseButtonClicked);

	// 	QObject::connect(ui->resetButton, &QPushButton::clicked, this, &AshmanixTimer::ResetButtonClicked);

	// 	QObject::connect(ui->toTimePlayButton, &QPushButton::clicked, this, &AshmanixTimer::ToTimePlayButtonClicked);

	// 	QObject::connect(ui->toTimeStopButton, &QPushButton::clicked, this, &AshmanixTimer::ToTimeStopButtonClicked);

	// 	QObject::connect(ui->deleteToolButton, &QPushButton::clicked, this, &AshmanixTimer::DeleteButtonClicked);

	// 	QObject::connect(ui->settingsToolButton, &QPushButton::clicked, this, &AshmanixTimer::SettingsButtonClicked);

	// 	QObject::connect(ui->timerDays, &QSpinBox::valueChanged, this, &AshmanixTimer::DaysChanged);

	// 	QObject::connect(ui->timerHours, &QSpinBox::valueChanged, this, &AshmanixTimer::HoursChanged);

	// 	QObject::connect(ui->timerMinutes, &QSpinBox::valueChanged, this, &AshmanixTimer::MinutesChanged);

	// 	QObject::connect(ui->timerSeconds, &QSpinBox::valueChanged, this, &AshmanixTimer::SecondsChanged);

	// 	QObject::connect(ui->dateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &AshmanixTimer::DateTimeChanged);

	// 	QObject::connect(ui->moveUpToolButton, &QPushButton::clicked, this, &AshmanixTimer::EmitMoveTimerUpSignal);

	// 	QObject::connect(ui->moveDownToolButton, &QPushButton::clicked, this, &AshmanixTimer::EmitMoveTimerDownSignal);

	// 	QObject::connect(ui->periodToolButton, &QPushButton::clicked, this,
	// 			 [this]() { ToggleTimeType(CountdownType::PERIOD); });

	// 	QObject::connect(ui->datetimeToolButton, &QPushButton::clicked, this,
	// 			 [this]() { ToggleTimeType(CountdownType::DATETIME); });
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
		settingsDialogUi->ToggleCounterCheckBoxes(false);

	websocketNotifier->SendStateEvent(countdownTimerData.timerId, "started",
					  countdownTimerData.source.selectedSource);
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
		settingsDialogUi->ToggleCounterCheckBoxes(true);

	websocketNotifier->SendStateEvent(countdownTimerData.timerId, "stopped",
					  countdownTimerData.source.selectedSource);
}

void AshmanixTimer::InitialiseTimerTime(bool setTimeLeftToUI)
{
	timerEngine->Initialise();
	QObject::connect(this, &AshmanixTimer::RequestTimerReset, this, &AshmanixTimer::HandleTimerReset);
	if (setTimeLeftToUI)
		countdownTimerData.timeLeftInMillis = uiManager->GetMillisFromPeriodUI();
}

void AshmanixTimer::SetSourceText(QString newText)
{
	obs_source_t *selectedSource =
		obs_get_source_by_name(countdownTimerData.source.selectedSource.toStdString().c_str());

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
	if (countdownTimerData.source.selectedScene.length()) {
		obs_source_t *source =
			obs_get_source_by_name(countdownTimerData.source.selectedScene.toStdString().c_str());
		if (source != NULL) {
			obs_frontend_set_current_scene(source);
			obs_source_release(source);
		}
	}
}

std::string AshmanixTimer::GetFullHotkeyName(std::string name, const char *joinText)
{
	static std::string fullName;
	fullName = std::string(name) + std::string(joinText) + countdownTimerData.timerId.toStdString();
	return fullName;
}

bool AshmanixTimer::AddTime(const char *stringTime, bool isCountingUp)
{
	bool result = uiManager->AddTime(stringTime, isCountingUp);

	if (!isCountingUp)
		emit RequestTimerReset(true);

	return result;
}

bool AshmanixTimer::SetTime(const char *stringTime)
{
	bool result = uiManager->SetTime(stringTime);
	emit RequestTimerReset();

	return result;
}
// --------------------------------- Public Slots ----------------------------------

// ------------------------------- Private Slots ----------------------------------

void AshmanixTimer::TimerAdjust()
{
	// Flag for ending timer
	bool endTimer = false;
	bool isCountingDown = !countdownTimerData.shouldCountUp;
	long long timerPeriodMillis = countdownTimerData.timeLeftInMillis;

	if (isCountingDown) {
		// Counting down
		if (countdownTimerData.selectedCountdownType == CountdownType::PERIOD) {
			// If selected tab is period
			if (countdownTimerData.smoothenPeriodTimer) {
				timerPeriodMillis -= TIMERPERIOD;
			} else {
				timerPeriodMillis = static_cast<long long>(
					QDateTime::currentDateTime().msecsTo(countdownTimerData.timeAtTimerStart));
			}
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
		if (countdownTimerData.selectedCountdownType == CountdownType::PERIOD) {
			if (countdownTimerData.smoothenPeriodTimer) {
				timerPeriodMillis += TIMERPERIOD;
			} else {
				timerPeriodMillis = static_cast<long long>(
					countdownTimerData.timeAtTimerStart.msecsTo(QDateTime::currentDateTime()));
			}
			// If selected tab is period
			if (timerPeriodMillis >= uiManager->GetMillisFromPeriodUI())
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

		uiManager->UpdateDateTimeDisplay(countdownTimerData.timeLeftInMillis);

		// Send tick event
		QString timeString = uiManager->ConvertDateTimeToFormattedDisplayString(
			countdownTimerData.timeLeftInMillis, countdownTimerData.display.showLeadingZero);

		websocketNotifier->SendTickEvent(countdownTimerData.timerId, timeString,
						 countdownTimerData.timeLeftInMillis);
	}

	if (endTimer == true) {
		if (countdownTimerData.display.showEndMessage) {
			QString outputEndMessageString = countdownTimerData.display.endMessage;
			QString timeString = uiManager->ConvertDateTimeToFormattedDisplayString(
				countdownTimerData.timeLeftInMillis, countdownTimerData.display.showLeadingZero);
			outputEndMessageString.replace(TIMETEMPLATECODE, timeString);

			SetSourceText(outputEndMessageString.toStdString().c_str());
		}
		if (countdownTimerData.display.showEndScene) {
			SetCurrentScene();
		}
		if (isCountingDown) {
			ui->timeDisplay->display(AshmanixTimer::ZEROSTRING);
			countdownTimerData.timeLeftInMillis = 0;
		} else {
			if (countdownTimerData.selectedCountdownType == CountdownType::PERIOD) {
				countdownTimerData.timeLeftInMillis = uiManager->GetMillisFromPeriodUI();
			} else {
				countdownTimerData.timeLeftInMillis =
					countdownTimerData.timeAtTimerStart.msecsTo(ui->dateTimeEdit->dateTime());
			}
			uiManager->UpdateDateTimeDisplay(countdownTimerData.timeLeftInMillis);
		}
		// Send completion event
		websocketNotifier->SendStateEvent(countdownTimerData.timerId, "completed",
						  countdownTimerData.source.selectedSource);
		StopTimerCounting();
		return;
	}
}

void AshmanixTimer::HandleTimerReset(bool restartOnly)
{
	if (countdownTimerData.timer && countdownTimerData.timer->isActive()) {
		if (restartOnly) {
			timerEngine->Start();
		} else {
			switch (countdownTimerData.selectedCountdownType) {
			case CountdownType::PERIOD:
				uiManager->HandleTimerAction(TimerAction::Reset);
				uiManager->HandleTimerAction(TimerAction::Play);
				break;

			case CountdownType::DATETIME:
				uiManager->HandleTimerAction(TimerAction::ToTimePlay);
				break;

			default:
				break;
			}
		}
	}
}

void AshmanixTimer::EmitMoveTimerDownSignal()
{
	emit MoveTimer(QString("down"), countdownTimerData.timerId);
}

void AshmanixTimer::EmitMoveTimerUpSignal()
{
	emit MoveTimer(QString("up"), countdownTimerData.timerId);
}
