#include "single-colour-rule-widget.hpp"
#include <obs-module.h>
#include <QDir>
#include "../../utils/obs-utils.hpp"

SingleColourRuleWidget::SingleColourRuleWidget(QWidget *parent, QSharedPointer<ColourRule> colourRule)
	: QWidget(parent),
	  m_colourRule(colourRule),
	  m_ui(new Ui::SingleColourRuleWidget)
{
	m_ui->setupUi(this);
	SetupWidgetUI();
	ConnectUISignalHandlers();
}

QString SingleColourRuleWidget::GetID() const
{
	return m_colourRule.data()->GetID();
}

QSharedPointer<ColourRule> SingleColourRuleWidget::GetColourRule() const
{
	return m_colourRule;
}

void SingleColourRuleWidget::SetData(QSharedPointer<ColourRule> in_colourRule)
{
	if (in_colourRule)
		m_colourRule = in_colourRule;

	ToggleBlockAllUISignals(true);

	// Need to set the date time for min and max edits from incoming data

	// m_ui->minTimeEdit->setTime(static_cast<int>(m_colourRule->getKey()));
	// m_ui->maxTimeEdit->setTime(static_cast<int>(m_colourRule->getComparisonType()));

	ToggleBlockAllUISignals(false);
}

void SingleColourRuleWidget::UpdateStyledUIComponents()
{
	QString baseUrl = obs_frontend_is_theme_dark() ? GetDataFolderPath() + "/icons/dark/"
						       : GetDataFolderPath() + "/icons/light/";
	QString trashIconUrl = QDir::fromNativeSeparators(baseUrl + "trash.svg");
	if (QFileInfo::exists(trashIconUrl)) {
		QIcon trashIcon(trashIconUrl);
		m_ui->deleteToolButton->setIcon(trashIcon);
	}
}

//  ------------------------------------------------- Private Slots --------------------------------------------------

void SingleColourRuleWidget::HandleColourButtonPushed()
{
	obs_log(LOG_INFO, "Colour button clicked!");
}

//  ----------------------------------------------- Private Functions ------------------------------------------------

void SingleColourRuleWidget::ConnectUISignalHandlers()
{
	QObject::connect(m_ui->colourPushButton, &QPushButton::clicked, this,
			 &SingleColourRuleWidget::HandleColourButtonPushed);

	QObject::connect(m_ui->colourPushButton, &QPushButton::clicked, this,
			 &SingleColourRuleWidget::HandleColourButtonPushed);
}

void SingleColourRuleWidget::SetupWidgetUI()
{
	// m_ui->maxTimeEdit->setToolTip(obs_module_text("DialogTextColourMaxTimeEditTip"));
	// m_ui->minTimeEdit->setToolTip(obs_module_text("DialogTextColourMinTimeEditTip"));

	m_ui->minTimeDaySpinBox->setRange(0, 999);
	m_ui->minTimeDaySpinBox->setToolTip(obs_module_text("DaysCheckboxLabel"));

	m_ui->minTimeHourSpinBox->setRange(0, 23);
	m_ui->minTimeHourSpinBox->setToolTip(obs_module_text("HoursCheckboxLabel"));

	m_ui->minTimeMinuteSpinBox->setRange(0, 59);
	m_ui->minTimeMinuteSpinBox->setToolTip(obs_module_text("MinutesCheckboxLabel"));

	m_ui->minTimeSecondSpinBox->setRange(0, 59);
	m_ui->minTimeSecondSpinBox->setToolTip(obs_module_text("SecondsCheckboxLabel"));

	m_ui->maxTimeDaySpinBox->setRange(0, 999);
	m_ui->maxTimeDaySpinBox->setToolTip(obs_module_text("DaysCheckboxLabel"));

	m_ui->maxTimeHourSpinBox->setRange(0, 23);
	m_ui->maxTimeHourSpinBox->setToolTip(obs_module_text("HoursCheckboxLabel"));

	m_ui->maxTimeMinuteSpinBox->setRange(0, 59);
	m_ui->maxTimeMinuteSpinBox->setToolTip(obs_module_text("MinutesCheckboxLabel"));

	m_ui->maxTimeSecondSpinBox->setRange(0, 59);
	m_ui->maxTimeSecondSpinBox->setToolTip(obs_module_text("SecondsCheckboxLabel"));

	UpdateStyledUIComponents();
}

void SingleColourRuleWidget::ToggleBlockAllUISignals(bool shouldBlock)
{
	m_ui->minTimeDaySpinBox->blockSignals(shouldBlock);
	m_ui->minTimeHourSpinBox->blockSignals(shouldBlock);
	m_ui->minTimeMinuteSpinBox->blockSignals(shouldBlock);
	m_ui->minTimeSecondSpinBox->blockSignals(shouldBlock);
	m_ui->maxTimeDaySpinBox->blockSignals(shouldBlock);
	m_ui->maxTimeHourSpinBox->blockSignals(shouldBlock);
	m_ui->maxTimeMinuteSpinBox->blockSignals(shouldBlock);
	m_ui->minTimeSecondSpinBox->blockSignals(shouldBlock);
}

void SingleColourRuleWidget::HandleTimerChange(TimerType type, TimerDuration time)
{
	if (!m_colourRule) {
		obs_log(LOG_WARNING, "No colour rule detected!");
		return;
	}

	switch (type) {
	case TimerType::START:
		m_ui->minTimeDaySpinBox->setValue(time.days);
		m_ui->minTimeHourSpinBox->setValue(time.hours);
		m_ui->minTimeMinuteSpinBox->setValue(time.minutes);
		m_ui->minTimeSecondSpinBox->setValue(time.seconds);
		break;
	case TimerType::END:
		m_ui->maxTimeDaySpinBox->setValue(time.days);
		m_ui->maxTimeHourSpinBox->setValue(time.hours);
		m_ui->maxTimeMinuteSpinBox->setValue(time.minutes);
		m_ui->maxTimeSecondSpinBox->setValue(time.seconds);
		break;
	}
}
