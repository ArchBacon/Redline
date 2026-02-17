# This script takes a list of relative file paths, recreates their folder structure in a new folder
# named 'engine_and_tools_start', copies the files, and zips the folder. The folder is deleted after zipping.
import os
import shutil
import zipfile

# List of relative file paths to include
file_list = [
    ".clang-format",
    ".clang-tidy",
    ".gitattributes",
    ".gitignore",
    "bee.sln",
    "format.bat",
    "license.txt",
    "include/core/audio.hpp",
    "include/core/device.hpp",
    "include/core/engine.hpp",
    "include/core/fileio.hpp",
    "include/core/input.hpp",
    "include/math/math.hpp",
    "include/platform/opengl/device_gl.hpp",
    "include/platform/opengl/open_gl.hpp",
    "include/tools/input_mapping.hpp",
    "include/tools/log.hpp",
    "include/tools/tools.hpp",
    "include/tools/warnings.hpp",
    "source/core/audio.cpp",
    "source/core/engine.cpp",
    "source/core/fileio.cpp",
    "source/platform/opengl/device_gl.cpp",
    "source/platform/opengl/open_gl.cpp",
    "source/platform/pc/core/fileio_pc.cpp",
    "source/platform/pc/core/input_pc.cpp",
    "source/tools/log.cpp",
    "source/tools/tools.cpp",
    "assets",
    "external",
    "properties",
]

DEST_ROOT = 'engine_and_tools_start'
ZIP_NAME = 'engine_and_tools_start.zip'


def recreate_structure_and_copy(file_list, dest_root):
	if os.path.exists(dest_root):
		shutil.rmtree(dest_root)
	for rel_path in file_list:
		src_path = os.path.normpath(rel_path)
		abs_src_path = os.path.abspath(src_path)
		dest_path = os.path.join(dest_root, src_path)
		if os.path.isdir(abs_src_path):
			# Copy entire directory tree
			for root, dirs, files in os.walk(abs_src_path):
				rel_root = os.path.relpath(root, abs_src_path)
				dest_dir = os.path.join(dest_path, rel_root) if rel_root != '.' else dest_path
				os.makedirs(dest_dir, exist_ok=True)
				for file in files:
					src_file = os.path.join(root, file)
					dest_file = os.path.join(dest_dir, file)
					shutil.copy2(src_file, dest_file)
		elif os.path.isfile(abs_src_path):
			os.makedirs(os.path.dirname(dest_path), exist_ok=True)
			shutil.copy2(abs_src_path, dest_path)
		else:
			print(f'Warning: {rel_path} does not exist.')


def zip_folder(folder_path, zip_name):
    with zipfile.ZipFile(zip_name, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(folder_path):
            for file in files:
                abs_file = os.path.join(root, file)
                rel_file = os.path.relpath(abs_file, folder_path)
                zipf.write(abs_file, rel_file)

# Hack: Copy the 'xsr' folder (two levels up) into the 'external' folder inside DEST_ROOT
def add_xsr_to_external(dest_root):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    xsr_src = os.path.abspath(os.path.join(script_dir, '..', '..', '..', 'xsr'))
    external_dest = os.path.join(dest_root, 'external', 'xsr')
    if os.path.isdir(xsr_src):
        if os.path.exists(external_dest):
            shutil.rmtree(external_dest)
        shutil.copytree(xsr_src, external_dest)
        print(f"Added 'xsr' folder to {external_dest}")
    else:
        print(f"Warning: 'xsr' folder not found at {xsr_src}")

if __name__ == '__main__':
    if not file_list:
        print('Please populate the file_list variable with relative file paths to include.')
    else:
        recreate_structure_and_copy(file_list, DEST_ROOT)
        add_xsr_to_external(DEST_ROOT)
        zip_folder(DEST_ROOT, ZIP_NAME)
        shutil.rmtree(DEST_ROOT)
        print(f'Created {ZIP_NAME} containing the selected files and removed the temporary folder.')
