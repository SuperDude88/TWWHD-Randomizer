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
Playing the randomizer on console requires a Wii U with the Aroma environment and sigpatches as well as at least 2 GB of free space on your Wii U's internal storage or a connected USB storage device. A guide for installing the EnvironmentLoader can be found [here](https://wiiu.hacks.guide/#/). After setting up the EnvironmentLoader, you can download Aroma [here](https://aroma.foryour.cafe/), and add [01_sigpatches.rpx](https://github.com/marco-calautti/SigpatchesModuleWiiU/releases) to `sd:/wiiu/environments/aroma/modules/setup`.

Generating a seed to play on console can be done in one of two ways as explained below:

#### Using the Desktop Application (Currently Windows Only)
This method will require that you have a decrypted USA copy of TWWHD on your computer. The decrypted game consists of a folder with 3 subfolders (`code`, `content`, and `meta`). It is *not* a single file like an `.iso` or a `.wad`. If you have the vanilla game digitally installed, you can dump it with [dumpling](https://github.com/emiyl/dumpling).

After you've setup homebrew on your console, download the latest `wwhd_rando_windows.zip` from the releases page and extract its contents to wherever you wish. At this point we recommend creating 2 new folders, `rando` and `rando console`, in the folder where you extracted the application to. These will hold the decrypted randomized game and the encrypted console package respectively. Now, copy the vanilla decrypted game (`code`, `content`, and `meta`) into the `rando` folder. This is so the application doesn't have to copy over the entire game to the `rando` folder every time you generate a seed and significantly reduces generation time. 

Next, double-click on `wwhd_rando.exe` to open the application. Set the `Base Game Folder` as the folder which holds your vanilla game folder and the `Output Folder` as the `rando` folder we created earlier. Once you've finished selecting your desired settings, go to the `Console` tab in the application and select `Repack for Console`. Set `Console Output Folder` as the `rando console` folder we created earlier. Now click the `Randomize` button in the bottom-right and wait for the seed generation to finish. Afterwards, you will have an installable console package in the `rando console` folder. Note that generating a new seed will overwrite the previous seed that was generated.

To install the console package we're going to use WUP Installer GX2. Download the latest `wup_installer_gx2.wuhb` from [here](https://github.com/Fangal-Airbag/wup-installer-gx2/releases/latest) and copy it to the `sd:/wiiu/apps` folder on your SD card. Next, create a new folder `install` on the root of your SD card (if it doesn't already exist) and copy the `rando console` folder into this one. Your SD card should now look like `sd:/install/rando console/(encrypted game files)`. Insert your SD card back into your console and power it on. If you previously installed a randomizer seed, we recommend deleting it first before installing the new one. You can do this by going into the System Settings > Data Management > Copy/Move/Delete Data and deleting the title from there. Next, load up the Aroma environment you installed earlier by going into the Health and Safety Information app and selecting aroma from the list. Now select WUP Installer GX2 from your Wii U home menu and after it loads select `rando console`. Select `install` and then choose where you want to install the game. The installation process will take a few minutes. Once it's done, press the Home Menu button to exit out of WUP Installer GX2 and you should see the randomized game on your Wii U home menu. Enjoy!

#### Using the Homebrew Application
This method requires an already existing digital install of TWWHD (USA) on your Wii U. In theory it supports discs, but this has not been tested. If you have a physical copy and discs aren't working (expected), you can dump installable files using [wudd](https://github.com/wiiu-env/wudd). You must also have 2GB (TODO: check this number, also make the console do it for me) of console space available for the randomizer channel (*\*This storage is reserved after first-time set up*).

Once homebrew is set up, copy the `wwhd_randomizer.wuhb` fro the releases page into `sd:/wiiu/apps`. Run the randomizer from the home menu to patch the game. To change settings, replace or edit the `config.yaml` in the randomizer's save directory (`sd:/wiiu/save/<8 hex digits> (TWWHD Randomizer)`). There is currently no way to configure settings directly in the homebrew app.

To play the randomized game, you must have a CFW/sigpatches active. This should always be the case as long as you loaded Aroma with sigpatches.

The first time it runs, the randomizer will create a home menu channel for the randomized game. This can take some time (it needs to transfer all the game data), but only happens once.

### On Emulator
This method will require that you have a decrypted USA copy of TWWHD on your computer. The decrypted game consists of a folder with 3 subfolders (`code`, `content`, and `meta`). It is *not* a single file like an `.iso` or a `.wad`. If you have the vanilla game digitally installed, you can dump it with [dumpling](https://github.com/emiyl/dumpling).

Download the latest `wwhd_rando_windows.zip` from the releases page and extract its contents to wherever you wish. At this point we recommend creating a new folder `rando` in the folder where you extracted the application to. This will hold the randomized game. Now, copy the vanilla deencrypted game (`code`, `content`, and `meta`) into the `rando` folder. This is so the application doesn't have to copy over the entire game to the `rando` folder every time you generate a seed and significantly reduces generation time. 

Next, double-click on `wwhd_rando.exe` to open the application. Set the `Base Game Folder` as the folder which holds your vanilla game folder and the `Output Folder` as the `rando` folder we created earlier. Once you've finished selecting your desired settings, click the `Randomize` button in the bottom-right and wait for the seed generation to finish. Afterwards, you will have the randomized game in the `rando` folder. Note that generating a new seed will overwrite the previous seed that was generated.

Open up your emulator of choice (probably CEMU) and add the `rando` folder to the emulator's game paths to have it pick up the randomized game. The randomized game will have an orange, mirrored icon of the vanilla game. Now just click on it in the emulator to load it up. Enjoy! 

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
To build the Desktop Application, download and install the latest version of (Qt)[https://www.qt.io/download]. Make sure to install the Desktop MingW Kit when installing. After opening up Qt Creator, select `Open Project` and select the `CMakeLists.txt` file in the `gui` folder of the repo. When prompted to select a kit, select the MingW one you installed earlier. Then press the green play button in the bottom left of the application to build and run the project.

## Credits
All of this was made possible thanks to tremendous help from [csunday95](https://github.com/csunday95) and [gymnast86](https://github.com/gymnast86), along with much bugfixing assistance from the devs at [ForTheUsers](https://fortheusers.org/). Everything here was based on the research and code already written by the [original randomizer team](https://github.com/LagoLunatic/wwrando#credits).
