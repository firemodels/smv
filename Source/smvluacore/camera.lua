--- @media camera
local camera = {}
function camera.get()
    local camera = {
        rotationType = camera_get_rotation_type(),
        rotationIndex = camera_get_rotation_index(),
        eyePos = {
            x = camera_get_eyex(),
            y = camera_get_eyey(),
            z = camera_get_eyez()
        },
        zoom = camera_get_zoom(),
        viewDir = camera_get_viewdir(),
        zAngle = {
            az = camera_get_az(),
            elev = camera_get_elev()
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

function camera.set(camera)
    if camera == nil then
        error("camera.set: camera does not exist")
    end
    if type(camera) == "string" then
        setviewpoint(camera)
        return
    end
    camera_set_rotation_type(camera.rotationType)
    camera_set_eyex(camera.eyePos.x)
    camera_set_eyey(camera.eyePos.y)
    camera_set_eyez(camera.eyePos.z)
    camera_set_zoom(camera.zoom)
    -- TODO: viewAngle
    -- TODO: directionAngle
    -- TODO: elevationAngle
    camera_set_viewdir(camera.viewDir.x, camera.viewDir.y, camera.viewDir.z)
    camera_set_projection_type(camera.projectionType)
    camera_set_elev(camera.zAngle.elev)
    camera_set_az(camera.zAngle.az)
end

return camera
