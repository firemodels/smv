function loadAndSumPL3DFrame(n)
    for i,v in ipairs(pl3d.frames[n].entries) do
        load.datafile(v.file)
    end
    -- initsmvdata()
    local var1 = 0
    local var2 = 0
    local var3 = 0
    local var4 = 0
    local var5 = 0
    local nvals = 0
    for i,v in ipairs(pl3d.frames[n].entries) do
        local a,b,c,d,e,vals = get_qdata_sum(v.blocknumber)
        var1 = var1 + a
        var2 = var2 + b
        var3 = var3 + c
        var4 = var4 + d
        var5 = var5 + e
        nvals = nvals + vals
    end
    return var1,var2,var3,var4,var5,nvals
end

function loadAndSumPL3DFrameMesh(n,blocknumber)
    for i,v in ipairs(pl3d.frames[n].entries) do
        print(i,v)
    end
    local e = pl3d.frames[n].entries[blocknumber]
    load.datafile(e.file)
    -- initsmvdata()
    local var1 = 0
    local var2 = 0
    local var3 = 0
    local var4 = 0
    local var5 = 0
    local nvals = 0

    local v = pl3d.frames[n].entries[blocknumber]
    local a,b,c,d,e,vals = get_qdata_sum(v.blocknumber)
    var1 = var1 + a
    var2 = var2 + b
    var3 = var3 + c
    var4 = var4 + d
    var5 = var5 + e
    nvals = nvals + vals
    return var1,var2,var3,var4,var5,nvals
end


function loadAndSumPL3DFrameMeshBounded(n,blocknumber,i1,i2,j1,j2,k1,k2)
    for i,v in ipairs(pl3d.frames[n].entries) do
        print(i,v)
    end
    local e = pl3d.frames[n].entries[blocknumber]
    load.datafile(e.file)
    -- initsmvdata()
    local var1 = 0
    local var2 = 0
    local var3 = 0
    local var4 = 0
    local var5 = 0
    local nvals = 0

    local v = pl3d.frames[n].entries[blocknumber]
    print("calling qdata")
    local a,b,c,d,e,vals = get_qdata_sum_bounded(v.blocknumber,i1,i2,j1,j2,k1,k2)
    var1 = var1 + a
    var2 = var2 + b
    var3 = var3 + c
    var4 = var4 + d
    var5 = var5 + e
    nvals = nvals + vals
    return var1,var2,var3,var4,var5,nvals
end


function loadAndMaxPL3DFrameMeshBounded(n,blocknumber,i1,i2,j1,j2,k1,k2)
    for i,v in ipairs(pl3d.frames[n].entries) do
        print(i,v)
    end
    local e = pl3d.frames[n].entries[blocknumber]
    load.datafile(e.file)
    -- initsmvdata()
    local var1 = 0
    local var2 = 0
    local var3 = 0
    local var4 = 0
    local var5 = 0
    local nvals = 0

    local v = pl3d.frames[n].entries[blocknumber]
    print("calling qdata")
    local a,b,c,d,e,vals = get_qdata_max_bounded(v.blocknumber,i1,i2,j1,j2,k1,k2)
    var1 = var1 + a
    var2 = var2 + b
    var3 = var3 + c
    var4 = var4 + d
    var5 = var5 + e
    nvals = nvals + vals
    return var1,var2,var3,var4,var5,nvals
end

function loadAndMeanPL3DFrameMesh(n, blocknumber)
    local var1,var2,var3,var4,var5,nvals = loadAndSumPL3DFrameMesh(n, blocknumber)
    return var1/nvals,var2/nvals,var3/nvals,var4/nvals,var5/nvals
end

function loadAndMeanPL3DFrameMeshBounded(n, blocknumber,i1,i2,j1,j2,k1,k2)
    local var1,var2,var3,var4,var5,nvals = loadAndSumPL3DFrameMeshBounded(n, blocknumber,i1,i2,j1,j2,k1,k2)
    return var1/nvals,var2/nvals,var3/nvals,var4/nvals,var5/nvals
end

function loadAndMeanPL3DFrame(n)
    local var1,var2,var3,var4,var5,nvals = loadAndSumPL3DFrame(n)
    return var1/nvals,var2/nvals,var3/nvals,var4/nvals,var5/nvals
end

function loadAndMeanPL3DVectors()
    local vec1 = {name = plot3dinfo[1].label[1].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[1].longlabel,
        units = plot3dinfo[1].label[1].unit,
        values = {}}}
    local vec2 = {name = plot3dinfo[1].label[2].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[2].longlabel,
        units = plot3dinfo[1].label[2].unit,
        values = {}}}
    local vec3 = {name = plot3dinfo[1].label[3].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[3].longlabel,
        units = plot3dinfo[1].label[3].unit,
        values = {}}}
    local vec4 = {name = plot3dinfo[1].label[4].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[4].longlabel,
        units = plot3dinfo[1].label[4].unit,
        values = {}}}
    local vec5 = {name = plot3dinfo[1].label[5].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[5].longlabel,
        units = plot3dinfo[1].label[5].unit,
        values = {}}}
    for i,v in ipairs(pl3d.frames) do
        local var1,var2,var3,var4,var5 = loadAndMeanPL3DFrame(i)
        vec1.x.values[i] = v.time
        vec2.x.values[i] = v.time
        vec3.x.values[i] = v.time
        vec4.x.values[i] = v.time
        vec5.x.values[i] = v.time

        vec1.y.values[i] = var1
        vec2.y.values[i] = var2
        vec3.y.values[i] = var3
        vec4.y.values[i] = var4
        vec5.y.values[i] = var5
    end
    return vec1,vec2,vec3,vec4,vec5
end

function loadAndMeanPL3DVectorsMesh(blocknumber)
    local vec1 = {name = plot3dinfo[1].label[1].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[1].longlabel,
        units = plot3dinfo[1].label[1].unit,
        values = {}}}
    local vec2 = {name = plot3dinfo[1].label[2].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[2].longlabel,
        units = plot3dinfo[1].label[2].unit,
        values = {}}}
    local vec3 = {name = plot3dinfo[1].label[3].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[3].longlabel,
        units = plot3dinfo[1].label[3].unit,
        values = {}}}
    local vec4 = {name = plot3dinfo[1].label[4].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[4].longlabel,
        units = plot3dinfo[1].label[4].unit,
        values = {}}}
    local vec5 = {name = plot3dinfo[1].label[5].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[5].longlabel,
        units = plot3dinfo[1].label[5].unit,
        values = {}}}
    for i,v in ipairs(pl3d.frames) do
        local var1,var2,var3,var4,var5 = loadAndMeanPL3DFrameMesh(i,blocknumber)
        vec1.x.values[i] = v.time
        vec2.x.values[i] = v.time
        vec3.x.values[i] = v.time
        vec4.x.values[i] = v.time
        vec5.x.values[i] = v.time

        vec1.y.values[i] = var1
        vec2.y.values[i] = var2
        vec3.y.values[i] = var3
        vec4.y.values[i] = var4
        vec5.y.values[i] = var5
    end
    return vec1,vec2,vec3,vec4,vec5
end

function loadAndMeanPL3DVectorsMeshBounded(blocknumber,i1,i2,j1,j2,k1,k2)
    local vec1 = {name = plot3dinfo[1].label[1].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[1].longlabel,
        units = plot3dinfo[1].label[1].unit,
        values = {}}}
    local vec2 = {name = plot3dinfo[1].label[2].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[2].longlabel,
        units = plot3dinfo[1].label[2].unit,
        values = {}}}
    local vec3 = {name = plot3dinfo[1].label[3].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[3].longlabel,
        units = plot3dinfo[1].label[3].unit,
        values = {}}}
    local vec4 = {name = plot3dinfo[1].label[4].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[4].longlabel,
        units = plot3dinfo[1].label[4].unit,
        values = {}}}
    local vec5 = {name = plot3dinfo[1].label[5].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[5].longlabel,
        units = plot3dinfo[1].label[5].unit,
        values = {}}}
    for i,v in ipairs(pl3d.frames) do
        local var1,var2,var3,var4,var5 = loadAndMeanPL3DFrameMeshBounded(i,blocknumber,i1,i2,j1,j2,k1,k2)
        vec1.x.values[i] = v.time
        vec2.x.values[i] = v.time
        vec3.x.values[i] = v.time
        vec4.x.values[i] = v.time
        vec5.x.values[i] = v.time

        vec1.y.values[i] = var1
        vec2.y.values[i] = var2
        vec3.y.values[i] = var3
        vec4.y.values[i] = var4
        vec5.y.values[i] = var5
    end
    return vec1,vec2,vec3,vec4,vec5
end


function loadAndMaxPL3DVectorsMeshBounded(blocknumber,i1,i2,j1,j2,k1,k2)
    local vec1 = {name = plot3dinfo[1].label[1].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[1].longlabel,
        units = plot3dinfo[1].label[1].unit,
        values = {}}}
    local vec2 = {name = plot3dinfo[1].label[2].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[2].longlabel,
        units = plot3dinfo[1].label[2].unit,
        values = {}}}
    local vec3 = {name = plot3dinfo[1].label[3].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[3].longlabel,
        units = plot3dinfo[1].label[3].unit,
        values = {}}}
    local vec4 = {name = plot3dinfo[1].label[4].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[4].longlabel,
        units = plot3dinfo[1].label[4].unit,
        values = {}}}
    local vec5 = {name = plot3dinfo[1].label[5].longlabel,
        x = {name = "Time", units = "s", values = {}}, y =
        {name = plot3dinfo[1].label[5].longlabel,
        units = plot3dinfo[1].label[5].unit,
        values = {}}}
    for i,v in ipairs(pl3d.frames) do
        local var1,var2,var3,var4,var5 = loadAndMaxPL3DFrameMeshBounded(i,blocknumber,i1,i2,j1,j2,k1,k2)
        vec1.x.values[i] = v.time
        vec2.x.values[i] = v.time
        vec3.x.values[i] = v.time
        vec4.x.values[i] = v.time
        vec5.x.values[i] = v.time

        vec1.y.values[i] = var1
        vec2.y.values[i] = var2
        vec3.y.values[i] = var3
        vec4.y.values[i] = var4
        vec5.y.values[i] = var5
    end
    return vec1,vec2,vec3,vec4,vec5
end
