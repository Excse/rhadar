foreach(test_name IN ITEMS rhadar_message rhadar_json)
    add_executable(${test_name}_test ${test_name}.cpp)
    target_link_libraries(${test_name}_test PRIVATE rhadar::rhadar)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${test_name}_test PRIVATE
            -Wall -Wextra -Werror -pedantic -fno-exceptions -fno-rtti)
    endif()
    add_test(NAME ${test_name} COMMAND ${test_name}_test)
endforeach()
