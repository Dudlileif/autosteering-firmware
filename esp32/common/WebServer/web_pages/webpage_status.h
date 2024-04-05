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

#ifdef BASE_STATION
const char status_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
    <title>Base Station</title>
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
        <h2>Base Station</h2>
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
            </tr>
            <tr>
                <td>
                    <span id="esp_uptime">%ESP_UPTIME%</span>
                </td>
            </tr>
        </table>
        <table>
            <tr>
                <td>
                    <button onclick="refreshUptime()">
                        <h3>Refresh uptime</h3>
                    </button>
                </td>
                <td>
                    <button onclick="rebootESP()">
                        <h3>Reboot ESP</h3>
                    </button>
                </td>
            </tr>
        </table>
        <script>
            function refreshUptime() {
                var xhr = new XMLHttpRequest();
                xhr.open("GET", "/refresh_esp_uptime", true);
                xhr.onreadystatechange = function () {
                    if (xhr.readyState == 4 && xhr.status == 200) {
                        document.getElementById("esp_uptime").innerHTML = xhr.responseText;
                    }
                };
                xhr.send();
            }

            function rebootESP() {
                var xhr = new XMLHttpRequest();
                xhr.open("GET", "/reboot", true);
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
            }
        </script>

    </div>
</body>

</html>
)rawliteral";
#endif

#ifndef BASE_STATION
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
                    <button onclick="refreshUptime()">
                        <h3>Refresh uptime</h3>
                    </button>
                </td>
                <td>
                    <button onclick="getTeensyCrashReport()">
                        <h3>Refresh crash report</h3>
                    </button>
                </td>
                <td>
                    <button onclick="rebootTeensy()">
                        <h3>Reboot Teensy</h3>
                    </button>
                </td>
                <td>
                    <button onclick="rebootESP()">
                        <h3>Reboot ESP</h3>
                    </button>
                </td>
            </tr>
        </table>
        <script>
            function refreshUptime() {
                var xhr = new XMLHttpRequest();
                xhr.open("GET", "/refresh_teensy_uptime", true);
                xhr.onreadystatechange = function () {
                    if (xhr.readyState == 4 && xhr.status == 200) {
                        document.getElementById("teensy_uptime").innerHTML = xhr.responseText;
                    }
                };
                xhr.send();

                var xhr2 = new XMLHttpRequest();
                xhr2.open("GET", "/refresh_esp_uptime", true);
                xhr2.onreadystatechange = function () {
                    if (xhr2.readyState == 4 && xhr2.status == 200) {
                        document.getElementById("esp_uptime").innerHTML = xhr2.responseText;
                    }
                };
                xhr2.send();
            }

            function getTeensyCrashReport() {
                var xhr = new XMLHttpRequest();
                xhr.open("GET", "/refresh_teensy_crash_report", true);
                xhr.onreadystatechange = function () {
                    if (xhr.readyState == 4 && xhr.status == 200) {
                        document.getElementById("teensy_crash_report").innerHTML = xhr.responseText;
                    }
                };
                xhr.send();
            }

            function rebootTeensy() {
                var xhr = new XMLHttpRequest();
                xhr.open("GET", "/reboot_teensy", true);
                xhr.send();
            }

            function rebootESP() {
                var xhr = new XMLHttpRequest();
                xhr.open("GET", "/reboot", true);
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
            }
        </script>

    </div>
</body>

</html>
)rawliteral";
#endif