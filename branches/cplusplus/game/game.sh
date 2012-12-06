#!/bin/sh

# *** Compiling SoF2 VMs with linux utils ***
#
# You can compile all your compiler tools using the vanilla q3 1.32b src. You may get some errors about specific
# Genesis ROM files (*.md), this is because activision did a bad job providing these essential files in UNIX format,
# instead they're optimized for DOS (Windows). You can use tools such as 'dos2unix' to convert all those files.
#
# Make sure all utils are compiled against your older system headers (that are also compatible with your *.so)
# You can use full system paths but it's easier to just create a few symlinks to these binaries, so you can 
# use them in the future against other projects as well.
# ln -s <destination> <linkname>
# e.g.: ln -s /home/aj/sof2/SDK/bin/linux-utils/q3asm /usr/local/bin/q3asm
# --- Boe!Man 8/3/11 - 12:59 PM

mkdir -p vm
cd vm

CC="lcc -A -DQ3_VM -DMISSIONPACK -DQAGAME -D_BOE_DBG -S -Wf-target=bytecode -Wf-g -I../../cgame -I../../game -I../../ui"

$CC  ../g_main.c
$CC  ../g_syscalls.c

$CC  ../bg_misc.c
$CC  ../bg_lib.c
$CC  ../bg_pmove.c
$CC  ../bg_slidemove.c
$CC  ../bg_player.c
$CC  ../bg_weapons.c
$CC  ../bg_gametype.c
$CC  ../q_math.c
$CC  ../q_shared.c

$CC  ../ai_main.c
$CC  ../ai_util.c
$CC  ../ai_wpnav.c

$CC  ../boe_admcmds.c
$CC  ../boe_utils.c

$CC  ../rpm_tcmds.c
$CC  ../rpm_refcmds.c

$CC  ../g_active.c
$CC  ../g_antilag.c
$CC  ../g_bot.c
$CC  ../g_client.c
$CC  ../g_cmds.c
$CC  ../g_combat.c
$CC  ../g_gametype.c
$CC  ../g_items.c
$CC  ../g_misc.c
$CC  ../g_missile.c
$CC  ../g_mover.c
$CC  ../g_session.c
$CC  ../g_spawn.c
$CC  ../g_svcmds.c
$CC  ../g_target.c
$CC  ../g_team.c
$CC  ../g_trigger.c
$CC  ../g_utils.c
$CC  ../g_weapon.c

$CC  ../henkENT.c
$CC  ../henk_hs.c
$CC  ../henk_functions.c
$CC  ../rpm_functions.c

q3asm -f ../game

mkdir "./../../../base/vm"
cp *.qvm "../../../base/vm"

cd ..
