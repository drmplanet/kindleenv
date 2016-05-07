local lfs = require("libs/libkoreader-lfs")
local DataStorage = require("datastorage")
local dump = require("dump")
local purgeDir = require("ffi/util").purgeDir

local DocSettings = {}

local history_dir = DataStorage:getHistoryDir()

function DocSettings:getSidecarDir(doc_path)
    return doc_path:match("(.*)%.")..".sdr"
end

function DocSettings:getHistoryPath(fullpath)
    return history_dir .. "/[" .. fullpath:gsub("(.*/)([^/]+)","%1] %2"):gsub("/","#") .. ".lua"
end

function DocSettings:getPathFromHistory(hist_name)
    -- 1. select everything included in brackets
    local s = string.match(hist_name,"%b[]")
    -- 2. crop the bracket-sign from both sides
    -- 3. and finally replace decorative signs '#' to dir-char '/'
    return string.gsub(string.sub(s,2,-3),"#","/")
end

function DocSettings:getNameFromHistory(hist_name)
    -- at first, search for path length
    local s = string.len(string.match(hist_name,"%b[]"))
    -- and return the rest of string without 4 last characters (".lua")
    return string.sub(hist_name, s+2, -5)
end

function DocSettings:purgeDocSettings(doc_path)
    purgeDir(self:getSidecarDir(doc_path))
    os.remove(self:getHistoryPath(doc_path))
end

function DocSettings:open(docfile)
    -- TODO(zijiehe): Remove history_path, use only sidecar.
    local new = { data = {} }
    local ok, stored
    if docfile == ".reader" then
        -- we handle reader setting as special case
        new.history_file = DataStorage:getDataDir() .. "/settings.reader.lua"

        ok, stored = pcall(dofile, new.history_file)
    else
        new.history_file = self:getHistoryPath(docfile)

        local sidecar = self:getSidecarDir(docfile)
        if lfs.attributes(sidecar, "mode") ~= "directory" then
            lfs.mkdir(sidecar)
        end
        new.sidecar_file = sidecar.."/"..docfile:match(".*%/(.*)")..".lua"

        ok, stored = pcall(dofile, new.sidecar_file or "")
        if not ok then
            ok, stored = pcall(dofile, new.history_file or "")
            if not ok then
                -- try legacy conf path, for backward compatibility. this also
                -- takes care of reader legacy setting
                ok, stored = pcall(dofile, docfile..".kpdfview.lua")
            end
        end
    end
    if ok and stored then
        new.data = stored
    end

    return setmetatable(new, { __index = DocSettings})
end

function DocSettings:readSetting(key)
    return self.data[key]
end

function DocSettings:saveSetting(key, value)
    self.data[key] = value
end

function DocSettings:delSetting(key)
    self.data[key] = nil
end

function DocSettings:flush()
    -- write serialized version of the data table into
    --  i) history directory in root directory of KOReader
    -- ii) sidecar directory in the same directory of the document
    if not self.history_file and not self.sidecar_file then
        return
    end

    local serials = {}
    if self.history_file then
        pcall(table.insert, serials, io.open(self.history_file, "w"))
    end
    if self.sidecar_file then
        pcall(table.insert, serials, io.open(self.sidecar_file, "w"))
    end
    os.setlocale('C', 'numeric')
    local s_out = dump(self.data)
    for _, f_out in ipairs(serials) do
        if f_out ~= nil then
            f_out:write("-- we can read Lua syntax here!\nreturn ")
            f_out:write(s_out)
            f_out:write("\n")
            f_out:close()
        end
    end
end

function DocSettings:close()
    self:flush()
end

function DocSettings:clear()
    if self.history_file then
        os.remove(self.history_file)
    end
    if self.sidecar_file then
        os.remove(self.sidecar_file)
    end
end

return DocSettings
