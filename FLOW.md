# Idol Natsumi – GameState catalog

- Source: `enum GameState` in `idol_natsumi/idol_natsumi.ino` (the block that starts at `VERSION_SCREEN` and ends at `TRAVEL_SCREEN`).
- 199 members. Every one is listed below, in declaration order inside its feature.
- Scope is `idol_natsumi/idol_natsumi.ino` only.
- Each `GameState` also selects a `ScreenState` (`TEXT`, `CARD`, `DIALOG`, `GAME`, `IDLE`, `ROOM`) inside `changeState()`. That second enum is the draw path.
- Player-facing flow issues from the earlier review are `NFLOW-01`..`10`. See that flow review. This file only catalogs states.

## How a numbered chain reads

A numbered suffix is the next beat of the same activity, unless the note says the numbers are pages of one menu. The usual shape is entry, then the activity, then a result. The middle beat might be a timed picture, a dialog, a countdown, or a minigame.

Two chains reuse later numbers out of numeric order: Neko Cafe, and Emily's rock-paper-scissors (`PFC_GAME`). Those sections give the play order on their own.

Twelve members are never entered. They are marked **Unused** or **Reserved** in the tables: `TITLE_SCREEN`, `IDLE_STATS`, `REST_MEDITATE2`, `COMP_STATUS`, `MATSURI_TICKETS3`, `NEKO_CAFE15`..`NEKO_CAFE19`, `BUSINESS_FRONT`, `TRAVEL_SCREEN`.

## Where the groups sit in the enum

Declaration order. A few features are split, so the same section name appears more than once.

| In the enum | Feature section |
|---|---|
| `VERSION_SCREEN` .. `CONTINUE_GAME` | Boot / title |
| `INTRO` .. `INTRO7` | Intro |
| `HOME_LOOP` | Home hub |
| `FLOWERS_MARKET` .. `FLOWERS_MARKET7` | Garden / flowers |
| `FOOD_MENU` .. `FOOD_ORDER9` | Food / kitchen |
| `HEALTH_MENU` .. `HEALTH_ONSEN` | Health |
| `IDLE_HOME`, `IDLE_STATS` | Home hub |
| `REST_MENU` .. `REST_SLEEP2` | Rest |
| `STATS_SCREEN` | Home hub |
| `GARDEN_MENU` .. `GARDEN_CLEANUP` | Garden / flowers |
| `TRAIN_MENU` .. `TRAIN_STATUS` | Training |
| `COMP_EXPLAIN` .. `COMP_FAILED` | Competition |
| `MATSURI_TITLE` .. `MATSURI_GARAPON4` | Matsuri |
| `ACTION_OUTCOME`, `EVENTS_MENU`, `INVENTORY_SCREEN`, `CARDS_SCREEN` | Home hub |
| `DOOR_KNOCK` .. `PFC_GAME9` | Visitors / Emily RPS |
| `CHALLENGES_SCREEN` .. `CHALLENGE_DONE3` | Home hub |
| `NEKO_CAFE` .. `NEKO_CAFE19` | Food / kitchen |
| `PAY_SCREEN` .. `TRAVEL_SCREEN` | Travel / pay / meta |

## Boot / title

Live boot is `VERSION_SCREEN` → `M5_SCREEN` → `MOTTO_SCREEN` → `TITLE_SCREEN2`. Each auto beat waits a short timer.

| State | What it maps to |
|---|---|
| `VERSION_SCREEN` | Opening text card: title, "for M5 Cardputer", copyright, version. Advances to the M5 logo. |
| `M5_SCREEN` | M5 logo card. Advances to the motto. |
| `MOTTO_SCREEN` | Motto card. Advances to the title menu. |
| `TITLE_SCREEN` | **Unused.** Same title art as the menu, and its handler would advance to `TITLE_SCREEN2`. Nothing enters it. |
| `TITLE_SCREEN2` | Title menu: New Game, Continue, and the intro. |
| `NEW_GAME` | Resets the in-memory save (age 11, home tile, empty delivery flag) and drops into `HOME_LOOP`. |
| `CONTINUE_GAME` | Loads the SD save, or runs the same reset when no file exists, then opens `HOME_LOOP`. The loader copies the saved `current_state` into `loadedContinueState` and leaves it there: `continueStateLoaded` is never set, so Continue always opens home. |

## Intro

Any key steps forward. The chain returns to `TITLE_SCREEN2`. `gameStarted` stays false, so this story never starts a save.

| State | What it maps to |
|---|---|
| `INTRO` | Natsumi introduces herself (bedroom). |
| `INTRO2` | She names her town (countryside). |
| `INTRO3` | She says she wants to become an idol. |
| `INTRO4` | She names the player as her manager (lounge). |
| `INTRO5` | She asks for a good life, training, and competitions. |
| `INTRO6` | She describes the game (garden). |
| `INTRO7` | "Good luck" card. Any key returns to the title menu. |

## Home hub

These states are the home room, the boards opened from it, the shared result card, and the challenge list. In the enum they are scattered; the table is still declaration order.

| State | What it maps to |
|---|---|
| `HOME_LOOP` | Home room (lounge). The menu opens Stats, Inventory, Food, Training, Competition, Health, Rest, Garden, Events, and Cards. Space opens Challenges. |
| `IDLE_HOME` | Tutorial and stat slideshow, after a quiet wait at home with the menu closed and no visitor to play. A key returns home. |
| `IDLE_STATS` | **Unused.** Configured beside the slideshow. Nothing enters it, and its slide call is commented out. |
| `STATS_SCREEN` | Status board. |
| `ACTION_OUTCOME` | Shared result card ("+1" or "MAX" for one stat). Where it returns depends on the activity that opened it: a menu, home, the next delivery line, a ticket award, or a challenge banner. |
| `EVENTS_MENU` | Events menu. Matsuri and Challenges are live. Jobs and Festivals toast "coming soon" and go home. |
| `INVENTORY_SCREEN` | Inventory pages (fridge and other stock). |
| `CARDS_SCREEN` | Card collection viewer. |
| `CHALLENGES_SCREEN` | Challenge list. ESC returns to wherever opened it (`challengesReturnState`). |
| `CHALLENGE_DONE` | "Challenge complete" banner. Auto-advances. |
| `CHALLENGE_DONE2` | Challenge-complete dialog. |
| `CHALLENGE_DONE3` | Closing beat of the banner, then back to `latestMenu`. |

## Food / kitchen

Food menu items: Cooking, Restaurant, Order, Conbini, Cafe. Cooking and Order stay on the home tile. Restaurant and Neko Cafe are the city. Conbimart is the outskirts. A ride plays only when that tile differs from the current one (see Travel).

`FOOD_REST2`..`4` and `FOOD_ORDER2`..`4` are dishes you browse, not three steps in a row. The cafe drink, size, and seat numbers work the same way.

| State | What it maps to |
|---|---|
| `FOOD_MENU` | Kitchen menu. |
| `FOOD_CONBINI` | Conbimart front. Advances straight into the shop. |
| `FOOD_CONBINI2` | Shop grid. Confirming a purchase opens the pay screens, then the cashier. |
| `FOOD_CONBINI3` | Cashier dialog after pay. |
| `FOOD_COOK` | Open fridge. Arrows move, Enter toggles an ingredient, Space cooks. |
| `FOOD_COOK2` | Photo of the cooked dish, then `ACTION_OUTCOME`. |
| `FOOD_REST` | Waitress greeting. Any key opens the menu. Age gate is 15+. |
| `FOOD_REST2` | Tofu vegetable teishoku. Left/right moves among the three dishes. |
| `FOOD_REST3` | Grilled salmon teishoku. |
| `FOOD_REST4` | Fried chicken teishoku. Enter with enough money opens pay; `returnTo` is `FOOD_REST5`. |
| `FOOD_REST5` | "Your food is on the way." Entered after pay. Any key goes to `ACTION_OUTCOME`, which then starts the eating cards. |
| `FOOD_REST6` | Eating, step 1. Timed. |
| `FOOD_REST7` | Eating, step 2. Timed. |
| `FOOD_REST8` | Eating, step 3. Timed, then `ACTION_OUTCOME`, then the food menu. |
| `FOOD_ORDER` | Orderibi boot screen. Advances to the first dish. |
| `FOOD_ORDER2` | Cheap bento. Left/right moves among the three dishes. |
| `FOOD_ORDER3` | Healthy salad. |
| `FOOD_ORDER4` | Deluxe teishoku. Enter opens pay. |
| `FOOD_ORDER5` | "Order confirmed" card. Advances on its own. |
| `FOOD_ORDER6` | Arms `waitingForFoodDelivery`, saves, and returns home. The doorbell waits until Natsumi is home or on a top-level menu. |
| `FOOD_ORDER7` | Delivered-food picture when the order arrives. Advances to the courier dialog. |
| `FOOD_ORDER8` | Courier dialog. A key opens `ACTION_OUTCOME`, which continues to the next line. |
| `FOOD_ORDER9` | Second courier line, then `ACTION_OUTCOME`, then home. |

Neko Cafe is declared later in the enum, after the challenge screens. Play order is `NEKO_CAFE` → `NEKO_CAFE2` → drink (`3`/`4`/`5`) → size (`6`/`7`) → pay → `NEKO_CAFE12` → seat (`8`/`9`) → `NEKO_CAFE10` → `NEKO_CAFE11` → `NEKO_CAFE14` → back to `NEKO_CAFE2`. A size the player cannot afford goes to `NEKO_CAFE13` and back to the drinks.

| State | What it maps to |
|---|---|
| `NEKO_CAFE` | Cafe front. Timed lead-in to the welcome. |
| `NEKO_CAFE2` | Welcome dialog at the counter. Also the return after the seat. |
| `NEKO_CAFE3` | Matcha, one page of the drink menu. |
| `NEKO_CAFE4` | Hojicha. |
| `NEKO_CAFE5` | Soda. |
| `NEKO_CAFE6` | Small size. |
| `NEKO_CAFE7` | Large size. Enter opens pay when the player can afford it (`returnTo` is `NEKO_CAFE12`). |
| `NEKO_CAFE8` | Lounge seat (nap). |
| `NEKO_CAFE9` | Table seat (reading). |
| `NEKO_CAFE10` | Short lead-in before the drink. |
| `NEKO_CAFE11` | Drink, with thought bubbles. Timed (shorter for a small cup), then the seat scene. |
| `NEKO_CAFE12` | "Enjoy your drink" dialog after pay, then the seat choice. |
| `NEKO_CAFE13` | Cannot afford that size. Returns to the drink menu. |
| `NEKO_CAFE14` | The seat itself: nap on the lounge, or reading at the table. Timed, then back to the welcome. |
| `NEKO_CAFE15` | **Reserved, unused.** No art and no transition. The enum comment covers `NEKO_CAFE15`..`NEKO_CAFE19`. |
| `NEKO_CAFE16` | **Reserved, unused.** |
| `NEKO_CAFE17` | **Reserved, unused.** |
| `NEKO_CAFE18` | **Reserved, unused.** |
| `NEKO_CAFE19` | **Reserved, unused.** |

## Health

Health menu: Wash, Doctor, Temple, Onsen. Wash is home. The doctor is the city. Temple and onsen are the countryside.

| State | What it maps to |
|---|---|
| `HEALTH_MENU` | Bathroom menu. |
| `HEALTH_WASH` | Shower minigame (temperature slider). A good shower plays the step cards; too hot or too cold skips to the result. |
| `HEALTH_WASH2` | Shower step 1, after a good temperature. |
| `HEALTH_WASH3` | Shower step 2. |
| `HEALTH_WASH4` | Shower step 3, then the result. |
| `HEALTH_WASH5` | Shower result. A good wash shows a toast and `ACTION_OUTCOME`. A bad wash returns to the health menu. |
| `HEALTH_DOCTOR` | Doctor's greeting. |
| `HEALTH_DOCTOR2` | Office hold. Timed into the exam cards. |
| `HEALTH_DOCTOR3` | Exam step 1. |
| `HEALTH_DOCTOR4` | Exam step 2. |
| `HEALTH_DOCTOR5` | Exam step 3, then the advice. |
| `HEALTH_DOCTOR6` | Advice on hunger, hygiene, energy, fitness, and performance, then home. |
| `HEALTH_TEMPLE` | Priest's greeting. |
| `HEALTH_TEMPLE2` | Temple hold. Timed into the prayer cards. |
| `HEALTH_TEMPLE3` | Priest step 1. |
| `HEALTH_TEMPLE4` | Priest step 2. |
| `HEALTH_TEMPLE5` | Priest step 3, then the closing line. |
| `HEALTH_TEMPLE6` | Advice on culture, grace, popularity, and spirit, then home. |
| `HEALTH_ONSEN` | Bath scene for Natsumi's age. Entering sets hygiene to full. |

## Rest

Rest menu: Meditate, Sleep. Both are home.

| State | What it maps to |
|---|---|
| `REST_MENU` | Bedroom menu. |
| `REST_MEDITATE` | Meditation timer. Finishing opens `ACTION_OUTCOME`, which returns home. |
| `REST_MEDITATE2` | **Unused.** A "feeling better" dialog is drawn for it, and a screen config exists. Nothing enters it. |
| `REST_SLEEP` | Sleep, with the energy bar. Finishing opens `ACTION_OUTCOME`, which continues to the wake-up line. |
| `REST_SLEEP2` | "I feel much better" after sleep. |

## Garden / flowers

The market states are declared just after `HOME_LOOP`. The garden menu is declared after `STATS_SCREEN`. Market is opened from the training menu (the Market item) and sells flowers grown here. The garden plot is home.

| State | What it maps to |
|---|---|
| `FLOWERS_MARKET` | Market entry. No flowers: toast and home. Otherwise the step cards start. |
| `FLOWERS_MARKET2` | Market step 1. Timed. |
| `FLOWERS_MARKET3` | Market step 2. Timed. |
| `FLOWERS_MARKET4` | Market step 3. Timed, then the stall. |
| `FLOWERS_MARKET5` | Choose a price and sell. |
| `FLOWERS_MARKET6` | Sale animation. |
| `FLOWERS_MARKET7` | Sale debrief, then `ACTION_OUTCOME` (grace). |
| `GARDEN_MENU` | Tool menu: Plant, Water, Pick, Cleanup. |
| `GARDEN_LOOP` | The plot and cursor. Home opens this directly. |
| `GARDEN_PLANT` | Plant the selected tile, then back to the plot. |
| `GARDEN_WATER` | Water the selected tile, then back to the plot. |
| `GARDEN_PICK` | Pick the selected tile, then back to the plot. |
| `GARDEN_CLEANUP` | Clear the selected tile, then back to the plot. |

## Training

Training menu: Sing, Dance, Swim, Gym, Run, Library, Market. Market is `FLOWERS_MARKET` (above).

Sing, dance, swim, gym, and run share one chain: countdown, minigame, teacher debrief. The debrief opens `ACTION_OUTCOME`, which then opens `TRAIN_STATUS`.

| State | What it maps to |
|---|---|
| `TRAIN_MENU` | Training map menu. |
| `TRAIN_SING` | Singing countdown (3-2-1). City. |
| `TRAIN_SING2` | Singing minigame. |
| `TRAIN_SING3` | Music-teacher debrief. |
| `TRAIN_DANCE` | Dance countdown. Outskirts. |
| `TRAIN_DANCE2` | Dance minigame. |
| `TRAIN_DANCE3` | Dance-teacher debrief. |
| `TRAIN_SWIM` | Swim countdown. City. |
| `TRAIN_SWIM2` | Swim minigame. |
| `TRAIN_SWIM3` | Swim-teacher debrief. |
| `TRAIN_GYM` | Gym countdown. Outskirts. |
| `TRAIN_GYM2` | Gym minigame. |
| `TRAIN_GYM3` | Gym-teacher debrief. |
| `TRAIN_RUN` | Run countdown. Countryside. |
| `TRAIN_RUN2` | Run minigame. |
| `TRAIN_RUN3` | Run debrief. |
| `TRAIN_LIBRARY` | Library reading timer (age-specific picture). City. Finishing opens `ACTION_OUTCOME`. |
| `TRAIN_STATUS` | Post-lesson status dialog after sing, dance, swim, gym, or run. |

## Competition

Menu: Local, Departmental, Regional, National. Local is the city, departmental is Hanamori, regional is Osaka, national is Tokyo.

The suffix is the same beat at every tier:

| Suffix | Beat |
|---|---|
| (none) | Arrival card. |
| `2` | Second arrival card. |
| `3` | Host dialog. |
| `4` | 3-2-1 countdown. |
| `5` | Note-catching game. |
| `6` | Host debrief. |

A finished run passes through `COMP_COMPLETE` (on to that tier's debrief) or `COMP_FAILED` (home). National adds `COMP_NAT7` after the debrief.

| State | What it maps to |
|---|---|
| `COMP_EXPLAIN` | Rules dialog when the player cannot enter the chosen tier. Any key returns home. |
| `COMP_MENU` | Competition menu. |
| `COMP_LOCAL` | Local arrival. |
| `COMP_LOCAL2` | Local second arrival card. |
| `COMP_LOCAL3` | Local host. |
| `COMP_LOCAL4` | Local countdown. |
| `COMP_LOCAL5` | Local note game. |
| `COMP_LOCAL6` | Local debrief. A new tier unlock opens `ACTION_OUTCOME`; otherwise home. |
| `COMP_DEPT` | Departmental arrival (Hanamori). |
| `COMP_DEPT2` | Departmental second arrival card. |
| `COMP_DEPT3` | Departmental host. |
| `COMP_DEPT4` | Departmental countdown. |
| `COMP_DEPT5` | Departmental note game. |
| `COMP_DEPT6` | Departmental debrief. Same unlock rule as local. |
| `COMP_REG` | Regional arrival (Osaka). |
| `COMP_REG2` | Regional second arrival card. |
| `COMP_REG3` | Regional host. |
| `COMP_REG4` | Regional countdown. |
| `COMP_REG5` | Regional note game. |
| `COMP_REG6` | Regional debrief. Same unlock rule as local. |
| `COMP_NAT` | National arrival (Tokyo). |
| `COMP_NAT2` | National second arrival card. |
| `COMP_NAT3` | National host. |
| `COMP_NAT4` | National countdown. |
| `COMP_NAT5` | National note game. |
| `COMP_NAT6` | National debrief, then the champion card (`COMP_NAT7`) on a win or a loss. |
| `COMP_NAT7` | National champion card, then home. |
| `COMP_STATUS` | **Unused.** The enum comment says there is no screen yet. Nothing enters it. |
| `COMP_COMPLETE` | Success flash. Routes to the debrief of the tier that just finished (`previousState`). |
| `COMP_FAILED` | Failure flash, then home. |

## Matsuri

Opened from Events. `MATSURI_TITLE` is the outskirts tile. The three menu pages are stalls you switch with left/right. `MATSURI_SAVORY` and `MATSURI_SAVORY2` are two dishes. Sugary is one dish. Garapon is the prize wheel.

| State | What it maps to |
|---|---|
| `MATSURI_TITLE` | Festival banner. Advances to the stall menu. |
| `MATSURI_TICKETS` | Ticket award card (after some results). Advances to the ticket dialog. |
| `MATSURI_TICKETS2` | Ticket dialog (`allocateTickets`), then back to `latestMenu`. |
| `MATSURI_TICKETS3` | **Unused.** A handler would leave to `latestMenu` or the challenge banner. Nothing enters it. |
| `MATSURI_MENU` | Savory stall page. Enter opens takoyaki / yakisoba. |
| `MATSURI_MENU2` | Sugary stall page. Enter opens kakigōri. |
| `MATSURI_MENU3` | Garapon page. Enter opens the wheel when Natsumi has a ticket, otherwise the no-ticket dialog. |
| `MATSURI_COST` | Buy / Cancel for the stall just chosen. Buy spends the ticket. |
| `MATSURI_SAVORY` | Takoyaki. |
| `MATSURI_SAVORY2` | Yakisoba. Left/right switches with takoyaki. |
| `MATSURI_SAVORY3` | "Enjoy your food" after a savory buy, then the outcome lead-in. |
| `MATSURI_SAVORY4` | Not enough tickets for savory. Returns home. |
| `MATSURI_SAVORY5` | Savory result. Advances to `ACTION_OUTCOME`, which returns to the stall menu. |
| `MATSURI_SUGARY` | Kakigōri. |
| `MATSURI_SUGARY2` | "Enjoy your food" after a sugary buy. |
| `MATSURI_SUGARY3` | Not enough tickets for sugary. Returns home. |
| `MATSURI_SUGARY4` | Sugary result. Advances to `ACTION_OUTCOME`, which returns to the stall menu. |
| `MATSURI_GARAPON` | Garapon dialog when Natsumi has a ticket. Continues to Buy / Cancel. |
| `MATSURI_GARAPON2` | Not enough tickets for the wheel. Returns home. |
| `MATSURI_GARAPON3` | Garapon minigame. |
| `MATSURI_GARAPON4` | Wheel result. Advances to `ACTION_OUTCOME`, which returns to the stall menu. |

## Visitors / Emily RPS

A quiet wait at home can open `DOOR_KNOCK` (birthday, a competition win, a perfect lesson, or grandma when hunger is empty). `VISITOR_PORTRAIT` sits between `DOOR_KNOCK2` and `DOOR_KNOCK3` in play, and is declared after `DOOR_KNOCK10`.

Emily (`visitor == 5`) leaves `DOOR_KNOCK6` into rock-paper-scissors. Everyone else gets the gift.

| State | What it maps to |
|---|---|
| `DOOR_KNOCK` | The door. A key starts the visit. |
| `DOOR_KNOCK2` | Door line, then the portrait. |
| `DOOR_KNOCK3` | Next line after the portrait, then the visitor at the door. |
| `DOOR_KNOCK4` | Visitor at the door (portrait depends on who came). |
| `DOOR_KNOCK5` | Chat. Lounge for family; bedroom for Emily. |
| `DOOR_KNOCK6` | Following line. Emily starts `PFC_GAME`. Anyone else continues to the gift. |
| `DOOR_KNOCK7` | Gift picture (envelope, pear, cake, or the RPS result). Advances to `ACTION_OUTCOME`, which continues the visit. |
| `DOOR_KNOCK8` | Line after the gift. |
| `DOOR_KNOCK9` | Teatime picture for that visitor. |
| `DOOR_KNOCK10` | Goodbye. Home, or `CHALLENGE_DONE` when a challenge just completed. |
| `VISITOR_PORTRAIT` | Framed portrait of the visitor, between the second and third knock beats. |

`PFC_GAME` is pierre-feuille-ciseaux, three rounds against Emily. `PFC_GAME8` (both hands) and `PFC_GAME9` (that round's result) are reused after every throw. Play order: title → round prompt → throw → hands → result, three times, then `DOOR_KNOCK7`.

| State | What it maps to |
|---|---|
| `PFC_GAME` | Title ("press Enter"). |
| `PFC_GAME2` | Round 1 prompt. |
| `PFC_GAME3` | Round 1 throw (rock, paper, scissors). |
| `PFC_GAME4` | Round 2 prompt. |
| `PFC_GAME5` | Round 2 throw. |
| `PFC_GAME6` | Round 3 prompt. |
| `PFC_GAME7` | Round 3 throw. |
| `PFC_GAME8` | Both hands, shown after each throw. |
| `PFC_GAME9` | Round result, shown after the hands. The third result returns to the gift. |

## Travel / pay / meta

`prepareTravel()` runs on every `changeState()`. A bike ride plays only when the destination tile differs from `currentTile`. Coming home on that ride spends 1 Energy. The same tile means no bike and no energy cost.

`displayBusinessFront()` then flashes the place just entered. It also flashes on a same-tile hop for the entry states in `showsBusinessFrontOnArrival()` (restaurant, each training stop, the flower market, doctor, temple, onsen, the matsuri banner, and each competition tier). Later beats of those activities are not in that list, so the front does not flash twice. Conbimart and Neko Cafe are left out because their first scene is already the front. The home front flashes only when the ride itself plays.

After a successful activity, `postActivityLeave()` sends Natsumi to `HOME_LOOP` when Energy or Hunger is below 1, and otherwise reopens `latestMenu`. The bike still runs only when that destination's tile differs. The check is the leave after the result (training status, library, finished meal, cooking, onsen, a good shower, a flower sale, the Conbimart cashier, a matsuri stall). It is not called from `PAY_SCREEN`, a cancel, or not-enough-money. A challenge banner still plays first; the leave runs after `CHALLENGE_DONE3`. Relaunching that same entry skips the business-front flash once (`skipNextBusinessFront`). Conbimart and Neko Cafe skip their timed front instead: same tile opens the shop or the welcome, and a redo from another tile still rides.

The ride is a direct call to `manageTravel()`, and the front is a direct call to `displayBusinessFront()`. Both run inside `prepareTravel()`.

| Tile | Entry states |
|---|---|
| `HOME` | `HOME_LOOP`, `FOOD_MENU`, `FOOD_COOK`, `FOOD_ORDER`, `HEALTH_WASH`, `REST_MEDITATE`, `REST_SLEEP`, `GARDEN_LOOP` |
| `CITY` | `FOOD_REST`, `NEKO_CAFE`, `TRAIN_SING`, `TRAIN_SWIM`, `TRAIN_LIBRARY`, `FLOWERS_MARKET`, `HEALTH_DOCTOR`, `COMP_LOCAL` |
| `OUTSKIRTS` | `FOOD_CONBINI`, `TRAIN_DANCE`, `TRAIN_GYM`, `MATSURI_TITLE` |
| `COUNTRYSIDE` | `TRAIN_RUN`, `HEALTH_TEMPLE`, `HEALTH_ONSEN` |
| `HANAMORI` | `COMP_DEPT` |
| `OSAKA` | `COMP_REG` |
| `TOKYO` | `COMP_NAT` |

Pay is the shared register. Three timed beats of the same pay picture, then `returnTo` (restaurant, conbini, Orderibi, or the cafe).

| State | What it maps to |
|---|---|
| `PAY_SCREEN` | Pay picture, first beat: money on hand. |
| `PAY_SCREEN2` | Second beat: amount due. Money is deducted on entry. A restaurant meal (hunger and grace) or Conbimart stock is granted here too. Orderibi and the cafe only deduct; the food and the drink continue in their own chains. |
| `PAY_SCREEN3` | Third beat: balance after the charge, then `returnTo`. |
| `BUSINESS_FRONT` | **Unused.** An idle handler would leave to `latestMenu`. Arrival fronts are `displayBusinessFront()` inside `prepareTravel()`. |
| `TRAVEL_SCREEN` | **Unused.** A game handler would call `manageTravel()`. The ride is already called from `prepareTravel()`. |
