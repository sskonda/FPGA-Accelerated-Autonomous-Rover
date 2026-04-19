if(DEFINED PICO_SDK_PATH AND EXISTS "${PICO_SDK_PATH}/pico_sdk_init.cmake")
    set(_pico_sdk_path "${PICO_SDK_PATH}")
elseif(DEFINED ENV{PICO_SDK_PATH} AND EXISTS "$ENV{PICO_SDK_PATH}/pico_sdk_init.cmake")
    set(_pico_sdk_path "$ENV{PICO_SDK_PATH}")
else()
    set(_pico_sdk_search_roots
        "${CMAKE_CURRENT_LIST_DIR}/pico-sdk"
        "${CMAKE_CURRENT_LIST_DIR}/../pico-sdk"
        "$ENV{USERPROFILE}/.pico-sdk/sdk"
    )

    foreach(_search_root IN LISTS _pico_sdk_search_roots)
        if(EXISTS "${_search_root}/pico_sdk_init.cmake")
            set(_pico_sdk_path "${_search_root}")
            break()
        endif()

        if(EXISTS "${_search_root}")
            file(GLOB _pico_sdk_candidates LIST_DIRECTORIES true "${_search_root}/*")
            list(SORT _pico_sdk_candidates COMPARE NATURAL ORDER DESCENDING)
            foreach(_candidate IN LISTS _pico_sdk_candidates)
                if(EXISTS "${_candidate}/pico_sdk_init.cmake")
                    set(_pico_sdk_path "${_candidate}")
                    break()
                endif()
            endforeach()
        endif()

        if(DEFINED _pico_sdk_path)
            break()
        endif()
    endforeach()
endif()

if(NOT DEFINED _pico_sdk_path)
    message(FATAL_ERROR
        "Unable to locate a full Raspberry Pi Pico SDK checkout.\n"
        "Set PICO_SDK_PATH to a directory containing pico_sdk_init.cmake."
    )
endif()

set(PICO_SDK_PATH "${_pico_sdk_path}" CACHE PATH "Path to the Raspberry Pi Pico SDK" FORCE)
include("${PICO_SDK_PATH}/pico_sdk_init.cmake")
