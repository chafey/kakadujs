
add_executable(cpptest main.cpp)

target_link_libraries(cpptest PRIVATE kakadu kakaduappsupport kakadujs)

set_property(TARGET cpptest PROPERTY CXX_STANDARD 23)
