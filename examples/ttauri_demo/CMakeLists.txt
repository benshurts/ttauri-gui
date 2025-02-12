
#-------------------------------------------------------------------
# Build Target: ttauri_demo                             (executable)
#-------------------------------------------------------------------

add_executable(ttauri_demo WIN32 MACOSX_BUNDLE)

target_link_libraries(ttauri_demo PRIVATE ttauri)
target_include_directories(ttauri_demo PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)

add_subdirectory(src)

add_custom_target(ttauri_demo_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../../resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    DEPENDS
    ttauri
    resources/ttauri_demo.png
    resources/locale/en.po
    resources/locale/nl.po
)

add_dependencies(ttauri_demo ttauri_demo_resources)

if(MSVC)
    if(TT_ENABLE_ANALYSIS)
        set(ttauri_demo_do_not_analyze
            src/preferences_controller.cpp
            src/main_window_controller.cpp
            src/audio_preferences_controller.cpp
            src/main.cpp
            src/application_controller.cpp
            src/license_preferences_controller.cpp
        )
        set_source_files_properties(${ttauri_demo_do_not_analyze} PROPERTIES COMPILE_FLAGS -analyze-)
    endif()
endif()

#-------------------------------------------------------------------
# Installation Rules: ttauri_demo
#-------------------------------------------------------------------

install(TARGETS ttauri_demo DESTINATION examples/ttauri_demo)

install(DIRECTORY resources/ DESTINATION examples/ttauri_demo/resources)

# copy additional "ttauri library" resources from top-level
install(DIRECTORY ../../resources/  DESTINATION examples/ttauri_demo/resources)

# copy vcpkg dependencies (experimental)
if (VCPKG_TOOLCHAIN)
    x_vcpkg_install_local_dependencies(TARGETS ttauri_demo DESTINATION examples/ttauri_demo)
endif()

