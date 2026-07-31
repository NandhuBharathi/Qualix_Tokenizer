add_executable(test_types
    test_types.cpp
)
target_link_libraries(test_types PRIVATE qualix)
target_include_directories(test_types PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/tests
)
add_test(NAME test_types COMMAND test_types)

add_executable(test_error
    test_error.cpp
)
target_link_libraries(test_error PRIVATE qualix)
target_include_directories(test_error PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/tests
)
add_test(NAME test_error COMMAND test_error)

add_executable(test_status
    test_status.cpp
)
target_link_libraries(test_status PRIVATE qualix)
target_include_directories(test_status PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/tests
)
add_test(NAME test_status COMMAND test_status)

add_executable(test_result
    test_result.cpp
)
target_link_libraries(test_result PRIVATE qualix)
target_include_directories(test_result PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/tests
)
add_test(NAME test_result COMMAND test_result)

add_executable(test_config
    test_config.cpp
)
target_link_libraries(test_config PRIVATE qualix)
target_include_directories(test_config PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/tests
)
add_test(NAME test_config COMMAND test_config)

add_executable(test_version
    test_version.cpp
)
target_link_libraries(test_version PRIVATE qualix)
target_include_directories(test_version PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/tests
)
add_test(NAME test_version COMMAND test_version)

add_executable(test_logger
    test_logger.cpp
)
target_link_libraries(test_logger PRIVATE qualix)
target_include_directories(test_logger PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/tests
)
add_test(NAME test_logger COMMAND test_logger)

add_executable(test_platform
    test_platform.cpp
)
target_link_libraries(test_platform PRIVATE qualix)
target_include_directories(test_platform PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/tests
)
add_test(NAME test_platform COMMAND test_platform)
