package.path=package.path .. ";" .. "../../SMV/Build/gnu_linux_64/?.lua"
local smv = require("smv")
local iniparser = require("iniparser")
local string = require("string")

local instance, view, case = smv.load_default()
print("Running script for " .. case.chid .. ".")
print("Date: " .. os.date("%c"))

iniparser.parseINI("test.ini")
instance.exit()
