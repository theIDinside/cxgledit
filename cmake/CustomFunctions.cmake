include(CMakeParseArguments)

function(SET_CXX_20 REQUIRED_ARG)
    set_target_properties(${REQUIRED_ARG} PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED True
            CXX_EXTENSIONS FALSE)
endfunction()

function(SetupLibrary)
    cmake_parse_arguments(
            PARSED_ARGS # prefix of output variables
            "" # list of names of the boolean arguments (only defined ones will be true)
            "NAME" # list of names of mono-valued arguments
            "SOURCES" # list of names of multi-valued arguments (output variables are lists)
            ${ARGN} # arguments of the function to parse, here we take the all original ones
    )
    # note: if it remains unparsed arguments, here, they can be found in variable PARSED_ARGS_UNPARSED_ARGUMENTS
    if(NOT PARSED_ARGS_NAME)
        message(FATAL_ERROR "You must provide a name")
    endif(NOT PARSED_ARGS_NAME)
    message("Provided sources are:")
    add_library(${PARSED_ARGS_NAME} SHARED ${PARSED_ARGS_SOURCES})
    SET_CXX_20(${PARSED_ARGS_NAME})
endfunction(SetupLibrary)


function(SetupExecutable)
    cmake_parse_arguments(
            PARSED_ARGS # prefix of output variables
            "" # list of names of the boolean arguments (only defined ones will be true)
            "NAME" # list of names of mono-valued arguments
            "SOURCES;DEPENDENCIES" # list of names of multi-valued arguments (output variables are lists)
            ${ARGN} # arguments of the function to parse, here we take the all original ones
    )
    # note: if it remains unparsed arguments, here, they can be found in variable PARSED_ARGS_UNPARSED_ARGUMENTS
    if(NOT PARSED_ARGS_NAME)
        message(FATAL_ERROR "You must provide a name")
    endif(NOT PARSED_ARGS_NAME)
    if(NOT PARSED_ARGS_SOURCES)
        message(FATAL_ERROR "You must provide sources for an executable")
    endif(NOT PARSED_ARGS_SOURCES)

    add_executable(${PARSED_ARGS_NAME} ${PARSED_ARGS_SOURCES})
    target_link_libraries(${PARSED_ARGS_NAME} ${PARSED_ARGS_DEPENDENCIES})
    SET_CXX_20(${PARSED_ARGS_NAME})
endfunction(SetupExecutable)
