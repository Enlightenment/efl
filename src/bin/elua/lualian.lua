-- Lualian application
-- for use with Elua

local lualian = require("lualian")
local  getopt = require("getopt")

local opts, args, arg_parser = getopt.parse {
    usage = "Usage: %prog [OPTIONS] file1.eo file2.eo ... fileN.eo",
    args  = arg, descs = {
        { "h", "help"   , false, help = "Show this message." },
        { "v", "verbose", false, help = "Be verbose." },
        { "I", "include",  true, help = "Include a directory.",
            metavar = "DIR"
        },
        { "L", "library",  true, help = "Specify a C library name." },
        { "M", "module" ,  true, help = "Specify a module name." },
        { "P", "prefix" ,  true, help = "Specify a class name prefix "
            .. "to strip out for public interfaces."
        },
        { "o", "output" ,  true, help = "Specify output file name(s), by "
            .. "default goes to stdout."
        }
    }
}

if not opts then
    io.stderr:write(args, "\n")
    getopt.help(arg_parser, io.stderr)
    return
end

local include_dirs = {}
local output_files = {}
local libname, modname, cprefix = nil, nil, ""

local printv = function() end

for i, opt in ipairs(opts) do
    local on = opt[1]
    if on == "h" then
        getopt.help(arg_parser, io.stdout)
        return
    end
    if on == "v" then
        printv = print
    elseif on == "I" then
        include_dirs[#include_dirs + 1] = opt[2]
    elseif on == "L" then
        libname = opt[2]
    elseif on == "M" then
        modname = opt[2]
    elseif on == "P" then
        cprefix = opt[2]
    elseif on == "o" then
        output_files[#output_files + 1] = opt[2]
    end
end

if not libname then
    io.stderr:write("library name not specified\n")
    getopt.help(arg_parser, io.stderr)
end

for i, v in ipairs(include_dirs) do
    lualian.include_dir(v)
end

for i, fname in ipairs(args) do
    printv("Generating for file: " .. fname)
    local fstream = io.stdout
    if output_files[i] then
        printv("  Output file: " .. output_files[i])
        fstream = io.open(output_files[i], "w")
        if not fstream then
            error("Cannot open output file: " .. output_files)
        end
    else
        printv("  Output file: printing to stdout...")
    end
    lualian.generate(fname, modname, libname, cprefix, fstream)
end