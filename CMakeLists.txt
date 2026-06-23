cmake_minimum_required(VERSION 3.24)

if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

project(Soma VERSION 0.1.0 LANGUAGES CXX)

option(SOMA_USE_BUNDLED_DEPENDENCIES "Prefer dependencies from the Dependencies directory" ON)
set(SOMA_FFMPEG_RUNTIME_DIR "" CACHE PATH "Directory containing FFmpeg runtime DLLs required by sfeMovie")

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" config_upper)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${CMAKE_BINARY_DIR}/bin")
endforeach()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(SOMA_USE_BUNDLED_DEPENDENCIES)
    list(PREPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/SFML-3.1.0")
    set(SFEMOVIE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/sfe-movie/install")
    set(GLEW_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/glew-2.3.1")
endif()

find_package(SFML 3.0 REQUIRED COMPONENTS Graphics Audio)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

if(SOMA_USE_BUNDLED_DEPENDENCIES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/sfe-movie/src/Movie.cpp")
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/sfe-movie/cmake/Modules")
    if(NOT FFMPEG_ROOT)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/ffmpeg/include/libavformat/avformat.h")
            set(FFMPEG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/ffmpeg" CACHE PATH "Location of FFmpeg" FORCE)
        elseif(EXISTS "C:/ffmpeg/include/libavformat/avformat.h")
            set(FFMPEG_ROOT "C:/ffmpeg" CACHE PATH "Location of FFmpeg" FORCE)
        endif()
    endif()
    # The bundled find module exports one value with PARENT_SCOPE, so invoke it
    # from a function to give it a valid parent directory scope.
    function(soma_find_ffmpeg)
        find_package(FFMPEG REQUIRED)
    endfunction()
    soma_find_ffmpeg()
    if(NOT FFMPEG_FOUND STREQUAL "YES")
        message(FATAL_ERROR "FFmpeg development files were not found. Set FFMPEG_ROOT to the FFmpeg prefix.")
    endif()

    file(GLOB SFEMOVIE_SOURCES CONFIGURE_DEPENDS
        "Dependencies/sfe-movie/src/*.cpp"
        "Dependencies/sfe-movie/src/*.hpp"
        "Dependencies/sfe-movie/src/DebugTools/*.cpp"
        "Dependencies/sfe-movie/src/DebugTools/*.hpp"
        "Dependencies/sfe-movie/include/sfeMovie/*.hpp"
    )

    # jowie94/sfeMovie currently configures swresample through individual
    # AVOptions. FFmpeg 8 accepts an unspecified AAC layout during probing but
    # rejects it when initializing swresample. Generate a patched translation
    # unit that supplies a concrete default layout and uses swr_alloc_set_opts2.
    set(SFEMOVIE_AUDIO_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/sfe-movie/src/AudioStream.cpp")
    file(READ "${SFEMOVIE_AUDIO_SOURCE}" SFEMOVIE_AUDIO_CONTENT)
    if(NOT SFEMOVIE_AUDIO_CONTENT MATCHES "swr_alloc_set_opts2")
        string(REPLACE [=[
        /* create resampler context */
        m_swrCtx = swr_alloc();
        CHECK(m_swrCtx, "AudioStream::initResampler() - out of memory");
        
        // Some media files don't define the channel layout, in this case take a default one
        // according to the channels' count
        if (m_codecCtx->ch_layout.nb_channels == 0)
        {
            av_channel_layout_default(&m_codecCtx->ch_layout, m_codecCtx->ch_layout.nb_channels);
        }
        
        /* set options */
        av_opt_set_chlayout   (m_swrCtx, "in_channel_layout", &m_codecCtx->ch_layout, 0);
        av_opt_set_int        (m_swrCtx, "in_sample_rate",     m_codecCtx->sample_rate,    0);
        av_opt_set_sample_fmt (m_swrCtx, "in_sample_fmt",      m_codecCtx->sample_fmt,     0);
        
        AVChannelLayout stereoLayout = AV_CHANNEL_LAYOUT_STEREO;
        av_opt_set_chlayout   (m_swrCtx, "out_channel_layout", &stereoLayout,             0);
        av_opt_set_int        (m_swrCtx, "out_sample_rate",    m_codecCtx->sample_rate,    0);
        av_opt_set_sample_fmt (m_swrCtx, "out_sample_fmt",     AV_SAMPLE_FMT_S16,               0);
        ]=] [=[
        // Derive a concrete layout when FFmpeg reports only a channel count.
        AVChannelLayout inputLayout{};
        if (av_channel_layout_check(&m_codecCtx->ch_layout) &&
            m_codecCtx->ch_layout.order != AV_CHANNEL_ORDER_UNSPEC)
        {
            CHECK(av_channel_layout_copy(&inputLayout, &m_codecCtx->ch_layout) >= 0,
                  "AudioStream::initResampler() - channel layout copy error");
        }
        else
        {
            av_channel_layout_default(&inputLayout,
                                      m_codecCtx->ch_layout.nb_channels > 0
                                          ? m_codecCtx->ch_layout.nb_channels
                                          : 2);
        }

        AVChannelLayout stereoLayout = AV_CHANNEL_LAYOUT_STEREO;
        err = swr_alloc_set_opts2(&m_swrCtx,
                                  &stereoLayout,
                                  AV_SAMPLE_FMT_S16,
                                  m_codecCtx->sample_rate,
                                  &inputLayout,
                                  m_codecCtx->sample_fmt,
                                  m_codecCtx->sample_rate,
                                  0,
                                  nullptr);
        av_channel_layout_uninit(&inputLayout);
        CHECK(err >= 0 && m_swrCtx, "AudioStream::initResampler() - resampler allocation error");
        ]=] SFEMOVIE_AUDIO_CONTENT "${SFEMOVIE_AUDIO_CONTENT}")
    endif()
    if(NOT SFEMOVIE_AUDIO_CONTENT MATCHES "swr_alloc_set_opts2")
        message(FATAL_ERROR "Could not apply the bundled sfeMovie FFmpeg channel-layout compatibility patch")
    endif()
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated/sfeMovie")
    set(SFEMOVIE_PATCHED_AUDIO_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/generated/sfeMovie/AudioStream.cpp")
    file(WRITE "${SFEMOVIE_PATCHED_AUDIO_SOURCE}" "${SFEMOVIE_AUDIO_CONTENT}")
    list(REMOVE_ITEM SFEMOVIE_SOURCES "${SFEMOVIE_AUDIO_SOURCE}")
    list(APPEND SFEMOVIE_SOURCES "${SFEMOVIE_PATCHED_AUDIO_SOURCE}")

    add_library(sfeMovie SHARED ${SFEMOVIE_SOURCES})
    add_library(sfeMovie::sfeMovie ALIAS sfeMovie)
    target_compile_features(sfeMovie PRIVATE cxx_std_20)
    target_compile_definitions(sfeMovie PRIVATE
        SFE_EXPORTS
        __STDC_CONSTANT_MACROS
        __STDC_LIMIT_MACROS
    )
    target_include_directories(sfeMovie
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/sfe-movie/include"
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/sfe-movie/src"
            ${FFMPEG_INCLUDE_DIRS}
    )
    target_link_directories(sfeMovie PRIVATE ${FFMPEG_LIBRARY_DIRS})
    target_link_libraries(sfeMovie PRIVATE
        SFML::Graphics
        SFML::Audio
        ${FFMPEG_LIBRARIES}
    )
    set_target_properties(sfeMovie PROPERTIES FOLDER "ThirdParty")
else()
    find_package(sfeMovie REQUIRED)
endif()

file(GLOB_RECURSE SPRITER_SOURCES CONFIGURE_DEPENDS
    "third_party/SpriterPlusPlus/*.cpp"
    "third_party/SpriterPlusPlus/*.h"
    "third_party/SpriterPlusPlus/*.hpp"
)

add_library(spriterplusplus STATIC ${SPRITER_SOURCES})
add_library(Soma::SpriterPlusPlus ALIAS spriterplusplus)
target_compile_features(spriterplusplus PUBLIC cxx_std_20)
target_include_directories(spriterplusplus PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SpriterPlusPlus")
target_link_libraries(spriterplusplus PUBLIC SFML::Graphics SFML::Audio)
set_target_properties(spriterplusplus PROPERTIES FOLDER "ThirdParty")

file(GLOB_RECURSE SOMA_ENGINE_SOURCES CONFIGURE_DEPENDS
    "src/engine/*.cpp"
    "src/engine/*.hpp"
    "src/engine/*.h"
)

add_library(soma_engine STATIC ${SOMA_ENGINE_SOURCES})
add_library(Soma::Engine ALIAS soma_engine)
target_compile_features(soma_engine PUBLIC cxx_std_20)
target_include_directories(soma_engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(soma_engine PUBLIC SFML::Graphics SFML::Audio)
if(MSVC)
    target_compile_options(soma_engine PUBLIC "/FI${CMAKE_CURRENT_SOURCE_DIR}/src/engine/SfmlCompat.hpp")
else()
    target_compile_options(soma_engine PUBLIC "-include${CMAKE_CURRENT_SOURCE_DIR}/src/engine/SfmlCompat.hpp")
endif()

file(GLOB_RECURSE SOMA_GAME_SOURCES CONFIGURE_DEPENDS
    "src/game/*.cpp"
    "src/game/*.hpp"
    "src/game/*.h"
)
list(FILTER SOMA_GAME_SOURCES EXCLUDE REGEX
    "/(ItemRowPanelDragAndDrop|ItemRowPanelSelect|ScrollableItemPanelDragAndDrop|ScrollableItemPanelSelect|PlayerState)\\.cpp$"
)

add_executable(Soma "src/main.cpp" ${SOMA_GAME_SOURCES})
target_compile_features(Soma PRIVATE cxx_std_20)
target_include_directories(Soma PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(Soma PRIVATE
    Soma::Engine
    Soma::SpriterPlusPlus
    sfeMovie::sfeMovie
    GLEW::GLEW
    OpenGL::GL
)
set_target_properties(Soma PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Soma>"
)

if(MSVC)
    target_compile_options(Soma PRIVATE /W3 /permissive- /utf-8)
    target_compile_options(soma_engine PRIVATE /W3 /permissive- /utf-8)
    target_compile_options(spriterplusplus PRIVATE /W3 /permissive- /utf-8)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source" FILES "src/main.cpp" ${SOMA_ENGINE_SOURCES} ${SOMA_GAME_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SpriterPlusPlus" PREFIX "ThirdParty/SpriterPlusPlus" FILES ${SPRITER_SOURCES})

add_custom_command(TARGET Soma POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/Data"
            "$<TARGET_FILE_DIR:Soma>/Data"
    COMMENT "Copying game data"
)

if(WIN32 AND SOMA_USE_BUNDLED_DEPENDENCIES)
    file(GLOB SFML_RUNTIME_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/SFML-3.1.0/bin/*.dll")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(GLEW_RUNTIME_DLL "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/glew-2.3.1/bin/Release/x64/glew32.dll")
    else()
        set(GLEW_RUNTIME_DLL "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/glew-2.3.1/bin/Release/Win32/glew32.dll")
    endif()

    if(NOT SOMA_FFMPEG_RUNTIME_DIR)
        if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/ffmpeg/bin")
            set(SOMA_FFMPEG_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/ffmpeg/bin")
        elseif(IS_DIRECTORY "C:/ffmpeg/bin")
            set(SOMA_FFMPEG_RUNTIME_DIR "C:/ffmpeg/bin")
        endif()
    endif()
    file(GLOB FFMPEG_RUNTIME_DLLS
        "${SOMA_FFMPEG_RUNTIME_DIR}/avcodec-*.dll"
        "${SOMA_FFMPEG_RUNTIME_DIR}/avformat-*.dll"
        "${SOMA_FFMPEG_RUNTIME_DIR}/avutil-*.dll"
        "${SOMA_FFMPEG_RUNTIME_DIR}/swresample-*.dll"
        "${SOMA_FFMPEG_RUNTIME_DIR}/swscale-*.dll"
    )
    if(NOT FFMPEG_RUNTIME_DLLS)
        message(WARNING "FFmpeg runtime DLLs were not found. Set SOMA_FFMPEG_RUNTIME_DIR before running Soma.")
    endif()

    add_custom_command(TARGET Soma POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                ${SFML_RUNTIME_DLLS}
                "${GLEW_RUNTIME_DLL}"
                ${FFMPEG_RUNTIME_DLLS}
                "$<TARGET_FILE_DIR:Soma>"
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime dependencies"
    )
endif()
