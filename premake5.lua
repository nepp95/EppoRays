workspace "EppoApp"
    architecture "x86_64"
    startproject "EppoApp"

    configurations {
        "Debug",
        "Release"
    }

    flags {
        "MultiProcessorCompile"
    }

    OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    group "Dependencies"
        include "EppoCore/EppoCore"
    
    group "App"
        include "EppoApp"
    group ""