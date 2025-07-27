#ifndef TIMERENGINE_H
#define TIMERENGINE_H

#include <QWidget>
#include <QObject>

// Forward declarations
class QTimer;
class AshmanixTimer;
class TimerUIManager;
struct TimerWidgetStruct;

class TimerEngine : public QObject {
	Q_OBJECT

public:
	explicit TimerEngine(AshmanixTimer *parent, TimerWidgetStruct *data);
	void Initialise();
	void Start();
	void Stop();
	void Set(long long millis, bool reset);
	bool AddTime();
	bool SetTime(const char *stringTime);

signals:
	void TimerStarted();
	void TimerStopped();
	void TimerReset();
	void TimerTicked(long long millis);
	void TimerEnded();

private slots:
	void Adjust();

private:
	long long lastDisplayedSeconds = -1;

	AshmanixTimer *parent;
	TimerWidgetStruct *data;
	QTimer *qtimer = nullptr;
};

#endif // TIMERENGINE_H
