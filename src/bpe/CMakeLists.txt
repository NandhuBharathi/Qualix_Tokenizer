target_sources(
    qualix
    PRIVATE
        pair_counter.cpp
        merger.cpp
        symbol_allocator.cpp
        trainer.cpp
        vocabulary.cpp
        symbolizer.cpp
        encoder.cpp
)

target_sources(
    qualix
    PRIVATE
        span_encoder.cpp
        corpus_trainer.cpp
        model.cpp
)

target_sources(
    qualix
    PRIVATE
        model_trainer.cpp
        binary_format.cpp
    byte_fallback.cpp
        serializer.cpp
    decoder.cpp
)


target_sources(
    qualix
    PRIVATE
        deserializer.cpp
    model_io.cpp
)
