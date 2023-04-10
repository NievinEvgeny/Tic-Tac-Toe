set(target_name libserver)

add_library(${target_name} STATIC
    connection.c
    ${PROJECT_SOURCE_DIR}/include/server/connection.h
    error_handler.c
    ${PROJECT_SOURCE_DIR}/include/server/error_handler.h
    game.c
    ${PROJECT_SOURCE_DIR}/include/server/game.h
)

include(CompileOptions)
set_compile_options(${target_name})

target_include_directories(
    ${target_name}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include/
)