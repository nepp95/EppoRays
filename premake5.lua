workspace "EppoRays"
    architecture "x86_64"
    startproject "EppoRays"

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
        include "EppoRays"
    group ""