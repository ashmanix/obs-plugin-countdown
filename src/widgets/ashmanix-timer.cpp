#include "ashmanix-timer.hpp"

AshmanixTimer::AshmanixTimer(QWidget *parent, QString id,
			     obs_websocket_vendor vendor)
	: QWidget(parent),
	  ui(new Ui::AshmanixTimer)
{
	countdownTimerData = new TimerWidgetStruct;

	ui->setupUi(this);
	countdownTimerData->timerId = id;
	this->setProperty("id", countdownTimerData->timerId);
	vendor = vendor;

#if __APPLE__
	// ui->hsDaysLeft->changeSize(0, 20, QSizePolicy::Fixed,
	// 			   QSizePolicy::Fixed);
	// ui->hsDaysRight->changeSize(0, 20, QSizePolicy::Fixed,
	// 			    QSizePolicy::Fixed);
	// ui->hsHoursLeft->changeSize(0, 20, QSizePolicy::Fixed,
	// 			    QSizePolicy::Fixed);
	// ui->hsHoursRight->changeSize(0, 20, QSizePolicy::Fixed,
	// 			     QSizePolicy::Fixed);
	// ui->hsMinutesLeft->changeSize(0, 20, QSizePolicy::Fixed,
	// 			      QSizePolicy::Fixed);
	// ui->hsMinutesRight->changeSize(0, 20, QSizePolicy::Fixed,
	// 			       QSizePolicy::Fixed);
	// ui->hsSecondsLeft->changeSize(0, 20, QSizePolicy::Fixed,
	// 			      QSizePolicy::Fixed);
	// ui->hsSecondsRight->changeSize(0, 20, QSizePolicy::MinimumExpanding,
	// 			       QSizePolicy::MinimumExpanding);
#endif

	SetupTimerWidgetUI(countdownTimerData);

	ConnectUISignalHandlers();

	InitialiseTimerTime(countdownTimerData);
}

AshmanixTimer::~AshmanixTimer()
{
	this->deleteLater();
}

QString AshmanixTimer::GetTimerID()
{
	return countdownTimerData->timerId;
}

QPushButton *AshmanixTimer::GetDeleteButton()
{
	return ui->deleteButton;
}

void AshmanixTimer::SetupTimerWidgetUI(TimerWidgetStruct *countdownStruct)
{
	TimerWidgetStruct *context = countdownStruct;
	ui->timerNameLabel->setText(
		QString("Timer: %1").arg(countdownStruct->timerId));

	ui->settingsButton->setProperty("themeID", "propertiesIconSmall");
	ui->settingsButton->setText("");
	ui->settingsButton->setEnabled(true);
	ui->settingsButton->setToolTip(obs_module_text("AddTimerButtonTip"));

	ui->deleteButton->setProperty("themeID", "removeIconSmall");
	ui->deleteButton->setText("");
	ui->deleteButton->setEnabled(true);
	ui->deleteButton->setToolTip(obs_module_text("DeleteTimerButtonTip"));

	ui->timeDisplay->display(AshmanixTimer::ZEROSTRING);

	ui->dateTimeEdit->setMinimumDate(QDate::currentDate());
	ui->dateTimeEdit->setMaximumDate(QDate::currentDate().addDays(999));

	ui->timerDays->setMaxLength(3);
	ui->timerDays->setValidator(new QRegularExpressionValidator(
		QRegularExpression("^(0|[1-9]\\d{0,2})$"), this));
	ui->timerDays->setToolTip(obs_module_text("DaysCheckboxLabel"));

	ui->timerHours->setMaxLength(2);
	ui->timerHours->setValidator(new QRegularExpressionValidator(
		QRegularExpression("^(0?[0-9]|1[0-9]|2[0-3])$"), this));
	ui->timerHours->setToolTip(obs_module_text("HoursCheckboxLabel"));

	ui->timerMinutes->setMaxLength(2);
	ui->timerMinutes->setValidator(new QRegularExpressionValidator(
		QRegularExpression("^[1-5]?[0-9]"), this));
	ui->timerMinutes->setToolTip(obs_module_text("MinutesCheckboxLabel"));

	ui->timerSeconds->setAlignment(Qt::AlignCenter);
	ui->timerSeconds->setMaxLength(2);
	ui->timerSeconds->setValidator(new QRegularExpressionValidator(
		QRegularExpression("^[1-5]?[0-9]"), this));
	ui->timerSeconds->setToolTip(obs_module_text("SecondsCheckboxLabel"));

	ui->countdownTypeTabWidget->setTabText(
		0, obs_module_text("SetPeriodTabLabel"));
	ui->countdownTypeTabWidget->setTabText(
		1, obs_module_text("SetDatetimeTabLabel"));

	ui->countdownTypeTabWidget->setTabToolTip(
		0, obs_module_text("SetPeriodTabTip"));
	ui->countdownTypeTabWidget->setTabToolTip(
		1, obs_module_text("SetDatetimeTabTip"));

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

void AshmanixTimer::ConnectUISignalHandlers()
{
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

	QObject::connect(ui->deleteButton, SIGNAL(clicked()),
			 SLOT(DeleteButtonClicked()));

	QObject::connect(ui->settingsButton, SIGNAL(clicked()),
			 SLOT(SettingsButtonClicked()));
}

QString
AshmanixTimer::ConvertDateTimeToFormattedDisplayString(long long timeInMillis,
						       bool showLeadingZero)
{

	QString formattedDateTimeString = GetFormattedTimerString(
		countdownTimerData->showDays, countdownTimerData->showHours,
		countdownTimerData->showMinutes,
		countdownTimerData->showSeconds, showLeadingZero, timeInMillis);

	return (formattedDateTimeString == "") ? "Nothing selected!"
					       : formattedDateTimeString;
}

void AshmanixTimer::StartTimerCounting(TimerWidgetStruct *context)
{
	context->isPlaying = true;
	context->timer->start(COUNTDOWNPERIOD);
	ui->playButton->setEnabled(false);
	ui->pauseButton->setEnabled(true);
	ui->resetButton->setEnabled(false);

	ui->toTimePlayButton->setEnabled(false);
	ui->toTimeStopButton->setEnabled(true);

	ui->timerDays->setEnabled(false);
	ui->timerHours->setEnabled(false);
	ui->timerMinutes->setEnabled(false);
	ui->timerSeconds->setEnabled(false);

	ui->countdownTypeTabWidget->tabBar()->setEnabled(false);
	ui->dateTimeEdit->setEnabled(false);

	SendTimerStateEvent(context->timerId, "started");
}

void AshmanixTimer::StopTimerCounting(TimerWidgetStruct *context)
{
	context->isPlaying = false;
	context->timer->stop();
	ui->playButton->setEnabled(true);
	ui->pauseButton->setEnabled(false);
	ui->resetButton->setEnabled(true);

	ui->toTimePlayButton->setEnabled(true);
	ui->toTimeStopButton->setEnabled(false);

	ui->timerDays->setEnabled(true);
	ui->timerHours->setEnabled(true);
	ui->timerMinutes->setEnabled(true);
	ui->timerSeconds->setEnabled(true);

	ui->countdownTypeTabWidget->tabBar()->setEnabled(true);
	ui->dateTimeEdit->setEnabled(true);

	SendTimerStateEvent(context->timerId, "stopped");
}

void AshmanixTimer::InitialiseTimerTime(TimerWidgetStruct *context)
{
	context->timer = new QTimer();
	QObject::connect(context->timer, SIGNAL(timeout()),
			 SLOT(TimerAdjust()));

	context->timeLeftInMillis = GetMillisFromPeriodUI();
}

bool AshmanixTimer::IsSetTimeZero(TimerWidgetStruct *context)
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

void AshmanixTimer::UpdateDateTimeDisplay(long long timeInMillis)
{
	ui->timeDisplay->display(ConvertMillisToDateTimeString(timeInMillis));
	QString formattedDisplayTime = ConvertDateTimeToFormattedDisplayString(
		timeInMillis, countdownTimerData->showLeadingZero);
	SetSourceText(formattedDisplayTime);
}

void AshmanixTimer::SetSourceText(QString newText)
{
	obs_source_t *selectedSource = obs_get_source_by_name(
		countdownTimerData->selectedSource.toStdString().c_str());

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

void AshmanixTimer::SetCurrentScene()
{
	if (countdownTimerData->selectedScene.length()) {
		obs_source_t *source = obs_get_source_by_name(
			countdownTimerData->selectedScene.toStdString().c_str());
		if (source != NULL) {
			obs_frontend_set_current_scene(source);
			obs_source_release(source);
		}
	}
}

long long AshmanixTimer::GetMillisFromPeriodUI()
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

void AshmanixTimer::SendWebsocketEvent(const char *eventName,
				       obs_data_t *eventData)
{
	if (!vendor)
		return;

	obs_websocket_vendor_emit_event(vendor, eventName, eventData);
}

void AshmanixTimer::SendTimerTickEvent(QString timerId,
				       long long timeLeftInMillis)
{
	obs_data_t *eventData = obs_data_create();

	// Convert milliseconds to readable format
	QString timeString = ConvertDateTimeToFormattedDisplayString(
		timeLeftInMillis, countdownTimerData->showLeadingZero);

	obs_data_set_string(eventData, "timer_id",
			    timerId.toStdString().c_str());
	obs_data_set_string(eventData, "time_display",
			    timeString.toStdString().c_str());
	obs_data_set_int(eventData, "time_left_ms", timeLeftInMillis);

	SendWebsocketEvent("timer_tick", eventData);
	obs_data_release(eventData);
}

void AshmanixTimer::SendTimerStateEvent(QString timerId, const char *state)
{
	obs_data_t *eventData = obs_data_create();
	obs_data_set_string(eventData, "timer_id",
			    timerId.toStdString().c_str());
	obs_data_set_string(eventData, "state", state);

	if (countdownTimerData->selectedSource.length() > 0) {
		obs_data_set_string(eventData, "text_source",
				    countdownTimerData->selectedSource
					    .toStdString()
					    .c_str());
	}

	SendWebsocketEvent("timer_state_changed", eventData);
	obs_data_release(eventData);
}

// void AshmanixTimer::DeleteRequested() {
//     emit RequestDelete(countdownTimerData->timerId);
// }

void AshmanixTimer::PlayButtonClicked()
{
	TimerWidgetStruct *context = countdownTimerData;

	if (ui->countdownTypeTabWidget->currentIndex() == 1) {
		ui->countdownTypeTabWidget->setCurrentIndex(0);
	}

	if ((!context->shouldCountUp && IsSetTimeZero(context)) ||
	    (context->shouldCountUp &&
	     context->timeLeftInMillis >= GetMillisFromPeriodUI()))
		return;

	ui->timeDisplay->display(
		ConvertMillisToDateTimeString(context->timeLeftInMillis));
	StartTimerCounting(context);
}

void AshmanixTimer::PauseButtonClicked()
{
	TimerWidgetStruct *context = countdownTimerData;

	if (ui->countdownTypeTabWidget->currentIndex() == 1) {
		ui->countdownTypeTabWidget->setCurrentIndex(0);
	}

	StopTimerCounting(context);
	SendTimerStateEvent(context->timerId, "paused");
}

void AshmanixTimer::ResetButtonClicked()
{
	TimerWidgetStruct *context = countdownTimerData;

	if (ui->countdownTypeTabWidget->currentIndex() == 1) {
		ui->countdownTypeTabWidget->setCurrentIndex(0);
	}

	StopTimerCounting(context);
	countdownTimerData->shouldCountUp
		? context->timeLeftInMillis = 0
		: context->timeLeftInMillis = GetMillisFromPeriodUI();

	UpdateDateTimeDisplay(context->timeLeftInMillis);
}

void AshmanixTimer::ToTimeStopButtonClicked()
{
	TimerWidgetStruct *context = countdownTimerData;

	if (ui->countdownTypeTabWidget->currentIndex() == 0) {
		ui->countdownTypeTabWidget->setCurrentIndex(1);
	}

	StopTimerCounting(context);
}

void AshmanixTimer::ToTimePlayButtonClicked()
{
	TimerWidgetStruct *context = countdownTimerData;

	if (ui->countdownTypeTabWidget->currentIndex() == 0) {
		ui->countdownTypeTabWidget->setCurrentIndex(1);
	}

	if (context->shouldCountUp) {
		context->timeToCountUpToStart = QDateTime::currentDateTime();
		context->timeLeftInMillis = 0;
	} else {
		context->timeLeftInMillis = CalcToCurrentDateTimeInMillis(
			ui->dateTimeEdit->dateTime(), COUNTDOWNPERIOD);
	}

	ui->timeDisplay->display(
		ConvertMillisToDateTimeString(context->timeLeftInMillis));
	StartTimerCounting(context);
}

void AshmanixTimer::SettingsButtonClicked()
{
	obs_log(LOG_INFO, "Settings button clicked for Timer %s",
		(countdownTimerData->timerId).toStdString().c_str());
	if (!settingsDialogUi) {
		settingsDialogUi =
			new SettingsDialog(this, countdownTimerData->timerId);
	}
	settingsDialogUi->setVisible(true);
}

void AshmanixTimer::DeleteButtonClicked()
{
	// obs_log(LOG_INFO, (QString("Delete button clicked for Timer %1")
	// 			   .arg(countdownTimerData->timerId))
	// 			  .toStdString()
	// 			  .c_str());
	emit RequestDelete(countdownTimerData->timerId);
}

void AshmanixTimer::TimerAdjust()
{
	TimerWidgetStruct *context = countdownTimerData;
	// Flag for ending timer
	bool endTimer = false;
	bool isCountingDown = !context->shouldCountUp;
	long long timerPeriodMillis = context->timeLeftInMillis;

	if (isCountingDown) {
		// Counting down
		if (ui->countdownTypeTabWidget->currentIndex() == 0) {
			// If selected tab is period
			timerPeriodMillis -= COUNTDOWNPERIOD;
		} else {
			// If selected tab is datetime
			timerPeriodMillis = CalcToCurrentDateTimeInMillis(
				ui->dateTimeEdit->dateTime(), COUNTDOWNPERIOD);
		}
		if (timerPeriodMillis < COUNTDOWNPERIOD)
			endTimer = true;
	} else {
		// When counting up always add to current timer

		// Check if we need to end timer
		if (ui->countdownTypeTabWidget->currentIndex() == 0) {
			timerPeriodMillis += COUNTDOWNPERIOD;
			// If selected tab is period
			if (timerPeriodMillis >= GetMillisFromPeriodUI())
				endTimer = true;
		} else {
			timerPeriodMillis =
				context->timeToCountUpToStart.msecsTo(
					QDateTime::currentDateTime());
			// If selected tab is datetime
			if ((context->timeToCountUpToStart.msecsTo(
				    ui->dateTimeEdit->dateTime())) -
				    timerPeriodMillis <=
			    COUNTDOWNPERIOD)
				endTimer = true;
		}
	}

	context->timeLeftInMillis = timerPeriodMillis;
	UpdateDateTimeDisplay(context->timeLeftInMillis);

	// Send tick event
	SendTimerTickEvent(context->timerId, context->timeLeftInMillis);

	if (endTimer == true) {
		if (context->showEndMessage) {
			SetSourceText(
				context->endMessage.toStdString().c_str());
		}
		if (context->showEndSource) {
			SetCurrentScene();
		}
		if (isCountingDown) {
			ui->timeDisplay->display(AshmanixTimer::ZEROSTRING);
			context->timeLeftInMillis = 0;
		} else {
			if (ui->countdownTypeTabWidget->currentIndex() == 0) {
				context->timeLeftInMillis =
					GetMillisFromPeriodUI();
			} else {
				context->timeLeftInMillis =
					context->timeToCountUpToStart.msecsTo(
						ui->dateTimeEdit->dateTime());
			}
			UpdateDateTimeDisplay(context->timeLeftInMillis);
		}
		// Send completion event
		SendTimerStateEvent(context->timerId, "completed");
		StopTimerCounting(context);
		return;
	}
}

void AshmanixTimer::HandleTimerReset()
{
	countdownTimerData->timer->stop();
	countdownTimerData->timer->start(COUNTDOWNPERIOD);
}
