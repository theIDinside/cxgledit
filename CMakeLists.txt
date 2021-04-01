cmake_minimum_required(VERSION 3.16)
project(cxgledit LANGUAGES CXX C)

set(EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)
include(./cmake/CustomFunctions.cmake)

message("Project source dir ${PROJECT_SOURCE_DIR}")
set(SRC_DIR ./src)

set(DEP_DIR ${PROJECT_SOURCE_DIR}/deps)
set(RUN_FROM_FOLDER ${PROJECT_SOURCE_DIR}/bin/run)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PROJECT_SOURCE_DIR}/bin/relwithdebinfo)

# flags taken from; https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md
set(CLANG_FLAGS -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op
        -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2 -Wlifetime)

set(GCC_FLAGS ${CLANG_FLAGS})
set(MSVC_FLAGS /W4 /w14242 /w14254 /w14263 /w14265 /w14287 /we4289 /w14296 /w14311 /w14545 /w14546 /w14547 /w14549 /w14555 /w14619 /w14640 /w14826 /w14905 /w14906 /w14928)

Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master
        GIT_PROGRESS TRUE
)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG master
        GIT_PROGRESS TRUE
)

FetchContent_Declare(
        cxutils
        GIT_REPOSITORY https://github.com/theIDinside/cxutils.git
        GIT_TAG master
        GIT_PROGRESS TRUE
)

FetchContent_GetProperties(fmt)
FetchContent_GetProperties(glfw)
FetchContent_GetProperties(cxutils)

if (NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
    add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})
endif ()

if (NOT glfw_POPULATED)
    FetchContent_Populate(glfw)
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif ()

if (NOT cxutils_POPULATED)
    FetchContent_Populate(cxutils)
    add_subdirectory(${cxutils_SOURCE_DIR} ${cxutils_BINARY_DIR})
endif ()

set(SOURCES
        src/main.cpp
        src/app.cpp

        # CORE sources
        src/core/core.hpp
        src/core/strops.cpp                     src/core/strops.hpp
        src/core/buffer/text_data.cpp           src/core/buffer/text_data.hpp
        src/core/buffer/data_manager.cpp        src/core/buffer/data_manager.hpp
        src/cfg/configuration.cpp               src/cfg/configuration.hpp
        src/core/math/vector.cpp                src/core/math/vector.hpp
        src/core/math/matrix.cpp                src/core/math/matrix.hpp
        src/core/buffer/file_context.cpp        src/core/buffer/file_context.hpp
        src/core/buffer/std_string_buffer.cpp   src/core/buffer/std_string_buffer.hpp
        src/core/buffer/bookmark.cpp            src/core/buffer/bookmark.hpp
        src/core/buffer/text_rep.cpp            src/core/buffer/text_rep.hpp

        # UI Render Sources
        src/ui/core/opengl.cpp                  src/ui/core/opengl.hpp
        src/ui/core/layout.cpp                  src/ui/core/layout.hpp
        src/ui/views/view.cpp                   src/ui/views/view.hpp
        src/ui/views/status_bar.cpp             src/ui/views/status_bar.hpp
        src/ui/views/editor_window.cpp          src/ui/views/editor_window.hpp
        src/ui/views/modal.cpp                  src/ui/views/modal.hpp
        src/ui/views/cursors/view_cursor.cpp    src/ui/views/cursors/view_cursor.hpp
        src/ui/views/view_enums.hpp

        # Widgets (Base objects)
        src/ui/widgets/panel.cpp                src/ui/widgets/panel.hpp
        src/ui/widgets/widget.cpp               src/ui/widgets/widget.hpp
        src/ui/widgets/layout/layout.cpp        src/ui/widgets/layout/layout.hpp
        src/ui/widgets/layout/box.cpp           src/ui/widgets/layout/box.hpp
        src/ui/widgets/layout/single_fill.cpp   src/ui/widgets/layout/single_fill.hpp

        # Widgets (Impl)
        src/ui/widgets/panel.cpp                src/ui/widgets/panel.hpp
        src/ui/widgets/editor_window.cpp        src/ui/widgets/editor_window.hpp
        src/ui/widgets/status_bar.cpp           src/ui/widgets/status_bar.hpp
        src/ui/widgets/text_view.cpp            src/ui/widgets/text_view.hpp

        src/ui/managers/widget_manager.cpp      src/ui/managers/widget_manager.hpp
        src/ui/managers/font_library.cpp src/ui/managers/font_library.hpp
        src/ui/managers/shader_library.cpp src/ui/managers/shader_library.hpp
        src/ui/managers/view_manager.cpp src/ui/managers/view_manager.hpp

        # Rendering 
        src/ui/render/font.cpp src/ui/render/font.hpp
        src/ui/render/shader.cpp src/ui/render/shader.hpp
        src/ui/render/texture.cpp src/ui/render/texture.hpp
        src/ui/render/vertex_buffer.cpp src/ui/render/vertex_buffer.hpp
        src/ui/render/vertex_container.cpp src/ui/render/vertex_container.hpp

        src/ui/syntax_highlighting.cpp src/ui/syntax_highlighting.hpp
        # UTIL sources
        src/utils/utils.cpp src/utils/utils.hpp
        src/utils/fileutil.cpp src/utils/fileutil.hpp
        src/utils/strops.hpp
        # COMMAND sources
        src/core/commands/command_interpreter.cpp src/core/commands/command_interpreter.hpp
        src/core/commands/file_manager.cpp src/core/commands/file_manager.hpp
        # CONFIG sources
        src/cfg/types/cursor_options.hpp )

set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
set(GLFW_DIR ${DEP_DIR}/glfw)

add_library(glad ./src/glad.c)

target_include_directories(glad PRIVATE "${DEP_DIR}/include")
set(FT_DIR ${DEP_DIR}/freetype-2.10.4)

add_subdirectory(${FT_DIR})

# Our custom add_library & add_executable calls, to shorten down this file a little tiny bit and make it more skimmable
# defined in ./cmake/CustomFunctions.cmake
SetupLibrary(NAME keybound
        SOURCES deps/include/bindingslib/keybindings.cpp)

set_target_properties(keybound PROPERTIES CXX_STANDARD 20)



SetupExecutable(NAME ${PROJECT_NAME}
        SOURCES ${SOURCES}
        INCLUDE_DIRS "${SRC_DIR}" "${DEP_DIR}/include" "${FT_DIR}/include" "${GLFW_DIR}/include"
        DEPENDENCIES fmt ${OPENGL_LIBRARIES} glfw ${GLFW_LIBRARIES} glad ${CMAKE_DL_LIBS} freetype cxutils keybound)


# Test for widgets, layout functionality etc
SetupExecutable(NAME widget_test

        SOURCES tests/widget_test.cpp
                src/ui/widgets/widget.cpp           src/ui/widgets/widget.hpp
                src/ui/widgets/panel.cpp            src/ui/widgets/panel.hpp
                src/ui/widgets/layout/layout.cpp    src/ui/widgets/layout/layout.hpp
                src/ui/widgets/layout/box.cpp       src/ui/widgets/layout/box.hpp
                src/core/math/vector.cpp            src/core/math/vector.hpp

        INCLUDE_DIRS "${SRC_DIR}"
        DEPENDENCIES fmt cxutils)


# target_include_directories(${PROJECT_NAME} PRIVATE "${SRC_DIR}")
target_compile_definitions(${PROJECT_NAME} PUBLIC "GLFW_INCLUDE_NONE")
# target_include_directories(${PROJECT_NAME} PRIVATE ${DEP_DIR}/include)
# target_include_directories(${PROJECT_NAME} PRIVATE ${FT_DIR}/include)
# target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include")

add_dependencies(cxgledit keybound)


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message("You are using CLANG. Setting CLANG flags: ${CLANG_FLAGS}")
    target_compile_options(${PROJECT_NAME} PRIVATE ${CLANG_FLAGS})
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message("You are using GCC. Setting GCC flags: ${GCC_FLAGS}")
    target_compile_options(${PROJECT_NAME} PRIVATE ${GCC_FLAGS})
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    message("You are using MSVC. Setting MSVC flags: ${MSVC_FLAGS}")
    target_compile_options(${PROJECT_NAME} PRIVATE ${MSVC_FLAGS})
endif()

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
    message("Setting up variables for RelWithDebInfo")
    set(CMAKE_C_FLAGS "/Ot2 /Z7 /Gy ${CMAKE_C_FLAGS}")
    message(Cmake flags for RelWithDebInfo: ${CMAKE_C_FLAGS})
    message("Build type is ${CMAKE_BUILD_TYPE}. Copying assets to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO}/assets")
    message("Debug has instrumentation features enabled.")
    target_compile_definitions(cxgledit PUBLIC INSTRUMENTATION KEEP_LOGS IS_DEBUG DEBUG)
    add_custom_command(TARGET cxgledit POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO}/assets)
    add_custom_command(TARGET cxgledit POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/test_src ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO}/test)
else ()
    # set -DINTRINSICS_ENABLED, when AVX2 character occurence scanning is implemented
    message("Build type is ${CMAKE_BUILD_TYPE}. Copying assets to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/assets")
    message("Debug has instrumentation features enabled.")
    target_compile_definitions(cxgledit PUBLIC INSTRUMENTATION KEEP_LOGS IS_DEBUG DEBUG)
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