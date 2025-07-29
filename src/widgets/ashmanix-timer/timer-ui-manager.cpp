#include "timer-ui-manager.hpp"
#include <obs.h>
#include <obs.hpp>
#include "../../ui/ui_AshmanixTimer.h"
#include "../../utils/timer-utils.hpp"
#include "../ashmanix-timer.hpp"
#include "../settings-dialog.hpp"
#include "countdown-widget.hpp"

TimerUIManager::TimerUIManager(QWidget *parent, Ui::AshmanixTimer *ui, TimerWidgetStruct *countdownTimerData,
			       CountdownDockWidget *countdownDockWidget, const char *zeroString)
	: parent(parent),
	  ui(ui),
	  data(countdownTimerData),
	  countdownDockWidget(countdownDockWidget),
	  zeroString(zeroString)
{
	SetupUI();
	ConnectUISignalHandlers();
}
// ------------------------------ Public Functions ----------------------------------
void TimerUIManager::SetupUI()
{
	ui->timerNameLabel->setText(QString("Timer: %1").arg(data->timerId));

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

	ui->timeDisplay->display(zeroString);

	ui->dateTimeEdit->setMinimumDate(QDate::currentDate());
	ui->dateTimeEdit->setMaximumDate(QDate::currentDate().addDays(999));

	ui->timerDays->setRange(0, 999);
	ui->timerDays->setToolTip(obs_module_text("DaysCheckboxLabel"));

	ui->timerHours->setRange(0, 23);
	ui->timerHours->setToolTip(obs_module_text("HoursCheckboxLabel"));

	ui->timerMinutes->setRange(0, 59);
	ui->timerMinutes->setToolTip(obs_module_text("MinutesCheckboxLabel"));

	ui->timerSeconds->setRange(0, 59);
	ui->timerSeconds->setToolTip(obs_module_text("SecondsCheckboxLabel"));

	data->periodVLayout = ui->periodWidget;
	ui->periodToolButton->setText(obs_module_text("SetPeriodTabLabel"));
	ui->periodToolButton->setToolTip(obs_module_text("SetPeriodTabTip"));
	data->datetimeVLayout = ui->datetimeWidget;
	ui->datetimeToolButton->setText(obs_module_text("SetDatetimeTabLabel"));
	ui->datetimeToolButton->setToolTip(obs_module_text("SetDatetimeTabTip"));
	UpdateStyles();
	ToggleTimeType(data->selectedCountdownType);

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

	data->isPlaying = false;
}

void TimerUIManager::ConnectUISignalHandlers()
{
	QObject::connect(ui->playButton, &QPushButton::clicked, this,
			 [this]() { HandleTimerAction(TimerAction::PLAY); });

	QObject::connect(ui->pauseButton, &QPushButton::clicked, this,
			 [this]() { HandleTimerAction(TimerAction::PAUSE); });

	QObject::connect(ui->resetButton, &QPushButton::clicked, this,
			 [this]() { HandleTimerAction(TimerAction::RESET); });

	QObject::connect(ui->toTimePlayButton, &QPushButton::clicked, this,
			 [this]() { HandleTimerAction(TimerAction::TO_TIME_PLAY); });

	QObject::connect(ui->toTimeStopButton, &QPushButton::clicked, this,
			 [this]() { HandleTimerAction(TimerAction::TO_TIME_STOP); });

	QObject::connect(ui->deleteToolButton, &QPushButton::clicked, this, &TimerUIManager::DeleteButtonClicked);

	QObject::connect(ui->settingsToolButton, &QPushButton::clicked, this, &TimerUIManager::SettingsButtonClicked);

	QObject::connect(ui->timerDays, &QSpinBox::valueChanged, this,
			 [this](int value) { HandleTimerValueChange("days", value); });

	QObject::connect(ui->timerHours, &QSpinBox::valueChanged, this,
			 [this](int value) { HandleTimerValueChange("hours", value); });

	QObject::connect(ui->timerMinutes, &QSpinBox::valueChanged, this,
			 [this](int value) { HandleTimerValueChange("minutes", value); });

	QObject::connect(ui->timerSeconds, &QSpinBox::valueChanged, this,
			 [this](int value) { HandleTimerValueChange("seconds", value); });

	QObject::connect(ui->dateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &TimerUIManager::DateTimeChanged);

	QObject::connect(ui->moveUpToolButton, &QPushButton::clicked, this,
			 [this]() { emit MoveTimer(Direction::UP); });

	QObject::connect(ui->moveDownToolButton, &QPushButton::clicked, this,
			 [this]() { emit MoveTimer(Direction::DOWN); });

	QObject::connect(ui->periodToolButton, &QPushButton::clicked, this,
			 [this]() { ToggleTimeType(CountdownType::PERIOD); });

	QObject::connect(ui->datetimeToolButton, &QPushButton::clicked, this,
			 [this]() { ToggleTimeType(CountdownType::DATETIME); });
}

void TimerUIManager::SetTimeUI()
{
	ui->dateTimeEdit->setDateTime(data->dateTime);

	ui->timerDays->setValue(data->periodDuration.days);
	ui->timerHours->setValue(data->periodDuration.hours);
	ui->timerMinutes->setValue(data->periodDuration.minutes);
	ui->timerSeconds->setValue(data->periodDuration.seconds);

	ui->timerNameLabel->setText(QString("Timer: %1").arg(data->timerId));

	ToggleTimeType(data->selectedCountdownType);

	UpdateDisplay(data->timeLeftInMillis);
}

bool TimerUIManager::AddTime(const char *stringTime, bool isCountingUp)
{
	bool result = false;
	QDateTime currentDateTime = QDateTime::currentDateTime();
	long long timeInMillis = ConvertStringPeriodToMillis(stringTime);

	long long newTimeLeftInMillis = std::max((data->timeLeftInMillis + timeInMillis), 0LL);

	if (data->selectedCountdownType == CountdownType::PERIOD) {
		long long uiPeriodInMillis = GetMillisFromPeriodUI();
		uiPeriodInMillis = std::max((uiPeriodInMillis + timeInMillis), 0LL);

		PeriodData periodData = ConvertMillisToPeriodData(uiPeriodInMillis);
		UpdateTimerPeriod(periodData);

		result = true;
	} else if (data->selectedCountdownType == CountdownType::DATETIME) {
		QDateTime updatedDateTime;
		updatedDateTime = ui->dateTimeEdit->dateTime().addMSecs(timeInMillis);
		ui->dateTimeEdit->setDateTime(updatedDateTime);
		result = true;
	}

	if (!isCountingUp) {
		data->timeAtTimerStart = data->timeAtTimerStart.addMSecs(timeInMillis);
		// If we are using the smooth period timer option then we need to set time left in millis
		if (data->selectedCountdownType == CountdownType::PERIOD && data->smoothenPeriodTimer)
			data->timeLeftInMillis = newTimeLeftInMillis;

		UpdateDisplay(newTimeLeftInMillis);
	}

	return result;
}

bool TimerUIManager::SetTime(const char *stringTime)
{
	long long timeInMillis = ConvertStringPeriodToMillis(stringTime);

	if (timeInMillis < 0)
		return false;

	bool result = false;
	QDateTime currentDateTime = QDateTime::currentDateTime();

	if (data->selectedCountdownType == CountdownType::PERIOD) {
		PeriodData periodData = ConvertMillisToPeriodData(timeInMillis);
		UpdateTimerPeriod(periodData);
		result = true;
	} else if (data->selectedCountdownType == CountdownType::DATETIME) {
		QDateTime updatedDateTime;
		updatedDateTime = currentDateTime.addMSecs(timeInMillis + TIMERPERIOD);
		ui->dateTimeEdit->setDateTime(updatedDateTime);
		result = true;
	}

	return result;
}

void TimerUIManager::UpdateStyles()
{
	// Set toolbutton colour checked to darkened colour
	QColor bgColor = ui->periodToolButton->palette().color(QPalette::Button);
	QColor darkenedColor = bgColor.darker(150);
	parent->setStyleSheet(QString("QToolButton:checked { background-color: %1; } ").arg(darkenedColor.name()));
}

void TimerUIManager::ToggleTimeType(CountdownType type)
{
	if (data->periodVLayout && data->datetimeVLayout) {
		data->datetimeVLayout->hide();
		data->periodVLayout->hide();
		ui->periodToolButton->setChecked(false);
		ui->datetimeToolButton->setChecked(false);

		if (type == CountdownType::PERIOD) {
			data->periodVLayout->show();
			data->selectedCountdownType = type;
			ui->periodToolButton->setChecked(true);
		} else if (type == CountdownType::DATETIME) {
			data->datetimeVLayout->show();
			data->selectedCountdownType = type;
			ui->datetimeToolButton->setChecked(true);
		}
	} else {
		obs_log(LOG_WARNING, "Period and/or Datetime layouts not found!");
	}
}

long long TimerUIManager::GetMillisFromPeriodUI()
{
	long long days_ms = static_cast<long long>(ui->timerDays->text().toInt()) * 24 * 60 * 60 * 1000;
	long long hours_ms = static_cast<long long>(ui->timerHours->text().toInt()) * 60 * 60 * 1000;
	long long minutes_ms = static_cast<long long>(ui->timerMinutes->text().toInt()) * 60 * 1000;
	long long seconds_ms = static_cast<long long>(ui->timerSeconds->text().toInt()) * 1000;

	return days_ms + hours_ms + minutes_ms + seconds_ms;
}

QDateTime TimerUIManager::GetToTimeEditDateTime()
{
	return ui->dateTimeEdit->dateTime();
}

void TimerUIManager::UpdateTimeDisplayTooltip()
{
	QString detailsTooltip = "";

	detailsTooltip += obs_module_text("TextSourceLabel");
	detailsTooltip += " : ";
	detailsTooltip += data->source.selectedSource;
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("StartOnStreamCheckBoxLabel");
	detailsTooltip += " : ";
	detailsTooltip += data->startOnStreamStart ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("ResetOnStreamStartCheckBoxLabel");
	detailsTooltip += " : ";
	detailsTooltip += data->resetTimerOnStreamStart ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("EndMessageLabel");
	detailsTooltip += " : ";
	if (data->display.showEndMessage) {
		detailsTooltip += "✓ ";
		detailsTooltip += data->display.endMessage;
	} else {
		detailsTooltip += "-";
	}
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("SwitchScene");
	detailsTooltip += " : ";
	if (data->display.showEndScene) {
		detailsTooltip += "✓ ";
		detailsTooltip += data->source.selectedScene;
	} else {
		detailsTooltip += "-";
	}
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("DaysCheckboxLabel");
	detailsTooltip += " : ";
	detailsTooltip += data->display.showDays ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("HoursCheckboxLabel");
	detailsTooltip += " : ";
	detailsTooltip += data->display.showHours ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("MinutesCheckboxLabel");
	detailsTooltip += " : ";
	detailsTooltip += data->display.showMinutes ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("SecondsCheckboxLabel");
	detailsTooltip += " : ";
	detailsTooltip += data->display.showSeconds ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("LeadZeroCheckboxLabel");
	detailsTooltip += " : ";
	detailsTooltip += data->display.showLeadingZero ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("CountUpCheckBoxLabel");
	detailsTooltip += " : ";
	detailsTooltip += data->shouldCountUp ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("DialogFormatOutputLabel");
	detailsTooltip += " : ";
	detailsTooltip += data->display.useFormattedOutput ? "✓" : "-";
	detailsTooltip += "\n";

	detailsTooltip += obs_module_text("DialogSmoothTimerLabel");
	detailsTooltip += " : ";
	detailsTooltip += data->smoothenPeriodTimer ? "✓" : "-";

	ui->timeDisplay->setToolTip(detailsTooltip);
}

void TimerUIManager::UpdateDisplay(long long timeToUpdateInMillis)
{
	ui->timeDisplay->display(ConvertMillisToDateTimeString(timeToUpdateInMillis));
}

bool TimerUIManager::IsSetTimeZero()
{
	bool isZero = false;

	if (data->timeLeftInMillis == 0) {
		isZero = true;
	} else if (ui->timerDays->text().toInt() == 0 && ui->timerHours->text().toInt() == 0 &&
		   ui->timerMinutes->text().toInt() == 0 && ui->timerSeconds->text().toInt() == 0) {
		isZero = true;
	}

	return isZero;
}

void TimerUIManager::UpdateTimerPeriod(PeriodData periodData)
{
	data->periodDuration.days = periodData.days;
	data->periodDuration.hours = periodData.hours;
	data->periodDuration.minutes = periodData.minutes;
	data->periodDuration.seconds = periodData.seconds;

	ui->timerDays->setValue(data->periodDuration.days);
	ui->timerHours->setValue(data->periodDuration.hours);
	ui->timerMinutes->setValue(data->periodDuration.minutes);
	ui->timerSeconds->setValue(data->periodDuration.seconds);
}

void TimerUIManager::SetHideMultiTimerUIButtons(bool shouldHide)
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

void TimerUIManager::SetIsUpButtonDisabled(bool isDisabled)
{
	ui->moveUpToolButton->setDisabled(isDisabled);
}

void TimerUIManager::SetIsDownButtonDisabled(bool isDisabled)
{
	ui->moveDownToolButton->setDisabled(isDisabled);
}

void TimerUIManager::TimerStateChange(TimerCommand command)
{
	if (command == TimerCommand::START) {
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

	} else if (command == TimerCommand::STOP) {
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
	}
}

void TimerUIManager::SetZeroTimeDisplay()
{
	ui->timeDisplay->display(zeroString);
}

QDateTime TimerUIManager::GetToDateTimeValue()
{
	return ui->dateTimeEdit->dateTime();
}

void TimerUIManager::SetTimerIDLabel(QString newId)
{
	ui->timerNameLabel->setText(QString("Timer: %1").arg(newId));
}

// --------------------------------- Public Slots ----------------------------------

void TimerUIManager::HandleTimerAction(TimerAction action)
{
	QDateTime currentDateTime = QDateTime::currentDateTime();
	long long periodUIInMillis = GetMillisFromPeriodUI();

	switch (action) {
	case TimerAction::PLAY:
		if (data->selectedCountdownType == CountdownType::DATETIME) {
			ToggleTimeType(CountdownType::PERIOD);
		}
		UpdateDisplay(data->timeLeftInMillis);

		if ((!data->shouldCountUp && IsSetTimeZero()) ||
		    (data->shouldCountUp && data->timeLeftInMillis >= periodUIInMillis))
			return;

		if (data->shouldCountUp) {
			data->timeAtTimerStart = currentDateTime.addMSecs(-(data->timeLeftInMillis));
		} else {
			data->timeAtTimerStart = currentDateTime.addMSecs(data->timeLeftInMillis);
		}

		emit TimerChange(TimerCommand::START);
		break;
	case TimerAction::PAUSE:
		if (data->selectedCountdownType == CountdownType::DATETIME) {
			ToggleTimeType(CountdownType::PERIOD);
		}

		emit TimerChange(TimerCommand::STOP);
		break;
	case TimerAction::RESET:
		if (data->selectedCountdownType == CountdownType::DATETIME) {
			ToggleTimeType(CountdownType::PERIOD);
		}

		emit TimerChange(TimerCommand::STOP);
		data->shouldCountUp ? data->timeLeftInMillis = 0 : data->timeLeftInMillis = GetMillisFromPeriodUI();

		UpdateDisplay(data->timeLeftInMillis);
		break;
	case TimerAction::TO_TIME_PLAY:
		if (data->selectedCountdownType == CountdownType::PERIOD) {
			ToggleTimeType(CountdownType::DATETIME);
		}

		data->timeAtTimerStart = QDateTime::currentDateTime();

		if (data->shouldCountUp) {
			data->timeLeftInMillis = 0;
		} else {
			data->timeLeftInMillis = data->timeAtTimerStart.msecsTo(ui->dateTimeEdit->dateTime());
			if (data->timeLeftInMillis < 0)
				data->timeLeftInMillis = 0;
		}

		UpdateDisplay(data->timeLeftInMillis);
		emit TimerChange(TimerCommand::START);
		break;
	case TimerAction::TO_TIME_STOP:
		if (data->selectedCountdownType == CountdownType::PERIOD) {
			ToggleTimeType(CountdownType::DATETIME);
		}

		emit TimerChange(TimerCommand::STOP);
		break;
	}
}

// --------------------------------- Private Slots -----------------------------------

void TimerUIManager::SettingsButtonClicked()
{
	if (!settingsDialogUi) {
		settingsDialogUi = QSharedPointer<SettingsDialog>::create(this, data, countdownDockWidget);

		QObject::connect(settingsDialogUi.data(), &SettingsDialog::SettingsUpdated, this,
				 [this]() { UpdateTimeDisplayTooltip(); });
	}
	if (settingsDialogUi->isVisible()) {
		settingsDialogUi->raise();
		settingsDialogUi->activateWindow();
	} else {
		settingsDialogUi->setVisible(true);
	}
}

void TimerUIManager::DeleteButtonClicked()
{
	emit RequestDelete(data->timerId);
}

void TimerUIManager::HandleTimerValueChange(const QString &type, int value)
{
	if (type == "days") {
		data->periodDuration.days = value;
	} else if (type == "hours") {
		data->periodDuration.hours = value;
	} else if (type == "minutes") {
		data->periodDuration.minutes = value;
	} else if (type == "seconds") {
		data->periodDuration.seconds = value;
	} else {
		obs_log(LOG_WARNING, "Unknown time type: %s", type.toStdString().c_str());
		return;
	}
}

void TimerUIManager::DateTimeChanged(QDateTime newDateTime)
{
	data->dateTime = newDateTime;
}

// ------------------------------ Private Functions ----------------------------------
