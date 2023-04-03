set(server_name server)

add_executable(${server_name})

include(CompileOptions)
set_compile_options(${server_name})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(
  ${server_name}
  PUBLIC
    Threads::Threads
)

target_sources(
  ${server_name}
  PRIVATE
  server.c
)

target_include_directories(
  ${server_name}
  PRIVATE
    ${PROJECT_SOURCE_DIR}/include/
)