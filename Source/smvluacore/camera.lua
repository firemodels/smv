--- @module 'camera'
local camera = {}
function camera.get()
    local camera = {
        rotationType = smvlib.camera_get_rotation_type(),
        rotationIndex = smvlib.camera_get_rotation_index(),
        eyePos = {
            x = smvlib.camera_get_eyex(),
            y = smvlib.camera_get_eyey(),
            z = smvlib.camera_get_eyez()
        },
        zoom = smvlib.camera_get_zoom(),
        viewDir = smvlib.camera_get_viewdir(),
        zAngle = {
            az = smvlib.camera_get_az(),
            elev = smvlib.camera_get_elev()
        }
    }
    return camera
end

function camera.print(camera)
    io.write(string.format("rotationType: %d\n", camera.rotationType))
    -- io.write(string.format("rotationIndex: %d\n", camera.rotationIndex))
    io.write(string.format("viewId: %s\n", camera.viewId))
    io.write(string.format("eyePos: (%f,%f,%f)\n",
        camera.eyePos.x, camera.eyePos.y, camera.eyePos.x))
    io.write(string.format("zoom: %f\n", camera.zoom))
    io.write(string.format("viewAngle: %f\n", camera.viewAngle))
    io.write(string.format("directionAngle: %f\n", camera.directionAngle))
    io.write(string.format("elevationAngle: %f\n", camera.elevationAngle))
    io.write(string.format("projectionType: %d\n", camera.projectionType))
    io.write(string.format("viewDir: (%f,%f,%f)\n",
        camera.viewDir.x, camera.viewDir.y, camera.viewDir.z))
    io.write(string.format("zAngle: (%f,%f)\n",
        camera.zAngle.az, camera.zAngle.elev))
    -- io.write(string.format("transformMatrix: %d\n", camera.transformMatrix))
    local clipping = camera.clipping
    io.write(string.format("clipping: "))
    if clipping == nil or clipping.mode == nil then
        io.write(string.format("mode: 0\n"))
    else
        io.write(string.format("mode: %d\n", clipping.mode))
    end
    io.write(string.format("  x: ", camera.clipping))
    if clipping == nil or clipping.x == nil or clipping.x.min == nil then
        io.write(string.format("-inf", camera.clipping))
    else
        io.write(string.format("%f", clipping.x.min))
    end
    io.write(" to ")
    if clipping == nil or clipping.x == nil or clipping.x.max == nil then
        io.write(string.format("+inf", camera.clipping))
    else
        io.write(string.format("%f", clipping.x.max))
    end
    io.write("\n")
    io.write(string.format("  y: ", camera.clipping))
    if clipping == nil or clipping.y == nil or clipping.y.min == nil then
        io.write(string.format("-inf", camera.clipping))
    else
        io.write(string.format("%f", clipping.y.min))
    end
    io.write(" to ")
    if clipping == nil or clipping.y == nil or clipping.y.max == nil then
        io.write(string.format("+inf", camera.clipping))
    else
        io.write(string.format("%f", clipping.y.max))
    end
    io.write("\n")
    io.write(string.format("  z: ", camera.clipping))
    if clipping == nil or clipping.z == nil or clipping.z.min == nil then
        io.write(string.format("-inf", camera.clipping))
    else
        io.write(string.format("%f", clipping.z.min))
    end
    io.write(" to ")
    if clipping == nil or clipping.z == nil or clipping.z.max == nil then
        io.write(string.format("+inf", camera.clipping))
    else
        io.write(string.format("%f", clipping.z.max))
    end
    io.write("\n")
    -- transformMatrix = {
    --      1.000000 0.000000 0.000000 0.000000
    --      0.000000 1.000000 0.000000 0.000000
    --      0.000000 0.000000 1.000000 0.000000
    --      0.000000 0.000000 0.000000 1.000000
    -- },
end

function camera.set_projection(v)
    if not (type(v) == "number" and (v == 0 or v == 1)) then
        error("projection type: " .. v .. " invalid")
    end
    local errorcode = smvlib.camera_set_projection_type(v)
    assert(errorcode == 0, string.format("set_projection_type errorcode: %d\n", errorcode))
    return errorcode
end

function camera.set_orthographic()
    return camera.set_projection(1)
end

--     get = function()
--         return smvlib.camera_get_projection_type()
--     end,
--     set = function(v)
--         if not (type(v) == "number" and (v == 0 or v == 1)) then
--             error("projection type: " .. v .. " invalid")
--         end
--         local errorcode = smvlib.camera_set_projection_type(v)
--         assert(errorcode == 0, string.format("set_projection_type errorcode: %d\n", errorcode))
--         return errorcode
--     end
-- }
function camera.set_az(az)
    return smvlib.camera_set_az(az)
end
function camera.set_elev(az)
    return smvlib.camera_set_elev(az)
end
function camera.zoom_to_fit()
    return smvlib.camera_zoom_to_fit()
end
function camera.from_z_max()
    -- TODO: Determine best rotation.
    camera.set_orthographic()
    camera.set_elev(90)
    camera.set_az(90)
    camera.zoom_to_fit()
end
function camera.from_y_min()
    camera.set("YMIN")
end
function camera.set_ortho_preset(camera)
    smvlib.set_ortho_preset(camera)
end
    function camera.set(camera)
    if camera == nil then
        error("camera.set: camera does not exist")
    end
    if type(camera) == "string" then
        smvlib.setviewpoint(camera)
        return
    end
    smvlib.camera_set_rotation_type(camera.rotationType)
    smvlib.camera_set_projection_type(camera.projectionType)
    smvlib.camera_set_az(camera.zAngle.az)
    smvlib.camera_set_elev(camera.zAngle.elev)
    smvlib.camera_set_zoom(camera.zoom)
    smvlib.camera_set_eyex(camera.eyePos.x)
    smvlib.camera_set_eyey(camera.eyePos.y)
    smvlib.camera_set_eyez(camera.eyePos.z)
    -- TODO: viewAngle
    -- TODO: directionAngle
    -- TODO: elevationAngle
    smvlib.camera_set_viewdir(camera.viewDir.x, camera.viewDir.y, camera.viewDir.z)
    -- TODO: the below is nonsensical, but it helps
    smvlib.settime(smvlib.gettime())

end

return camera
