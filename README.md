# Idol-Natsumi
Manage the daily life and artistic career of Natsumi Hasegawa, a Japanese idol from the Shōwa era.

## Presentation

*Idol Natsumi* is a Tamagotchi like game, with a short game loop, simple mechanics, that can be played on-the-go. The player has to be able to progress in the game even by just playing it for 5 minutes.

In *Idol Natsumi*, the player takes care of and raises a Japanese girl named Natsumi Hasegawa from age 11 to age 21. Her goal in life is to become a popular singer in Japan.

## Core mechanics

### General navigation

* **TAB** key open or closes the main menu
* **ESCAPE** key goes back to Home screen
* **ENTER** key validates whatever option is selected
* **UP/DOWN** Navigate menus

### The 4 stages of Natsumi's life

* Between 11 years old and 13 years old: preteen - No access to Restaurant, Order and Competition
* Between 13 years old and 15 years old: teenager phase 1 - Gains access to Order and local Competition
* Between 15 and 18 years old: teenager phase 2 - All limits are lifted
* Between 18 and 21 years old: young adult

Age progression is automatic, Natsumi ages one year every 24 hours.

### Statistics

Stat caps: 0 is the lowest while 4 is the highest.

* **Hunger** (integer; 0 to 4; indicates how hungry Natsumi is)
* **Hygiene** (integer; 0 to 4; indicates how clean Natsumi is)
* **Energy** (integer; 0 to 4; indicates how energized Natsumi is)
* **Spirit** (integer; 0 to 4; indicates the general mood of Natsumi)
* **Popularity** (integer; 0 to 4; indicates how popular Natsumi is)
* **Performance** (integer; 0 to 4; shows how good of an entertainer Natsumi is)
* **Fitness** (integer; 0 to 4; indicates how healthy Natsumi is)
* **Culture** (integer; 0 to 4; indicates how knowledgeable Natsumi is)
* **Charm** (integer; 0 to 4; shows how well-mannered and likeable Natsumi is)


| STAT          | TYPE       | RANGE | UP (activities or events that raise it)                                   | DOWN (causes that lower it)                                      |
|----------------|------------|--------|----------------------------------------------------------------------------|------------------------------------------------------------------|
| **Age**        | Passive    | 0–∞    | Increases automatically every `agingInterval` ms                          | —                                                                |
| **Hunger**     | Passive    | 0–4    | COOK, RESTAURANT, ORDER                                                   | Decreases by 1 every `hungerInterval` ms                         |
| **Hygiene**    | Passive    | 0–4    | WASH, ONSEN                                                               | Decreases by 1 every `hygieneInterval` ms                        |
| **Energy**     | Passive    | 0–4    | GARDEN, TEMPLE, ONSEN, NAP, BEDTIME                                       | Decreases by 1 every `energyInterval` ms                         |
| **Spirit**     | Derived    | 0–4    | Improves when Hygiene, Energy, Hunger, Performance, or Popularity are high or through meditation | Declines when those stats are low                                |
| **Popularity** | Active     | 0–4    | COMPETITION (LOCAL, DEPARTMENTAL, REGIONAL, NATIONAL)                     | —                                                                |
| **Performance**| Active     | 0–4    | TRAINING → SING, DANCE                                                   | —                                                                |
| **Fitness**    | Passive    | 0–4    | TRAINING → SWIM, GYM, WALK                                               | Decreases by 1 every time Age increases by 1 year                |
| **Culture**    | Active     | 0–4    | TRAINING → LIBRARY                                                      | —                                                                |
| **Charm**      | Active     | 0–4    | FOOD → COOK, RESTAURANT, ORDER                                           | —                                                                |

