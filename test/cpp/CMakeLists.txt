# supresss warnings generated from including Kakadu headers
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wno-deprecated-volatile)
  add_compile_options(-Wno-return-type)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_compile_options(-Wno-return-type)
  add_compile_options(-Wno-volatile)
  add_compile_options(-Wno-deprecated-declarations) # KAKADU_THREADING=ON
endif()

add_executable(cpptest main.cpp)

target_link_libraries(cpptest PRIVATE libkdu)

target_compile_features(cpptest PUBLIC cxx_std_20)
