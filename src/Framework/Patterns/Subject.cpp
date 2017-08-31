#include "Subject.hpp"



//erases the particular observer from the list
void CSubject::RemoveObserver(std::shared_ptr<CObserver> _observer)
{
	std::list<std::weak_ptr<CObserver>>::iterator i;

	for (i = m_pObserverList.begin(); i != m_pObserverList.end(); i++)
	{
		auto var = i->lock();
		if (var == _observer)
		{
			i = m_pObserverList.erase(i);
			return;
		}
	}
}




void CSubject::Notify(int _subject, int _action, int _object)
{
	//notifies the action to each observer
	for (auto a : m_pObserverList)
		a.lock()->OnNotify(_subject, _action, _object);
}