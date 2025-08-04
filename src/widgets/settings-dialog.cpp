#include "settings-dialog.hpp"

SettingsDialog::SettingsDialog(QWidget *parent, TimerWidgetStruct *tData, CountdownDockWidget *mWidget)
	: QDialog(parent),
	  ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);
	m_timerData = tData;
	m_mainWidget = mWidget;
	QString dialogTitle = QString("Timer %1").arg(m_timerData->timerId);
	this->setWindowTitle(dialogTitle);

	SetupDialogUI(m_timerData);

	ConnectUISignalHandlers();

	ConnectObsSignalHandlers();
}

SettingsDialog::~SettingsDialog()
{
	// Disconnect OBS signals before the dialog is destroyed
	signal_handler_disconnect(obs_get_signal_handler(), "source_create", OBSSourceCreated, ui.data());
	signal_handler_disconnect(obs_get_signal_handler(), "source_destroy", OBSSourceDeleted, ui.data());
	signal_handler_disconnect(obs_get_signal_handler(), "source_rename", OBSSourceRenamed, ui.data());

	this->deleteLater();
}

void SettingsDialog::ToggleCounterCheckBoxes(bool isEnabled)
{
	ui->countUpCheckBox->setEnabled(isEnabled);
	ui->smoothPeriodTimerCheckBox->setEnabled(isEnabled);
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

	ui->resetTimerOnStreamStartCheckBox->setText(obs_module_text("ResetOnStreamStartCheckBoxLabel"));
	ui->resetTimerOnStreamStartCheckBox->setToolTip(obs_module_text("ResetOnStreamStartCheckBoxTip"));

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

	ui->smoothPeriodTimerCheckBox->setText(obs_module_text("DialogSmoothTimerLabel"));
	ui->smoothPeriodTimerCheckBox->setToolTip(obs_module_text("SmoothPeriodTimerTip"));

	ui->generalGroupBox->setTitle(obs_module_text("DialogGeneralGroupBoxTitle"));
	ui->timerStartGroupBox->setTitle(obs_module_text("DialogTimerStartGroupBoxTitle"));
	ui->timerEndGroupBox->setTitle(obs_module_text("DialogTimerEndGroupBoxTitle"));
	ui->timeFormatGroupBox->setTitle(obs_module_text("DialogTimeFormatGroupBoxTitle"));
	ui->timerTypeGroupBox->setTitle(obs_module_text("DialogTimerTypeGroupBoxTitle"));

	ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setText(obs_module_text("DialogButtonApplyLabel"));
	ui->dialogButtonBox->button(QDialogButtonBox::Ok)->setText(obs_module_text("DialogButtonOkLabel"));
	ui->dialogButtonBox->button(QDialogButtonBox::Cancel)->setText(obs_module_text("DialogButtonCancelLabel"));

	// Add Colour Groupbox to Settings Dialog
	m_colourChangeWidget = new ColourChangeWidget(this, settingsDialogData);
	ui->extraWidgetContents->layout()->addWidget(m_colourChangeWidget);

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
	QObject::connect(ui->sceneSourceDropdownList, &QComboBox::currentTextChanged, this,
			 &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->formatOutputLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->endMessageLineEdit, &QLineEdit::textChanged, this, &SettingsDialog::FormChangeDetected);
	QObject::connect(m_colourChangeWidget, &ColourChangeWidget::ColourRuleChanged, this,
			 &SettingsDialog::FormChangeDetected);

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	QObject::connect(ui->startOnStreamStartCheckBox, &QCheckBox::checkStateChanged, this,
			 &SettingsDialog::StartOnStreamStartCheckBoxSelected);
	QObject::connect(ui->resetTimerOnStreamStartCheckBox, &QCheckBox::checkStateChanged, this,
			 &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->switchSceneCheckBox, &QCheckBox::checkStateChanged, this,
			 &SettingsDialog::SceneSwitchCheckBoxSelected);
	QObject::connect(ui->endMessageCheckBox, &QCheckBox::checkStateChanged, this,
			 &SettingsDialog::EndMessageCheckBoxSelected);
	QObject::connect(ui->formatOutputCheckBox, &QCheckBox::checkStateChanged, this,
			 &SettingsDialog::FormatOutputCheckBoxSelected);
	QObject::connect(ui->endMessageCheckBox, &QCheckBox::checkStateChanged, this,
			 &SettingsDialog::EndMessageCheckBoxSelected);
	QObject::connect(ui->formatOutputCheckBox, &QCheckBox::checkStateChanged, this,
			 &SettingsDialog::FormatOutputCheckBoxSelected);

	QObject::connect(ui->daysCheckBox, &QCheckBox::checkStateChanged, this, &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->hoursCheckBox, &QCheckBox::checkStateChanged, this, &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->minutesCheckBox, &QCheckBox::checkStateChanged, this, &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->secondsCheckBox, &QCheckBox::checkStateChanged, this, &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->leadZeroCheckBox, &QCheckBox::checkStateChanged, this,
			 &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->countUpCheckBox, &QCheckBox::checkStateChanged, this, &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->smoothPeriodTimerCheckBox, &QCheckBox::checkStateChanged, this,
			 &SettingsDialog::FormChangeDetected);
#else
	QObject::connect(ui->startOnStreamStartCheckBox, &QCheckBox::stateChanged, this,
			 &SettingsDialog::StartOnStreamStartCheckBoxSelected);
	QObject::connect(ui->resetTimerOnStreamStartCheckBox, &QCheckBox::stateChanged, this,
			 &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->switchSceneCheckBox, &QCheckBox::stateChanged, this,
			 &SettingsDialog::SceneSwitchCheckBoxSelected);
	QObject::connect(ui->endMessageCheckBox, &QCheckBox::stateChanged, this,
			 &SettingsDialog::EndMessageCheckBoxSelected);
	QObject::connect(ui->formatOutputCheckBox, &QCheckBox::stateChanged, this,
			 &SettingsDialog::FormatOutputCheckBoxSelected);
	QObject::connect(ui->endMessageCheckBox, &QCheckBox::stateChanged, this,
			 &SettingsDialog::EndMessageCheckBoxSelected);
	QObject::connect(ui->formatOutputCheckBox, &QCheckBox::stateChanged, this,
			 &SettingsDialog::FormatOutputCheckBoxSelected);

	QObject::connect(ui->daysCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->hoursCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->minutesCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->secondsCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->leadZeroCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->countUpCheckBox, &QCheckBox::stateChanged, this, &SettingsDialog::FormChangeDetected);
	QObject::connect(ui->smoothPeriodTimerCheckBox, &QCheckBox::stateChanged, this,
			 &SettingsDialog::FormChangeDetected);
#endif

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
	signal_handler_connect(obs_get_signal_handler(), "source_create", OBSSourceCreated, ui.data());

	signal_handler_connect(obs_get_signal_handler(), "source_destroy", OBSSourceDeleted, ui.data());

	signal_handler_connect(obs_get_signal_handler(), "source_rename", OBSSourceRenamed, ui.data());
}

void SettingsDialog::ApplyFormChanges()
{
	isError = false;
	if (m_timerData != nullptr) {
		QLineEdit *idLineEdit = ui->timerIdLineEdit;
		QString setTImerId = idLineEdit->text();
		if ((setTImerId != m_timerData->timerId && m_mainWidget)) {
			Result updateIdResult = m_mainWidget->UpdateTimerList(m_timerData->timerId, setTImerId);
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
		m_timerData->source.selectedSource = ui->textSourceDropdownList->currentText();

		m_timerData->startOnStreamStart = ui->startOnStreamStartCheckBox->isChecked();
		m_timerData->resetTimerOnStreamStart = ui->resetTimerOnStreamStartCheckBox->isChecked();

		m_timerData->display.showEndMessage = ui->endMessageCheckBox->isChecked();
		m_timerData->display.endMessage = ui->endMessageLineEdit->text();
		m_timerData->display.showEndScene = ui->switchSceneCheckBox->isChecked();
		m_timerData->source.selectedScene = ui->sceneSourceDropdownList->currentText();

		m_timerData->display.showDays = ui->daysCheckBox->isChecked();
		m_timerData->display.showHours = ui->hoursCheckBox->isChecked();
		m_timerData->display.showMinutes = ui->minutesCheckBox->isChecked();
		m_timerData->display.showSeconds = ui->secondsCheckBox->isChecked();
		m_timerData->display.showLeadingZero = ui->leadZeroCheckBox->isChecked();

		m_timerData->display.useFormattedOutput = ui->formatOutputCheckBox->isChecked();
		m_timerData->display.outputStringFormat = ui->formatOutputLineEdit->text();

		m_timerData->smoothenPeriodTimer = ui->smoothPeriodTimerCheckBox->isChecked();

		m_timerData->shouldCountUp = ui->countUpCheckBox->isChecked();

		m_timerData->display.useTextColour = m_colourChangeWidget->GetShouldUseColourChange();
		m_timerData->display.mainTextColour = m_colourChangeWidget->GetMainTextColour();
		m_timerData->display.colourRuleList = m_colourChangeWidget->GetColourRuleList();

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

		int textSelectIndex = ui->textSourceDropdownList->findText(settingsDialogData->source.selectedSource);
		if (textSelectIndex != -1)
			ui->textSourceDropdownList->setCurrentIndex(textSelectIndex);

		ui->startOnStreamStartCheckBox->setChecked(settingsDialogData->startOnStreamStart);
		ui->resetTimerOnStreamStartCheckBox->setChecked(settingsDialogData->resetTimerOnStreamStart);

		ui->resetTimerOnStreamStartCheckBox->setEnabled(settingsDialogData->startOnStreamStart);

		int sceneSelectIndex = ui->sceneSourceDropdownList->findText(settingsDialogData->source.selectedScene);
		if (sceneSelectIndex != -1)
			ui->sceneSourceDropdownList->setCurrentIndex(sceneSelectIndex);

		ui->daysCheckBox->setChecked(settingsDialogData->display.showDays);
		ui->hoursCheckBox->setChecked(settingsDialogData->display.showHours);
		ui->minutesCheckBox->setChecked(settingsDialogData->display.showMinutes);
		ui->secondsCheckBox->setChecked(settingsDialogData->display.showSeconds);
		ui->leadZeroCheckBox->setChecked(settingsDialogData->display.showLeadingZero);

		ui->countUpCheckBox->setChecked(settingsDialogData->shouldCountUp);
		if (settingsDialogData->isPlaying)
			ui->countUpCheckBox->setEnabled(false);

		ui->endMessageCheckBox->setChecked(settingsDialogData->display.showEndMessage);
		ui->endMessageLineEdit->setEnabled(settingsDialogData->display.showEndMessage);
		ui->endMessageLineEdit->setText(settingsDialogData->display.endMessage);

		ui->formatOutputCheckBox->setChecked(settingsDialogData->display.useFormattedOutput);
		ui->formatOutputLineEdit->setText(settingsDialogData->display.outputStringFormat);
		if (!settingsDialogData->display.useFormattedOutput)
			ui->formatOutputLineEdit->setEnabled(false);

		ui->smoothPeriodTimerCheckBox->setChecked(settingsDialogData->smoothenPeriodTimer);

		ui->switchSceneCheckBox->setChecked(settingsDialogData->display.showEndScene);
		ui->sceneSourceDropdownList->setEnabled(settingsDialogData->display.showEndScene);

		ui->dialogButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

		if (m_colourChangeWidget) {
			m_colourChangeWidget->SetData(settingsDialogData);
		}
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)

void SettingsDialog::StartOnStreamStartCheckBoxSelected(Qt::CheckState state)
{
	if (state == Qt::CheckState::Checked) {
		ui->resetTimerOnStreamStartCheckBox->setEnabled(true);
	} else {
		ui->resetTimerOnStreamStartCheckBox->setEnabled(false);
	}
	FormChangeDetected();
}

void SettingsDialog::EndMessageCheckBoxSelected(Qt::CheckState state)
{
	if (state == Qt::CheckState::Checked) {
		ui->endMessageLineEdit->setEnabled(true);
	} else {
		ui->endMessageLineEdit->setEnabled(false);
	}
	FormChangeDetected();
}

void SettingsDialog::SceneSwitchCheckBoxSelected(Qt::CheckState state)
{
	if (state == Qt::CheckState::Checked) {
		ui->sceneSourceDropdownList->setEnabled(true);
	} else {
		ui->sceneSourceDropdownList->setEnabled(false);
	}
	FormChangeDetected();
}

void SettingsDialog::FormatOutputCheckBoxSelected(Qt::CheckState state)
{
	if (state == Qt::CheckState::Checked) {
		ui->formatOutputLineEdit->setEnabled(true);
	} else {
		ui->formatOutputLineEdit->setEnabled(false);
	}
	FormChangeDetected();
}

#else

void SettingsDialog::StartOnStreamStartCheckBoxSelected(int state)
{
	if (state) {
		ui->resetTimerOnStreamStartCheckBox->setEnabled(true);
	} else {
		ui->resetTimerOnStreamStartCheckBox->setEnabled(false);
	}
	FormChangeDetected();
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

#endif

void SettingsDialog::ApplyButtonClicked()
{
	ApplyFormChanges();
}

void SettingsDialog::CancelButtonClicked()
{
	SetFormDetails(m_timerData);
	this->reject();
}

void SettingsDialog::OkButtonClicked()
{
	ApplyFormChanges();
	if (!isError)
		this->reject();
}
