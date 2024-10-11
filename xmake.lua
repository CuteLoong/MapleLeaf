set_project("MapleLeaf")
set_version("1.0.0", {build = "%Y%m%d%H%M"})
set_languages("c++17")
set_arch("x64")

add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode/"})

if is_plat("windows") then 
    set_toolchains("msvc")
elseif is_plat("linux") then
    set_toolchains("clang")
end

rootPath = os.projectdir():gsub("\\", "\\\\")

add_includedirs("Config/")
set_configvar("VERSION_MAJOR", 1)
set_configvar("VERSION_MINOR", 1)
set_configvar("VERSION_ALTER", 0)
set_configvar("PROJECT_DIR", rootPath)
set_configvar("MAPLELEAF_SCENE_DEBUG", false)
set_configvar("MAPLELEAF_SHADER_DEBUG", true)
set_configvar("MAPLELEAF_DEVICE_DEBUG", false)
set_configvar("MAPLELEAF_GRAPHIC_DEBUG", false)
set_configvar("MAPLELEAF_GPUSCENE_DEBUG", false)
set_configvar("MAPLELEAF_PIPELINE_DEBUG", false)
set_configvar("MAPLELEAF_VALIDATION_DEBUG", true)
set_configvar("MAPLELEAF_DESCRIPTOR_DEBUG", false)
set_configvar("MAPLELEAF_RENDERSTAGE_DEBUG", false)
set_configvar("MAPLELEAF_RAY_TRACING", false)

set_configdir("Config") 
add_configfiles("./config.h.in")

add_requires("volk 1.3.268", "glm", "glfw", "spirv-tools 1.3.268", "glslang", "assimp", "stb", "boost")
add_requires("imgui", {configs = {glfw_vulkan = true}})
add_packages("glm", "glfw", "volk", "spirv-tools", "glslang", "assimp", "stb", "boost", "imgui")
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
"Core/Graphics/AccelerationStruct",
"Core/Files", 
"Core/Models", 
"Core/Materials", 
"Core/Resources", 
"Core/Meshes", 
"Core/Scenes", 
"Core/Scenes/Light", 
"Core/Scenes/Shadow", 
"Core/Scenes/Skybox", 
"Core/Scenes/Animation",
"Core/ASScene", 
"Core/Importers", 
"Core/Importers/Builder",
"Core/Bitmaps",
"Core/Inputs",
"Core/GPUScene",
"Core/Imgui",
"Core/Utils/SampleGenerators"
)

add_includedirs(
"RenderPass",
"RenderPass/Deferred",
"RenderPass/Shadow",
"RenderPass/IndirectDraw",
"RenderPass/Imgui",
"RenderPass/AO",
"RenderPass/HiZDraw",
"RenderPass/Mesh",
"RenderPass/StereoMask",
"RenderPass/Blur",
"RenderPass/SSR",
"RenderPass/RawSSR",
"RenderPass/SCSSR",
"RenderPass/Skybox",
"RenderPass/RayTracing"
)

target("RenderPass")
    set_kind("static")
    add_files(
    "RenderPass/Deferred/*.cpp",
    "RenderPass/Shadow/*.cpp",
    "RenderPass/IndirectDraw/*.cpp",
    "RenderPass/Imgui/*.cpp",
    "RenderPass/AO/*.cpp",
    "RenderPass/HiZDraw/*.cpp",
    "RenderPass/Mesh/*.cpp",
    "RenderPass/StereoMask/*.cpp",
    "RenderPass/Blur/*.cpp",
    "RenderPass/SSR/*.cpp",
    "RenderPass/RawSSR/*.cpp",
    "RenderPass/SCSSR/*.cpp",
    "RenderPass/Skybox/*.cpp",
    "RenderPass/RayTracing/*.cpp",
    "RenderPass/InterpolationStereo/*.cpp",
    "RenderPass/InterpolationMono/*.cpp"
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
    "Core/Graphics/AccelerationStruct/*.cpp", 
    "Core/Files/*.cpp", 
    "Core/Models/*.cpp", 
    "Core/Materials/*.cpp", 
    "Core/Resources/*.cpp", 
    "Core/Meshes/*.cpp", 
    "Core/Scenes/*.cpp", 
    "Core/Scenes/Light/*.cpp", 
    "Core/Scenes/Shadow/*.cpp", 
    "Core/Scenes/Skybox/*.cpp", 
    "Core/Scenes/Animation/*.cpp",
    "Core/ASScene/*.cpp", 
    "Core/Importers/*.cpp", 
    "Core/Importers/Builder/*.cpp",
    "Core/Bitmaps/*.cpp",
    "Core/Inputs/*.cpp",
    "Core/GPUScene/*.cpp",
    "Core/Imgui/*.cpp",
    "Core/Utils/SampleGenerators/*.cpp"
    )
target_end()

target("MapleLeaf")
    set_kind("binary")
    add_deps("Core")
    add_deps("RenderPass")
    add_includedirs("App/", "Renderer/GPURenderer", "Renderer/MultiDrawRenderer", "Renderer/StereoRenderer", "Renderer/RawSSRRenderer", "Renderer/SCSSRRenderer", "Renderer/RayTracingRenderer", "Renderer/StereoRayTracingRenderer", "Renderer/WarpRenderer")
    add_files("App/*.cpp", "Renderer/GPURenderer/*.cpp", "Renderer/MultiDrawRenderer/*.cpp", "Renderer/StereoRenderer/*.cpp", "Renderer/RawSSRRenderer/*.cpp", "Renderer/SCSSRRenderer/*.cpp","Renderer/RayTracingRenderer/*.cpp", "Renderer/StereoRayTracingRenderer/*.cpp", "Renderer/WarpRenderer/*.cpp")
target_end()