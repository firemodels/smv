--- @module 'unload'
local unload = {}

function unload.all()
    smvlib.unloadall()
end

function unload.tour()
    smvlib.unloadtour()
end

return unload
