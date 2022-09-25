#ifndef COUNTDOWNWIDGET_H
#define COUNTDOWNWIDGET_H

// #include <QWidget>
#include <QDockWidget>
#include <QEvent>

class CountdownDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit CountdownDockWidget(QWidget *parent);
    virtual void changeEvent(QEvent *event);
};

#endif // COUNTDOWNWIDGET_H