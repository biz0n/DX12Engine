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

