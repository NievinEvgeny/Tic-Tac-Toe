set(client_name client)

add_executable(${client_name})

include(CompileOptions)
set_compile_options(${client_name})

target_sources(
  ${client_name}
  PRIVATE
  client.c
)

target_link_libraries(
  ${client_name}
  PUBLIC
    libclient
)

target_include_directories(
  ${client_name}
  PRIVATE
    ${PROJECT_SOURCE_DIR}/include/
)

add_subdirectory(libclient)