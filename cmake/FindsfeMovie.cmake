find_path(sfeMovie_INCLUDE_DIR
    NAMES sfeMovie/Movie.hpp
    HINTS "${SFEMOVIE_ROOT}"
    PATH_SUFFIXES include
)

find_library(sfeMovie_LIBRARY_RELEASE
    NAMES sfeMovie
    HINTS "${SFEMOVIE_ROOT}"
    PATH_SUFFIXES lib
)

find_library(sfeMovie_LIBRARY_DEBUG
    NAMES sfeMovie-d sfeMovied sfeMovie
    HINTS "${SFEMOVIE_ROOT}"
    PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(sfeMovie
    REQUIRED_VARS sfeMovie_INCLUDE_DIR sfeMovie_LIBRARY_RELEASE
)

if(sfeMovie_FOUND AND NOT TARGET sfeMovie::sfeMovie)
    add_library(sfeMovie::sfeMovie SHARED IMPORTED)
    set_target_properties(sfeMovie::sfeMovie PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${sfeMovie_INCLUDE_DIR}"
        IMPORTED_IMPLIB_RELEASE "${sfeMovie_LIBRARY_RELEASE}"
        IMPORTED_IMPLIB_RELWITHDEBINFO "${sfeMovie_LIBRARY_RELEASE}"
        IMPORTED_IMPLIB_MINSIZEREL "${sfeMovie_LIBRARY_RELEASE}"
        IMPORTED_IMPLIB_DEBUG "${sfeMovie_LIBRARY_DEBUG}"
    )
    if(WIN32)
        find_file(sfeMovie_DLL NAMES sfeMovie.dll sfeMovie-d.dll HINTS "${SFEMOVIE_ROOT}" PATH_SUFFIXES bin)
        set_target_properties(sfeMovie::sfeMovie PROPERTIES
            IMPORTED_LOCATION_RELEASE "${sfeMovie_DLL}"
            IMPORTED_LOCATION_RELWITHDEBINFO "${sfeMovie_DLL}"
            IMPORTED_LOCATION_MINSIZEREL "${sfeMovie_DLL}"
            IMPORTED_LOCATION_DEBUG "${sfeMovie_DLL}"
        )
    endif()
endif()

mark_as_advanced(sfeMovie_INCLUDE_DIR sfeMovie_LIBRARY_RELEASE sfeMovie_LIBRARY_DEBUG sfeMovie_DLL)

