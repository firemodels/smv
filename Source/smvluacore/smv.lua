--- @module 'smv'
local smv = {}
smv.bounds = require("bounds")
local _clipping = require("clipping")
smv.clipping = _clipping
smv.load = require("load")
smv.unload = require("unload")
smv.render = require("render")
smv.view = require("view")
smv.tour = require("tour")
smv.camera = require("camera")
smv.window = require("window")
smv.exit = smvlib.exit
smv.yieldscript = smvlib.yieldscript

local function basic_ortho_camera()
    return {
        rotationType    = 0,
        -- todo: geometry dependent
        eyePos          = { x = 0.5, y = -1.0, z = 0.5 },
        -- todo: geometry dependent
        zoom            = 1.0,
        viewAngle       = 0,
        directionAngle  = 0,
        elevationAngle  = 0,
        projectionType  = 1,
        -- todo: geometry dependent
        viewDir         = { x = 0.5, y = 0.5, z = 0.5 },
        zAngle          = { az = 0.000000, elev = 0.000000 },
        transformMatrix = nil,
        clipping        = nil
    }
end

function smv.load_default()
    local case = smvlib.load_default()
    rawset(case, "load_slice_std", function(self, slice_type, axis, distance)
        smv.load.slice_std(self, slice_type, axis, distance)
    end)
    rawset(case, "camera_bottom", function(self)
        local camera = basic_ortho_camera()
        camera.zAngle.az = 0.0
        camera.zAngle.elev = -90.0
        return camera
    end)
    rawset(case, "camera_top", function(self)
        local camera = basic_ortho_camera()
        camera.zAngle.az = 0.0
        camera.zAngle.elev = 90.0
        return camera
    end)
    rawset(case, "camera_front", function(self)
        local camera = basic_ortho_camera()
        camera.zAngle.az = 0.0
        camera.zAngle.elev = 0.0
        return camera
    end)
    rawset(case, "camera_left", function(self)
        local camera = basic_ortho_camera()
        camera.zAngle.az = -90.0
        camera.zAngle.elev = 0.0
        return camera
    end)
    rawset(case, "camera_right", function(case)
        local camera = basic_ortho_camera()
        camera.zAngle.az = 90.0
        camera.zAngle.elev = 0.0
        return camera
    end)
    rawset(case, "camera_right", function(case)
        local camera = basic_ortho_camera()
        camera.zAngle.az = 180.0
        camera.zAngle.elev = 0.0
        return camera
    end)
    return nil, nil, case
end

smv.getfinalframe = function() return smvlib.get_nglobal_times() - 1 end

function smv.settimeend()
    local nframes = smvlib.get_nglobal_times()
    smvlib.setframe(nframes - 1)
end

function smv.togglecolorbarflip()
    smvlib.setcolorbarflip(1 - smvlib.getcolorbarflip())
end

function smv.colorbarnormal()
    smvlib.setcolorbarflip(1)
end

smv.timebar = {}
local _timebar = {
    visibility = {
        get = function()
            return smvlib.get_timebar_visibility()
        end,
        set = function(v)
            return smvlib.set_timebar_visibility(v)
        end,
        -- toggle = function ()
        --     timebar.visibility = not timebar.visibility
        -- end
    },
}
local timebar_mt = {
    -- get method
    __index = function(t, k)
        if type(_timebar[k]) == "function" then
            return _timebar[k]
        else
            return _timebar[k].get()
        end
    end,
    -- set method
    __newindex = function(t, k, v)
        _timebar[k].set(v)
    end
}
setmetatable(smv.timebar, timebar_mt)

local _smv = {
    time = {
        get = function()
            return smvlib.gettime()
        end,
        set = function(v)
            return smvlib.settime(v)
        end,
        -- toggle = function ()
        --     timebar.visibility = not timebar.visibility
        -- end
    },
    -- clipping = {
    --     get = function()
    --         return _clipping.get()
    --     end,
    --     set = function(v)
    --         print("setting clipping to " .. v)
    --         return _clipping.set(v)
    --     end,
    -- },
}
local smv_mt = {
    -- get method
    __index = function(t, k)
        if type(_smv[k]) == "function" then
            return _smv[k]
        else
            return _smv[k].get()
        end
    end,
    -- set method
    __newindex = function(t, k, v)
        _smv[k].set(v)
    end
}
setmetatable(smv, smv_mt)

return smv
