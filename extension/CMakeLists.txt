# ============================================================================
#  extension/  -  the PreSonusStudioLive.dll target itself
# ============================================================================

include(FetchContent)
FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG        v1.3.1
  GIT_SHALLOW    TRUE)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(zlib)

add_library(psl_bridge STATIC
  src/bridge/Logger.cpp
  src/bridge/Dispatcher.cpp
  src/bridge/RealGpHost.cpp
  src/bridge/ExtensionContext.cpp
  src/bridge/ScriptFunctions.cpp
  src/bridge/ConfigStore.cpp)

add_library(psl_protocol STATIC
  src/protocol/MessageProtocol.cpp
  src/protocol/ParamKey.cpp
  src/protocol/HexUtil.cpp
  src/protocol/PvParser.cpp
  src/protocol/PcParser.cpp
  src/protocol/PsParser.cpp
  src/protocol/MsParser.cpp
  src/protocol/DataClient.cpp
  src/protocol/PacketParser.cpp
  src/protocol/ZlibInflate.cpp
  src/protocol/Ubjson.cpp
  src/protocol/ZlibState.cpp
  src/protocol/CkAssembler.cpp
  src/protocol/ZbParser.cpp
  src/protocol/SessionPacketDecoder.cpp
  src/protocol/FdAssembler.cpp
  src/protocol/FdParser.cpp
  src/protocol/KeepAlive.cpp
  src/protocol/MixerConnection.cpp
  src/transport/ScriptedTransport.cpp
  src/transport/WinSockTransport.cpp)

target_include_directories(psl_protocol
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  PRIVATE
    ${zlib_SOURCE_DIR}
    ${zlib_BINARY_DIR})

target_link_libraries(psl_protocol
  PUBLIC
    zlibstatic
  PRIVATE
    ws2_32)

target_compile_features(psl_protocol PUBLIC cxx_std_20)

if(MSVC)
  target_compile_options(psl_protocol PRIVATE
    /W4 /permissive- /utf-8 /Zc:__cplusplus /Zc:preprocessor /EHsc)
  target_compile_definitions(psl_protocol PRIVATE
    _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
  if(PSL_WARNINGS_AS_ERRORS)
    target_compile_options(psl_protocol PRIVATE /WX)
  endif()
endif()

target_include_directories(psl_bridge
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(psl_bridge
  PUBLIC
    gigperformer::sdk::c)

target_compile_features(psl_bridge PUBLIC cxx_std_20)

target_compile_definitions(psl_bridge PRIVATE
  PSL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  PSL_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  PSL_VERSION_PATCH=${PROJECT_VERSION_PATCH}
  PSL_VERSION_TAG="phase0")

if(MSVC)
  target_compile_options(psl_bridge PRIVATE
    /W4 /permissive- /utf-8 /Zc:__cplusplus /Zc:preprocessor /EHsc)
  target_compile_definitions(psl_bridge PRIVATE
    _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
  if(PSL_WARNINGS_AS_ERRORS)
    target_compile_options(psl_bridge PRIVATE /WX)
  endif()
endif()

add_library(PreSonusStudioLive SHARED
  src/LibMain.h
  src/LibMain.cpp
  src/Version.h)

target_link_libraries(PreSonusStudioLive
  PRIVATE
    psl_bridge
    psl_protocol
    gigperformer::sdk::cpp)

target_include_directories(PreSonusStudioLive
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(PreSonusStudioLive PROPERTIES
  OUTPUT_NAME    PreSonusStudioLive
  PREFIX         ""                # avoid "lib" prefix on MinGW etc.
  CXX_STANDARD   20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)

# ----------------------------------------------------------------------------
#  Compiler / linker flags
# ----------------------------------------------------------------------------
if(MSVC)
  target_compile_options(PreSonusStudioLive PRIVATE
    /W4
    /permissive-
    /utf-8
    /Zc:__cplusplus
    /Zc:preprocessor
    /EHsc)

  target_compile_definitions(PreSonusStudioLive PRIVATE
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
    WIN32_LEAN_AND_MEAN)

  if(PSL_WARNINGS_AS_ERRORS)
    target_compile_options(PreSonusStudioLive PRIVATE /WX)
  endif()
endif()

# Make our project version available to the C++ code.
target_compile_definitions(PreSonusStudioLive PRIVATE
  PSL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  PSL_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  PSL_VERSION_PATCH=${PROJECT_VERSION_PATCH}
  PSL_VERSION_TAG="phase0")

# ----------------------------------------------------------------------------
#  Install: drop the DLL into the GP Extensions folder.
#  Run:  cmake --install build --config Debug --component dev
# ----------------------------------------------------------------------------
install(TARGETS PreSonusStudioLive
  COMPONENT  dev
  RUNTIME    DESTINATION "${PSL_INSTALL_DIR}"
  LIBRARY    DESTINATION "${PSL_INSTALL_DIR}")

# ----------------------------------------------------------------------------
#  Expose the target to the test tree (so tests can later link against
#  individual translation units once Phase 1 starts producing a static lib).
# ----------------------------------------------------------------------------
add_library(PreSonusStudioLive::PreSonusStudioLive ALIAS PreSonusStudioLive)
