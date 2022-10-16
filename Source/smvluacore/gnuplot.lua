--[[
=================================================================
*
* Copyright (c) 2013-2014 Lucas Hermann Negri
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify,
* merge, publish, distribute, sublicense, and/or sell copies of the
* Software, and to permit persons to whom the Software is furnished
* to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* ==================================================================
--]]

local gnuplot = {}

-- ** auxiliary functions **
local temp_files = {}

local function remove_temp_files()
    for _, fname in ipairs(temp_files) do
        os.remove(fname)
    end
end

local function write_temp_file(content)
    -- local name = os.tmpname()
    local name = tostring(math.floor(math.random()*100000))
    local file = io.open(name, 'w')
    file:write(content)
    file:close()

    table.insert(temp_files, name)

    return name
end

local function add(t, ...)
    table.insert(t, string.format(...))
end

-- which strings should be quoted
local quoted = {
    xlabel      = true,
    ylabel      = true,
    zlabel      = true,
    xformat     = true,
    yformat     = true,
    zformat     = true,
    decimalsign = true,
    output      = true,
    title       = true,
}

local special = {
    xformat     = 'set format x "%s"',
    yformat     = 'set format y "%s"',
    width       = false,
    height      = false,
    fname       = false,
    fsize       = false,
}

-- outputs that should be persistent
local persist = {
    wxt = true,
    qt  = true,
}

-- terminal types
gnuplot.terminal = {
    png = "pngcairo enhanced",
    svg = "svg dashed enhanced",
    pdf = "pdfcairo linewidth 4 rounded fontscale 1.0",
    wxt = "wxt enhanced",
    qt  = "qt enhanced",
}

local options = {
    -- header
    function(g, code)
        local tm   = gnuplot.terminal[g._type]
        if (not tm) then
            print("tm:", tm)
            -- error("nil tm")
        end
        local size = ""
        if g.width and g.height then
            size = ("size %s, %s"):format(g.width, g.height)
        end
        add(code, 'set terminal %s %s font "%s,%d"', tm, size, g.fname, g.fsize)
    end,

    -- configs
    function(g, code)
        for k, v in pairs(g) do
            if k:sub(1,1) ~= '_' then
                -- string. ex.: set logscale x or set xlabel "X label"
                if type(v) == 'string' or type(v) == 'number' then
                    if special[k] ~= nil then
                        local fmt = special[k]

                        if fmt ~= false then
                            add(code, fmt, v)
                        end
                    elseif quoted[k] then
                        add(code, 'set %s "%s"', k, v)
                    else
                        add(code, 'set %s %s', k, v)
                    end
                -- boolean. ex.: set grid
                elseif type(v) == 'boolean' then
                    add(code, '%s %s', v and 'set' or 'unset', k)
                end
            end
        end
    end,

    -- style
    function(g, code)
        if type(g.style) == 'table' then
            for _, v in ipairs(g.style) do
                add(code, 'set style %s', v)
            end
        end
    end,

    -- constants
    function(g, code)
        if type(g.consts) == 'table' then
            for k, v in pairs(g.consts) do
                add(code, '%s = %s', k, v)
            end
        end
    end,
}

-- returns a string with the gnuplot script
function gnuplot.codegen(g, cmd, path)
    g._type  = g.type or path:match("%.([^%.]+)$")
    g.type   = nil
    if not persist[g._type] then g.output = path end

    local code = {}
    for _, f in ipairs(options) do
        f(g, code)
    end

    local plot_cmd = {}

    for i = 1, #g.data do
        local d    = g.data[i]
        local u    = d.using and 'u ' .. table.concat(d.using, ':') or ''
        local name = d.file  and string.format('"%s"', d[1]) or d[1]

        local opts = {name, u}
        table.insert(opts, d.with      and 'w  ' .. d.with         )
        table.insert(opts, d.width     and 'lw ' .. d.width        )
        table.insert(opts, d.style     and 'ls ' .. d.style        )
        table.insert(opts, d.type      and 'lt ' .. d.type         )
        table.insert(opts, d.color     and 'lc ' .. d.color        )
        table.insert(opts, d.ptype     and 'pt ' .. d.ptype        )
        table.insert(opts, d.psize     and 'ps ' .. d.psize        )
        table.insert(opts, d.pinterval and 'pi ' .. d.pinterval    )
        table.insert(opts, 't "' .. (d.title or '') .. '"'         )

        add(plot_cmd, table.concat(opts, ' '))
        g.__gpcache = nil
    end

    add(code, cmd .. ' ' .. table.concat(plot_cmd, ', '))
    if g._pause then add(code, 'pause ' .. g._pause) end

    return table.concat(code, '\n')
end

osname = ""

if package.config:sub(1,1) == "\\" then
    osname = "windows"
else
    osname = "not windows"
end

function gnuplot.do_plot(g, cmd, path)
    local code = gnuplot.codegen(g, cmd, path)
    local name = write_temp_file( code )
    local opt = ""
    if persist[g._type] then opt = '--persist' end
    -- A hack to use 'start' on windows to get around path issues.
    local command = string.format("%s %s %s", gnuplot.bin, opt, name)
    local success
    local result
    local err_code
    success, result, err_code = os.execute( command )
    if not success then error(string.format("plot failed with %d", err_code)) end
    return g
end

-- 2D plot
function gnuplot.plot(g, path)
    local ret = gnuplot.do_plot(g, "plot", path)
    remove_temp_files()
    return ret
end

-- 3D plot
function gnuplot.splot(g, path)
    local ret = gnuplot.do_plot(g, "splot", path)
    remove_temp_files()
    return ret
end

-- ** Constructor and mt **

local plot_mt
plot_mt = {
    -- constructor
    __call = function(_, p)
        setmetatable(p, plot_mt)
        return p
    end,

    -- defaults
    __index = {
        fname  = "Arial",
        fsize  = 12,
        plot   = gnuplot.plot,
        splot  = gnuplot.splot,
        bin    = 'gnuplot',
        grid   = 'back',
        xlabel = 'X',
        ylabel = 'Y'
    },
}

setmetatable(gnuplot, plot_mt)
setmetatable(plot_mt, {__gc = remove_temp_files }) -- ugly

-- ** Data to plot **

-- native gnuplot function
function gnuplot.gpfunc(arg)
    return arg
end

-- simple: data already in a file
function gnuplot.file(arg)
    arg.file = true
    return arg
end

-- data is in a table that must be saved to a temp file
function gnuplot.array(arg)
    local array = {}
    local data  = arg[1]

    if not data.__gpcache then
        for line = 1, #data[1] do
            -- insert regular line
            local aux = {}
            for col = 1, #data do
                table.insert(aux, data[col][line])
            end

            table.insert(array, table.concat(aux, ' '))

             -- insert a blank line, for contour plots
            if arg.blank and line % arg.blank == 0 then
                table.insert(array, '')
            end
        end

        local lines  = table.concat(array, '\n')
        data.__gpcache = write_temp_file(lines)
    end

    arg[1] = data.__gpcache
    return gnuplot.file(arg)
end

-- arg[1] must implement serialize(self), that should return a string to be
-- saved in a temp file
function gnuplot.custom(arg)
    local data = arg[1]

    if not data.__gpcache then
        local lines = data:serialize()
        data.__gpcache = write_temp_file(lines)
    end

    arg[1] = data.__gpcache
    return gnuplot.file(arg)
end

-- generate data from a function, then save it to a temp file
function gnuplot.func(arg)
    local array = { {}, {} }
    local func  = arg[1]
    local range = arg.range or {-5, 5, 0.1}

    for x = range[1], range[2], range[3] do
        table.insert(array[1], x       )
        table.insert(array[2], func(x) )
    end

    arg[1] = array
    return gnuplot.array(arg)
end

return gnuplot
