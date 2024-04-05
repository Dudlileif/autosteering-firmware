// Copyright (C) 2024 Gaute Hagen
//
// This file is part of Autosteering Firmware.
//
// Autosteering Firmware is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Autosteering Firmware is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Autosteering Firmware. If not, see https://www.gnu.org/licenses/.

const char firmware_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
    <title>%TITLE%</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style>
        .content {
            padding-left: 1rem;
            padding-right: 1rem;
            padding-top: 1rem;
            padding-right: 1rem;
            margin: auto;
            max-width: 750px;
        }

        .content h2 {
            text-align: center;
        }

        .content h3 {
            text-align: center;
        }
    </style>
</head>

<body>
    <div class="content">
        <h2>%TITLE%</h2>
        <form action="/">
            <button type="submit">
                <h3>Menu</h3>
            </button>
        </form>

        <h3>Update firmware</h3>
        <p>
        <h4>Installed versions</h4>
        ESP: %ESP_FIRMWARE_VERSION%
        %TEENSY_VERSION_SEGMENT%
        </p>
        <p>Free Storage: <span id="free_storage">-</span> | Used Storage: <span id="used_storage">-</span> | Total
            Storage: <span id="total_storage">-</span>
        </p>
        <p id="upload_button">
            <button onclick="showUploadButton()">Upload File</button>
        </p>
        <p id="status"></p>
        <p id="upload_details_header" hidden></p>
        <p id="upload_details" hidden></p>
        <p id="file_list_details_header"></p>
        <p id="file_list_details"></p>
        <script>
            listFiles();

            function updateStorageSizes() {
                var xhr = new XMLHttpRequest();
                xhr.open("GET", "/storage_sizes", true);
                xhr.responseType = "json";
                xhr.onreadystatechange = function () {
                    if (xhr.readyState == 4 && xhr.status == 200) {
                        let data = xhr.response;
                        document.getElementById("free_storage").innerText = humanReadableSize(data["free"]);
                        document.getElementById("used_storage").innerText = humanReadableSize(data["used"]);
                        document.getElementById("total_storage").innerText = humanReadableSize(data["total"]);
                    }
                };
                xhr.send();
            }

            function listFiles() {
                var xhr = new XMLHttpRequest();
                xhr.open("GET", "/list_files", true);
                xhr.onreadystatechange = function () {
                    if (xhr.readyState == 4 && xhr.status == 200) {
                        document.getElementById("file_list_details_header").innerHTML = "<h4>Files<h4>";
                        document.getElementById("file_list_details").innerHTML = xhr.responseText;
                        updateStorageSizes();
                    }
                };
                xhr.send();
            }

            function installDownloadDeleteButton(filename, action) {
                var urltocall = "/file?name=" + filename + "&action=" + action;
                var xhr = new XMLHttpRequest();
                if (action == "install") {
                    xhr.open("GET", urltocall, true);
                    xhr.onreadystatechange = function () {
                        if (xhr.readyState == 4 && xhr.status == 200) {
                            document.getElementById("status").innerHTML = xhr.responseText;
                            listFiles();
                        }
                    };
                    xhr.send();

                }
                if (action == "delete") {
                    xhr.open("GET", urltocall, true);
                    xhr.onreadystatechange = function () {
                        if (xhr.readyState == 4 && xhr.status == 200) {
                            document.getElementById("status").innerHTML = xhr.responseText;
                            listFiles();
                        };
                    };
                    xhr.send();
                }
                if (action == "download") {
                    document.getElementById("status").innerHTML = "";
                    window.open(urltocall, "_blank");
                }
            }

            function showUploadButton() {
                document.getElementById("upload_button").hidden = true;
                document.getElementById("upload_details_header").hidden = false;
                document.getElementById("upload_details").hidden = false;
                document.getElementById("upload_details_header").innerHTML = "<h4>Upload File<h4>";
                document.getElementById("status").innerHTML = "";
                var uploadform = `
                <form method = "POST" action = "/" enctype="multipart/form-data"><input type="file" name="data"/>
                    <input type="submit" name="upload" value="Upload" title = "Upload File" accept=".bin, .hex">
                </form>`;
                document.getElementById("upload_details").innerHTML = uploadform;
                var uploadform = `
                <form id="upload_form" enctype="multipart/form-data" method="post">
                    <input type="file" name="file1" id="file1" onchange="uploadFile()" accept=".bin, .hex">
                    <br>
                    <progress id="progressBar" value="0" max="100" style="width:300px;"></progress>                    
                    <h4 id="status"></h4>
                    <p id="loaded_n_total"></p>
                </form>`;
                document.getElementById("upload_details").innerHTML = uploadform;
            }

            function _(elementId) {
                return document.getElementById(elementId);
            }

            function uploadFile() {
                var file = _("file1").files[0];
                var formData = new FormData();
                formData.append("file1", file);
                var ajax = new XMLHttpRequest;
                ajax.upload.addEventListener("progress", progressHandler, false);
                ajax.addEventListener("load", completeHandler, false);
                ajax.addEventListener("error", errorHandler, false);
                ajax.addEventListener("abort", abortHandler, false);
                ajax.open("POST", "/");
                ajax.send(formData);
            }

            function humanReadableSize(bytes) {
                if (bytes < 1024)
                    return bytes + " bytes";
                else if (bytes < (1024 * 1024))
                    return (bytes / 1024.0).toFixed(2) + " KB";
                else if (bytes < (1024 * 1024 * 1024))
                    return (bytes / 1024.0 / 1024.0).toFixed(2) + " MB";
                else
                    return (bytes / 1024.0 / 1024.0 / 1024.0).toFixed(2) + " GB";
            }

            function progressHandler(event) {
                _("loaded_n_total").innerHTML = "Uploaded " + humanReadableSize(event.loaded);
                var percent = (event.loaded / event.total) * 100;
                _("progressBar").value = Math.round(percent);
                _("status").innerHTML = Math.round(percent) + "% uploaded... please wait";
                if (percent >= 100) {
                    _("status").innerHTML = "Please wait, writing file to filesystem";
                }
            }

            function completeHandler(event) {
                _("status").innerHTML = "Upload Complete";
                _("progressBar").value = 0;
                document.getElementById("upload_details_header").hidden = true;
                document.getElementById("upload_details").hidden = true;
                document.getElementById("upload_button").hidden = false;
                listFiles();
            }

            function errorHandler(event) {
                _("status").innerHTML = "Upload Failed";
                document.getElementById("upload_details_header").hidden = true;
                document.getElementById("upload_details").hidden = true;
                document.getElementById("upload_button").hidden = false;

            }

            function abortHandler(event) {
                _("status").innerHTML = "inUpload Aborted";
                document.getElementById("upload_details_header").hidden = true;
                document.getElementById("upload_details").hidden = true;
                document.getElementById("upload_button").hidden = false;

            }

            function getTeensyVersion() {
                var xhr = new XMLHttpRequest();
                xhr.open("GET", "/refresh_teensy_version", true);
                xhr.onreadystatechange = function () {
                    if (xhr.readyState == 4 && xhr.status == 200) {
                        document.getElementById("teensy_version").innerHTML = xhr.responseText;
                    }
                };
                xhr.send();
            }

            var eventsSeenBefore = false;
            if (!!window.EventSource) {
                var source = new EventSource('/events');

                source.addEventListener('open', function (e) {
                    console.log("Events Connected");
                    if (eventsSeenBefore) {
                        location.reload();
                    }
                    eventsSeenBefore = true;
                }, false);

                source.addEventListener('error', function (e) {
                    if (e.target.readyState != EventSource.OPEN) {
                        console.log("Events Disconnected");
                    }
                }, false);

                source.addEventListener('message', function (e) {
                    console.log("message", e.data);
                }, false);

                source.addEventListener('progress', function (e) {
                    console.log("progress", e.data);
                    document.getElementById("status").innerHTML = e.data;
                    if (e.data == "Teensy update sent.") {
                        setTimeout(getTeensyVersion, 2000);
                    }
                }, false);
            }

        </script>
    </div>
</body>

</html>
)rawliteral";