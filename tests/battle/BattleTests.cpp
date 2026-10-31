#include "BattleTestSupport.hpp"

#include <filesystem>
#include <string_view>

int main(int argc, char** argv)
{
    soma::tests::check(argc == 2 || argc == 3);
    const auto contentRoot = std::filesystem::absolute(argv[1]);
    const auto selectedSuite = argc == 3 ? std::string_view{argv[2]} : std::string_view{"all"};
    const auto battle = soma::content::BattleContentLoader{}.load(contentRoot);
    soma::tests::check(battle.valid());
    const auto rules = soma::content::RuleContentLoader{}.load(contentRoot);
    soma::tests::check(rules.valid());

    if (selectedSuite == "all" || selectedSuite == "core") soma::tests::runBattleCoreTests(battle, rules);
    if (selectedSuite == "all" || selectedSuite == "status") soma::tests::runBattleStatusEffectTests(battle, rules);
    if (selectedSuite == "all" || selectedSuite == "advanced") soma::tests::runAdvancedBattleMechanicTests(battle);
    soma::tests::check(selectedSuite == "all" || selectedSuite == "core" || selectedSuite == "status" || selectedSuite == "advanced");
}
