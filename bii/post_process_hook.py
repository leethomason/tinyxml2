import os
import shutil

try:
	if not os.path.exists(os.path.join(bii.project_folder, "bin")):
		os.mkdir(os.path.join(bii.project_folder, "bin"))

	if os.path.exists(os.path.join(bii.project_folder, "bin", "resources")):
		shutil.rmtree(os.path.join(bii.project_folder, "bin", "resources"))

	shutil.copytree(os.path.join(bii.block_folder, "resources"), os.path.join(bii.project_folder, "bin", "resources"))
except Exception as e:
	print e.message
	print "Copy the resources folder into bon folder manually"
