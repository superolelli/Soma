#include "XMLUtilities.hpp"
#include <SFML\System\String.hpp>

namespace pugi {


    void loadBuffFromXML(const xml_node & buffNode, Buff & buff)
    {
		if(buffNode.child("duration"))
			buff.duration = buffNode.child("duration").text().as_int();

		if (buffNode.child("isPositive"))
			buff.isPositive = buffNode.child("isPositive").text().as_bool();

		if (buffNode.child("onSelf"))
			buff.onSelf = buffNode.child("onSelf").text().as_bool();
        
        if (buffNode.child("armour"))
            buff.stats.armour = buffNode.child("armour").text().as_int(); 
        
        if (buffNode.child("damage"))
            buff.stats.damageMin = buffNode.child("damage").text().as_int();

		if (buffNode.child("damage"))
			buff.stats.damageMax = buffNode.child("damage").text().as_int();
        
        if (buffNode.child("initiative"))
            buff.stats.initiative = buffNode.child("initiative").text().as_int();
        
        if (buffNode.child("maxHealth"))
            buff.stats.maxHealth = buffNode.child("maxHealth").text().as_int();
        
        if (buffNode.child("currentHealth"))
            buff.stats.currentHealth = buffNode.child("currentHealth").text().as_int(); 

		if (buffNode.child("criticalHit"))
			buff.stats.criticalHit = buffNode.child("criticalHit").text().as_int();

		if (buffNode.child("precision"))
			buff.stats.precision = buffNode.child("precision").text().as_int();

		if (buffNode.child("dodge"))
			buff.stats.dodge = buffNode.child("dodge").text().as_int();

		if (buffNode.child("dexterity"))
			buff.stats.attributes.dexterity = buffNode.child("dexterity").text().as_int();

		if (buffNode.child("strength"))
			buff.stats.attributes.strength = buffNode.child("strength").text().as_int();

		if (buffNode.child("constitution"))
			buff.stats.attributes.constitution = buffNode.child("constitution").text().as_int();

		if (buffNode.child("speed"))
			buff.stats.attributes.speed = buffNode.child("speed").text().as_int();

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
			if(effectNode.child("confusion").child("rounds"))
				effect.confusion = effectNode.child("confusion").child("rounds").text().as_int();

			if (effectNode.child("confusion").child("probability"))
				effect.confusionProbability = effectNode.child("confusion").child("probability").text().as_float();
		}

		if (effectNode.child("mark"))
			effect.mark = effectNode.child("mark").text().as_int();

		if (effectNode.child("putToSleepProbability"))
			effect.putToSleepProbability = effectNode.child("putToSleepProbability").text().as_float();

		if (effectNode.child("removeBuffs"))
			effect.removeBuffs = effectNode.child("removeBuffs").text().as_bool();

		if (effectNode.child("removeDebuffs"))
			effect.removeDebuffs = effectNode.child("removeDebuffs").text().as_bool();

		if (effectNode.child("buff"))
			loadBuffFromXML(effectNode.child("buff"), effect.buff);
    }




	void loadAbilityFromXML(const xml_node & abilityNode, Ability & ability)
	{
		if (abilityNode.attribute("name"))
		{
			std::string str(abilityNode.attribute("name").as_string());
			ability.name = sf::String::fromUtf8(str.begin(), str.end());
		}

		if (abilityNode.child("description"))
		{
			std::string str(abilityNode.child("description").text().as_string());
			ability.description = sf::String::fromUtf8(str.begin(), str.end());
		}

		if (abilityNode.child("animation"))
			ability.animation = abilityNode.child("animation").text().as_string();

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

		if (abilityNode.child("attackAll"))
			ability.possibleAims.attackAll = abilityNode.child("attackAll").text().as_bool();

		for (xml_node &pos : abilityNode.child("positions").children())
		{
			ability.possibleAims.position[pos.attribute("id").as_int()] = pos.text().as_bool();
		}

		if (abilityNode.child("precision"))
			ability.precisionModificator = abilityNode.child("precision").text().as_int();

		if (abilityNode.child("criticalHit"))
			ability.criticalHitModificator = abilityNode.child("criticalHit").text().as_int();

		if(abilityNode.child("lessTargetsMoreDamage"))
			ability.lessTargetsMoreDamage = abilityNode.child("lessTargetsMoreDamage").text().as_float();

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

	void loadAttributesFromXML(const xml_node & attributeNode, CombatantStats & stats)
	{
		stats.armour = attributeNode.attribute("armour").as_int();
		stats.maxHealth = attributeNode.attribute("health").as_int();
		stats.damageMin = attributeNode.attribute("damageMin").as_int();
		stats.damageMax = attributeNode.attribute("damageMax").as_int();
		stats.initiative = attributeNode.attribute("initiative").as_int();
		stats.criticalHit = attributeNode.attribute("criticalHit").as_int();
		stats.dodge = attributeNode.attribute("dodge").as_int();
		stats.precision = attributeNode.attribute("precision").as_int();
		stats.attributes.dexterity = attributeNode.attribute("dexterity").as_int();
		stats.attributes.speed = attributeNode.attribute("speed").as_int();
		stats.attributes.strength = attributeNode.attribute("strength").as_int();
		stats.attributes.constitution = attributeNode.attribute("constitution").as_int();
		
		stats.currentHealth = stats.maxHealth;
	}

	void loadLevelSpecsFromXML(const xml_node & levelSpecNode, LevelSpecs & specs)
	{
		specs.level = levelSpecNode.attribute("id").as_int();
		specs.battleProbability = levelSpecNode.child("battleProbability").text().as_float();
		specs.numberOfRooms = levelSpecNode.child("numberOfRooms").text().as_int();
		specs.endBackground = backgroundIdentifierMap[levelSpecNode.child("endBackground").text().as_string()];

		specs.reward.dice = levelSpecNode.child("reward").child("dice").text().as_int();
		specs.reward.cards = levelSpecNode.child("reward").child("cards").text().as_int();

		for (xml_node &group : levelSpecNode.child("enemyGroups").children())
		{
			std::array<CombatantID, 4> newGroup;
			loadEnemyGroupFromXML(group, newGroup);
			specs.possibleEnemyGroups.push_back(newGroup);
		}

		loadEnemyGroupFromXML(levelSpecNode.child("bossGroup"), specs.bossGroup);

		for (xml_node &background : levelSpecNode.child("backgrounds"))
		{
			specs.possibleBackgrounds.push_back(backgroundIdentifierMap[background.text().as_string()]);
		}
	}


	void loadEnemyGroupFromXML(const xml_node & enemyGroupNode, std::array<CombatantID, 4> &newGroup)
	{
		newGroup = { CombatantID::Undefined, CombatantID::Undefined, CombatantID::Undefined, CombatantID::Undefined };
		int currentEnemy = 0;
		for (xml_node &enemy : enemyGroupNode)
		{
			std::string str(enemy.text().as_string());
			newGroup[currentEnemy++] = combatantIdentifierMap[sf::String::fromUtf8(str.begin(), str.end())];
		}
	}
}