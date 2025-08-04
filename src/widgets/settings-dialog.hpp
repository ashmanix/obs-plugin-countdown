#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QScopedPointer>

#include <obs-frontend-api.h>
#include <obs-module.h>

#include "../plugin-support.h"
#include "../utils/timer-utils.hpp"
#include "../ui/ui_SettingsDialog.h"
#include "../countdown-widget.hpp"
#include "./settings/colour-change-widget.hpp"

class SettingsDialog : public QDialog {
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget *parent = nullptr, TimerWidgetStruct *tData = nullptr,
				CountdownDockWidget *mWidget = nullptr);
	~SettingsDialog() override;

	void ToggleCounterCheckBoxes(bool isEnabled);

private:
	enum SourceType { TEXT_SOURCE = 1, SCENE_SOURCE = 2 };
	QScopedPointer<Ui::SettingsDialog> ui;
	TimerWidgetStruct *m_timerData;
	CountdownDockWidget *m_mainWidget;
	ColourChangeWidget *m_colourChangeWidget = nullptr;
	bool isError = false;

	void SetupDialogUI(TimerWidgetStruct *settingsDialogData);
	void GetOBSSourceList();
	void ConnectUISignalHandlers();
	void ConnectObsSignalHandlers();
	void ApplyFormChanges();
	void SetFormDetails(TimerWidgetStruct *settingsDialogData);

	static bool GetTextSources(void *list_property, obs_source_t *source);
	static void OBSSourceCreated(void *param, calldata_t *calldata);
	static void OBSSourceDeleted(void *param, calldata_t *calldata);
	static void OBSSourceRenamed(void *param, calldata_t *calldata);

	static int CheckSourceType(obs_source_t *source);

protected:
	void showEvent(QShowEvent *event) override;

signals:
	void SettingsUpdated();

private slots:
	void FormChangeDetected();
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	void StartOnStreamStartCheckBoxSelected(Qt::CheckState state);
	void EndMessageCheckBoxSelected(Qt::CheckState state);
	void SceneSwitchCheckBoxSelected(Qt::CheckState state);
	void FormatOutputCheckBoxSelected(Qt::CheckState state);
#else
	void StartOnStreamStartCheckBoxSelected(int state);
	void EndMessageCheckBoxSelected(int state);
	void SceneSwitchCheckBoxSelected(int state);
	void FormatOutputCheckBoxSelected(int state);
#endif
	void ApplyButtonClicked();
	void CancelButtonClicked();
	void OkButtonClicked();
};

#endif // SETTINGSDIALOG_H
