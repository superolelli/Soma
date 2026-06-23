#pragma once


class ObserverNotification {
public:
	virtual ~ObserverNotification() {};
};

//base class for the observer pattern
class CObserver
{
public:

	virtual void OnNotify(ObserverNotification const &_notification) {};
};


