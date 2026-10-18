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

if(NOT FFMPEG_ROOT AND EXISTS "C:/ffmpeg")
    set(FFMPEG_ROOT "C:/ffmpeg" CACHE PATH "Location of FFmpeg" FORCE)
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
    src/simulation/battle/AbilityTargets.cpp
    src/simulation/battle/BattleRules.cpp
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
    src/content/BattleContentLoader.cpp
    src/content/EconomyContentLoader.cpp
    src/content/GameFlowContentLoader.cpp
    src/content/RuleContentLoader.cpp
    src/content/WorldContentLoader.cpp
)
add_library(Soma::Content ALIAS soma_content)
target_include_directories(soma_content PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/spriterplusplus/nlohmann-json")
target_link_libraries(soma_content PUBLIC Soma::Gameplay)

add_library(soma_game STATIC
    src/game/AuthoredLevelLoader.cpp
    src/game/CampaignBattleContentBuilder.cpp
    src/game/CampaignProgression.cpp
    src/game/CampaignState.cpp
    src/game/EconomyQueries.cpp
    src/game/EconomyService.cpp
    src/game/EquipmentBonusCalculator.cpp
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
    src/presentation/sfml/BattleLaunchOptions.cpp
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
    src/presentation/sfml/BattleActionPresentation.cpp
    src/presentation/sfml/BattleActionStarter.cpp
    src/presentation/sfml/BattleActionTimeline.cpp
    src/presentation/sfml/BattleHud.cpp
    src/presentation/sfml/BattleInteractionController.cpp
    src/presentation/sfml/BattleSceneBlur.cpp
    src/presentation/sfml/BattleScreenRenderer.cpp
    src/presentation/sfml/BattleSoundPlayer.cpp
    src/presentation/sfml/EconomyTextures.cpp
    src/presentation/sfml/EconomyTooltip.cpp
    src/presentation/sfml/EconomySoundPlayer.cpp
    src/presentation/sfml/EquipmentTransition.cpp
    src/presentation/sfml/EquipmentConnectionRenderer.cpp
    src/presentation/sfml/FormattedTextLayout.cpp
    src/presentation/sfml/InventoryPanel.cpp
    src/presentation/sfml/MainMenuScreen.cpp
    src/presentation/sfml/MainRoomScreen.cpp
    src/presentation/sfml/ShopPanel.cpp
    src/presentation/sfml/SkillPanel.cpp
    src/presentation/sfml/NinePatch.cpp
    src/presentation/sfml/PauseMenu.cpp
    src/presentation/sfml/UiCursor.cpp
    src/presentation/sfml/ResourceCounter.cpp
    src/presentation/sfml/RuleBuilderPanel.cpp
    src/presentation/sfml/SpriterAnimationPlayer.cpp
    src/presentation/sfml/StartupScreen.cpp
    src/presentation/sfml/StatusGlossary.cpp
)
add_library(Soma::PresentationSfml ALIAS soma_presentation_sfml)
target_include_directories(soma_presentation_sfml PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(soma_presentation_sfml
    PUBLIC Soma::Presentation
    PRIVATE SpriterPlusPlus SFML::Graphics SFML::Audio
)

add_executable(Soma
      src/presentation/sfml/BattleMain.cpp
      src/presentation/sfml/BattleApplication.cpp
      src/presentation/sfml/BattleScreenSession.cpp
      src/presentation/sfml/BattleTransitionPlayer.cpp
      src/presentation/sfml/SfeMovieVideoPlayer.cpp
      src/presentation/sfml/EconomyApplication.cpp
      src/presentation/sfml/ExplorationApplication.cpp
      src/presentation/sfml/GameApplication.cpp
      src/presentation/sfml/ExplorationConsumablePanel.cpp
      src/presentation/sfml/ExplorationSession.cpp
      src/presentation/sfml/ExplorationMusicPlayer.cpp
      src/presentation/sfml/ExplorationHud.cpp
      src/presentation/sfml/ExplorationPartyStatus.cpp
      src/presentation/sfml/ExplorationSpeechBubbles.cpp
      src/presentation/sfml/LootDialog.cpp
      src/presentation/sfml/LevelResultDialog.cpp
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
if(NOT SOMA_FFMPEG_RUNTIME_DIR)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/ffmpeg/bin")
        set(SOMA_FFMPEG_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/ffmpeg/bin")
    elseif(EXISTS "C:/ffmpeg/bin")
        set(SOMA_FFMPEG_RUNTIME_DIR "C:/ffmpeg/bin")
    endif()
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

    add_executable(soma_battle_tests tests/battle/BattleTests.cpp)
    target_include_directories(soma_battle_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_battle_tests PRIVATE Soma::Content Soma::Simulation)
    add_test(NAME soma_battle
             COMMAND soma_battle_tests "${CMAKE_CURRENT_SOURCE_DIR}/Data/content/v1")

    add_executable(soma_presentation_tests tests/presentation/PresentationTests.cpp)
    target_include_directories(soma_presentation_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(soma_presentation_tests PRIVATE Soma::Presentation)
    add_test(NAME soma_presentation COMMAND soma_presentation_tests)

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
endif()
