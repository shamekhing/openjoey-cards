#pragma once
#include <cstdint>

namespace openjoey {

// Card classification: one of the three card frame families this engine models.
enum class CardType : uint8_t { Monster, Spell, Trap };

// ── Duel-runtime placement enums ─────────────────────────────────────────────
// Position and Location describe where a card sits *during a duel*, not what
// it *is*. They live here because Card carries them as fields today; they are
// scheduled to move behind a CardDef/CardState split in a future revision
// (see docs/API.md "Planned evolution"). New code should treat them as
// duel-state, not card-identity.
enum class Position : uint8_t { FaceUp, FaceDown };

enum class Location : uint8_t {
  Hand,
  Deck,
  ExtraDeck,
  Field,
  Graveyard,
  Banished,
  None
};

} // namespace openjoey
