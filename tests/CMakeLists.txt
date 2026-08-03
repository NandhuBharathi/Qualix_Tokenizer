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

qualix_add_test(test_bpe_types)

qualix_add_test(test_bpe_pair_counter)

qualix_add_test(test_bpe_merger)

qualix_add_test(test_bpe_merge_rule)

qualix_add_test(test_bpe_symbol_allocator)
qualix_add_test(test_bpe_trainer)

qualix_add_test(test_bpe_vocabulary)

qualix_add_test(test_bpe_symbolizer)

qualix_add_test(test_bpe_span_encoder)


add_executable(
    test_bpe_corpus_trainer
    test_bpe_corpus_trainer.cpp
)

target_link_libraries(
    test_bpe_corpus_trainer
    PRIVATE
        qualix
)

add_test(
    NAME test_bpe_corpus_trainer
    COMMAND test_bpe_corpus_trainer
)


add_executable(
    test_bpe_encoder
    test_bpe_encoder.cpp
)

target_link_libraries(
    test_bpe_encoder
    PRIVATE
        qualix
)

add_test(
    NAME test_bpe_encoder
    COMMAND test_bpe_encoder
)


qualix_add_test(test_bpe_model)

qualix_add_test(test_bpe_model_trainer)


add_executable(
    test_bpe_binary_format
    test_bpe_binary_format.cpp
)

target_link_libraries(
    test_bpe_binary_format
    PRIVATE
        qualix
)

add_test(
    NAME test_bpe_binary_format
    COMMAND test_bpe_binary_format
)


add_executable(
    test_bpe_serializer
    test_bpe_serializer.cpp
)

target_link_libraries(
    test_bpe_serializer
    PRIVATE
        qualix
)

add_test(
    NAME test_bpe_serializer
    COMMAND test_bpe_serializer
)


add_executable(
    test_bpe_deserializer
    test_bpe_deserializer.cpp
)

target_link_libraries(
    test_bpe_deserializer
    PRIVATE
        qualix
)

add_test(
    NAME test_bpe_deserializer
    COMMAND test_bpe_deserializer
)


add_executable(
    test_bpe_model_io
    test_bpe_model_io.cpp
)

target_link_libraries(
    test_bpe_model_io
    PRIVATE
    qualix
)

add_test(
    NAME test_bpe_model_io
    COMMAND test_bpe_model_io
)


add_executable(
    test_bpe_end_to_end
    test_bpe_end_to_end.cpp
)

target_link_libraries(
    test_bpe_end_to_end
    PRIVATE
        qualix
)

add_test(
    NAME test_bpe_end_to_end
    COMMAND test_bpe_end_to_end
)


add_executable(
    test_bpe_decoder
    test_bpe_decoder.cpp
)

target_link_libraries(
    test_bpe_decoder
    PRIVATE
    qualix
)

add_test(
    NAME test_bpe_decoder
    COMMAND test_bpe_decoder
)


add_executable(
    test_bpe_model_decode
    test_bpe_model_decode.cpp
)

target_link_libraries(
    test_bpe_model_decode
    PRIVATE
    qualix
)

add_test(
    NAME test_bpe_model_decode
    COMMAND test_bpe_model_decode
)


add_executable(test_bpe_byte_fallback
    test_bpe_byte_fallback.cpp
)

target_link_libraries(test_bpe_byte_fallback
    PRIVATE
        qualix
)

add_test(
    NAME test_bpe_byte_fallback
    COMMAND test_bpe_byte_fallback
)


add_executable(test_bpe_byte_fallback_vocabulary
    test_bpe_byte_fallback_vocabulary.cpp
)

target_link_libraries(test_bpe_byte_fallback_vocabulary
    PRIVATE
        qualix
)

add_test(
    NAME test_bpe_byte_fallback_vocabulary
    COMMAND test_bpe_byte_fallback_vocabulary
)


add_executable(
    test_bpe_symbolizer_fallback
    test_bpe_symbolizer_fallback.cpp
)

target_link_libraries(
    test_bpe_symbolizer_fallback
    PRIVATE
    qualix
)

add_test(
    NAME test_bpe_symbolizer_fallback
    COMMAND test_bpe_symbolizer_fallback
)

add_executable(test_bpe_decoder_fallback
    test_bpe_decoder_fallback.cpp
)

target_link_libraries(test_bpe_decoder_fallback
    PRIVATE
        qualix
)

add_test(
    NAME test_bpe_decoder_fallback
    COMMAND test_bpe_decoder_fallback
)



add_executable(test_bpe_model_inference_freeze
    test_bpe_model_inference_freeze.cpp
)

target_link_libraries(test_bpe_model_inference_freeze
    PRIVATE
        qualix
)

add_test(
    NAME test_bpe_model_inference_freeze
    COMMAND test_bpe_model_inference_freeze
)


# ---------------------------------------------------------------------
# #75 Trained Model Universal OOV Round-Trip
# ---------------------------------------------------------------------

add_executable(
    test_bpe_oov_roundtrip
    test_bpe_oov_roundtrip.cpp
)

target_link_libraries(
    test_bpe_oov_roundtrip
    PRIVATE
    qualix
)

add_test(
    NAME test_bpe_oov_roundtrip
    COMMAND test_bpe_oov_roundtrip
)


add_executable(
    test_bpe_model_corruption
    test_bpe_model_corruption.cpp
)

target_link_libraries(
    test_bpe_model_corruption
    PRIVATE
    qualix
)

add_test(
    NAME test_bpe_model_corruption
    COMMAND test_bpe_model_corruption
)

add_executable(test_bpe_determinism test_bpe_determinism.cpp)
target_link_libraries(test_bpe_determinism PRIVATE qualix)
add_test(NAME test_bpe_determinism COMMAND test_bpe_determinism)

add_executable(test_bpe_model_invariants test_bpe_model_invariants.cpp)
target_link_libraries(test_bpe_model_invariants PRIVATE qualix)
add_test(NAME test_bpe_model_invariants COMMAND test_bpe_model_invariants)

add_executable(test_bpe_adversarial test_bpe_adversarial.cpp)
target_link_libraries(test_bpe_adversarial PRIVATE qualix)
add_test(NAME test_bpe_adversarial COMMAND test_bpe_adversarial)

add_executable(bench_bpe bench_bpe.cpp)
target_link_libraries(bench_bpe PRIVATE qualix)

add_executable(bench_bpe_probe bench_bpe_probe.cpp)
target_link_libraries(bench_bpe_probe PRIVATE qualix)


add_executable(test_rule_performance
    test_rule_performance.cpp
)

target_link_libraries(test_rule_performance
    PRIVATE qualix
)


add_executable(test_grapheme_performance
    test_grapheme_performance.cpp
)

target_link_libraries(test_grapheme_performance
    PRIVATE qualix
)


add_executable(test_pretokenizer_profile_large
    test_pretokenizer_profile_large.cpp
)

target_link_libraries(test_pretokenizer_profile_large
    PRIVATE qualix
)


add_executable(test_bpe_exact_workload
    test_bpe_exact_workload.cpp
)

target_link_libraries(test_bpe_exact_workload
    PRIVATE qualix
)

add_test(
    NAME test_bpe_exact_workload
    COMMAND test_bpe_exact_workload
)


add_executable(test_bpe_scaling
    test_bpe_scaling.cpp
)

target_link_libraries(test_bpe_scaling
    PRIVATE qualix
)

add_test(
    NAME test_bpe_scaling
    COMMAND test_bpe_scaling
)


add_executable(
    test_rule_router_performance
    test_rule_router_performance.cpp
)

target_link_libraries(
    test_rule_router_performance
    PRIVATE
    qualix
)


add_executable(
    test_pretokenizer_thread_scaling
    test_pretokenizer_thread_scaling.cpp
)

target_link_libraries(
    test_pretokenizer_thread_scaling
    PRIVATE
    qualix
)


add_executable(
    test_parallel_bottleneck
    test_parallel_bottleneck.cpp
)

target_link_libraries(
    test_parallel_bottleneck
    PRIVATE
    qualix
)


add_executable(test_rule_router_stats
    test_rule_router_stats.cpp
)

target_link_libraries(test_rule_router_stats
    PRIVATE qualix
)


add_executable(test_router_second_stage
    test_router_second_stage.cpp
)

target_link_libraries(test_router_second_stage
    PRIVATE qualix
)


add_executable(test_router_prefix_hint
    test_router_prefix_hint.cpp
)

target_link_libraries(test_router_prefix_hint
    PRIVATE qualix
)


add_executable(
    test_streaming_pretokenizer
    test_streaming_pretokenizer.cpp
)

target_link_libraries(
    test_streaming_pretokenizer
    PRIVATE
    qualix
)

add_test(
    NAME test_streaming_pretokenizer
    COMMAND test_streaming_pretokenizer
)


add_executable(
    test_streaming_boundary
    test_streaming_boundary.cpp
)

target_link_libraries(
    test_streaming_boundary
    PRIVATE
    qualix
)

add_test(
    NAME test_streaming_boundary
    COMMAND test_streaming_boundary
)


add_executable(
    test_streaming_incremental
    test_streaming_incremental.cpp
)

target_link_libraries(
    test_streaming_incremental
    PRIVATE
    qualix
)

add_test(
    NAME test_streaming_incremental
    COMMAND test_streaming_incremental
)


add_executable(
    test_streaming_differential
    test_streaming_differential.cpp
)

target_link_libraries(
    test_streaming_differential
    PRIVATE
    qualix
)


