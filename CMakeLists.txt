cmake_minimum_required(VERSION 3.21)
project(openjoey-cards VERSION 0.1.0 LANGUAGES CXX)

# ── Standalone support: bootstrap sibling deps unless a superbuild did ───────
if(NOT TARGET openjoey_core)
  set(OPENJOEY_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH
      "Directory containing the sibling openjoey-* repos")
  add_subdirectory("${OPENJOEY_DEPS_DIR}/openjoey-core" ${CMAKE_BINARY_DIR}/openjoey-core)
endif()

# ── openjoey_cards: card domain logic. Raylib-free on purpose: gameplay and
#    its tests link ONLY this target and must never pull in raylib.
add_library(openjoey_cards INTERFACE)
add_library(openjoey::cards ALIAS openjoey_cards)
target_compile_features(openjoey_cards INTERFACE cxx_std_17)
target_include_directories(openjoey_cards INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(openjoey_cards INTERFACE openjoey::core)  # nlohmann via core

# ── openjoey_cards_ui: card widgets. raylib arrives transitively via uikit.
#    (uikit is intentionally NOT bootstrapped here: standalone card-domain
#    builds stay raylib-free, and nothing outside a UI consumer links cards_ui.)
add_library(openjoey_cards_ui INTERFACE)
add_library(openjoey::cards_ui ALIAS openjoey_cards_ui)
target_compile_features(openjoey_cards_ui INTERFACE cxx_std_17)
target_include_directories(openjoey_cards_ui INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(openjoey_cards_ui INTERFACE openjoey::cards openjoey::uikit)

# ── Tests: raylib-free (link cards + core only, never cards_ui) ─────────────
option(OPENJOEY_CARDS_BUILD_TESTS "Build openjoey-cards tests" ON)
if(OPENJOEY_CARDS_BUILD_TESTS)
  enable_testing()
  add_executable(openjoey_cards_tests tests/tests.cpp)
  target_include_directories(openjoey_cards_tests PRIVATE tests/catch2)
  target_compile_options(openjoey_cards_tests PRIVATE -Wall -Wextra)
  target_link_libraries(openjoey_cards_tests PRIVATE openjoey::cards openjoey::core)
  add_test(NAME openjoey_cards_tests COMMAND openjoey_cards_tests)
endif()
