-- Lualian application
-- for use with Elua

local lualian = require("lualian")
local  getopt = require("getopt")

local gen_file = function(opts, i, fname)
    local printv  = opts["v"] and print or function() end
    printv("Generating for file: " .. fname)
    local ofile   = opts["o"] and opts["o"][i] or nil
    local fstream = io.stdout
    if ofile then
        printv("  Output file: " .. ofile)
        fstream = io.open(ofile, "w")
        if not fstream then
            error("Cannot open output file: " .. ofile)
        end
    else
        printv("  Output file: printing to stdout...")
    end
    lualian.generate(fname, opts["M"], opts["L"], fstream)
end

getopt.parse {
    usage = "Usage: %prog [OPTIONS] file1.eo file2.eo ... fileN.eo",
    args  = arg,
    descs = {
        { category = "General" },

        { "h", "help", nil, help = "Show this message.", metavar = "CATEGORY",
            callback = getopt.help_cb(io.stdout)
        },
        { "v", "verbose", false, help = "Be verbose." },

        { category = "Generator" },

        { "I", "include", true, help = "Include a directory.", metavar = "DIR",
            list = {}
        },
        { "L", "library", true, help = "Specify a C library name." },
        { "M", "module",  true, help = "Specify a module name."    },
        { "o", "output", true, help = "Specify output file name(s), by "
            .. "default goes to stdout.",
            list = {}
        }
    },
    error_cb = function(parser, msg)
        io.stderr:write(msg, "\n")
        getopt.help(parser, io.stderr)
    end,
    done_cb = function(parser, opts, args)
        if not opts["h"] then
            if not opts["L"] then
                io.stderr:write("library name not specified\n")
                getopt.help(parser, io.stderr)
            else
                for i, v in ipairs(opts["I"] or {}) do
                    lualian.include_dir(v)
                end
                for i, fname in ipairs(args) do
                    gen_file(opts, i, fname)
                end
            end
        end
    end
}

return true
