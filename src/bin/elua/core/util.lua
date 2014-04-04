-- elua core utilities used in other modules

local M = {}

M.Object = {
    __call = function(self, ...)
        local r = {
            __index = self, __proto = self, __call = self.__call,
            __tostring = self.__tostring
        }
        setmetatable(r, r)
        if self.__ctor then self.__ctor(r, ...) end
        return r
    end,

    clone = function(self, o)
        o = o or {}
        o.__index, o.__proto, o.__call = self, self, self.__call
        if not o.__tostring then
            o.__tostring = self.__tostring
        end
        setmetatable(o, o)
        return o
    end,

    is_a = function(self, base)
        if self == base then return true end
        local pt = self.__proto
        local is = (pt == base)
        while not is and pt do
            pt = pt.__proto
            is = (pt == base)
        end
        return is
    end,

    __tostring = function(self)
        return ("Object: %s"):format(self.name or "unnamed")
    end
}

return M