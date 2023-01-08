link_libraries(${PROJECT_NAME})
add_executable(test_catalogue test_catalogue.cpp)
add_executable(test_parse test_parse.cpp)

set(example_targets
        test_catalogue
        test_parse)

foreach (target ${example_targets})
    target_compile_options(${target} PRIVATE -Wall -Wextra -Werror)
endforeach ()

#set_property(TARGET ${example_targets} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})
#set_property(TARGET ${example_targets} PROPERTY CXX_STANDARD_REQUIRED ON)
#set_property(TARGET ${example_targets} PROPERTY CXX_EXTENSIONS OFF)
