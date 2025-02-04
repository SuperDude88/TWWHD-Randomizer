# Custom Model Installation

This file explains how to install a custom model into the randomizer. Support is
currently experimental and may softlock your game!

## Extracting Necessary Files

Download any custom model from Gamebanana and extract the zip file. You will
probably find a bunch of folders and from there we want to find some files :

- Either a `permanent_3d.pack` file or a `Link.szs` file (ideally the latter).

- Files with the following names, if present:
  - `Jailnit.aaf`
  - `voice_0.aw`

If you could find them within a folder with a name like `Raw Files` or
`Extra Files`, great! Otherwise, you may need to dig around folders with names
like `Cafe` or `Common`. The `Link.szs` file is essential; the others are
optional and the model creator might not have provided them.

If you have `permanent_3d.pack` and not `Link.szs`, we'll need just a few more
steps. You'll need to use
[Toolbox](https://github.com/KillzXGaming/Switch-Toolbox/releases/tag/Final) to
export a `Link.szs`. Simply open the `.pack` file in Toolbox, scroll to find
`Link.szs`, right-click and `Export Raw Data`.

## Installing on Console

Access your SD card (either directly or over FTP) and find the save data for the
randomizer app, usually `sd:/wiiu/apps/save/<8 hex digits> (TWWHD Randomizer)`.
Inside, find a folder called `custom_models`. Create a new directory with your
model's name and place `Link.szs`, along with the other, optional files.

You can now load the randomizer app on your Wii U and select the model from the
Color tab.

Randomize the game, and you'll find link has changed into someone else! Have
fun!
