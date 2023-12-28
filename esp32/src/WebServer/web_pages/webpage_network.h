const char network_html[] PROGMEM = R"rawliteral(
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

        .content table {
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
        <form action="/update_network_config">
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
        <h3>Client network configuration</h3>
        <form action="/update_network_config">
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
        <form action="/update_network_config">
            <table>
                <tr>
                    <th>SSID</th>
                    <th>Password</th>
                    <th></th>
                </tr>
                %AP_CONFIG_PLACEHOLDER%
            </table>
        </form>
        <form action="/reboot">
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
                        <input type="submit" value="Reboot" id="reboot" name="reboot">
                    </td>
                </tr>
            </table>
        </form>
        <br>
        <h3>Network ports</h3>
        <form action="/update_network_config">
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
            if (element.checked) { xhr.open("GET", "/update_network_config?" + element.id + "=on", true); }
            else { xhr.open("GET", "/update_network_config?" + element.id + "=off", true); }
            xhr.send();
        }
    </script>
</body>

</html>
)rawliteral";