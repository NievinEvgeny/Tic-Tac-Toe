set(client_name client)

add_executable(${client_name})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(
  ${client_name}
  PUBLIC
    Threads::Threads
)

include(CompileOptions)
set_compile_options(${client_name})

target_sources(
  ${client_name}
  PRIVATE
  client.c
)

target_include_directories(
  ${client_name}
  PRIVATE
    ${PROJECT_SOURCE_DIR}/include/
)