# openjoey-cards — API contract

Version 0.1.0 · header-only · C++17 · GPL-3.0

This document is the stable contract between `openjoey-cards` and its
consumers (`openjoey-gameplay`, `openjoey-app`, and any future repo under the
OpenJoey2 umbrella). Anything listed here will not change without a version
bump and a migration note in the README.

## 1. Targets & layering

```
openjoey-core -> openjoey-uikit -> openjoey-cards -> openjoey-gameplay -> openjoey-app
```

| Target | Raylib | Consumers |
|---|---|---|
| `openjoey::cards` | **never** | gameplay, app, tests |
| `openjoey::cards_ui` | yes (via uikit) | app only |

**Invariant:** `openjoey::cards` must stay raylib-free — gameplay and headless
test binaries link only this target.

## 2. Entry points

```cpp
#include <openjoey/cards.hpp>      // whole domain API
#include <openjoey/cards_ui.hpp>   // widgets (UI consumers only)
```

## 3. Types

### `openjoey::Card` (`cards/Card.hpp`)

One flat aggregate, grouped by ownership:

* **Definition** (what a card *is*; written once by the parser):
  `name`, `cardId`, `imageId`, `description`, `type`, `atk`, `def`, `level`,
  `effects`.
* **Duel state** (where a card is *now*; mutated by openjoey-gameplay):
  `owner`, `controller`, `location`, `position`, `setThisTurn`,
  `placedThisTurn`, `equippedCards`, `counters`.
* **Presentation** (raylib-free formatting helpers for UI layers):
  `isMonster()/isSpell()/isTrap()`, `cardTypeTag()`, `statLine()`,
  `shortStat()`, `isFaceUp()`.

Contracts:

* `operator==` is **identity by id**: true iff both cards have a non-zero
  `cardId` and the ids match. An id-less card equals nothing.
* `imageId == cardId` for every card produced by the parser.

### `openjoey::CardEffect` (`cards/CardEffect.hpp`)

```cpp
struct CardEffect { EffectID id; EffectType timing; uint8_t speed; int costLP; };
```

**Frozen brace-init contract:** consumers (e.g. gameplay tests) initialize it
positionally — `{EffectID::Move_Draw, EffectType::Trigger, 2, 100}`. Field
order must not change without a coordinated version bump. Defaults:
`None / Ignition / 1 / 0`. Pinned by `openjoey_cards_tests` ("[effect]").

### `openjoey::cards::ParseError` / `ParseResult` (`cards/CardParser.hpp`)

```cpp
ParseResult cards::parseRemoteCardJson(const std::string &content);
```

* Input: a remote card-data provider payload (`{"data": [ ... ]}`).
* Never throws, never writes to stderr; problems land in `result.errors`.
* Cards are de-duplicated by `cardId` (first entry wins, input order kept).
* Entries without a usable id are skipped; nameless cards get `"Card <id>"`.
* `"?"`/string stats map to `0`; Xyz `rank` falls back into `level`.
* `ok()` is true iff at least one card was parsed.

### `openjoey::CardDatabase` (`cards/CardDatabase.hpp`)

* Owns its cards; lookups return **non-owning** pointers valid until the
  database is destroyed, moved from, or reloaded.
* **Move-only** (copy deleted — copying would dangle the index; move keeps
  element addresses).
* All members are defined in-class → ODR-safe in multi-TU programs.
* `LoadFromFile(path)` / `LoadFromString(content)`: on any failure the
  database is left **empty** and `false` is returned.
* `GetCardById` / `GetCardByName` → `nullptr` when absent.
* `FindByName(needle)` → substring match, results **sorted by cardId
  ascending** (deterministic regardless of hash order); empty needle matches
  all.
* `GetAllCards()`, `size()`, `empty()`, `Clear()`.

### `openjoey::cards::compare::*` (`cards/Compare.hpp`)

`byName`, `byType`, `byId`, `byLevel`, `byAtk`, `byDef` — all strict weak
orderings with a name tiebreak (except `byId`), suitable for `std::sort`.

## 4. UI widgets (`cards_ui` target)

`Thumbnail`, `CardList`, `CardGrid`, `DeckStats` — immediate-mode draw helpers
taking `const Card&`/`CardImageCache&`. `CardImageCache` owns a background
download thread (main-thread `Get()`/`PollAndLoad()` per frame). `fitText()`
ellipsizes a string to a pixel width. StyleSheet constants come from
`openjoey::uikit`.

`CardPreview` (`cards/ui/CardPreview.hpp`, v0.2) — the one *stateful* widget:
portrait image, type/stat line, and a scrollable description with
`SetCard()`/`SetCardBack()`/`scroll()`; call `SetCard()` each frame before
`Draw(bounds, cache)`. Used by the app's duel and image-cache test screens.
Note: `openjoey::ui` widgets that are card-domain live here (`cards/ui/`),
not in the app — app-local layout stays in `openjoey-app/include/ui/`.

## 5. Planned evolution (v0.2)

The def/state split: `Card` currently mixes definition and duel-state fields.
Planned shape:

```cpp
struct CardDef    { ...identity + stats + effects... };   // parser/db payload
struct CardState  { const CardDef* def; owner/controller/location/... };
```

`Position`/`Location` (now in `cards/CardEnums.hpp`) move to duel state.
Consumers affected: gameplay zones (`Card*` → state carrying a def pointer)
and app UI (read accessors change). This will land with coordinated updates in
`openjoey-gameplay` and `openjoey-app` — not silently.

## 6. Versioning

Semver-ish: patch = internal fixes, minor = additive API, major = breaking
changes with a README migration table (see "Migrating from the pre-0.1.0
layout").
