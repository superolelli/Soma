#include "XMLUtilities.hpp"
#include <SFML\System\String.hpp>
#include "../Resources/BackgroundIDs.hpp"
#include "../Resources/StringManager.hpp"
#include "../Resources/ObjectPropertiesManager.hpp"

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
            buff.stats["armour"] = buffNode.child("armour").text().as_int(); 
        
        if (buffNode.child("damageMin"))
            buff.stats["damageMin"] = buffNode.child("damageMin").text().as_int();

		if (buffNode.child("damageMax"))
			buff.stats["damageMax"] = buffNode.child("damageMax").text().as_int();

		if (buffNode.child("damage"))
		{
			buff.stats["damageMin"] = buffNode.child("damage").text().as_int();
			buff.stats["damageMax"] = buffNode.child("damage").text().as_int();
		}
        
        if (buffNode.child("initiative"))
            buff.stats["initiative"] = buffNode.child("initiative").text().as_int();
        
        if (buffNode.child("maxHealth"))
            buff.stats["maxHealth"] = buffNode.child("maxHealth").text().as_int();
        
        if (buffNode.child("currentHealth"))
            buff.stats["currentHealth"] = buffNode.child("currentHealth").text().as_int(); 

		if (buffNode.child("criticalHit"))
			buff.stats["criticalHit"] = buffNode.child("criticalHit").text().as_int();

		if (buffNode.child("precision"))
			buff.stats["precision"] = buffNode.child("precision").text().as_int();

		if (buffNode.child("dodge"))
			buff.stats["dodge"] = buffNode.child("dodge").text().as_int();

		if (buffNode.child("healing"))
			buff.stats["healing"] = buffNode.child("healing").text().as_int();

		if (buffNode.child("sleepResistance"))
			buff.stats["sleepResistance"] = buffNode.child("sleepResistance").text().as_int();

		if (buffNode.child("confusionResistance"))
			buff.stats["confusionResistance"] = buffNode.child("confusionResistance").text().as_int();

		if (buffNode.child("debuffResistance"))
			buff.stats["debuffResistance"] = buffNode.child("debuffResistance").text().as_int();

		if (buffNode.child("decayResistance"))
			buff.stats["decayResistance"] = buffNode.child("decayResistance").text().as_int();
    }



    void loadAbilityEffectFromXML(const xml_node & effectNode, AbilityEffect & effect)
    {
		if (effectNode.child("criticalHit"))
			effect.criticalHitModificator = effectNode.child("criticalHit").text().as_int();

		if (effectNode.child("lessTargetsMoreDamage"))
			effect.lessTargetsMoreDamage = effectNode.child("lessTargetsMoreDamage").text().as_float();

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

		if (effectNode.child("decay"))
		{
			if (effectNode.child("decay").child("damage"))
				effect.decay = effectNode.child("decay").child("damage").text().as_int();

			if (effectNode.child("decay").child("rounds"))
				effect.decayRounds = effectNode.child("decay").child("rounds").text().as_int();
		}

		if (effectNode.child("bounty"))
			effect.bounty = effectNode.child("bounty").text().as_int();

		if (effectNode.child("dynamite"))
			effect.dynamite = effectNode.child("dynamite").text().as_bool();

		if (effectNode.child("removeBuffs"))
			effect.removeBuffs = effectNode.child("removeBuffs").text().as_bool();

		if (effectNode.child("removeDebuffs"))
			effect.removeDebuffs = effectNode.child("removeDebuffs").text().as_bool();

		if (effectNode.child("stealBuffs"))
			effect.stealBuffs = effectNode.child("stealBuffs").text().as_bool();

		if (effectNode.child("addMiss"))
			effect.addMiss = effectNode.child("addMiss").text().as_int();

		if (effectNode.child("addMissSelf"))
			effect.addMissSelf = effectNode.child("addMissSelf").text().as_int();

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

		if (abilityNode.child("attackAllPlayers"))
			ability.possibleAims.attackAllPlayers = abilityNode.child("attackAllPlayers").text().as_bool();

		if (abilityNode.child("attackAllEnemies"))
			ability.possibleAims.attackAllEnemies = abilityNode.child("attackAllEnemies").text().as_bool();

		if (abilityNode.child("attackSelf"))
			ability.possibleAims.attackSelf = abilityNode.child("attackSelf").text().as_bool();

		for (xml_node &pos : abilityNode.child("positions").children())
		{
			ability.possibleAims.position[pos.attribute("id").as_int()] = pos.text().as_bool();
		}

		if (abilityNode.child("precision"))
			ability.precisionModificator = abilityNode.child("precision").text().as_int();

		if (abilityNode.child("fatigue"))
			ability.fatigue = abilityNode.child("fatigue").text().as_int();

		for (auto &effect : abilityNode.children("effect"))
		{
			if (!effect.attribute("target"))
			{
				loadAbilityEffectFromXML(effect, ability.effectFriendly);
				ability.effectHostile = ability.effectFriendly;
			}
			else {
				ability.canTargetEnemiesOrFriends = true;

				if (std::strcmp(effect.attribute("target").as_string(), "friend") == 0)
					loadAbilityEffectFromXML(effect, ability.effectFriendly);
				else
					loadAbilityEffectFromXML(effect, ability.effectHostile);
			}
		}
	}

	void loadAttributesFromXML(const xml_node & attributeNode, CombatantAttributes & stats)
	{
		std::vector<std::string> toLoad = {"armour", "maxHealth", "damageMin", "damageMax", "initiative", "criticalHit", "dodge", "precision",
										   "healing", "sleepResistance", "confusionResistance", "debuffResistance", "decayResistance"};

		for (auto& s : toLoad)
		{
			if (attributeNode.attribute(s.c_str()))
				stats[s] = attributeNode.attribute(s.c_str()).as_int();
		}
		
		stats["currentHealth"] = stats["maxHealth"];
	}


	void loadPossibleLootFromXML(const xml_node& enemyNode, std::vector<ItemID> &loot)
	{
		for (auto &c : enemyNode.child("Loot").children())
		{
			std::string name = c.text().as_string();
			loot.push_back(itemIdentifierMap[name]);
		}
	}


	void loadLevelSpecsFromXML(const xml_node & levelSpecNode, LevelSpecs & specs)
	{
		specs.level = levelSpecNode.attribute("id").as_int();
		specs.battleProbability = levelSpecNode.child("battleProbability").text().as_float();
		specs.lootableProbability = levelSpecNode.child("lootableProbability").text().as_float();
		specs.numberOfRooms = levelSpecNode.child("numberOfRooms").text().as_int();
		specs.endBackground = backgroundIdentifierMap[specs.levelType][levelSpecNode.child("endBackground").text().as_string()];

		for (xml_node &group : levelSpecNode.child("enemyGroups").children())
		{
			std::array<CombatantID, 4> newGroup;
			loadEnemyGroupFromXML(group, newGroup);
			specs.possibleEnemyGroups.push_back(newGroup);
		}

		loadEnemyGroupFromXML(levelSpecNode.child("bossGroup"), specs.bossGroup);

		for (xml_node &background : levelSpecNode.child("backgrounds"))
		{
			specs.possibleBackgrounds.push_back(backgroundIdentifierMap[specs.levelType][background.text().as_string()]);
		}
	}


	void loadEnemyGroupFromXML(const xml_node & enemyGroupNode, std::array<CombatantID, 4> &newGroup)
	{
		newGroup = { CombatantID::Undefined, CombatantID::Undefined, CombatantID::Undefined, CombatantID::Undefined };
		int currentEnemy = 0;
		for (xml_node &enemy : enemyGroupNode)
		{
			std::string str(enemy.text().as_string());
			auto it = std::find(g_pObjectProperties->combatantNames.begin(), g_pObjectProperties->combatantNames.end(), sf::String::fromUtf8(str.begin(), str.end()));
			auto index = std::distance(g_pObjectProperties->combatantNames.begin(), it);
			newGroup[currentEnemy++] = CombatantID(index);
		}
	}
}