set_project("MapleLeaf")
set_version("1.0.0", {build = "%Y%m%d%H%M"})
set_languages("c++17")
set_arch("x64")

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode/"})

if is_plat("windows") then 
    set_toolchains("msvc")
elseif is_plat("linux") then
    set_toolchains("clang")
end

add_requires("volk", "glm", "glfw", "spirv-tools", "glslang", "assimp", "stb", "boost")
add_packages("glm", "glfw", "volk", "spirv-tools", "glslang", "assimp", "stb", "boost")
add_includedirs(
"Core/Devices", 
"Core/Utils", 
"Core/Maths", 
"Core/Engine",
"Core/Graphics", 
"Core/Graphics/Instance", 
"Core/Graphics/Devices", 
"Core/Graphics/Swapchain", 
"Core/Graphics/Resources", 
"Core/Graphics/Renderer", 
"Core/Graphics/Commands", 
"Core/Graphics/Descriptors/",
"Core/Graphics/Pipelines", 
"Core/Files", 
"Core/Models", 
"Core/Materials", 
"Core/Resources", 
"Core/Meshes", 
"Core/Scenes", 
"Core/Importers", 
"Core/Importers/Builder",
"Core/Bitmaps",
"Core/Inputs"
)

add_includedirs(
"RenderPass/Deferred"
)

target("Deferred")
    set_kind("static")
    add_files(
    "RenderPass/Deferred/*.cpp"
    )
target_end()

target("Core")
    set_kind("static")
    add_files(
    "Core/Devices/*.cpp", 
    "Core/Utils/*.cpp", 
    "Core/Maths/*.cpp", 
    "Core/Engine/*.cpp", 
    "Core/Graphics/*.cpp", 
    "Core/Graphics/Instance/*.cpp", 
    "Core/Graphics/Devices/*.cpp", 
    "Core/Graphics/Swapchain/*.cpp", 
    "Core/Graphics/Resources/*.cpp", 
    "Core/Graphics/Renderer/*.cpp", 
    "Core/Graphics/Commands/*.cpp", 
    "Core/Graphics/Descriptors/*.cpp", 
    "Core/Graphics/Pipelines/*.cpp", 
    "Core/Files/*.cpp", 
    "Core/Models/*.cpp", 
    "Core/Materials/*.cpp", 
    "Core/Meshes/*.cpp", 
    "Core/Scenes/*.cpp", 
    "Core/Importers/*.cpp", 
    "Core/Importers/Builder/*.cpp",
    "Core/Bitmaps/*.cpp",
    "Core/Inputs/*.cpp"
    )
target_end()

target("MapleLeaf")
    set_kind("binary")
    add_deps("Core")
    add_deps("Deferred")
    add_includedirs("Test/")
    add_files("Test/*.cpp")
    -- add_files("./main.cpp") 
target_end()