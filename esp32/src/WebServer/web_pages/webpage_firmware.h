const char firmware_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
    <title>Tractor Autosteering</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style>
        .content {
            padding-left: 1rem;
            padding-right: 1rem;
            padding-top: 1rem;
            padding-right: 1rem;
            margin: auto;
            max-width: 500px;
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
        <h2>Tractor Autosteering</h2>
        <form action="/">
            <button type="submit">
                <h3>Menu</h3>
            </button>
        </form>

        <h3>Update firmware</h3>
        <p>
        <h4>Installed versions</h4>
        ESP: %ESP_FIRMWARE_DATE%
        <br>
        Teensy: %TEENSY_FIRMWARE_DATE%
        <form action="/refresh_teensy_version">
            <input type="submit" value="Refresh Teensy version">
        </form>
        </p>
        <p>Free Storage: <span id="free_stotrage">%FREE_STORAGE%</span> | Used Storage: <span
                id="used_storage">%USED_STORAGE%</span> | Total Storage: <span id="total_storage">%TOTAL_STORAGE%</span>
        </p>
        <p>
            <button onclick="listFilesButton()">List Files</button>
            <button onclick="showUploadButton()">Upload File</button>
        </p>
        <p id="status"></p>
        <p id="detailsheader"></p>
        <p id="details"></p>
        <script>
            function listFilesButton() {
                xmlhttp = new XMLHttpRequest();
                xmlhttp.open("GET", "/list_files", false);
                xmlhttp.send();
                document.getElementById("detailsheader").innerHTML = "<h4>Files<h4>";
                document.getElementById("details").innerHTML = xmlhttp.responseText;
            }

            function installDownloadDeleteButton(filename, action) {
                var urltocall = "/file?name=" + filename + "&action=" + action;
                xmlhttp = new XMLHttpRequest();
                if (action == "install") {
                    xmlhttp.open("GET", urltocall, false);
                    xmlhttp.send();
                    document.getElementById("status").innerHTML = xmlhttp.responseText;
                    xmlhttp.open("GET", "/list_files", false);
                    xmlhttp.send();
                }
                if (action == "delete") {
                    xmlhttp.open("GET", urltocall, false);
                    xmlhttp.send();
                    document.getElementById("status").innerHTML = xmlhttp.responseText;
                    xmlhttp.open("GET", "/list_files", false);
                    xmlhttp.send();
                    document.getElementById("details").innerHTML = xmlhttp.responseText;
                }
                if (action == "download") {
                    document.getElementById("status").innerHTML = "";
                    window.open(urltocall, "_blank");
                }
            }

            function showUploadButton() {
                document.getElementById("detailsheader").innerHTML = "<h4>Upload File<h4>";
                document.getElementById("status").innerHTML = "";
                var uploadform = "<form method = \"POST\" action = \"/\" enctype=\"multipart/form-data\"><input type=\"file\" name=\"data\"/><input type=\"submit\" name=\"upload\" value=\"Upload\" title = \"Upload File\" accept=\".bin, .hex\"></form>";
                document.getElementById("details").innerHTML = uploadform;
                var uploadform =
                    "<form id=\"upload_form\" enctype=\"multipart/form-data\" method=\"post\">" +
                    "<input type=\"file\" name=\"file1\" id=\"file1\" onchange=\"uploadFile()\" accept=\".bin, .hex\"><br>" +
                    "<progress id=\"progressBar\" value=\"0\" max=\"100\" style=\"width:300px;\"></progress>" +
                    "<h4 id=\"status\"></h4>" +
                    "<p id=\"loaded_n_total\"></p>" +
                    "</form>";
                document.getElementById("details").innerHTML = uploadform;
            }

            function _(el) {
                return document.getElementById(el);
            }

            function uploadFile() {
                var file = _("file1").files[0];
                var formData = new FormData();
                formData.append("file1", file);
                var ajax = new XMLHttpRequest;
                ajax.upload.addEventListener("progress", progressHandler, false);
                ajax.addEventListener("load", completeHandler, false); // doesnt appear to ever get called even upon success
                ajax.addEventListener("error", errorHandler, false);
                ajax.addEventListener("abort", abortHandler, false);
                ajax.open("POST", "/");
                ajax.send(formData);
            }

            function humanReadableSize(bytes) {
                if (bytes < 1024)
                    return bytes + " bytes";
                else if (bytes < (1024 * 1024))
                    return bytes / 1024.0 + " KB";
                else if (bytes < (1024 * 1024 * 1024))
                    return bytes / 1024.0 / 1024.0 + " MB";
                else
                    return bytes / 1024.0 / 1024.0 / 1024.0 + " GB";
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
                xmlhttp = new XMLHttpRequest();
                xmlhttp.open("GET", "/list_files", false);
                xmlhttp.send();
                document.getElementById("status").innerHTML = "File Uploaded";
                document.getElementById("detailsheader").innerHTML = "<h4>Files<h4>";
                document.getElementById("details").innerHTML = xmlhttp.responseText;
            }

            function errorHandler(event) {
                _("status").innerHTML = "Upload Failed";
            }

            function abortHandler(event) {
                _("status").innerHTML = "inUpload Aborted";
            }

        </script>
    </div>
</body>

</html>
)rawliteral";