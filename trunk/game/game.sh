#!/bin/bash

# *** Compiling 1fx. Mod Linux build (sof2mp_gamei386.so) ***
#
# You can compile the Mod on an ancient Debian Woody R0 platform.
# The Mod depends on the following utilites:
#
# - gcc 2.95.4 (can be installed from CD1 of the Debian Woody install media).
# - dos2unix (the old compiler requires all line endings to be LF instead of CRLF).
#
# The files that need to be compiled, need to be in the dos2unix section, the compile section and the linker section.
# Header files should go only in the dos2unix section.
#
# PLEASE NOTE: this build is a *release* build. All debug symbols, flags etc. are omitted, even in the so called test releases.
#
# For further information regarding this, please check the 1fx. Mod source code.
# There's a whole section regarding *.so considerations.
# --- Boe!Man 1/26/13 - 11:01 AM

echo "Enter the type of build:"
echo "1: Public release build (e.g. 0.70)"
echo "2: Test/Beta release build (e.g. 0.70t)"
echo -n "Enter your choice and press [ENTER]: "
read choice

if [ "$choice" == "1" ]; then
	buildoptions="-s -O2 -fstack-check -DNDEBUG -DMISSIONPACK -DQAGAME -D_SOF2 -fPIC -c"
	clear
	echo -e "Building a \e[00;36mPublic release build\e[00m" 
elif [ "$choice" == "2" ]; then
	buildoptions="-s -O2 -fstack-check -D_DEBUG -DDEBUG -DBUILDING_REF_GL -DDebugSoF2 -DMISSIONPACK -DQAGAME -D_SOF2 -D_BOE_DBG -fPIC -c"
	clear
	echo -e "Building a \e[00;36mTest/Beta release build\e[00m" 
else
	clear
	echo "Invalid choice specified, exitting.."
	exit 1
fi

# Clean up compile_log file and Mod (we're recompiling so..).
rm -f compile_log
rm -f sof2mp_gamei386.so

# dos2unix section, convert CRLF line endings to LF.
echo "Converting CRLF to LF.."
# SQLite files.
dos2unix ./sqlite/sqlite3.c
dos2unix ./sqlite/sqlite3.h
dos2unix ./sqlite/sqlite3ext.h
# Main files.
dos2unix 1fx_gt.c
dos2unix ai_main.c
dos2unix ai_util.c
dos2unix ai_wpnav.c
dos2unix bg_gametype.c
dos2unix bg_lib.c
dos2unix bg_misc.c
dos2unix bg_player.c
dos2unix bg_pmove.c
dos2unix bg_slidemove.c
dos2unix bg_weapons.c
dos2unix boe_admcmds.c
dos2unix boe_utils.c
dos2unix g_active.c
dos2unix g_antilag.c
dos2unix g_bot.c
dos2unix g_client.c
dos2unix g_cmds.c
dos2unix g_combat.c
dos2unix g_gametype.c
dos2unix g_items.c
dos2unix g_main.c
dos2unix g_misc.c
dos2unix g_missile.c
dos2unix g_mover.c
dos2unix g_session.c
dos2unix g_spawn.c
dos2unix g_svcmds.c
dos2unix g_syscalls.c
dos2unix g_target.c
dos2unix g_team.c
dos2unix g_trigger.c
dos2unix g_utils.c
dos2unix g_weapon.c
dos2unix henkENT.c
dos2unix henk_functions.c
dos2unix henk_hs.c
dos2unix q_math.c
dos2unix q_shared.c
dos2unix rpm_functions.c
dos2unix rpm_refcmds.c
dos2unix rpm_tcmds.c
# Header files go here.
dos2unix 1fx_gt.h
dos2unix ai_main.h
dos2unix anims.h
dos2unix be_aas.h
dos2unix be_ai_char.h
dos2unix be_ai_chat.h
dos2unix be_ai_gen.h
dos2unix be_ai_goal.h
dos2unix be_ai_move.h
dos2unix be_ai_weap.h
dos2unix be_ea.h
dos2unix bg_lib.h
dos2unix bg_local.h
dos2unix bg_public.h
dos2unix bg_weapons.h
dos2unix boe_local.h
dos2unix botlib.h
dos2unix chars.h
dos2unix g_local.h
dos2unix g_public.h
dos2unix g_team.h
dos2unix inv.h
dos2unix match.h
dos2unix q_shared.h
dos2unix surfaceflags.h
dos2unix syn.h

# Compile the Mod.
echo "Now compiling, check compile_log for errors if the build fails.."
gcc $buildoptions g_main.c -o g_main.o 2>> compile_log
gcc $buildoptions g_syscalls.c -o g_syscalls.o 2>> compile_log
gcc $buildoptions bg_misc.c -o bg_misc.o 2>> compile_log
gcc $buildoptions bg_lib.c -o bg_lib.o 2>> compile_log
gcc $buildoptions bg_pmove.c -o bg_pmove.o 2>> compile_log
gcc $buildoptions bg_slidemove.c -o bg_slidemove.o 2>> compile_log
gcc $buildoptions bg_player.c -o bg_player.o 2>> compile_log
gcc $buildoptions bg_weapons.c -o bg_weapons.o 2>> compile_log
gcc $buildoptions g_gametype.c -o g_gametype.o 2>> compile_log
gcc $buildoptions q_math.c -o q_math.o 2>> compile_log
gcc $buildoptions q_shared.c -o q_shared.o 2>> compile_log
gcc $buildoptions ai_main.c -o ai_main.o 2>> compile_log
gcc $buildoptions ai_util.c -o ai_util.o 2>> compile_log
gcc $buildoptions ai_wpnav.c -o ai_wpnav.o 2>> compile_log
gcc $buildoptions bg_gametype.c -o bg_gametype.o 2>> compile_log
gcc $buildoptions boe_admcmds.c -o boe_admcmds.o 2>> compile_log
gcc $buildoptions boe_utils.c -o boe_utils.o 2>> compile_log
gcc $buildoptions g_active.c -o g_active.o 2>> compile_log
gcc $buildoptions g_antilag.c -o g_antilag.o 2>> compile_log
gcc $buildoptions g_bot.c -o g_bot.o 2>> compile_log
gcc $buildoptions g_client.c -o g_client.o 2>> compile_log
gcc $buildoptions g_cmds.c -o g_cmds.o 2>> compile_log
gcc $buildoptions g_combat.c -o g_combat.o 2>> compile_log
gcc $buildoptions g_items.c -o g_items.o 2>> compile_log
gcc $buildoptions g_misc.c -o g_misc.o 2>> compile_log
gcc $buildoptions g_missile.c -o g_missile.o 2>> compile_log
gcc $buildoptions g_mover.c -o g_mover.o 2>> compile_log
gcc $buildoptions g_session.c -o g_session.o 2>> compile_log
gcc $buildoptions g_spawn.c -o g_spawn.o 2>> compile_log
gcc $buildoptions g_svcmds.c -o g_svcmds.o 2>> compile_log
gcc $buildoptions g_target.c -o g_target.o 2>> compile_log
gcc $buildoptions g_team.c -o g_team.o 2>> compile_log
gcc $buildoptions g_trigger.c -o g_trigger.o 2>> compile_log
gcc $buildoptions g_utils.c -o g_utils.o 2>> compile_log
gcc $buildoptions g_weapon.c -o g_weapon.o 2>> compile_log
gcc $buildoptions henkENT.c -o henkENT.o 2>> compile_log
gcc $buildoptions henk_functions.c -o henk_functions.o 2>> compile_log
gcc $buildoptions henk_hs.c -o henk_hs.o 2>> compile_log
gcc $buildoptions rpm_functions.c -o rpm_functions.o 2>> compile_log
gcc $buildoptions rpm_refcmds.c -o rpm_refcmds.o 2>> compile_log
gcc $buildoptions rpm_tcmds.c -o rpm_tcmds.o 2>> compile_log
gcc $buildoptions 1fx_gt.c -o 1fx_gt.o 2>> compile_log
# SQLite
gcc -s -fstack-check -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_ENABLE_MEMSYS5 -fPIC -c ./sqlite/sqlite3.c -o ./sqlite/sqlite3.o 2>> compile_log
echo "Now linking the shared object.."
# There are several compile lines of interest.
# The first one is the one we use everywhere, it's default as of revision 950. This compiles the Mod dynamically with static dependencies.
ld -s -shared ai_main.o ai_util.o ai_wpnav.o bg_gametype.o bg_lib.o bg_misc.o bg_player.o bg_pmove.o bg_slidemove.o bg_weapons.o boe_admcmds.o boe_utils.o g_active.o g_antilag.o g_bot.o g_client.o g_cmds.o g_combat.o g_gametype.o g_items.o g_main.o g_misc.o g_missile.o g_mover.o g_session.o g_spawn.o g_svcmds.o g_syscalls.o g_target.o g_team.o g_trigger.o g_utils.o g_weapon.o henkENT.o henk_functions.o henk_hs.o q_math.o q_shared.o rpm_functions.o rpm_refcmds.o rpm_tcmds.o 1fx_gt.o ./sqlite/sqlite3.o -Bstatic /usr/lib/libpthread.a /usr/lib/libm.a /usr/lib/libc.a /usr/lib/gcc-lib/i386-linux/2.95.4/libgcc.a /usr/lib/libdl.a -o sof2mp_gamei386.so 2>> compile_log
# This line compiles the shared object with only dynamic objects and dependencies.
#gcc -shared -Wl -s ai_main.o ai_util.o ai_wpnav.o bg_gametype.o bg_lib.o bg_misc.o bg_player.o bg_pmove.o bg_slidemove.o bg_weapons.o boe_admcmds.o boe_utils.o g_active.o g_antilag.o g_bot.o g_client.o g_cmds.o g_combat.o g_gametype.o g_items.o g_main.o g_misc.o g_missile.o g_mover.o g_session.o g_spawn.o g_svcmds.o g_syscalls.o g_target.o g_team.o g_trigger.o g_utils.o g_weapon.o henkENT.o henk_functions.o henk_hs.o q_math.o q_shared.o rpm_functions.o rpm_refcmds.o rpm_tcmds.o 1fx_gt.o ./sqlite/sqlite3.o -o sof2mp_gamei386.so -lm -ldl -lpthread
# This is an old line we only used in the past, uses ld as linker.
#ld -s -shared ai_main.o ai_util.o ai_wpnav.o bg_gametype.o bg_lib.o bg_misc.o bg_player.o bg_pmove.o bg_slidemove.o bg_weapons.o boe_admcmds.o boe_utils.o g_active.o g_antilag.o g_bot.o g_client.o g_cmds.o g_combat.o g_gametype.o g_items.o g_main.o g_misc.o g_missile.o g_mover.o g_session.o g_spawn.o g_svcmds.o g_syscalls.o g_target.o g_team.o g_trigger.o g_utils.o g_weapon.o henkENT.o henk_functions.o henk_hs.o q_math.o q_shared.o rpm_functions.o rpm_refcmds.o rpm_tcmds.o 1fx_gt.o ./sqlite/sqlite3.o -o sof2mp_gamei386.so

# Now check if the output file was indeed created..
if [ -f ./sof2mp_gamei386.so ]; then
	echo -e "Compiling the Mod was \e[00;32msuccessfull\e[00m!"
else
	echo -e "Compiling the Mod \e[00;31mfailed\e[00m!"
	echo "Please check the compile_log file for more information.."
fi

# Clean up some files..
rm -f *.o
rm -f ./sqlite/sqlite3.o

# Done!
exit 0
