-- EFL LuaJIT bindings: Eina (file module)
-- For use with Elua

local ffi = require("ffi")
local C = ffi.C

local iterator = require("eina.iterator")

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

    extern int errno;
    char *strerror(int);

    void free(void *ptr);
]]

local cutil = require("cutil")
local util  = require("util")

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
    return v == 1
end

local Iterator = iterator.Iterator

M.Ls_Iterator = Iterator:clone {
    __ctor = function(self, dir)
        return Iterator.__ctor(self, eina.eina_file_ls(dir))
    end,

    next = function(self)
        local  v = Iterator.next(self)
        if not v then return nil end
        local  r = ffi.string(v, eina.eina_stringshare_strlen(v))
        eina.eina_stringshare_del(v)
        return r
    end
}

M.ls = M.Ls_Iterator

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

local Direct_Info = util.Object:clone {
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
            return false, ffi.string(C.strerror(C.errno))
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

M.Stat_Ls_Iterator = Iterator:clone {
    __ctor = function(self, dir)
        return Iterator.__ctor(self, eina.eina_file_stat_ls(dir))
    end,

    next = direct_info_iterator_next
}

M.stat_ls = M.Stat_Ls_Iterator

M.Direct_Ls_Iterator = Iterator:clone {
    __ctor = function(self, dir)
        return Iterator.__ctor(self, eina.eina_file_direct_ls(dir))
    end,

    next = direct_info_iterator_next
}

M.direct_ls = M.Direct_Ls_Iterator

M.path_sanitize = function(path)
    local v = eina.eina_file_path_sanitize(path)
    if v == nil then return nil end
    local r = ffi.string(v)
    C.free(v)
    return r
end

return M