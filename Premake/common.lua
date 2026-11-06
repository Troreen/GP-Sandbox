dirs = {}
local root = path.getabsolute("..", _SCRIPT_DIR)
dirs.root = root .. "/"
dirs.source = root .. "/Source/"
dirs.engine = dirs.source .. "Engine/"
dirs.utilities = dirs.source .. "Utilities/"
dirs.dependencies = root .. "/Dependencies/"
dirs.lib = root .. "/Lib/"
