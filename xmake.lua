set_project("MapleLeaf")
set_version("1.1", {build = "%Y%m%d%H%M"})
set_languages("c++17")
set_toolchains("clang")
set_arch("x64")

add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode/"})

add_requires("volk", "glm", "glfw")
add_packages("glm", "glfw", "volk")
add_includedirs("Core/Devices", "Core/Utils", "Core/Maths", "Core/App")

target("Core")
    set_kind("static")
    add_files("Core/Devices/*.cpp", "Core/Maths/*.cpp", "Core/App/*.cpp")
target_end()

target("MapleLeaf")
    set_kind("binary")
    add_deps("Core")
    add_files("./main.cpp") 
target_end()