#pragma once
#include "openjoey/cards/Card.hpp"

namespace openjoey::cards::compare {

// ── Ordering comparators for card collections ────────────────────────────────
// Every comparator is a strict weak ordering, suitable for std::sort. Each
// falls back to the name so the resulting order is total (stable output).
// Previously these lived on Card as static members; sorting is a UI concern
// and now lives here.

inline bool byName(const Card &a, const Card &b) { return a.name < b.name; }

inline bool byType(const Card &a, const Card &b) {
  return a.type != b.type ? static_cast<int>(a.type) < static_cast<int>(b.type)
                          : byName(a, b);
}

inline bool byId(const Card &a, const Card &b) { return a.cardId < b.cardId; }

inline bool byLevel(const Card &a, const Card &b) {
  return a.level != b.level ? a.level < b.level : byName(a, b);
}

inline bool byAtk(const Card &a, const Card &b) {
  return a.atk != b.atk ? a.atk < b.atk : byName(a, b);
}

inline bool byDef(const Card &a, const Card &b) {
  return a.def != b.def ? a.def < b.def : byName(a, b);
}

} // namespace openjoey::cards::compare
