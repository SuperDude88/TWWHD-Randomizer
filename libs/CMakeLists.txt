cmake_minimum_required(VERSION 3.13)

target_sources(wwhd_rando PRIVATE base64pp.cpp AES.cpp hashing/sha1.cpp hashing/sha256.cpp)

set(BUILD_SHARED_LIBS OFF)

# Modded zlib-ng for fast yaz0
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
if(DEFINED DEVKITPRO)
    option(HAVE_THREAD_LOCAL OFF)
    add_compile_definitions(Z_TLS=)
endif()
option(WITH_MAINTAINER_WARNINGS "Build with project maintainer warnings" OFF)
option(WITH_CODE_COVERAGE "Enable code coverage reporting" OFF)
option(WITH_INFLATE_STRICT "Build with strict inflate distance checking" OFF)
option(WITH_INFLATE_ALLOW_INVALID_DIST "Build with zero fill for inflate invalid distances" OFF)
option(WITH_UNALIGNED "Support unaligned reads on platforms that support it" ON)
add_compile_definitions(NO_GZIP=1)
add_subdirectory("zlib-ng")

# Disable extra stuff for yaml-cpp
option(YAML_CPP_BUILD_CONTRIB OFF)
option(YAML_CPP_BUILD_TOOLS OFF)
option(YAML_CPP_BUILD_TESTS OFF)
option(YAML_CPP_INSTALL OFF)
option(YAML_MSVC_SHARED_RT OFF)
add_subdirectory("yaml-cpp")
# Prevent position independent code from generating relocations that Wii U doesn't support
set_property(TARGET yaml-cpp PROPERTY POSITION_INDEPENDENT_CODE OFF) # yaml-cpp enables this automatically when BUILD_SHARED_LIBS is off, override it here

option(tinyxml2_BUILD_TESTING OFF)
add_subdirectory("tinyxml2")

target_link_libraries(wwhd_rando PRIVATE zlib yaml-cpp tinyxml2)
