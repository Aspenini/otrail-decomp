-- xmake.lua - build the Oregon Trail port and drive the decompilation tools.
--
--   xmake                     build the port (downloads SDL2 automatically)
--   xmake run oregon_trail    play it  (a window with SDL; screenshots without)
--   xmake f --sdl=n && xmake  build the headless (no-deps) backend instead
--
--   xmake decomp              reverse-engineer the original game
--   xmake verify              re-check the unpack (the regression gate)
--   xmake assets              extract the game art
--   xmake status              show progress
--
-- Python 3 is still required: the tools in tools/ and port/assets/ read and
-- analyse the original binary. xmake orchestrates them and builds the C port;
-- it fetches SDL2 for you so there are no manual dependencies.

set_project("oregon_trail")
set_version("0.1.0")
set_languages("c99")
set_warnings("all")
add_rules("mode.debug", "mode.release")
set_defaultmode("release")

local GAME     = "Oregon_The_1990/OREGON.EXE"
local GAMEDIR  = "Oregon_The_1990"
local UNPACKED = "build/OREGON_unpacked.exe"

-- The decomp tasks need a Python 3 interpreter; the helper lives in
-- tools/xmake/pyhelper.lua so it can be `import`ed inside each task sandbox
-- (top-level functions aren't visible there).
add_moduledirs("tools/xmake")

-- ----------------------------------------------------------------- the port
option("sdl")
    set_default(true)
    set_showmenu(true)
    set_description("Interactive SDL2 window (SDL2 auto-downloaded). Off = headless PNG output.")
option_end()

if has_config("sdl") then
    add_requires("libsdl2", {alias = "sdl2"})
end

target("oregon_trail")
    set_kind("binary")
    set_targetdir("build")
    add_files("port/core/*.c")
    if is_plat("windows") then
        add_defines("_CRT_SECURE_NO_WARNINGS")   -- we use portable C stdio on purpose
    end

    -- generate the built-in font header before compiling
    before_build(function (target)
        import("lib.detect.find_tool")
        local t = find_tool("python3") or find_tool("python")
        if t then os.execv(t.program, {"port/assets/make_font.py"}) end
    end)

    if has_config("sdl") then
        add_files("port/platform/sdl/pal_sdl.c")
        add_packages("sdl2")
    else
        add_files("port/platform/file/pal_file.c")
    end

    -- `xmake run oregon_trail` plays it, with the game directory available
    on_run(function (target)
        os.setenv("OTRAIL_GAMEDIR", path.absolute(GAMEDIR))
        if not has_config("sdl") then
            os.mkdir("build/play")
            os.setenv("OTRAIL_FRAME", path.absolute("build/play/frame.png"))
            os.setenv("OTRAIL_KEYS", " 3 1 6")   -- splash -> top ten -> travel -> quit
            cprint("${color.warning}headless build: writing a screenshot tour to build/play/")
        end
        os.execv(target:targetfile())
    end)
target_end()

-- ------------------------------------------------------------- decomp tasks
task("decomp")
    on_run(function ()
        local py = import("pyhelper")()
        os.mkdir("build")
        os.execv(py, {"tools/unlzexe.py", GAME, UNPACKED})
        os.execv(py, {"tools/map_segments.py", UNPACKED})
        os.execv(py, {"tools/match_inventory.py", UNPACKED})
        os.execv(py, {"tools/verify.py"})
        os.execv(py, {"tools/render_progress_svg.py"})
        cprint("${color.success}decompiled -> %s", UNPACKED)
    end)
    set_menu {usage = "xmake decomp",
              description = "Reverse-engineer the game (unpack, map, verify, dashboard)"}
task_end()

task("dosbuild")
    on_run(function ()
        os.execv(import("pyhelper")(), {"tools/match_build.py", "--build-only"})
    end)
    set_menu {usage = "xmake dosbuild",
              description = "Compile match/ with Turbo C 2.0 under DOSBox-X (stage, build, extract)"}
task_end()

task("match")
    on_run(function ()
        os.mkdir("build/match")
        os.execv(import("pyhelper")(), {"tools/match_build.py"})
    end)
    set_menu {usage = "xmake match",
              description = "Build match/ under DOSBox-X and diff vs the original (% matched)"}
task_end()

task("verify")
    on_run(function ()
        os.execv(import("pyhelper")(), {"tools/verify.py"})
    end)
    set_menu {usage = "xmake verify", description = "Re-check the unpack (regression gate)"}
task_end()

task("assets")
    on_run(function ()
        os.mkdir("build/assets")
        os.execv(import("pyhelper")(), {"port/assets/pcxlib.py", GAMEDIR .. "/OTMCGA.PCL", "build/assets"})
        cprint("${color.success}art extracted -> build/assets/")
    end)
    set_menu {usage = "xmake assets", description = "Extract the game art (images)"}
task_end()

task("status")
    on_run(function ()
        os.execv(import("pyhelper")(), {"tools/status.py"})
    end)
    set_menu {usage = "xmake status", description = "Show how far along the project is"}
task_end()
