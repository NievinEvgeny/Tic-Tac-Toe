set(target_name libclient)

add_library(${target_name} STATIC
    connection.c
    ${PROJECT_SOURCE_DIR}/include/client/connection.h
    error_handler.c
    ${PROJECT_SOURCE_DIR}/include/client/error_handler.h
    game.c
    ${PROJECT_SOURCE_DIR}/include/client/game.h
)

include(CompileOptions)
set_compile_options(${target_name})

target_include_directories(
    ${target_name}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/
)