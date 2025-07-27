#ifndef WEBSOCKETNOTIFIER_H
#define WEBSOCKETNOTIFIER_H

#include <QObject>
#include <obs-data.h>

struct TimerWidgetStruct;

class WebsocketNotifier : public QObject {
	Q_OBJECT

public:
	explicit WebsocketNotifier(obs_websocket_vendor vendor);
	void SendTickEvent(QString timerId, QString timeString, long long timeLeftInMillis);
	void SendStateEvent(QString timerId, const char *state, QString selectedSource = "");

signals:
	void RequestSendWebsocketEvent(const char *eventName, obs_data_t *eventData);

private:
	obs_websocket_vendor vendor = nullptr;
	void EmitWebsocketSendEvent(const char *eventName, obs_data_t *eventData);
};

#endif // WEBSOCKETNOTIFIER_H
