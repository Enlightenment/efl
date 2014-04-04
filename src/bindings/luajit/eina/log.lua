-- EFL LuaJIT bindings: Eina (log module)
-- For use with Elua

local ffi = require("ffi")
local C = ffi.C

ffi.cdef [[
    typedef enum _Eina_Log_Level {
        EINA_LOG_LEVEL_CRITICAL,
        EINA_LOG_LEVEL_ERR,
        EINA_LOG_LEVEL_WARN,
        EINA_LOG_LEVEL_INFO,
        EINA_LOG_LEVEL_DBG,
        EINA_LOG_LEVELS,
        EINA_LOG_LEVEL_UNKNOWN = (-2147483647 - 1)
    } Eina_Log_Level;

    extern int EINA_LOG_DOMAIN_GLOBAL;

    int  eina_log_domain_register  (const char *name, const char *color);
    void eina_log_domain_unregister(int domain);

    int  eina_log_domain_registered_level_get(int domain);
    void eina_log_domain_registered_level_set(int domain, int level);

    void eina_log_print(int            domain,
                        Eina_Log_Level level,
                        const char    *file,
                        const char    *function,
                        int            line,
                        const char    *fmt,
                        ...);

    typedef struct _Domain_Private {
        int domain;
    } Domain_Private;

    typedef struct _Domain Domain;
]]

local cutil = require("cutil")
local util  = require("util")

local M = {}

local eina
local default_domain, global_domain

local init = function()
    eina = util.lib_load("eina")
    global_domain  = ffi.new("Domain_Private", eina.EINA_LOG_DOMAIN_GLOBAL)
    default_domain = global_domain
end

local shutdown = function()
    util.lib_unload("eina")
    default_domain, global_domain = nil, nil
end

cutil.init_module(init, shutdown)

M.level = {
    CRITICAL = C.EINA_LOG_LEVEL_CRITICAL,
    ERR      = C.EINA_LOG_LEVEL_ERR,
    WARN     = C.EINA_LOG_LEVEL_WARN,
    INFO     = C.EINA_LOG_LEVEL_INFO,
    DBG      = C.EINA_LOG_LEVEL_DBG,
    UNKNOWN  = C.EINA_LOG_LEVEL_UNKNOWN,
    levels   = C.EINA_LOG_LEVELS
}

M.color = {
    LIGHTRED  = "\x1B[31;1m",
    RED       = "\x1B[31m",
    LIGHTBLUE = "\x1B[34;1m",
    BLUE      = "\x1B[34m",
    GREEN     = "\x1B[32;1m",
    YELLOW    = "\x1B[33;1m",
    ORANGE    = "\x1B[0;33m",
    WHITE     = "\x1B[37;1m",
    LIGHTCYAN = "\x1B[36;1m",
    CYAN      = "\x1B[36m",
    RESET     = "\x1B[0m",
    HIGH      = "\x1B[1m"
}

local get_dom = function(dom)
    dom = dom:__get_domain()
    if not dom then return nil end
    return ffi.cast("Domain_Private*", dom).domain
end

local unregister_dom = function(dom)
    eina.eina_log_domain_unregister(ffi.cast("Domain_Private*", dom).domain)
end

local log_full = function(dom, level, file, func, line, msg)
    dom = get_dom(dom)
    if not dom then return end
    eina.eina_log_print(dom, level, file, func, line, msg)
end
M.log_full = log_full

local getinfo = debug.getinfo

local log = function(dom, level, msg, loff)
    local info = getinfo(2 + (loff or 0), "nlSf")
    log_full(dom, level, info.source,
        info.name or "<" .. tostring(info.func) .. ">", info.currentline, msg)
end
M.log = log

M.Domain_Base = util.Object:clone {
    set_level = function(self, level)
        local dom = get_dom(self)
        if not dom then return end
        eina.eina_log_domain_registered_level_set(dom, level)
    end,

    get_level = function(self)
        local dom = get_dom(self)
        if not dom then return -1 end
        return eina.eina_log_domain_registered_level_get(dom)
    end,

    __get_domain = function(self)
        return nil
    end,

    log = function(self, level, msg, loff)
        log(self, level, msg, (loff or 0) + 1)
    end
}

M.Domain_Global = M.Domain_Base:clone {
    __get_domain = function(self)
        return global_domain
    end
}

M.global_domain = M.Domain_Global

M.Domain_Default = M.Domain_Base:clone {
    __get_domain = function(self)
        return default_domain
    end
}

M.default_domain = M.Domain_Default

M.Domain = M.Domain_Base:clone {
    __ctor = function(self, name, color)
        self.__domain = ffi.cast("Domain*", ffi.new("Domain_Private",
            eina.eina_log_domain_register(name, color)))
        ffi.gc(self.__domain, unregister_dom)
    end,

    unregister = function(self)
        if not self.__domain then return end
        unregister_dom(ffi.gc(self.__domain, nil))
        self.__domain = nil
    end,

    __get_domain = function(self)
        return self.__domain
    end
}

return M