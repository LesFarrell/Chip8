![/images/splash2.png](/images/splash.png)

# SuperChip Emulator

This is my attempt at writing a Chip8 / SuperChip emulator in C.
I think every programmer should at least attempt to create a Chip8 emulator as it serves as the perfect introduction to how computers work at a lower level.

The Emulator is using the cross platform Tigr Graphics Library https://github.com/erkkah/tigr to handle the Window creation and process keypresses. It's single C file which makes it very easy to use and debug and it's perfect if you just want to get something on screen quickly!

It should be fairly easy to convert the emulator code itself to a different platform as the only thing that's platform specific is the Windows File browser and some optional Windows console code but that should be easy to remove or replace.

### Commands
In addition to the normal CHIP8 keys of:

| **1** | **2** | **3** | **4** |
| **q** | **w** | **e** | **r** |
| **a** | **s** | **d** | **f** |
| **z** | **x** | **c** | **v** |

### you can use the following keys:

**O** - to browse for a ROM file.
**L** - Reload the current ROM Image.



I've tried the emulator with quite a few games and most seem to work without to many problems.

![/images/sweetcoptor.png](/images/sweetcoptor.png)
![/images/outlaws.png](/images/outlaws.png)
![/images/sokoban.png](/images/sokoban.png)
