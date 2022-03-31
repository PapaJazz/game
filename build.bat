@echo off

set CompilerFlags=-Ddebug=1 -MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -FC -Z7
set LinkerFlags=-incremental:no -opt:ref -subsystem:windows,5.1 C:\Users\alex\Desktop\game\SDL2-2.0.18\lib\x86\SDL2.lib C:\Users\alex\Desktop\game\SDL2-2.0.18\lib\x86\SDL2main.lib shell32.lib user32.lib gdi32.lib winmm.lib
set IncludeFlags=C:\Users\alex\Desktop\game\SDL2-2.0.18\include

IF NOT EXIST build mkdir build

pushd build

del *.pdb > NUL 2> NUL
echo waiting for PDB > lock.tmp
cl %CompilerFlags% ..\game.cpp /LD /link -incremental:no -opt:ref -PDB:%random%.pdb /EXPORT:game_main_init /EXPORT:game_main_update 
del lock.tmp
cl %CompilerFlags% /Fe:game.exe ..\windows.cpp /I%IncludeFlags% /link %LinkerFlags% 

popd



REM -Fmwindows.map 
