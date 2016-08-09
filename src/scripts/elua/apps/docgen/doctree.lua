local util = require("util")

local dutil = require("docgen.util")
local writer = require("docgen.writer")

local M = {}

local Node = util.Object:clone {
}

local gen_doc_refd = function(str)
    if not str then
        return nil
    end
    local pars = dutil.str_split(str, "\n\n")
    for i = 1, #pars do
        pars[i] = writer.Buffer():write_par(pars[i]):finish()
    end
    return table.concat(pars, "\n\n")
end

M.Doc = Node:clone {
    __ctor = function(self, doc)
        self.doc = doc
    end,

    summary_get = function(self)
        if not self.doc then
            return nil
        end
        return self.doc:summary_get()
    end,

    description_get = function(self)
        if not self.doc then
            return nil
        end
        return self.doc:description_get()
    end,

    since_get = function(self)
        if not self.doc then
            return nil
        end
        return self.doc:since_get()
    end,

    brief_get = function(self, doc2)
        if not self.doc and (not doc2 or not doc2.doc) then
            return "No description supplied."
        end
        local doc1, doc2 = self.doc, doc2 and doc2.doc or nil
        if not doc1 then
            doc1, doc2 = doc2, doc1
        end
        return gen_doc_refd(doc1:summary_get())
    end,

    full_get = function(self, doc2)
        if not self.doc and (not doc2 or not doc2.doc) then
            return "No description supplied."
        end
        local doc1, doc2 = self.doc, doc2 and doc2.doc or nil
        if not doc1 then
            doc1, doc2 = doc2, doc1
        end
        local sum1 = doc1:summary_get()
        local desc1 = doc1:description_get()
        local edoc = ""
        if doc2 then
            local sum2 = doc2:summary_get()
            local desc2 = doc2:description_get()
            if not desc2 then
                if sum2 then edoc = "\n\n" .. sum2 end
            else
                edoc = "\n\n" .. sum2 .. "\n\n" .. desc2
            end
        end
        if not desc1 then
            return gen_doc_refd(sum1 .. edoc)
        end
        return gen_doc_refd(sum1 .. "\n\n" .. desc1 .. edoc)
    end
}

return M
