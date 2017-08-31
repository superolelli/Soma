#pragma once


#include "Observer.hpp"
#include <list>
#include <memory>

//base class for subjects (observer pattern)
class CSubject
{
public:

	void AddObserver(std::shared_ptr<CObserver> _observer){ m_pObserverList.push_back(_observer); }
	void RemoveObserver(std::shared_ptr<CObserver> _observer);

	//notifies an action to all the observers
	void Notify(int _subject, int _action, int _object);

protected:

	//a list with all the observers
	std::list<std::weak_ptr<CObserver>> m_pObserverList;

};


