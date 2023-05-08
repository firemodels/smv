--- @module 'window'
local window = {}

window.size = function(width, height)
    smvlib.setwindowsize(width, height)
end

return window
