#include "settings-dialog.hpp"

SettingsDialog::SettingsDialog(QWidget *parent, TimerWidgetStruct *tData, CountdownDockWidget *mWidget)
	: QDialog(parent),
	  ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);
	timerData = tData;
	mainWidget = mWidget;
	QString dialogTitle = QString("Timer %1").arg(timerData->timerId);
	this->setWindowTitle(dialogTitle);

	SetupDialogUI(timerData);

	ConnectUISignalHandlers();

	ConnectObsSignalHandlers();
}

SettingsDialog::~SettingsDialog()
{
	// Disconnect OBS signals before the dialog is destroyed
	signal_handler_disconnect(obs_get_signal_handler(), "source_create", OBSSourceCreated, ui);
	signal_handler_disconnect(obs_get_signal_handler(), "source_destroy", OBSSourceDeleted, ui);
	signal_handler_disconnect(obs_get_signal_handler(), "source_rename", OBSSourceRenamed, ui);

	this->deleteLater();
}

void SettingsDialog::SetCountUpCheckBoxEnabled(bool isEnabled)
{
	ui->countUpCheckBox->setEnabled(isEnabled);
}

void SettingsDialog::SetupDialogUI(TimerWidgetStruct *settingsDialogData)
{
	ui->daysCheckBox->setText(obs_module_text("DaysCheckboxLabel"));
	ui->daysCheckBox->setToolTip(obs_module_text("DaysCheckBoxTip"));

	ui->hoursCheckBox->setText(obs_module_text("HoursCheckboxLabel"));
	ui->hoursCheckBox->setToolTip(obs_module_text("HoursCheckBoxTip"));

	ui->minutesCheckBox->setText(obs_module_text("MinutesCheckboxLabel"));
	ui->minutesCheckBox->setToolTip(obs_module_text("MinutesCheckBoxTip"));

	ui->secondsCheckBox->setText(obs_module_text("SecondsCheckboxLabel"));
	ui->secondsCheckBox->setToolTip(obs_module_text("SecondsCheckBoxTip"));

	ui->leadZeroCheckBox->setText(obs_module_text("LeadZeroCheckboxLabel"));
	ui->leadZeroCheckBox->setToolTip(obs_module_text("LeadZeroCheckBoxTip"));

	ui->countUpCheckBox->setText(obs_module_text("CountUpCheckBoxLabel"));
	ui->countUpCheckBox->setToolTip(obs_module_text("CountUpCheckBoxTip"));

	ui->startOnStreamStartCheckBox->setText(obs_module_text("StartOnStreamCheckBoxLabel"));
	ui->startOnStreamStartCheckBox->setToolTip(obs_module_text("StartOnStreamCheckBoxTip"));

	ui->timerIdLineEdit->setToolTip(obs_module_text("timerIdLineEditTip"));
	ui->timerIdLabel->setText(obs_module_text("TimerIdLabel"));

	ui->textSourceDropdownList->setToolTip(obs_module_text("TextSourceDropdownTip"));
	ui->textSourceDropdownList->addItem("");
	ui->textSourceDropdownLabel->setText(obs_module_text("TextSourceLabel"));

	ui->endMessageCheckBox->setCheckState(Qt::Unchecked);
	ui->endMessageCheckBox->setToolTip(obs_module_text("EndMessageCheckBoxTip"));
	ui->endMessageCheckBox->setText(obs_module_text("EndMessageLabel"));

	ui->endMessageLineEdit->setEnabled(false);
	ui->endMessageLineEdit->setToolTip(obs_module_text("EndMessageLineEditTip"));

	ui->switchSceneCheckBox->setCheckState(Qt::Unchecked);
	ui->switchSceneCheckBox->setToolTip(obs_module_text("SwitchSceneCheckBoxTip"));
	ui->switchSceneCheckBox->setText(obs_module_text("SwitchScene"));

	ui->sceneSourceDropdownList->setEnabled(false);
	ui->sceneSourceDropdownList->setToolTip(obs_module_text("SceneSourceDropdownTip"));
	ui->sceneSourceDropdownList->addItem("");

	ui->formatOutputCheckBox->setText(obs_module_text("DialogFormatOutputLabel"));
	ui->formatOutputCheckBox->setToolTip(obs_module_text("FormatOutputTip"));

	ui->generalGroupBox->setTitle(obs_module_text("DialogGeneralGroupBoxTitle"));
	ui->timerStartGroupBox->setTitle(obs_module_text("DialogTimerStartGroupBoxTitle"));
	ui->timerEndGroupBox->setTitle(obs_module_text("DialogTimerEndGroupBoxTitle"));
	ui->timeFormatGroupBox->setTitle(obs_module_text("DialogTimeFormatGroupBoxTitle"));
	ui->timerTypeGroupBox->setTitle(obs_module_text("DialogTimerTypeGroupBoxTitle"));

	ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setText(obs_module_text("DialogButtonApplyLabel"));
	ui->dialogButtonBox->button(QDialogButtonBox::Ok)->setText(obs_module_text("DialogButtonOkLabel"));
	ui->dialogButtonBox->button(QDialogButtonBox::Cancel)->setText(obs_module_text("DialogButtonCancelLabel"));

	ui->byLabel->setText(obs_module_text("DialogInfoByLabel"));
	ui->contributorsLabel->setText(obs_module_text("DialogInfoConstributorsLabel"));
	ui->versionLabel->setText(obs_module_text("DialogInfoVersionLabel"));
	ui->versionTextLabel->setText(PLUGIN_VERSION);

	GetOBSSourceList();

	// Set form based on timer data
	SetFormDetails(settingsDialogData);
}

void SettingsDialog::GetOBSSourceList()
{
	// Get All Text Sources
	obs_enum_sources(GetTextSources, ui->textSourceDropdownList);

	char **sceneList = obs_frontend_get_scene_names();

	if (sceneList == nullptr) {
		obs_log(LOG_INFO, "No scenes found or failed to retrieve scenes");
		return;
	}

	for (char **scene = sceneList; *scene != nullptr; ++scene) {
		ui->sceneSourceDropdownList->addItem(QString(*scene));
	}

	bfree(sceneList);
}

void SettingsDialog::ConnectUISignalHandlers()
{
	QObject::connect(ui->timerIdLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->textSourceDropdownList, &QComboBox::currentTextChanged, this,
			 &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->startOnStreamStartCheckBox, &QCheckBox::stateChanged, this,
			 &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->switchSceneCheckBox, &QCheckBox::stateChanged, this,
			 &SettingsDialog::SceneSwitchCheckBoxSelected);

	QObject::connect(ui->sceneSourceDropdownList, &QComboBox::currentTextChanged, this,
			 &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->endMessageCheckBox, &QCheckBox::stateChanged, this,
			 &SettingsDialog::EndMessageCheckBoxSelected);

	QObject::connect(ui->formatOutputCheckBox, &QCheckBox::stateChanged, this,
			 &SettingsDialog::FormatOutputCheckBoxSelected);

	QObject::connect(ui->formatOutputLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->endMessageLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->daysCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->hoursCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->minutesCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->secondsCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->leadZeroCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->countUpCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::FormChangeDetected);

	QObject::connect(ui->dialogButtonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::OkButtonClicked);

	QObject::connect(ui->dialogButtonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::CancelButtonClicked);

	QPushButton *applyButton = ui->dialogButtonBox->button(QDialogButtonBox::Apply);
	if (applyButton) {
		connect(applyButton, &QPushButton::clicked, this, &SettingsDialog::ApplyButtonClicked);
	}
}

bool SettingsDialog::GetTextSources(void *list_property, obs_source_t *source)
{
	if (!source)
		return true;
	int sourceType = CheckSourceType(source);
	// If not sourceType we need;
	if (!sourceType)
		return true;

	QComboBox *sourceListUi = static_cast<QComboBox *>(list_property);

	const char *name = obs_source_get_name(source);

	if (sourceType == TEXT_SOURCE) {
		sourceListUi->addItem(name);
	}
	return true;
}

void SettingsDialog::ConnectObsSignalHandlers()
{
	// Source Signals
	signal_handler_connect(obs_get_signal_handler(), "source_create", OBSSourceCreated, ui);

	signal_handler_connect(obs_get_signal_handler(), "source_destroy", OBSSourceDeleted, ui);

	signal_handler_connect(obs_get_signal_handler(), "source_rename", OBSSourceRenamed, ui);
}

void SettingsDialog::ApplyFormChanges()
{
	isError = false;
	if (timerData != nullptr) {
		QLineEdit *idLineEdit = ui->timerIdLineEdit;
		QString setTImerId = idLineEdit->text();
		if ((setTImerId != timerData->timerId && mainWidget)) {
			Result updateIdResult = mainWidget->UpdateTimerList(timerData->timerId, setTImerId);
			if (updateIdResult.success == true) {
				idLineEdit->setStyleSheet("");
				QString dialogTitle = QString("Timer %1").arg(setTImerId);
				this->setWindowTitle(dialogTitle);
			} else {
				// Show popup with error
				isError = true;
				idLineEdit->setStyleSheet("border: 1px solid rgb(192, 0, 0);");
				obs_log(LOG_WARNING, updateIdResult.errorMessage.toStdString().c_str());
				QMessageBox::warning(this, ui->timerIdLabel->text(), updateIdResult.errorMessage);
				return;
			}
		}
		timerData->selectedSource = ui->textSourceDropdownList->currentText();

		timerData->startOnStreamStart = ui->startOnStreamStartCheckBox->isChecked();

		timerData->showEndMessage = ui->endMessageCheckBox->isChecked();
		timerData->endMessage = ui->endMessageLineEdit->text();
		timerData->showEndScene = ui->switchSceneCheckBox->isChecked();
		timerData->selectedScene = ui->sceneSourceDropdownList->currentText();

		timerData->showDays = ui->daysCheckBox->isChecked();
		timerData->showHours = ui->hoursCheckBox->isChecked();
		timerData->showMinutes = ui->minutesCheckBox->isChecked();
		timerData->showSeconds = ui->secondsCheckBox->isChecked();
		timerData->showLeadingZero = ui->leadZeroCheckBox->isChecked();

		timerData->useFormattedOutput = ui->formatOutputCheckBox->isChecked();
		timerData->outputStringFormat = ui->formatOutputLineEdit->text();

		timerData->shouldCountUp = ui->countUpCheckBox->isChecked();

		ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
		emit SettingsUpdated();
	} else {
		obs_log(LOG_WARNING, "No timer data found!");
	}
}

void SettingsDialog::SetFormDetails(TimerWidgetStruct *settingsDialogData)
{
	if (settingsDialogData != nullptr) {
		ui->timerIdLineEdit->setText(settingsDialogData->timerId);

		int textSelectIndex = ui->textSourceDropdownList->findText(settingsDialogData->selectedSource);
		if (textSelectIndex != -1)
			ui->textSourceDropdownList->setCurrentIndex(textSelectIndex);

		ui->startOnStreamStartCheckBox->setChecked(settingsDialogData->startOnStreamStart);

		int sceneSelectIndex = ui->sceneSourceDropdownList->findText(settingsDialogData->selectedScene);
		if (sceneSelectIndex != -1)
			ui->sceneSourceDropdownList->setCurrentIndex(sceneSelectIndex);

		ui->daysCheckBox->setChecked(settingsDialogData->showDays);
		ui->hoursCheckBox->setChecked(settingsDialogData->showHours);
		ui->minutesCheckBox->setChecked(settingsDialogData->showMinutes);
		ui->secondsCheckBox->setChecked(settingsDialogData->showSeconds);
		ui->leadZeroCheckBox->setChecked(settingsDialogData->showLeadingZero);

		ui->countUpCheckBox->setChecked(settingsDialogData->shouldCountUp);
		if (settingsDialogData->isPlaying)
			ui->countUpCheckBox->setEnabled(false);

		ui->endMessageCheckBox->setChecked(settingsDialogData->showEndMessage);
		ui->endMessageLineEdit->setEnabled(settingsDialogData->showEndMessage);
		ui->endMessageLineEdit->setText(settingsDialogData->endMessage);

		ui->formatOutputCheckBox->setChecked(settingsDialogData->useFormattedOutput);
		ui->formatOutputLineEdit->setText(settingsDialogData->outputStringFormat);
		if (!settingsDialogData->useFormattedOutput)
			ui->formatOutputLineEdit->setEnabled(false);

		ui->switchSceneCheckBox->setChecked(settingsDialogData->showEndScene);
		ui->sceneSourceDropdownList->setEnabled(settingsDialogData->showEndScene);

		ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	} else {
		obs_log(LOG_WARNING, "No timer data found!");
	}
}

void SettingsDialog::OBSSourceCreated(void *param, calldata_t *calldata)
{
	auto ui = static_cast<Ui::SettingsDialog *>(param);
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

void SettingsDialog::OBSSourceDeleted(void *param, calldata_t *calldata)
{
	auto ui = static_cast<Ui::SettingsDialog *>(param);

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
		int textIndexToRemove = ui->textSourceDropdownList->findText(name);
		ui->textSourceDropdownList->removeItem(textIndexToRemove);
	} else if (sourceType == SCENE_SOURCE) {
		int sceneIndexToRemove = ui->sceneSourceDropdownList->findText(name);
		ui->sceneSourceDropdownList->removeItem(sceneIndexToRemove);
	}
};

void SettingsDialog::OBSSourceRenamed(void *param, calldata_t *calldata)
{
	auto ui = static_cast<Ui::SettingsDialog *>(param);

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
		int textListIndex = ui->textSourceDropdownList->findText(oldName);
		if (textListIndex == -1)
			return;
		ui->textSourceDropdownList->setItemText(textListIndex, newName);
	} else if (sourceType == SCENE_SOURCE) {
		int sceneListIndex = ui->sceneSourceDropdownList->findText(oldName);
		if (sceneListIndex == -1)
			return;
		ui->sceneSourceDropdownList->setItemText(sceneListIndex, newName);
	}
};

int SettingsDialog::CheckSourceType(obs_source_t *source)
{
	const char *source_id = obs_source_get_unversioned_id(source);
	if (strcmp(source_id, "text_ft2_source") == 0 || strcmp(source_id, "text_gdiplus") == 0 ||
	    strcmp(source_id, "text_pango_source") == 0) {
		return TEXT_SOURCE;
	} else if (strcmp(source_id, "scene") == 0) {
		return SCENE_SOURCE;
	}
	return 0;
}

void SettingsDialog::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);

	// Reset stylings
	ui->timerIdLineEdit->setStyleSheet("");
}

void SettingsDialog::FormChangeDetected()
{
	ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void SettingsDialog::EndMessageCheckBoxSelected(int state)
{
	if (state) {
		ui->endMessageLineEdit->setEnabled(true);
	} else {
		ui->endMessageLineEdit->setEnabled(false);
	}
	FormChangeDetected();
}

void SettingsDialog::SceneSwitchCheckBoxSelected(int state)
{
	if (state) {
		ui->sceneSourceDropdownList->setEnabled(true);
	} else {
		ui->sceneSourceDropdownList->setEnabled(false);
	}
	FormChangeDetected();
}

void SettingsDialog::FormatOutputCheckBoxSelected(int state)
{
	if (state) {
		ui->formatOutputLineEdit->setEnabled(true);
	} else {
		ui->formatOutputLineEdit->setEnabled(false);
	}
	FormChangeDetected();
}

void SettingsDialog::ApplyButtonClicked()
{
	ApplyFormChanges();
}

void SettingsDialog::CancelButtonClicked()
{
	SetFormDetails(timerData);
	this->reject();
}

void SettingsDialog::OkButtonClicked()
{
	ApplyFormChanges();
	if (!isError)
		this->reject();
}
