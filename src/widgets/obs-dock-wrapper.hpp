#ifndef OBSDOCK_H
#define OBSDOCK_H

#include <QWidget>
#include <obs.h>

class OBSDock : public QWidget {
	Q_OBJECT

public:
	explicit inline OBSDock(const QString &title, QWidget *parent = nullptr) : QWidget(parent)
	{
		UNUSED_PARAMETER(title);
	}

	explicit inline OBSDock(QWidget *parent = nullptr) : QWidget(parent) {}

	virtual ~OBSDock() = default;
};

#endif // OBSDOCK_H
