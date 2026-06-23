function(add_c2_test name src)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE c2cudaq)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_c2_test(test_arith  test_arith.cpp)
add_c2_test(test_graph  test_graph.cpp)
add_c2_test(test_factor test_factor.cpp)
