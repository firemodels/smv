print("Running script for " .. fdsprefix .. ".")
--hidewindow()
print("Date: " .. os.date("%c"))
package.path=package.path .. ";" .. "../../SMV/Build/gnu_linux_64/?.lua"
smv = require "smv"
-- ssf = require "ssf"
iniparser = require "iniparser"
string = require "string"

parseINI("test.ini")
exit()