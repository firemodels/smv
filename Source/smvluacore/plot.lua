local gp = require('gnuplot')
local lfs = require('lfs')

function createDV(cols, colXIndex, colYIndex, name)
    return {x = cols[colXIndex], y = cols[colYIndex], name = name}
end

function addDVs(dvs,newName)
    local newDV = {}
    newDV.name = newName
    newDV.x = {values = {}}
    newDV.y = {values = {}}
    newDV.x.units = dvs[1].x.units
    newDV.x.name = dvs[1].x.name
    newDV.y.units = dvs[1].y.units
    newDV.y.name = dvs[1].y.name
    for i,v in ipairs(dvs[1].x.values) do
        newDV.x.values[i] = v
        local yval = 0
        for j,b in ipairs(dvs) do
            yval = yval + b.y.values[i]
        end
        newDV.y.values[i] = yval
    end
    return newDV
end

function negateDV(dv)
    local newDV = {
        name = dv.name,
        x = {name = dv.x.name, units = dv.x.units, values = dv.x.values},
        y = {name = dv.y.name, units = dv.y.units, values = {}}
    }
    for i,v in ipairs(dv.y.values) do
        newDV.y.values[i] = -1*dv.y.values[i]
    end
    return newDV
end

function abs(n)
    if (n<0)
    then
        return -n
    else
        return n
    end
end

function absDV(dv)
    local newDV = {
        name = dv.name,
        x = {name = dv.x.name, units = dv.x.units, values = dv.x.values},
        y = {name = dv.y.name, units = dv.y.units, values = {}}
    }
    for i,v in ipairs(dv.y.values) do
        newDV.y.values[i] = abs(dv.y.values[i])
    end
    return newDV
end

function createDVNamed(cols, colXName, colYName, name)
    local xVector
    local yVector
    -- Find the index of the x vector.
    for i,v in ipairs(cols) do
      if (v.name == (colXName)) then
        xVector = v
        break
      end
    end
    if not xVector then
        print("no value for: "..colYName)
        return nil
    end
    -- Find the index of the y vector.
    for i,v in ipairs(cols) do
      if (v.name == (colYName)) then
        yVector = v
        break
      end
    end
    if not yVector then
        print("no value for: "..colYName)
        return nil
    end
    return {x = xVector, y = yVector, name = name}
end

function plotDV(dir, dvs, title, opts)
    if type(dvs) ~= "table" then error("dvs must be a table")
    -- If dvs is a single data vector then use plotSingleDV.
    elseif dvs.x and dvs.y then
        plotMultiDV(dir, {dvs}, title, opts)
    else
        plotMultiDV(dir, dvs, title, opts)
    end
end

function plotSingleDV( dir, dv, title )
    if(dv == nil) then error("dv is nil") end
    lfs.mkdir(dir)
    local g = gp{
        -- All optional, with sane defaults.
        width  = 800,
        height = 500,
        xlabel = dv.x.name .. " (" .. dv.x.units .. ")",
        ylabel = dv.y.name .. " (" .. dv.y.units .. ")",
        key    = "top left",
        title = title .. " for " .. chid:gsub("_","\\\\_"),
        -- consts = {
        --     gamma = 2.5
        -- },

        data = {}

    }

    g.data[1] =  gp.array {  -- plot from an 'array-like' thing in memory. Could be a
                    -- numlua matrix, for example.
            {
                dv.x.values,  -- x
                dv.y.values   -- y
            },

            title = title,          -- optional
            using = {1,2},              -- optional
            with  = 'linespoints'       -- optional
        }
    g:plot(dir .. "/".. title .. '.png')
    -- g:plot(dir .. "/".. title .. '.emf')
end

function plotMultiDV( dir, dvs, title, opts)
    lfs.mkdir(dir)
    local g = gp{
        -- All optional, with sane defaults.
        width  = 800,
        height = 500,
        xlabel = dvs[1].x.name .. " (" .. dvs[1].x.units .. ")",
        ylabel = dvs[1].y.name .. " (" .. dvs[1].y.units .. ")",
        key    = "top left",
        title = title .. " for " .. chid:gsub("_","\\\\_"),
        -- consts = {
        --     gamma = 2.5
        -- },

        data = {}

    }
    if opts then
        for k,v in pairs(opts) do
            g[k] = v
        end
    end
    if (not opts) or (not opts.fname) then
        g.fname = "Arial"
    end
    if (not opts) or (not opts.fsize) then
        g.fsize = 8
    end
    for i,dv in ipairs(dvs) do
        local arr = {  -- plot from an 'array-like' thing in memory. Could be a
                        -- numlua matrix, for example.
                {
                    dv.x.values,  -- x
                    dv.y.values   -- y
                },

                title = dv.y.name,          -- optional
                using = {1,2}              -- optional
            }
        if dv.name then arr.title = dv.name else arr.title = dv.y.name end
        if (dv.with) then arr.with = dv.with else arr.with = 'linespoints' end
        g.data[i] =  gp.array(arr)
    end
    g:plot(dir .. "/".. title .. '.png')
end

function stdGrowthRate(alpha,t)
  if (t<=0) then
    return 0
  else
    return alpha*t^2
  end
end

local slowAlpha = 1000/600^2;
local mediumAlpha = 1000/300^2;
local fastAlpha = 1000/150^2;
local ultrafastAlpha = 1000/75^2;

local slowColour = '#008000'
local mediumColour = '#FF0000'
local fastColour = '#00BFBF'
local ultrafastColour = '#BF00BF'

function createStdHRRCurves(dv, offset)
    if not offset then offset = 0 end
    local maxHRR = 0
    print(dv)
    for i,v in ipairs(dv.y.values) do
        if v > maxHRR then maxHRR = v end
    end
    if maxHRR == 0 then maxHRR = 1 end
    local slowDV = {name = "Slow HRR", x={name = "Time", units = "s", values = {}}, y={name = "Slow HRR", units = "kW", values = {}}}
    local mediumDV = {name = "Medium HRR", x={name = "Time", units = "s", values = {}}, y={name = "Medium HRR", units = "kW", values = {}}}
    local fastDV = {name = "Fast HRR", x={name = "Time", units = "s", values = {}}, y={name = "Fast HRR", units = "kW", values = {}}}
    local ultrafastDV = {name = "Ultrafast HRR", x={name = "Time", units = "s", values = {}}, y={name = "Ultrafast HRR", units = "kW", values = {}}}
    for i,v in ipairs(dv.x.values) do
        local slowVal = stdGrowthRate(slowAlpha,v-offset)
        local mediumVal = stdGrowthRate(mediumAlpha,v-offset)
        local fastVal = stdGrowthRate(fastAlpha,v-offset)
        local ultrafastVal = stdGrowthRate(ultrafastAlpha,v-offset)
        if slowVal < (1.1*maxHRR) then
            slowDV.x.values[i] = v
            slowDV.y.values[i] = slowVal
        end
        if mediumVal < (1.1*maxHRR) then
            mediumDV.x.values[i] = v
            mediumDV.y.values[i] = mediumVal
        end
        if fastVal < (1.1*maxHRR) then
            fastDV.x.values[i] = v
            fastDV.y.values[i] = fastVal
        end
        if ultrafastVal < (1.1*maxHRR) then
            ultrafastDV.x.values[i] = v
            ultrafastDV.y.values[i] = ultrafastVal
        end
    end
    slowDV.with = string.format('lines dashtype 2 lt rgb \"%s\"', slowColour)
    mediumDV.with = string.format('lines dashtype 2 lt rgb \"%s\"', mediumColour)
    fastDV.with = string.format('lines dashtype 2 lt rgb \"%s\"', fastColour)
    ultrafastDV.with = string.format('lines dashtype 2 lt rgb \"%s\"', ultrafastColour)
    return slowDV, mediumDV, fastDV, ultrafastDV
end

function plotHRRDV(plotDir, hrrDV, name, offset, config)
    local vecs
    if hrrDV.x and hrrDV.y then
        print(hrrDV.name)
        local slowDV,mediumDV,fastDV,ultrafastDV = createStdHRRCurves(hrrDV, offset)
        vecs = {hrrDV,slowDV,mediumDV,fastDV,ultrafastDV}
    else
        -- TODO: find the best set of x points from each HRR DV, instead of using
        -- hrrDV[1]
        print(hrrDV[1].name)
        local slowDV,mediumDV,fastDV,ultrafastDV = createStdHRRCurves(hrrDV[1], offset)
        vecs = hrrDV
        for i,v in ipairs({slowDV,mediumDV,fastDV,ultrafastDV}) do
            print(i, v.name)
            vecs[#vecs+1] = v
        end
        for i,v in ipairs(vecs) do
            print(i, v.name)
        end
    end
    return plotDV(plotDir, vecs, name, config)
end

function printVec(vec)
    print(string.format("%s %s (%s) - %s (%s)", vec.name, vec.x.name,
                        vec.x.units, vec.y.name, vec.y.units))
    for i,v in ipairs(vec.x.values) do
        print(string.format("  x: %.2f y: %.2f", vec.x.values[i], vec.y.values[i]))
    end
end
