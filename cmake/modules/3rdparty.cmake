include(ExternalProject)

set(AWESOMEASSERT_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/AwesomeAssert-prefix)
ExternalProject_Add(
  AwesomeAssert
  GIT_REPOSITORY  "https://github.com/muggenhor/awesome-assert.git"
  GIT_TAG         "a997c5a1510db2c610d1da12ea6f5e9b0d954ad7"
  CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:STRING=<INSTALL_DIR>
  PREFIX           ${AWESOMEASSERT_PREFIX}
  INSTALL_DIR      ${AWESOMEASSERT_PREFIX}
  BUILD_BYPRODUCTS ${AWESOMEASSERT_PREFIX}/lib/libAwesomeAssert.so
  USES_TERMINAL_TEST ON
  TEST_BEFORE_INSTALL ON
)

add_library(AwesomeAssert::AwesomeAssert SHARED IMPORTED)
add_dependencies(AwesomeAssert::AwesomeAssert AwesomeAssert)

set_target_properties(AwesomeAssert::AwesomeAssert PROPERTIES
  INTERFACE_COMPILE_FEATURES "cxx_constexpr;cxx_explicit_conversions;cxx_extern_templates;cxx_func_identifier;cxx_noexcept;cxx_nullptr;cxx_override;cxx_range_for;cxx_rvalue_references;cxx_static_assert;cxx_strong_enums;cxx_trailing_return_types"
  INTERFACE_INCLUDE_DIRECTORIES "${AWESOMEASSERT_PREFIX}/include"
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
  IMPORTED_LOCATION "${AWESOMEASSERT_PREFIX}/lib/libAwesomeAssert.so"
  IMPORTED_SONAME "libAwesomeAssert.so"
)

# Prevent CMake from complaining about this directories missing when the AwesomeAssert::AwesomeAssert target gets used as dependency
file(MAKE_DIRECTORY "${AWESOMEASSERT_PREFIX}/include")
