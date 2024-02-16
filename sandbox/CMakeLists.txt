add_executable(sandbox main.cpp)
target_link_libraries(sandbox PRIVATE js)
install(TARGETS sandbox RUNTIME DESTINATION bin)
