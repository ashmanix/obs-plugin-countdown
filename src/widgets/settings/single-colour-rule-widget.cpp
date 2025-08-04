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

	SetData(m_colourRule.data());

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

void SingleColourRuleWidget::SetData(ColourRule *colourRule)
{
	ToggleBlockAllUISignals(true);

	if (!m_colourRule) {
		m_colourRule = QSharedPointer<ColourRule>::create("", colourRule->GetMinTime(),
								  colourRule->GetMaxTime(), colourRule->GetColour());
	} else {
		m_colourRule->SetTime(ColourRule::TimerType::MIN, colourRule->GetMinTime());
		m_colourRule->SetTime(ColourRule::TimerType::MAX, colourRule->GetMaxTime());
		m_colourRule->SetColour(colourRule->GetColour());
	}

	m_ui->maxTimeDaySpinBox->setValue(colourRule->GetMaxTime().days);
	m_ui->maxTimeHourSpinBox->setValue(colourRule->GetMaxTime().hours);
	m_ui->maxTimeMinuteSpinBox->setValue(colourRule->GetMaxTime().minutes);
	m_ui->maxTimeSecondSpinBox->setValue(colourRule->GetMaxTime().seconds);

	m_ui->minTimeDaySpinBox->setValue(colourRule->GetMinTime().days);
	m_ui->minTimeHourSpinBox->setValue(colourRule->GetMinTime().hours);
	m_ui->minTimeMinuteSpinBox->setValue(colourRule->GetMinTime().minutes);
	m_ui->minTimeSecondSpinBox->setValue(colourRule->GetMinTime().seconds);

	SetTextColour(colourRule->GetColour());

	ToggleBlockAllUISignals(false);
}

void SingleColourRuleWidget::SetID(QString newId)
{
	m_colourRule->SetID(newId);
}

void SingleColourRuleWidget::SetLabel(QString labelValue)
{
	m_ui->idLabel->setText(labelValue);
}

void SingleColourRuleWidget::SetMaxTime(PeriodData newTime)
{
	m_colourRule->SetTime(ColourRule::TimerType::MAX, newTime);
	SetData(m_colourRule.data());
}

void SingleColourRuleWidget::SetMinTime(PeriodData newTime)
{
	m_colourRule->SetTime(ColourRule::TimerType::MIN, newTime);
	SetData(m_colourRule.data());
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

	QFont idLabelFont = m_ui->idLabel->font();
	idLabelFont.setBold(true);

	SetTextColour(m_colourRule->GetColour());
}

void SingleColourRuleWidget::SetEnabled(bool isEnabled)
{
	m_ui->idLabel->setEnabled(isEnabled);

	m_ui->minTimeDaySpinBox->setEnabled(isEnabled);
	m_ui->minTimeHourSpinBox->setEnabled(isEnabled);
	m_ui->minTimeMinuteSpinBox->setEnabled(isEnabled);
	m_ui->minTimeSecondSpinBox->setEnabled(isEnabled);
	m_ui->maxTimeDaySpinBox->setEnabled(isEnabled);
	m_ui->maxTimeHourSpinBox->setEnabled(isEnabled);
	m_ui->maxTimeMinuteSpinBox->setEnabled(isEnabled);
	m_ui->maxTimeSecondSpinBox->setEnabled(isEnabled);

	m_ui->minTimeLabel->setEnabled(isEnabled);
	m_ui->maxTimeLabel->setEnabled(isEnabled);

	m_ui->deleteToolButton->setEnabled(isEnabled);
	m_ui->colourPushButton->setEnabled(isEnabled);
}

void SingleColourRuleWidget::ValidateColourRuleTimes(ColourRule::TimerType changeType, PeriodData timeToValidate)
{
	if (changeType == ColourRule::TimerType::MAX) {
		auto ruleMinTime = m_colourRule->GetMinTime();
		if (!IsPeriodDataAfter(timeToValidate, ruleMinTime)) {
			auto newTime = AddSecondsToTimerDuration(timeToValidate, -1);
			SetMinTime(newTime);
		}
	} else if (changeType == ColourRule::TimerType::MIN) {
		auto ruleMaxTime = m_colourRule->GetMaxTime();
		if (!IsPeriodDataBefore(timeToValidate, ruleMaxTime)) {
			auto newTime = AddSecondsToTimerDuration(timeToValidate, 1);
			SetMaxTime(newTime);
		}
	}
}

//  ------------------------------------------------- Private Slots --------------------------------------------------

void SingleColourRuleWidget::HandleColourButtonPushed()
{
	QColor initialColour = m_colourRule->GetColour().isValid() ? m_colourRule->GetColour() : Qt::white;

	QColor newSelectedColour =
		QColorDialog::getColor(initialColour, this, obs_module_text("DialogTextColourChooseMainColourTitle"));

	if (newSelectedColour.isValid()) {
		m_colourRule->SetColour(newSelectedColour);
		SetTextColour(newSelectedColour);
		emit ChangeDetected();
	}
}

//  ----------------------------------------------- Private Functions ------------------------------------------------

void SingleColourRuleWidget::ConnectUISignalHandlers()
{
	QObject::connect(m_ui->colourPushButton, &QPushButton::clicked, this,
			 &SingleColourRuleWidget::HandleColourButtonPushed);

	QObject::connect(m_ui->deleteToolButton, &QPushButton::clicked, this,
			 [this]() { emit RemoveColoureRule(m_colourRule->GetID()); });

	QObject::connect(m_ui->minTimeDaySpinBox, &QSpinBox::valueChanged, this, [this](int value) {
		HandleTimerChange(ColourRule::TimerType::MIN, ColourRule::DurationType::DAYS, value);
	});
	QObject::connect(m_ui->minTimeHourSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
		HandleTimerChange(ColourRule::TimerType::MIN, ColourRule::DurationType::HOURS, value);
	});
	QObject::connect(m_ui->minTimeMinuteSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
		HandleTimerChange(ColourRule::TimerType::MIN, ColourRule::DurationType::MINUTES, value);
	});
	QObject::connect(m_ui->minTimeSecondSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
		HandleTimerChange(ColourRule::TimerType::MIN, ColourRule::DurationType::SECONDS, value);
	});

	QObject::connect(m_ui->maxTimeDaySpinBox, &QSpinBox::valueChanged, this, [this](int value) {
		HandleTimerChange(ColourRule::TimerType::MAX, ColourRule::DurationType::DAYS, value);
	});
	QObject::connect(m_ui->maxTimeHourSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
		HandleTimerChange(ColourRule::TimerType::MAX, ColourRule::DurationType::HOURS, value);
	});
	QObject::connect(m_ui->maxTimeMinuteSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
		HandleTimerChange(ColourRule::TimerType::MAX, ColourRule::DurationType::MINUTES, value);
	});
	QObject::connect(m_ui->maxTimeSecondSpinBox, &QSpinBox::valueChanged, this, [this](int value) {
		HandleTimerChange(ColourRule::TimerType::MAX, ColourRule::DurationType::SECONDS, value);
	});
}

void SingleColourRuleWidget::SetupWidgetUI()
{
	m_ui->minTimeDaySpinBox->setRange(0, 999);
	m_ui->minTimeDaySpinBox->setToolTip(obs_module_text("DaysCheckboxLabel"));
	m_ui->minTimeDaySpinBox->setSuffix(obs_module_text("DialogTextColourTimeDaySuffix"));

	m_ui->minTimeHourSpinBox->setRange(0, 23);
	m_ui->minTimeHourSpinBox->setToolTip(obs_module_text("HoursCheckboxLabel"));
	m_ui->minTimeHourSpinBox->setSuffix(obs_module_text("DialogTextColourTimeHourSuffix"));

	m_ui->minTimeMinuteSpinBox->setRange(0, 59);
	m_ui->minTimeMinuteSpinBox->setToolTip(obs_module_text("MinutesCheckboxLabel"));
	m_ui->minTimeMinuteSpinBox->setSuffix(obs_module_text("DialogTextColourTimeMinuteSuffix"));

	m_ui->minTimeSecondSpinBox->setRange(0, 59);
	m_ui->minTimeSecondSpinBox->setToolTip(obs_module_text("SecondsCheckboxLabel"));
	m_ui->minTimeSecondSpinBox->setSuffix(obs_module_text("DialogTextColourTimeSecondSuffix"));

	m_ui->maxTimeDaySpinBox->setRange(0, 999);
	m_ui->maxTimeDaySpinBox->setToolTip(obs_module_text("DaysCheckboxLabel"));
	m_ui->maxTimeDaySpinBox->setSuffix(obs_module_text("DialogTextColourTimeDaySuffix"));

	m_ui->maxTimeHourSpinBox->setRange(0, 23);
	m_ui->maxTimeHourSpinBox->setToolTip(obs_module_text("HoursCheckboxLabel"));
	m_ui->maxTimeHourSpinBox->setSuffix(obs_module_text("DialogTextColourTimeHourSuffix"));

	m_ui->maxTimeMinuteSpinBox->setRange(0, 59);
	m_ui->maxTimeMinuteSpinBox->setToolTip(obs_module_text("MinutesCheckboxLabel"));
	m_ui->maxTimeMinuteSpinBox->setSuffix(obs_module_text("DialogTextColourTimeMinuteSuffix"));

	m_ui->maxTimeSecondSpinBox->setRange(0, 59);
	m_ui->maxTimeSecondSpinBox->setToolTip(obs_module_text("SecondsCheckboxLabel"));
	m_ui->maxTimeSecondSpinBox->setSuffix(obs_module_text("DialogTextColourTimeSecondSuffix"));

	m_ui->minTimeLabel->setText(obs_module_text("DialogTextColourRuleMinLabel"));
	m_ui->maxTimeLabel->setText(obs_module_text("DialogTextColourRuleMaxLabel"));

	m_ui->colourPushButton->setToolTip(obs_module_text("DialogTextColourColourButtonTip"));
	m_ui->deleteToolButton->setToolTip(obs_module_text("DialogTextColourRemoveColourButtonTip"));

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
	m_ui->maxTimeSecondSpinBox->blockSignals(shouldBlock);
}

void SingleColourRuleWidget::HandleTimerChange(ColourRule::TimerType timerType, ColourRule::DurationType durationType,
					       int value)
{
	if (!m_colourRule) {
		obs_log(LOG_WARNING, "No colour rule detected!");
		return;
	}
	PeriodData newTime;

	switch (timerType) {
	case ColourRule::TimerType::MIN:
		newTime = m_colourRule->GetMinTime();
		break;
	case ColourRule::TimerType::MAX:
		newTime = m_colourRule->GetMaxTime();
		break;
	}

	switch (durationType) {
	case ColourRule::DurationType::DAYS:
		newTime.days = value;
		break;
	case ColourRule::DurationType::HOURS:
		newTime.hours = value;
		break;
	case ColourRule::DurationType::MINUTES:
		newTime.minutes = value;
		break;
	case ColourRule::DurationType::SECONDS:
		newTime.seconds = value;
		break;
	}

	m_colourRule->SetTime(timerType, newTime);

	ValidateColourRuleTimes(timerType, newTime);

	emit ChangeTimeDetected(this, timerType, newTime);
}

void SingleColourRuleWidget::SetTextColour(QColor colour)
{
	QColor hover = colour.lighter(110);
	QColor pressed = colour.darker(110);
	QColor disabled = colour.darker(120);

	auto rgba = [](const QColor &c) {
		return QString("rgba(%1,%2,%3,%4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alphaF());
	};

	const QString name = m_ui->colourPushButton->objectName();

	const QString style = QString("QPushButton#%1 { background-color: %2; }"
				      "QPushButton#%1:hover { background-color: %3; }"
				      "QPushButton#%1:pressed { background-color: %4; }"
				      "QPushButton#%1:disabled { background-color: %5; }")
				      .arg(name, colour.name(), rgba(hover), rgba(pressed), rgba(disabled));

	m_ui->colourPushButton->setStyleSheet(style);
}
