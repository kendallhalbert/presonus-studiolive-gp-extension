# ============================================================================
#  tests/  -  GoogleTest-based unit tests
#  ---------------------------------------------------------------------------
#  Phase 0 only ships a single sanity test (`test_sanity`). The point is to
#  exercise the FetchContent + ctest pipeline before Phase 1 starts adding
#  real coverage. See docs/GP_EXTENSION_PLAN.md §6 for the testing strategy.
# ============================================================================

include(FetchContent)

# Pin GoogleTest. v1.14.0 is the latest stable that builds clean on MSVC
# and is supported by CMake 4.x's policy defaults.
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
  GIT_SHALLOW    TRUE)

# Don't override the parent project's compile flags or the MSVC CRT.
set(gtest_force_shared_crt ON  CACHE BOOL "" FORCE)
set(BUILD_GMOCK            ON  CACHE BOOL "" FORCE)
set(INSTALL_GTEST          OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Bundle GoogleTest's CMake helpers (gtest_discover_tests, etc.).
include(GoogleTest)

# ---------------------------------------------------------------------------
#  Per-translation-unit executables (matches the §6.1 testing approach).
# ---------------------------------------------------------------------------
#
# Each .cpp under unit/ becomes its own test_<name>.exe. We deliberately keep
# the tests isolated from each other so Phase 1+ can add new files without
# rebuilding unrelated ones.
function(psl_add_unit_test name)
  add_executable(test_${name} unit/test_${name}.cpp)
  target_link_libraries(test_${name} PRIVATE
    GTest::gtest_main
    GTest::gmock
    psl_bridge
    psl_protocol)
  target_compile_definitions(test_${name} PRIVATE
    PSL_FIXTURE_ROOT="${CMAKE_SOURCE_DIR}/tests/fixtures")
  target_compile_features(test_${name} PRIVATE cxx_std_20)
  target_include_directories(test_${name} PRIVATE
    ${CMAKE_SOURCE_DIR}/extension/src)

  # Mirror the defines that extension/ sets on the DLL target so headers
  # like Version.h compile in the test TU exactly as they do in production.
  target_compile_definitions(test_${name} PRIVATE
    PSL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    PSL_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    PSL_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    PSL_VERSION_TAG="phase0")

  if(MSVC)
    target_compile_options(test_${name} PRIVATE /W4 /permissive- /utf-8)
    target_compile_definitions(test_${name} PRIVATE
      _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
  endif()
  set_target_properties(test_${name} PROPERTIES FOLDER tests)
  gtest_discover_tests(test_${name})
endfunction()

# ---------------------------------------------------------------------------
#  Phase 0 sanity test.  Phase 1 will replace / extend this list.
# ---------------------------------------------------------------------------
psl_add_unit_test(sanity)
psl_add_unit_test(dispatcher)
psl_add_unit_test(logger)
psl_add_unit_test(gp_host)
psl_add_unit_test(extension_context)
psl_add_unit_test(config_store)
psl_add_unit_test(message_protocol)
psl_add_unit_test(param_parsers)
psl_add_unit_test(data_client)
psl_add_unit_test(zlib_handshake)
psl_add_unit_test(ubjson)
psl_add_unit_test(session_packet_decoder)
psl_add_unit_test(fd_assembler)
psl_add_unit_test(keepalive)
psl_add_unit_test(mixer_connection)
