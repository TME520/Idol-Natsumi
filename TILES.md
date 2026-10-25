# Idol Natsumi – Tiles and activities

- Source: `prepareTravel()`, `showsBusinessFrontOnArrival()`, and `displayBusinessFront()` in `idol_natsumi/idol_natsumi.ino`.
- Tiles are strings (`currentTile`, `targetTile`). There is no location enum. The travel comment names seven: `HOME`, `CITY`, `OUTSKIRTS`, `COUNTRYSIDE`, `HANAMORI`, `OSAKA`, `TOKYO`.
- `prepareTravel()` runs on every `changeState()`. A state that is not in its switch leaves `currentTile` where it already is.
- Beat-by-beat state list: [FLOW.md](FLOW.md). This file is the map: which activity sits on which tile, and when the bike and the front flash play.

## Tiles

The ride decodes one PNG into the background, then `manageTravel()` scrolls the bike (`/idolnat/sprites/natsumi_bicycle1-80x45.png` and `natsumi_bicycle2-80x45.png`).

| Tile | Travel PNG | States that set this tile |
|---|---|---|
| `HOME` | `/idolnat/screens/tile_home.png` | `HOME_LOOP`, `FOOD_MENU`, `FOOD_COOK`, `FOOD_ORDER`, `HEALTH_WASH`, `REST_MEDITATE`, `REST_SLEEP`, `GARDEN_LOOP` |
| `CITY` | `/idolnat/screens/tile_city.png` | `FOOD_REST`, `NEKO_CAFE`, `TRAIN_SING`, `TRAIN_SWIM`, `TRAIN_LIBRARY`, `FLOWERS_MARKET`, `HEALTH_DOCTOR`, `COMP_LOCAL` |
| `OUTSKIRTS` | `/idolnat/screens/tile_outskirts.png` | `FOOD_CONBINI`, `TRAIN_DANCE`, `TRAIN_GYM`, `MATSURI_TITLE` |
| `COUNTRYSIDE` | `/idolnat/screens/tile_countryside.png` | `TRAIN_RUN`, `HEALTH_TEMPLE`, `HEALTH_ONSEN` |
| `HANAMORI` | `/idolnat/screens/tile_hanamori.png` | `COMP_DEPT` |
| `OSAKA` | `/idolnat/screens/tile_osaka.png` | `COMP_REG` |
| `TOKYO` | `/idolnat/screens/tile_tokyo.png` | `COMP_NAT` |

`currentTile` and `targetTile` start as `"HOME"`. `resetGameState()` sets both back to `"HOME"`. New Game calls that reset, and so does Continue when no save file exists. The SD save does not store the map tile (`garden_tiles` is the plot). Continue with a save still opens `HOME_LOOP` (`continueStateLoaded` is never set), so play resumes at home.

## Travel rules

`prepareTravel()` picks `targetTile` from `currentState`, then compares it with `currentTile`.

| Situation | Bike | Energy | Front |
|---|---|---|---|
| Destination tile differs, and the destination is `HOME` | Ride plays, then `currentTile` becomes `HOME` | 1 Energy if Energy is above 0. The ride still plays at 0; Energy does not go below 0. | Home front (`natsumi_home_front.png`) after the ride |
| Destination tile differs, and the destination is any other tile | Ride plays, then `currentTile` becomes that tile | No Energy cost | Destination front after the ride, except Conbimart and Neko Cafe |
| Destination tile is already `currentTile` | No ride | No Energy cost | Front only for the entry states in `showsBusinessFrontOnArrival()` |

`FOOD_MENU`, `FOOD_COOK`, and `FOOD_ORDER` are `HOME` states, together with `HOME_LOOP`, `HEALTH_WASH`, `REST_MEDITATE`, `REST_SLEEP`, and `GARDEN_LOOP`. The restaurant (after the meal) and the Conbimart cashier return to `latestMenu`. The food menu sets that to `FOOD_MENU`, so the return rides home and pays the Energy cost once. ESC from the cafe drink menu takes the same path. The cafe seat loops to `NEKO_CAFE2` and stays on `CITY`.

Later beats of an activity (`FOOD_REST2`, `TRAIN_SING2`, `COMP_LOCAL2`, a matsuri stall, and so on) are not in the switch. They keep the tile their entry state already set, and they are not in `showsBusinessFrontOnArrival()`, so the front does not flash again.

Menus that are not in the switch stay on the current tile until a destination state runs. Opened from home, that tile is `HOME`: `HEALTH_MENU`, `REST_MENU`, `TRAIN_MENU`, `COMP_MENU`, `EVENTS_MENU`. `GARDEN_MENU` is the tool menu on the plot (Enter on `GARDEN_LOOP`), and its tools do not travel. `FOOD_MENU` is the exception among menus: entering it sets `HOME` immediately.

Returning to `TRAIN_MENU`, `HEALTH_MENU`, `COMP_MENU`, or `EVENTS_MENU` after an activity (via `latestMenu`) also leaves `currentTile` on the place just visited. The next destination in the switch, or `HOME_LOOP` from ESC, is what moves the tile.

## Activities by tile

Paths are the home-menu number keys and the Enter row. The same items are in `drawMenu()`. A tired check (`energy < 1`, toast "Natsumi is too tired") blocks Food, Training, Competition, Health, Garden, and Events. Rest, Stats, Inventory, and Cards stay available. Training, Competition, Health, and Events also wait if a food delivery is pending.

### HOME

| Activity | Entry state | Access |
|---|---|---|
| Home room | `HOME_LOOP` | New Game, Continue, and ESC / Tab from the menus. Home menu: Stats, Inventory, Food, Training, Competition, Health, Rest, Garden, Events, Cards. Space opens Challenges. |
| Idle slideshow | `IDLE_HOME` | Quiet wait on `HOME_LOOP` with the menu closed and no visitor to play. A key returns to `HOME_LOOP`. |
| Visitors | `DOOR_KNOCK` | Same wait: birthday, a competition win, a perfect lesson, or grandma when hunger is empty. See [FLOW.md](FLOW.md) (Visitors / Emily RPS). |
| Emily, rock-paper-scissors | `PFC_GAME` | Emily (`visitor == 5`) leaves `DOOR_KNOCK6` into `PFC_GAME`. |
| Stats, Inventory, Cards | `STATS_SCREEN`, `INVENTORY_SCREEN`, `CARDS_SCREEN` | Home menu 0, 1, and 9. These states do not set a tile. Cards returns to `HOME_LOOP`. |
| Food menu, cooking, Orderibi | `FOOD_MENU`, `FOOD_COOK`, `FOOD_ORDER` | Home menu 2 opens `FOOD_MENU` (sets `HOME`). Cooking is food 0. Order is food 2, age above 10. Both set `HOME`. |
| Shower | `HEALTH_WASH` | Health menu 0. Sets `HOME`. |
| Meditate, sleep | `REST_MEDITATE`, `REST_SLEEP` | Rest menu 0 and 1. Both set `HOME`. Sleep requires Energy below 4. |
| Garden plot | `GARDEN_LOOP` | Home menu 7. Sets `HOME`. Enter on a plot cell opens `GARDEN_MENU`. |

`IDLE_HOME`, the door, and Emily's game are not in the `prepareTravel()` switch. Only `HOME_LOOP` starts them, after that state has already set `HOME`.

### CITY

| Activity | Entry state | Access |
|---|---|---|
| Restaurant | `FOOD_REST` | Food menu 1. Age above 14. Younger, or a pending delivery, does not travel. |
| Neko Cafe | `NEKO_CAFE` | Food menu 4. |
| Singing | `TRAIN_SING` | Training menu 0. |
| Swimming | `TRAIN_SWIM` | Training menu 2. |
| Library | `TRAIN_LIBRARY` | Training menu 5. |
| Flower market | `FLOWERS_MARKET` | Training menu 6 (Market). No flowers: toast and `HOME_LOOP`. |
| Doctor | `HEALTH_DOCTOR` | Health menu 1. The advice returns to `HOME_LOOP`. |
| Local competition | `COMP_LOCAL` | Competition menu 0, when entry is open and `natsumi.competition == 3`. |

### OUTSKIRTS

| Activity | Entry state | Access |
|---|---|---|
| Conbimart | `FOOD_CONBINI` | Food menu 3. |
| Dance | `TRAIN_DANCE` | Training menu 1. |
| Gym | `TRAIN_GYM` | Training menu 3. |
| Matsuri | `MATSURI_TITLE` | Events menu 0. The banner advances to the stall menu. |

### COUNTRYSIDE

| Activity | Entry state | Access |
|---|---|---|
| Run | `TRAIN_RUN` | Training menu 4. |
| Temple | `HEALTH_TEMPLE` | Health menu 2. The closing line returns to `HOME_LOOP`. |
| Onsen | `HEALTH_ONSEN` | Health menu 3. |

### HANAMORI, OSAKA, TOKYO

These three tiles are competition only.

| Tile | Entry state | Access |
|---|---|---|
| `HANAMORI` | `COMP_DEPT` | Competition menu 1 (Departmental), when entry is open and `natsumi.competition == 7`. |
| `OSAKA` | `COMP_REG` | Competition menu 2 (Regional), when entry is open and `natsumi.competition == 11`. |
| `TOKYO` | `COMP_NAT` | Competition menu 3 (National), when entry is open and `natsumi.competition == 15`. |

Entry is open when hunger, hygiene, energy, performance, fitness, culture, and grace are all full (`isCompetitionEnabled()`). Otherwise the menu opens `COMP_EXPLAIN`, which returns to `HOME_LOOP`. A tier that is still locked, or already finished, toasts and the number keys return to `HOME_LOOP`.

## States that do not set a tile

These are outside the `prepareTravel()` switch. They keep `currentTile`. Chains that follow a trip (restaurant dishes, training minigames, competition beats, matsuri stalls, the cafe after the front, garden tools, pay inside a shop) stay on the tile the entry state set. The groups below are global: they are not a place on the map.

| Group | States | Where it sits |
|---|---|---|
| Boot and title | `VERSION_SCREEN`, `M5_SCREEN`, `MOTTO_SCREEN`, `TITLE_SCREEN`, `TITLE_SCREEN2`, `NEW_GAME`, `CONTINUE_GAME` | Before play. `TITLE_SCREEN` is unused. |
| Intro | `INTRO` .. `INTRO7` | Title menu, item 2. Returns to `TITLE_SCREEN2`. Does not start a save. |
| Pay | `PAY_SCREEN`, `PAY_SCREEN2`, `PAY_SCREEN3` | Shared register for the restaurant, Conbimart, Orderibi, and the cafe, then `returnTo`. |
| Result card | `ACTION_OUTCOME` | Shared "+1" / "MAX" card. The return depends on the activity. See [FLOW.md](FLOW.md). |
| Challenges | `CHALLENGES_SCREEN`, `CHALLENGE_DONE`, `CHALLENGE_DONE2`, `CHALLENGE_DONE3` | Space on the home menu, or Events menu 2. ESC uses `challengesReturnState`. |
| After a lesson | `TRAIN_STATUS` | After the sing, dance, swim, gym, or run debrief (`ACTION_OUTCOME` opens it). |
| Competition gates | `COMP_EXPLAIN`, `COMP_FAILED`, `COMP_COMPLETE` | Rules when entry is closed; failure flash then home; success flash into that tier's debrief. `COMP_STATUS` is unused. |
| Ticket award | `MATSURI_TICKETS`, `MATSURI_TICKETS2` | After some results. `MATSURI_TICKETS3` is unused. |
| Jobs, Festivals | No state | Events menu 1 and 3. Toast "Jobs: coming soon!" or "Festivals: coming soon!", then `HOME_LOOP` (that state applies the home tile rule). |
| Unused travel states | `BUSINESS_FRONT`, `TRAVEL_SCREEN` | Nothing enters them. An idle handler on `BUSINESS_FRONT` would leave to `latestMenu`. A game handler on `TRAVEL_SCREEN` would call `manageTravel()`. The live ride and the live front are the direct calls inside `prepareTravel()`. |

## Front flash

`showsBusinessFrontOnArrival()` is the same-tile list. `displayBusinessFront()` also runs after a real ride (`triggerTravel`), including the ride home. One call per transition, after the bike when the bike plays.

| Entry state | Front PNG |
|---|---|
| `FOOD_REST` | `/idolnat/screens/restaurant_front.png` |
| `TRAIN_SING` | `/idolnat/screens/singing_school_front.png` |
| `TRAIN_SWIM` | `/idolnat/screens/swimming_pool_front.png` |
| `TRAIN_DANCE` | `/idolnat/screens/ballet_school_front.png` |
| `TRAIN_GYM` | `/idolnat/screens/gym_front.png` |
| `TRAIN_RUN` | `/idolnat/screens/run_forest_front.png` |
| `TRAIN_LIBRARY` | `/idolnat/screens/high_school_bg.png` |
| `FLOWERS_MARKET` | `/idolnat/screens/flowers_market_front.png` |
| `HEALTH_DOCTOR` | `/idolnat/screens/doctor_office_front.png` |
| `HEALTH_TEMPLE` | `/idolnat/screens/temple_front.png` |
| `HEALTH_ONSEN` | `/idolnat/screens/onsen_front.png` |
| `MATSURI_TITLE` | `/idolnat/screens/matsuri_bg.png` |
| `COMP_LOCAL` | `/idolnat/screens/competition_local.png` |
| `COMP_DEPT` | `/idolnat/screens/competition_departmental.png` |
| `COMP_REG` | `/idolnat/screens/competition_regional.png` |
| `COMP_NAT` | `/idolnat/screens/competition_national.png` |

Left out of that flash, on purpose:

- `FOOD_CONBINI` and `NEKO_CAFE`. `preloadImages()` already uses `conbimart_front.png` and `neko_cafe_front.png` as the scene. Calling `displayBusinessFront()` as well would flash the same picture twice. A cross-tile ride to either place still plays the bike, then the scene front, with no extra flash.
- Home (`HOME_LOOP`, `FOOD_MENU`, `FOOD_COOK`, `FOOD_ORDER`, `HEALTH_WASH`, `REST_MEDITATE`, `REST_SLEEP`, `GARDEN_LOOP`). Those states are absent from `showsBusinessFrontOnArrival()`. `natsumi_home_front.png` plays only when the ride itself plays.
- Later beats of a listed activity. They are not in `showsBusinessFrontOnArrival()`, so a hop from the restaurant into singing (both `CITY`) flashes the singing-school front once, and a hop from `FOOD_REST` to `FOOD_REST2` flashes nothing.

## Menu to destination

| Menu | Item | State | Tile |
|---|---|---|---|
| Food | 0 Cooking | `FOOD_COOK` | `HOME` |
| Food | 1 Restaurant | `FOOD_REST` | `CITY` (age above 14) |
| Food | 2 Order | `FOOD_ORDER` | `HOME` (age above 10) |
| Food | 3 Conbini | `FOOD_CONBINI` | `OUTSKIRTS` |
| Food | 4 Cafe | `NEKO_CAFE` | `CITY` |
| Training | 0 Sing | `TRAIN_SING` | `CITY` |
| Training | 1 Dance | `TRAIN_DANCE` | `OUTSKIRTS` |
| Training | 2 Swim | `TRAIN_SWIM` | `CITY` |
| Training | 3 Gym | `TRAIN_GYM` | `OUTSKIRTS` |
| Training | 4 Run | `TRAIN_RUN` | `COUNTRYSIDE` |
| Training | 5 Library | `TRAIN_LIBRARY` | `CITY` |
| Training | 6 Market | `FLOWERS_MARKET` | `CITY` |
| Competition | 0 Local | `COMP_LOCAL` | `CITY` (`competition == 3`) |
| Competition | 1 Departmental | `COMP_DEPT` | `HANAMORI` (`competition == 7`) |
| Competition | 2 Regional | `COMP_REG` | `OSAKA` (`competition == 11`) |
| Competition | 3 National | `COMP_NAT` | `TOKYO` (`competition == 15`) |
| Health | 0 Wash | `HEALTH_WASH` | `HOME` |
| Health | 1 Doctor | `HEALTH_DOCTOR` | `CITY` |
| Health | 2 Temple | `HEALTH_TEMPLE` | `COUNTRYSIDE` |
| Health | 3 Onsen | `HEALTH_ONSEN` | `COUNTRYSIDE` |
| Rest | 0 Meditate | `REST_MEDITATE` | `HOME` |
| Rest | 1 Sleep | `REST_SLEEP` | `HOME` (Energy below 4) |
| Garden | Home menu 7 | `GARDEN_LOOP` | `HOME` |
| Garden | 0 Plant | `GARDEN_PLANT` | stays `HOME` |
| Garden | 1 Water | `GARDEN_WATER` | stays `HOME` |
| Garden | 2 Pick | `GARDEN_PICK` | stays `HOME` |
| Garden | 3 Cleanup | `GARDEN_CLEANUP` | stays `HOME` |
| Events | 0 Matsuri | `MATSURI_TITLE` | `OUTSKIRTS` |
| Events | 1 Jobs | toast, then `HOME_LOOP` | no destination tile |
| Events | 2 Challenges | `CHALLENGES_SCREEN` | no tile change |
| Events | 3 Festivals | toast, then `HOME_LOOP` | no destination tile |

Food items 1–4 also refuse to leave while a delivery is pending. Restaurant and Order send an under-age Natsumi to `HOME_LOOP` with a toast.
