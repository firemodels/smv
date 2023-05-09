--- @module 'clipping'
local clipping = {}

local _clipping = {
    mode = {
        get = function()
            return smvlib.get_clipping_mode()
        end,
        set = function(v)
            return smvlib.set_clipping_mode(v)
        end
    },
    set = function(xMin, xMax, yMin, yMax, zMin, zMax)
        print("setting clipping")
        clipping.x.set(xMin, xMax)
        clipping.y.set(yMin, yMax)
        clipping.z.set(zMin, zMax)
    end,
    get = function()
        return {
            x = clipping._x,
            y = clipping._y,
            z = clipping._z,
            mode = clipping.mode
        }
    end
}

-- the real table value
clipping._x = {
    set = function(min, max)
        clipping.x.min = min
        clipping.x.max = max
    end,
    min = {
        set = function(v)
            if v ~= nil
            then smvlib.set_sceneclip_x_min(true, v)
            else smvlib.set_sceneclip_x_min(false, 0)
            end
        end
    },
    max = {
        set = function(v)
            if v ~= nil
            then smvlib.set_sceneclip_x_max(true, v)
            else smvlib.set_sceneclip_x_max(false, 0)
            end
        end,
    }

}
clipping.x = {} -- the proxy
local x_mt = {
    -- get method
    __index = function(t, k)
        if type(clipping._x[k]) == "function" then
            return clipping._x[k]
        else
            return clipping._x[k].get()
        end
    end,
    -- set method
    __newindex = function(t, k, v)
        clipping._x[k].set(v)
    end
}
setmetatable(clipping.x, x_mt)

-- the real table value
clipping._y = {
    set = function(min, max)
        clipping.y.min = min
        clipping.y.max = max
    end,
    min = {
        set = function(v)
            if v ~= nil
            then smvlib.set_sceneclip_y_min(true, v)
            else smvlib.set_sceneclip_y_min(false, 0)
            end
        end
    },
    max = {
        set = function(v)
            if v ~= nil
            then smvlib.set_sceneclip_y_max(true, v)
            else smvlib.set_sceneclip_y_max(false, 0)
            end
        end,
    }

}
clipping.y = {} -- the proxy
local y_mt = {
    -- get method
    __index = function(t, k)
        if type(clipping._y[k]) == "function" then
            return clipping._y[k]
        else
            return clipping._y[k].get()
        end
    end,
    -- set method
    __newindex = function(t, k, v)
        clipping._y[k].set(v)
    end
}
setmetatable(clipping.y, y_mt)

-- the real table value
clipping._z = {
    set = function(min, max)
        clipping.z.min = min
        clipping.z.max = max
    end,
    min = {
        set = function(v)
            if v ~= nil
            then smvlib.set_sceneclip_z_min(true, v)
            else smvlib.set_sceneclip_z_min(false, 0)
            end
        end
    },
    max = {
        set = function(v)
            if v ~= nil
            then smvlib.set_sceneclip_z_max(true, v)
            else smvlib.set_sceneclip_z_max(false, 0)
            end
        end,
    }

}
clipping.z = {} -- the proxy
local z_mt = {
    -- get method
    __index = function(t, k)
        if type(clipping._z[k]) == "function" then
            return clipping._z[k]
        else
            return clipping._z[k].get()
        end
    end,
    -- set method
    __newindex = function(t, k, v)
        clipping._z[k].set(v)
    end
}
setmetatable(clipping.z, z_mt)

local clipping_mt = {
    -- get method
    __index = function(t, k)
        if type(_clipping[k]) == "function" then
            return _clipping[k]
        else
            return _clipping[k].get()
        end
    end,
    -- set method
    __newindex = function(t, k, v)
        return _clipping[k].set(v)
    end
}
setmetatable(clipping, clipping_mt)

return clipping
