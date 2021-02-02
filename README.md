# Burnout Paradise Road Rule Limit Remover
Removes minimum time limit of 2 seconds from Time Road Rules and maximum score limit of $1000000000 from Showtime Road Rules by modifying the game's executable.

This would normally be done via dll, but since some cross-platform capability is nice and I don't feel like making sprx mods, this is how it's gonna be.

## Required tools for limit removal on console
RRLR will not decrypt or decompress executables, nor will it create a CON/PIRS/LIVE or PS3/PS4 package. You will need to do this yourself.

While there is no encryption or compression on PC, there is on console. Most PS4 dumpers decrypt/decompress the executable by default, so this section will only cover PS3 and X360.

PS3 executables can be decrypted and resigned using [TrueAncestor SELF Resigner](https://www.psx-place.com/resources/trueancestor-self-resigner-by-jjkkyu.33/). The command-line GUI is fairly straightforward - tpyically, you would choose option 1 to decrypt and 2 or 3 or resigning. If you are using a PSN version, you will need to provide a RAP file for decryption.

X360 executables can be decrypted etc using [xextool](http://xorloser.com/blog/?p=395). This is run via command line. A typical decryption/decompression command looks like `xextool -c u -e u c:\default.xex`, while the reverse would use `-c c -e e` instead.

If you need them, there are much better tutorials for package creation than what I could write (use Google), but as a start, some apps on Xbox are [Velocity](https://community.wemod.com/t/release-velocity-xbox360-cross-platform-file-manager-v0-1-0-0/2623), [Le Fluffie](https://gbatemp.net/download/le-fluffie.28975), and [Horizon](https://www.wemod.com/horizon), while PS3 has aldostools' [PS3 Tools](https://www.psx-place.com/threads/ps3-tools-collection-by-aldostools-includes-over-50-tools-utility-for-your-cfw-enabled-ps3.360/). PS4 also has package creation tools, but they're from the Sony PS4 [redacted], so I can't link them.

## Building
Requires CMake and Qt6. Qt should be in PATH.

```
mkdir build
cd build
cmake ..
cmake --build .
```