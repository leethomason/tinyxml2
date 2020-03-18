--
-- Requires: Premake 5 (https://premake.github.io/)
-- Usage: premake5 --file=premake5.lua [project / makefile format, refer to premake5 --help] --target=[target from below]
--

-- target option
tbl_target_values =
{
	{ "windows", "VS2015 projects targeting Windows 32/64 bits" },
	{ "macosx", "Xcode4 projects targeting OS X" },
}

newoption
{
	trigger = "target",
	description = "Build environment and target to generate projects for.",
	allowed = tbl_target_values
}

-- validation
target_env = _OPTIONS["target"]
if not target_env then
	print "Command-line option --target is required with one of the following values:"
	for _, v in ipairs(tbl_target_values) do
		print(v[1])
	end
	os.exit(1)
end

-- solution
workspace "tinyxml2"

	tbl_platforms = {}
    if target_env == "windows" then
        tbl_platforms = {
            "x86",
            "x64",
        }
    elseif target_env == "macosx" then
        tbl_platforms = {
            "Universal64"
        }
    end
	platforms(tbl_platforms)

	tbl_configurations = {
		"Debug",
		"Release",
	}
	configurations(tbl_configurations)

	sln_location = ".projects/"..target_env
	location(sln_location)

	bin_location = ".artifacts/"..target_env
	obj_location = ".intermediate/"..target_env

	for _, p in ipairs(tbl_platforms) do
		for _, c in ipairs(tbl_configurations) do
			local pc = p.."-"..c
			filter{ "platforms:"..p, c }
				targetdir(bin_location.."/"..pc)
				libdirs(bin_location.."/"..pc)
				objdir(obj_location.."/"..pc)
		end
	end

	filter("not Release")
		optimize "Debug"
		symbols "On"
	filter{ "Release" }
		optimize "Full"
	filter{}

	-- projects
	project "tinyxml2"

		kind "staticlib"

		files {
			"tinyxml2.h",
			"tinyxml2.cpp"
		}

	project "xmltest"

		kind "consoleapp"

		links {
			"tinyxml2"
		}

		files {
		    "xmltest.cpp"
        }
