set(TARGET_NAME Engine)


add_library(${TARGET_NAME})

file(GLOB_RECURSE ENGINE_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.hpp *.h)

target_sources(${TARGET_NAME} PRIVATE ${ENGINE_SOURCES})

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} PUBLIC SFML::Graphics SFML::Audio)


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_SOURCES})