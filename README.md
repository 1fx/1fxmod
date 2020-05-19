# 1fx. Mod
1fx. Mod is a free server mod for all major versions of Soldier of Fortune II:
Double Helix, mainly focusing on technical improvements, performance,
security and flexibility. The mod is server side only but features built-in
support for the popular client mods RPM 2k3 (v1.00) and ROCmod (v1.03).

### Overview
Development has traditionally focused on adding functionality popular among
players and trying to (technically) improve the core where possible. The most
notable features include:

* Shared code base for all major SoF2MP versions: SoF2 v1.02t (MP Test), SoF2
v1.00 (Retail/Full/Silver) and SoF2 v1.03 (Gold).
* Compiles as shared library rather than QVM on Windows (x86), Linux (x86) and
Mac OS X (ppc, incomplete), providing better performance and greater
flexibility.
* Runtime memory patches for all known exploits of the game, this includes DoS,
q3infoboom and q3dirtrav protection.
* SQLite backend for user and system databases.
* Extensive Admin system with a large set of Admin commands, multiple levels and
easy to execute from the chat.
* 1fx. Client Additions support for HTTP downloads through the client.
* Multiple client mod support which can be changed from the server across map
restarts (does not require fs_game).
* The additional game types Hide&Seek and Humans&Zombies (requires 1fx. Client
Additions on SoF2 v1.03 and are not available for SoF2 v1.02t).
* MVCHAT (multilingual voice chat) sound system allowing up to 999 sounds per
language (a total of 5) with the help of an internal sound cache system to
bypass the 256 sounds limit.
* A competition mode which can be used for scrims, including referee support.
* Lots of settings to tweak the look and feel such as messages and user
prefixes.

.. and much *much* more.

### Repository structure
Traditionally hosted in a Subversion repository, this repository has retained
the same structure it originally did. The repository is split up in the
following directories:

* *branches/060*: 1fx. Mod 0.60 was the last QVM-based release and was meant to
be a LTS release. Only one patch release was made, 0.61, at the same time of the
0.70 release. This directory contains that source code.
* *branches/brainworks*: An idea to port Brainworks (based on Quake 3) to SoF2
to allow for more realistic bots. Almost no progress made in this branch.
* *branches/cplusplus*: An idea to convert the core to C++ instead of C.
Impractical after various tests due to the C ABI with the engine and therefore
abandoned.
* *files*: Files to be bundled with either public or beta releases. Largely
abandoned.
* *trunk*: The latest 1fx. Mod source code.

Most of you will only ever need the *trunk* folder.

### Getting started
1fx. Mod has been developed almost exclusively on Linux based systems and
therefore lacks complete Visual Studio/MSVC support. We've tried our best on
keeping platform specific code such as the runtime patches at least *compatible*
with MSVC, but please bear in mind that it is likely to be broken. Any
contributions for better MSVC support are very welcome.

#### SoF2 v1.00 on Linux
Scattered throughout the source code you will find multiple comments regarding
SoF2 v1.00 on Linux. The *sof2ded* binary released by Raven Software way back in
2002 is statically linked, which complicates matters when trying to run (let
alone build) for SoF2 v1.00 on recent Linux distributions.

We've spent a tremendous amount of time in getting everything as stable as it is
right now, but the same restrictions still apply now as they did back then.
Building for SoF2 v1.00 requires a legacy system and an old compiler (gcc 2.95)
in order to run correctly on new systems. To make matters worse, servers also
need a set of runtime libraries in order to load the shared library correctly,
which we've bundled with our mod, named 1fxlib.

We have always used an installation of Debian Woody (3.0) to build 1fx. Mod. The
libraries from 1fxlib are also from this distribution. It has become
increasingly hard to properly configure and compile gcc 2.95 for modern Linux
systems, which is why it may be most convenient to just virtualize Debian Woody
and compile the Mod on that system, but any distribution that has an old 2.2
or 2.4 kernel with LinuxThreads support and gcc 2.95 *should* work fine. Bear in
mind that we have only ever successfully tested Debian Woody.

Also keep in mind that gcc 2.95 refrains you from using newer language features,
since it lacks complete C99 support, let alone any support for C11 or newer. You
can find the status of C99 features in GCC
[here](http://gcc.gnu.org/c99status.html "Status of C99 features in GCC").

For more information on this topic please take a look at the *boe_local.h* and
*game.sh* files.

SoF2 v1.03 on Linux does not have the same problems as it is dynamically linked
due to supporting the PunkBuster shared libraries. Any modern Linux distribution
can be used without any problem to compile the Mod for SoF2 v1.03.

#### Game type modules
Traditionally, SoF2 game types run as a separate module next to the game module.
This has also been the case for 1fx. Mod up until we made the switch from QVMs
to shared libraries. Being unable to fix the loading of game type module (or any
external shared library for that matter) on SoF2 v1.00 Linux (at the time), we
have incorporated all game type code into a single *1fx_gt.c* file and don't
load any additional game type files anymore.

#### Compiling the game module
1fx. Mod has been supporting more than one operating system since 0.70, and more
than one game version since version 0.80. To support compiling for all these
individual versions and operating systems we created a shell script that allowed
for easy compilation, namely the **game.sh** script.

As mentioned before, MSVC is not supported, and we therefore recommend compiling
the Mod with gcc. On Windows, this means compiling the Mod with MinGW, which can
be easily installed in conjunction with
[MSYS2](https://www.msys2.org/ "MSYS2 installer"). The old(er) MinGW/MSYS from the
[official MinGW site](http://mingw.org/ "MinGW and MSYS projects") is not
supported anymore, and requires pthreadGC2.dll from the
[pthreads-win32](https://sourceware.org/pthreads-win32/ "POSIX Threads for Win32")
project. It is recommended to just use MSYS2 instead.

If no parameters are specified, the script will run in interactive mode. Prior
to showing the available build choices, it will attempt to do an automatic
detection. It adheres to the following rules:

* SoF2 v1.00 (Linux) is selected when gcc 2.95 is detected.
* SoF2 v1.03 (Linux) is selected on Linux platforms with a gcc version other
than 2.95.
* SoF2 v1.03 (Windows) is selected when a MSYS environment is detected.
* SoF2 v1.03 (Mac OS X) is selected when a PowerPC-based kernel is detected.

After this initial detection, the script will ask you what you want to build:

```bash
$ ./game.sh
Enter the type of build:
1: Public release build (e.g. 0.70)
2: Public release build with developer (e.g. 0.78-dev)
3: 3D/Regular Pre-release build (e.g. 0.76t.1-pre) (optionally with developer)
4: Nightly build (w/ debug symbols, e.g. 0.76t-master)
Enter your choice and press [ENTER]:
```

The automatic detection only covers a handful of all supported configurations.
For more control, you may also pass arguments to the script. The script supports
the following parameters:

* -cc: Specifies compiler name (default: "gcc").
* -l: Specifies linker name (default: "cc" on Mac OS X, "gcc" everywhere else).
* -p: Platform, which is important, especially during cross compilation. The
following options are supported (and are self-explanatory):
    * linux
    * win32
    * osx
* -c: Compile choice, either 1-4 or 3.1/3.2/3.3/3.4.
* -o: Output filename.

If not building for SoF2 v1.00, the version must also be specified:

* -g: Build for Gold (v1.03).
* -d: Build for MP TEST/Demo (v1.02t).

The following *optional* parameters may also be specified:

* -v: Verbose mode, will print e.g. full compile lines.
* -noclear: Don't clear upon selecting choice.

Here's an example of compiling the Mod using parameters (for Windows, SoF2 v1.03
(Gold)):

```bash
$ ./game.sh -cc gcc -l gcc -p win32 -c 2 -o /c/SoF2Gold/1fx/sof2mp_gamex86.dll -g
```

Or cross compiling the Mod for on some Linux system (for Windows, SoF2 v1.02t
(MP TEST/Demo)):

```bash
$ ./game.sh -cc i686-w64-mingw32-gcc -l i686-w64-mingw32-gcc -p win32 -c 2 -o ~/sof2mp_gamex86.dll -d
```

If compilation fails and you didn't alter anything in the code yet, this is
usually something in the run time not correctly installed or set. Verbose output
is always logged to the *compile_log* file.

### License
1fx. Mod is licensed under the GNU General Public License v2.0. You can find a
copy of the license in the LICENSE.md file in the repository root.
