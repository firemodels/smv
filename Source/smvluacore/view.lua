--- @module 'view'
local view = { colorbar = {}, blockages = {}, color = {}, titlebox = {}, surfaces = {}, devices = {}, outline = {} }
view.camera = require("camera")
local _view = {
    -- colorbar = {
    --     get = function()
    --         return get_colorbar_visibility()
    --     end,
    --     set = function(setting)
    --         if (type(setting) == "boolean")
    --             then set_colorbar_visibility(setting)
    --         else
    --             error("the argument of set_colorbar_visibility must be a boolean, but it is a " .. type(setting))
    --         end
    --     end
    -- },
    render_directory = {
        get = function()
            return smvlib.getrenderdir()
        end,
        set = function(v)
            return smvlib.setrenderdir(v)
        end
    },
    render = require("render"),
    bounds = require("bounds"),
    show_chid = {
        get = function()
            return smvlib.get_chid_visibility()
        end,
        set = function(v)
            return smvlib.set_chid_visibility(v)
        end
    },
    show_title = {
        get = function()
            return smvlib.get_title_visibility()
        end,
        set = function(v)
            return smvlib.set_title_visibility(v)
        end
    },
    show_smv_version = {
        get = function()
            return smvlib.get_smv_version_visibility()
        end,
        set = function(v)
            return smvlib.set_smv_version_visibility(v)
        end
    },
    show_kernel_version = {
        get = function()
            return smvlib.get_version_info_visibility()
        end,
        set = function(v)
            return smvlib.set_version_info_visibility(v)
        end
    },
    frame = {
        get = function()
            return smvlib.getframe()
        end,
        set = function(v)
            return smvlib.setframe(v)
        end
    },
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
    viewpoint = {
        get = function()
            return smvlib.getviewpoint()
        end,
        set = function(v)
            local errorcode = smvlib.setviewpoint(v)
            assert(errorcode == 0, string.format("setviewpoint errorcode: %d\n", errorcode))
            return errorcode
        end
    },
    color2bar = {
        get = function()
            return smvlib.get_color2bar_colors()
        end,
        set = function(colors)
            print("setting color2bar colors")
            return smvlib.set_color2bar_colors(#colors, colors)
        end
    },
    load = function(case)
    end
}
local view_mt = {
    -- get method
    __index = function(t, k)
        if type(_view[k]) == "function" then
            return _view[k]
        elseif k == "render" or k == "bounds" then
                return _view[k]
        else
            return _view[k].get()
        end
    end,
    -- set method
    __newindex = function(t, k, v)
        assert(_view[k], "_view." .. tostring(k) .. " does not exist.")
        _view[k].set(v)
    end
}
function _view.set_time_end()
    local nframes = smvlib.get_nglobal_times()
    smvlib.setframe(nframes - 1)
end

setmetatable(view, view_mt)

local _colorbar = {
    flip = {
        get = function()
            return smvlib.getcolorbarflip()
        end,
        set = function(v)
            return smvlib.setcolorbarflip(v)
        end
    },
    texture_flag = {
        get = function()
            return nil -- getcolorbarflip()
        end,
        set = function(v)
            return smvlib.set_colorbar_textureflag(v)
        end
    },
    index = {
        get = function()
            return smvlib.getcolorbarindex()
        end,
        set = function(v)
            smvlib.setcolorbarindex(v)
        end
    },
    show = {
        get = function()
            return smvlib.get_colorbar_visibility()
        end,
        set = function(setting)
            if (type(setting) == "boolean")
            then
                smvlib.set_colorbar_visibility(setting)
            else
                error("the argument of set_colorbar_visibility must be a boolean, but it is a " .. type(setting))
            end
        end
    },
    colors = {
        get = function()
            return smvlib.get_colorbar_colors()
        end,
        set = function(colors)
            print("setting colorbar colors")
            return smvlib.set_colorbar_colors(colors)
        end
    },
    preset = {
        -- get = function()
        --     return smvlib.get_colorbar_preset()
        -- end,
        set = function(presetName)
            return smvlib.set_named_colorbar(presetName)
        end
    }
}
local colorbar_mt = {
    -- get method
    __index = function(t, k)
        if type(_colorbar[k]) == "function" then
            return _colorbar[k]
        else
            return _colorbar[k].get()
        end
    end,
    -- set method
    __newindex = function(t, k, v)
        _colorbar[k].set(v)
    end
}
setmetatable(view.colorbar, colorbar_mt)

local _titlebox = {
    add_line = function(line)
        return smvlib.add_title_line(line)
    end

}
local titlebox_mt = {
    -- get method
    __index = function(t, k)
        if type(_titlebox[k]) == "function" then
            return _titlebox[k]
        else
            return _titlebox[k].get()
        end
    end,
    -- set method
    __newindex = function(t, k, v)
        _titlebox[k].set(v)
    end
}

setmetatable(view.titlebox, titlebox_mt)


local _color = {
    ambientlight = {
        -- get = function ()
        --     return getcolorbarflip()
        -- end,
        set = function(v)
            return smvlib.set_ambientlight(v.r, v.g, v.b)
        end
    },
    flip = {
        get = function()
            return smvlib.get_flip()
        end,
        set = function(v)
            return smvlib.set_flip(v)
        end
    },
    background = {
        get = function()
            return smvlib.get_backgroundcolor()
        end,
        set = function(v)
            smvlib.set_backgroundcolor(v.r, v.g, v.b)
        end
    },
    foreground = {
        get = function()
            return smvlib.get_foregroundcolor()
        end,
        set = function(v)
            smvlib.set_foregroundcolor(v.r, v.g, v.b)
        end
    },
    blockcolor = {
        -- get = function()
        --     return get_colorbar_visibility()
        -- end,
        set = function(v)
            return smvlib.set_blockcolor(v.r, v.g, v.b)
        end
    },
    blockshininess = {
        -- get = function ()
        --     return getcolorbarindex()
        -- end,
        set = function(v)
            return smvlib.set_blockshininess(v)
        end
    },
    blockspecular = {
        -- get = function()
        --     return get_colorbar_visibility()
        -- end,
        set = function(v)
            return smvlib.set_blockspecular(v.r, v.g, v.b)
        end
    },
    boundcolor = {
        -- get = function()
        --     return get_colorbar_visibility()
        -- end,
        set = function(v)
            return smvlib.set_boundcolor(v.r, v.g, v.b)
        end
    },
    diffuselight = {
        -- get = function()
        --     return get_colorbar_visibility()
        -- end,
        set = function(v)
            return smvlib.set_diffuselight(v.r, v.g, v.b)
        end
    }
}
local color_mt = {
    -- get method
    __index = function(t, k)
        if type(_color[k]) == "function" then
            return _color[k]
        else
            return _color[k].get()
        end
    end,
    -- set method
    __newindex = function(t, k, v)
        _color[k].set(v)
    end
}
setmetatable(view.color, color_mt)

-- View Method
-- 1 - Defined in input file
-- 2 - Solid
-- 3 - Outine only
-- 4 - Outline added
-- 5 - Hidden
local viewMethodTable = {
    as_input = 1,
    solid = 2,
    outline_only = 3,
    outline_added = 4,
    hidden = 5
}
local outlineColorTable = {
    blockage = 1,
    foreground = 2
}
local locationViewTable = {
    grid = 1,
    exact = 2,
    cad = 3
}
local convertTo = function(table, strValue)
    local v = table[strValue]
    if v == nil then
        error("invalid view method: " .. strValue)
    else
        return v
    end
end
local convertFrom = function(table, intValue)
    for key, value in pairs(table) do
        if (value == intValue) then
            return key
        end
    end
    error("invalid view method value: " .. intValue)
end

local _blockages = {
    method = {
        -- get
        -- blockage_view_method(int setting)
        set = function(setting)
            if (type(setting) == "string") then
                smvlib.blockage_view_method(convertTo(viewMethodTable, setting))
            else
                error("view.blockages.method expected string but got " .. type(setting))
            end
        end
    },
    outline_color = {
        -- get
        -- blockage_view_method(int setting)
        set = function(setting)
            if (type(setting) == "string") then
                smvlib.blockage_outline_color(convertTo(outlineColorTable, setting))
            else
                error("view.blockages.outline_color expected string but got " .. type(setting))
            end
        end
    },
    locations = {
        -- get
        -- blockage_view_method(int setting)
        set = function(setting)
            if (type(setting) == "string") then
                smvlib.blockage_locations(convertTo(locationViewTable, setting))
            else
                error("view.blockages.locations expected string but got " .. type(setting))
            end
        end
    },
    hide_all = function()
        return smvlib.blockages_hide_all()
    end
    -- get = function()
    --     return smvlib.get_blockages_visibility()
    -- end,
    -- set = function(v)
    --     return smvlib.set_blockages_visibility(v)
    -- end
}

local blockages_mt = {
    -- get method
    __index = function(t, k)
        if type(_blockages[k]) == "function" then
            return _blockages[k]
        else
            return _blockages[k].get()
        end
    end,
    -- set method
    __newindex = function(t, k, v)
        _blockages[k].set(v)
    end
}
setmetatable(view.blockages, blockages_mt)

local _surfaces = {
    hide_all = function()
        return smvlib.surfaces_hide_all()
    end
    -- get = function()
    --     return smvlib.get_surfaces_visibility()
    -- end,
    -- set = function(v)
    --     return smvlib.set_surfaces_visibility(v)
    -- end
}
local surfaces_mt = {
    -- get method
    __index = function(t, k)
        if type(_surfaces[k]) == "function" then
            return _surfaces[k]
        else
            return _surfaces[k].get()
        end
    end,
    -- set method
    __newindex = function(t, k, v)
        _surfaces[k].set(v)
    end
}
setmetatable(view.surfaces, surfaces_mt)

local _devices = {
    hide_all = function()
        return smvlib.devices_hide_all()
    end
    -- get = function()
    --     return smvlib.get_surfaces_visibility()
    -- end,
    -- set = function(v)
    --     return smvlib.set_surfaces_visibility(v)
    -- end
}
local devices_mt = {
    -- get method
    __index = function(t, k)
        if type(_devices[k]) == "function" then
            return _devices[k]
        else
            return _devices[k].get()
        end
    end,
    -- set method
    __newindex = function(t, k, v)
        _devices[k].set(v)
    end
}
setmetatable(view.devices, devices_mt)

local _outline = {
    hide = function()
        return smvlib.outline_hide()
    end
    -- get = function()
    --     return smvlib.get_surfaces_visibility()
    -- end,
    -- set = function(v)
    --     return smvlib.set_surfaces_visibility(v)
    -- end
}
local outline_mt = {
    -- get method
    __index = function(t, k)
        if type(_outline[k]) == "function" then
            return _outline[k]
        else
            return _outline[k].get()
        end
    end,
    -- set method
    __newindex = function(t, k, v)
        _outline[k].set(v)
    end
}
setmetatable(view.outline, outline_mt)

return view
