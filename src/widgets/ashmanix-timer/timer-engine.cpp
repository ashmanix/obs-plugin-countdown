#include "timer-engine.hpp"
#include <QTimer>
#include "../ashmanix-timer.hpp"
#include "../ashmanix-timer/timer-ui-manager.hpp"
#include "../../utils/obs-utils.hpp"
#include "../../utils/timer-utils.hpp"

TimerEngine::TimerEngine(AshmanixTimer *parent, TimerWidgetStruct *data) : parent(parent), data(data) {}

// ------------------------------ Public Functions ----------------------------------

void TimerEngine::Initialise()
{
	if (qtimer == nullptr)
		qtimer = new QTimer();

	data->timer = qtimer;
	QObject::connect(data->timer, SIGNAL(timeout()), SLOT(Adjust()));
}

void TimerEngine::Stop()
{
	if (qtimer->isActive()) {
		qtimer->stop();
		emit TimerStopped();
	}
}

void TimerEngine::Start()
{
	if (qtimer->isActive()) {
		qtimer->start();
		emit TimerStopped();
	}
}

void TimerEngine::Set(long long millis, bool reset)
{
	if (reset)
		data->timeLeftInMillis = millis;

	if (!qtimer->isActive())
		qtimer->start(TIMERPERIOD);

	data->timeAtTimerStart = QDateTime::currentDateTime().addMSecs(data->shouldCountUp ? -data->timeLeftInMillis
											   : data->timeLeftInMillis);
	emit TimerStarted();
}

// --------------------------------- Private Slots ----------------------------------

void TimerEngine::Adjust()
{
	if (data->shouldCountUp) {
		data->timeLeftInMillis += TIMERPERIOD;
	} else {
		data->timeLeftInMillis -= TIMERPERIOD;
		if (data->timeLeftInMillis <= 0) {
			data->timeLeftInMillis = 0;
			Stop();
			emit TimerEnded();
			return;
		}
	}

	emit TimerTicked(data->timeLeftInMillis);
}
