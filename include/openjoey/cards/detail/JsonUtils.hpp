#pragma once
// ── Internal JSON mapping helpers for CardParser. ────────────────────────────
// NOT part of the public API: include "openjoey/cards/CardParser.hpp" instead.
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

namespace openjoey::cards::detail {

// Remote providers occasionally encode stats as "?" or strings; anything
// unparseable maps to 0.
inline int parseStatField(const nlohmann::json &j, const char *key) {
  if (!j.contains(key))
    return 0;
  const auto &v = j.at(key);
  if (v.is_null())
    return 0;
  if (v.is_number_integer())
    return static_cast<int>(v.get<int>());
  if (v.is_number_unsigned())
    return static_cast<int>(v.get<unsigned>());
  if (v.is_number_float())
    return static_cast<int>(v.get<double>());
  if (v.is_string()) {
    const std::string s = v.get<std::string>();
    if (s == "?" || s.empty())
      return 0;
    try {
      return std::stoi(s);
    } catch (...) {
      return 0;
    }
  }
  return 0;
}

inline int optIntMember(const nlohmann::json &j, const char *key, int def = 0) {
  auto it = j.find(key);
  if (it == j.end() || it->is_null() || !it->is_number())
    return def;
  if (it->is_number_unsigned())
    return static_cast<int>(it->get<uint64_t>());
  if (it->is_number_integer())
    return static_cast<int>(it->get<int64_t>());
  return static_cast<int>(it->get<double>());
}

inline std::string optStringMember(const nlohmann::json &j, const char *key,
                                   const std::string &def = {}) {
  auto it = j.find(key);
  if (it == j.end() || it->is_null() || !it->is_string())
    return def;
  return it->get<std::string>();
}

inline uint32_t optCardId(const nlohmann::json &j) {
  auto it = j.find("id");
  if (it == j.end() || it->is_null())
    return 0;
  if (it->is_number_unsigned())
    return static_cast<uint32_t>(it->get<uint64_t>());
  if (it->is_number_integer()) {
    const auto v = it->get<int64_t>();
    if (v <= 0)
      return 0;
    return static_cast<uint32_t>(v);
  }
  return 0;
}

// Remote card-data entry → Card. Only well-formed numeric/string members are
// read; nothing here throws for missing fields.
inline Card cardFromRemoteJson(const nlohmann::json &j) {
  Card c;
  c.cardId = optCardId(j);
  c.name = optStringMember(j, "name");
  c.description = optStringMember(j, "desc");
  c.imageId = c.cardId; // image filename == remote card id (see fetch scripts)

  const std::string frame = optStringMember(j, "frameType");
  c.frameType = frame; // "normal"/"effect"/"fusion"/"ritual"/... verbatim

  if (frame == "spell" || frame == "skill")
    c.type = CardType::Spell;
  else if (frame == "trap")
    c.type = CardType::Trap;
  else
    c.type = CardType::Monster;

  c.atk   = parseStatField(j, "atk");
  c.def   = parseStatField(j, "def");
  c.level = optIntMember(j, "level", 0);
  const int rank = optIntMember(j, "rank", 0);
  if (c.level == 0 && rank > 0)
    c.level = rank; // Xyz monsters carry rank, not level

  return c;
}

} // namespace openjoey::cards::detail
