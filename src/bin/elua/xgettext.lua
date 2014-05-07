-- Xgettext application
-- provides a drop-in replacement of xgettext that supports Lua (but not any
-- other language)

local cutil  = require("cutil")
local getopt = require("getopt")

local VERSION = "1.0.0"

local opts, args = getopt.parse {
    usage = "Usage: %prog [OPTION] [INPUTFILE]...",
    args  = arg,
    descs = {
        { category = "Input file location" },

        { metavar = "INPUTFILE ...", help = "input files" },
        { "f", "files-from", true, metavar = "FILE",
            help = "get list of input files from FILE"
        },
        { "D", "directory", true, help = "add DIRECTORY to list for input "
            .. "files search\nIf input file is -, standard input is read."
        },

        { category = "Output file location" },

        { "d", "default-domain", true, metavar = "NAME",
            help = "use NAME.po for output (instead of messages.po)"
        },
        { "o", "output", true, metavar = "FILE",
            help = "write output to specified file"
        },
        { "p", "output-dir", true, metavar = "DIR", help = "output files "
            .. "will be placed in directory DIR\nIf output file is -, "
            .. "output is written to standard output."
        },

        { category = "Input file interpretation" },

        { "L", "language", true, help = false },
        { "C", "c++", false, help = false },
        { nil, "from-code", true, metavar = "NAME", help = "encoding of "
            .. "input files\nBy default the input files are assumed to "
            .. "be in ASCII."
        },

        { category = "Operation mode" },

        { "j", "join-existing", false,
            help = "join essages with existing file"
        },
        { "x", "exclude-file", true, metavar = "FILE.po",
            help = "entries from FILE.po are not extracted"
        },
        { "c", "add-comments", nil, metavar = "TAG", help = "place comment "
            .. "blocks (optionally starting with TAG) and preceding "
            .. "keyword lines in output file"
        },

        { category = "Language specific options" },

        { "a", "extract-all", false, help = "extract all strings" },
        { "k", "keyword", nil, metavar = "WORD", help = "look for WORD as an "
            .. "additional keyword or if not given, do no use default keywords"
        },
        { nil, "flag", true, metavar = "WORD:ARG:FLAG", help = "additional "
            .. "flag for strings inside the argument number ARG of keyword WORD"
        },

        { category = "Output details" },

        { "T", "trigraphs", false, help = false },
        { nil, "qt", false, help = false },
        { nil, "kde", false, help = false },
        { nil, "boost", false, help = false },
        { nil, "debug", false, help = "more detailed formatstring "
            .. "recognition results"
        },
        { nil, "color", nil, metavar = "WHEN", help = "use colors and other "
            .. "text attributes always or if WHEN. WHEN may be 'always', "
            .. "'never', 'auto', or 'html'"
        },
        { nil, "style", true, metavar = "STYLEFILE", help = "specify CSS "
            .. "style rule file for --color"
        },
        { "e", "no-escape", false, help = "do not use C escapes in output "
            .. "(default)"
        },
        { "E", "escape", false, help = "use C escapes in output, no "
            .. "extended chars"
        },
        { nil, "force-po", false, help = "write PO file even if empty" },
        { "i", "indent", false, help = "wrute the .po file using indented "
            .. "style"
        },
        { nil, "no-location", false, help = "do not write '#: filename:line' "
            .. "lines"
        },
        { "n", "add-location", false, help = "generate '#: filename:line' "
            .. "lines (default)"
        },
        { nil, "strict", false, help = "write out strict Uniforum "
            .. "conforming .po file"
        },
        { nil, "properties-output", false, help = "write out a Java "
            .. ".properties file"
        },
        { nil, "stringtable-output", false, help = "write out a NeXTstep/"
            .. "GNUstep .strings file"
        },
        { "w", "width", true, metavar = "NUMBER", help = "set output page "
            .. "width"
        },
        { nil, "no-wrap", false, "do not break long message lines, longer "
            .. "than the output page width, into several lines"
        },
        { "s", "sort-output", false, help = "generate sorted output" },
        { "F", "sort-by-file", false, help = "sort output by file location" },
        { nil, "omit-header", false, help = "don't write header with "
            .. "'msgid \"\"' entry"
        },
        { nil, "copyright-holder", true, metavar = "STRING", help = "set "
            .. "copyright holder in output"
        },
        { nil, "foreign-user", false, help = "omit copyright in output "
            .. "for foreign user"
        },
        { nil, "package-name", true, metavar = "PACKAGE", help = "set package "
            .. "name in output"
        },
        { nil, "package-version", true, metavar = "VERSION", help = "set "
            .. "package version in output"
        },
        { nil, "msgid-bugs-address", true, metavar = "EMAIL@ADDRESS", help =
            "set report address for msgid bugs"
        },
        { "m", "msgstr-prefix", true, metavar = "STRING", help = "use STRING "
            .. "or \"\" as prefix for msgstr values"
        },
        { "M", "msgstr-suffix", true, metavar = "STRING", help = "use STRING "
            .. "or \"\" as suffix for msgstr values"
        },

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

if not opts or opts["h"] or opts["v"] or #args == 0 then
    return true
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

local hasxgettext = os.getenv("XGETTEXT")
if hasxgettext and not onlylua then
    local gargs = { hasxgettext }
    if not opts["j"] and not neverlua then
        for i, v in ipairs(args) do
            if v:lower():match("^.+%.lua$") then
                gargs[#gargs + 1] = "--join-existing"
                break
            end
        end
    end
    for i, opt in ipairs(opts) do
        gargs[#gargs + 1] = build_opt(opt)
    end
    local ngargs = #gargs
    for i, v in ipairs(args) do
        if not v:lower():match("^.+%.lua$") then
            gargs[#gargs + 1] = v
        end
    end
    if #gargs ~= ngargs then
        cutil.exec(unpack(gargs))
    end
end

return true