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

Import("env")

def increase_dev_version(source, target, env) -> None:
    version_file_path: str = "dev_version"
    dev_version = 1
    if os.path.exists(version_file_path):
        with open(version_file_path, "r", encoding="utf-8") as version_file:
            dev_version = int(version_file.readline()) + 1

    with open(version_file_path, "w", encoding="utf-8") as version_file:
        version_file.write(str(dev_version))

if env.GetProjectConfig().getboolean("common", "dev_build"):
    env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", increase_dev_version)
