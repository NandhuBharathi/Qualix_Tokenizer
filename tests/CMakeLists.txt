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
