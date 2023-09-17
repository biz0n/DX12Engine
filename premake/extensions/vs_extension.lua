-- wait for PR: https://github.com/premake/premake-core/pull/1952

require('vstudio')

premake.api.register {
    name = "usestandardpreprocessor",
        scope = "config",
        kind = "string",
        allowed = {
            "On",
            "Off"
        }
}

premake.api.register {
        name = "copyDistenation",
        scope = "config",
        kind = "string",
}

function copyToOutputDirectory(filecfg, condition)
    --premake.vstudio.vc2010.element("DeploymentContent", condition, "true")
    --premake.vstudio.vc2010.element("CopyToOutputDirectory", condition, "Always")
    if not filecfg or filecfg.copyDistenation then
        premake.vstudio.vc2010.element("DestinationFolders", condition, filecfg.copyDistenation)
    end
end

function useStandardPreprocessor(cfg)
    if _ACTION >= "vs2019" and cfg.usestandardpreprocessor ~= nil then
        if cfg.usestandardpreprocessor == 'On' then
            premake.vstudio.vc2010.element("UseStandardPreprocessor", nil, "true")
        else
            premake.vstudio.vc2010.element("UseStandardPreprocessor", nil, "false")
        end
    end
end

premake.override(premake.vstudio.vc2010.elements, "clCompile", function(base, prj)
    local calls = base(prj)
    table.insertafter(calls, premake.vstudio.vc2010.scanSourceForModuleDependencies, useStandardPreprocessor)
    return calls
end)


---
-- None group
---
premake.vstudio.vc2010.categories.CopyToOutputDirectory = {
        name = "CopyToOutputDirectory",
        priority = 5,

        emitFiles = function(prj, group)
            local fileCfgFunc = {
                copyToOutputDirectory,
            }

            premake.vstudio.vc2010.emitFiles(prj, group, "CopyFileToFolders", nil, fileCfgFunc)
        end,

        emitFilter = function(prj, group)
            premake.vstudio.vc2010.filterGroup(prj, group, "CopyFileToFolders")
        end
    }