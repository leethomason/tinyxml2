# Python program to set the version.
##############################################
import os.path


def main():
	version = {}
	version["major"] = int(raw_input("Major: "))
	version["minor"] = int(raw_input("Minor: "))
	version["patch"] = int(raw_input("Patch: "))

	
	cmake_contents = [
		"set(VERSION_MAJOR {major})\n".format(**version),
		"set(VERSION_MINOR {minor})\n".format(**version),
		"set(VERSION_PATCH {patch})\n".format(**version)
	]
	
	include_contents = [
		"static const int TIXML2_MAJOR_VERSION = {major};\n".format(**version),
		"static const int TIXML2_MINOR_VERSION = {minor};\n".format(**version),
		"static const int TIXML2_PATCH_VERSION = {patch};\n".format(**version)
	]
	
	dox_contents = [
		"PROJECT_NUMBER = {major}.{minor}.{patch}\n".format(**version)
	]
	
	
	config_dir = "config"
	try:
		os.mkdir(config_dir)
	except OSError:
		pass  # directory already exists
	
	print "=> Writting config files with version [{major}.{minor}.{patch}]".format(**version)
	
	config_files = []
	config_files.append({"filepath" : os.path.join(config_dir, "version.cmake"), "contents" : cmake_contents})
	config_files.append({"filepath" : os.path.join(config_dir, "version.h"), "contents" : include_contents})
	config_files.append({"filepath" : os.path.join(config_dir, "version.dox"), "contents" : dox_contents})
	
	for c in config_files:
		print "=> Writting '{filepath}'".format(**c)
		write_config_file(c)
	
	print "=> Done"

def write_config_file(config):
	try:
		f = open(config["filepath"], 'w')
		f.writelines(config["contents"])
	except IOError:
		print "=! Could not write '{0}'".format(os.path.basename(config["filepath"]))


if __name__ == "__main__":
	main()
