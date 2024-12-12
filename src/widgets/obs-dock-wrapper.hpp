#ifndef OBSDOCK_H
#define OBSDOCK_H

#include <QWidget>
#include <obs.h>

class OBSDock : public QWidget {
	Q_OBJECT

public:
	inline OBSDock(const QString &title, QWidget *parent = nullptr)
		: QWidget(parent)
	{
        UNUSED_PARAMETER(title);
	}

	inline OBSDock(QWidget *parent = nullptr) : QWidget(parent) {}
};

#endif // OBSDOCK_H
