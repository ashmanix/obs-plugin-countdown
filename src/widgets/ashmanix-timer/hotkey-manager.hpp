#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H

#include <QObject>
#include <QWidget>
#include <obs-data.h>

class Ui_AshmanixTimer;
struct TimerWidgetStruct;

class HotkeyManager {
public:
	static inline const char *TIMERSTARTHOTKEYNAME = "Ashmanix_Countdown_Timer_Start";
	static inline const char *TIMERPAUSEHOTKEYNAME = "Ashmanix_Countdown_Timer_Pause";
	static inline const char *TIMERSETHOTKEYNAME = "Ashmanix_Countdown_Timer_Set";
	static inline const char *TIMERTOTIMESTARTHOTKEYNAME = "Ashmanix_Countdown_Timer_To_Time_Start";
	static inline const char *TIMERTOTIMESTOPHOTKEYNAME = "Ashmanix_Countdown_Timer_To_Time_Stop";

	HotkeyManager(QWidget *parentWidget, Ui_AshmanixTimer *ui, TimerWidgetStruct *data);
	void RegisterAllHotkeys(obs_data_t *saved_data);
	void UnregisterAllHotkeys();
	std::string GetFullHotkeyName(const std::string &nameString, const char *joinText = "_");

private:
	QWidget *parent;
	Ui_AshmanixTimer *ui;
	TimerWidgetStruct *data;
};

#endif // HOTKEYMANAGER_H
