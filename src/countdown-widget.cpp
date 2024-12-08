#include "countdown-widget.hpp"

CountdownDockWidget::CountdownDockWidget(QWidget *parent)
	: QWidget(parent),
	  ui(new Ui::CountdownTimer)
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
		firstTimerWidget =
			static_cast<AshmanixTimer *>(layout->widget());
	}
	return firstTimerWidget;
}

void CountdownDockWidget::ConfigureWebSocketConnection()
{
	vendor = obs_websocket_register_vendor(VENDORNAME);

	if (!vendor) {
		obs_log(LOG_ERROR, "Error registering vendor to websocket!");
		return;
	}

	obs_websocket_vendor_register_request(
		vendor, "period_play", HandleWebsocketButtonPressRequest,
		new WebsocketCallbackData{this, PERIOD_PLAY, NULL, TIMERIDKEY});
	obs_websocket_vendor_register_request(
		vendor, "period_pause", HandleWebsocketButtonPressRequest,
		new WebsocketCallbackData{this, PERIOD_PAUSE, NULL,
					  TIMERIDKEY});
	obs_websocket_vendor_register_request(
		vendor, "period_set", HandleWebsocketButtonPressRequest,
		new WebsocketCallbackData{this, PERIOD_SET, NULL, TIMERIDKEY});

	obs_websocket_vendor_register_request(
		vendor, "to_time_play", HandleWebsocketButtonPressRequest,
		new WebsocketCallbackData{this, TO_TIME_PLAY, NULL,
					  TIMERIDKEY});
	obs_websocket_vendor_register_request(
		vendor, "to_time_stop", HandleWebsocketButtonPressRequest,
		new WebsocketCallbackData{this, TO_TIME_STOP, NULL,
					  TIMERIDKEY});

	obs_websocket_vendor_register_request(
		vendor, "get_timer_state", GetTimerStateViaWebsocket,
		new WebsocketCallbackData{this, GET_TIME, NULL, TIMERIDKEY});

	obs_websocket_vendor_register_request(
		vendor, "add_time", ChangeTimerTimeViaWebsocket,
		new WebsocketCallbackData{this, ADD_TIME, "time_to_add",
					  TIMERIDKEY});

	obs_websocket_vendor_register_request(
		vendor, "set_time", ChangeTimerTimeViaWebsocket,
		new WebsocketCallbackData{this, SET_TIME, "time_to_set",
					  TIMERIDKEY});
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
	ui->playAllButton->setToolTip(
		obs_module_text("StartAllTimersButtonTip"));

	ui->stopAllButton->setProperty("themeID", "stopIcon");
	ui->stopAllButton->setProperty("class", "icon-media-stop");
	ui->stopAllButton->setEnabled(true);
	ui->stopAllButton->setToolTip(
		obs_module_text("StopAllTimersButtonTip"));
}

void CountdownDockWidget::ConnectUISignalHandlers()
{
	QObject::connect(ui->addTimerButton, &QPushButton::clicked, this,
			 &CountdownDockWidget::AddTimerButtonClicked);

	QObject::connect(ui->playAllButton, &QPushButton::clicked, this,
			 &CountdownDockWidget::StartAllTimers);

	QObject::connect(ui->stopAllButton, &QPushButton::clicked, this,
			 &CountdownDockWidget::StopAllTimers);
}

void CountdownDockWidget::ConnectTimerSignalHandlers(AshmanixTimer *timerWidget)
{
	connect(timerWidget, &AshmanixTimer::RequestDelete, this,
		&CountdownDockWidget::RemoveTimerButtonClicked);

	connect(timerWidget, &AshmanixTimer::RequestSendWebsocketEvent, this,
		&CountdownDockWidget::HandleWebsocketSendEvent);

	connect(timerWidget, &AshmanixTimer::MoveTimer, this,
		&CountdownDockWidget::MoveTimerInList);
}

void CountdownDockWidget::SaveSettings()
{
	obs_data_t *settings = obs_data_create();
	obs_data_array_t *obsDataArray = obs_data_array_create();

	QVBoxLayout *mainLayout = ui->timerMainLayout;

	for (int i = 0; i < mainLayout->count(); ++i) {
		QLayoutItem *item = mainLayout->itemAt(i);
		if (item) {
			AshmanixTimer *timerWidget =
				qobject_cast<AshmanixTimer *>(item->widget());
			if (timerWidget) {
				TimerWidgetStruct *timerData =
					timerWidget->GetTimerData();
				if (timerData) {
					obs_data_t *dataObject =
						obs_data_create();
					timerWidget
						->SaveTimerWidgetDataToOBSSaveData(
							dataObject);
					obs_data_array_push_back(obsDataArray,
								 dataObject);

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
		addTimerHotkeyId, addTimerHotkeyName,
		obs_module_text("AddTimerHotkeyDescription"),
		[this]() { ui->addTimerButton->click(); },
		"Add Timer Hotkey Pressed", savedData);

	LoadHotkey(
		startAllTimersHotkeyId, startAllTimersHotkeyName,
		obs_module_text("StartAllTimersHotkeyDescription"),
		[this]() { ui->playAllButton->click(); },
		"Start All Timers Hotkey Pressed", savedData);

	LoadHotkey(
		stopAllTimersHotkeyId, stopAllTimersHotkeyName,
		obs_module_text("StopAllTimersHotkeyDescription"),
		[this]() { ui->stopAllButton->click(); },
		"Stop All Timers Hotkey Pressed", savedData);
}

void CountdownDockWidget::UnregisterAllHotkeys()
{
	if (addTimerHotkeyId)
		obs_hotkey_unregister(addTimerHotkeyId);
}

void CountdownDockWidget::AddTimer(obs_data_t *savedData)
{
	AshmanixTimer *newTimer = new AshmanixTimer(this, vendor, savedData);

	timerWidgetMap.insert(newTimer->GetTimerID(), newTimer);
	ConnectTimerSignalHandlers(newTimer);

	timerListLayout->addWidget(newTimer);

	AshmanixTimer *firstTimerWidget = GetFirstTimerWidget();

	if (GetNumberOfTimers() == 1 && firstTimerWidget) {
		firstTimerWidget->SetIsDeleteButtonDisabled(true);
	} else {
		firstTimerWidget->SetIsDeleteButtonDisabled(false);
	}

	UpdateTimerListMoveButtonState();
}

void CountdownDockWidget::UpdateTimerListMoveButtonState()
{
	int timerWidgetCount = ui->timerMainLayout->count();
	for (int i = 0; i < timerWidgetCount; i++) {
		AshmanixTimer *timerWidget = static_cast<AshmanixTimer *>(
			ui->timerMainLayout->itemAt(i)->widget());
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

void CountdownDockWidget::UpdateWidgetStyles(
	CountdownDockWidget *countdownDockWidget)
{
	int timerWidgetCount =
		countdownDockWidget->ui->timerMainLayout->count();
	for (int i = 0; i < timerWidgetCount; i++) {
		AshmanixTimer *timerWidget = static_cast<AshmanixTimer *>(
			countdownDockWidget->ui->timerMainLayout->itemAt(i)
				->widget());
		if (timerWidget) {
			timerWidget->UpdateStyles();
		}
	}
}

void CountdownDockWidget::OBSFrontendEventHandler(enum obs_frontend_event event,
						  void *private_data)
{

	CountdownDockWidget *countdownDockWidget =
		(CountdownDockWidget *)private_data;

	switch (event) {
	case OBS_FRONTEND_EVENT_FINISHED_LOADING: {
		CountdownDockWidget::LoadSavedSettings(countdownDockWidget);
	} break;
	case OBS_FRONTEND_EVENT_THEME_CHANGED: {
		CountdownDockWidget::UpdateWidgetStyles(countdownDockWidget);
	} break;
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
		obs_data_array_t *timersArray =
			obs_data_get_array(data, "timer_widgets");
		if (timersArray) {
			size_t count = obs_data_array_count(timersArray);
			for (size_t i = 0; i < count; ++i) {
				obs_data_t *timerDataObj =
					obs_data_array_item(timersArray, i);
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

AshmanixTimer *CountdownDockWidget::AttemptToGetTimerWidgetById(
	CountdownDockWidget *countdownWidget, const char *websocketTimerID)
{
	AshmanixTimer *timer = nullptr;
	if (websocketTimerID != nullptr && strlen(websocketTimerID) > 0) {
		timer = countdownWidget->timerWidgetMap.value(websocketTimerID,
							      nullptr);
	} else if (countdownWidget->timerListLayout->count()) {
		QLayoutItem *layoutItem =
			countdownWidget->timerListLayout->itemAt(0);
		if (layoutItem) {
			timer = qobject_cast<AshmanixTimer *>(
				layoutItem->widget());
		}
	}
	return timer;
}

void CountdownDockWidget::ChangeTimerTimeViaWebsocket(obs_data_t *request_data,
						      obs_data_t *response_data,
						      void *priv_data)
{
	auto *callback_data = static_cast<WebsocketCallbackData *>(priv_data);
	WebsocketRequestType requestType = callback_data->requestType;
	const char *requestDataTimeKey = callback_data->requestDataKey;
	const char *requestTimerIdKey = callback_data->requestTimerIdKey;

	const char *websocketDataTime =
		obs_data_get_string(request_data, requestDataTimeKey);

	if (websocketDataTime == nullptr || strlen(websocketDataTime) == 0) {
		obs_data_set_bool(response_data, "success", false);
		QString error_message =
			QString("%1 field is missing from request!")
				.arg(requestDataTimeKey);
		obs_data_set_string(response_data, "message",
				    error_message.toStdString().c_str());
	} else {
		CountdownDockWidget *countdownWidget = callback_data->instance;
		const char *websocketTimerID =
			obs_data_get_string(request_data, requestTimerIdKey);

		AshmanixTimer *timer = AttemptToGetTimerWidgetById(
			countdownWidget, websocketTimerID);

		if (timer != nullptr) {
			long long timeInMillis =
				ConvertStringPeriodToMillis(websocketDataTime);

			if (timeInMillis > 0) {
				bool result = timer->AlterTime(requestType,
							       timeInMillis);
				const char *type_string =
					requestType == ADD_TIME ? "added"
								: "set";
				obs_log(LOG_INFO,
					"Time %s due to websocket call: %s",
					type_string, websocketDataTime);
				obs_data_set_bool(response_data, "success",
						  result);
			} else {
				obs_log(LOG_WARNING,
					"Timer time NOT changed from websocket request.");
				obs_data_set_bool(response_data, "success",
						  false);
				obs_data_set_string(
					response_data, "message",
					"Timer time wasn't changed. Ensure time is in format \"dd:hh:mm:ss\"");
			}
		} else {
			obs_log(LOG_WARNING,
				"Countdown widget not found for websocket request!");
			obs_data_set_bool(response_data, "success", false);
			obs_data_set_string(response_data, "message",
					    "Error trying to update time!");
		}
	}
}

void CountdownDockWidget::GetTimerStateViaWebsocket(obs_data_t *request_data,
						    obs_data_t *response_data,
						    void *priv_data)
{
	auto *callback_data = static_cast<WebsocketCallbackData *>(priv_data);
	const char *requestTimerIdKey = callback_data->requestTimerIdKey;

	CountdownDockWidget *countdownWidget = callback_data->instance;
	const char *websocketTimerID =
		obs_data_get_string(request_data, requestTimerIdKey);

	AshmanixTimer *timer =
		AttemptToGetTimerWidgetById(countdownWidget, websocketTimerID);

	if (timer != nullptr) {
		TimerWidgetStruct *timerData = timer->GetTimerData();
		obs_data_set_bool(response_data, "is_running",
				  timerData->isPlaying);
		obs_data_set_int(response_data, "time_left_ms",
				 timerData->timeLeftInMillis);

		obs_data_set_string(response_data, "timer_id",
				    timerData->timerId.toStdString().c_str());

		obs_data_set_bool(response_data, "success", true);

	} else {
		obs_log(LOG_WARNING,
			"Countdown widget not found for websocket request!");
		obs_data_set_bool(response_data, "success", false);
		obs_data_set_string(response_data, "message",
				    "Error trying to get timer data!");
	}
}

void CountdownDockWidget::HandleWebsocketButtonPressRequest(
	obs_data_t *request_data, obs_data_t *response_data, void *priv_data)
{
	auto *callback_data = static_cast<WebsocketCallbackData *>(priv_data);
	const char *requestTimerIdKey = callback_data->requestTimerIdKey;
	WebsocketRequestType requestType = callback_data->requestType;
	CountdownDockWidget *countdownWidget = callback_data->instance;
	const char *websocketTimerID =
		obs_data_get_string(request_data, requestTimerIdKey);

	AshmanixTimer *timer =
		AttemptToGetTimerWidgetById(countdownWidget, websocketTimerID);

	if (timer != nullptr) {
		switch (requestType) {
		case PERIOD_PLAY:
			timer->PressPlayButton();
			obs_data_set_bool(response_data, "success", true);
			obs_data_set_bool(response_data, "message",
					  "Play button pressed");
			break;
		case PERIOD_PAUSE:
			timer->PressStopButton();
			obs_data_set_bool(response_data, "success", true);
			obs_data_set_bool(response_data, "message",
					  "Pause button pressed");
			break;
		case PERIOD_SET:
			timer->PressResetButton();
			obs_data_set_bool(response_data, "success", true);
			obs_data_set_bool(response_data, "message",
					  "Reset button pressed");
			break;
		case TO_TIME_PLAY:
			timer->PressToTimePlayButton();
			obs_data_set_bool(response_data, "success", true);
			obs_data_set_bool(response_data, "message",
					  "To Time play button pressed");
			break;
		case TO_TIME_STOP:
			timer->PressToTimeStopButton();
			obs_data_set_bool(response_data, "success", true);
			obs_data_set_bool(response_data, "message",
					  "To Time stop button pressed");
			break;
		default:
			obs_data_set_bool(response_data, "success", false);
			obs_data_set_bool(response_data, "message",
					  "No buttons pressed");
			break;
		}
	} else {
		obs_log(LOG_WARNING,
			"Countdown widget not found for websocket timer state change request!");
		obs_data_set_bool(response_data, "success", false);
		obs_data_set_string(response_data, "message",
				    "Error trying to change timer state!");
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
		obs_log(LOG_INFO, (QString("Timer %1 deleted").arg(id))
					  .toStdString()
					  .c_str());
	}

	// There should always be 1 timer in list therefore we disable
	// the delete button if only 1 timer is left.
	AshmanixTimer *firstTimerWidget = GetFirstTimerWidget();
	int noOfTImers = GetNumberOfTimers();
	if (noOfTImers == 1 && firstTimerWidget) {
		firstTimerWidget->SetIsDeleteButtonDisabled(true);
	}
	UpdateTimerListMoveButtonState();
}

void CountdownDockWidget::HandleWebsocketSendEvent(const char *eventName,
						   obs_data_t *eventData)
{
	if (!vendor)
		return;

	obs_websocket_vendor_emit_event(vendor, eventName, eventData);
}

void CountdownDockWidget::MoveTimerInList(QString direction, QString id)
{
	AshmanixTimer *timerWidget = timerWidgetMap.find(id).value();
	if (timerWidget) {
		//Gets the index of the widget within the layout
		const int index = ui->timerMainLayout->indexOf(timerWidget);
		if (!(direction == "up" && index == 0) &&
		    !(direction == "down" &&
		      index == (ui->timerMainLayout->count() - 1))) {
			const int newIndex = direction == "up" ? index - 1
							       : index + 1;
			ui->timerMainLayout->removeWidget(timerWidget);
			ui->timerMainLayout->insertWidget(newIndex,
							  timerWidget);
			UpdateTimerListMoveButtonState();
		}
	}
}

void CountdownDockWidget::StartAllTimers()
{
	int timerWidgetCount = ui->timerMainLayout->count();
	for (int i = 0; i < timerWidgetCount; i++) {
		AshmanixTimer *timerWidget = static_cast<AshmanixTimer *>(
			ui->timerMainLayout->itemAt(i)->widget());
		if (timerWidget) {
			timerWidget->StartTimer();
		}
	}
}

void CountdownDockWidget::StopAllTimers()
{
	int timerWidgetCount = ui->timerMainLayout->count();
	for (int i = 0; i < timerWidgetCount; i++) {
		AshmanixTimer *timerWidget = static_cast<AshmanixTimer *>(
			ui->timerMainLayout->itemAt(i)->widget());
		if (timerWidget) {
			timerWidget->StopTimer();
		}
	}
}
