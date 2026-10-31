#include "content/rules/RuleContentLoader.hpp"
#include "game/RuleBuilderService.hpp"
#include "gameplay/rules/ComposedRule.hpp"
#include "presentation/text/RuleDescriptionBuilder.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace
{
    void check(bool condition)
    {
        if (!condition)
        {
            std::cerr << "Rule test failed\n";
            std::exit(1);
        }
    }
} // namespace

int main(int argc, char** argv)
{
    check(argc == 2);
    const auto loaded = soma::content::RuleContentLoader{}.load(std::filesystem::path{argv[1]});
    check(loaded.valid());
    check(loaded.content.fragments.size() == 71U);
    const auto countType = [&loaded](soma::content::RuleFragmentType type) {
        return std::ranges::count_if(loaded.content.fragments, [type](const auto& entry) {
            return entry.second.type == type;
        });
    };
    check(countType(soma::content::RuleFragmentType::trigger) == 24);
    check(countType(soma::content::RuleFragmentType::subject) == 16);
    check(countType(soma::content::RuleFragmentType::effect) == 20);
    check(countType(soma::content::RuleFragmentType::modifier) == 11);

    soma::game::CampaignState state;
    soma::game::RuleBuilderService service{state, loaded.content};
    const auto subject = soma::gameplay::RuleFragmentId{"rule.subject.this_combatant"};
    const auto trigger = soma::gameplay::RuleFragmentId{"rule.trigger.health_lost"};
    const auto effect = soma::gameplay::RuleFragmentId{"rule.effect.damage_one"};
    const auto modifier = soma::gameplay::RuleFragmentId{"rule.modifier.armour_up"};
    const auto otherSubject = soma::gameplay::RuleFragmentId{"rule.subject.all_friends"};
    check(service.grant(subject).hasValue());
    check(service.grant(effect).hasValue());
    check(service.grant(modifier).hasValue());
    check(service.typeDiscovered(soma::content::RuleFragmentType::subject));
    check(!service.typeDiscovered(soma::content::RuleFragmentType::trigger));
    check(service.purchase(otherSubject).hasValue());
    check(state.ruleFragmentCount(otherSubject) == 1);
    check(service.grant(trigger).hasValue());
    check(service.assign(0, subject).hasValue());
    check(service.mode(0) == soma::content::RuleMode::triggered);
    check(service.assign(0, trigger).hasValue());
    check(service.assign(0, effect).hasValue());
    check(soma::presentation::buildGermanRuleDescription(state.rules().at(0), service.mode(0), loaded.content) ==
          "Wenn ein Kämpfer Leben verliert, verliert dieser Kämpfer 1 Leben.");
    check(soma::presentation::completedRuleMode(state.rules().at(0)) == soma::content::RuleMode::triggered);
    const auto activeDescriptions = soma::presentation::buildGermanActiveRuleDescriptions(state.rules(), loaded.content);
    check(activeDescriptions.size() == 1U);
    check(activeDescriptions.front() == "Wenn ein Kämpfer Leben verliert, verliert dieser Kämpfer 1 Leben.");
    check(!loaded.content.activeRulesToggle.empty());
    check(loaded.content.activeRulesPresentation.textWidth > 0);
    check(loaded.content.activeRulesPresentation.togglePosition.y < 52);

    soma::gameplay::ComposedRule pluralRule;
    pluralRule.trigger = trigger;
    pluralRule.subject = otherSubject;
    pluralRule.effect = effect;
    check(soma::presentation::buildGermanRuleDescription(pluralRule, soma::content::RuleMode::triggered, loaded.content) ==
          "Wenn ein Kämpfer Leben verliert, verlieren alle Freunde jeweils 1 Leben.");
    soma::gameplay::ComposedRule decayRageRule;
    decayRageRule.trigger = soma::gameplay::RuleFragmentId{"rule.trigger.decay_damage_received"};
    decayRageRule.subject = subject;
    decayRageRule.effect = soma::gameplay::RuleFragmentId{"rule.effect.rage_two"};
    check(soma::presentation::buildGermanRuleDescription(decayRageRule, soma::content::RuleMode::triggered, loaded.content) ==
          "Wenn ein Kämpfer Schaden durch Verfall erhält, verfällt dieser Kämpfer für 2 Runden in Raserei.");

    check(!service.assign(0, modifier).hasValue());
    check(state.ruleFragmentCount(modifier) == 1);
    check(state.ruleSlotsUnlocked() == 1);
    check(service.unlockNextSlot().hasValue());
    check(state.ruleSlotsUnlocked() == 2);
    check(state.currency(soma::gameplay::Currency::cards) == 750);
    check(service.assign(1, otherSubject).hasValue());
    check(!service.assign(1, modifier).hasValue());
    const auto absoluteSubject = soma::gameplay::RuleFragmentId{"rule.subject.all_full_health"};
    check(service.grant(absoluteSubject).hasValue());
    check(service.assign(1, absoluteSubject).hasValue());
    check(service.assign(1, modifier).hasValue());
    check(soma::presentation::completedRuleMode(state.rules().at(1)) == soma::content::RuleMode::standing);
    check(soma::presentation::buildGermanActiveRuleDescriptions(state.rules(), loaded.content).size() == 2U);
}
