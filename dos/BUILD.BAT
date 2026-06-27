@echo off
REM ---------------------------------------------------------------------------
REM BUILD.BAT - compile the staged matching sources with Turbo C 2.0.
REM
REM Run from the host via:  python3 tools/dosrun.py --check OUT\BUILD.OK "CALL BUILD.BAT"
REM (xmake dosbuild / xmake match do this for you).
REM
REM Layout (mounted by dosrun.py):
REM   C:           the repo's dos/ folder (this file lives here)
REM   C:\SRC       matching C sources staged from the host's match/ dir
REM   C:\OUT       compiler output (.OBJ) + the BUILD.OK success sentinel
REM   C:\INC       project headers for the matching sources
REM   D:\TC        a Turbo C 2.0 install  (D: mounted from TURBOC or dos/TC)
REM
REM Flags mirror the original build (see docs/matching.md): large model (-ml)
REM and stack checking on (-N). Compile only (-c); the harness diffs the .OBJ's
REM code bytes, so no link step is needed.
REM ---------------------------------------------------------------------------
if exist C:\OUT\BUILD.OK del C:\OUT\BUILD.OK

if not exist D:\TC\BIN\TCC.EXE goto notc

cd C:\OUT
for %%F in (C:\SRC\*.C) do D:\TC\BIN\TCC -ml -N -c -ID:\TC\INCLUDE -IC:\INC %%F
if errorlevel 1 goto failed

echo OK> C:\OUT\BUILD.OK
echo BUILD.BAT: compiled OK
goto end

:notc
echo BUILD.BAT: Turbo C not found at D:\TC\BIN\TCC.EXE
echo            point TURBOC at a Turbo C 2.0 install (see dos\README.md)
goto end

:failed
echo BUILD.BAT: compilation failed

:end
