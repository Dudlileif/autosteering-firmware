const char status_html[] PROGMEM = R"rawliteral(
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
        <h2>Tractor Autosteering</h2>
        <form action="/">
            <button type="submit">
                <h3>Menu</h3>
            </button>
        </form>
        <h3>Hardware Status</h3>
        <h4>Uptime</h4>
        <table>
            <tr>
                <th>
                    <h4>
                        ESP32
                    </h4>
                </th>
                <th>
                    <h4> | </h4>
                </th>
                <th>
                    <h4>
                        Teensy
                    </h4>
                </th>
            </tr>
            <tr>
                <td>
                    <span id="esp_uptime">%ESP_UPTIME%</span>
                </td>
                <td> | </td>
                <td>
                    <span id="teensy_uptime">%TEENSY_UPTIME%</span>
                </td>
            </tr>
        </table>
        <h4>Teensy crash report</h4>
        <p id="teensy_crash_report">
            %TEENSY_CRASH_REPORT%
        </p>
        <table>
            <tr>
                <td>
                    <button onclick="refreshUptime()"><h3>Refresh uptime</h3></button>
                </td>
                <td>
                    <button onclick="refreshUptime()"><h3>Refresh crash report</h3></button>
                </td>
                <td>
                    <button onclick="rebootTeensy()"><h3>Reboot Teensy</h3></button>
                </td>
                <td>
                    <button onclick="rebootESP()"><h3>Reboot ESP</h3></button>
                </td>
            </tr>
        </table>
        <script>
            function refreshUptime(){
                xmlhttp = new XMLHttpRequest();
                xmlhttp.open("GET", "/refresh_teensy_uptime", false);
                xmlhttp.send();
                document.getElementById("teensy_uptime").innerHTML = xmlhttp.responseText;

                xmlhttp2 = new XMLHttpRequest();
                xmlhttp2.open("GET", "/refresh_esp_uptime", false);
                xmlhttp2.send();
                document.getElementById("esp_uptime").innerHTML = xmlhttp2.responseText;
            }

            function getTeensyCrashReport(){
                xmlhttp = new XMLHttpRequest();
                xmlhttp.open("GET", "/refresh_teensy_crash_report", false);
                xmlhttp.send();
                document.getElementById("teensy_crash_report").innerHTML = xmlhttp.responseText;
            }
            
            function rebootTeensy(){
                xmlhttp = new XMLHttpRequest();
                xmlhttp.open("GET", "/reboot_teensy", false);
                xmlhttp.send();
            }

            function rebootESP(){
                xmlhttp = new XMLHttpRequest();
                xmlhttp.open("GET", "/reboot", false);
                xmlhttp.send();
            }

        </script>

    </div>
</body>

</html>
)rawliteral";