add_executable(test_iban iban.cpp test_iban.cpp)
target_link_libraries(test_iban GTest::gtest_main)

gtest_discover_tests(test_iban)