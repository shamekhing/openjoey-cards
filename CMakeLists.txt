cmake_minimum_required(VERSION 3.21)
project(openjoey-cards VERSION 0.0.1 LANGUAGES CXX)

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
add_library(openjoey_cards_ui INTERFACE)
add_library(openjoey::cards_ui ALIAS openjoey_cards_ui)
target_compile_features(openjoey_cards_ui INTERFACE cxx_std_17)
target_include_directories(openjoey_cards_ui INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(openjoey_cards_ui INTERFACE openjoey::cards openjoey::uikit)
