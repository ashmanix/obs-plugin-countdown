#include "websocket-notifier.hpp"

WebsocketNotifier::WebsocketNotifier(obs_websocket_vendor vendor)
{
	vendor = &vendor;
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

	if (selectedSource.length() > 0) {
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
