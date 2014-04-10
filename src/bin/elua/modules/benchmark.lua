-- Elua benchmark module
-- Dependencies: eina.counter, eina.log, util, cutil

local cutil   = require("cutil")
local util    = require("util")
local counter = require("eina.counter")
local log     = require("eina.log")

local Counter = counter.Counter

local M = {}

local dom

cutil.init_module(function()
    dom = log.Domain("elua_benchmark")
    if not dom:is_valid() then
        log.err("Could not register log domain: elua_benchmark")
        return false
    end
    return true
end, function()
    dom:unregister()
    dom = nil
    return true
end)

local PLOT_MASK = "bench_%s_%s.gnuplot"
local DATA_MASK = "bench_%s_%s.%s.data"

M.Benchmark = util.Object:clone {
    __ctor = function(self, name, run)
        self.name, self.run = name, run
        self.runs = {}
    end,

    register = function(self, name, bench_cb, c_start, c_end, c_step)
        self.runs[#self.runs + 1] = {
            cb = bench_cb, name = name, c_start = c_start, c_end = c_end,
            c_step = c_step
        }
        return true
    end,

    run = function(self, pmask, dmask)
        pmask, dmask  = pmask or PLOT_MASK, dmask or DATA_MASK
        local  fname  = pmask:format(self.name, self.run)
        local  plots  = io.open(fname, "w")
        if not plots then
            return nil
        end
        local fnames = {}
        fnames[#fnames + 1] = fname
        plots:write(([[
set   autoscale                        # scale axes automatically
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set terminal png size 1024,768
set output "output_%s_%s.png"
set title  "%s %s"
set xlabel "tests"
set ylabel "time"
plot ]]):format(self.name, self.run, self.name, self.run))
        local first = false
        for i, run in ipairs(self.runs) do
            local fname = dmask:format(self.name, self.run, run.name)
            local datas = io.open(fname, "w")
            if datas then
                fnames[#fnames + 1] = fname
                local cnt = Counter(run.name)
                for i = run.c_start, run.c_end, run.c_step do
                    io.stderr:write("Run ", run.name, ": ", i, "\n")
                    cnt:start()
                    run.cb(i)
                    cnt:stop(i)
                end
                local ret = cnt:dump()
                if ret then
                    datas:write(ret)
                end
                cnt:free()
                datas:close()
                if not first then
                    first = true
                else
                    plots:write(", \\\n")
                end
                plots:write(("\"%s\" using 1:2 title \'%s\' with line")
                    :format(fname, run.name))
            end
        end
        plots:write("\n")
        plots:close()
        return fnames
    end
}

return M