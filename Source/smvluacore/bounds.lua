--- @module 'bounds'
local bounds = {}
bounds.slices = {}
bounds.pl3d = {}
-- bounds.slices["SOOT VISIBILITY"] = {}

-- function bounds.slices["VIS_C0.9H0.1"].x.set(xMin, xMax)
-- TODO: currently the call to set bounds needs to be made after the slice is loaded
function bounds.slices.set(name, vals)
    local min = vals.min
    local max = vals.max
    print("setting bounds for " .. name .. " to " .. min .. "-" .. max)
    local set_min = 2
    local set_max = 2
    if (min ~= nil)
    then
        set_min = 1
    end
    if (max ~= nil)
    then
        set_max = 1
    end
    smvlib.set_slice_bounds(name, set_min, min, set_max, max)
end

function bounds.pl3d.set(name, min, max)
    if (min == nil)
    then
        smvlib.set_pl3d_bound_min(name, false, 0)
    else
        smvlib.set_pl3d_bound_min(name, true, min)
    end
    if (max == nil)
    then
        smvlib.set_pl3d_bound_max(name, false, 0)
    else
        smvlib.set_pl3d_bound_max(name, true, max)
    end
end

function bounds.slices.get(name)
    local min = smvlib.get_slice_bound_min(name)
    local max = smvlib.get_slice_bound_max(name)
    return { min = min, max = max }
end

local slices_mt = {
    -- get method
    __index = function(t, quantity_name)
        local vals = bounds.slices.get(quantity_name)
        local vt = {}
        setmetatable(vt, {
            __newindex = function(t, k, v)
                vals[k] = v
                return bounds.slices.set(quantity_name, vals)
            end,
            __index = function(t, k)
                return vals[k]
            end
        })
        return vt
    end,
    -- set method
    __newindex = function(t, k, v)
        return bounds.slices.set(k, v)
    end
}
setmetatable(bounds.slices, slices_mt)
local pl3d_mt = {
    -- get method
    __index = function(t, k)
        return bounds.pl3d.get(k)
    end,
    -- set method
    __newindex = function(t, k, v)
        return bounds.pl3d.set(k, v)
    end
}
setmetatable(bounds.pl3d, pl3d_mt)

return bounds
