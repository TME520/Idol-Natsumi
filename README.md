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

### The 4 stages of Natsumi's life

* Between 11 years old and 13 years old: preteen - No access to Restaurant, Order and Conpetition
* Between 13 years old and 15 years old: teenager phase 1 - Gains access to Order and local Competition
* Between 15 and 18 years old: teenager phase 2 - All limits are lifted
* Between 18 and 21 years old: young adult

Age progression is automatic, Natsumi ages one year every 24 hours.

### Statistics

* **Hunger** (integer; 0 to 4; indicates how hungry Natsumi is)
* **Hygiene** (integer; 0 to 4; indicates how clean Natsumi is)
* **Energy** (integer; 0 to 4; indicates how energized Natsumi is)
* **Skill** (integer; 0 to 4; indicates how good of a performer Natsumi is)
* **Mood** (integer; 0 to 2; indicates how good Natsumi feels)
* **Popularity** (integer; 0 to 4; indicates how popular Natsumi is)

Stat caps: 0 is the lowest while 4 is the highest. These stats are exclusively influenced by what the player does.

* Hunger: goes down by one every 2 hours. If reaches 0, Natsumi looses all her energy and her mood goes down by 1
* Hygiene: goes down by one every 4 hours. If reaches 0, Natsumi looses all her energy and her mood goes down by 1
* Energy: goes down by one every 4 hours. If reaches 0, Natsumi can't train and/or compete
* Skill: goes up by training and/or competing (0: Natsumi is limited to local competition, 1 & 2: Natsumi is limited to regional competition, 3+: Natsumi can participate in national competitions)
* Mood: goes up by one after every player interaction; goes down by one every hour. If reaches 0, Natsumi looses all her energy
* Popularity: goes up after every successful competition

Competition success is tied to mini games in a way that is yet to be determined.
Energy gets restored through food and/or visits to the temple.
Nap restores 2 Energy points while bedtime fully restores Energy.

### Main menu

* 0: Display **statistics**
* 1: **Food**: Cook/Restaurant/Order <- Influences Hunger
* 2: **Train**: Singing/Dancing/Sport <- Mini games
* 3: **Compete**: Participate into local/departmental/regional/national competitions <- Influences popularity
* 4: **Health**: Wash/Go to the temple/Go to doctor
* 5: **Rest**: Nap/Bedtime
* 6: **Gardening**

