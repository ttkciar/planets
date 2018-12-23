# planets

Multi-player text game from a 1980s shared UNIX computing environment

## Game Play

To be written.

## Empires

To be written.

## Planets

To be written.

## Ships

To be written.

## User Commands

You don't need to be enrolled to use these commands.

### alias [ *alias* [ *command* ] ]

With two or more arguments, `alias` allows you to define an alias for a more complex command.

With one argument, `alias` reminds you what you defined the specified alias as.

With no arguments, `alias` displays a list of your aliases.

### dist *planet_num* *planet_num*

Calculate the distance between two specified planets.

### echo [ -n ] *words...*

Print the arguments. Prints a newline after the words, unless you use the `-n` option.

### enroll 

Add yourself to the game. You are assigned a random unused empire and a random unused planet.
You will be prompted to give your empire a name.
Your home planet will be initialized with random defense, tech, and production.

### help [ *topic* ]

With one argument, `help` displays documentation on the named topic or command.

With no arguments, `help` displays a list of commands.

### history

Displays your previous commands.

### next

Displays how long in hours, minutes, and seconds until the next turn starts.

### plot [ *planet_num* ]

Re-center the map on the specified planet number.
If you don't specify a planet, the map is centered on your original planet.

### quit (or q)

Exit the planets program. You can re-enter any time to continue managing your empire.

### range [ *planet_num* [ *how_many* ] ]

Displays planets in range, showing the planet number, name, x, y, and the distance.

With no arguments, it defaults to show 10 planets within range from your empire's home planet.

With one argument, it shows 10 planets within range from the specific planet number.

With two arguments, it shows your choice of how many planets.

### score

Displays the score of all empires. For each empire, it shows the empire name,
the number of planets controlled by that empire,
the total util of all the empire's planets,
the number of ships controlled by the empire,
the total resource worth of planets in the empire,
the amount of planetary development invested,
and the total of worth and investment.

### set [ *setting* [ *value* ]]

Allows you to display or change settings for your usage of the game.

- prompt - the string before you enter commands
- noverbose - turn off verbose output
- verbose - turn on verbose output

With no arguments, this displays your current prompt and other settings.

## Empire Commands

You must be enrolled in the game to use these commands.

### build < p | d | t > *planet_num* *amount*

Order the specified planet (or `*` for all planets) to build `amount`
in either production (`p`), defense (`d`), or technology (`t`).

### direct *ship_num* *planet_num*

Send one of your ships on a journey to a destination planet.

### direct *ship_num* *x* *y*

Send one of your ships on a journey to a coordinate.

### fleet [ -d ] *planet_num*

Display ships in your fleet.

Use the *planet_num* to display ships that are residing at the specified planet.

Use the `-d` flag to display ships en route to the specified planet.

### fleet < -b | -c | -s >

Display ships in your fleet.

Use the `-b`, `-c`, `-s` flags to display only battle ships, colony ships, or scout ships, respectively.

### log *planet_num*
### log -s *ship_num*

Displays all or a portion of the log file for a specified ship num or planet num.

### make *planet_num* < b | c | s > [ *units* ]

Make a ship at a planet you control.
Choose `b` for a battle ship, `c` for a colony ship, or `s` for a scout ship.
The planet must have enough reserves to provision a ship with the units you choose.
Battleships are provisioned with defense units,
colony ships are provisioned with residents,
and scout ships are provisioned with units of stealth.

### max < p | d | t > *planet_num*

Like `build`, but order the planet(s) to build their maximum amount.

### name *planet_num* *new_planet_name*
### name -s *ship_num* *new_ship_name*

Give a name to one of your planets or ships.

### planets [ *planet_num* ]

With one argument, display status of the specified planet num.

With no arguments, display status of all your empire's planets.

### scan

Display a scan of all ships seen by a planet you control.

### ship *ship_num*

Display status of the specified ship you control.

### source *file_name*

Read commands from the specified file.

### telegram [ -a ] [ *empire_name* ]

Send mail to the named empire.

You will be prompted to type a full message, ending with Ctrl-D.

You can address mail to all players by using "Players" as the recipient name.

You can address mail to the game administrator by using "Manager" as the recipient name.

### universe *printer*
### universe > *filename*

Output a complete game report to a printer or file.

### update < r | d | t | s > *ship_num* *amount*

Update a ship's points. The ship must be at rest at a planet.

You can update different resource types:

* When the resource is residents (`r`), the ship must be a colony ship.
Residents can be moved from the planet to a ship, or from a ship to a planet.

* When the resource is defense (`d`), the ship must be a be a battle ship.
Defense can only be increased. Ships have a maximum defense of 35.

* When the resource is stealth (`s`), the ship must be a scout ship.
Stealth can only be increased. Ships have a maximum stealth of 35.

* When the resource is tech (`t`), the ship can be any type.
Tech can only be increased.

You can choose the amount of resources:

* When the amount is a positive integer, resources from the planet are added to the ship.
The planet must have enough reserves to give the desired resources to your ship.

* When the amount of residents is `+`, all residents on the planet are added to the colony ship.

* When the amount is a negative integer, residents on the colony ship are added to the planet.

* When the amount is `-`, all residents on the colony ship are added to the planet.

## User Configuration File

To be written.

## Telegram Commands

To be written.

## Administrator commands

You must be the administrator to use these commands.

### empires

Display a list of empires and their player uid and login name.

### enroll *uid* *empire_num* *planet_num*

The master of the game can enroll a specified uid, creating an empire, and giving them the specified planet.

### hdr

Display the game header, including:

* up_last - last time a turn was done.
* up_time - time in seconds between turns.
* up_num - how many turns so far in the game.
* max_fleet - max number of ships any empire can control.
* ship_top - highest ship number used.

### init

Initialize a new game:
* Remove all planets from empire control.
* Set planetary resources randomly.
* Destroy all ships.
* Set the interval between turns to 6 hours.

### interval [ *seconds* ]

With one argument, set the interval between turns to the specified number of seconds.

With no arguments, display the current interval in minutes.

### lfiles

Initialize the telegram files for all empires.

### mark

Mark the game's last turn time to the current time.

### max_fleet *num*

Set the game's maximum number of ships to the specified positive integer.

### move *planet_num* *newx* *newy*

Move a planet's location to the new X, Y coordinates.

### rename *empire_num* *new_empire_name*

Rename an empire num to the specified new name.

### tfiles

Initialize the telegram files for all empires.

### turn

Advance the game by one turn.

### un_nuke *planet* *name* *emp* *x* *y* *util* *prod* *def* *tech* *res*

Restore a planet. Assign to it a name, empire owner, X, Y coordinates, and production and other stats.

### unlock

Clear the lock. This is used if the game crashes after locking the data files.

## Global Configuration File

To be written.

## Credits

This game and its source code is public domain.

* Charles Rand, original version at U.C. Berkeley.

* Chuck Peterson and Keith Reynolds (1986-04-07),
  completely re-written for the U.C. Santa Cruz Game Shell.
 
* Bill Karwin (2018-12-09), ported to C11, wrote new documentation,
  added command-line options and global config file.
