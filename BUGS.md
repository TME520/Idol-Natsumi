# Idol Natsumi – Bug Report

- Repo: TME520/Idol-Natsumi, branch `natsubot/update-19-2` (based on `update-19-2`)
- File reviewed: `idol_natsumi/idol_natsumi.ino` (13,176 lines), checked against `README.md`. `lab/` ignored.
- Line numbers refer to the file as reviewed at commit `f3f124f` and may drift as fixes land.

## Bug index

| Code | Severity | Title | Status |
|---|---|---|---|
| NB-H01 | High | Energy drops to 0 almost instantly on every debrief screen | Fixed (14ccc39) |
| NB-H02 | High | Opening INTRO from the title screen overwrites the real save with a blank character | Fixed (64dfb39) |
| NB-H03 | High | ESC can never leave the Matsuri; the player is trapped while holding tickets | Fixed (147cdd8) |
| NB-H04 | High | Orderibi delivers food the player can't afford (money goes negative) | Fixed (3100690) |
| NB-H05 | High | Missing or misnamed SD assets freeze the device for minutes | Fixed (b8d05f5, e06e2e4) |
| NB-M01 | Medium | Shared `changeStateCounter` drops transitions and can soft-lock the game | Fixed (4ad6473) |
| NB-M02 | Medium | Food delivery can be lost, or can pull the player out of whatever they are doing | Fixed (5d38a91) |
| NB-M03 | Medium | All submenus share one selection index | Fixed (ccf3598) |
| NB-M04 | Medium | Matsuri food and Garapon take the ticket (and feed Natsumi) before Buy/Cancel | Fixed (a9218fc) |
| NB-M05 | Medium | Leaving meditation early stops Spirit from ever updating again | Fixed (799dd2c) |
| NB-M06 | Medium | Rock-paper-scissors: an invalid key breaks the round sequence | Fixed (dcd7933) |
| NB-M07 | Medium | Competition state survives an aborted run | Fixed (d0b1b68) |
| NB-M08 | Medium | The screensaver or a visitor interrupts the player while they are using the home menu | Fixed (092d8ff) |
| NB-M09 | Medium | Rewards and penalties are applied inside draw code and repeat on every redraw | Fixed (8ed80f7) |
| NB-M10 | Medium | Flower sales never award Grace | Fixed (ba88da1) |
| NB-M11 | Medium | Minigames and activities have no ESC exit | Fixed (c892af9) |
| NB-M12 | Medium | `conbimartVisitsTotal` is not saved | Fixed (47e341d) |
| NB-L01 | Low | Sleep/Onsen energy recovery looks inverted | Open |
| NB-L02 | Low | Restaurant age gate doesn't match the README | Open |
| NB-L03 | Low | Low-battery toast is re-issued every frame | Fixed (5a7f3e6) |
| NB-L04 | Low | Cards and Challenges screens: ESC goes to `latestMenu`, not Home | Fixed (c988444) |
| NB-L05 | Low | Travel-home energy is charged at the wrong time | Open |
| NB-L06 | Low | Onsen sets Hygiene on exit, not on entry | Open |
| NB-L07 | Low | The outcome screen shows "MAX" instead of "+1" | Open |
| NB-L08 | Low | Garapon rewards may not be saved, and the currency shows wrongly | Open |
| NB-L09 | Low | Age caps at 22 and the game never ends | Open |
| NB-L10 | Low | Saved fields are never read back, and loads are not validated | Open |
| NB-L11 | Low | Swim minigame leftovers | Open |
| NB-L12 | Low | Bath game exit-delay code is dead | Open |
| NB-L13 | Low | The Pantry Snack fallback can never trigger | Open |
| NB-L14 | Low | Competition rules don't match the README | Open |
| NB-L15 | Low | NEW_GAME doesn't reset all globals | Open |
| NB-L16 | Low | Neko Cafe reading frames always use the 11-year-old sprites | Open |
| NB-L17 | Low | Timers are not rollover-safe | Open |
| NB-L18 | Low | Minor logic oddities | Open |
| NB-L19 | Low | Text typos | Open |

Codes are stable: do not renumber. Status is `Open` or `Fixed (<commit>)`.

## Compilation
I attempted compilation with arduino-cli and it **succeeded** (exit 0):
`arduino-cli compile -b m5stack:esp32:m5stack_cardputer --warnings all`
- Setup: core m5stack:esp32 3.3.10, M5Cardputer, M5Unified 0.2.26, M5GFX 0.2.33.
- Size: the sketch uses 754,599 bytes of flash (57%). Globals use 28,436 bytes (8%).
- No errors. All warnings are benign:
  - unused variables (1830, 1844, 4402, 5322, 6041, 6747, 7347, 7348, 7767)
  - many `key` variables that are set but never used
  - `competitionCompletionTime` is set but never used (7432)
  - missing initializers for `FoodDisplayItem` (9213)
  - many -Wswitch "enumeration not handled" warnings
  - one library macro redefinition (`digitalPinHasPWM`)

Two engine facts drive several of the bugs below:
- **Single key reader per frame:** `Keyboard.isChange()` is consumed by the first read (M5Cardputer Keyboard.cpp), so only the first handler in a frame sees a key press.
- **Shared transition counter:** `changeState()` only transitions when the global `changeStateCounter == delay`. Every other call increments that counter.

---

## HIGH

### NB-H01: Energy drops to 0 almost instantly on every debrief screen
- **Status:** Fixed (14ccc39): energy is deducted once per debrief screen via `debriefEnergyApplied`, which `changeState()` resets on each transition.
- **Where:** `miniGameDebrief()` 11656–11661, called every frame (50 ms) from `manageDialog()` for TRAIN_*3, COMP_*6 and FLOWERS_MARKET7.
- **What happens:** `if (natsumi.energy > 0) natsumi.energy--;` runs on every frame. Energy hits 0 within about 200 ms after any training, flower sale or competition. After that, most menus refuse with "Natsumi is too tired", and competitions (which need Energy 4) are blocked.
- **Fix:** Decrement energy once on entry, using a one-shot flag or `changeStateCounter == 0`.

### NB-H02: Opening INTRO from the title screen overwrites the real save with a blank character
- **Status:** Fixed (64dfb39): new `gameStarted` flag (set by NEW_GAME / CONTINUE_GAME) gates `updateAging()`, `updateStats()` and the pulse autosave.
- **Where:** `drawMenu()` title menu 8664–8668/8689 → INTRO states use the DIALOG config → `manageDialog()` calls `updateAging()` (4876). The birthday branch then runs (4387–4395). The autosave fires at `updateFiveSecondPulse()` 4643.
- **What happens:**
  - The zero-initialised `natsumi` goes from age 0 to 11, which counts as a "birthday": +10000 money, `saveRequired = true`, `birthdayVisitEnabled = true`.
  - The next pulse writes a blank character (empty stats, fridge, cards, garden and challenges) to `savegame.dat`. CONTINUE then loads that garbage.
  - The good save sits in `.bak` only until the next save rotates it out, so the player's progress is lost.
  - Side effect: if the player then picks NEW_GAME, a birthday visit with +10000 is still pending.
- **Fix:** Add a `gameStarted` flag. Skip `updateAging`/`updateStats`/autosave until NEW_GAME or a successful CONTINUE.

### NB-H03: ESC can never leave the Matsuri; the player is trapped while holding tickets
- **Status:** Fixed (147cdd8): ESC on MATSURI_MENU/2/3 in `matsuriMainMenu()` now goes to `EVENTS_MENU`; sub-screens still return to MATSURI_MENU.
- **Where:** `changeState()` 4235–4238 sets `latestMenu = MATSURI_MENU`. The ESC handler in `matsuriMainMenu()` 12341–12343, 12362–12364 and 12388–12390 does `changeState(0, latestMenu, 0)`, which goes back to MATSURI_MENU. Food and Garapon results also return to MATSURI_MENU (10988–10992).
- **What happens:** ESC just reloads the Matsuri screen. The only exits are the "not enough tickets" dialogs (`matsuriDialogs()` 11144–11160 → HOME_LOOP), so the player has to spend every ticket to get out.
- **Fix:** Make ESC in `matsuriMainMenu` go to `EVENTS_MENU` (or HOME_LOOP), and don't overwrite `latestMenu` with MATSURI_MENU.

### NB-H04: Orderibi delivers food the player can't afford (money goes negative)
- **Status:** Fixed (3100690): in `orderibiFoodSelection()` the PAY_SCREEN setup only runs when money is sufficient; otherwise the toast is shown and the player stays on the Orderibi menu.
- **Where:** `orderibiFoodSelection()` 12650–12659, 12680–12689, 12710–12719. Payment is taken in `managePayment()` (5310).
- **What happens:** The "Not enough money" toast is shown, but the code still goes to PAY_SCREEN. The price is deducted anyway, money goes negative, and the food is delivered.
- **Fix:** In the else branch, stay on or return to the menu instead of calling `changeState(PAY_SCREEN)`.

### NB-H05: Missing or misnamed SD assets freeze the device for minutes
- **Status:** Fixed (b8d05f5 assets, e06e2e4 code): the 33 missing files were copied into `idolnat/` at the exact paths the code requests (26 food icons, 5 onsen backgrounds, flowers_market_front, and the age-21 meditate sprite, which really is 90x135 so only its file name was wrong). `preloadImage()` now returns immediately when `SD.exists()` is false. A re-scan of all 366 image paths in the sketch finds none missing.
- **Where:** `preloadImage()` 1745–1810 retries 30 times with an 8 s delay, about 4 minutes of blocking per missing file.
- **Files the code requests that are not in the repo's `idolnat/` tree** (case-sensitive check):
  - `/idolnat/sprites/food_NNN.png` (26 files). They exist only in `idolnat/sprites/food/`.
    - FOOD_COOK2 (1941–1991) has no fallback.
    - `prepareFoodGrid` (9187–9212) falls back to `sprites/food/` only after the full retry, for each of 8 icons. Opening Cooking can take about 30 minutes.
  - `/idolnat/screens/onsen_{11,13,15,18,21}yo.png` (`onsenBackgroundForAge` 1854–1863). They exist only in the top-level `screens/`.
  - `/idolnat/screens/flowers_market_front.png` (`displayBusinessFront` 13135; travel to the Market). It exists only in the top-level `screens/`.
  - `/idolnat/sprites/natsumi_21yo_meditate-90x135.png` (3268). The real file is `-90x125.png`, so meditating at age 21+ freezes.
- **Fix:** Correct the paths or copy the files into `idolnat/`. Also make `preloadImage` check `SD.exists()` first and fail fast.
- **Caveat:** This assumes the repo's `idolnat/` folder is what gets copied to the SD card.

---

## MEDIUM

### NB-M01: Shared `changeStateCounter` drops transitions and can soft-lock the game
- **Status:** Fixed (4ad6473): `changeState()` remembers the pending delayed target (`pendingState`), ignores requests for other targets while it counts (so they can't overshoot it), and fires on `>=`. The lock-up is gone. Requests for another target made during a pending delay are still dropped, as before; callers that must not lose a transition (e.g. food delivery, NB-M02) need their own handling.
- **Where:** `changeState()` 3597–3604 and 4343–4345.
- **What happens:**
  - Any `changeState(X, 0)` made while a delayed transition is counting is silently dropped.
  - Two calls in the same frame can push the counter past `delay`. It then never equals `delay` again, so the transition never fires.
  - Concrete case: during FOOD_COOK2 (`showFood`, delay = 60), if a key press lands on the frame where the counter reaches 60, the `drawOverlay` any-key handler (9476–9483) bumps it to 61. The game is stuck on the food screen until reboot.
  - PAY_SCREEN has the same problem (`managePayment` plus the overlay handler).
- **Fix:** Keep a pending-target state, compare with `>=`, and reset the counter when the target changes.

### NB-M02: Food delivery can be lost, or can pull the player out of whatever they are doing
- **Status:** Fixed (5d38a91): after 10 pulses the delivery only fires from HOME_LOOP, IDLE_HOME, IDLE_STATS, FOOD/TRAIN/COMP/HEALTH/REST/EVENTS_MENU with no delayed transition pending; otherwise it retries every pulse. The flag is cleared on entering FOOD_ORDER7.
- **Where:** `updateFiveSecondPulse()` 4552–4561.
- **What happens:**
  - The code clears `waitingForFoodDelivery` and the counter, then calls `changeState(FOOD_ORDER7, 0)` from any state. That interrupts cooking, the garden, sleep or meditation (and see M5).
  - If a delayed transition is pending, the call is dropped (M1), and the food the player paid for never arrives.
- **Fix:** Deliver only when the player is at HOME_LOOP or idle with no pending transition; otherwise keep the flag set.

### NB-M03: All submenus share one selection index
- **Status:** Fixed (ccf3598): `manageRoom()` now uses the existing per-menu selection variables (food/training/competition/health/rest/events) and resets an out-of-range index to 0 before `drawMenu()`.
- **Where:** `manageRoom()` 5243–5251 passes `mainMenuSelection` for every menu except Home and Garden. `foodMenuSelection`, `trainingMenuSelection`, etc. (801–807) are declared but never used.
- **What happens:** The selection carries over between menus. For example, after picking item 6 in Training, the 2-item Rest menu has no highlight and ENTER does nothing until the player moves the cursor.
- **Fix:** Use a separate selection variable per `currentMenuType`, or clamp or reset the index when a menu opens.

### NB-M04: Matsuri food and Garapon take the ticket (and feed Natsumi) before Buy/Cancel
- **Status:** Fixed (a9218fc): ticket deduction (and hunger = 4 for food) moved from `matsuriFoodSelection()`/`matsuriMainMenu()` into the Buy branch of `matsuriSale()`; Cancel/ESC cost nothing.
- **Where:**
  - Food: `matsuriFoodSelection()` 12241–12246, 12270–12274 and 12290–12295 deduct the ticket and set hunger = 4.
  - Garapon: `matsuriMainMenu()` 12380–12382 deducts the ticket.
  - The confirmation comes later, in `matsuriSale()` 10880–10909 (MATSURI_COST), which offers Cancel/ESC.
- **What happens:** Choosing Cancel still costs the ticket, and for food Natsumi is still fed.
- **Fix:** Apply the cost and the effect only when Buy is confirmed in `matsuriSale()`.

### NB-M05: Leaving meditation early stops Spirit from ever updating again
- **Status:** Fixed (799dd2c): `changeState()` clears `meditationActive` on every transition out of REST_MEDITATE; the reward is still only granted when the timer completes.
- **Where:** `changeState()` REST_MEDITATE 4141 sets `meditationActive = true`. It is cleared only when the timer finishes (7725). The `drawOverlay` any-key handler (9476–9483) exits to HOME_LOOP without clearing it.
- **What happens:** After any key press (or a food-delivery interruption) during meditation, `updateSpirit()` (4437) skips its recalculation forever, until a full meditation is completed or the device reboots.
- **Fix:** Clear `meditationActive` whenever the player leaves REST_MEDITATE (for example in `changeState`).

### NB-M06: Rock-paper-scissors: an invalid key breaks the round sequence
- **Status:** Fixed (dcd7933): the stage only advances after a valid R/P/S hand (other keys are ignored), and round/score statics are reset on the title screen when a new game starts.
- **Where:** `managePFCGame()` 12503–12529. `pfcCurrentStage += 1` runs before the key is checked.
- **What happens:** Any key other than R, P, S or ESC still advances the stage. The game then jumps to the wrong screen (for example PFC_GAME9 instead of PFC_GAME8). With several stray presses the stage passes 12, no case matches, and the player is stuck until they press ESC.
  - Also, when the player exits with ESC, the static `roundIndex`/wins/draws are not reset, so the next visit's game ends early with stale scores.
- **Fix:** Increment the stage only after a valid hand, and reset the statics on ESC.

### NB-M07: Competition state survives an aborted run
- **Status:** Fixed (d0b1b68): entering COMP_*5 sets `competitionRunStarting`, which makes `manageCompetition()` reset all per-run statics, however the previous run ended. (The mid-run eligibility check itself is unchanged.)
- **Where:** `manageCompetition()` 7421, 7448, 7605–7606.
- **What happens:** Eligibility is re-checked every frame. If a stat drops below 4 mid-run (`updateStats` runs in `manageGame`), the player is sent to COMP_EXPLAIN. The static `competitionInitialized` stays true, so the next attempt resumes the old notes and counters.
- **Fix:** Check eligibility only on entry, and reset the statics whenever the player leaves.

### NB-M08: The screensaver or a visitor interrupts the player while they are using the home menu
- **Status:** Fixed (092d8ff): any key press resets `counterToScreensaver` (in `loop()`), and the HOME_LOOP pulse keeps it at 0 while the home menu is open; timings unchanged.
- **Where:** `updateFiveSecondPulse()` 4602 increments `counterToScreensaver` in HOME_LOOP. It is only reset in `changeState()` 3595–3597 (when not already HOME_LOOP) and in `slideStats` (11595).
- **What happens:** About 55 s after arriving home, the screensaver or a visitor appears even while the player is pressing TAB or the arrows.
- **Fix:** Reset the counter on any key press in HOME_LOOP.

### NB-M09: Rewards and penalties are applied inside draw code and repeat on every redraw
- **Status:** Fixed (8ed80f7): the Conbini/cooking Energy -1, ordering Grace +1, MATSURI_TICKETS2 ticket grant and visitor +10000 money / +1 pear are guarded by `overlayEffectApplied`, reset by `changeState()` on every transition, so each applies once per screen visit.
- **Where (all in `drawOverlay()`):**
  - DOOR_KNOCK7 outcome: +10000 money / +1 pear (10113–10118)
  - FOOD_ORDER9: +1 Grace (9643–9646)
  - FOOD_CONBINI3 and FOOD_COOK2: energy-- (9608–9611, 9822–9825)
  - MATSURI_TICKETS2: `allocateTickets()` (10306)
- **What happens:** Each one runs again whenever `l5NeedsRedraw` is set, which happens on stat ticks and when a toast expires (l0 cascades down to l5). For example, cooking can cost more than 1 Energy, and visitor gifts or tickets can be granted twice.
- **Fix:** Apply state changes once, on state entry, and keep draw functions free of game logic.

### NB-M10: Flower sales never award Grace
- **Status:** Fixed (ba88da1): `miniGameDebrief()` adds +1 Grace (capped at 4) when the FLOWERS_MARKET7 debrief is confirmed, once per completed sale.
- **Where:** The FLOWERS_MARKET7 outcome shows "+1 Grace" (10062–10068), but nothing increments grace for flower sales. The only `grace` increments are at 9644, 11866, 11902 and 11938.
- **What happens:** The README says selling flowers gives Grace, and the screen claims it does, but the player never gets it.
- **Fix:** Add `natsumi.grace` +1 (capped at 4) when the sale completes.

### NB-M11: Minigames and activities have no ESC exit
- **Status:** Fixed (c892af9): ESC (via new `escapePressed()`) aborts sing/dance/swim/gym/run, the countdowns, the library and the flower sale, resets their state and returns to `latestMenu`. No reward, no debrief, no Energy cost. Flowers already sold stay sold.
- **Where:**
  - sing 6096–6119
  - dance 6195–6223
  - swim 6400–6413
  - gym and run
  - library 7183–7224 (150 s)
  - flower sale 7335–7418
- **What happens:** The player is stuck until the activity finishes, although the README says ESC returns.
- **Fix:** Handle key 96: clean up and `changeState(0, latestMenu, 0)`.

### NB-M12: `conbimartVisitsTotal` is not saved
- **Status:** Fixed (47e341d): `conbimart_visits_total` is written to and read from the `[challenges]` section; old saves without the key load with 0, as before.
- **Where:** save 1463–1468 / load 1680–1685. `initializeChallengeProgress()` (637) zeroes it.
- **What happens:** Conbimart visits are lost on reload, so the "Go to Conbimart" challenge step may need another visit.
- **Fix:** Add the counter to the save file.

---

## LOW

- **NB-L01: Sleep/Onsen energy recovery looks inverted.**
  - Status: Open
  - Where: 4591, `random(220) > ((hunger+hygiene+spirit+fitness)*10)+10`.
  - What happens: a healthier Natsumi recovers *less* often (about 22% chance with max stats vs about 95% with zero stats).
  - Fix: flip the comparison (needs the author to confirm the intent).
- **NB-L02: Restaurant age gate doesn't match the README.**
  - Status: Open
  - Where: `drawMenu()` 8108/8196 uses `age > 14` (15+); the README says 16.
  - Fix: `age >= 16`, or update the README.
- **NB-L03: Low-battery toast is re-issued every frame.**
  - Status: Fixed (5a7f3e6): the warning is shown once below 10%, then every 5 minutes, only when no other toast is active; it re-arms at 15% or more, and negative levels are ignored.
  - Where: `printBatteryLevel()` 3443 runs every 50 ms.
  - What happens: below 10% the warning hides every other toast (e.g. "saved"). A negative level is not handled.
  - Fix: rate-limit the warning and ignore values below 0.
- **NB-L04: Cards and Challenges screens: ESC goes to `latestMenu`, not Home.**
  - Status: Fixed (c988444): Cards ESC goes to HOME_LOOP; Challenges ESC goes back to where it was opened from (Home or Events menu), stored in `challengesReturnState`.
  - Where: `browseCards()` 12854, `manageChallenges()` 12970.
  - What happens: opened from Home, ESC lands in whatever menu was used last (for example Food, or the Matsuri trap from H3).
  - Fix: return to HOME_LOOP.
- **NB-L05: Travel-home energy is charged at the wrong time.**
  - Status: Open
  - Where: `prepareTravel()` 13013–13024.
  - What happens: returning to HOME_LOOP from the city doesn't update `currentTile`. The ride home and its -1 Energy happen later, on the next home action (e.g. Sleep or Cook).
  - Fix: update the tile and charge energy when the player actually returns home.
- **NB-L06: Onsen sets Hygiene on exit, not on entry.**
  - Status: Open
  - Where: `manageOnsen()` 11820.
  - What happens: the README says entering the Onsen restores Hygiene.
  - Fix: set it on entry.
- **NB-L07: The outcome screen shows "MAX" instead of "+1".**
  - Status: Open
  - Where: 10044–10093.
  - What happens: the stat is incremented before the check, so going from 3 to 4 displays "MAX".
  - Fix: compare against the value before the increment.
- **NB-L08: Garapon rewards may not be saved, and the currency shows wrongly.**
  - Status: Open
  - Where: 5802/5809/5816.
  - What happens: those cases return before `saveRequired = true`. The result text uses "¥", which the default ASCII font doesn't render; the rest of the UI uses "$".
  - Fix: set `saveRequired` first and use "$".
- **NB-L09: Age caps at 22 and the game never ends.**
  - Status: Open
  - Where: `updateAging()` 4383–4385; the stats bar uses max 22 (10380).
  - What happens: the README describes ages 11–21, but there is no ending.
  - Fix: cap at 21 and add an end state.
- **NB-L10: Saved fields are never read back, and loads are not validated.**
  - Status: Open
  - Where: save 1375–1377; `gameStateFromString` 1306.
  - What happens:
    - `last_*_update` is saved but never loaded.
    - `continueStateLoaded` is never set, so the saved `current_state` is dead.
    - `gameStateFromString` only scans up to COMP_NAT6.
    - Loaded stats are not clamped to 0–4, and the file has no magic/version check.
  - Fix: load or remove those fields, and clamp and validate on load.
- **NB-L11: Swim minigame leftovers.**
  - Status: Open
  - Where: `spawnSwimShark` 6272.
  - What happens: sharks always get a fixed speed of 10, so `getRandomSwimSpeed` is unused. `swimHitFlash` is never set, so the hit effect never shows, and collisions have no penalty.
- **NB-L12: Bath game exit-delay code is dead.**
  - Status: Open
  - Where: `manageBathGame()` 6974–6981.
  - What happens: `finalizeBathOutcome` changes state immediately, so the delay never runs.
- **NB-L13: The Pantry Snack fallback can never trigger.**
  - Status: Open
  - Where: `cookSelectedRecipe()` 9133–9142.
  - What happens: every single food is also a one-item "recipe", so the fallback is unreachable, and eating one raw item counts as "Cook a recipe" for the challenge.
- **NB-L14: Competition rules don't match the README.**
  - Status: Open
  - Where: 7489–7522 and 7562–7571.
  - What happens: a single missed note fails the competition, while the README implies misses are allowed and only a perfect run grants Popularity. Also, notes can still be caught after they pass the player row.
- **NB-L15: NEW_GAME doesn't reset all globals.**
  - Status: Open
  - Where: 3652–3734 and 3741–3822.
  - What happens: `gardenTiles`/`gardenActive`, `waitingForFoodDelivery`, `foodDeliveryCounter`, `birthdayVisitEnabled`, `recentCompWin` and `isLatestTrainingPerfect` are not reset. This matters only after H2 or similar.
  - Fix: add a shared `resetGameState()`.
- **NB-L16: Neko Cafe reading frames always use the 11-year-old sprites** (11409–11411).
  - Status: Open
- **NB-L17: Timers are not rollover-safe.**
  - Status: Open
  - Where: `drawToast` `millis() > toastUntil` (7860), `manageStats` (10643), and the dance cue timer.
  - What happens: they misbehave after about 49.7 days of uptime.
  - Fix: use `millis() - start >= duration`.
- **NB-L18: Minor logic oddities.**
  - Status: Open
  - `gardenFlowerStageIndex` returns -1 for value 30 (5406–5407).
  - `isChallengeStepDone` indexes `challengeFlags` before its bounds check (523).
  - NEKO_CAFE15–19 and COMP_STATUS are unused states.
- **NB-L19: Text typos.**
  - Status: Open
  - "ssessions" (9835)
  - "Washed feels better" (7613)
  - "Carbunkle Cocktail" (409)
  - the red price text "$450(S) / $550(L)" doesn't match the actual $400/$550 (9948)
  - the Events menu says "CHALLENGES" where the README says "QUESTS" (787)
