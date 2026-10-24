# Idol Natsumi – Display improvements

- Repo: TME520/Idol-Natsumi, branch `improve_display_mgmt` (based on `update-19-2` after PR #107)
- Goal: faster screen refresh and less flicker on the M5Stack Cardputer (240x135, no PSRAM).
- One code per screen (or per group of states sharing the exact same background and draw path).
  Each code is one commit whose message starts with the code.

## How it works

- `frameCanvas`: one persistent 240x135 16-bit off-screen canvas (64,800 bytes), created at the
  very start of `setup()` by `createCanvases()`. If it can't be allocated, everything falls back
  to drawing straight on the display, as before.
- Background cache (ND-00): a screen opted in through `backgroundCacheEnabledFor()` decodes its
  background PNG into `frameCanvas` once. Every later L0 redraw (toast expiring, menu or overlay
  closing, layer cascade) is then a single `pushSprite(0, 0)` instead of a PNG decode.
- Cache key: the background file path (`currentBackgroundPath`, set by `preloadImage()`, cleared
  by `unloadImage()`). `frameCanvasBackgroundPath` says what the canvas holds; empty = nothing.
  Two screens with the same background file share the decoded image.
- Invalidation: `invalidateBackgroundCache()` is called by everything else that draws into
  `frameCanvas` (swim playfield, travel ride). A different path, a missing image or a PNG that is
  not 240x135 also bypasses or refreshes the cache.
- Only fully opaque 240x135 backgrounds drawn at (0, 0) by `drawBackground()` should be opted in.
- PNG bytes are kept in RAM for now (nothing frees them after decoding); freeing them per screen
  is a later, separate step.

## Code index

| Code | Screen / scope | Background | Status |
|---|---|---|---|
| ND-T1 | Travel ride (`manageTravel()`): tile decoded once, bike drawn through a 90x45 strip canvas | `tile_*.png` | Done (a7956da, delay set to 80 in 03927ac) |
| ND-00 | Shared infrastructure: background cache, path key, `invalidateBackgroundCache()`, opt-in switch, swim/travel invalidation | – | Done (81ca903) |
| ND-01 | HOME_LOOP (home room) | `lounge.png` | Done (12ec5dc) |
| ND-02 | FOOD_MENU | `kitchen.png` | Done (8bfd126) |
| ND-03 | HEALTH_MENU | `bathroom.png` | Done (8f3471f) |
| ND-04 | REST_MENU | `bedroom.png` | Done (603d8aa) |
| ND-05 | TRAIN_MENU | `map_training.png` | Done (dce252c) |
| ND-06 | COMP_MENU | `competition.png` | Done (6452707) |
| ND-07 | EVENTS_MENU | `cityscape_bg.png` | Done (63a3c5e) |
| ND-08 | GARDEN_LOOP / GARDEN_PLANT / GARDEN_WATER / GARDEN_PICK / GARDEN_CLEANUP / GARDEN_MENU | `garden_bg.png` | Done (f02015d) |
| ND-09 | MATSURI_MENU / MATSURI_MENU2 / MATSURI_MENU3 | `matsuri_menu0x.png` | Done (90eab7e) |
| ND-10a | Neko Cafe (NEKO_CAFE..NEKO_CAFE14) | `neko_cafe_*.png` | Done (1ff3216) |
| ND-10b | Onsen (HEALTH_ONSEN) | `onsen_*yo.png` | Done (9625b9d) |
| ND-10c | Library (TRAIN_LIBRARY) | `natsumi_*_library.png` | Open |
| ND-10d | Restaurant (FOOD_REST..FOOD_REST8) | `restaurant_bg.png` / teishoku / steps | Open |
| ND-10e | Conbimart (FOOD_CONBINI..FOOD_CONBINI3) | `conbimart_*.png` | Open |
| ND-10f | Flowers market (FLOWERS_MARKET..FLOWERS_MARKET7) | `flower_market_*.png` | Open |
| ND-10g | Doctor (HEALTH_DOCTOR..HEALTH_DOCTOR6) | `doctors_office_bg.png` / steps | Open |
| ND-10h | Temple (HEALTH_TEMPLE..HEALTH_TEMPLE6) | `temple_bg.png` / priest steps | Open |
| ND-11 | Title, intro and card screens (M5_SCREEN, TITLE_SCREEN, INTRO...) | various | Open |
| ND-12 | Business fronts shown after a ride (`displayBusinessFront()`) | `*_front.png` | Open |
| ND-13 | Free background PNG bytes after decoding (per screen, needs reload-on-miss) | – | Open |
| ND-14 | Retarget layers 1-5 (character, debug, toast, menu, overlay) to `frameCanvas` | – | Open |

Screens drawn in `manageGame()` (mini-games: swim, competition, dance, gym, run, Garapon...) do not
use `drawBackground()`; the swim game already draws through `frameCanvas`.
