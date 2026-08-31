#pragma once
#include "openjoey/cards/CardEffect.hpp"
#include "openjoey/cards/CardEnums.hpp"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace openjoey {

// ─── Card ────────────────────────────────────────────────────────────────────

// The one-and-only card class. A card's identity comes entirely from which
// Effects it subscribes to. No subclasses, ever.
//
// Fields are grouped by who owns them (see docs/API.md):
//   * Definition  — immutable per card id; populated by CardParser. What the
//                   card IS.
//   * Duel state  — mutated by openjoey-gameplay while the card is in play.
//                   Where the card is RIGHT NOW.
//   * Presentation — formatting helpers for UI layers (openjoey::cards_ui /
//                   openjoey-app). Raylib-free by design.
struct Card {
  // ── Definition: identity ───────────────────────────────────────────────────
  std::string name;
  uint32_t cardId = 0;   // remote card id, 0 = use cardId only
  uint32_t imageId = 0;  // mirrors cardId (set by CardParser)
  std::string description;

  // Card classification
  CardType type = CardType::Monster;
  // Fine-grained frame: "normal", "effect", "fusion", "ritual", "synchro",
  // "xyz", "spell", "trap" (derived from the remote data provider's type
  // string; empty until parsed). Lets consumers route Extra-Deck monsters
  // correctly.
  std::string frameType;

  // Equality is identity-by-id: two Cards are equal iff both have a non-zero
  // cardId and the ids match. An id-less Card (cardId == 0) equals nothing.
  bool operator==(const Card &other) const {
    return this->cardId != 0 && this->cardId == other.cardId;
  }

  // ── Definition: stats ──────────────────────────────────────────────────────
  int atk = 0;
  int def = 0;
  int level = 0;

  // ── Definition: subscribed effects ─────────────────────────────────────────
  // A Card's identity comes from the EffectIDs it subscribes to (see the
  // comment at the top of this struct). CardEffect lives next to this header,
  // so Card stays inside the cards layer with no dependency on a later layer.
  std::vector<CardEffect> effects;

  // ── Duel state: ownership & placement ──────────────────────────────────────
  int owner = -1;      // player index
  int controller = -1; // player index (may differ from owner)

  Location location = Location::None;
  Position position = Position::FaceDown;

  bool isFaceUp() const {
    return location == Location::Field && position == Position::FaceUp;
  }

  // Turn-state flags
  bool setThisTurn = false;
  bool placedThisTurn = false;

  // ── Duel state: attachments ────────────────────────────────────────────────
  std::vector<Card *> equippedCards; // non-owning equip attachments
  std::map<std::string, int> counters;

  // ── Presentation: queries & formatting ─────────────────────────────────────
  bool isMonster() const { return type == CardType::Monster; }
  bool isSpell() const { return type == CardType::Spell; }
  bool isTrap() const { return type == CardType::Trap; }
  // Fusion/Synchro/Xyz monsters are built in the Extra Deck (classic: only
  // Fusion exists; the others are still routed there for completeness).
  bool isExtraDeckMonster() const {
    return frameType == "fusion" || frameType == "synchro" ||
           frameType == "xyz";
  }
  std::string cardTypeTag() const {
    return this->isMonster() ? "[MON]"
           : this->isSpell() ? "[SPL]"
           : this->isTrap()  ? "[TRP]"
                             : "[UNK]";
  }
  std::string statLine() const {
    return this->isMonster() ? "Level " + std::to_string(this->level) +
                                   "  ATK " + std::to_string(this->atk) +
                                   "  DEF " + std::to_string(this->def)
                             : "";
  }
  std::string shortStat() const {
    return this->isMonster()
               ? "L" + std::to_string(this->level) + " " +
                     std::to_string(this->atk) + "/" + std::to_string(this->def)
               : "";
  }
};

} // namespace openjoey
