#ifndef COUNTDOWNWIDGET_H
#define COUNTDOWNWIDGET_H

// #include <QWidget>
#include <QDockWidget>

class CountdownDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit CountdownDockWidget(QWidget *parent);
};

#endif // COUNTDOWNWIDGET_H