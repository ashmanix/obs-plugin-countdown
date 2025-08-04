#include "colour-change-widget.hpp"
#include <obs-module.h>
#include <QFileInfo>
#include <QDir>
#include "../../utils/obs-utils.hpp"

ColourChangeWidget::ColourChangeWidget(QWidget *parent, TimerWidgetStruct *countdownTimerData)
	: QWidget(parent),
	  m_data(countdownTimerData),
	  m_ui(new Ui::TextColourSettingsWidget)
{
	m_ui->setupUi(this);

	SetData(countdownTimerData);

	SetupWidgetUI();
	ConnectUISignalHandlers();
}

void ColourChangeWidget::SetupWidgetUI()
{
	m_ui->timerTextColourGroupBox->setTitle(obs_module_text("DialogTextColourEnableCheckBox"));

	m_ui->addColourSettingToolButton->setToolTip(obs_module_text("DialogTextColourAddColourButtonTip"));
	m_ui->enableTextColourCheckBox->setToolTip(obs_module_text("DialogTextColourEnableCheckBox"));

	m_ui->mainColourLabel->setText(obs_module_text("DialogTextColourMainColourLabel"));
	m_ui->mainColourPushButton->setToolTip(obs_module_text("DialogTextColourColourButtonTip"));

	m_ui->rulesLabel->setText(obs_module_text("DialogTextColourRulesLabel"));

	UpdateStyledUIComponents();
}

void ColourChangeWidget::UpdateStyledUIComponents()
{
	QString baseUrl = obs_frontend_is_theme_dark() ? GetDataFolderPath() + "/icons/dark/"
						       : GetDataFolderPath() + "/icons/light/";
	QString plusIconUrl = QDir::fromNativeSeparators(baseUrl + "plus.svg");
	if (QFileInfo::exists(plusIconUrl)) {
		QIcon addIcon(plusIconUrl);
		m_ui->addColourSettingToolButton->setIcon(addIcon);
	}
	SetMainTextColour(m_mainTextColour);
}

QList<ColourRuleData> ColourChangeWidget::GetColourRuleList()
{
	QList<ColourRuleData> dataToGet;

	dataToGet.reserve(m_colourRules.size());

	for (const auto &rulePtr : m_colourRules) {
		if (!rulePtr)
			continue;

		dataToGet.append(ColourRuleData{rulePtr->GetMinTime(), rulePtr->GetMaxTime(), rulePtr->GetColour()});
	}
	return dataToGet;
}

QColor ColourChangeWidget::GetMainTextColour()
{
	return m_mainTextColour;
}

bool ColourChangeWidget::GetShouldUseColourChange()
{
	return m_ui->enableTextColourCheckBox->checkState() == Qt::Checked;
}

void ColourChangeWidget::SetData(TimerWidgetStruct *countdownTimerData)
{
	if (!countdownTimerData)
		return;

	blockSignals(true);
	ClearSelection();

	if (countdownTimerData->display.mainTextColour != nullptr) {
		m_mainTextColour = countdownTimerData->display.mainTextColour;
	} else {
		m_mainTextColour = Qt::white;
	}

	m_ui->enableTextColourCheckBox->setChecked(countdownTimerData->display.useTextColour);
	SetEnabled(countdownTimerData->display.useTextColour);

	if (!countdownTimerData->display.colourRuleList.isEmpty()) {
		for (auto i = countdownTimerData->display.colourRuleList.begin(),
			  end = countdownTimerData->display.colourRuleList.end();
		     i != end; i++) {
			auto rule = *i;
			auto cRule = QSharedPointer<ColourRule>::create("", rule.minTime, rule.maxTime, rule.colour);
			AddColourRule(cRule);
		}
	}

	blockSignals(false);
}

void ColourChangeWidget::ClearSelection()
{
	blockSignals(true);
	// destroy widgets
	for (auto *w : std::as_const(m_colourRuleWidgetIds))
		w->deleteLater();
	m_colourRuleWidgetIds.clear();
	m_colourRules.clear();
	blockSignals(false);
}

//  ----------------------------------------------- Private Slots ---------------------------------------------------
void ColourChangeWidget::HandleAddButtonClicked()
{
	AddColourRule();
	emit ColourRuleChanged();
}

void ColourChangeWidget::HandleColourRuleDeletion(QString id)
{
	if (auto *widgetToRemove = m_colourRuleWidgetIds.value(id, nullptr)) {
		auto it = std::find_if(m_colourRules.begin(), m_colourRules.end(),
				       [&](const QSharedPointer<ColourRule> &rule) { return rule->GetID() == id; });
		if (it != m_colourRules.end())
			m_colourRules.erase(it);

		// Remove from layout + delete UI
		m_ui->colourRulesVerticalLayout->removeWidget(widgetToRemove);
		m_colourRuleWidgetIds.remove(id);
		widgetToRemove->deleteLater();

		UpdateAllRuleLabels();

		emit ColourRuleChanged();
	}
}

void ColourChangeWidget::HandleMainColourButtonClick()
{
	QColor initialColour = m_mainTextColour.isValid() ? m_mainTextColour : Qt::white;

	QColor newSelectedColour =
		QColorDialog::getColor(initialColour, this, obs_module_text("DialogTextColourChooseMainColourTitle"));

	if (newSelectedColour.isValid()) {
		m_mainTextColour = newSelectedColour;
		SetMainTextColour(m_mainTextColour);
		emit ColourRuleChanged();
	}
}

void ColourChangeWidget::HandleRuleChange(SingleColourRuleWidget *ruleWidget, ColourRule::TimerType type,
					  PeriodData timeDuration)
{
	UNUSED_PARAMETER(ruleWidget);
	UNUSED_PARAMETER(type);
	UNUSED_PARAMETER(timeDuration);

	int ruleListSize = m_colourRules.size();
	if (ruleListSize < 2) // Only loop when list is has more than 1 item in it
		return;

	int ruleIndex = m_colourRules.indexOf(ruleWidget->GetColourRule());
	if (ruleIndex == -1)
		return;

	// Update all widget times before this one
	for (auto i = ruleIndex - 1; i >= 0; --i) {
		auto cRule = m_colourRules.at(i);
		auto cRuleAfter = m_colourRules.at(i + 1);
		if (!cRule || !cRuleAfter)
			continue;

		auto maxAtIndex = cRule->GetMaxTime();
		auto minTimeAtIndexAfter = cRuleAfter->GetMinTime();

		if (!IsPeriodDataBefore(maxAtIndex, minTimeAtIndexAfter)) {
			auto ruleWidget = m_colourRuleWidgetIds.value(cRule->GetID());
			if (ruleWidget) {
				auto timeOneSecBeforeMax = AddSecondsToTimerDuration(minTimeAtIndexAfter, -1);
				ruleWidget->SetMaxTime(timeOneSecBeforeMax);
				ruleWidget->ValidateColourRuleTimes(ColourRule::TimerType::MAX, timeOneSecBeforeMax);
			}
		}
	}

	// Update all widget times after this one
	for (auto i = ruleIndex + 1; i < ruleListSize; ++i) {
		auto cRule = m_colourRules.at(i);
		auto cRuleBefore = m_colourRules.at(i - 1);
		if (!cRule || !cRuleBefore)
			continue;

		auto minAtIndex = cRule->GetMinTime();
		auto maxTimeAtIndexBefore = cRuleBefore->GetMaxTime();

		if (!IsPeriodDataAfter(minAtIndex, maxTimeAtIndexBefore)) {
			auto ruleWidget = m_colourRuleWidgetIds.value(cRule->GetID());
			if (ruleWidget) {
				auto timeOneSecAfterMin = AddSecondsToTimerDuration(maxTimeAtIndexBefore, +1);
				ruleWidget->SetMinTime(timeOneSecAfterMin);
				ruleWidget->ValidateColourRuleTimes(ColourRule::TimerType::MIN, timeOneSecAfterMin);
			}
		}
	}

	emit ColourRuleChanged();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void ColourChangeWidget::HandleEnableCheckBoxSelected(Qt::CheckState state)
{
	if (state == Qt::CheckState::Checked) {
		SetEnabled(true);
	} else {
		SetEnabled(false);
	}
	emit ColourRuleChanged();
}

#else

void ColourChangeWidget::HandleEnableCheckBoxSelected(int state)
{
	if (state) {
		SetEnabled(true);
	} else {
		SetEnabled(false);
	}
	emit ColourRuleChanged();
}

#endif

//  ----------------------------------------------- Private Functions ------------------------------------------------

void ColourChangeWidget::ConnectUISignalHandlers()
{
	QObject::connect(m_ui->addColourSettingToolButton, &QToolButton::clicked, this,
			 &ColourChangeWidget::HandleAddButtonClicked);

	QObject::connect(m_ui->mainColourPushButton, &QPushButton::clicked, this,
			 &ColourChangeWidget::HandleMainColourButtonClick);

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	QObject::connect(m_ui->enableTextColourCheckBox, &QCheckBox::checkStateChanged, this,
			 &ColourChangeWidget::HandleEnableCheckBoxSelected);
#else
	QObject::connect(m_ui->enableTextColourCheckBox, &QCheckBox::clicked, this,
			 &ColourChangeWidget::HandleEnableCheckBoxSelected);
#endif
}

void ColourChangeWidget::ConnectColourRuleSignalHandlers(SingleColourRuleWidget *colourRuleWidget)
{
	QObject::connect(colourRuleWidget, &SingleColourRuleWidget::ChangeDetected, this,
			 [this]() { emit ColourRuleChanged(); });
	QObject::connect(colourRuleWidget, &SingleColourRuleWidget::ChangeTimeDetected, this,
			 &ColourChangeWidget::HandleRuleChange);
	QObject::connect(colourRuleWidget, &SingleColourRuleWidget::RemoveColoureRule, this,
			 &ColourChangeWidget::HandleColourRuleDeletion);
}

void ColourChangeWidget::AddColourRule(QSharedPointer<ColourRule> in_colourRule)
{
	bool isNew = false;

	if (!in_colourRule) {
		isNew = true;
		in_colourRule = QSharedPointer<ColourRule>::create();
		in_colourRule->SetTime(ColourRule::TimerType::MAX, {0, 0, 0, 1});
	}

	m_colourRules.push_back(in_colourRule);
	auto newColourRuleWidget = new SingleColourRuleWidget(this, in_colourRule);
	m_colourRuleWidgetIds.insert(newColourRuleWidget->GetID(), newColourRuleWidget);

	// Set the widget label to index number
	int index = m_colourRules.indexOf(in_colourRule);
	newColourRuleWidget->SetLabel(QString::number(index + 1));

	// Do this before connecting signal handlers
	// Set min and max times based on previous rule if there is one
	if (isNew && index != 0) {
		auto prevRule = m_colourRules.at(index - 1);
		if (prevRule) {
			auto maxTimeFromPrevRule = prevRule->GetMaxTime();
			auto newMinTime = AddSecondsToTimerDuration(maxTimeFromPrevRule, 1);
			newColourRuleWidget->SetMinTime(newMinTime);
			auto newMaxTime = AddSecondsToTimerDuration(newMinTime, 1);
			newColourRuleWidget->SetMaxTime(newMaxTime);
		}
	}

	ConnectColourRuleSignalHandlers(newColourRuleWidget);

	m_ui->colourRulesVerticalLayout->addWidget(newColourRuleWidget);
	emit ColourRuleChanged();
}

void ColourChangeWidget::SetEnabled(bool isEnabled)
{
	m_ui->addColourSettingToolButton->setEnabled(isEnabled);
	m_ui->mainColourLabel->setEnabled(isEnabled);
	m_ui->mainColourPushButton->setEnabled(isEnabled);
	m_ui->rulesLabel->setEnabled(isEnabled);
	m_ui->colourRulesVerticalLayout->setEnabled(isEnabled);

	for (auto i = m_colourRuleWidgetIds.begin(), end = m_colourRuleWidgetIds.end(); i != end; ++i) {
		auto *widget = i.value();
		widget->SetEnabled(isEnabled);
	}
}

void ColourChangeWidget::UpdateAllRuleLabels()
{
	for (auto i = m_colourRuleWidgetIds.begin(), end = m_colourRuleWidgetIds.end(); i != end; ++i) {
		auto *widget = i.value();
		int index = m_colourRules.indexOf(widget->GetColourRule());
		widget->SetLabel(QString::number(index + 1));
	}
}

void ColourChangeWidget::SetMainTextColour(QColor colour)
{
	QColor hover = colour.lighter(110);   // +10%
	QColor pressed = colour.darker(110);  // -10%
	QColor disabled = colour.darker(120); // optional

	auto rgba = [](const QColor &c) {
		return QString("rgba(%1,%2,%3,%4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alphaF());
	};

	const QString name = m_ui->mainColourPushButton->objectName();

	const QString style = QString("QPushButton#%1 { background-color: %2; }"
				      "QPushButton#%1:hover { background-color: %3; }"
				      "QPushButton#%1:pressed { background-color: %4; }"
				      "QPushButton#%1:disabled { background-color: %5; }")
				      .arg(name, colour.name(), rgba(hover), rgba(pressed), rgba(disabled));

	m_ui->mainColourPushButton->setStyleSheet(style);
}
