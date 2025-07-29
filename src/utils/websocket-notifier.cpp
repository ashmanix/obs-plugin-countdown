#include "websocket-notifier.hpp"
#include "../countdown-widget.hpp"
#include "../widgets/ashmanix-timer.hpp"
#include "plugin-support.h"

WebsocketNotifier::WebsocketNotifier(CountdownDockWidget *countdownDockWidget)
	: countdownDockWidget(countdownDockWidget)
{

	vendor = obs_websocket_register_vendor(VENDORNAME);

	if (!vendor) {
		obs_log(LOG_ERROR, "Error registering vendor to websocket!");
		return;
	}

	obs_websocket_vendor_register_request(
		vendor, "period_play", WebSocketButtonRequest,
		new WebsocketCallbackData{countdownDockWidget, WebsocketRequestType::PERIOD_PLAY, nullptr, TIMERIDKEY});
	obs_websocket_vendor_register_request(vendor, "period_pause", WebSocketButtonRequest,
					      new WebsocketCallbackData{countdownDockWidget,
									WebsocketRequestType::PERIOD_PAUSE, nullptr,
									TIMERIDKEY});
	obs_websocket_vendor_register_request(
		vendor, "period_set", WebSocketButtonRequest,
		new WebsocketCallbackData{countdownDockWidget, WebsocketRequestType::PERIOD_SET, nullptr, TIMERIDKEY});

	obs_websocket_vendor_register_request(vendor, "to_time_play", WebSocketButtonRequest,
					      new WebsocketCallbackData{countdownDockWidget,
									WebsocketRequestType::TO_TIME_PLAY, nullptr,
									TIMERIDKEY});
	obs_websocket_vendor_register_request(vendor, "to_time_stop", WebSocketButtonRequest,
					      new WebsocketCallbackData{countdownDockWidget,
									WebsocketRequestType::TO_TIME_STOP, nullptr,
									TIMERIDKEY});

	obs_websocket_vendor_register_request(
		vendor, "play_all", WebSocketButtonRequest,
		new WebsocketCallbackData{countdownDockWidget, WebsocketRequestType::PLAY_ALL, nullptr, nullptr});

	obs_websocket_vendor_register_request(
		vendor, "stop_all", WebSocketButtonRequest,
		new WebsocketCallbackData{countdownDockWidget, WebsocketRequestType::STOP_ALL, nullptr, nullptr});

	obs_websocket_vendor_register_request(
		vendor, "get_timer_state", GetTimerStateViaWebsocket,
		new WebsocketCallbackData{countdownDockWidget, WebsocketRequestType::GET_TIME, nullptr, TIMERIDKEY});

	obs_websocket_vendor_register_request(vendor, "add_time", ChangeTimerTimeViaWebsocket,
					      new WebsocketCallbackData{countdownDockWidget,
									WebsocketRequestType::ADD_TIME, "time_to_add",
									TIMERIDKEY});

	obs_websocket_vendor_register_request(vendor, "set_time", ChangeTimerTimeViaWebsocket,
					      new WebsocketCallbackData{countdownDockWidget,
									WebsocketRequestType::SET_TIME, "time_to_set",
									TIMERIDKEY});
}

void WebsocketNotifier::SendTickEvent(QString timerId, QString timeString, long long timeLeftInMillis)
{
	obs_data_t *eventData = obs_data_create();

	obs_data_set_string(eventData, "timer_id", timerId.toStdString().c_str());
	obs_data_set_string(eventData, "time_display", timeString.toStdString().c_str());
	obs_data_set_int(eventData, "time_left_ms", timeLeftInMillis);

	EmitWebsocketSendEvent("timer_tick", eventData);
	obs_data_release(eventData);
}

void WebsocketNotifier::SendStateEvent(QString timerId, const char *state, QString selectedSource)
{
	obs_data_t *eventData = obs_data_create();
	obs_data_set_string(eventData, "timer_id", timerId.toStdString().c_str());
	obs_data_set_string(eventData, "state", state);

	if (!selectedSource.isEmpty()) {
		obs_data_set_string(eventData, "text_source", selectedSource.toStdString().c_str());
	}

	EmitWebsocketSendEvent("timer_state_changed", eventData);
	obs_data_release(eventData);
}

void WebsocketNotifier::EmitWebsocketSendEvent(const char *eventName, obs_data_t *eventData)
{
	if (!vendor)
		return;

	obs_websocket_vendor_emit_event(vendor, eventName, eventData);
}

void WebsocketNotifier::WebSocketButtonRequest(obs_data_t *request_data, obs_data_t *response_data, void *priv_data)
{
	auto *callback_data = static_cast<WebsocketCallbackData *>(priv_data);
	WebsocketRequestType requestType = callback_data->requestType;
	CountdownDockWidget *countdownWidget = callback_data->instance;

	auto setResponse = [&](bool success, const char *msg) {
		obs_data_set_bool(response_data, "success", success);
		obs_data_set_string(response_data, "message", msg);
	};

	switch (requestType) {
	case WebsocketRequestType::PLAY_ALL:
		countdownWidget->StartAllTimers();
		setResponse(true, "Start All Timers button pressed");
		return;
	case WebsocketRequestType::STOP_ALL:
		countdownWidget->StopAllTimers();
		setResponse(true, "Stop All Timers button pressed");
		return;
	default:
		break;
	}

	const char *requestTimerIdKey = callback_data->requestTimerIdKey;
	const char *websocketTimerID = obs_data_get_string(request_data, requestTimerIdKey);
	AshmanixTimer *timer = countdownWidget->AttemptToGetTimerWidgetById(websocketTimerID);

	if (!timer) {
		obs_log(LOG_WARNING, "Countdown widget not found for websocket timer state change request!");
		obs_data_set_bool(response_data, "success", false);
		obs_data_set_string(response_data, "message", "Error trying to change timer state!");
		return;
	}

	switch (requestType) {
	case WebsocketRequestType::PERIOD_PLAY:
		timer->ActivateTimerAction(TimerAction::PLAY);
		setResponse(true, "Play button pressed");
		break;
	case WebsocketRequestType::PERIOD_PAUSE:
		timer->ActivateTimerAction(TimerAction::PAUSE);
		setResponse(true, "Pause button pressed");
		break;
	case WebsocketRequestType::PERIOD_SET:
		timer->ActivateTimerAction(TimerAction::RESET);
		setResponse(true, "Reset button pressed");
		break;
	case WebsocketRequestType::TO_TIME_PLAY:
		timer->ActivateTimerAction(TimerAction::TO_TIME_PLAY);
		setResponse(true, "To Time play button pressed");
		break;
	case WebsocketRequestType::TO_TIME_STOP:
		timer->ActivateTimerAction(TimerAction::TO_TIME_STOP);
		setResponse(true, "To Time stop button pressed");
		break;
	default:
		setResponse(false, "No buttons pressed");
		break;
	}
}

void WebsocketNotifier::GetTimerStateViaWebsocket(obs_data_t *request_data, obs_data_t *response_data, void *priv_data)
{
	auto *callback_data = static_cast<WebsocketCallbackData *>(priv_data);
	const char *requestTimerIdKey = callback_data->requestTimerIdKey;

	CountdownDockWidget *countdownWidget = callback_data->instance;
	const char *websocketTimerID = obs_data_get_string(request_data, requestTimerIdKey);

	AshmanixTimer *timer = countdownWidget->AttemptToGetTimerWidgetById(websocketTimerID);

	if (timer != nullptr) {
		TimerWidgetStruct *timerData = timer->GetTimerData();
		obs_data_set_bool(response_data, "is_running", timerData->isPlaying);
		obs_data_set_int(response_data, "time_left_ms", timerData->timeLeftInMillis);

		obs_data_set_string(response_data, "timer_id", timerData->timerId.toStdString().c_str());

		obs_data_set_bool(response_data, "success", true);

	} else {
		obs_log(LOG_WARNING, "Countdown widget not found for websocket request!");
		obs_data_set_bool(response_data, "success", false);
		obs_data_set_string(response_data, "message", "Error trying to get timer data!");
	}
}

void WebsocketNotifier::ChangeTimerTimeViaWebsocket(obs_data_t *request_data, obs_data_t *response_data,
						    void *priv_data)
{
	auto *callback_data = static_cast<WebsocketCallbackData *>(priv_data);
	WebsocketRequestType requestType = callback_data->requestType;
	const char *requestDataTimeKey = callback_data->requestDataKey;
	const char *requestTimerIdKey = callback_data->requestTimerIdKey;

	const char *websocketDataTime = obs_data_get_string(request_data, requestDataTimeKey);

	QString dataStr = websocketDataTime ? QString::fromUtf8(websocketDataTime) : QString();

	if (dataStr.isEmpty()) {
		obs_data_set_bool(response_data, "success", false);
		QString error_message = QString("%1 field is missing from request!").arg(requestDataTimeKey);
		obs_data_set_string(response_data, "message", error_message.toStdString().c_str());
	} else {
		CountdownDockWidget *countdownWidget = callback_data->instance;
		const char *websocketTimerID = obs_data_get_string(request_data, requestTimerIdKey);

		AshmanixTimer *timer = countdownWidget->AttemptToGetTimerWidgetById(websocketTimerID);

		if (timer != nullptr) {
			bool result = timer->AlterTime(requestType, websocketDataTime);
			const char *type_string = requestType == WebsocketRequestType::ADD_TIME ? "added" : "set";
			obs_log(LOG_INFO, "Time %s due to websocket call: %s", type_string, websocketDataTime);
			obs_data_set_bool(response_data, "success", result);
		} else {
			obs_log(LOG_WARNING, "Countdown widget not found for websocket request!");
			obs_data_set_bool(response_data, "success", false);
			obs_data_set_string(response_data, "message", "Error trying to update time!");
		}
	}
}
