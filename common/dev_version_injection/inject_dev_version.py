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

import datetime
import os

Import("env")

VERSION_FILE = "dev_version"
dev_version = 0

dev_build: bool = env.GetProjectConfig().getboolean("common", "dev_build")
if dev_build:
    if os.path.exists(VERSION_FILE):
        try:
            with open(VERSION_FILE, "r", encoding="utf-8") as version_file:
                dev_version = int(version_file.readline())
        except (OSError, ValueError):
            print("No dev version file found")

    version = "{}{}".format(
        env.GetProjectConfig().get("common", "version_string"),
        "_dev.{}".format(dev_version) if dev_build else "",
    )
    env.GetProjectConfig().set("common", "version_string", version)
    print("Dev version:", version)

env.GetProjectConfig().set("common", "build_timestamp", str(datetime.datetime.now()))

check_sum_path: str = os.path.join(env["PROJECT_DIR"], ".pio/build/project.checksum")
if os.path.exists(check_sum_path):
    os.remove(check_sum_path)
