#include "countdown-widget.hpp"
#include "widgets/ashmanix-timer.hpp"
#include "utils/websocket-notifier.hpp"

CountdownDockWidget::CountdownDockWidget(QWidget *parent) : OBSDock(parent), ui(new Ui::CountdownTimer)
{
	// Register custom type for signals and slots
	qRegisterMetaType<obs_data_t *>("obs_data_t*");

	ui->setupUi(this);
	timerListLayout = ui->timerMainLayout;

	SetupCountdownWidgetUI();
	resize(300, 380);

	obs_frontend_add_event_callback(OBSFrontendEventHandler, this);

	ConnectUISignalHandlers();
}

CountdownDockWidget::~CountdownDockWidget()
{
	SaveSettings();
	UnregisterAllHotkeys();
}

int CountdownDockWidget::GetNumberOfTimers()
{
	return static_cast<int>(timerWidgetMap.size());
}

AshmanixTimer *CountdownDockWidget::GetFirstTimerWidget()
{
	QLayoutItem *layout = ui->timerMainLayout->itemAt(0);
	AshmanixTimer *firstTimerWidget = nullptr;
	if (layout) {
		firstTimerWidget = static_cast<AshmanixTimer *>(layout->widget());
	}
	return firstTimerWidget;
}

bool CountdownDockWidget::IsDuplicateTimerName(QString name)
{
	AshmanixTimer *result = timerWidgetMap.value(name, nullptr);
	if (!result)
		return false;
	return true;
}

Result CountdownDockWidget::UpdateTimerList(QString oldId, QString newId)
{
	if (IsDuplicateTimerName(newId))
		return {false, obs_module_text("DialogDuplicateIdError")};

	AshmanixTimer *foundTimer = timerWidgetMap.take(oldId);
	Result result = {false, ""};

	if (!foundTimer) {
		obs_log(LOG_ERROR, "Could not find timer ID %s in saved list!", oldId.toStdString().c_str());
		result = {false, obs_module_text("DialogTimerIdUpdateError")};
	} else {
		foundTimer->SetTimerID(newId);
		timerWidgetMap.insert(newId, foundTimer);
		result = {true, ""};
	}
	return result;
}

void CountdownDockWidget::ConfigureWebSocketConnection()
{
	websocketNotifier = new WebsocketNotifier(this);
}

void CountdownDockWidget::SetupCountdownWidgetUI()
{
	ui->addTimerButton->setProperty("themeID", "addIconSmall");
	ui->addTimerButton->setProperty("class", "icon-plus");
	ui->addTimerButton->setEnabled(true);
	ui->addTimerButton->setToolTip(obs_module_text("AddTimerButtonTip"));

	ui->playAllButton->setProperty("themeID", "playIcon");
	ui->playAllButton->setProperty("class", "icon-media-play");
	ui->playAllButton->setEnabled(true);
	ui->playAllButton->setToolTip(obs_module_text("StartAllTimersButtonTip"));

	ui->stopAllButton->setProperty("themeID", "stopIcon");
	ui->stopAllButton->setProperty("class", "icon-media-stop");
	ui->stopAllButton->setEnabled(true);
	ui->stopAllButton->setToolTip(obs_module_text("StopAllTimersButtonTip"));

	this->setStyleSheet("#dialogMainWidget QDialogButtonBox QPushButton {"
			    "   width: auto;"
			    "   height: auto;"
			    "   padding: 4px 8px;"
			    "   margin: 0;"
			    "}"
			    "#mainTimerWidget {"
			    "   border-left: none;"
			    "   border-right: none;"
			    "}");
}

void CountdownDockWidget::ConnectUISignalHandlers()
{
	QObject::connect(ui->addTimerButton, &QPushButton::clicked, this, &CountdownDockWidget::AddTimerButtonClicked);

	QObject::connect(ui->playAllButton, &QPushButton::clicked, this, &CountdownDockWidget::StartAllTimers);

	QObject::connect(ui->stopAllButton, &QPushButton::clicked, this, &CountdownDockWidget::StopAllTimers);
}

void CountdownDockWidget::ConnectTimerSignalHandlers(AshmanixTimer *timerWidget)
{
	connect(timerWidget, &AshmanixTimer::RequestDelete, this, &CountdownDockWidget::RemoveTimerButtonClicked);

	connect(timerWidget, &AshmanixTimer::RequestSendWebsocketEvent, this,
		&CountdownDockWidget::HandleWebsocketSendEvent);

	connect(timerWidget, &AshmanixTimer::MoveTimer, this, &CountdownDockWidget::MoveTimerInList);
}

void CountdownDockWidget::SaveSettings()
{
	obs_data_t *settings = obs_data_create();
	obs_data_array_t *obsDataArray = obs_data_array_create();

	QVBoxLayout *mainLayout = ui->timerMainLayout;

	for (int i = 0; i < mainLayout->count(); ++i) {
		QLayoutItem *item = mainLayout->itemAt(i);
		if (item) {
			AshmanixTimer *timerWidget = qobject_cast<AshmanixTimer *>(item->widget());
			if (timerWidget) {
				TimerWidgetStruct *timerData = timerWidget->GetTimerData();
				if (timerData) {
					obs_data_t *dataObject = obs_data_create();
					timerWidget->SaveData(dataObject);
					obs_data_array_push_back(obsDataArray, dataObject);

					obs_data_release(dataObject);
				}
			}
		}
	}

	obs_data_set_array(settings, "timer_widgets", obsDataArray);

	// ----------------------------------- Save Hotkeys -----------------------------------
	SaveHotkey(settings, addTimerHotkeyId, addTimerHotkeyName);
	SaveHotkey(settings, startAllTimersHotkeyId, startAllTimersHotkeyName);
	SaveHotkey(settings, stopAllTimersHotkeyId, stopAllTimersHotkeyName);
	// ------------------------------------------------------------------------------------

	char *file = obs_module_config_path(CONFIG);
	if (!obs_data_save_json(settings, file)) {
		char *path = obs_module_config_path("");
		if (path) {
			os_mkdirs(path);
			bfree(path);
		}
		obs_data_save_json(settings, file);
	}
	obs_data_array_release(obsDataArray);
	obs_data_release(settings);
	bfree(file);
}

void CountdownDockWidget::RegisterAllHotkeys(obs_data_t *savedData)
{
	LoadHotkey(
		addTimerHotkeyId, addTimerHotkeyName, obs_module_text("AddTimerHotkeyDescription"),
		[this]() { ui->addTimerButton->click(); }, "Add Timer Hotkey Pressed", savedData);

	LoadHotkey(
		startAllTimersHotkeyId, startAllTimersHotkeyName, obs_module_text("StartAllTimersHotkeyDescription"),
		[this]() { ui->playAllButton->click(); }, "Start All Timers Hotkey Pressed", savedData);

	LoadHotkey(
		stopAllTimersHotkeyId, stopAllTimersHotkeyName, obs_module_text("StopAllTimersHotkeyDescription"),
		[this]() { ui->stopAllButton->click(); }, "Stop All Timers Hotkey Pressed", savedData);
}

void CountdownDockWidget::UnregisterAllHotkeys()
{
	if (addTimerHotkeyId)
		obs_hotkey_unregister(addTimerHotkeyId);
}

void CountdownDockWidget::AddTimer(obs_data_t *savedData)
{
	AshmanixTimer *newTimer = new AshmanixTimer(this, websocketNotifier, savedData, this);

	timerWidgetMap.insert(newTimer->GetTimerID(), newTimer);
	ConnectTimerSignalHandlers(newTimer);

	timerListLayout->addWidget(newTimer);

	ToggleUIForMultipleTimers();
	UpdateTimerListMoveButtonState();
}

void CountdownDockWidget::UpdateTimerListMoveButtonState()
{
	int timerWidgetCount = ui->timerMainLayout->count();
	for (int i = 0; i < timerWidgetCount; i++) {
		AshmanixTimer *timerWidget = static_cast<AshmanixTimer *>(ui->timerMainLayout->itemAt(i)->widget());
		if (timerWidget) {
			if (i == 0) {
				timerWidget->SetIsUpButtonDisabled(true);
				timerWidget->SetIsDownButtonDisabled(false);
			} else if (i == (timerWidgetCount - 1)) {
				timerWidget->SetIsUpButtonDisabled(false);
				timerWidget->SetIsDownButtonDisabled(true);
			} else {
				timerWidget->SetIsUpButtonDisabled(false);
				timerWidget->SetIsDownButtonDisabled(false);
			}
		}
	}
}

void CountdownDockWidget::StartTimersOnStreamStart(CountdownDockWidget *countdownDockWidget)
{
	int timerWidgetCount = countdownDockWidget->ui->timerMainLayout->count();
	for (int i = 0; i < timerWidgetCount; i++) {
		AshmanixTimer *timerWidget =
			static_cast<AshmanixTimer *>(countdownDockWidget->ui->timerMainLayout->itemAt(i)->widget());
		TimerWidgetStruct *timerData = timerWidget->GetTimerData();
		if (timerWidget && timerData->startOnStreamStart) {
			timerWidget->StartTimer(timerData->resetTimerOnStreamStart);
		}
	}
}

void CountdownDockWidget::UpdateWidgetStyles(CountdownDockWidget *countdownDockWidget)
{
	int timerWidgetCount = countdownDockWidget->ui->timerMainLayout->count();
	for (int i = 0; i < timerWidgetCount; i++) {
		AshmanixTimer *timerWidget =
			static_cast<AshmanixTimer *>(countdownDockWidget->ui->timerMainLayout->itemAt(i)->widget());
		if (timerWidget) {
			timerWidget->UpdateStyles();
		}
	}
}

void CountdownDockWidget::OBSFrontendEventHandler(enum obs_frontend_event event, void *private_data)
{

	CountdownDockWidget *countdownDockWidget = (CountdownDockWidget *)private_data;

	switch (event) {
	case OBS_FRONTEND_EVENT_FINISHED_LOADING:
		CountdownDockWidget::LoadSavedSettings(countdownDockWidget);
		break;
	case OBS_FRONTEND_EVENT_THEME_CHANGED:
		CountdownDockWidget::UpdateWidgetStyles(countdownDockWidget);
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STARTED:
		CountdownDockWidget::StartTimersOnStreamStart(countdownDockWidget);
		break;
	default:
		break;
	}
}

void CountdownDockWidget::LoadSavedSettings(CountdownDockWidget *dockWidget)
{
	char *file = obs_module_config_path(CONFIG);
	obs_data_t *data = nullptr;
	if (file) {
		data = obs_data_create_from_json_file(file);
		bfree(file);
	}
	if (data) {
		// Get Save Data
		obs_data_array_t *timersArray = obs_data_get_array(data, "timer_widgets");
		if (timersArray) {
			size_t count = obs_data_array_count(timersArray);
			for (size_t i = 0; i < count; ++i) {
				obs_data_t *timerDataObj = obs_data_array_item(timersArray, i);
				dockWidget->AddTimer(timerDataObj);
			}
		}

		// Add widget if none were loaded from save file
		// as we must have at least 1 timer
		if (dockWidget->GetNumberOfTimers() == 0) {
			dockWidget->AddTimer();
		}

		dockWidget->RegisterAllHotkeys(data);

		obs_data_release(data);
	}
}

AshmanixTimer *CountdownDockWidget::AttemptToGetTimerWidgetById(const char *websocketTimerID)
{
	AshmanixTimer *timer = nullptr;
	if (websocketTimerID != nullptr && strlen(websocketTimerID) > 0) {
		timer = this->timerWidgetMap.value(websocketTimerID, nullptr);
	} else if (this->timerListLayout->count()) {
		QLayoutItem *layoutItem = this->timerListLayout->itemAt(0);
		if (layoutItem) {
			timer = qobject_cast<AshmanixTimer *>(layoutItem->widget());
		}
	}
	return timer;
}

void CountdownDockWidget::ToggleUIForMultipleTimers()
{

	AshmanixTimer *firstTimerWidget = nullptr;
	int noOfTImers = GetNumberOfTimers();

	if (noOfTImers == 1) {
		// If only one timer left we use timer map to get last timer
		// as deletion of timers is delayed and we might get an old
		// reference when getting timer in layout
		firstTimerWidget = static_cast<AshmanixTimer *>(timerWidgetMap.begin().value());
		if (firstTimerWidget) {
			firstTimerWidget->SetHideMultiTimerUIButtons(true);
			ui->playAllButton->hide();
			ui->stopAllButton->hide();
		}
	} else {
		firstTimerWidget = GetFirstTimerWidget();
		if (firstTimerWidget) {
			firstTimerWidget->SetHideMultiTimerUIButtons(false);
			ui->playAllButton->show();
			ui->stopAllButton->show();
		}
	}
}

// --------------------------------- Private Slots ----------------------------------

void CountdownDockWidget::AddTimerButtonClicked()
{
	AddTimer();
}

void CountdownDockWidget::RemoveTimerButtonClicked(QString id)
{
	AshmanixTimer *itemToBeRemoved = timerWidgetMap.value(id, nullptr);

	if (itemToBeRemoved) {
		itemToBeRemoved->deleteLater();
		timerWidgetMap.remove(id);
		obs_log(LOG_INFO, (QString("Timer %1 deleted").arg(id)).toStdString().c_str());
	}

	ToggleUIForMultipleTimers();
	UpdateTimerListMoveButtonState();
}

void CountdownDockWidget::HandleWebsocketSendEvent(const char *eventName, obs_data_t *eventData)
{
	if (!vendor)
		return;

	obs_websocket_vendor_emit_event(vendor, eventName, eventData);
}

void CountdownDockWidget::MoveTimerInList(Direction direction, QString id)
{
	AshmanixTimer *timerWidget = timerWidgetMap.find(id).value();
	if (timerWidget) {
		//Gets the index of the widget within the layout
		const int index = ui->timerMainLayout->indexOf(timerWidget);
		if (!(direction == Direction::UP && index == 0) &&
		    !(direction == Direction::DOWN && index == (ui->timerMainLayout->count() - 1))) {
			const int newIndex = direction == Direction::UP ? index - 1 : index + 1;
			ui->timerMainLayout->removeWidget(timerWidget);
			ui->timerMainLayout->insertWidget(newIndex, timerWidget);
			UpdateTimerListMoveButtonState();
		}
	}
}

void CountdownDockWidget::StartAllTimers()
{
	int timerWidgetCount = ui->timerMainLayout->count();
	for (int i = 0; i < timerWidgetCount; i++) {
		AshmanixTimer *timerWidget = static_cast<AshmanixTimer *>(ui->timerMainLayout->itemAt(i)->widget());
		if (timerWidget) {
			timerWidget->StartTimer();
		}
	}
}

void CountdownDockWidget::StopAllTimers()
{
	int timerWidgetCount = ui->timerMainLayout->count();
	for (int i = 0; i < timerWidgetCount; i++) {
		AshmanixTimer *timerWidget = static_cast<AshmanixTimer *>(ui->timerMainLayout->itemAt(i)->widget());
		if (timerWidget) {
			timerWidget->StopTimer();
		}
	}
}
