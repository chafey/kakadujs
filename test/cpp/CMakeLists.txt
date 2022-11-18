
add_executable(cpptest main.cpp)

if(UNIX)
    target_link_libraries(cpptest PRIVATE 
        libkdu 
        ${KAKADU_ROOT}/srclib_ht/${KAKADU_PLATFORM}/libkdu_ht.a m
    )
elseif(WIN32) 
    target_link_libraries(cpptest PRIVATE 
        libkdu 
        ${KAKADU_ROOT}/srclib_ht/${KAKADU_PLATFORM}/kdu_ht2019R.lib
    )
endif()

target_compile_features(cpptest PUBLIC cxx_std_20)
