package.path=package.path .. ";" .. "../../SMV/Build/gnu_linux_64/?.lua"
local smv = require("smv")
local string = require("string")
local instance, view, case = smv.load_default()
print("Running script for " .. case.chid .. ".")
print("Date: " .. os.date("%c"))

blockage_view_method(3)
render("outline only raw")
view.blockages.method = "outline_only"
render("outline only interface")
blockage_outline_color(1)
render("blockageokage color outline raw")
blockage_outline_color(2)
render("foreground color outline raw")
instance.exit()
