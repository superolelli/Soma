#include "XMLUtilities.hpp"


namespace pugi {


    void loadBuffFromXML(const xml_node & buffNode, Buff & buff)
    {
		if(buffNode.attribute("duration"))
			buff.duration = buffNode.attribute("duration").as_int();

		if (buffNode.attribute("isPositive"))
			buff.isPositive = buffNode.attribute("isPositive").as_bool();

		if (buffNode.attribute("onSelf"))
			buff.onSelf = buffNode.attribute("onSelf").as_bool();
        
        if (buffNode.child("armour"))
            buff.attributes.armour = buffNode.child("armour").text().as_int();
        
        if (buffNode.child("damage"))
            buff.attributes.damage = buffNode.child("damage").text().as_int();
        
        if (buffNode.child("dexterity"))
            buff.attributes.dexterity = buffNode.child("dexterity").text().as_int();
        
        if (buffNode.child("initiative"))
            buff.attributes.initiative = buffNode.child("initiative").text().as_int();
        
        if (buffNode.child("maxHealth"))
            buff.attributes.maxHealth = buffNode.child("maxHealth").text().as_int();
        
        if (buffNode.child("currentHealth"))
            buff.attributes.currentHealth = buffNode.child("currentHealth").text().as_int(); 
    }



    void loadAbilityEffectFromXML(const xml_node & effectNode, AbilityEffect & effect)
    {
		if (effectNode.child("damageFactor"))
			effect.damageFactor = effectNode.child("damageFactor").text().as_float();

		if (effectNode.child("heal"))
			effect.heal = effectNode.child("heal").text().as_int();

		if (effectNode.child("healSelf"))
			effect.healSelf = effectNode.child("healSelf").text().as_int();

		if (effectNode.child("confusion"))
		{
			effect.confusion = effectNode.child("confusion").text().as_int();
			if (effectNode.child("confusiont").attribute("probability"))
				effect.confusionProbability = effectNode.child("confusion").attribute("probability").as_float();
		}

		if (effectNode.child("mark"))
			effect.mark = effectNode.child("mark").text().as_int();

		if (effectNode.child("putToSleep"))
			effect.putToSleepProbability = effectNode.child("putToSleepProbability").text().as_float();

		if (effectNode.child("removeBuffs"))
			effect.removeBuffs = effectNode.child("removeBuffs").text().as_bool();

		if (effectNode.child("removeDebuffs"))
			effect.removeDebuffs = effectNode.child("removeDebuffs").text().as_bool();

		if (effectNode.child("buff"))
			loadBuffFromXML(effectNode.child("buff"), effect.buff);
    }




	void loadAbilityFromXML(const xml_node & abilityNode, PlayerAbility & ability)
	{
		if (abilityNode.attribute("name"))
			ability.name = abilityNode.attribute("name").as_string();

		if (abilityNode.child("animation"))
			ability.animation = abilityNode.child_value("animation");

		for (auto &effectAnimation : abilityNode.children("effectAnimation"))
		{
			if (!effectAnimation.attribute("target"))
			{
				ability.effectAnimationFriendly = effectAnimation.text().as_string();
				ability.effectAnimationHostile = effectAnimation.text().as_string();
			}
			else if (std::strcmp(effectAnimation.attribute("target").as_string(), "friend") == 0)
				ability.effectAnimationFriendly = effectAnimation.text().as_string();
			else
				ability.effectAnimationHostile = effectAnimation.text().as_string();
		}

		if (abilityNode.child("howMany"))
			ability.possibleAims.howMany = abilityNode.child("howMany").text().as_int();

		for (xml_node &pos : abilityNode.child("positions").children())
		{
			ability.possibleAims.position[pos.attribute("id").as_int()] = pos.text().as_bool();
		}

		for (auto &effect : abilityNode.children("effect"))
		{
			if (!effect.attribute("target"))
			{
				loadAbilityEffectFromXML(effect, ability.effectFriendly);
				ability.effectHostile = ability.effectFriendly;
			}
			else if (std::strcmp(effect.attribute("target").as_string(), "friend") == 0)
				loadAbilityEffectFromXML(effect, ability.effectFriendly);
			else
				loadAbilityEffectFromXML(effect, ability.effectHostile);
			
		}
	}


}