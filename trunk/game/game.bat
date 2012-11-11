@set include=

del /q vm
mkdir vm
cd vm
set cc=..\..\..\bin\sof2lcc -A -DQ3_VM -DQAGAME -DMISSIONPACK -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui %1

%cc%  ../g_main.c
@if errorlevel 1 goto quit

%cc%  ../g_syscalls.c
@if errorlevel 1 goto quit

%cc%  ../bg_misc.c
@if errorlevel 1 goto quit
%cc%  ../bg_lib.c
@if errorlevel 1 goto quit
%cc%  ../bg_pmove.c
@if errorlevel 1 goto quit
%cc%  ../bg_slidemove.c
@if errorlevel 1 goto quit
%cc%  ../bg_player.c
@if errorlevel 1 goto quit
%cc%  ../bg_weapons.c
@if errorlevel 1 goto quit
%cc%  ../bg_gametype.c
@if errorlevel 1 goto quit
%cc%  ../q_math.c
@if errorlevel 1 goto quit
%cc%  ../q_shared.c
@if errorlevel 1 goto quit

%cc%  ../ai_main.c
@if errorlevel 1 goto quit
%cc%  ../ai_util.c
@if errorlevel 1 goto quit
%cc%  ../ai_wpnav.c
@if errorlevel 1 goto quit

%cc%  ../boe_admcmds.c
@if errorlevel 1 goto quit
%cc%  ../boe_utils.c
@if errorlevel 1 goto quit

%cc%  ../rpm_tcmds.c
@if errorlevel 1 goto quit
%cc%  ../rpm_refcmds.c
@if errorlevel 1 goto quit
%cc%  ../rpm_functions.c
@if errorlevel 1 goto quit

%cc%  ../g_active.c
@if errorlevel 1 goto quit
%cc%  ../g_antilag.c
@if errorlevel 1 goto quit
%cc%  ../g_bot.c
@if errorlevel 1 goto quit
%cc%  ../g_client.c
@if errorlevel 1 goto quit
%cc%  ../g_cmds.c
@if errorlevel 1 goto quit
%cc%  ../g_combat.c
@if errorlevel 1 goto quit
%cc%  ../g_gametype.c
@if errorlevel 1 goto quit
%cc%  ../g_items.c
@if errorlevel 1 goto quit
%cc%  ../g_misc.c
@if errorlevel 1 goto quit
%cc%  ../g_missile.c
@if errorlevel 1 goto quit
%cc%  ../g_mover.c
@if errorlevel 1 goto quit
%cc%  ../g_session.c
@if errorlevel 1 goto quit
%cc%  ../g_spawn.c
@if errorlevel 1 goto quit
%cc%  ../g_svcmds.c
@if errorlevel 1 goto quit
%cc%  ../g_target.c
@if errorlevel 1 goto quit
%cc%  ../g_team.c
@if errorlevel 1 goto quit
%cc%  ../g_trigger.c
@if errorlevel 1 goto quit
%cc%  ../g_utils.c
@if errorlevel 1 goto quit
%cc%  ../g_weapon.c
@if errorlevel 1 goto quit
%cc% ../henkENT.c
@if errorlevel 1 goto quit
%cc% ../henk_functions.c
@if errorlevel 1 goto quit
%cc% ../henk_hs.c
@if errorlevel 1 goto quit

..\..\..\bin\sof2asm -f ../game
@if errorlevel 1 goto quit

mkdir "..\..\..\base\vm"
copy *.map "..\..\..\base\vm"
copy *.qvm "..\..\..\base\vm"

:quit
cd ..

pause
echo ""