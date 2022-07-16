# TWWHD-Randomizer
# This is currently very broken and crashes on console

* [Information](#Information)
  * [Getting Stuck](#Getting-Stuck)
  * [Reporting Bugs](#Reporting-Bugs)
* [Setup](#Setup)
  * [On Console](#On-Console)
  * [On Emulator](#On-Emulator)
* [Credits](#Credits)

## Information
A Randomizer for *The Legend of Zelda: The Wind Waker HD*.

It shuffles all the items, makes the world open from the start, removes most cutscenes, and adds other tweaks to speed up mundane tasks. While most features from the original Wind Waker Randomizer are present, a few are still missing due to the remake's code changes.

Settings are currently changed through the config.yaml, but a GUI is planned for a later date.

### Getting Stuck
If you can not find anywhere to progress, you should first check the spoiler log. The spoiler log is generated in the same folder as the randomizer program (SD:/wiiu/apps/randomizer) and contains information on everything in the seed.

If you have checked the log and are still stuck, it is possible you have encountered a [bug](#Reporting-Bugs).

### Reporting Bugs
If you seem to have discovered a bug, let us know in our [Discord server](TODO) or by opening an issue. Be sure to share the config file for the seed when reporting a problem.

## Setup
You can download the latest stable version of the randomizer from the releases page. 

This randomizer currently supports the USA version of TWWHD. The European and Japanese versions of the game will not work. Additionally, the randomizer requires the game to be installed to the console storage (not a USB or SD card). If you have a physical copy, you can create a digital install using [disc2app](https://github.com/koolkdev/disc2app).

### On Console
Running the randomizer requires a homebrewed Wii U. A guide for this can be found [here](https://wiiu.hacks.guide/#/). You must have roughly 3 to 3.5GB of free space on your SD card for the randomizer's data.


Once homebrew is set up, copy the "randomizer" folder to "SD:/wiiu/apps". The randomizer folder should contain a .rpx file, 3 folders (asm, assets, and logic), and a config.yaml. Run the randomizer from the homebrew menu to patch the game. To play the randomized game, you must have a CFW active. If you installed homebrew recently, this should happen automatically, as long as you booted homebrew after the Wii U was restarted.

The first time it runs, the randomizer will create a backup of the game on your SD card. This can take some time, but only happens once. **Once this folder is created, do not touch it. Deleting or modifying it will force you to reinstall the game from the eShop or disc to play the unmodified version.**

### On Emulator
This randomizer does not currently have a build for Windows, Mac, or Linux. The code does work on a computer but requires you to set the correct paths in the config. The base game must be an unmodified, decrypted USA copy, and the output directory must be another copy of the game that will be overwritten. For faster randomization, you can change the number of threads in `server/command/RandoSession.cpp` to match the thread count of your system.

## Credits
All of this was made possible thanks to tremendous help from csunday95 and gymnast86, along with the research and code already written by the [original randomizer](https://github.com/LagoLunatic/wwrando) team.
