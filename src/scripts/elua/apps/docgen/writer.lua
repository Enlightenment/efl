local util = require("util")

local dutil = require("docgen.util")
local dtree = require("docgen.doctree")

local M = {}

local root_nspace, features

M.has_feature = function(fname)
    if not features then
        return false
    end
    return not not features[fname]
end

local allowed_incflags = {
    noheader = { "noheader", "showheader" },
    firstseconly = { "firstseconly", "fullpage" },
    readmore = { "readmore", "noreadmore" },
    footer = { "footer", "nofooter" },
    link = { "link", "nolink" },
    permalink = { "permalink", "nopermalink" },
    date = { "date", "nodate" },
    mdate = { "mdate", "nomdate" },
    user = { "user", "nouser" },
    comments = { "comments", "nocomments" },
    linkbacks = { "linkbacks", "nolinkbacks" },
    tags = { "tags", "notags" },
    editbutton = { "editbtn", "noeditbtn" },
    redirect = { "redirect", "noredirect" },
    indent = { "indent", "noindent" },
    linkonly = { "linkonly", "nolinkonly" },
    title = { "title", "notitle" },
    pageexists = { "pageexists", "nopageexists" },
    parlink = { "parlink", "noparlink" },
    order = { { "id", "title", "created", "modified", "indexmenu", "custom" } },
    rsort = { "rsort", "sort" },
    depth = 0,
    inline = true,
    beforeeach = "",
    aftereach = ""
}

local writers = {}

local Buffer = {
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

local write_include = function(self, tp, name, flags)
    local it_to_tp = {
        [self.INCLUDE_PAGE] = "page",
        [self.INCLUDE_SECTION] = "section",
        [self.INCLUDE_NAMESPACE] = "namespace",
        [self.INCLUDE_TAG] = "tagtopic"
    }
    if type(name) == "table" then
        if name[#name] == true then
            name[#name] = nil
            name = ":" .. root_nspace .. ":"
                       .. table.concat(name, ":")
        elseif name[#name] == false then
            name[#name] = nil
            name = ":" .. root_nspace .. "-include:"
                       .. table.concat(name, ":")
        else
            name = table.concat(name, ":")
        end
    end
    self:write_raw("{{", it_to_tp[tp], ">", name);
    if flags then
        if tp == self.INCLUDE_SECTION and flags.section then
            self:write_raw("#", flags.section)
        end
        flags.section = nil
        local flstr = {}
        for k, v in pairs(flags) do
            local allow = allowed_incflags[k]
            if allow ~= nil then
                if type(allow) == "boolean" then
                    flstr[#flstr + 1] = k
                elseif type(allow) == "number" or type(allow) == "string" then
                    if type(v) ~= type(allow) then
                        error("invalid value type for flag " .. k)
                    end
                    flstr[#flstr + 1] = k .. "=" .. v
                elseif type(allow) == "table" then
                    if type(allow[1]) == "table" then
                        local valid = false
                        for i, vv in ipairs(allow[1]) do
                            if v == vv then
                                flstr[#flstr + 1] = k .. "=" .. v
                                valid = true
                                break
                            end
                        end
                        if not valid then
                            error("invalid value " .. v .. " for flag " .. k)
                        end
                    elseif type(allow[1]) == "string" and
                           type(allow[2]) == "string" then
                        if v then
                            flstr[#flstr + 1] = allow[1]
                        else
                            flstr[#flstr + 1] = allow[2]
                        end
                    end
                end
            else
                error("invalid include flag: " .. tostring(k))
            end
        end
        flstr = table.concat(flstr, "&")
        if #flstr > 0 then
            self:write_raw("&", flstr)
        end
    end
    self:write_raw("}}")
    self:write_nl()
    return self
end

M.set_backend = function(bend)
    M.Writer = assert(writers[bend], "invalid generation backend")
    M.Buffer = M.Writer:clone(Buffer)
end

writers["dokuwiki"] = util.Object:clone {
    INCLUDE_PAGE = 0,
    INCLUDE_SECTION = 1,
    INCLUDE_NAMESPACE = 2,
    INCLUDE_TAG = 3,

    __ctor = function(self, path, title)
        local subs
        if type(path) == "table" then
            subs = dutil.path_join(unpack(path))
        else
            subs = dutil.nspace_to_path(path)
        end
        dutil.mkdir_p(subs)
        self.file = assert(io.open(dutil.make_page(subs, "txt"), "w"))
        if title then
            if M.has_feature("title") then
                self:write_raw("~~Title: ", title, "~~")
                self:write_nl()
            else
                self:write_h(title, 1)
            end
        end
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

    write_include = function(self, tp, name, flags)
        return write_include(self, tp, name, flags)
    end,

    write_editable = function(self, ns, name)
        ns[#ns + 1] = name
        ns[#ns + 1] = false
        self:write_include(self.INCLUDE_PAGE, ns, {
            date = false, user = false, link = false
        })
        -- restore the table for later reuse
        -- the false gets deleted by write_include
        ns[#ns] = nil
    end,

    write_inherited = function(self, ns)
        ns[#ns + 1] = true
        self:write_include(self.INCLUDE_PAGE, ns, {
            editbutton = false, date = false, user = false, link = false
        })
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
            if target[#target] == false then
                target[#target] = nil
                target = ":" .. root_nspace .. "-include:"
                             .. table.concat(target, ":")
            else
                target[#target] = nil
                target = ":" .. root_nspace .. ":"
                             .. table.concat(target, ":")
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

    write_table = function(self, titles, tbl)
        if titles then
            self:write_raw("^ ", table.concat(titles, " ^ "), " ^\n")
        end
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

    write_par = function(self, str)
        local tokp = dtree.DocTokenizer(str)
        local notetypes = M.has_feature("notes") and {
            [tokp.MARK_NOTE] = "<note>\n",
            [tokp.MARK_WARNING] = "<note warning>\n",
            [tokp.MARK_REMARK] = "<note tip>\n",
            [tokp.MARK_TODO] = "<note>\n**TODO:** "
        } or {}
        local hasraw, hasnote = false, false
        while tokp:tokenize() do
            local tp = tokp:type_get()
            local tag = notetypes[tp]
            if tag then
                self:write_raw(tag)
                hasnote = true
            else
                if not hasraw then
                    self:write_raw("%%")
                    hasraw = true
                end
                if tp == tokp.REF then
                    local reft = tokp:ref_resolve(true)
                    local str = tokp:text_get()
                    if str:sub(1, 1) == "[" then
                        str = str:sub(2, #str - 1)
                    end
                    self:write_raw("%%")
                    self:write_link(reft, str)
                    self:write_raw("%%")
                else
                    local str = tokp:text_get()
                    assert(str, "internal tokenizer error")
                    -- replace possible %% chars
                    str = str:gsub("%%%%", "%%%%<nowiki>%%%%</nowiki>%%%%")
                    if tp == tokp.MARKUP_MONOSPACE then
                        self:write_raw("%%''" .. str .. "''%%")
                    else
                        self:write_raw(str)
                    end
                end
            end
        end
        self:write_raw("%%")
        if hasnote then
            self:write_raw("\n</note>")
        end
        return self
    end,

    write_folded = function(self, title, func)
        if M.has_feature("folds") then
            self:write_raw("++++ ", title, " |\n\n")
        end
        func(self)
        if M.has_feature("folds") then
            self:write_raw("\n\n++++")
        end
        return self
    end,

    finish = function(self)
        self.file:close()
    end
}

M.init = function(root_ns, ftrs)
    root_nspace = root_ns
    features = ftrs
end

return M
