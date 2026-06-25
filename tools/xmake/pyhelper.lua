-- Shared helper for xmake tasks: locate a Python 3 interpreter.
import("lib.detect.find_tool")

function main()
    local t = find_tool("python3") or find_tool("python")
    assert(t, "Python 3 not found - the decompilation tools require it.")
    return t.program
end
