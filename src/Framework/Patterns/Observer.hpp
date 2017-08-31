#pragma once


//base class for the observer pattern
class CObserver
{
public:

	virtual void OnNotify(int _subject, int _action, int _object) = 0;
};


