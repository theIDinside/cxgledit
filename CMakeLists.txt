cmake_minimum_required(VERSION 3.16)
project(cxgledit)
set(EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)
set(CMAKE_CXX_STANDARD 20)
message("Project source dir ${PROJECT_SOURCE_DIR}")
set(SRC_DIR ./src)

set(DEP_DIR ${PROJECT_SOURCE_DIR}/deps)
message("Hello fucker")

set(RUN_FROM_FOLDER ${PROJECT_SOURCE_DIR}/bin/run)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PROJECT_SOURCE_DIR}/bin/relwithdebinfo)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/deps/fmt
)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG master
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/deps/glfw
)

FetchContent_MakeAvailable(fmt glfw)

if (NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
    add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})
endif ()

if (NOT glfw_POPULATED)
    FetchContent_Populate(glfw)
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif ()

set(UI_RENDER_SOURCES
        src/ui/managers/font_library.cpp src/ui/managers/font_library.hpp
        src/ui/managers/shader_library.cpp src/ui/managers/shader_library.hpp
        src/ui/render/font.cpp src/ui/render/font.hpp
        src/ui/render/shader.cpp src/ui/render/shader.hpp
        src/ui/render/texture.cpp src/ui/render/texture.hpp
        src/ui/render/vertex_buffer.cpp src/ui/render/vertex_buffer.hpp

        src/ui/view.cpp src/ui/view.hpp
        src/ui/cursors/view_cursor.cpp src/ui/cursors/view_cursor.hpp

        src/ui/managers/view_manager.cpp src/ui/managers/view_manager.hpp
        src/ui/core/layout.cpp src/ui/core/layout.hpp
        src/ui/editor_window.cpp src/ui/editor_window.hpp
        src/ui/status_bar.cpp src/ui/status_bar.hpp
        src/ui/syntax_highlighting.cpp src/ui/syntax_highlighting.hpp
        src/ui/core/opengl.cpp src/ui/core/opengl.hpp
        src/ui/modal.cpp src/ui/modal.hpp
        src/ui/panel.cpp src/ui/panel.hpp
        )

set(UTIL_SOURCES
        src/utils/utils.cpp src/utils/utils.hpp
        src/utils/fileutil.cpp src/utils/fileutil.hpp
        src/utils/strops.hpp)

set(CORE_SOURCE
        src/core/core.hpp
        src/core/strops.cpp src/core/strops.hpp
        src/core/text_data.cpp src/core/text_data.hpp
        src/core/data_manager.cpp src/core/data_manager.hpp
        src/cfg/configuration.cpp src/cfg/configuration.hpp
        src/core/vector.cpp src/core/vector.hpp src/core/matrix.cpp src/core/matrix.hpp)

set(COMMANDS_SOURCE
        src/core/commands/command.cpp src/core/commands/command.hpp
        src/core/commands/command_interpreter.cpp src/core/commands/command_interpreter.hpp src/core/commands/file_manager.cpp src/core/commands/file_manager.hpp)

add_executable(cxgledit
        src/main.cpp
        src/app.cpp
        ${CORE_SOURCE}
        ${UI_RENDER_SOURCES}
        ${UTIL_SOURCES}
        ${COMMANDS_SOURCE} )

target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
set(GLFW_DIR ${DEP_DIR}/glfw)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
# This doesn't need to be done, now that we use FetchContent to grab the library off of github. The wonders of 2020
# if we do this; it will conflict with the settings, as two targets with the name "glfw" will be set. So don't do it.
# it's just kept here for a mental reminder for me.
# add_subdirectory(${GLFW_DIR})

add_library(glad ./src/glad.c)




target_include_directories(glad PRIVATE "${DEP_DIR}/include")
set(FT_DIR ${DEP_DIR}/freetype-2.10.4)
add_subdirectory(${FT_DIR})

# Setting up properties & configurations for project
target_include_directories(${PROJECT_NAME} PRIVATE "${SRC_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")
target_include_directories(${PROJECT_NAME} PRIVATE ${DEP_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${FT_DIR}/include)
message("OpenGL libraries are set to: ${OPENGL_LIBRARIES}. CMAKE_DL_LIBS set to: ${CMAKE_DL_LIBS}")

target_link_libraries(cxgledit fmt::fmt ${OPENGL_LIBRARIES} glfw ${GLFW_LIBRARIES} glad ${CMAKE_DL_LIBS} freetype)

add_library(keybound SHARED deps/include/bindingslib/keybindings.cpp)
add_dependencies(cxgledit keybound)
target_compile_definitions(keybound PRIVATE "GLFW_INCLUDE_NONE")
# TODO(count character occurence, using avx): set add_definitions(-DINTRINSICS_ENABLED) when feature is implemented
if (CMAKE_BUILD_TYPE STREQUAL Release)
    # set -DINTRINSICS_ENABLED, when AVX2 character occurence scanning is implemented
    message("Build flags for release: ${CMAKE_CXX_FLAGS_RELEASE}")
    message("Build type is ${CMAKE_BUILD_TYPE}. Copying assets to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/assets")
    message("Release has instrumentation features disabled. To enable instrumentation for Release mode, build with type ReleaseInst")
    add_custom_command(TARGET cxgledit POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/assets)
    add_custom_command(TARGET cxgledit POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/test_src ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/test)

elseif(CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
    add_definitions(-DDEBUG)
    message("Build type is ${CMAKE_BUILD_TYPE}. Copying assets to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO}/assets")
    message("Debug has instrumentation features enabled.")
    target_compile_definitions(cxgledit PUBLIC INSTRUMENTATION KEEP_LOGS IS_DEBUG)
    add_custom_command(TARGET cxgledit POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO}/assets)
    add_custom_command(TARGET cxgledit POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/test_src ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO}/test)
else ()
    # set -DINTRINSICS_ENABLED, when AVX2 character occurence scanning is implemented
    add_definitions(-DDEBUG)
    message("Build type is ${CMAKE_BUILD_TYPE}. Copying assets to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/assets")
    message("Debug has instrumentation features enabled.")
    target_compile_definitions(cxgledit PUBLIC INSTRUMENTATION KEEP_LOGS IS_DEBUG)
    add_custom_command(TARGET cxgledit POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/assets)
    add_custom_command(TARGET cxgledit POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/test_src ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/test)
endif ()

# This is simply used for testing. This is so we can set "working directory" to RUN_FROM_FOLDER, regardless
# of what build type we are running, because it will just be the shaders, and test files stored there
add_custom_command(TARGET cxgledit POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/assets ${RUN_FROM_FOLDER}/assets)
add_custom_command(TARGET cxgledit POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/test_src ${RUN_FROM_FOLDER}/test)