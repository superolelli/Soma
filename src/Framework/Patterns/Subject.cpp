#include "Subject.hpp"



//erases the particular observer from the list
void CSubject::RemoveObserver(CObserver *_observer)
{
	std::list<CObserver*>::iterator i;

	for (i = m_pObserverList.begin(); i != m_pObserverList.end(); i++)
	{
		if (*i == _observer)
		{
			i = m_pObserverList.erase(i);
			return;
		}
	}
}




void CSubject::Notify(ObserverNotification const &_notification)
{
	//notifies the action to each observer
	for (auto a : m_pObserverList)
		a->OnNotify(_notification);
}