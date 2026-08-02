function(qualix_add_test name)
    add_executable(${name}
        ${name}.cpp
    )

    target_link_libraries(${name}
        PRIVATE
            qualix
    )

    target_include_directories(${name}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/tests
    )

    add_test(
        NAME ${name}
        COMMAND ${name}
    )
endfunction()

qualix_add_test(test_types)
qualix_add_test(test_error)
qualix_add_test(test_status)
qualix_add_test(test_result)
qualix_add_test(test_config)
qualix_add_test(test_version)
qualix_add_test(test_logger)
qualix_add_test(test_platform)
qualix_add_test(test_codepoint)
qualix_add_test(test_utf8)

qualix_add_test(test_grapheme)
qualix_add_test(test_grapheme_property)
qualix_add_test(test_indic_conjunct)
qualix_add_test(test_grapheme_segmenter)
qualix_add_test(test_grapheme_conformance)
qualix_add_test(test_normalizer)
qualix_add_test(test_nfd_conformance)
qualix_add_test(test_nfc_conformance)
qualix_add_test(test_nfkd_conformance)
qualix_add_test(test_nfkc_conformance)
qualix_add_test(test_span)
qualix_add_test(test_general_category)
qualix_add_test(test_general_category_conformance)
qualix_add_test(test_grapheme_classifier)
qualix_add_test(test_grapheme_classifier_integration)
qualix_add_test(test_pretokenizer)
qualix_add_test(test_emoji_property)
qualix_add_test(test_emoji_classifier)
qualix_add_test(test_rule_engine)
qualix_add_test(test_url_rule)
qualix_add_test(test_url_pretokenizer)
qualix_add_test(test_email_rule)
qualix_add_test(test_email_pretokenizer)
qualix_add_test(test_number_rule)

qualix_add_test(test_number_pretokenizer)
qualix_add_test(test_percentage_rule)
qualix_add_test(test_percentage_pretokenizer)
qualix_add_test(test_currency_rule)
qualix_add_test(test_currency_pretokenizer)

add_executable(
    test_date_rule
    test_date_rule.cpp
)

target_link_libraries(
    test_date_rule
    PRIVATE
        qualix
)

add_test(
    NAME test_date_rule
    COMMAND test_date_rule
)

add_executable(
    test_date_pretokenizer
    test_date_pretokenizer.cpp
)

target_link_libraries(
    test_date_pretokenizer
    PRIVATE
        qualix
)

add_test(
    NAME test_date_pretokenizer
    COMMAND test_date_pretokenizer
)



add_executable(
    test_time_rule
    test_time_rule.cpp
)

target_link_libraries(
    test_time_rule
    PRIVATE
        qualix
)

add_test(
    NAME test_time_rule
    COMMAND test_time_rule
)


add_executable(
    test_time_pretokenizer
    test_time_pretokenizer.cpp
)

target_link_libraries(
    test_time_pretokenizer
    PRIVATE
        qualix
)

add_test(
    NAME test_time_pretokenizer
    COMMAND test_time_pretokenizer
)


add_executable(
    test_measurement_rule
    test_measurement_rule.cpp
)

target_link_libraries(
    test_measurement_rule
    PRIVATE
        qualix
)

add_test(
    NAME test_measurement_rule
    COMMAND test_measurement_rule
)


add_executable(
    test_measurement_pretokenizer
    test_measurement_pretokenizer.cpp
)

target_link_libraries(
    test_measurement_pretokenizer
    PRIVATE
        qualix
)

add_test(
    NAME test_measurement_pretokenizer
    COMMAND test_measurement_pretokenizer
)


add_executable(
    test_phone_rule
    test_phone_rule.cpp
)

target_link_libraries(
    test_phone_rule
    PRIVATE
        qualix
)

add_test(
    NAME test_phone_rule
    COMMAND test_phone_rule
)

add_executable(
    test_phone_pretokenizer
    test_phone_pretokenizer.cpp
)

target_link_libraries(
    test_phone_pretokenizer
    PRIVATE
        qualix
)

add_test(
    NAME test_phone_pretokenizer
    COMMAND test_phone_pretokenizer
)

add_executable(
    test_math_rule
    test_math_rule.cpp
)

target_link_libraries(
    test_math_rule
    PRIVATE
        qualix
)

add_test(
    NAME test_math_rule
    COMMAND test_math_rule
)



add_executable(
    test_math_pretokenizer
    test_math_pretokenizer.cpp
)

target_link_libraries(
    test_math_pretokenizer
    PRIVATE
        qualix
)

add_test(
    NAME test_math_pretokenizer
    COMMAND test_math_pretokenizer
)
