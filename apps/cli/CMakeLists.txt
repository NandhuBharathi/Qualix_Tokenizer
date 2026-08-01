add_executable(
    qualix_cli
    main.cpp
)

target_link_libraries(
    qualix_cli
    PRIVATE
        qualix
)

target_compile_features(
    qualix_cli
    PRIVATE
        cxx_std_20
)

set_target_properties(
    qualix_cli
    PROPERTIES
        OUTPUT_NAME qualix
)
