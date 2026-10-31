#include "content/JsonFileReader.hpp"

#include <fstream>
#include <stdexcept>

namespace soma::content::detail
{
    namespace
    {
        int integer(const Json& value, const char* name, int fallback = 0)
        {
            return value.contains(name) ? value.at(name).get<int>() : fallback;
        }
    } // namespace

    Json readJsonFile(const std::filesystem::path& path)
    {
        std::ifstream stream(path);
        if (!stream)
        {
            throw std::runtime_error("Could not open " + path.string());
        }
        try
        {
            return Json::parse(stream);
        }
        catch (const Json::parse_error& error)
        {
            throw std::runtime_error("Could not parse " + path.string() + ": " + error.what());
        }
    }

    AttributeDelta readAttributeDelta(const Json& value)
    {
        AttributeDelta result;
        result.armour = integer(value, "armour");
        const auto damage = integer(value, "damage");
        result.damageMinimum = integer(value, "damageMin", damage);
        result.damageMaximum = integer(value, "damageMax", damage);
        result.initiative = integer(value, "initiative");
        result.maximumHealth = integer(value, "maxHealth");
        result.criticalHitChance = integer(value, "criticalHit");
        result.precision = integer(value, "precision");
        result.dodge = integer(value, "dodge");
        result.healing = integer(value, "healing");
        result.sleepResistance = integer(value, "sleepResistance");
        result.confusionResistance = integer(value, "confusionResistance");
        result.debuffResistance = integer(value, "debuffResistance");
        result.decayResistance = integer(value, "decayResistance");
        return result;
    }
} // namespace soma::content::detail
