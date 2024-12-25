cmake_minimum_required(VERSION 3.13)

target_sources(wwhd_rando PRIVATE hash-library/sha1.cpp hash-library/sha256.cpp)

set(BUILD_SHARED_LIBS OFF)

# zlib-ng for RPX compression
option(WITH_GZFILEOP "Compile with support for gzFile related functions" OFF)
option(ZLIB_COMPAT "Compile with zlib compatible API" OFF)
option(ZLIB_ENABLE_TESTS "Build test binaries" OFF)
option(ZLIBNG_ENABLE_TESTS "Test zlib-ng specific API" OFF)
option(WITH_GTEST "Build gtest_zlib" OFF)
option(WITH_FUZZERS "Build test/fuzz" OFF)
option(WITH_BENCHMARKS "Build test/benchmarks" OFF)
option(WITH_BENCHMARK_APPS "Build application benchmarks" OFF)
option(WITH_OPTIM "Build with optimisation" ON)
option(WITH_REDUCED_MEM "Reduced memory usage for special cases (reduces performance)" OFF)
option(WITH_NEW_STRATEGIES "Use new strategies" ON)
option(WITH_MAINTAINER_WARNINGS "Build with project maintainer warnings" OFF)
option(WITH_CODE_COVERAGE "Enable code coverage reporting" OFF)
option(WITH_INFLATE_STRICT "Build with strict inflate distance checking" OFF)
option(WITH_INFLATE_ALLOW_INVALID_DIST "Build with zero fill for inflate invalid distances" OFF)
add_compile_definitions(NO_GZIP)
add_subdirectory("zlib-ng")

# Disable extra stuff for yaml-cpp
option(YAML_CPP_BUILD_CONTRIB OFF)
option(YAML_CPP_BUILD_TOOLS OFF)
option(YAML_CPP_BUILD_TESTS OFF)
option(YAML_CPP_INSTALL OFF)
option(YAML_MSVC_SHARED_RT OFF)
option(YAML_ENABLE_PIC OFF)
add_subdirectory("yaml-cpp")

option(tinyxml2_BUILD_TESTING OFF)
option(tinyxml2_INSTALL_PKGCONFIG OFF)
add_subdirectory("tinyxml2")

add_subdirectory("AES")

add_subdirectory("base64pp")

target_link_libraries(wwhd_rando PRIVATE zlib yaml-cpp tinyxml2 AES base64pp)
