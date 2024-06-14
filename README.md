# TWWHD-Randomizer
As this is a new randomizer, there may still be bugs until it is more thoroughly tested. Some of the information below may change with time.

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

It shuffles all the items, makes the world open from the start, removes most cutscenes, and adds other tweaks to speed up mundane tasks. While most features from the [original Wind Waker Randomizer](https://github.com/LagoLunatic/wwrando) are present, some are yet to be ported due to the remake's changes.

### Getting Stuck
If you can not find anywhere to progress, you should first check the spoiler log. The spoiler log is generated in the randomizer's save directory (`sd:/wiiu/save/<8 hex digits> (TWWHD Randomizer)` on console) and contains information on everything in the seed.

If you have checked the log and are still stuck, it is possible you have encountered a [bug](#Reporting-Bugs).

### Reporting Bugs
If you seem to have discovered a bug, let us know in our [Discord server](https://discord.gg/wPvdQ2Krrm) or by [opening an issue](https://github.com/SuperDude88/TWWHD-Randomizer/issues). Be sure to share the spoiler log for the seed when reporting a problem.

## Setup
You can download the latest stable version of the randomizer from the releases page. 

This randomizer currently supports the USA version of TWWHD. The European and Japanese versions of the game will not work, nor will the original Wind Waker.

### On Console
#### Preparing Homebrew
Playing the randomizer on console requires a Wii U with the Aroma environment and sigpatches. A guide for installing Aroma can be found [here](https://wiiu.hacks.guide). After following the guide, you will need to add [01_sigpatches.rpx](https://github.com/marco-calautti/SigpatchesModuleWiiU/releases) to `sd:/wiiu/environments/aroma/modules/setup`. Make sure to read each page of the guide closely, and pay attention to which environment you are booting into.

#### Using the Homebrew Application
The patcher requires an accessible version of TWWHD (USA) on your Wii U (digital install or inserted disc). Discs have not yet been tested, but if they are not working, you can dump installable files using [wudd](https://github.com/wiiu-env/wudd). You must also have 2GB of space available for the randomized channel, either on the console's internal memory or a connected USB storage device (*\*This storage is reserved after first-time setup*).

Once homebrew is set up, copy the `wwhd_randomizer.wuhb` from the releases page into `sd:/wiiu/apps`. Run the app from the home menu to open the patcher.

The first time it runs, the patcher will create a home menu channel for the randomized game. This can take some time (it needs to transfer all the game data), but only happens once. Subsequent randomizations do not take as long.

#### Running the Game
To play the randomized game, you must have a CFW/sigpatches active. This should always be the case as long as you loaded Aroma with sigpatches. The randomized channel will have an orange, mirrored icon of the vanilla game.

### On Emulator
This method will require that you have a decrypted USA copy of TWWHD on your computer. The decrypted game consists of a folder with 3 subfolders (`code`, `content`, and `meta`). It is *not* a single file like an `.iso` or a `.wad`. You can dump a disc or digital install from your Wii U with [dumpling](https://cemu.cfw.guide/using-dumpling.html).

Download the latest randomizer from the [releases page](https://github.com/SuperDude88/TWWHD-Randomizer/releases) and extract its contents to wherever you wish (Windows and Mac are currently supported). After extraction, double-click on the `wwhd_rando` application to open it (`wwhd_rando.exe` on Windows). Set the `Base Game Folder` to the folder which holds your vanilla game and the `Output Folder` to the location you want to place the randomized game. Once you've finished selecting your desired settings, click the `Randomize` button in the bottom-right and wait for the seed generation to finish. Afterwards, you will have the randomized game in the output folder. Note that generating a new seed will overwrite any previous seed in the output folder.

Open up your emulator of choice (probably [Cemu](https://github.com/cemu-project/Cemu)) and add your output folder to the emulator's game paths to have it pick up the randomized game. The randomized game will have an orange, mirrored icon of the vanilla game. Now just click on it in the emulator to load it up. Enjoy!

### Building
#### The Wii U Homebrew Application
To build for Wii U, use the dockerfile and run the image.
From the source directory, this would be:

```
docker build -t <image-tag> .
docker run -i -e BUILD_TYPE=randomizer -v "$(pwd):/src" <image-tag>
```

You can also change the build type to "asm" to rebuild the assembly patches, or "full" to build the assembly and the randomizer.

The randomizer program (wuhb) will be placed in the build folder.

#### The Desktop Application
To build the Desktop Application, download and install the latest version of [Qt](https://www.qt.io/download-qt-installer-oss). Make sure to install the correct kit for your platform. After opening up Qt Creator, select `Open Project` and select the `CMakeLists.txt` file in the repo root. When prompted to select a kit, select the one you installed earlier. Define `QT_GUI` in the project's CMake configuation, then press the green play button in the bottom left of the application to build and run the project.

## Credits
All of this was made possible thanks to tremendous help from [csunday95](https://github.com/csunday95) and [gymnast86](https://github.com/gymnast86), along with bugfixing assistance from the devs at [ForTheUsers](https://fortheusers.org/). Translations were done by azer67 (French), Cithiel (Spanish), and Nacho (Spanish). Much of the work here was based on the research and code already written by the [original randomizer team](https://github.com/LagoLunatic/wwrando#credits).
