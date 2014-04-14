-- EFL LuaJIT bindings: Eina (file module)
-- For use with Elua

local ffi = require("ffi")
local C = ffi.C

local iterator = require("eina.iterator")
require("eina.xattr")

ffi.cdef [[
    typedef unsigned char Eina_Bool;

    typedef struct _Eina_File_Direct_Info Eina_File_Direct_Info;
    typedef struct _Eina_Stat             Eina_Stat;
    typedef struct _Eina_File_Line        Eina_File_Line;

    typedef void (*Eina_File_Dir_List_Cb)(const char *name, const char *path,
        void *data);

    typedef Eina_Bool (*Eina_File_Copy_Progress)(void *data,
        unsigned long long done, unsigned long long total);

    typedef enum {
        EINA_FILE_UNKNOWN,
        EINA_FILE_FIFO,
        EINA_FILE_CHR,
        EINA_FILE_DIR,
        EINA_FILE_BLK,
        EINA_FILE_REG,
        EINA_FILE_LNK,
        EINA_FILE_SOCK,
        EINA_FILE_WHT
    } Eina_File_Type;

    typedef struct _Eina_File Eina_File;

    typedef enum {
        EINA_FILE_RANDOM,
        EINA_FILE_SEQUENTIAL,
        EINA_FILE_WILLNEED,
        EINA_FILE_POPULATE,
        EINA_FILE_DONTNEED,
        EINA_FILE_REMOVE
    } Eina_File_Populate;

    struct _Eina_File_Direct_Info {
        size_t               path_length;
        size_t               name_length;
        size_t               name_start;
        Eina_File_Type       type;
        char                 path[8192];
    };

    struct _Eina_Stat {
        unsigned long int    dev;
        unsigned long int    ino;
        unsigned int         mode;
        unsigned int         nlink;
        unsigned int         uid;
        unsigned int         gid;
        unsigned long int    rdev;
        unsigned long int    size;
        unsigned long int    blksize;
        unsigned long int    blocks;
        unsigned long int    atime;
        unsigned long int    atimensec;
        unsigned long int    mtime;
        unsigned long int    mtimensec;
        unsigned long int    ctime;
        unsigned long int    ctimensec;
    };

    struct _Eina_File_Line {
       const char *start;
       const char *end;
       unsigned int index;
       unsigned long long length;
    };

    EAPI Eina_Bool eina_file_dir_list(const char *dir, Eina_Bool recursive,
        Eina_File_Dir_List_Cb cb, void *data);
    EAPI Eina_Iterator *eina_file_ls(const char *dir);
    EAPI Eina_Iterator *eina_file_stat_ls(const char *dir);
    EAPI int eina_file_statat(void *container, Eina_File_Direct_Info *info,
        Eina_Stat *buf);
    EAPI Eina_Iterator *eina_file_direct_ls(const char *dir);
    EAPI char *eina_file_path_sanitize(const char *path);

    typedef enum {
        EINA_FILE_COPY_DATA       = 0,
        EINA_FILE_COPY_PERMISSION = (1 << 0),
        EINA_FILE_COPY_XATTR      = (1 << 1)
    } Eina_File_Copy_Flags;

    EAPI Eina_Bool eina_file_copy(const char *src, const char *dst,
        Eina_File_Copy_Flags flags, Eina_File_Copy_Progress cb,
        const void *cb_data);
    EAPI Eina_File *eina_file_open(const char *name, Eina_Bool shared);
    EAPI Eina_File *eina_file_virtualize(const char *virtual_name,
        const void *data, unsigned long long length, Eina_Bool copy);
    EAPI Eina_Bool eina_file_virtual(Eina_File *file);
    EAPI Eina_Bool eina_file_refresh(Eina_File *file);
    EAPI Eina_File *eina_file_dup(const Eina_File *file);
    EAPI void eina_file_close(Eina_File *file);
    EAPI size_t eina_file_size_get(const Eina_File *file);
    EAPI time_t eina_file_mtime_get(const Eina_File *file);
    EAPI const char *eina_file_filename_get(const Eina_File *file);
    EAPI Eina_Iterator *eina_file_xattr_get(Eina_File *file);
    EAPI Eina_Iterator *eina_file_xattr_value_get(Eina_File *file);
    EAPI void *eina_file_map_all(Eina_File *file, Eina_File_Populate rule);
    EAPI void *eina_file_map_new(Eina_File *file, Eina_File_Populate rule,
        unsigned long int offset, unsigned long int length);
    EAPI void eina_file_map_free(Eina_File *file, void *map);
    EAPI void eina_file_map_populate(Eina_File *file, Eina_File_Populate rule,
        const void *map, unsigned long int offset, unsigned long int length);
    EAPI Eina_Iterator *eina_file_map_lines(Eina_File *file);
    EAPI Eina_Bool eina_file_map_faulted(Eina_File *file, void *map);

    void eina_stringshare_del   (const char *str);
    int  eina_stringshare_strlen(const char *str);

    char *strerror(int);
    void free(void*);
]]

local cutil = require("cutil")
local util  = require("util")

local Object = util.Object

local M = {}

local eina

local init = function()
    eina = util.lib_load("eina")
end

local shutdown = function()
    util.lib_unload("eina")
end

cutil.init_module(init, shutdown)

M.dir_list = function(dir, recursive, cb)
    if not cb or not dir then return false end
    local cbp = ffi.cast("Eina_File_Dir_List_Cb", function(name, path, data)
        return cb(name, path)
    end)
    local v = eina.eina_file_dir_list(dir, recursive or false, cbp, nil)
    cbp:free()
    return v ~= 0
end

local Iterator = iterator.Iterator

local Ls_Iterator = Iterator:clone {
    __ctor = function(self, selfmt, dir)
        return Iterator.__ctor(self, selfmt, eina.eina_file_ls(dir))
    end,

    next = function(self)
        local  v = Iterator.next(self)
        if not v then return nil end
        local  r = ffi.string(v, eina.eina_stringshare_strlen(v))
        eina.eina_stringshare_del(v)
        return r
    end
}

M.ls = function(dir) return Ls_Iterator(dir) end

local file_type_map = {
    [C.EINA_FILE_UNKNOWN] = "unknown",
    [C.EINA_FILE_FIFO]    = "fifo",
    [C.EINA_FILE_CHR]     = "chr",
    [C.EINA_FILE_DIR]     = "dir",
    [C.EINA_FILE_BLK]     = "blk",
    [C.EINA_FILE_REG]     = "reg",
    [C.EINA_FILE_LNK]     = "lnk",
    [C.EINA_FILE_SOCK]    = "sock",
    [C.EINA_FILE_WHT]     = "wht",
}

local Direct_Info = Object:clone {
    __ctor = function(self, path, name_start, name_length, tp)
        self.path        = path
        self.name_start  = name_start
        self.name_length = name_length
        self.type        = tp
    end,

    statat = function(self, container)
        if not container then return false, "invalid container" end
        local info = ffi.new("Eina_File_Direct_Info", #self.path,
            self.name_length, self.name_start - 1,
            C["EINA_FILE_" .. self.type:upper()], self.path)
        local buf = ffi.new("Eina_Stat")
        if eina.eina_file_statat(container, info, buf) ~= 0 then
            return false, ffi.string(C.strerror(ffi.errno()))
        end
        return buf
    end
}

local direct_info_iterator_next = function(self)
    local  v = Iterator.next(self)
    if not v then return nil end
    local s = ffi.cast("Eina_File_Direct_Info*", v)
    local path = ffi.string(s.path, s.path_length)
    local ns = tonumber(s.name_start)
    local nl = tonumber(s.name_length)
    local tp = file_type_map[s.type]
    return Direct_Info(path, ns, nl, tp), self:container_get()
end

local Stat_Ls_Iterator = Iterator:clone {
    __ctor = function(self, selfmt, dir)
        return Iterator.__ctor(self, selfmt, eina.eina_file_stat_ls(dir))
    end,
    next = direct_info_iterator_next
}

M.stat_ls = function(dir) return Stat_Ls_Iterator(dir) end

local Direct_Ls_Iterator = Iterator:clone {
    __ctor = function(self, selfmt, dir)
        return Iterator.__ctor(self, selfmt, eina.eina_file_direct_ls(dir))
    end,
    next = direct_info_iterator_next
}

M.direct_ls = function(dir) return Direct_Ls_Iterator(dir) end

M.path_sanitize = function(path)
    local v = eina.eina_file_path_sanitize(path)
    if v == nil then return nil end
    local r = ffi.string(v)
    C.free(v)
    return r
end

M.copy_flags = {
    DATA = 0, PERMISION = 1, XATTR = 2
}

M.copy = function(source, destination, flags, cb)
    if not source or not destination then return false end
    flags = flags or 0
    local cbp
    if cb then
        cbp = ffi.cast("Eina_File_Copy_Progress", function(data, done, total)
            return not not cb(done, total)
        end)
    end
    local v = eina.eina_file_copy(source, destination, flags, cbp, nil)
    if cbp then cbp:free() end
    return v ~= 0
end

local Xattr_Iterator = Iterator:clone {
    __ctor = function(self, selfmt, file)
        return Iterator.__ctor(self, selfmt, eina.eina_file_xattr_get(file))
    end,
    next = function(self)
        local  v = Iterator.next(self)
        if not v then return nil end
        return ffi.string(v)
    end
}

local Xattr_Value_Iterator = Iterator:clone {
    __ctor = function(self, selfmt, file)
        return Iterator.__ctor(self, selfmt,
            eina.eina_file_xattr_value_get(file))
    end,
    next = function(self)
        local  v = Iterator.next(self)
        if not v then return nil end
        v = ffi.cast(v, "Eina_Xattr*")
        return ffi.string(v.name), ffi.string(v.value, v.length)
    end
}

M.populate     = {
    RANDOM     = 0,
    SEQUENTIAL = 1,
    WILLNEED   = 2,
    POPULATE   = 3,
    DONTNEED   = 4,
    REMOVE     = 5
}

local Line_Iterator = Iterator:clone {
    __ctor = function(self, selfmt, file)
        return Iterator.__ctor(self, selfmt, eina.eina_file_map_lines(file))
    end,
    next = function(self)
        local  v = Iterator.next(self)
        if not v then return nil end
        v = ffi.cast(v, "Eina_File_Line*")
        return ffi.string(v.start, v.length), tonumber(v.index)
    end
}

M.File = ffi.metatype("Eina_File", {
    __new = function(self, name, shared)
        return self.open(name, shared)
    end,
    __len = function(self)
        return self:size_get()
    end,
    __index = {
        open = function(name, shared)
            return eina.eina_file_open(name, shared)
        end,
        virtualize = function(vname, data, length, copy)
            return eina.eina_file_virtualize(vname, data, length,
                copy or false)
        end,

        close = function(self)
            return eina.eina_file_close(self)
        end,

        dup = function(self)
            return eina.eina_file_dup(self)
        end,

        is_virtual = function(self)
            return eina.eina_file_virtual(self) ~= 0
        end,
        refresh = function(self)
            return eina.eina_file_refresh(self) ~= 0
        end,

        size_get = function(self)
            return tonumber(eina.eina_file_size_get(self))
        end,

        mtime_get = function(self)
            return tonumber(eina.eina_file_mtime_get(self))
        end,

        filename_get = function(self)
            return ffi.string(eina.eina_file_filename_get(self))
        end,

        xattr_get = function(self) return Xattr_Iterator(self) end,
        xattr_value_get = function(self) Xattr_Value_Iterator(self) end,

        map_all = function(self, rule, raw)
            local v = ffi.cast("char*", eina.eina_file_map_all(self, rule or 0))
            if v == nil then return nil end
            if not raw then
                local r = ffi.string(v)
                self:map_free(v)
                return r
            end
            return v
        end,

        map_new = function(self, rule, offset, length, raw)
            local v = ffi.cast("char*", eina.eina_file_map_new(self, rule or 0,
                offset or 0, length))
            if v == nil then return nil end
            if not raw then
                local r = ffi.string(v, length)
                self:map_free(v)
                return r
            end
            return v
        end,

        map_free = function(self, map)
            return eina.eina_file_map_free(self, map)
        end,

        map_populate = function(self, rule, map, offset, length)
            return eina.eina_file_map_populate(self, rule or 0, offset or 0,
                length)
        end,

        map_faulted = function(self, map)
            return eina.eina_file_map_faulted(self, map) ~= 0
        end,

        lines = function(self) return Line_Iterator(self) end
    }
})

return M