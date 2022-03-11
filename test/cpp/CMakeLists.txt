
add_executable(cpptest main.cpp)

target_link_libraries(cpptest PRIVATE 
    libkdu 
    ${KAKADU_ROOT}/srclib_ht/${KAKADU_PLATFORM}/libkdu_ht.a
)

target_compile_features(cpptest PUBLIC cxx_std_17)
target_link_options(cpptest PUBLIC "-lm")
