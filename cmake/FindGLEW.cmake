find_path(GLEW_INCLUDE_DIR NAMES GL/glew.h HINTS "${GLEW_ROOT}" PATH_SUFFIXES include)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_glew_arch x64)
else()
    set(_glew_arch Win32)
endif()

find_library(GLEW_LIBRARY
    NAMES glew32 glew GLEW
    HINTS "${GLEW_ROOT}"
    PATH_SUFFIXES "lib/Release/${_glew_arch}" lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW REQUIRED_VARS GLEW_INCLUDE_DIR GLEW_LIBRARY)

if(GLEW_FOUND AND NOT TARGET GLEW::GLEW)
    add_library(GLEW::GLEW UNKNOWN IMPORTED)
    set_target_properties(GLEW::GLEW PROPERTIES
        IMPORTED_LOCATION "${GLEW_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(GLEW_INCLUDE_DIR GLEW_LIBRARY)

