local util = require("util")

local eomap = require("docgen.mappings")
local dutil = require("docgen.util")

local M = {}

local root_nspace, use_notes

M.Writer = util.Object:clone {
    __ctor = function(self, path)
        local subs
        if type(path) == "table" then
            subs = dutil.path_join(unpack(path))
        else
            subs = dutil.nspace_to_path(path)
        end
        dutil.mkdir_p(subs)
        self.file = assert(io.open(dutil.make_page(subs), "w"))
    end,

    write_raw = function(self, ...)
        self.file:write(...)
        return self
    end,

    write_nl = function(self, n)
        self:write_raw(("\n"):rep(n or 1))
        return self
    end,

    write_h = function(self, heading, level, nonl)
        local s = ("="):rep(7 - level)
        self:write_raw(s, " ", heading, " ", s, "\n")
        if not nonl then
            self:write_nl()
        end
        return self
    end,

    write_fmt = function(self, fmt1, fmt2, ...)
        self:write_raw(fmt1, ...)
        self:write_raw(fmt2)
        return self
    end,

    write_b = function(self, ...)
        self:write_fmt("**", "**", ...)
        return self
    end,

    write_i = function(self, ...)
        self:write_fmt("//", "//", ...)
        return self
    end,

    write_u = function(self, ...)
        self:write_fmt("__", "__", ...)
        return self
    end,

    write_s = function(self, ...)
        self:write_fmt("<del>", "</del>", ...)
        return self
    end,

    write_m = function(self, ...)
        self:write_fmt("''", "''", ...)
        return self
    end,

    write_sub = function(self, ...)
        self:write_fmt("<sub>", "</sub>", ...)
        return self
    end,

    write_sup = function(self, ...)
        self:write_fmt("<sup>", "</sup>", ...)
        return self
    end,

    write_br = function(self, nl)
        self:write_raw("\\\\", nl and "\n" or " ")
        return self
    end,

    write_pre_inline = function(self, ...)
        self:write_fmt("%%", "%%", ...)
        return self
    end,

    write_pre = function(self, ...)
        self:write_fmt("<nowiki>\n", "\n</nowiki>", ...)
        return self
    end,

    write_code = function(self, str, lang)
        lang = lang and (" " .. lang) or ""
        self:write_raw("<code" .. lang .. ">\n", str, "\n</code>\n")
    end,

    write_link = function(self, target, title)
        if type(target) == "table" then
            if target[#target] == true then
                target[#target] = nil
                target = ":" .. root_nspace .. ":"
                             .. table.concat(target, ":")
            else
                target = table.concat(target, ":")
            end
        end
        if not title then
            self:write_raw("[[", target:lower(), "|", target, "]]")
            return
        end
        target = target:lower()
        if type(title) == "string" then
            self:write_raw("[[", target, "|", title, "]]")
            return self
        end
        self:write_raw("[[", target, "|")
        title(self)
        self:write_raw("]]")
        return self
    end,

    write_graph = function(self, tbl)
        self:write_raw("<graphviz>\n")
        self:write_raw("digraph ", tbl.type, " {\n")

        for k, v in pairs(tbl.attrs or {}) do
            self:write_raw("    ", k, " = \"", v, "\"\n")
        end

        local write_attrs = function(attrs)
            if not attrs then
                return
            end
            self:write_raw(" [")
            local first = true
            for k, v in pairs(attrs) do
                if not first then
                    self:write_raw(", ")
                end
                self:write_raw(k, " = \"", v, "\"")
                first = false
            end
            self:write_raw("]")
        end

        if tbl.node then
            self:write_nl()
            self:write_raw("    node")
            write_attrs(tbl.node)
            self:write_nl()
        end
        if tbl.edge then
            if not tbl.node then self:write_nl() end
            self:write_raw("    edge")
            write_attrs(tbl.edge)
            self:write_nl()
        end

        self:write_nl()
        for i, v in ipairs(tbl.nodes) do
            local nname = v.name
            v.name = nil
            self:write_raw("    ", nname)
            write_attrs(v)
            self:write_nl()
        end

        self:write_nl()
        for i, v in ipairs(tbl.connections) do
            local from, to, sep, attrs = v[1], v[2], (v[3] or "->")
            if type(from) == "table" then
                self:write_raw("    {", table.concat(from, ", "), "}")
            else
                self:write_raw("    ", from)
            end
            self:write_raw(" ", sep, " ")
            if type(to) == "table" then
                self:write_raw("{", table.concat(to, ", "), "}")
            else
                self:write_raw(to)
            end
            write_attrs(v[4])
            self:write_nl()
        end

        self:write_raw("}\n</graphviz>")
    end,

    write_table = function(self, titles, tbl)
        self:write_raw("^ ", table.concat(titles, " ^ "), " ^\n")
        for i, v in ipairs(tbl) do
            self:write_raw("| ", table.concat(v,  " | "), " |\n")
        end
        return self
    end,

    write_list = function(self, tbl, ord)
        local prec = ord and "-" or "*"
        for i, v in ipairs(tbl) do
            local lvl, str = 1, v
            if type(v) == "table" then
                lvl, str = v[1] + 1, v[2]
            end
            local pbeg, pend = str:match("([^\n]+)\n(.+)")
            if not pbeg then
                pbeg = str
            end
            self:write_raw(("  "):rep(lvl), prec, " ", str, "\n")
            if pend then
                self:write_raw(pend, "\n")
            end
        end
        return self
    end,

    write_par_markup = function(self, str)
        self:write_raw("%%")
        local f = str:gmatch(".")
        local c = f()
        while c do
            if c == "\\" then
                c = f()
                if c ~= "@" and c ~= "$" then
                    self:write_raw("\\")
                end
                self:write_raw(c)
                c = f()
            elseif c == "$" then
                c = f()
                if c and c:match("[a-zA-Z_]") then
                    local wbuf = { c }
                    c = f()
                    while c and c:match("[a-zA-Z0-9_]") do
                        wbuf[#wbuf + 1] = c
                        c = f()
                    end
                    self:write_raw("%%''" .. table.concat(wbuf) .. "''%%")
                else
                    self:write_raw("$")
                end
            elseif c == "@" then
                c = f()
                if c and c:match("[a-zA-Z_]") then
                    local rbuf = { c }
                    c = f()
                    while c and c:match("[a-zA-Z0-9_.]") do
                        rbuf[#rbuf + 1] = c
                        c = f()
                    end
                    local ldot = false
                    if rbuf[#rbuf] == "." then
                        ldot = true
                        rbuf[#rbuf] = nil
                    end
                    local title = table.concat(rbuf)
                    self:write_raw("%%")
                    self:write_link(eomap.gen_nsp_ref(title, true), title)
                    self:write_raw("%%")
                    if ldot then
                        self:write_raw(".")
                    end
                else
                    self:write_raw("@")
                end
            elseif c == "%" then
                c = f()
                if c == "%" then
                    c = f()
                    self:write_raw("%%<nowiki>%%</nowiki>%%")
                else
                    self:write_raw("%")
                end
            else
                self:write_raw(c)
                c = f()
            end
        end
        self:write_raw("%%")
        return self
    end,

    write_par = function(self, str)
        local notetypes = use_notes and {
            ["Note: "] = "<note>\n",
            ["Warning: "] = "<note warning>\n",
            ["Remark: "] = "<note tip>\n",
            ["TODO: "] = "<note>\n**TODO:** "
        } or {}
        local tag
        for k, v in pairs(notetypes) do
            if str:match("^" .. k) then
                tag = v
                str = str:sub(#k + 1)
                break
            end
        end
        if tag then
            self:write_raw(tag)
            self:write_par_markup(str)
            self:write_raw("\n</note>")
        else
            self:write_par_markup(str)
        end
        return self
    end,

    finish = function(self)
        self.file:close()
    end
}

M.Buffer = M.Writer:clone {
    __ctor = function(self)
        self.buf = {}
    end,

    write_raw = function(self, ...)
        for i, v in ipairs({ ... }) do
            self.buf[#self.buf + 1] = v
        end
        return self
    end,

    finish = function(self)
        self.result = table.concat(self.buf)
        self.buf = {}
        return self.result
    end
}

M.init = function(root_ns, notes)
    root_nspace = root_ns
    use_notes = notes
end

return M
