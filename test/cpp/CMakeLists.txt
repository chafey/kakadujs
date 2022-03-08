
add_executable(cpptest main.cpp)

target_link_libraries(cpptest PRIVATE 
    libkdu 
    ${KAKADU_ROOT}/srclib_ht/Mac-x86-64-gcc/libkdu_ht.a
)

target_compile_features(cpptest PUBLIC cxx_std_17)

