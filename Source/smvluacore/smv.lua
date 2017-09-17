--- @module smv
local smv = {}
bounds = require("bounds")
clipping = require("clipping")
load = require("load")
unload = require("unload")
require("render")
-- view = require("view")
require("view")
tour = require("tour")
camera = require("camera")
window = require("window")

-- set the defaults for renderall
render_startframe = 0
render_skipframe = 1

smv.getfinalframe = function()return get_nglobal_times()-1 end
getfinalframe = smv.getfinalframe

function smv.settimeend()
    nframes = get_nglobal_times()
    setframe(nframes-1)
end
settimeend = smv.settimeend

function smv.togglecolorbarflip()
    setcolorbarflip(1-getcolorbarflip())
end
togglecolorbarflip = smv.togglecolorbarflip

function smv.colorbarnormal()
    setcolorbarflip(1)
end
colorbarnormal = smv.colorbarnormal
case = {}
_case = {
    chid = {
        get = function()
            return chid
        end,
        set = function()
            error("case.chid is read-only")
        end
    },
    slices = {
        get = function()
            return sliceinfo
        end,
        set = function()
            error("case.slices is read-only")
        end,
        -- __len = function()
        --     error("len called slices")
        -- end
    },
    -- TODO: provide this by overriding the len operator
    nslices = {
        get = function()
            return #case.slices
        end,
        set = function()
            error("case.nslices is read-only")
        end,
    },
    meshes = {
        get = function()
            -- this relies on initsmvdata being called first
            return meshinfo
        end,
        set = function()
            error("case.meshes is read-only")
        end
    },
    nmeshes = {
        get = function()
            return #case.meshes
        end,
        set = function()
            error("case.nmeshes is read-only")
        end,
    }
}
local case_mt = {
   -- get method
   __index = function (t,k)
       if type(_case[k]) == "function" then
           return _case[k]
       else
           return _case[k].get()
       end
   end,
   -- set method
   __newindex = function (t,k,v)
       _case[k].set(v)
   end
}
setmetatable(case, case_mt)

timebar = {}
_timebar = {
    visibility = {
        get = function()
            return get_timebar_visibility()
        end,
        set = function(v)
            return set_timebar_visibility(v)
        end,
        -- toggle = function ()
        --     timebar.visibility = not timebar.visibility
        -- end
    },
}
local timebar_mt = {
   -- get method
   __index = function (t,k)
       if type(_timebar[k]) == "function" then
           return _timebar[k]
       else
           return _timebar[k].get()
       end
   end,
   -- set method
   __newindex = function (t,k,v)
       _timebar[k].set(v)
   end
}
setmetatable(timebar, timebar_mt)



time = {}
function time.set(time)
    -- TODO: determine if the time is available
    return settime(time)
end


return smv
