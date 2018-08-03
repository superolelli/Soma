#include "Anna.hpp"



void PlayerAnna::Init(int _id, CGameEngine *_engine)
{
	combatantObject = g_pModels->modelAnna->getNewEntityInstance("anna");

	this->Player::Init(_id, _engine);
}


bool PlayerAnna::DoAbility(int _id, std::vector<Combatant*> &_targets)
{
	if (status.IsConfused())
	{
		if (rand() % 4 == 0)
			status.LooseHealth(1);
	}

	switch (_id) {
	case 0:
		for (Combatant* t : _targets)
			indomitableHappiness(t);
		break;
	case 1:
		for (Combatant* t : _targets)
			wordGush(t);
		break;
	case 2:
		for (Combatant* t : _targets)
			sideOfNoseBlow(t);
		break;
	case 3:
		for (Combatant* t : _targets)
			deathHug(t);
		break;
	}
	return true;
}

void PlayerAnna::indomitableHappiness(Combatant* _target)
{
	if(rand()%3 == 0)
		_target->Status().Confuse(3);

	_target->Status().GainHealth(3);
}

void PlayerAnna::wordGush(Combatant* _target)
{
	if (rand() % 4 == 0)
		_target->Status().PutToSleep();

	_target->Status().LooseHealth(status.GetDamage());
}

void PlayerAnna::sideOfNoseBlow(Combatant* _target)
{
	_target->Status().LooseHealth(status.GetDamage() * 0.4f);
}

void PlayerAnna::deathHug(Combatant* _target)
{
	_target->Status().LooseHealth(status.GetDamage());

	status.GainHealth(1);
}

void PlayerAnna::StartAbilityAnimation(int _ability)
{
	combatantObject->setCurrentAnimation("word_gush");
	combatantObject->setCurrentTime(0);

	lastPosition = combatantObject->getPosition();

	combatantObject->setScale(SpriterEngine::point(0.8, 0.8));

	combatantObject->setPosition(SpriterEngine::point(int(lastPosition.x) - (engine->GetWindow().getView().getCenter().x - engine->GetWindow().getView().getSize().x / 2), 800));
}
