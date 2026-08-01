add_library(qualix)

target_compile_features(qualix PUBLIC cxx_std_20)

target_include_directories(
    qualix
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_subdirectory(core)
add_subdirectory(unicode)
add_subdirectory(pretokenizer)
