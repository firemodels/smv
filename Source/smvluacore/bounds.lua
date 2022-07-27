--- @module bounds
local bounds = {}
bounds.slices = {}
bounds.pl3d = {}
-- bounds.slices["VIS_C0.9H0.1"] = {}
-- bounds.slices["VIS_C0.9H0.1"].x = {}
-- function bounds.slices["VIS_C0.9H0.1"].x.set(xMin, xMax)
function bounds.slices.set(name, min, max)
    local set_min = 2
    local set_max = 2
    if (min ~= nil)
        then
            set_min = 0
        end
    if (max ~= nil)
        then
            set_max = 0
    end
    set_slice_bounds(name,set_min,min,set_max,max)
end

function bounds.pl3d.set(name, min, max)
    if (min == nil)
        then
            set_pl3d_bound_min(name, false, 0)
        else
            set_pl3d_bound_min(name, true, min)
    end
    if (max == nil)
        then
            set_pl3d_bound_max(name, false, 0)
        else
            set_pl3d_bound_max(name, true, max)
    end
end

function bounds.slices.get(name)
    local min = get_slice_bound_min(name)
    local max = get_slice_bound_max(name)
    return {min = min, max = max}
end

return bounds
