# openjoey-cards

Card domain for the OpenJoey split: card types, remote card-data parsing, the
card database, and card UI widgets. Header-only, C++17.

## Targets

| Target | Contents | Depends on |
|---|---|---|
| `openjoey::cards` | Domain logic — raylib-free (gameplay + its tests link ONLY this) | `openjoey::core` (nlohmann/json) |
| `openjoey::cards_ui` | Card widgets | `openjoey::cards` + `openjoey::uikit` (raylib transitively) |

## Public API

One include each:

```cpp
#include <openjoey/cards.hpp>      // domain: Card, CardEffect, CardDatabase, parser, comparators
#include <openjoey/cards_ui.hpp>   // widgets: CardGrid, CardList, CardPreview, DeckStats, Thumbnail, CardImageCache
```

Layout under `include/openjoey/`:

| Header | Owns |
|---|---|
| `cards/Card.hpp` | `struct Card` (identity + duel state + presentation helpers) |
| `cards/CardEnums.hpp` | `CardType`, `Position`, `Location` |
| `cards/CardEffect.hpp` | `EffectType`, `CardEffect` (brace-init order is a frozen contract) |
| `cards/CardParser.hpp` | `cards::parseRemoteCardJson()` → `ParseResult{cards, errors}` |
| `cards/CardDatabase.hpp` | `CardDatabase` — move-only, ODR-safe, deterministic search |
| `cards/Compare.hpp` | `cards::compare::{byName,byType,byId,byLevel,byAtk,byDef}` |
| `cards/ui/…` | Widgets (cards_ui target only) |
| `cards/detail/…` | Internal; not part of the API |

Full contract: [`docs/API.md`](docs/API.md).

## Building & testing

Standalone (bootstraps `openjoey-core` from the sibling directory; tests are
raylib-free):

```sh
cmake -S . -B build && cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
```

Via the superbuild: configure `openjoey-app` and it pulls this repo in
automatically; its `ctest` run includes `openjoey_cards_tests`.

## Migrating from the pre-0.1.0 layout

| Old include | New include |
|---|---|
| `card/Card.hpp` | `<openjoey/cards/Card.hpp>` |
| `card/CardEffect.hpp` | `<openjoey/cards/CardEffect.hpp>` |
| `card/CardParser.hpp` | `<openjoey/cards/CardParser.hpp>` |
| `card/CardDatabase.hpp` | `<openjoey/cards/CardDatabase.hpp>` |
| `card/ui/*.hpp` | `<openjoey/cards/ui/*.hpp>` |

API changes in 0.1.0: `Card::sortBy*` → `cards::compare::by*`;
`CardDatabase::GetCardsByName` → `CardDatabase::FindByName` (now deterministic);
`tryLoadRemoteCardJson` → `cards::parseRemoteCardJson` (errors returned, no
`std::cerr`); `CardDatabase` is move-only (copying would dangle its index).

Extracted from OpenJoey2@21f1d8e. Depends on: core (+ uikit for UI).
