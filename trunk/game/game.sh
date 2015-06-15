#!/bin/bash

# *** Compiling 1fx. Mod on *nix-based platforms ***
#
# This script allows you to compile 1fx. Mod for all platforms except Windows (not even MinGW).
# The Gold port (v1.03) and the Retail port (v1.00) are both supported by this script and should be
# properly detected during script run-time.
#
# In order to compile the Mod for your platform you should meet one of the following criteria:
# - SoF2 v1.00 (Linux) only works if gcc 2.95 is detected (such as the default Debian Woody install).
# - SoF2 v1.03 (Linux) is used automatically on Linux platforms with a gcc version other than 2.95.
# - SoF2 v1.03 (Mac OS X) is used automatically when a PowerPC-based kernel is loaded.
#
# The script also depends on the following utilites:
#
# - dos2unix (some compilers require all line endings to be LF instead of CRLF). Mac OS X doesn't have this utility in the operating system by default.
#
# The files that need to be compiled, need to be in the dos2unix section, the compile section and the linker section.
# Header files should go only in the dos2unix section.
#
# PLEASE NOTE: most builds are a *release* build. All debug symbols, flags etc. are omitted, even in the so called test releases.
# *ONLY* if you build a nightly build, debug symbols won't be omitted.
#
#
# For further rather obsolete information regarding this, please check the 1fx. Mod source code.
# There's a whole section regarding *.so considerations.
# --- Boe!Man  1/26/13 - 11:01 AM
# Last update: 5/19/15 - 1:07 AM

# The compile options relevant for all builds are noted here.
buildoptions="-O2 -DMISSIONPACK -DQAGAME -D_SOF2 -fPIC"

# Some global booleans we switch depending on user choice and host platform.
gold=false
macosx=false

# Other global variables.
outfile="sof2mp_gamei386.so"
count=0

# Properly detect Mac OS X operating system.
if [[ `uname -p` == "powerpc" ]]; then
	macosx=true
	outfile="sof2mp_game"
else
	# No stack checking on Mac OS X (the stack limit of Mac's gcc (1024) is a bit silly, esp. in SoF2).
	buildoptions="$buildoptions -fstack-check"
fi

# Check what version to build. If the host system contains GCC 2.95, we assume we want to build for v1.00.
if ! [[ `gcc -v 2>&1 | tail -1 | awk '{print $3}'` == *"2.95"* ]]; then
	buildoptions="$buildoptions -D_GOLD"
	gold=true
fi

# Link the Mod based on the build type (default is without symbols).
strip="-s"

# Get the type of build to build.
buildtype=""
echo "Enter the type of build:"
echo "1: Public release build (e.g. 0.70)"
echo "2: Test/Beta release build (e.g. 0.70t)"
echo "3: 3D/Regular Pre-release build (e.g. 0.76t.1-pre)"
echo "4: Nightly build (w/ debug symbols, e.g. 0.76t-master)"
echo -n "Enter your choice and press [ENTER]: "
read choice

if [ "$choice" == "1" ]; then
	buildoptions="$buildoptions $strip -DNDEBUG"
	buildtype="Public release build"
elif [ "$choice" == "2" ]; then
	buildoptions="$buildoptions $strip -D_DEBUG -DDEBUG -DBUILDING_REF_GL -DDebugSoF2"
	buildtype="Test/Beta release build"
elif [ "$choice" == "3" ]; then
	buildoptions="$buildoptions $strip -D_DEBUG -DDEBUG -DBUILDING_REF_GL -DDebugSoF2"
	clear
	echo "What kind of pre-release should I build?"
	echo "1: Regular Pre-release build"
	echo "2: 3D-specific Pre-release build"
	echo -n "Enter your choice and press [ENTER]: "
	read choice
	if [ "$choice" == "1" ]; then
		buildtype="Regular Pre-release build"
	elif [ "$choice" == "2" ]; then
		buildoptions="$buildoptions -D_3DServer -D_awesomeToAbuse"
		buildtype="3D-specific Pre-release build"
	else
		echo "Invalid choice specified, exitting.."
		exit 1
	fi
elif [ "$choice" == "4" ]; then
	buildoptions="$buildoptions -g -D_DEBUG -DDEBUG -DBUILDING_REF_GL -DDebugSoF2 -D_NIGHTLY"
	strip=""
	buildtype="Nightly build"
else
	echo "Invalid choice specified, exitting.."
	exit 1
fi

# Summarize build environment and choices.
clear
printf "\e[00;36mPre-build summary\e[00m\n"
printf "=====================================================\n"
printf "\e[00;33m%-25s \e[00;32m%s\e[00m\n" "Build type" "$buildtype"
printf "\e[00;33m%-25s \e[00;32m%s\e[00m\n" "Output file" "$outfile"

if [ "$macosx" == false ]; then
	printf "\e[00;33m%-25s \e[00;32m%s\e[00m\n" "Target platform" "Linux"
else
	printf "\e[00;33m%-25s \e[00;32m%s\e[00m\n" "Target platform" "Mac OS X"
fi

if [ "$gold" == false ]; then
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

if [ "$gold" == false ]; then
	$cFileCount=$((cFileCount+1)) # Extra addition of the "special" TADNS file.
fi

# Clean up compile_log file and Mod (we're recompiling so..).
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
	if [ "$percentage" -lt 10 ]; then
		padding="  "
	elif [ "$percentage" -lt 100 ]; then
		padding=" "
	fi
	
	# Add .o file to the linkfiles.
	linkfiles="$linkfiles $oFile"

	# Compile the actual file.
	echo -e "\e[00;33m[$padding$percentage%] \e[00;32mBuilding $1 -> $oFile\e[00m"
	gcc $2 $1 -o $oFile 2>> compile_log
}

linkfiles=""
echo "Now compiling (verbose output to compile_log file).."
for cFile in *.c
do
	compile $cFile "$buildoptions"
done

# SQLite (and TADNS for SoF2 - v1.00).
if [ "$gold" == false ]; then
	compile "./tadns/tadns.c" "$strip -fstack-check -fPIC -c"
	compile "./sqlite/sqlite3.c" "$strip -fstack-check -DNDEBUG -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_ENABLE_MEMSYS5 -fPIC -c"
else
	if [ "$macosx" == false ]; then
		compile "./sqlite/sqlite3.c" "$strip -fstack-check -DNDEBUG -DSQLITE_OMIT_LOAD_EXTENSION -fPIC -c"
	else
		compile "./sqlite/sqlite3.c" "$strip -fstack-check -DNDEBUG -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_WITHOUT_ZONEMALLOC -fPIC -c"
	fi
fi

printf "Now linking the target file.. "
# Link the Mod.
if [ "$gold" == false ]; then
	# Static libraries to link against (SoF2 v1.00).
	libs="\
	/usr/lib/libpthread.a \
	/usr/lib/libm.a \
	/usr/lib/libc.a \
	/usr/lib/gcc-lib/i386-linux/2.95.4/libgcc.a \
	/usr/lib/libdl.a"

	# This links the Mod dynamically with static dependencies.
	ld $strip -shared $linkfiles -Bstatic $libs -o $outfile 2>> compile_log
else
	# Regular dynamic linking.
	if [ "$macosx" == false ]; then
		ld $strip -shared $linkfiles -lpthread -lm -lc -ldl -o $outfile 2>> compile_log
	else
		cc $linkfiles -dynamic -bundle -o $outfile -lpthread -lm -lc -ldl 2>> compile_log
	fi
fi
printf "done!\n"

printf "\n"
printf "\e[00;36mChecking end-result\e[00m\n"
printf "=====================================================\n"

# Now check if the output file was indeed created..
if [ -f ./$outfile ]; then
	echo -e "Compiling the Mod was \e[00;32msuccessfull\e[00m!"
	
	if [ "$gold" == false ]; then
		if [ $(stat -c %s compile_log 2>/dev/null || echo 0) -gt 0 ]; then
		    echo -e "Some \e[00;33mwarnings\e[00m did occur."
		else
		    echo "No warnings occured."
		fi
	else
		if [ "$macosx" == true ]; then
			param="-f%z"
		else
			param="-c 0"
		fi

		if [ $(stat $param compile_log 2>/dev/null) -gt 0 ]; then
		    echo -e "Some \e[00;33mwarnings\e[00m did occur."
		else
		    echo "No warnings occured."
		fi
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
