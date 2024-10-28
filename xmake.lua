set_xmakever("2.9.0")
set_project("frozen")

set_version("1.1.0", { build = "%Y%m%d%H%M" })

option("modules", { defines = "TINYXML2_USE_MODULE", default = false })

add_rules("mode.release", "mode.debug")
set_symbols("hidden")

target("tinyxml")
    set_kind("$(kind)")
    if get_config("modules") then
        set_languages("c++20")
        add_files("tinyxml2.cppm", {public = true})
    end
    add_headerfiles("tinyxml2.h")
    add_files("tinyxml2.cpp")
    if is_mode("debug") then
        add_defines("TINYXML2_DEBUG", {public = true})
    end
    add_defines("TINYXML2_EXPORT", "_FILE_OFFSET_BITS=64")
    add_cxxflags("cl::/D_CRT_SRCURE_NO_WARNINGS")
    add_includedirs(".", {public = true})
    add_options("modules")

target("xmltest")
   set_kind("binary")
   if get_config("modules") then
      set_languages("c++20")
   end
   set_default(false)
   add_deps("tinyxml")
   add_files("xmltest.cpp")
   add_options("modules")
   add_tests("tests", {rundir = os.projectdir()})
     
