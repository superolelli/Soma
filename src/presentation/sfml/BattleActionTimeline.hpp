#pragma once

#include "presentation/sfml/BattleActionPresentation.hpp"
#include "presentation/sfml/BattleSoundPlayer.hpp"

namespace soma::presentation
{
    /// Advances the staged audiovisual playback of one simulated battle action without changing simulation state.
    class BattleActionTimeline
    {
      public:
        explicit BattleActionTimeline(BattleSoundPlayer& sounds);

        void update(float elapsedSeconds,
                    std::optional<PresentedAction>& action,
                    CombatantViews& views,
                    IAnimationPlayer& effect,
                    const simulation::BattleSimulation& battle,
                    std::vector<FloatingText>& floatingTexts) const;

      private:
        void updateAnnouncement(PresentedAction& action,
                                CombatantViews& views,
                                IAnimationPlayer& effect,
                                const simulation::BattleSimulation& battle) const;
        void updateExecution(std::optional<PresentedAction>& action,
                             CombatantViews& views,
                             IAnimationPlayer& effect,
                             const simulation::BattleSimulation& battle,
                             std::vector<FloatingText>& floatingTexts) const;
        void updateItemInterruption(PresentedAction& action,
                                    CombatantViews& views,
                                    const simulation::BattleSimulation& battle,
                                    std::vector<FloatingText>& floatingTexts) const;
        void updateReaction(PresentedAction& action,
                            CombatantViews& views,
                            const simulation::BattleSimulation& battle,
                            std::vector<FloatingText>& floatingTexts) const;
        void updateResult(std::optional<PresentedAction>& action,
                          CombatantViews& views,
                          const simulation::BattleSimulation& battle,
                          std::vector<FloatingText>& floatingTexts) const;
        void updateDefeat(std::optional<PresentedAction>& action,
                          CombatantViews& views,
                          const simulation::BattleSimulation& battle,
                          std::vector<FloatingText>& floatingTexts) const;
        void presentResults(PresentedAction& action,
                            CombatantViews& views,
                            const simulation::BattleSimulation& battle,
                            std::vector<FloatingText>& floatingTexts) const;
        static void beginDefeatOrFinish(std::optional<PresentedAction>& action, CombatantViews& views);
        void beginDeferredBatch(std::optional<PresentedAction>& action,
                                CombatantViews& views,
                                const simulation::BattleSimulation& battle,
                                std::vector<FloatingText>& floatingTexts) const;

        BattleSoundPlayer& sounds_;
    };
} // namespace soma::presentation
