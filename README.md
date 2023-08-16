# TWWHD-Randomizer
Game may still be unstable until it is more thoroughly tested. Everything here is subject to change.

* [Information](#Information)
  * [Getting Stuck](#Getting-Stuck)
  * [Reporting Bugs](#Reporting-Bugs)
* [Setup](#Setup)
  * [On Console](#On-Console)
  * [On Emulator](#On-Emulator)
  * [Building](#Building)
* [Credits](#Credits)

## Information
A Randomizer for *The Legend of Zelda: The Wind Waker HD*.

It shuffles all the items, makes the world open from the start, removes most cutscenes, and adds other tweaks to speed up mundane tasks. While most features from the original Wind Waker Randomizer are present, a few are still missing due to the remake's changes.

### Getting Stuck
If you can not find anywhere to progress, you should first check the spoiler log. The spoiler log is generated in the randomizer's save directory (`sd:/wiiu/save/<8 hex digits> (TWWHD Randomizer)` on console) and contains information on everything in the seed.

If you have checked the log and are still stuck, it is possible you have encountered a [bug](#Reporting-Bugs).

### Reporting Bugs
If you seem to have discovered a bug, let us know in our [Discord server](TODO) or by opening an issue. Be sure to share the config file (excluding file paths) for the seed when reporting a problem.

## Setup
You can download the latest stable version of the randomizer from the releases page. 

This randomizer currently supports the USA version of TWWHD. The European and Japanese versions of the game will not work, nor will the original Wind Waker.

### On Console
Running the randomizer requires a Wii U with the Aroma environment and sigpatches. A guide for installing the EnvironmentLoader can be found [here](https://wiiu.hacks.guide/#/). After setting up the EnvironmentLoader, you can download Aroma [here](https://aroma.foryour.cafe/), and add [01_sigpatches.rpx](https://github.com/marco-calautti/SigpatchesModuleWiiU/releases) to `sd:/wiiu/environments/aroma/modules/setup`.

To generate a patched game, the randomizer requires a digital install of TWWHD (on NAND or USB). In theory it supports discs, but it has not been tested. If you have a physical copy and discs aren't working (expected), you can dump installable files using [wudd](https://github.com/wiiu-env/wudd). You must also have 1.5GB (TODO: check this number, also make the console do it for me) of console space available for the randomizer channel (*\*This storage is reserved after first-time set up*).

Once homebrew is set up, copy the `wwhd_randomizer.wuhb` into "sd:/wiiu/apps". Run the randomizer from the home menu to patch the game. To change settings, replace or edit the `config.yaml` in the randomizer's save directory (`sd:/wiiu/save/<8 hex digits> (TWWHD Randomizer)`). There is currently no GUI for the homebrew app.

To play the randomized game, you must have a CFW/sigpatches active. This should always be the case as long as you loaded Aroma with sigpatches.

The first time it runs, the randomizer will create a home menu channel for the randomized game. This can take some time (it needs to transfer all the game data), but only happens once.

### On Emulator
A version of the randomizer with a GUI is available for Windows (Mac and Linux are untested, broken(?)). The base game must be an unmodified, decrypted USA copy, and the output directory must be another copy of the game that will be overwritten. You can get a compatible dump of the game from a console with [dumpling](https://github.com/emiyl/dumpling).

### Building
To build for Wii U, use the dockerfile and run the image.
From the source directory, this would be:

```
docker build -t <image-tag> .
docker run -i -e BUILD_TYPE=randomizer -v "$(pwd):/src" <image-tag>
```

You can also change the build type to "asm" to rebuild the assembly patches, or "full" to build the assembly and the randomizer.

The randomizer program (wuhb) will be placed in the build folder.

## Credits
All of this was made possible thanks to tremendous help from [csunday95](https://github.com/csunday95) and [gymnast86](https://github.com/gymnast86), along with much bugfixing assistance from the devs at [ForTheUsers](https://fortheusers.org/). Everything here was based on the research and code already written by the [original randomizer team](https://github.com/LagoLunatic/wwrando#credits).
