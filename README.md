# openjoey-cards

Card domain for the OpenJoey split: `card/{Card, CardEffect, CardParser,
CardDatabase}.hpp` plus card widgets in `card/ui/`.

Two targets:
- `openjoey::cards` — domain logic, raylib-free (gameplay + tests link this)
- `openjoey::cards_ui` — card widgets, links openjoey::uikit (raylib transitively)

Extracted from OpenJoey2@21f1d8e (CardPreview.hpp moved to openjoey-app: it was
the only card/ui header reaching into zone/). Depends on: core (+ uikit for UI).
