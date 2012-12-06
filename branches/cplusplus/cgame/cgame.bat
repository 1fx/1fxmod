@set include=

del /q vm
mkdir vm
cd vm
set cc=..\..\..\bin\sof2lcc -DQ3_VM -DMISSIONPACK -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui %1

%cc% ../../game/bg_misc.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_weapons.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_player.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_pmove.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_slidemove.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_lib.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_gametype.c
@if errorlevel 1 goto quit
%cc% ../../game/q_math.c
@if errorlevel 1 goto quit
%cc% ../../game/q_shared.c
@if errorlevel 1 goto quit
%cc% ../cg_consolecmds.c
@if errorlevel 1 goto quit
%cc% ../cg_draw.c
@if errorlevel 1 goto quit
%cc% ../cg_drawtools.c
@if errorlevel 1 goto quit
%cc% ../cg_effects.c
@if errorlevel 1 goto quit
%cc% ../cg_ents.c
@if errorlevel 1 goto quit
%cc% ../cg_event.c
@if errorlevel 1 goto quit
%cc% ../cg_gametype.c
@if errorlevel 1 goto quit
%cc% ../cg_gore.c
@if errorlevel 1 goto quit
%cc% ../cg_info.c
@if errorlevel 1 goto quit
%cc% ../cg_light.c
@if errorlevel 1 goto quit
%cc% ../cg_localents.c
@if errorlevel 1 goto quit
%cc% ../cg_main.c
@if errorlevel 1 goto quit
%cc% ../cg_miscents.c
@if errorlevel 1 goto quit
%cc% ../cg_players.c
@if errorlevel 1 goto quit
%cc% ../cg_playerstate.c
@if errorlevel 1 goto quit
%cc% ../cg_predict.c
@if errorlevel 1 goto quit
%cc% ../cg_scoreboard.c
@if errorlevel 1 goto quit
%cc% ../cg_servercmds.c
@if errorlevel 1 goto quit
%cc% ../cg_snapshot.c
@if errorlevel 1 goto quit
%cc% ../cg_view.c
@if errorlevel 1 goto quit
%cc% ../cg_weaponinit.c
@if errorlevel 1 goto quit
%cc% ../cg_weapons.c
@if errorlevel 1 goto quit
%cc% ../../ui/ui_shared.c
@if errorlevel 1 goto quit
%cc% ../cg_newDraw.c
@if errorlevel 1 goto quit

..\..\..\bin\sof2asm -f ../cgame
@if errorlevel 1 goto quit

mkdir "..\..\..\base\vm"
copy *.map "..\..\..\base\vm"
copy *.qvm "..\..\..\base\vm"

:quit
cd ..
