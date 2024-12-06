#include "settings-dialog.hpp"

SettingsDialog::SettingsDialog(QWidget *parent, TimerWidgetStruct *tData)
	: QDialog(parent),
	  ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);
	timerData = tData;
	QString dialogTitle = QString("Timer %1").arg(timerData->timerId);
	this->setWindowTitle(dialogTitle);

	SetupDialogUI(timerData);

	ConnectUISignalHandlers();

	ConnectObsSignalHandlers();
}

SettingsDialog::~SettingsDialog()
{
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
	ui->leadZeroCheckBox->setToolTip(
		obs_module_text("LeadZeroCheckBoxTip"));

	ui->countUpCheckBox->setText(obs_module_text("CountUpCheckBoxLabel"));
	ui->countUpCheckBox->setToolTip(obs_module_text("CountUpCheckBoxTip"));

	ui->textSourceDropdownList->setToolTip(
		obs_module_text("TextSourceDropdownTip"));
	ui->textSourceDropdownList->addItem("");
	ui->textSourceDropdownLabel->setText(
		obs_module_text("TextSourceLabel"));

	ui->endMessageCheckBox->setCheckState(Qt::Unchecked);
	ui->endMessageCheckBox->setToolTip(
		obs_module_text("EndMessageCheckBoxTip"));
	ui->endMessageCheckBox->setText(obs_module_text("EndMessageLabel"));

	ui->endMessageLineEdit->setEnabled(false);
	ui->endMessageLineEdit->setToolTip(
		obs_module_text("EndMessageLineEditTip"));

	ui->switchSceneCheckBox->setCheckState(Qt::Unchecked);
	ui->switchSceneCheckBox->setToolTip(
		obs_module_text("SwitchSceneCheckBoxTip"));
	ui->switchSceneCheckBox->setText(obs_module_text("SwitchScene"));

	ui->sceneSourceDropdownList->setEnabled(false);
	ui->sceneSourceDropdownList->setToolTip(
		obs_module_text("SceneSourceDropdownTip"));
	ui->sceneSourceDropdownList->addItem("");

	ui->applyPushButton->setEnabled(false);

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
		obs_log(LOG_INFO,
			"No scenes found or failed to retrieve scenes");
		return;
	}

	for (char **scene = sceneList; *scene != nullptr; ++scene) {
		ui->sceneSourceDropdownList->addItem(QString(*scene));
	}

	bfree(sceneList);
}

void SettingsDialog::ConnectUISignalHandlers()
{
	QObject::connect(ui->textSourceDropdownList,
			 SIGNAL(currentTextChanged(QString)),
			 SLOT(FormChangeDetected()));

	QObject::connect(ui->switchSceneCheckBox, SIGNAL(stateChanged(int)),
			 SLOT(SceneSwitchCheckBoxSelected(int)));

	QObject::connect(ui->sceneSourceDropdownList,
			 SIGNAL(currentTextChanged(QString)),
			 SLOT(FormChangeDetected()));

	QObject::connect(ui->endMessageCheckBox, SIGNAL(stateChanged(int)),
			 SLOT(EndMessageCheckBoxSelected(int)));

	QObject::connect(ui->endMessageLineEdit, SIGNAL(textChanged(QString)),
			 SLOT(FormChangeDetected()));

	QObject::connect(ui->daysCheckBox, SIGNAL(stateChanged(int)),
			 SLOT(FormChangeDetected()));

	QObject::connect(ui->hoursCheckBox, SIGNAL(stateChanged(int)),
			 SLOT(FormChangeDetected()));

	QObject::connect(ui->minutesCheckBox, SIGNAL(stateChanged(int)),
			 SLOT(FormChangeDetected()));

	QObject::connect(ui->secondsCheckBox, SIGNAL(stateChanged(int)),
			 SLOT(FormChangeDetected()));

	QObject::connect(ui->leadZeroCheckBox, SIGNAL(stateChanged(int)),
			 SLOT(FormChangeDetected()));

	QObject::connect(ui->countUpCheckBox, SIGNAL(stateChanged(int)),
			 SLOT(FormChangeDetected()));

	QObject::connect(ui->applyPushButton, SIGNAL(clicked()),
			 SLOT(ApplyButtonClicked()));

	QObject::connect(ui->cancelPushButton, SIGNAL(clicked()),
			 SLOT(CancelButtonClicked()));

	QObject::connect(ui->okPushButton, SIGNAL(clicked()),
			 SLOT(OkButtonClicked()));
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
	signal_handler_connect(obs_get_signal_handler(), "source_create",
			       OBSSourceCreated, ui);

	signal_handler_connect(obs_get_signal_handler(), "source_destroy",
			       OBSSourceDeleted, ui);

	signal_handler_connect(obs_get_signal_handler(), "source_rename",
			       OBSSourceRenamed, ui);
}

void SettingsDialog::ApplyFormChanges()
{
	if (timerData != nullptr) {
		obs_log(LOG_INFO, "Apply form changes");
		timerData->selectedSource =
			ui->textSourceDropdownList->currentText();

		timerData->showEndMessage = ui->endMessageCheckBox->isChecked();
		timerData->endMessage = ui->endMessageLineEdit->text();
		timerData->showEndScene = ui->switchSceneCheckBox->isChecked();
		timerData->selectedScene =
			ui->sceneSourceDropdownList->currentText();

		timerData->showDays = ui->daysCheckBox->isChecked();
		timerData->showHours = ui->hoursCheckBox->isChecked();
		timerData->showMinutes = ui->minutesCheckBox->isChecked();
		timerData->showSeconds = ui->secondsCheckBox->isChecked();
		timerData->showLeadingZero = ui->leadZeroCheckBox->isChecked();

		timerData->shouldCountUp = ui->countUpCheckBox->isChecked();

		ui->applyPushButton->setEnabled(false);
	} else {
		obs_log(LOG_WARNING, "No timer data found!");
	}
}

void SettingsDialog::SetFormDetails(TimerWidgetStruct *settingsDialogData)
{
	if (settingsDialogData != nullptr) {
		obs_log(LOG_INFO, "Setting form details");
		int textSelectIndex = ui->textSourceDropdownList->findText(
			settingsDialogData->selectedSource);
		if (textSelectIndex != -1)
			ui->textSourceDropdownList->setCurrentIndex(
				textSelectIndex);

		int sceneSelectIndex = ui->sceneSourceDropdownList->findText(
			settingsDialogData->selectedScene);
		if (sceneSelectIndex != -1)
			ui->sceneSourceDropdownList->setCurrentIndex(
				sceneSelectIndex);

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

		ui->switchSceneCheckBox->setChecked(settingsDialogData->showEndScene);
		ui->sceneSourceDropdownList->setEnabled(settingsDialogData->showEndScene);

		ui->applyPushButton->setEnabled(false);
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
		int textIndexToRemove =
			ui->textSourceDropdownList->findText(name);
		ui->textSourceDropdownList->removeItem(textIndexToRemove);
	} else if (sourceType == SCENE_SOURCE) {
		int sceneIndexToRemove =
			ui->sceneSourceDropdownList->findText(name);
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

int SettingsDialog::CheckSourceType(obs_source_t *source)
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

void SettingsDialog::FormChangeDetected()
{
	ui->applyPushButton->setEnabled(true);
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

void SettingsDialog::ApplyButtonClicked()
{
	obs_log(LOG_INFO, "Apply button clicked!");
	ApplyFormChanges();
}

void SettingsDialog::CancelButtonClicked()
{
	obs_log(LOG_INFO, "Cancel button clicked!");
	SetFormDetails(timerData);
	this->reject();
}

void SettingsDialog::OkButtonClicked()
{
	obs_log(LOG_INFO, "OK button clicked!");
	ApplyFormChanges();
	this->reject();
}
