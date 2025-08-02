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

void SingleColourRuleWidget::SetData(ColourRuleData colourRuleData)
{
	ToggleBlockAllUISignals(true);

	if (!m_colourRule) {
		m_colourRule = QSharedPointer<ColourRule>::create("", colourRuleData.startTime, colourRuleData.endTime,
								  colourRuleData.colour);
	} else {
		m_colourRule->SetStartTime(colourRuleData.startTime);
		m_colourRule->SetEndTime(colourRuleData.endTime);
		m_colourRule->SetColour(colourRuleData.colour);
	}

	m_ui->idLabel->setText(m_colourRule->GetID());
	m_ui->maxTimeDaySpinBox->setValue(colourRuleData.startTime.days);
	m_ui->maxTimeHourSpinBox->setValue(colourRuleData.startTime.hours);
	m_ui->maxTimeMinuteSpinBox->setValue(colourRuleData.startTime.minutes);
	m_ui->maxTimeSecondSpinBox->setValue(colourRuleData.startTime.seconds);

	m_ui->minTimeDaySpinBox->setValue(colourRuleData.endTime.days);
	m_ui->minTimeHourSpinBox->setValue(colourRuleData.endTime.hours);
	m_ui->minTimeMinuteSpinBox->setValue(colourRuleData.endTime.minutes);
	m_ui->minTimeSecondSpinBox->setValue(colourRuleData.endTime.seconds);

	SetTextColour(colourRuleData.colour);

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

//  ------------------------------------------------- Private Slots --------------------------------------------------

void SingleColourRuleWidget::HandleColourButtonPushed()
{
	obs_log(LOG_INFO, "Colour button clicked!");
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

	QObject::connect(m_ui->minTimeDaySpinBox, &QSpinBox::valueChanged, this, [this]() { emit ChangeDetected(); });
	QObject::connect(m_ui->minTimeHourSpinBox, &QSpinBox::valueChanged, this, [this]() { emit ChangeDetected(); });
	QObject::connect(m_ui->minTimeMinuteSpinBox, &QSpinBox::valueChanged, this,
			 [this]() { emit ChangeDetected(); });
	QObject::connect(m_ui->minTimeSecondSpinBox, &QSpinBox::valueChanged, this,
			 [this]() { emit ChangeDetected(); });

	QObject::connect(m_ui->maxTimeDaySpinBox, &QSpinBox::valueChanged, this, [this]() { emit ChangeDetected(); });
	QObject::connect(m_ui->maxTimeHourSpinBox, &QSpinBox::valueChanged, this, [this]() { emit ChangeDetected(); });
	QObject::connect(m_ui->maxTimeMinuteSpinBox, &QSpinBox::valueChanged, this,
			 [this]() { emit ChangeDetected(); });
	QObject::connect(m_ui->maxTimeSecondSpinBox, &QSpinBox::valueChanged, this,
			 [this]() { emit ChangeDetected(); });
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

	m_ui->colourPushButton->setToolTip(obs_module_text("DialogTextColourColourButtonTip"));
	m_ui->deleteToolButton->setToolTip(obs_module_text("DialogTextColourRemoveColourButtonTip"));

	m_ui->idLabel->setText(m_colourRule->GetID());

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
