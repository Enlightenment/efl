-- Xgettext application
-- provides a drop-in replacement of xgettext that supports Lua (but not any
-- other language)

local cutil  = require("cutil")
local getopt = require("getopt")

local VERSION = "1.0.0"

local input_sources  = {}
local search_dirs    = {}
local excluded_files = {}
local keywords       = {}
local flags          = {}

local opts_final = {}
local opts_nolua = {}

local opts, args = getopt.parse {
    usage = "Usage: %prog [OPTION] [INPUTFILE]...",
    args  = arg,
    descs = {
        { category = "Input file location" },

        { metavar = "INPUTFILE ...", help = "input files" },
        { "f", "files-from", true, metavar = "FILE",
            help = "get list of input files from FILE", list = input_sources
        },
        { "D", "directory", true, help = "add DIRECTORY to list for input "
            .. "files search\nIf input file is -, standard input is read.",
            list = search_dirs, opts = opts_nolua
        },

        { category = "Output file location" },

        { "d", "default-domain", true, metavar = "NAME",
            help = "use NAME.po for output (instead of messages.po)",
            opts = opts_final
        },
        { "o", "output", true, metavar = "FILE",
            help = "write output to specified file", opts = opts_final
        },
        { "p", "output-dir", true, metavar = "DIR", help = "output files "
            .. "will be placed in directory DIR\nIf output file is -, "
            .. "output is written to standard output.",
            opts = opts_final
        },

        { category = "Input file interpretation" },

        { "L", "language", true, help = false },
        { "C", "c++", false, help = false,
            callback = function(desc, parser, val, opts) opts["L"] = "C++" end
        },
        { nil, "from-code", true, metavar = "NAME", help = "encoding of "
            .. "input files\nOnly relevant for non-Lua inputs "
            .. "(Lua is always assumed to be UTF-8).", opts = opts_nolua
        },

        { category = "Operation mode" },

        { "j", "join-existing", false,
            help = "join messages with existing file", opts = opts_final
        },
        { "x", "exclude-file", true, metavar = "FILE.po",
            help = "entries from FILE.po are not extracted",
            list = excluded_files
        },
        { "c", "add-comments", nil, metavar = "TAG", help = "place comment "
            .. "blocks (optionally starting with TAG) and preceding "
            .. "keyword lines in output file", opts = opts_nolua
        },

        { category = "Language specific options" },

        { "a", "extract-all", false, help = "extract all strings",
            opts = opts_nolua
        },
        { "k", "keyword", nil, metavar = "WORD", help = "look for WORD as an "
            .. "additional keyword or if not given, do no use default keywords",
            opts = opts_nolua, list = keywords
        },
        { nil, "flag", true, metavar = "WORD:ARG:FLAG", help = "additional "
            .. "flag for strings inside the argument number ARG of keyword WORD",
            opts = opts_nolua, list = flags
        },
        { "T", "trigraphs", false, help = false, opts = opts_nolua },
        { nil, "qt", false, help = false, opts = opts_nolua },
        { nil, "kde", false, help = false, opts = opts_nolua },
        { nil, "boost", false, help = false, opts = opts_nolua },
        { nil, "debug", false, help = "more detailed formatstring "
            .. "recognition results", opts = opts_nolua
        },

        { category = "Output details" },

        { nil, "color", nil, metavar = "WHEN", help = "use colors and other "
            .. "text attributes always or if WHEN. WHEN may be 'always', "
            .. "'never', 'auto', or 'html'", opts = opts_final
        },
        { nil, "style", true, metavar = "STYLEFILE", help = "specify CSS "
            .. "style rule file for --color", opts = opts_final
        },
        { nil, "force-po", false, help = "write PO file even if empty",
            opts = opts_final
        },
        { "i", "indent", false, help = "write the .po file using indented "
            .. "style", opts = opts_final
        },
        { nil, "no-location", false, help = "do not write '#: filename:line' "
            .. "lines", opts = opts_nolua
        },
        { "n", "add-location", false, help = "generate '#: filename:line' "
            .. "lines (default)", opts = opts_nolua
        },
        { nil, "strict", false, help = "write out strict Uniforum "
            .. "conforming .po file", opts = opts_final
        },
        { nil, "properties-output", false, help = "write out a Java "
            .. ".properties file", opts = opts_final
        },
        { nil, "stringtable-output", false, help = "write out a NeXTstep/"
            .. "GNUstep .strings file", opts = opts_final
        },
        { "w", "width", true, metavar = "NUMBER", help = "set output page "
            .. "width", opts = opts_final
        },
        { nil, "no-wrap", false, "do not break long message lines, longer "
            .. "than the output page width, into several lines",
            opts = opts_final
        },
        { "s", "sort-output", false, help = "generate sorted output",
            opts = opts_final
        },
        { "F", "sort-by-file", false, help = "sort output by file location",
            opts = opts_final
        },
        { nil, "omit-header", false, help = "don't write header with "
            .. "'msgid \"\"' entry", opts = opts_final
        },
        { nil, "copyright-holder", true, metavar = "STRING", help = "set "
            .. "copyright holder in output", opts = opts_final
        },
        { nil, "foreign-user", false, help = "omit copyright in output "
            .. "for foreign user", opts = opts_final
        },
        { nil, "package-name", true, metavar = "PACKAGE", help = "set package "
            .. "name in output", opts = opts_final
        },
        { nil, "package-version", true, metavar = "VERSION", help = "set "
            .. "package version in output", opts = opts_final
        },
        { nil, "msgid-bugs-address", true, metavar = "EMAIL@ADDRESS", help =
            "set report address for msgid bugs", opts = opts_final
        },
        { "m", "msgstr-prefix", true, metavar = "STRING", help = "use STRING "
            .. "or \"\" as prefix for msgstr values", opts = opts_final
        },
        { "M", "msgstr-suffix", true, metavar = "STRING", help = "use STRING "
            .. "or \"\" as suffix for msgstr values", opts = opts_final
        },

        { category = "Binaries" },

        { "X", "xgettext", true, metavar = "PATH", help = "path to xgettext." },

        { category = "Informative output" },

        { "h", "help", nil, help = "display this help and exit",
            callback = getopt.help_cb(io.stdout)
        },
        { "v", "version", false, help = "output version information and exit",
            callback = function(p, d)
                print("elua-xgettext (EFL) " .. VERSION)
            end
        },
        error_cb = function(parser, msg)
            io.stderr:write(msg, "\n")
            getopt.help(parser, io.stderr)
        end,
        done_cb = function(parser, opts, args)
        end
    }
}

if not opts or opts["h"] or opts["v"] then
    return true
end

-- transform some lists into mappings
for i = 1, #excluded_files do
    excluded_files[excluded_files[i]] = true
    excluded_files[i] = nil
end
for i = 1, #keywords do
    keywords[keywords[i]] = true
    keywords[i] = nil
end

-- at least one default path
if #search_dirs == 0 then
    search_dirs[1] = "."
end

local build_opt = function(opt)
    local buf = {}
    if opt.short then
        buf[1] = "-"
        buf[2] = opt.short
        if opt.val then
            buf[3] = opt.val
        end
    else
        buf[1] = "--"
        buf[2] = opt.long
        if opt.val then
            buf[3] = "="
            buf[4] = opt.val
        end
    end
    return table.concat(buf)
end

local onlylua  = opts["L"] and opts["L"]:lower() == "lua"
local neverlua = opts["L"] and opts["L"]:lower() ~= "lua"

local  hasxgettext = opts["X"]
if not hasxgettext then
       hasxgettext = os.getenv("XGETTEXT")
end

if not hasxgettext then
    return true
end

local f = io.open(hasxgettext)
if not f then
    return true
end
f:close()

local input_files = {}
for i, v in ipairs(input_sources) do
    local f = io.open(v)
    if f then
        for line in f:lines() do
            if not line:lower():match("^.+%.lua$") then
                input_files[#input_files + 1] = line
            end
        end
    end
end
for i, v in ipairs(args) do
    input_files[#input_files + 1] = v
end

local args_nolua = {}
for i, opt in ipairs(opts_nolua) do
    args_nolua[#args_nolua + 1] = build_opt(opt)
end
args_nolua[#args_nolua + 1] = "--omit-header"
args_nolua[#args_nolua + 1] = "--output=-"
args_nolua[#args_nolua + 1] = false

local parsed_files = {}
for i, fname in ipairs(input_files) do
    if not excluded_files[fname] then
        if onlylua or (not neverlua and fname:lower():match("^.+%.lua$")) then
            -- parse lua files here
        else
            args_nolua[#args_nolua] = fname
            local f = assert(cutil.popenv(hasxgettext, "r",
                unpack(args_nolua)))
            local s = f:read("*all")
            parsed_files[#parsed_files + 1] = s
            f:close()
        end
    end
end

local args_final = {}
for i, opt in ipairs(opts_final) do
    args_final[#args_final + 1] = build_opt(opt)
end
args_final[#args_final + 1] = "--language=PO"
args_final[#args_final + 1] = "-"
local f = assert(cutil.popenv(hasxgettext, "w", unpack(args_final)))
f:write(table.concat(parsed_files, "\n\n"))
f:close()

return true