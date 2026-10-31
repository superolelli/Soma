cmake_minimum_required(VERSION 3.28)

project(Soma VERSION 0.2.0 LANGUAGES CXX)

if(NOT WIN32 OR NOT MSVC)
    message(FATAL_ERROR "Soma supports Windows with MSVC only.")
endif()

include(CTest)

option(SOMA_USE_BUNDLED_DEPENDENCIES "Use dependencies from the repository" ON)
if(SOMA_USE_BUNDLED_DEPENDENCIES)
    list(PREPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/SFML-3.1.0")
endif()

find_package(SFML 3.0 REQUIRED COMPONENTS Graphics Audio)

if(NOT FFMPEG_ROOT)
    if(SOMA_USE_BUNDLED_DEPENDENCIES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/ffmpeg/include")
        set(FFMPEG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/ffmpeg" CACHE PATH "Location of FFmpeg" FORCE)
    elseif(EXISTS "C:/ffmpeg/include")
        set(FFMPEG_ROOT "C:/ffmpeg" CACHE PATH "Location of FFmpeg" FORCE)
    endif()
endif()
set(SFEMOVIE_BUILD_STATIC FALSE CACHE BOOL "Build sfeMovie as a shared library" FORCE)
set(SFEMOVIE_BUILD_UNIT_TESTS FALSE CACHE BOOL "Disable sfeMovie's own test suite" FORCE)
add_subdirectory(Dependencies/sfe-movie EXCLUDE_FROM_ALL)
set_target_properties(sfeMovie PROPERTIES FOLDER "ThirdParty")
target_include_directories(sfeMovie PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/sfe-movie/include")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_compile_options(/MP /W4 /permissive- /utf-8 /Zc:__cplusplus)
option(SOMA_WARNINGS_AS_ERRORS "Treat first-party warnings as errors" ON)
if(SOMA_WARNINGS_AS_ERRORS)
    add_compile_options(/WX)
endif()

add_library(soma_gameplay STATIC src/gameplay/random/SeededRandom.cpp)
add_library(Soma::Gameplay ALIAS soma_gameplay)
target_include_directories(soma_gameplay PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library(soma_simulation STATIC
    src/simulation/battle/BattleSimulation.cpp
    src/simulation/battle/BattleEffects.cpp
    src/simulation/battle/BattleDecayEffects.cpp
    src/simulation/battle/BattleStatusEffects.cpp
    src/simulation/battle/BattleTargeting.cpp
    src/simulation/battle/BattleTurnFlow.cpp
    src/simulation/battle/AbilityTargets.cpp
    src/simulation/battle/BattleRules.cpp
    src/simulation/battle/BattleStandingRules.cpp
    src/simulation/battle/EnemyPolicy.cpp
    src/simulation/exploration/LevelGenerator.cpp
    src/simulation/exploration/LevelRun.cpp
    src/simulation/rewards/LootGenerator.cpp
    src/simulation/rewards/LevelRewardCalculator.cpp
    src/simulation/rules/ModifierPipeline.cpp
    src/simulation/rules/RuleEventQueue.cpp
)
add_library(Soma::Simulation ALIAS soma_simulation)
target_include_directories(soma_simulation PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(soma_simulation PUBLIC Soma::Gameplay)

add_library(soma_content STATIC
    src/content/battle/BattleContentLoader.cpp
    src/content/battle/BattleContentValidation.cpp
    src/content/battle/BattleDefinitionReader.cpp
    src/content/battle/BattlePresentationReader.cpp
    src/content/economy/EconomyContentLoader.cpp
    src/content/economy/EconomyItemContentLoader.cpp
    src/content/economy/EconomySkillContentLoader.cpp
    src/content/economy/EconomyPresentationContentLoader.cpp
    src/content/flow/GameFlowContentLoader.cpp
    src/content/JsonFileReader.cpp
    src/content/rules/RuleContentLoader.cpp
    src/content/world/WorldContentLoader.cpp
    src/content/world/WorldContentValidation.cpp
)
add_library(Soma::Content ALIAS soma_content)
target_include_directories(soma_content PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/spriterplusplus/nlohmann-json")
target_link_libraries(soma_content PUBLIC Soma::Gameplay)

add_library(soma_game STATIC
    src/game/AuthoredLevelLoader.cpp
    src/game/CampaignBattleContentBuilder.cpp
    src/game/CampaignLifecycleService.cpp
    src/game/CampaignProgression.cpp
    src/game/CampaignState.cpp
    src/game/EconomyQueries.cpp
    src/game/EconomyService.cpp
    src/game/EquipmentBonusCalculator.cpp
    src/game/ExplorationOutcomeService.cpp
    src/game/ExplorationConsumableUseService.cpp
    src/game/ShopStockGenerator.cpp
    src/game/RuleBuilderService.cpp
    src/persistence/SaveRepository.cpp
    src/persistence/SaveSlotService.cpp
)
add_library(Soma::Game ALIAS soma_game)
target_include_directories(soma_game PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(soma_game PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/spriterplusplus/nlohmann-json")
target_link_libraries(soma_game PUBLIC Soma::Content Soma::Simulation)

add_library(soma_presentation STATIC
    src/presentation/battle/BattleEventPresentation.cpp
    src/presentation/battle/BattleEventBatcher.cpp
    src/presentation/sfml/app/BattleLaunchOptions.cpp
    src/presentation/battle/BattlePresentation.cpp
    src/presentation/battle/BattlePresentationSequence.cpp
    src/presentation/text/AbilityDescription.cpp
    src/presentation/text/FormattedText.cpp
    src/presentation/text/RuleDescriptionBuilder.cpp
    src/presentation/text/SkillDescription.cpp
)
add_library(Soma::Presentation ALIAS soma_presentation)
target_include_directories(soma_presentation PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(soma_presentation PUBLIC Soma::Gameplay)

add_subdirectory(src/thirdparty/spriterplusplus)
target_compile_options(SpriterPlusPlus PRIVATE /W0 /WX-)
set_target_properties(SpriterPlusPlus PROPERTIES FOLDER "ThirdParty")

add_library(soma_presentation_sfml STATIC
    src/presentation/sfml/common/ActiveRulesPanel.cpp
    src/presentation/sfml/battle/BattleActionPresentation.cpp
    src/presentation/sfml/battle/BattleActionStarter.cpp
    src/presentation/sfml/battle/BattleActionTimeline.cpp
    src/presentation/sfml/battle/BattleCombatantInformation.cpp
    src/presentation/sfml/battle/BattleHud.cpp
    src/presentation/sfml/battle/BattleStatusHud.cpp
    src/presentation/sfml/battle/BattleTooltip.cpp
    src/presentation/sfml/battle/BattleInteractionController.cpp
    src/presentation/sfml/battle/BattleSceneBlur.cpp
    src/presentation/sfml/battle/BattleScreenRenderer.cpp
    src/presentation/sfml/battle/BattleScreenOverlays.cpp
    src/presentation/sfml/battle/BattleScreenActionRendering.cpp
    src/presentation/sfml/battle/BattleSoundPlayer.cpp
    src/presentation/sfml/economy/EconomyTextures.cpp
    src/presentation/sfml/economy/EconomyTooltip.cpp
    src/presentation/sfml/economy/EconomySoundPlayer.cpp
    src/presentation/sfml/economy/EquipmentTransition.cpp
    src/presentation/sfml/economy/EquipmentConnectionRenderer.cpp
    src/presentation/sfml/common/FormattedTextLayout.cpp
    src/presentation/sfml/economy/InventoryPanel.cpp
    src/presentation/sfml/economy/InventoryPanelRendering.cpp
    src/presentation/sfml/app/MainMenuScreen.cpp
    src/presentation/sfml/app/MainRoomScreen.cpp
    src/presentation/sfml/economy/ShopPanel.cpp
    src/presentation/sfml/economy/ShopPanelRendering.cpp
    src/presentation/sfml/economy/ShopPanelSelection.cpp
    src/presentation/sfml/economy/SkillPanel.cpp
    src/presentation/sfml/economy/SkillPanelRendering.cpp
    src/presentation/sfml/common/NinePatch.cpp
    src/presentation/sfml/common/PauseMenu.cpp
    src/presentation/sfml/common/ParticleSystem.cpp
    src/presentation/sfml/common/UiCursor.cpp
    src/presentation/sfml/common/ResourceCounter.cpp
    src/presentation/sfml/economy/RuleBuilderPanel.cpp
    src/presentation/sfml/economy/RuleBuilderPanelLayout.cpp
    src/presentation/sfml/economy/RuleBuilderPanelRendering.cpp
    src/presentation/sfml/common/SpriterAnimationPlayer.cpp
    src/presentation/sfml/app/StartupScreen.cpp
    src/presentation/sfml/common/StatusGlossary.cpp
)
add_library(Soma::PresentationSfml ALIAS soma_presentation_sfml)
target_include_directories(soma_presentation_sfml PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(soma_presentation_sfml
    PUBLIC Soma::Presentation
    PRIVATE SpriterPlusPlus SFML::Graphics SFML::Audio
)

add_executable(Soma
      src/presentation/sfml/app/BattleMain.cpp
      src/presentation/sfml/app/BattleApplication.cpp
      src/presentation/sfml/battle/BattleScreenSession.cpp
      src/presentation/sfml/battle/BattleTransitionPlayer.cpp
      src/presentation/sfml/common/SfeMovieVideoPlayer.cpp
      src/presentation/sfml/app/EconomyApplication.cpp
      src/presentation/sfml/app/ExplorationApplication.cpp
      src/presentation/sfml/app/GameApplication.cpp
      src/presentation/sfml/exploration/ExplorationConsumablePanel.cpp
      src/presentation/sfml/exploration/ExplorationSession.cpp
      src/presentation/sfml/exploration/ExplorationInput.cpp
      src/presentation/sfml/exploration/ExplorationBattle.cpp
      src/presentation/sfml/exploration/ExplorationRendering.cpp
      src/presentation/sfml/exploration/ExplorationMusicPlayer.cpp
      src/presentation/sfml/exploration/ExplorationHud.cpp
      src/presentation/sfml/exploration/ExplorationPartyStatus.cpp
      src/presentation/sfml/exploration/ExplorationSpeechBubbles.cpp
      src/presentation/sfml/exploration/LootDialog.cpp
      src/presentation/sfml/exploration/LevelResultDialog.cpp
  )
target_include_directories(Soma PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(Soma PRIVATE sfeMovie
    Soma::Game
    Soma::PresentationSfml
    SFML::Graphics
    SFML::Audio
)
add_custom_command(TARGET Soma POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:sfeMovie>"
            "$<TARGET_FILE_DIR:Soma>"
)

set(SOMA_FFMPEG_RUNTIME_DIR "" CACHE PATH "Directory containing the FFmpeg runtime DLLs required by sfeMovie")
if(NOT SOMA_FFMPEG_RUNTIME_DIR AND FFMPEG_ROOT AND EXISTS "${FFMPEG_ROOT}/bin")
    set(SOMA_FFMPEG_RUNTIME_DIR "${FFMPEG_ROOT}/bin")
endif()
if(SOMA_FFMPEG_RUNTIME_DIR)
    file(GLOB SOMA_FFMPEG_RUNTIME_DLLS CONFIGURE_DEPENDS "${SOMA_FFMPEG_RUNTIME_DIR}/*.dll")
    if(SOMA_FFMPEG_RUNTIME_DLLS)
        add_custom_command(TARGET Soma POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    ${SOMA_FFMPEG_RUNTIME_DLLS}
                    "$<TARGET_FILE_DIR:Soma>"
            COMMENT "Copying FFmpeg runtime libraries required by sfeMovie"
            COMMAND_EXPAND_LISTS
        )
    endif()
endif()
set_target_properties(Soma PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_custom_command(TARGET Soma POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:SFML::Graphics>"
            "$<TARGET_FILE:SFML::Audio>"
            "$<TARGET_FILE:SFML::System>"
            "$<TARGET_FILE_DIR:Soma>"
    COMMENT "Copying SFML runtime libraries"
)

add_executable(soma_content_validator tools/content_validator/main.cpp)
target_include_directories(soma_content_validator PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(soma_content_validator PRIVATE Soma::Content)

add_custom_target(validate_content
    COMMAND soma_content_validator "${CMAKE_CURRENT_SOURCE_DIR}/Data/content/v1"
    DEPENDS soma_content_validator
    COMMENT "Validating versioned Soma JSON content"
    VERBATIM
)

if(BUILD_TESTING)
    add_executable(soma_foundation_tests tests/foundation/FoundationTests.cpp)
    target_include_directories(soma_foundation_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_foundation_tests PRIVATE Soma::Simulation)
    add_test(NAME soma_foundation COMMAND soma_foundation_tests)

    add_executable(soma_battle_content_tests tests/battle/BattleContentTests.cpp)
    target_include_directories(soma_battle_content_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_battle_content_tests PRIVATE Soma::Content)
    add_test(NAME soma_battle_content
             COMMAND soma_battle_content_tests "${CMAKE_CURRENT_SOURCE_DIR}/Data/content/v1")

    add_executable(soma_content_validation_tests tests/content/ContentValidationTests.cpp)
    target_include_directories(soma_content_validation_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_content_validation_tests PRIVATE Soma::Content)
    add_test(NAME soma_content_validation COMMAND soma_content_validation_tests)

    add_executable(soma_battle_tests
        tests/battle/AdvancedBattleMechanicTests.cpp
        tests/battle/BattleCoreTests.cpp
        tests/battle/BattleStatusEffectTests.cpp
        tests/battle/BattleTests.cpp
    )
    target_include_directories(soma_battle_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_battle_tests PRIVATE Soma::Content Soma::Simulation)
    add_test(NAME soma_battle_core
             COMMAND soma_battle_tests "${CMAKE_CURRENT_SOURCE_DIR}/Data/content/v1" core)
    add_test(NAME soma_battle_status
             COMMAND soma_battle_tests "${CMAKE_CURRENT_SOURCE_DIR}/Data/content/v1" status)
    add_test(NAME soma_battle_advanced
             COMMAND soma_battle_tests "${CMAKE_CURRENT_SOURCE_DIR}/Data/content/v1" advanced)

    add_executable(soma_enemy_policy_tests tests/battle/EnemyPolicyTests.cpp)
    target_include_directories(soma_enemy_policy_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_enemy_policy_tests PRIVATE Soma::Simulation)
    add_test(NAME soma_enemy_policy COMMAND soma_enemy_policy_tests)

    add_executable(soma_presentation_tests tests/presentation/PresentationTests.cpp)
    target_include_directories(soma_presentation_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_presentation_tests PRIVATE Soma::Presentation)
    add_test(NAME soma_presentation COMMAND soma_presentation_tests)

    add_executable(soma_battle_event_batcher_tests tests/presentation/BattleEventBatcherTests.cpp)
    target_include_directories(soma_battle_event_batcher_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_battle_event_batcher_tests PRIVATE Soma::Presentation)
    add_test(NAME soma_battle_event_batcher COMMAND soma_battle_event_batcher_tests)

    add_executable(soma_battle_action_timeline_tests tests/presentation/BattleActionTimelineTests.cpp)
    target_include_directories(soma_battle_action_timeline_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_battle_action_timeline_tests PRIVATE Soma::PresentationSfml Soma::Simulation SFML::Graphics)
    add_test(NAME soma_battle_action_timeline COMMAND soma_battle_action_timeline_tests)

    add_executable(soma_battle_interaction_tests tests/presentation/BattleInteractionControllerTests.cpp)
    target_include_directories(soma_battle_interaction_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_battle_interaction_tests PRIVATE Soma::PresentationSfml Soma::Simulation SFML::Graphics)
    add_test(NAME soma_battle_interaction COMMAND soma_battle_interaction_tests)

    add_executable(soma_economy_tests tests/economy/EconomyTests.cpp)
    target_include_directories(soma_economy_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_economy_tests PRIVATE Soma::Game)
    add_test(NAME soma_economy COMMAND soma_economy_tests)

    add_executable(soma_economy_content_tests tests/economy/EconomyContentTests.cpp)
    target_include_directories(soma_economy_content_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_economy_content_tests PRIVATE Soma::Game)
    add_test(NAME soma_economy_content COMMAND soma_economy_content_tests "${CMAKE_CURRENT_SOURCE_DIR}/Data/content/v1")

    add_executable(soma_world_content_tests tests/world/WorldContentTests.cpp)
    target_include_directories(soma_world_content_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_world_content_tests PRIVATE Soma::Content Soma::Simulation)
    add_test(NAME soma_world_content COMMAND soma_world_content_tests "${CMAKE_CURRENT_SOURCE_DIR}/Data/content/v1")

    add_executable(soma_rule_tests tests/rules/RuleTests.cpp)
    target_include_directories(soma_rule_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_rule_tests PRIVATE Soma::Game Soma::Presentation)
    add_test(NAME soma_rules COMMAND soma_rule_tests "${CMAKE_CURRENT_SOURCE_DIR}/Data/content/v1")

    add_executable(soma_persistence_tests tests/persistence/PersistenceTests.cpp)
    target_include_directories(soma_persistence_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_persistence_tests PRIVATE Soma::Game)
    add_test(NAME soma_persistence
             COMMAND soma_persistence_tests
                     "${CMAKE_CURRENT_SOURCE_DIR}/Data/content/v1"
                     "${CMAKE_CURRENT_BINARY_DIR}/test-saves"
                     "${CMAKE_CURRENT_SOURCE_DIR}/Data/Saves/developer.json")

    add_executable(soma_save_failure_tests tests/persistence/SaveRepositoryFailureTests.cpp)
    target_include_directories(soma_save_failure_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_save_failure_tests PRIVATE Soma::Game)
    add_test(NAME soma_save_failure_preservation
             COMMAND soma_save_failure_tests "${CMAKE_CURRENT_BINARY_DIR}/test-save-failures")

    add_executable(soma_campaign_lifecycle_tests tests/game/CampaignLifecycleTests.cpp)
    target_include_directories(soma_campaign_lifecycle_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_campaign_lifecycle_tests PRIVATE Soma::Game)
    add_test(NAME soma_campaign_lifecycle
             COMMAND soma_campaign_lifecycle_tests
                     "${CMAKE_CURRENT_SOURCE_DIR}/Data/content/v1"
                     "${CMAKE_CURRENT_BINARY_DIR}/campaign-lifecycle-saves")
endif()
