#include "countdown-widget.hpp"

const char CountdownDockWidget::ZEROSTRING[] = "00:00:00:00";

CountdownDockWidget::CountdownDockWidget(QWidget *parent)
	: QWidget(parent),
	  ui(new Ui::CountdownTimer)
{

	ui->setupUi(this);
	timerListLayout = ui->timerMainLayout;

	SetupCountdownWidgetUI();
	resize(300, 380);

	obs_frontend_add_event_callback(OBSFrontendEventHandler, this);

	ConnectUISignalHandlers();

	RegisterHotkeys();
}

CountdownDockWidget::~CountdownDockWidget()
{
	SaveSettings();
	UnregisterHotkeys();
}

void CountdownDockWidget::ConfigureWebSocketConnection()
{
	vendor = obs_websocket_register_vendor(VENDORNAME);

	if (!vendor) {
		obs_log(LOG_ERROR, "Error registering vendor to websocket!");
		return;
	}

	// #define WEBSOCKET_CALLBACK(method, log_action)                              \
// 	[](obs_data_t *request_data, obs_data_t *response_data,             \
// 	   void *incoming_data) {                                           \
// 		UNUSED_PARAMETER(request_data);                             \
// 		CountdownDockWidget &cdWidget =                             \
// 			*static_cast<CountdownDockWidget *>(incoming_data); \
// 		obs_log(LOG_INFO, log_action " due to websocket call");     \
// 		method();                                                   \
// 		obs_data_set_bool(response_data, "success", true);          \
// 	}

	// 	obs_websocket_vendor_register_request(
	// 		vendor, "period_play",
	// 		WEBSOCKET_CALLBACK(cdWidget.ui->playButton->click,
	// 				   "Period play button pressed"),
	// 		this);
	// 	obs_websocket_vendor_register_request(
	// 		vendor, "period_pause",
	// 		WEBSOCKET_CALLBACK(cdWidget.ui->pauseButton->click,
	// 				   "Period pause button pressed"),
	// 		this);
	// 	obs_websocket_vendor_register_request(
	// 		vendor, "period_set",
	// 		WEBSOCKET_CALLBACK(cdWidget.ui->resetButton->click,
	// 				   "Period Set button pressed"),
	// 		this);

	// 	obs_websocket_vendor_register_request(
	// 		vendor, "to_time_play",
	// 		WEBSOCKET_CALLBACK(cdWidget.ui->toTimePlayButton->click,
	// 				   "To time play button pressed"),
	// 		this);
	// 	obs_websocket_vendor_register_request(
	// 		vendor, "to_time_stop",
	// 		WEBSOCKET_CALLBACK(cdWidget.ui->toTimeStopButton->click,
	// 				   "To time stop button pressed"),
	// 		this);

	// 	obs_websocket_vendor_register_request(
	// 		vendor, "get_timer_state",
	// 		[](obs_data_t *request_data, obs_data_t *response_data,
	// 		   void *priv_data) {
	// 			UNUSED_PARAMETER(request_data);
	// 			CountdownDockWidget *self =
	// 				static_cast<CountdownDockWidget *>(priv_data);

	// 			obs_data_set_bool(response_data, "is_running",
	// 					  self->countdownTimerData->isPlaying);
	// 			obs_data_set_int(
	// 				response_data, "time_left_ms",
	// 				self->countdownTimerData->timeLeftInMillis);

	// 			obs_data_set_bool(response_data, "success", true);
	// 		},
	// 		this);
	// #undef WEBSOCKET_CALLBACK

	obs_websocket_vendor_register_request(
		vendor, "get_timer_state", GetTimerStateViaWebsocket,
		new WebsocketCallbackData{this,GET_TIME, NULL, "timer_id"});

	obs_websocket_vendor_register_request(
		vendor, "add_time", ChangeTimerTimeViaWebsocket,
		new WebsocketCallbackData{this, ADD_TIME, "time_to_add",
					  "timer_id"});

	obs_websocket_vendor_register_request(
		vendor, "set_time", ChangeTimerTimeViaWebsocket,
		new WebsocketCallbackData{this, SET_TIME, "time_to_set",
					  "timer_id"});
}

void CountdownDockWidget::SetupCountdownWidgetUI()
{

	// CountdownWidgetStruct *context = countdownStruct;

	ui->addTimerButton->setProperty("themeID", "addIconSmall");
	ui->addTimerButton->setEnabled(true);
	ui->addTimerButton->setToolTip(obs_module_text("AddTimerButtonTip"));

	// ui->timeDisplay->display(CountdownDockWidget::ZEROSTRING);

	// ui->dateTimeEdit->setMinimumDate(QDate::currentDate());
	// ui->dateTimeEdit->setMaximumDate(QDate::currentDate().addDays(999));

	// ui->daysCheckBox->setText(obs_module_text("DaysCheckboxLabel"));
	// ui->daysCheckBox->setCheckState(Qt::Checked);
	// ui->daysCheckBox->setToolTip(obs_module_text("DaysCheckBoxTip"));
	// ui->timerDays->setValidator(new QRegularExpressionValidator(
	// 	QRegularExpression("^(0|[1-9]\\d{0,2})$"), this));

	// ui->hoursCheckBox->setText(obs_module_text("HoursCheckboxLabel"));
	// ui->hoursCheckBox->setCheckState(Qt::Checked);
	// ui->hoursCheckBox->setToolTip(obs_module_text("HoursCheckBoxTip"));
	// ui->timerHours->setMaxLength(2);
	// ui->timerHours->setValidator(new QRegularExpressionValidator(
	// 	QRegularExpression("^(0?[0-9]|1[0-9]|2[0-3])$"), this));

	// ui->minutesCheckBox->setText(obs_module_text("MinutesCheckboxLabel"));
	// ui->minutesCheckBox->setCheckState(Qt::Checked);
	// ui->minutesCheckBox->setToolTip(obs_module_text("MinutesCheckBoxTip"));
	// ui->timerMinutes->setMaxLength(2);
	// ui->timerMinutes->setValidator(new QRegularExpressionValidator(
	// 	QRegularExpression("^[1-5]?[0-9]"), this));

	// ui->secondsCheckBox->setText(obs_module_text("SecondsCheckboxLabel"));
	// ui->secondsCheckBox->setCheckState(Qt::Checked);
	// ui->secondsCheckBox->setToolTip(obs_module_text("SecondsCheckBoxTip"));
	// ui->timerSeconds->setAlignment(Qt::AlignCenter);
	// ui->timerSeconds->setMaxLength(2);
	// ui->timerSeconds->setValidator(new QRegularExpressionValidator(
	// 	QRegularExpression("^[1-5]?[0-9]"), this));

	// ui->leadZeroCheckBox->setText(obs_module_text("LeadZeroCheckboxLabel"));
	// ui->leadZeroCheckBox->setCheckState(Qt::Checked);
	// ui->leadZeroCheckBox->setToolTip(
	// 	obs_module_text("LeadZeroCheckBoxTip"));

	// ui->countUpCheckBox->setText(obs_module_text("CountUpCheckBoxLabel"));
	// ui->countUpCheckBox->setCheckState(Qt::Checked);
	// ui->countUpCheckBox->setToolTip(obs_module_text("CountUpCheckBoxTip"));

	// ui->countdownTypeTabWidget->setTabText(
	// 	0, obs_module_text("SetPeriodTabLabel"));
	// ui->countdownTypeTabWidget->setTabText(
	// 	1, obs_module_text("SetDatetimeTabLabel"));
	// ui->countdownTypeTabWidget->setToolTip(
	// 	obs_module_text("SetCountdownTypeTip"));

	// ui->textSourceDropdownList->setToolTip(
	// 	obs_module_text("TextSourceDropdownTip"));
	// ui->textSourceDropdownLabel->setText(
	// 	obs_module_text("TextSourceLabel"));

	// ui->endMessageCheckBox->setCheckState(Qt::Unchecked);
	// ui->endMessageCheckBox->setToolTip(
	// 	obs_module_text("EndMessageCheckBoxTip"));
	// ui->endMessageCheckBox->setText(obs_module_text("EndMessageLabel"));
	// // ui->timerEndLabel->setEnabled(false);
	// ui->endMessageLineEdit->setEnabled(false);
	// ui->endMessageLineEdit->setToolTip(
	// 	obs_module_text("EndMessageLineEditTip"));

	// ui->switchSceneCheckBox->setCheckState(Qt::Unchecked);
	// ui->switchSceneCheckBox->setToolTip(
	// 	obs_module_text("SwitchSceneCheckBoxTip"));
	// ui->switchSceneCheckBox->setText(obs_module_text("SwitchScene"));
	// // ui->sceneSwitchLabel->setEnabled(false);
	// ui->sceneSourceDropdownList->setEnabled(false);
	// ui->sceneSourceDropdownList->setToolTip(
	// 	obs_module_text("SceneSourceDropdownTip"));

	// ui->playButton->setProperty("themeID", "playIcon");
	// ui->playButton->setEnabled(true);
	// ui->playButton->setToolTip(obs_module_text("PlayButtonTip"));
	// ui->pauseButton->setProperty("themeID", "pauseIcon");
	// ui->pauseButton->setEnabled(false);
	// ui->pauseButton->setToolTip(obs_module_text("PauseButtonTip"));
	// ui->resetButton->setProperty("themeID", "restartIcon");
	// ui->resetButton->setToolTip(obs_module_text("ResetButtonTip"));

	// ui->toTimePlayButton->setProperty("themeID", "playIcon");
	// ui->toTimePlayButton->setEnabled(true);
	// ui->toTimePlayButton->setToolTip(
	// 	obs_module_text("ToTimePlayButtonTip"));
	// ui->toTimeStopButton->setProperty("themeID", "stopIcon");
	// ui->toTimeStopButton->setEnabled(false);
	// ui->toTimeStopButton->setToolTip(
	// 	obs_module_text("ToTimeStopButtonTip"));

	// context->isPlaying = false;
}

void CountdownDockWidget::ConnectUISignalHandlers()
{
	QObject::connect(ui->addTimerButton, SIGNAL(clicked()),
			 SLOT(AddTimerButtonClicked()));
}

void CountdownDockWidget::ConnectTimerSignalHandlers(AshmanixTimer *timerWidget)
{
	connect(timerWidget, &AshmanixTimer::RequestDelete, this,
		&CountdownDockWidget::RemoveTimerButtonClicked);

	connect(timerWidget, &AshmanixTimer::RequestSendWebsocketEvent, this,
		&CountdownDockWidget::HandleWebsocketSendEvent);
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
					SaveTimerWidgetDataToOBSSaveData(
						dataObject, timerData);
					obs_data_array_push_back(obsDataArray,
								 dataObject);
					obs_data_release(dataObject);
				}
			}
		}
	}

	// // Hotkeys
	// auto SaveHotkey = [](obs_data_t *sv_data, obs_hotkey_id id,
	// 		     const char *name) {
	// 	obs_log(LOG_INFO, "Hotkey ID: %i, Value: %s", (int)id, name);
	// 	if ((int)id == -1)
	// 		return;
	// 	OBSDataArrayAutoRelease array = obs_hotkey_save(id);
	// 	obs_data_set_array(sv_data, name, array);
	// };

	// obs_data_array_t *start_countdown_hotkey_save_array =
	// 	obs_hotkey_save(context->startCountdownHotkeyId);
	// obs_data_set_array(obsData, "Ashmanix_Countdown_Timer_Start",
	// 		   start_countdown_hotkey_save_array);
	// obs_data_array_release(start_countdown_hotkey_save_array);

	// obs_data_array_t *pause_countdown_hotkey_save_array =
	// 	obs_hotkey_save(context->pauseCountdownHotkeyId);
	// obs_data_set_array(obsData, "Ashmanix_Countdown_Timer_Pause",
	// 		   pause_countdown_hotkey_save_array);
	// obs_data_array_release(pause_countdown_hotkey_save_array);

	// obs_data_array_t *set_countdown_hotkey_save_array =
	// 	obs_hotkey_save(context->setCountdownHotkeyId);
	// obs_data_set_array(obsData, "Ashmanix_Countdown_Timer_Set",
	// 		   set_countdown_hotkey_save_array);
	// obs_data_array_release(set_countdown_hotkey_save_array);

	// SaveHotkey(obsData, context->startCountdownToTimeHotkeyId,
	// 	   "Ashmanix_Countdown_Timer_To_Time_Start");
	// SaveHotkey(obsData, context->stopCountdownToTimeHotkeyId,
	// 	   "Ashmanix_Countdown_Timer_To_Time_Stop");

	obs_data_set_array(settings, "timer_widgets", obsDataArray);
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
	bfree(file);
}

void CountdownDockWidget::RegisterHotkeys()
{
	// auto LoadHotkey = [](obs_data_t *s_data, obs_hotkey_id id,
	// 		     const char *name) {
	// 	if ((int)id == -1)
	// 		return;

	// 	OBSDataArrayAutoRelease array =
	// 		obs_data_get_array(s_data, name);

	// 	obs_hotkey_load(id, array);
	// 	// obs_data_array_release(array);
	// };

	// char *file = obs_module_config_path(CONFIG);
	// obs_data_t *saved_data = nullptr;
	// if (file) {
	// 	saved_data = obs_data_create_from_json_file(file);
	// 	bfree(file);
	// }

	// #define HOTKEY_CALLBACK(pred, method, log_action)                              \
// 	[](void *incoming_data, obs_hotkey_id, obs_hotkey_t *, bool pressed) { \
// 		Ui::CountdownTimer &countdownUi =                              \
// 			*static_cast<Ui::CountdownTimer *>(incoming_data);     \
// 		if ((pred) && pressed) {                                       \
// 			obs_log(LOG_INFO, log_action " due to hotkey");        \
// 			method();                                              \
// 		}                                                              \
// 	}
	// 	// Register Play Hotkey
	// 	context->startCountdownHotkeyId = (int)obs_hotkey_register_frontend(
	// 		"Ashmanix_Countdown_Timer_Start",
	// 		obs_module_text("StartCountdownHotkeyDescription"),
	// 		HOTKEY_CALLBACK(true, countdownUi.playButton->animateClick,
	// 				"Play Button Pressed"),
	// 		ui);
	// 	if (saved_data)
	// 		LoadHotkey(saved_data, context->startCountdownHotkeyId,
	// 			   "Ashmanix_Countdown_Timer_Start");

	// 	// Register Pause Hotkey
	// 	context->pauseCountdownHotkeyId = (int)obs_hotkey_register_frontend(
	// 		"Ashmanix_Countdown_Timer_Pause",
	// 		obs_module_text("PauseCountdownHotkeyDescription"),
	// 		HOTKEY_CALLBACK(true, countdownUi.pauseButton->animateClick,
	// 				"Pause Button Pressed"),
	// 		ui);
	// 	if (saved_data)
	// 		LoadHotkey(saved_data, context->pauseCountdownHotkeyId,
	// 			   "Ashmanix_Countdown_Timer_Pause");

	// 	// Register Reset Hotkey
	// 	context->setCountdownHotkeyId = (int)obs_hotkey_register_frontend(
	// 		"Ashmanix_Countdown_Timer_Set",
	// 		obs_module_text("SetCountdownHotkeyDescription"),
	// 		HOTKEY_CALLBACK(true, countdownUi.resetButton->animateClick,
	// 				"Set Button Pressed"),
	// 		ui);
	// 	if (saved_data)
	// 		LoadHotkey(saved_data, context->setCountdownHotkeyId,
	// 			   "Ashmanix_Countdown_Timer_Set");

	// 	// Register To Time Start Hotkey
	// 	context->startCountdownToTimeHotkeyId =
	// 		(int)obs_hotkey_register_frontend(
	// 			"Ashmanix_Countdown_Timer_To_Time_Start",
	// 			obs_module_text(
	// 				"StartCountdownToTimeHotkeyDescription"),
	// 			HOTKEY_CALLBACK(
	// 				true,
	// 				countdownUi.toTimePlayButton->animateClick,
	// 				"To Time Start Button Pressed"),
	// 			ui);
	// 	if (saved_data)
	// 		LoadHotkey(saved_data, context->startCountdownToTimeHotkeyId,
	// 			   "Ashmanix_Countdown_Timer_To_Time_Start");

	// 	// Register To Time Stop Hotkey
	// 	context->stopCountdownToTimeHotkeyId = (int)obs_hotkey_register_frontend(
	// 		"Ashmanix_Countdown_Timer_To_Time_Stop",
	// 		obs_module_text("StopCountdownToTimeHotkeyDescription"),
	// 		HOTKEY_CALLBACK(true,
	// 				countdownUi.toTimeStopButton->animateClick,
	// 				"To Time Stop Button Pressed"),
	// 		ui);
	// 	if (saved_data)
	// 		LoadHotkey(saved_data, context->stopCountdownToTimeHotkeyId,
	// 			   "Ashmanix_Countdown_Timer_To_Time_Stop");

	// obs_data_release(saved_data);
	// #undef HOTKEY_CALLBACK
}

void CountdownDockWidget::UnregisterHotkeys()
{
	// if (countdownTimerData->startCountdownHotkeyId)
	// 	obs_hotkey_unregister(
	// 		countdownTimerData->startCountdownHotkeyId);
	// if (countdownTimerData->pauseCountdownHotkeyId)
	// 	obs_hotkey_unregister(
	// 		countdownTimerData->pauseCountdownHotkeyId);
	// if (countdownTimerData->setCountdownHotkeyId)
	// 	obs_hotkey_unregister(countdownTimerData->setCountdownHotkeyId);

	// if (countdownTimerData->startCountdownToTimeHotkeyId)
	// 	obs_hotkey_unregister(
	// 		countdownTimerData->startCountdownToTimeHotkeyId);
	// if (countdownTimerData->stopCountdownToTimeHotkeyId)
	// 	obs_hotkey_unregister(
	// 		countdownTimerData->stopCountdownToTimeHotkeyId);
}

void CountdownDockWidget::AddTimer(
	TimerWidgetStruct timerData = TimerWidgetStruct())
{
	QString newId;

	if (!timerData.timerId.isEmpty()) {
		newId = timerData.timerId;
	} else {
		// Create a unique ID for the timer
		QUuid uuid = QUuid::createUuid();
		QByteArray hash = QCryptographicHash::hash(
			uuid.toByteArray(), QCryptographicHash::Md5);
		newId = QString(hash.toHex().left(
			8)); // We take the first 8 characters of the hash
	}

	AshmanixTimer *newTimer = new AshmanixTimer(this, newId, vendor);

	// Set timer data if provided
	newTimer->SetTimerData(timerData);

	timerWidgetMap.insert(newId, newTimer);
	ConnectTimerSignalHandlers(newTimer);

	timerListLayout->addWidget(newTimer);
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

		AshmanixTimer *timer = nullptr;

		if (websocketTimerID != nullptr &&
		    strlen(websocketTimerID) > 0) {
			timer = countdownWidget->timerWidgetMap.value(
				websocketTimerID, nullptr);
		} else if (countdownWidget->timerListLayout->count()) {
			QLayoutItem *layoutItem =
				countdownWidget->timerListLayout->itemAt(0);
			if (layoutItem) {
				timer = qobject_cast<AshmanixTimer *>(
					layoutItem->widget());
			}
		}

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

void CountdownDockWidget::OBSFrontendEventHandler(enum obs_frontend_event event,
						  void *private_data)
{

	CountdownDockWidget *countdownDockWidget =
		(CountdownDockWidget *)private_data;

	switch (event) {
	case OBS_FRONTEND_EVENT_FINISHED_LOADING: {
		// CountdownDockWidget::ConnectUISignalHandlers(context);
		CountdownDockWidget::LoadSavedSettings(countdownDockWidget);
	} break;
	default:
		break;
	}
}

void CountdownDockWidget::LoadSavedSettings(CountdownDockWidget *dockWidget)
{
	UNUSED_PARAMETER(dockWidget);
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
			int count = obs_data_array_count(timersArray);
			for (int i = 0; i < count; ++i) {
				obs_data_t *timerDataObj =
					obs_data_array_item(timersArray, i);
				TimerWidgetStruct timerWidgetData =
					TimerWidgetStruct();
				LoadTimerWidgetDataFromOBSSaveData(
					timerDataObj, &timerWidgetData);
				dockWidget->AddTimer(timerWidgetData);
			}
		}
		// 	const char *jsonString =
		// 		obs_data_get_string(data, "timer_data");
		// 	if (jsonString) {
		// 		QJsonDocument jsonDoc =
		// 			QJsonDocument::fromJson(QByteArray(jsonString));
		// 		QJsonArray jsonArray = jsonDoc.array();

		// 		for (const QJsonValue value : jsonArray) {
		// 			QJsonObject obj = value.toObject();
		// 			QString widgetKey = obj["key"].toString();

		// 			TimerWidgetStruct timerWidgetData =
		// 				JsonToTimerWidgetStruct(obj);
		// 			dockWidget->AddTimer(timerWidgetData);
		// 		}
		// }
		obs_data_release(data);
	}
}

// --------------------------------- Private Slots ----------------------------------

void CountdownDockWidget::AddTimerButtonClicked()
{
	// obs_log(LOG_INFO, "Adding timer to list!");
	// Create a unique ID for the timer
	QUuid uuid = QUuid::createUuid();
	QByteArray hash = QCryptographicHash::hash(uuid.toByteArray(),
						   QCryptographicHash::Md5);
	QString newId = QString(hash.toHex().left(
		8)); // We take the first 8 characters of the hash

	AshmanixTimer *newTimer = new AshmanixTimer(nullptr, newId, vendor);
	timerWidgetMap.insert(newId, newTimer);
	connect(newTimer, &AshmanixTimer::RequestDelete, this,
		&CountdownDockWidget::RemoveTimerButtonClicked);

	timerListLayout->addWidget(newTimer);
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
}

void CountdownDockWidget::HandleTimerReset()
{
	// countdownTimerData->timer->stop();
	// countdownTimerData->timer->start(COUNTDOWNPERIOD);
}

void CountdownDockWidget::HandleWebsocketSendEvent(const char *eventName,
						   obs_data_t *eventData)
{
	if (!vendor)
		return;

	obs_websocket_vendor_emit_event(vendor, eventName, eventData);
}
