set_project("MapleLeaf")
set_version("1.0.0", {build = "%Y%m%d%H%M"})
set_languages("c++17")
set_toolchains("clang")
set_arch("x64")

add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode/"})

add_requires("volk", "glm", "glfw")
add_packages("glm", "glfw", "volk")
add_includedirs("Core/Devices", "Core/Utils", "Core/Maths", "Core/Engine", "Core/Graphics", "Core/Graphics/Instance", "Core/Graphics/Devices", "Core/Graphics/Swapchain", "Core/Graphics/Resources", "Core/Graphics/Renderer", "Core/Graphics/Commands", "Core/Graphics/Pipelines")

target("Core")
    set_kind("static")
    add_files("Core/Devices/*.cpp", "Core/Utils/*.cpp", "Core/Maths/*.cpp", "Core/Engine/*.cpp", "Core/Graphics/*.cpp", "Core/Graphics/Instance/*.cpp", "Core/Graphics/Devices/*.cpp", "Core/Graphics/Swapchain/*.cpp", "Core/Graphics/Resources/*.cpp", "Core/Graphics/Renderer/*.cpp", "Core/Graphics/Commands/*.cpp", "Core/Graphics/Pipelines/*.cpp")
target_end()

target("MapleLeaf")
    set_kind("binary")
    add_deps("Core")
    add_includedirs("Test/")
    add_files("Test/*.cpp")
    -- add_files("./main.cpp") 
target_end()