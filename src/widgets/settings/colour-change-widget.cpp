#include "colour-change-widget.hpp"
#include <obs-module.h>
#include <QFileInfo>
#include <QDir>
#include <QColor>
#include "../../utils/obs-utils.hpp"

ColourChangeWidget::ColourChangeWidget(QWidget *parent, TimerWidgetStruct *countdownTimerData)
	: QWidget(parent),
	  data(countdownTimerData),
	  m_ui(new Ui::TextColourSettingsWidget)
{
	m_ui->setupUi(this);
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

	SetEnabled(false);

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
}

void ColourChangeWidget::SetData()
{

	// Need to get an input of a map of colour rules and then save them to this widget

	ClearSelection();
	// if (!m_pose)
	// return obs_log(LOG_WARNING, "No pose data found when loading blendshape rules!");

	blockSignals(true);
	// QMap<QString, QSharedPointer<BlendshapeRule>> *bsList = m_pose->getBlendshapeList();
	// // list rules
	// if (!bsList->isEmpty()) {
	// 	for (auto i = bsList->begin(), end = bsList->end(); i != end; i++) {
	// 		auto rule = *i;
	// 		addBlendshapeRule(rule);
	// 	}
	// }
	blockSignals(false);
}

void ColourChangeWidget::ClearSelection()
{
	blockSignals(true);
	// Clear widget list
	for (auto i = m_colourRulesWidgetMap.begin(), end = m_colourRulesWidgetMap.end(); i != end; i++) {
		auto rule = *i;
		rule->deleteLater();
	}

	m_colourRulesWidgetMap.clear();
	blockSignals(false);
}

//  ----------------------------------------------- Private Slots ---------------------------------------------------

void ColourChangeWidget::HandleEnableClick(bool isEnabled)
{
	SetEnabled(isEnabled);
}

void ColourChangeWidget::HandleAddButtonClicked()
{
	AddColourRule();
}

void ColourChangeWidget::HandleColourRuleDeleteButtonClicked(QString id)
{
	obs_log(LOG_INFO, "Trying to delete colour rule: %s", id.toStdString().c_str());
	SingleColourRuleWidget *itemToBeRemoved = m_colourRulesWidgetMap.value(id, nullptr);

	if (itemToBeRemoved) {
		m_colourRulesWidgetMap.remove(id);
		itemToBeRemoved->deleteLater();

		emit ColourRuleChanged();
		obs_log(LOG_INFO, (QString("Colour rule %1 deleted").arg(id)).toStdString().c_str());
	}
}

//  ----------------------------------------------- Private Functions ------------------------------------------------

void ColourChangeWidget::ConnectUISignalHandlers()
{
	QObject::connect(m_ui->addColourSettingToolButton, &QToolButton::clicked, this,
			 &ColourChangeWidget::HandleAddButtonClicked);

	QObject::connect(m_ui->enableTextColourCheckBox, &QToolButton::clicked, this,
			 &ColourChangeWidget::HandleEnableClick);
}

void ColourChangeWidget::ConnectColourRuleSignalHandlers(SingleColourRuleWidget *colourRuleWidget)
{
	QObject::connect(colourRuleWidget, &SingleColourRuleWidget::Change, this,
			 [this]() { emit ColourRuleChanged(); });
	QObject::connect(colourRuleWidget, &SingleColourRuleWidget::RemoveBlendshapeRule, this,
			 &ColourChangeWidget::HandleColourRuleDeleteButtonClicked);
}

void ColourChangeWidget::AddColourRule(QSharedPointer<ColourRule> in_colourRule)
{
	if (in_colourRule == nullptr) {
		in_colourRule = QSharedPointer<ColourRule>::create();
	}

	if (!in_colourRule.isNull()) {
		obs_log(LOG_INFO, "Adding colour rule to layout");
		auto newColourRuleWidget = new SingleColourRuleWidget(this, in_colourRule);
		m_colourRulesWidgetMap.insert(newColourRuleWidget->GetID(), newColourRuleWidget);
		ConnectColourRuleSignalHandlers(newColourRuleWidget);

		m_ui->colourRulesVerticalLayout->addWidget(newColourRuleWidget);
		emit ColourRuleChanged();
	}
}

void ColourChangeWidget::SetEnabled(bool isEnabled)
{
	m_ui->addColourSettingToolButton->setEnabled(isEnabled);
	m_ui->mainColourLabel->setEnabled(isEnabled);
	m_ui->mainColourPushButton->setEnabled(isEnabled);
}
