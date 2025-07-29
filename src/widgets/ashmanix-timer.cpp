#include "ashmanix-timer.hpp"
#include "../utils/websocket-notifier.hpp"

AshmanixTimer::AshmanixTimer(QWidget *parent, WebsocketNotifier *websocketNotifier, obs_data_t *savedData,
			     CountdownDockWidget *mDockWidget)
	: QWidget(parent),
	  websocketNotifier(websocketNotifier),
	  ui(new Ui::AshmanixTimer)
{
	// Register custom type for signals and slots
	qRegisterMetaType<obs_data_t *>("obs_data_t*");
	qRegisterMetaType<obs_data_t *>("CountdownDockWidget*");

	mainDockWidget = mDockWidget;

	countdownTimerData = TimerWidgetStruct();

	ui->setupUi(this);

	uiManager = new TimerUIManager(this, ui, &countdownTimerData, mDockWidget, ZEROSTRING);
	timerPersistence = new TimerPersistence();
	hotkeyManager = new HotkeyManager(this, ui, &countdownTimerData);

	if (savedData) {
		timerPersistence->LoadTimerWidgetDataFromOBSSaveData(&countdownTimerData, savedData);
		uiManager->SetTimeUI();
		InitialiseTimerTime(false);
	} else {
		InitialiseTimerTime();
	}

	this->setProperty("id", countdownTimerData.timerId);

	ConnectSignalHandlers();

	if (countdownTimerData.timerId.size() == 0) {
		// Create a unique ID for the timer
		QUuid uuid = QUuid::createUuid();
		QByteArray hash = QCryptographicHash::hash(uuid.toByteArray(), QCryptographicHash::Md5);
		QString newTimerId = QString(hash.toHex().left(8)); // We take the first 8 characters of the hash
		SetTimerID(newTimerId);
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
	uiManager->SetTimerIDLabel(newId);
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
			uiManager->HandleTimerAction(TimerAction::RESET);

		uiManager->HandleTimerAction(TimerAction::PLAY);
		break;

	case CountdownType::DATETIME:
		uiManager->HandleTimerAction(TimerAction::TO_TIME_PLAY);
		break;
	}
}

void AshmanixTimer::StopTimer()
{
	switch (countdownTimerData.selectedCountdownType) {
	case CountdownType::PERIOD:
		uiManager->HandleTimerAction(TimerAction::RESET);
		break;

	case CountdownType::DATETIME:
		uiManager->HandleTimerAction(TimerAction::TO_TIME_STOP);
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
	QObject::connect(uiManager, &TimerUIManager::MoveTimer, this,
			 [this](Direction direction) { emit MoveTimer(direction, countdownTimerData.timerId); });

	QObject::connect(uiManager, &TimerUIManager::RequestDelete, this,
			 [this]() { emit RequestDelete(countdownTimerData.timerId); });

	QObject::connect(uiManager, &TimerUIManager::TimerChange, this, [this](TimerCommand timerCommand) {
		switch (timerCommand) {
		case TimerCommand::START:
			StartTimerCounting();
			break;
		case TimerCommand::STOP:
			StopTimerCounting();
			break;

		default:
			break;
		};
	});
}

void AshmanixTimer::StartTimerCounting()
{
	countdownTimerData.isPlaying = true;
	countdownTimerData.timer->start(TIMERPERIOD);
	uiManager->TimerStateChange(TimerCommand::START);

	websocketNotifier->SendStateEvent(countdownTimerData.timerId, "started",
					  countdownTimerData.source.selectedSource);
}

void AshmanixTimer::StopTimerCounting()
{
	countdownTimerData.isPlaying = false;
	countdownTimerData.timer->stop();
	uiManager->TimerStateChange(TimerCommand::STOP);

	websocketNotifier->SendStateEvent(countdownTimerData.timerId, "stopped",
					  countdownTimerData.source.selectedSource);
}

void AshmanixTimer::InitialiseTimerTime(bool setTimeLeftToUI)
{
	countdownTimerData.timer = new QTimer();
	QObject::connect(countdownTimerData.timer, SIGNAL(timeout()), SLOT(TimerAdjust()));
	QObject::connect(this, &AshmanixTimer::RequestTimerReset, this, &AshmanixTimer::HandleTimerReset);
	if (setTimeLeftToUI)
		countdownTimerData.timeLeftInMillis = uiManager->GetMillisFromPeriodUI();
}

void AshmanixTimer::UpdateDateTimeDisplay(long long timeInMillis)
{

	long long timeToUpdateInMillis = std::max(timeInMillis, 0ll);
	uiManager->UpdateDisplay(timeToUpdateInMillis);
	QString formattedDisplayTime = ConvertDateTimeToFormattedDisplayString(
		timeToUpdateInMillis, countdownTimerData.display.showLeadingZero);

	QString outputString = formattedDisplayTime;

	if (countdownTimerData.display.useFormattedOutput) {
		outputString = countdownTimerData.display.outputStringFormat;
		outputString.replace(TIMETEMPLATECODE, formattedDisplayTime);
	}

	SetSourceText(outputString);
}

QString AshmanixTimer::ConvertDateTimeToFormattedDisplayString(long long timeInMillis, bool showLeadingZero)
{
	QString formattedDateTimeString =
		GetFormattedTimerString(countdownTimerData.display.showDays, countdownTimerData.display.showHours,
					countdownTimerData.display.showMinutes, countdownTimerData.display.showSeconds,
					showLeadingZero, timeInMillis);

	return (formattedDateTimeString == "") ? "Nothing selected!" : formattedDateTimeString;
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
				QDateTime::currentDateTime().msecsTo(uiManager->GetToDateTimeValue()));
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
			if ((countdownTimerData.timeAtTimerStart.msecsTo(uiManager->GetToDateTimeValue())) -
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
		QString timeString = ConvertDateTimeToFormattedDisplayString(
			countdownTimerData.timeLeftInMillis, countdownTimerData.display.showLeadingZero);

		websocketNotifier->SendTickEvent(countdownTimerData.timerId, timeString,
						 countdownTimerData.timeLeftInMillis);
	}

	if (endTimer == true) {
		if (countdownTimerData.display.showEndMessage) {
			QString outputEndMessageString = countdownTimerData.display.endMessage;
			QString timeString = ConvertDateTimeToFormattedDisplayString(
				countdownTimerData.timeLeftInMillis, countdownTimerData.display.showLeadingZero);
			outputEndMessageString.replace(TIMETEMPLATECODE, timeString);

			SetSourceText(outputEndMessageString.toStdString().c_str());
		}
		if (countdownTimerData.display.showEndScene) {
			SetCurrentScene();
		}
		if (isCountingDown) {
			uiManager->SetZeroTimeDisplay();
			countdownTimerData.timeLeftInMillis = 0;
		} else {
			if (countdownTimerData.selectedCountdownType == CountdownType::PERIOD) {
				countdownTimerData.timeLeftInMillis = uiManager->GetMillisFromPeriodUI();
			} else {
				countdownTimerData.timeLeftInMillis =
					countdownTimerData.timeAtTimerStart.msecsTo(uiManager->GetToDateTimeValue());
			}
			UpdateDateTimeDisplay(countdownTimerData.timeLeftInMillis);
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
			countdownTimerData.timer->start();
		} else {
			switch (countdownTimerData.selectedCountdownType) {
			case CountdownType::PERIOD:
				uiManager->HandleTimerAction(TimerAction::RESET);
				uiManager->HandleTimerAction(TimerAction::PLAY);
				break;

			case CountdownType::DATETIME:
				uiManager->HandleTimerAction(TimerAction::TO_TIME_PLAY);
				break;

			default:
				break;
			}
		}
	}
}
