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
// along with Autosteering Firmware. If not, see https://www.gnu.org/licenses.

const char network_html[] PROGMEM = R"rawliteral(
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

        .content table {
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
        <h3>Network setup</h3>
        <table>
            <tr>
                <td>
                    <h4>Show passwords</h4>
                </td>
                <td><input type="checkbox" name="show_passwords" id="show_passwords"
                        onclick="togglePasswordVisibility()">
                </td>
            </tr>
        </table>
        <h3>Hostname</h3>
        <form action="/update_network_config_local">
            <table>
                <tr>
                    <td>
                        <h4>Hostname</h4>
                    </td>
                    <td>
                        <input type="text" maxlength=32 name="hostname" id="hostname" value="%HOSTNAME_VALUE%">
                    </td>
                    <td>
                        <input type="submit" value="Submit">
                    </td>
                </tr>
            </table>
        </form>
        <p id="scan_results" hidden>
        <h3>Available networks</h3>
        <table id="scan_results_table" hidden></table>
        </p>
        <h3>Saved networks</h3>
        <form action="/update_network_config_local">
            <table>
                <tr>
                    <th>Network</th>
                    <th>SSID</th>
                    <th>Password</th>
                </tr>
                %NETWORK_CONFIG_PLACEHOLDER%
                <tr>
                    <td colspan="3">
                        <input type="submit" value="Submit">
                    </td>
                </tr>
            </table>
        </form>
        <br>
        <h3>AP network configuration</h3>
        <form action="/update_network_config_local">
            <table>
                <tr>
                    <th>SSID</th>
                    <th>Password</th>
                    <th></th>
                </tr>
                %AP_CONFIG_PLACEHOLDER%
            </table>
        </form>

        <table>
            <tr>
                <td>
                    <table>
                        <tr>
                            <td>
                                <h4>Start in AP mode</h4>
                            </td>
                            <td>
                                <input type="checkbox" name="start_in_ap_mode" id="start_in_ap_mode"
                                    onchange="toggleAPCheckbox(this)" %START_IN_AP_MODE_VALUE%>
                            </td>
                        </tr>
                    </table>
                </td>
                <td>
                    <button onclick="rebootESP()">Reboot</button>
                </td>
            </tr>
        </table>
        <br>
        %BASE_STATION_RELAY%
        <h3>Network ports</h3>
        <form action="/update_network_config_local">
            <table>
                <tr>
                    <th>Type</th>
                    <th>Port</th>
                </tr>
                %NETWORK_PORTS_PLACEHOLDER%
                <tr>
                    <td colspan="2">
                        <input type="submit" value="Submit">
                    </td>
                </tr>
            </table>
        </form>
    </div>
    <script>
        startNetworkScan();
        setInterval(startNetworkScan, 5000);

        function togglePasswordVisibility() {
            var idStartsWith = "password";
            document.querySelectorAll(`[id^=${idStartsWith}]`).forEach(element => {
                if (element.type == "password") {
                    element.type = "text";
                }
                else if (element.type == "text") {
                    element.type = "password";
                }
            });
        }

        function toggleAPCheckbox(element) {
            var xhr = new XMLHttpRequest();
            if (element.checked) { xhr.open("GET", "/update_network_config_local?" + element.id + "=on", true); }
            else { xhr.open("GET", "/update_network_config_local?" + element.id + "=off", true); }
            xhr.send();
        }

        function startNetworkScan() {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/scan_networks", true);
            xhr.onreadystatechange = function () {
                if (xhr.readyState == 4 && xhr.status == 200) {
                    setTimeout(setFoundNetworks, 1000);
                }
            };
            xhr.send();
        }

        function setFoundNetworks() {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/found_networks", true);
            xhr.responseType = "json";

            xhr.onreadystatechange = function () {
                if (xhr.readyState == 4 && xhr.status == 200) {
                    var data = xhr.response;

                    let element = document.getElementById("scan_results");
                    if (element.hidden) {
                        element.hidden = false;
                    }

                    if ("networks" in data) {
                        let tableElement = document.getElementById("scan_results_table");
                        if (tableElement.hidden) {
                            tableElement.hidden = false;
                        }
                        var table = `
                        <tr>
                            <th>SSID</th>
                            <th>AUTH</th>
                            <th>RSSI</th>
                        </tr>`;

                        data["networks"].forEach((network) => {
                            table += "<tr>";
                            table += "<td>" + network["ssid"] + "</td>";
                            table += "<td>" + network["auth_name"] + "</td>";
                            table += "<td>" + network["rssi"] + "</td>";
                            table += "</tr>";
                        });

                        tableElement.innerHTML = table;

                    }
                    else {

                    }
                }
            }
            xhr.send();
        };

        function rebootESP() {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/reboot", true);
            xhr.send();
        }

        var eventsSeenBefore = false;
        if (!!window.EventSource) {
            var source = new EventSource("/events");
            source.addEventListener("open", function (e) {
                console.log("Events Connected");
                if (eventsSeenBefore) {
                    location.reload();
                }
                eventsSeenBefore = true;
            }, false);
            source.addEventListener("networks", function (e) { 
                var networks =  e.data;
            })
        }

    </script>
</body>

</html>
)rawliteral";