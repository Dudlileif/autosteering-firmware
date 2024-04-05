# Copyright (C) 2024 Gaute Hagen
#
# This file is part of Autosteering Firmware.
#
# Autosteering Firmware is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Autosteering Firmware is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Autosteering Firmware.  If not, see <https://www.gnu.org/licenses/>.

import os
import shutil

Import("env")

def copy_built_file_to_build_directory(source, target, env) -> None:
    print("Program has been built!")

    dest_dir: str = os.path.join(env["PROJECT_DIR"], "../../builds")
    os.makedirs(dest_dir, exist_ok=True)

    firmware_type: str = (
        env.GetProjectConfig()
        .get("common", "firmware_type")
        .replace("'", "")
        .replace('"', "")
    )

    version: str = (
        env.GetProjectConfig()
        .get("common", "version_string")
        .replace("'", "")
        .replace('"', "")
    )

    build_timestamp: str = (
        env.GetProjectConfig()
        .get("common", "build_timestamp")
        .replace("'", "")
        .replace('"', "")
    )

    for file in os.listdir(os.path.join(env["PROJECT_DIR"], "../../builds/")):
        if file.startswith(firmware_type):
            path: str = os.path.join(env["PROJECT_DIR"], "../../builds", file)
            print("Removing old build: ", path)
            os.remove(path)

    print("Type:", firmware_type)
    print("Version:", version)
    print("Build timestamp:", build_timestamp)

    dest_path: str = os.path.join(
        env["PROJECT_DIR"],
        str.join("", ["../../builds/", firmware_type, "_", version, ".bin"]),
    )
    program_path: str = target[0].get_abspath()
    print("Program path:", program_path)
    print("Destination path:", dest_path)
    shutil.copy(program_path, dest_path)

    partitions_path: str = program_path.replace("firmware.bin", "partitions.bin")
    dest_path_partitions: str = os.path.join(
        env["PROJECT_DIR"],
        str.join(
            "",
            ["../../builds/", firmware_type, "_partitions_", version, ".bin"],
        ),
    )

    print("Partitions path:", partitions_path)
    print("Partitions destination path:", dest_path_partitions)
    shutil.copy(partitions_path, dest_path_partitions)

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", copy_built_file_to_build_directory)
