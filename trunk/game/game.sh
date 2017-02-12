#!/bin/bash

# *** 1fx. Mod compilation script ***
#
# This script allows you to compile 1fx. Mod for all platforms.
# The Gold port (v1.03) and the Retail port (v1.00) are both supported by this script and can be
# properly detected during script run-time. Command line arguments are supported for cross compilation.
#
# In order to compile the Mod automatically for your platform you should meet one of the following criteria:
# - SoF2 v1.00 (Linux) only works if gcc 2.95 is detected (such as the default Debian Woody install). It is then selected automatically.
# - SoF2 v1.03 (Linux) is used automatically on Linux platforms with a gcc version other than 2.95.
# - SoF2 v1.03 (Mac OS X) is used automatically when a PowerPC-based kernel is loaded.
# - SoF2 v1.03 (Windows) is used automatically when a MSYS environment is detected.
# If one of the above conditions are met, and no parameters are specified, the automatic detection will succeed.
#
# Parameters are supported, the following parameters must be specified (most of them are required for proper compilation):
# -cc: Specifies compiler name (default: "gcc").
# -l: Specifies linker name (default: "gcc" for win32, "ld" for Linux and "cc" for OS X).
# -p: Specifies platform (either "linux", "win32" or "osx").
# -c: Compile choice, either 1-4 or 3.1/3.2/3.3/3.4.
# -o: Output filename.
# -g: Build for Gold (v1.03). If not specified, the Mod will be compiled for v1.00.
# -v: Verbose mode, will print e.g. full compile lines.
# -noclear: Don't clear upon selecting choice.
# When parameters are used, no questions are asked and the script will continue with the given parameters.
# If parameters that are crucial to compiling the Mod correctly are missing, the script will break and throw an error.
#
# The script also depends on the following utilites:
# - dos2unix (some compilers require all line endings to be LF instead of CRLF). Mac OS X doesn't have this utility in the operating system by default.
#
# PLEASE NOTE: most builds are a *release* build, unless:
# - You're building a nightly build.
# - You're building a developer flavour.
#
#
# For other rather obsolete information regarding this, please check the 1fx. Mod source code.
# There's a whole section regarding *.so considerations.
# --- Boe!Man  1/26/13 - 11:01 AM
# Last update: 12/05/15 - 2:28 PM

# Enable aliases.
shopt -s expand_aliases

# The compile options relevant for all builds are noted here.
buildoptions="-O2 -DQAGAME -D_SOF2"

# Some global booleans we switch depending on user choice and host platform.
linux=false
gold=false
macosx=false
win32=false

# Other global variables.
compiler="gcc" # Default the compiler to gcc.
linker=""
count=0
verbose=false
noclear=false

# Link the Mod based on the build type (default is without symbols).
strip="-s"
buildtype=""
dev=false

# First, we detect if we're manually building the Mod or if we should automatically start determining what to build.
numargs=$#
if [ $numargs -gt 0 ]; then
    choice=""
    outfile=""

    # Arguments are used. Parse them.
    for ((i=1 ; i <= numargs ; i++))
    do
        if [ $1 == "-g" ]; then
            # Build for SoF2 v1.03 (Gold).
            buildoptions="$buildoptions -D_GOLD"
            gold=true
        elif [ $1 == "-v" ]; then
            # Verbose mode.
            verbose=true
        elif [ $1 == "-noclear" ]; then
			# Don't clear, mostly used in auto mode.
			noclear=true
        fi

        if [ $[$#-1] -lt 1 ] || [[ $2 = "-"* ]]; then
            shift
            continue
        fi

        if [ $1 == "-cc" ]; then
            # Compiler.
            compiler=$2
        elif [ $1 == "-p" ]; then
            # Platform.
            if [ $2 == "linux" ]; then
                printf
                linux=true
                win32=false
                macosx=false
            elif [ $2 == "win32" ]; then
                buildoptions="$buildoptions -DPTW32_STATIC_LIB"
                linux=false
                win32=true
                macosx=false
            elif [ $2 == "osx" ]; then
                linux=false
                win32=false
                macosx=true
            fi
        elif [ $1 == "-c" ]; then
            # Choice.
            choice=$2
        elif [ $1 == "-o" ]; then
            # Output file.
            outfile=$2
        elif [ $1 == "-l" ]; then
            # Linker.
            linker=$2
        fi

        # Advance argument.
        shift
    done

    # Check arguments specified, they must meet the specified requirements.
    if [[ $linux == false ]] && [[ $macosx == false ]] && [[ $win32 == false ]]; then
        echo "When using arguments, you MUST specify a platform to build for (linux, win32 or osx)."
        exit 1
    elif [[ $outfile == "" ]]; then
        echo "When using arguments, you MUST specify the output filename."
        exit 1
    elif [[ $choice == "" ]]; then
        echo "When using arguments, you MUST specify a choice."
        exit 1
    fi

    if [[ $macosx == false ]]; then
        buildoptions="$buildoptions -fstack-check"
    fi
else
	# Check what version to build. If the host system contains GCC 2.95, we assume we want to build for v1.00.
    if ! ([[ $compiler == "gcc" ]] && [[ `$compiler -v 2>&1 | tail -1 | awk '{print $3}'` == *"2.95"* ]]); then
        buildoptions="$buildoptions -D_GOLD"
        gold=true

        # Optionally detect Linux operating system.
		if [[ `uname -o` == *"Linux"* ]]; then
			linux=true
			outfile="sof2mp_gamei386.so"
		fi
    else
		# GCC 2.95, thus Linux.
		linux=true
        outfile="sof2mp_gamei386.so"
    fi

    # Properly detect Mac OS X operating system.
    if [[ `uname -p` == "powerpc" ]]; then
        macosx=true
        outfile="sof2mp_game"
    else
        # No stack checking on Mac OS X (the stack limit of Mac's gcc (1024) is a bit silly, esp. in SoF2).
        buildoptions="$buildoptions -fstack-check"
    fi

    # Properly detect MSYS environment (Windows operating system).
    if [[ `uname -a` == *"Msys"* ]] && [[ `uname -a` == "MINGW32"* ]]; then
        win32=true
        buildoptions="$buildoptions -DPTW32_STATIC_LIB"
        outfile="sof2mp_gamex86.dll"
    fi

    # Check what version to build. If the host system contains GCC 2.95, we assume we want to build for v1.00.
    if ! ([[ $compiler == "gcc" ]] && [[ `$compiler -v 2>&1 | tail -1 | awk '{print $3}'` == *"2.95"* ]]); then
        buildoptions="$buildoptions -D_GOLD"
        gold=true
    fi

    # Get the type of build to build.
    echo "Enter the type of build:"
    echo "1: Public release build (e.g. 0.70)"
    echo "2: Public release build with developer (e.g. 0.78-dev)"
    echo "3: 3D/Regular Pre-release build (e.g. 0.76t.1-pre) (optionally with developer)"
    echo "4: Nightly build (w/ debug symbols, e.g. 0.76t-master)"
    echo -n "Enter your choice and press [ENTER]: "
    read choice
fi

# Properly detect MSYS environment (Windows operating system), again, to use clear later on.
if [[ `uname -a` == *"Msys"* ]] && [[ `uname -a` == "MINGW32"* ]]; then
    alias clear="cls"
fi

m32=false
# Make sure we're building a 32 bit binary.
if [[ `$compiler -dumpmachine` == "x86_64"* ]]; then
    m32=true
    buildoptions="$buildoptions -m32"
fi

# Don't automatically turn on -fPIC on WIN32.
if [ $win32 == false ]; then
    buildoptions="$buildoptions -fPIC"
fi

# Parse choice.
if [ $choice == "1" ]; then
    buildoptions="$buildoptions $strip -DNDEBUG"
    buildtype="Public release build"
elif [ $choice == "2" ]; then
	buildtype="Public release build with developer"
    buildoptions="$buildoptions -g -DNDEBUG -D_DEV"
    dev=true
elif [[ $choice == "3" ]] || [[ $choice == *"3"* ]]; then
    if [[ $numargs -eq 0 ]]; then
        clear
        echo "What kind of pre-release should I build?"
        echo "1: Regular Pre-release build"
        echo "2: 3D-specific Pre-release build"
        echo "3: Regular Pre-release build with developer"
        echo "4: 3D-specific Pre-release build with developer"
        echo "5: Regular Pre-release build with developer and extra abuse cmds (USE WITH CAUTION!)"
        echo -n "Enter your choice and press [ENTER]: "
        read choice
    elif [[ $numargs -gt 0 ]] && [[ $choice == "3" ]]; then
        echo "Invalid choice specified, exitting.."
        exit 1
    fi

    if [[ $choice == "1" ]] || [[ $choice == "3.1" ]]; then
        buildtype="Regular Pre-release build"
        buildoptions="$buildoptions $strip -DNDEBUG -D_PRE"
    elif [[ $choice == "2" ]] || [[ $choice == "3.2" ]]; then
        buildoptions="$buildoptions $strip -DNDEBUG -D_PRE -D_3DServer -D_awesomeToAbuse"
        buildtype="3D-specific Pre-release build"
    elif [[ $choice == "3" ]] || [[ $choice == "3.3" ]]; then
		buildtype="Regular Pre-release build with developer"
        buildoptions="$buildoptions -DNDEBUG -D_PRE -D_DEV"
        dev=true
    elif [[ $choice == "4" ]] || [[ $choice == "3.4" ]]; then
		buildtype="3D-specific Pre-release build with developer"
        buildoptions="$buildoptions -DNDEBUG -D_PRE -D_DEV -D_3DServer -D_awesomeToAbuse"
        dev=true
    elif [[ $choice == "5" ]] || [[ $choice == "3.5" ]]; then
		buildtype="Regular Pre-release build with developer and extra abuse cmds"
        buildoptions="$buildoptions -DNDEBUG -D_PRE -D_DEV -D_awesomeToAbuse"
        dev=true
    else
        echo "Invalid choice specified, exitting.."
        exit 1
    fi
elif [ $choice == "4" ]; then
    buildoptions="$buildoptions -g -D_DEBUG -DDEBUG -D_NIGHTLY"
    strip=""
    buildtype="Nightly build"
else
    echo "Invalid choice specified, exitting.."
    exit 1
fi

# Add developer build options.
if [ $dev == true ]; then
	# Enforce debug symbols.
	buildoptions="$buildoptions -g"

	if [ $linux == true ]; then
		 buildoptions="$buildoptions -D_GNU_SOURCE"
	fi
fi

# Summarize build environment and choices.
if [ $noclear == false ]; then
	clear
fi
printf "\e[00;36mPre-build summary\e[00m\n"
printf "=====================================================\n"
printf "\e[00;33m%-25s \e[00;32m%s\e[00m\n" "Build type" "$buildtype"
printf "\e[00;33m%-25s \e[00;32m%s\e[00m\n" "Output file" "$outfile"

if [ $linux == true ]; then
    printf "\e[00;33m%-25s \e[00;32m%s\e[00m\n" "Target platform" "Linux"
elif [ $macosx == true ]; then
    printf "\e[00;33m%-25s \e[00;32m%s\e[00m\n" "Target platform" "Mac OS X"
elif [ $win32 == true ]; then
    printf "\e[00;33m%-25s \e[00;32m%s\e[00m\n" "Target platform" "WIN32"
fi

if [ $gold == false ]; then
    printf "\e[00;33m%-25s \e[00;32m%s\e[00m\n" "SoF2 version" "v1.00"
else
    printf "\e[00;33m%-25s \e[00;32m%s\e[00m\n" "SoF2 version" "v1.03"
fi

printf "\n"
printf "\e[00;36mMain compilation & linking\e[00m\n"
printf "=====================================================\n"

buildoptions="$buildoptions -c"

# Get the *.c file count.
cFileCount=`ls -l *.c | wc -l`
cFileCount="$(echo -e "${cFileCount}" | sed -e 's/^[[:space:]]*//')" # Remove whitespace.
cFileCount=$((10#$cFileCount+1)) # Convert to int + 1 for the "special" SQLite3 file.

if [ $gold == false ] && [ $linux == true ]; then
    cFileCount=$((cFileCount+1)) # Extra addition of the "special" TADNS file.
fi

# Clean up compile_log file and Mod.
rm -f compile_log
rm -f $outfile

# dos2unix section, convert CRLF line endings to LF.
printf "Converting CRLF to LF.. "

# Convert "special files" outside the directory first.
# SQLite files.
dos2unix ./sqlite/sqlite3.c &> /dev/null
dos2unix ./sqlite/sqlite3.h &> /dev/null
dos2unix ./sqlite/sqlite3ext.h &> /dev/null
# TADNS files.
dos2unix ./tadns/tadns.c &> /dev/null
dos2unix ./tadns/tadns.h &> /dev/null
dos2unix ./tadns/llist.h &> /dev/null

# Main files.
# *.c files.
for f in *.c
do
    dos2unix $f &> /dev/null
done
# *.h files.
for f in *.h
do
    dos2unix $f &> /dev/null
done
printf "done!\n"

# Compile the Mod.
function compile {
    padding=""

    # Increase counter.
    count=$((count+1))

    # Convert .c to .o.
    oFile="${1/.c/.o}"

    # Calculate the percentage in the process.
    percentage=$(printf '%i %i' $count $cFileCount | awk '{ pc=100*$1/$2; i=int(pc); print (pc-i<0.5)?i:i+1 }')
    percentage=$((10#$percentage)) # Convert to int.

    # Calculate padding based on percentage.
    if [ $percentage -lt 10 ]; then
        padding="  "
    elif [ $percentage -lt 100 ]; then
        padding=" "
    fi

    # Add .o file to the linkfiles.
    linkfiles="$linkfiles $oFile"

    # Compile the actual file.
    if [ $verbose == true ]; then
        echo $compiler $2 $1 -o $oFile 2>> compile_log
    else
        echo -e "\e[00;33m[$padding$percentage%] \e[00;32mBuilding $1 -> $oFile\e[00m"
    fi
    $compiler $2 $1 -o $oFile 2>> compile_log
}

linkfiles=""
echo "Now compiling (verbose output to compile_log file).."
for cFile in *.c
do
    compile $cFile "$buildoptions"
done

# SQLite (and TADNS for SoF2 - v1.00 on Linux).
if [[ $gold == false ]] && [[ $linux == true ]]; then
    compile "./tadns/tadns.c" "$strip -fstack-check -fPIC -c"
    compile "./sqlite/sqlite3.c" "$strip -fstack-check -DNDEBUG -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_ENABLE_MEMSYS5 -fPIC -c"
else
    if [ $m32 == true ]; then
        m32b="-m32"
    else
        m32b=""
    fi

    if [ $win32 == true ]; then
        compile "./sqlite/sqlite3.c" "$m32b $strip -fstack-check -DNDEBUG -DSQLITE_OMIT_LOAD_EXTENSION -c"
    elif [ $linux == true ]; then
        compile "./sqlite/sqlite3.c" "$m32b $strip -fstack-check -DNDEBUG -DSQLITE_OMIT_LOAD_EXTENSION -fPIC -c"
    elif [ $macosx == true ]; then
        compile "./sqlite/sqlite3.c" "$m32b $strip -fstack-check -DNDEBUG -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_WITHOUT_ZONEMALLOC -fPIC -c"
    fi
fi

printf "Now linking the target file.. "
# Link the Mod.
if [[ $gold == false ]] && [[ $linux == true ]]; then
    # Static libraries to link against (SoF2 v1.00 Linux).
    libs="\
    /usr/local/lib/libunwind-x86.a \
    /usr/local/lib/libunwind.a \
    /usr/lib/libpthread.a \
    /usr/lib/libm.a \
    /usr/lib/libc.a \
    /usr/lib/gcc-lib/i386-linux/2.95.4/libgcc.a \
    /usr/lib/libdl.a"

    # This links the Mod dynamically with static dependencies.
    if [ $dev == false ]; then
		ld $strip -shared $linkfiles -Bstatic $libs -o $outfile 2>> compile_log
	else
		ld -E -shared $linkfiles -Bstatic $libs -o $outfile 2>> compile_log
	fi
else
    # Regular dynamic linking.
    if [ $macosx == true ]; then
        if [[ $linker == "" ]]; then
            linker="cc"
        fi

        $linker $linkfiles -dynamic -bundle -o $outfile -lpthread -lm -lc -ldl 2>> compile_log
    elif [ $win32 == true ]; then
        if [ $m32 == true ]; then
            m32b="-m32"
        else
            m32b=""
        fi

        if [[ $linker == "" ]]; then
            linker="gcc"
        fi

        if [[ `uname -a` == *"Msys"* ]] && [[ `uname -a` == "MINGW32"* ]]; then
            pthread="-lpthreadGC2"
        else
            pthread="-lpthread"
        fi

		if [ $dev == false ]; then
			$linker $m32b $strip -shared $linkfiles -static -static-libgcc $pthread -o $outfile 2>> compile_log
		else
			$linker $m32b -shared $linkfiles -static -static-libgcc $pthread -o $outfile 2>> compile_log
		fi
    else # Linux.
        if [ $m32 == true ]; then
            m32b="-m elf_i386"
        else
            m32b=""
        fi

        if [[ $linker == "" ]]; then
            linker="ld"
        fi

		if [ $dev == false ]; then
			$linker $strip $m32b -shared $linkfiles -lpthread -lm -lc -ldl -o $outfile 2>> compile_log
		else
			# With developer enabled.
			if [[ $linker == "ld" ]]; then
				rdynamic="--export-dynamic"
			else
				rdynamic="-rdynamic"
			fi

			$linker $m32b $rdynamic -shared $linkfiles -lpthread -lm -lc -ldl -o $outfile 2>> compile_log
		fi
    fi
fi
printf "done!\n"

printf "\n"
printf "\e[00;36mChecking end-result\e[00m\n"
printf "=====================================================\n"

# Now check if the output file was indeed created..
if [ -f $outfile ]; then
    echo -e "Compiling the Mod was \e[00;32msuccessfull\e[00m!"

    if [ -s "compile_log" ]; then
        echo -e "Some \e[00;33mwarnings\e[00m did occur."
    else
        echo "No warnings occured."
    fi
else
    echo -e "Compiling the Mod \e[00;31mfailed\e[00m!"
    echo "Please check the compile_log file for more information.."
fi

# Clean up some files..
rm -rf *.o

# Done!
printf "\n"
exit 0
