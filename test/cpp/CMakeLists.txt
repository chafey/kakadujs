# supresss warnings generated from including Kakadu headers
add_compile_options(-Wno-volatile) # GCC
add_compile_options(-Wno-deprecated-declarations) # KAKADU_THREADING=ON
add_compile_options(-Wno-deprecated-volatile) #CLANG
add_compile_options(-Wno-return-type) #CLANG

add_executable(cpptest main.cpp)

target_link_libraries(cpptest PRIVATE libkdu)

target_compile_features(cpptest PUBLIC cxx_std_20)
