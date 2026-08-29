#pragma once
// ── openjoey::cards — public API umbrella (raylib-free) ──────────────────────
// One include for the whole card domain: types, parsing, database, comparators.
// Widget APIs (CardGrid, CardList, ...) live in <openjoey/cards_ui.hpp> and
// pull in raylib — never include that header from engine code.
//
// Dependency chain: core -> uikit -> cards -> gameplay -> app.
//   openjoey::cards      — depends on openjoey::core only (nlohmann/json via core)
//   openjoey::cards_ui   — additionally depends on openjoey::uikit (+ raylib)

#include "openjoey/cards/CardEnums.hpp"
#include "openjoey/cards/CardEffect.hpp"
#include "openjoey/cards/Card.hpp"
#include "openjoey/cards/Compare.hpp"
#include "openjoey/cards/CardParser.hpp"
#include "openjoey/cards/CardDatabase.hpp"
