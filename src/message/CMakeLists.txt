find_program(RE2C_EXE re2c REQUIRED)

function(add_re2c output input)
    add_custom_command(
        OUTPUT ${output}
        COMMAND ${RE2C_EXE} ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${input} -o ${output} ${ARGN}
        DEPENDS ${input}
    )
endfunction()

##################
# Message Parser #
##################

# I've had mixed results with GNU style lines in MSVC
# so I'm setting them explicitly here. Regardless, they
# don't do anything except for us, the developers.
if (MSVC)
    list(APPEND QUIRK_PARSER_ARGS --location-format msvc)
elseif()
    list(APPEND QUIRK_PARSER_ARGS --location-format gnu)
endif()

add_re2c(parser.c parser.re.c
    --utf8
    --storable-state
    --start-conditions
    -Wundefined-control-flow
    -Wunreachable-rules
    -Wuseless-escape
    -Wswapped-range
    -Wempty-character-class
    -Wmatch-empty-string
    -Wsentinel-in-midrule)

add_library(quirk-message STATIC parser.c)
target_compile_features(quirk-message PRIVATE c_std_90)
target_link_libraries(quirk-message PUBLIC quirk-headers)

if (WIN32)
    # We only support Windows 10... because it's the only one
    # still being supported. No, I won't support Windows XP, 7,
    # or even 8 or 8.1. Use a modern operating system.
    target_compile_definitions(quirk-message PRIVATE _WIN32_WINNT=0x0A00)
endif()

if (MSVC)
    target_link_options(quirk-message PRIVATE /NODEFAULTLIB)
endif()
