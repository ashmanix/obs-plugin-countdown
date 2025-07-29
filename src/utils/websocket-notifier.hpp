#ifndef WEBSOCKETNOTIFIER_H
#define WEBSOCKETNOTIFIER_H

#include <QObject>
#include <obs-data.h>
#include <obs-websocket-api.h>
#include "./timer-utils.hpp"

// Forward declarations
class CountdownDockWidget;

class WebsocketNotifier : public QObject {
	Q_OBJECT

public:
	struct WebsocketCallbackData {
		CountdownDockWidget *instance;
		WebsocketRequestType requestType;
		const char *requestDataKey;
		const char *requestTimerIdKey;
	};

	explicit WebsocketNotifier(CountdownDockWidget *countdownDockWidget);
	void SendTickEvent(QString timerId, QString timeString, long long timeLeftInMillis);
	void SendStateEvent(QString timerId, const char *state, QString selectedSource = "");

signals:
	void ChangeSingleTimerState(WebsocketRequestType request, QString timerId);
	void ChangeAllTimerState(WebsocketRequestType request);

private:
	CountdownDockWidget *countdownDockWidget = nullptr;
	obs_websocket_vendor vendor = nullptr;
	static inline const char *VENDORNAME = "ashmanix-countdown-timer";
	static inline const char *TIMERIDKEY = "timer_id";

	void EmitWebsocketSendEvent(const char *eventName, obs_data_t *eventData);
	static void WebSocketButtonRequest(obs_data_t *request_data, obs_data_t *response_data, void *priv_data);
	static void GetTimerStateViaWebsocket(obs_data_t *request_data, obs_data_t *response_data, void *priv_data);
	static void ChangeTimerTimeViaWebsocket(obs_data_t *request_data, obs_data_t *response_data, void *priv_data);
};

#endif // WEBSOCKETNOTIFIER_H
