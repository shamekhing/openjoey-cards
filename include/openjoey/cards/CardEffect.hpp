#pragma once
#include "openjoey/EffectID.hpp"
#include <cstdint>

namespace openjoey {

// ── Effect timing (Rulebook p.9 "Effect Monsters") ───────────────────────────
enum class EffectType : uint8_t {
    Ignition,     // activate during your Main Phase        (Spell Speed 1)
    Trigger,      // "when X happens" (includes Flip)
    Quick,        // Spell Speed 2 or higher
    Continuous,   // active while face-up on the field
    Cost,         // paid before activation (Tribute/Discard/Pay-LP...)
};

// ── A single subscribed effect on a Card ─────────────────────────────────────
// A Card's identity is the set of EffectIDs it carries (Card.hpp). The
// resolver in openjoey-gameplay's field/EffectResolver.hpp turns each id into
// concrete zone moves.
//
// ABI/API CONTRACT: CardEffect is brace-initialized positionally by external
// repos (e.g. openjoey-gameplay tests: `CardEffect{id, timing, speed, costLP}`).
// Do NOT reorder, insert, or remove fields without a coordinated version bump.
struct CardEffect {
    EffectID   id     = EffectID::None;
    EffectType timing = EffectType::Ignition;
    uint8_t    speed  = 1;     // Spell Speed: 1 / 2 / 3
    int        costLP = 0;     // LP cost when timing == Cost (0 = none)
};

} // namespace openjoey
