// openjoey-cards unit tests: CardDatabase, CardParser, comparators, CardEffect.
// Raylib-free by design: links openjoey::cards + openjoey::core only.
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "openjoey/cards.hpp"
#include "openjoey/EffectID.hpp"

#include <filesystem>
#include <string>
#include <type_traits>

using namespace openjoey;

// Tests run against a deterministic 6-card fixture committed next to this file,
// NOT the full 14k-card database in openjoey-content. Resolve it relative to
// this source file so it works regardless of CTest's working directory.
static std::string cardsPath() {
    std::filesystem::path p(__FILE__);
    return (p.parent_path() / "fixtures" / "cards.json").string();
}

// --- CardDatabase ------------------------------------------------------------

TEST_CASE("CardDatabase loads the starter cards.json", "[db]") {
    CardDatabase db;
    REQUIRE(db.LoadFromFile(cardsPath()));
    REQUIRE(db.size() == 6);
    REQUIRE_FALSE(db.empty());

    SECTION("lookups by id and name") {
        REQUIRE(db.GetCardById(89631139) != nullptr);          // Blue-Eyes
        REQUIRE(db.GetCardById(46986414) != nullptr);          // Dark Magician
        REQUIRE(db.GetCardByName("Kuriboh") != nullptr);
        REQUIRE(db.GetCardById(99999999) == nullptr);          // unknown id
        REQUIRE(db.GetCardByName("No Such Card") == nullptr);  // unknown name
    }

    SECTION("parsed fields match the JSON") {
        const Card* be = db.GetCardById(89631139);
        REQUIRE(be->name == "Blue-Eyes White Dragon");
        REQUIRE(be->isMonster());
        REQUIRE(be->atk == 3000);
        REQUIRE(be->def == 2500);
        REQUIRE(be->level == 8);

        const Card* ra = db.GetCardById(12580477);
        REQUIRE(ra->isSpell());
        REQUIRE(ra->atk == 0);

        const Card* mf = db.GetCardById(44095762);
        REQUIRE(mf->isTrap());

        // imageId mirrors cardId (CardParser sets it)
        REQUIRE(be->imageId == be->cardId);
        REQUIRE(be->imageId == 89631139);

        // frameType carries the remote-provider frame verbatim (drives Extra-Deck
        // routing and other subtype behavior downstream).
        REQUIRE(be->frameType == "normal");
        REQUIRE(mf->frameType == "trap");
        REQUIRE(ra->frameType == "spell");
    }
}

TEST_CASE("FindByName substring search is deterministic", "[db]") {
    CardDatabase db;
    REQUIRE(db.LoadFromFile(cardsPath()));

    auto hits = db.FindByName("Magic");
    REQUIRE(hits.size() == 1);
    REQUIRE(hits.front()->cardId == 46986414);

    // Results are sorted by cardId ascending regardless of hash order.
    auto all = db.FindByName(""); // empty needle matches every card
    REQUIRE(all.size() == db.size());
    for (std::size_t i = 1; i < all.size(); ++i)
        REQUIRE(all[i - 1]->cardId < all[i]->cardId);
}

TEST_CASE("LoadFromString parses an inline remote card-data payload", "[db][parser]") {
    CardDatabase db;
    REQUIRE(db.LoadFromString(R"({"data":[
        {"id":111,"name":"Alpha","desc":"","frameType":"normal","atk":100,"def":50,"level":2},
        {"id":222,"name":"Beta","desc":"","frameType":"spell"},
        {"id":222,"name":"Beta dupe","desc":"","frameType":"spell"},
        {"name":"NoId","desc":"","frameType":"trap"},
        {"id":"not-a-number"}
    ]})"));

    REQUIRE(db.size() == 2);
    REQUIRE(db.GetCardById(111)->atk == 100);
    REQUIRE(db.GetCardById(111)->isMonster());
    REQUIRE(db.GetCardById(222)->isSpell());
    REQUIRE(db.GetCardByName("Beta")->name == "Beta");       // duplicate id: first wins
    REQUIRE(db.GetCardByName("Beta dupe") == nullptr);
    REQUIRE(db.GetCardByName("NoId") == nullptr);            // id-less entry dropped
}

TEST_CASE("LoadFromFile missing file leaves the db empty", "[db]") {
    CardDatabase db;
    REQUIRE_FALSE(db.LoadFromFile("/nonexistent/path/cards.json"));
    REQUIRE(db.empty());
    REQUIRE(db.GetCardById(89631139) == nullptr);
}

TEST_CASE("Clear empties the database", "[db]") {
    CardDatabase db;
    REQUIRE(db.LoadFromFile(cardsPath()));
    db.Clear();
    REQUIRE(db.empty());
    REQUIRE(db.size() == 0);
    REQUIRE(db.GetCardById(89631139) == nullptr);
}

TEST_CASE("Database is movable but not copyable", "[db]") {
    STATIC_REQUIRE_FALSE(std::is_copy_constructible<CardDatabase>::value);
    STATIC_REQUIRE(std::is_move_constructible<CardDatabase>::value);

    CardDatabase source;
    REQUIRE(source.LoadFromFile(cardsPath()));
    const Card* be = source.GetCardById(89631139); // pointer into source storage

    CardDatabase moved(std::move(source));
    REQUIRE(moved.size() == 6);
    REQUIRE(moved.GetCardById(89631139) == be); // vector move keeps element addresses
}

// --- CardParser --------------------------------------------------------------

TEST_CASE("Parser rejects malformed payloads without throwing", "[parser]") {
    REQUIRE_FALSE(cards::parseRemoteCardJson("").ok());
    REQUIRE_FALSE(cards::parseRemoteCardJson("not json at all").ok());
    REQUIRE_FALSE(cards::parseRemoteCardJson(R"({"data": 42})").ok());
    REQUIRE_FALSE(cards::parseRemoteCardJson(R"([1,2,3])").ok());
}

TEST_CASE("Parser maps stat edge cases to sane values", "[parser]") {
    SECTION("'?' and string stats parse to 0") {
        auto r = cards::parseRemoteCardJson(
            R"({"data":[{"id":333,"name":"Mystic","frameType":"normal","atk":"?","def":"?","level":4}]})");
        REQUIRE(r.ok());
        REQUIRE(r.errors.empty());
        REQUIRE(r.cards.size() == 1);
        REQUIRE(r.cards[0].atk == 0);
        REQUIRE(r.cards[0].def == 0);
        REQUIRE(r.cards[0].level == 4);
    }
    SECTION("missing name is synthesized from the id") {
        auto r = cards::parseRemoteCardJson(R"({"data":[{"id":444,"frameType":"trap"}]})");
        REQUIRE(r.ok());
        REQUIRE(r.cards[0].name == "Card 444");
        REQUIRE(r.cards[0].isTrap());
    }
    SECTION("rank falls back into level for Xyz frames") {
        auto r = cards::parseRemoteCardJson(
            R"({"data":[{"id":555,"name":"Xyz","frameType":"xyz","rank":5}]})");
        REQUIRE(r.ok());
        REQUIRE(r.cards[0].level == 5);
    }
    SECTION("non-object entries are skipped, not fatal") {
        auto r = cards::parseRemoteCardJson(
            R"({"data":[42,{"id":666,"name":"Ok","frameType":"spell"}]})");
        REQUIRE(r.ok());
        REQUIRE(r.cards.size() == 1);
        REQUIRE_FALSE(r.errors.empty());
    }
}

// --- Comparators (Compare.hpp) -----------------------------------------------

TEST_CASE("Comparators are strict weak orderings", "[compare]") {
    Card a, b;
    a.cardId = 1; b.cardId = 2;
    a.name = "Zap"; b.name = "Apple";
    a.atk = 100; b.atk = 200;
    a.level = 3; b.level = 4;
    a.type = CardType::Monster;

    using openjoey::cards::compare::byAtk;
    using openjoey::cards::compare::byId;
    using openjoey::cards::compare::byLevel;
    using openjoey::cards::compare::byName;

    SECTION("byName: Apple before Zap") {
        REQUIRE_FALSE(byName(a, b));
        REQUIRE(byName(b, a));
    }
    SECTION("byAtk: 100 before 200") {
        REQUIRE(byAtk(a, b));
        REQUIRE_FALSE(byAtk(b, a));
    }
    SECTION("byLevel") {
        REQUIRE(byLevel(a, b));
        REQUIRE_FALSE(byLevel(b, a));
    }
    SECTION("byId") {
        REQUIRE(byId(a, b));
        REQUIRE_FALSE(byId(b, a));
    }
}

// --- CardEffect (brace-init contract) ----------------------------------------

TEST_CASE("CardEffect aggregate field order is stable", "[effect]") {
    // openjoey-gameplay brace-initializes CardEffect positionally; this test
    // pins the field order as an explicit API contract.
    CardEffect e{EffectID::Move_Draw, EffectType::Trigger, 2, 100};
    REQUIRE(e.id == EffectID::Move_Draw);
    REQUIRE(e.timing == EffectType::Trigger);
    REQUIRE(e.speed == 2);
    REQUIRE(e.costLP == 100);

    CardEffect def{}; // defaults: None / Ignition / speed 1 / no LP cost
    REQUIRE(def.id == EffectID::None);
    REQUIRE(def.timing == EffectType::Ignition);
    REQUIRE(def.speed == 1);
    REQUIRE(def.costLP == 0);
}

