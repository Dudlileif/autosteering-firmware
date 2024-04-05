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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Autosteering Firmware.  If not, see https://www.gnu.org/licenses/.

const char main_html[] PROGMEM = R"rawliteral(
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

        .buttons {
            width: 100%%;
            table-layout: fixed;
            border-collapse: collapse;
        }

        .buttons button {
            width: 100%%;
        }
    </style>
</head>

<body>
    <div class="content">
        <h2>%TITLE%</h2>
        <h3>IP Address: %IP_ADDRESS%</h3>
        <table class="buttons">
            <tr>
                <td>
                    <form action="/network">
                        <button type="submit">
                            <h3>Network configuration</h3>
                        </button>
                    </form>
                    <br>
                </td>
            </tr>
            %MOTOR%
            <tr>
                <td>
                    <form action="/firmware">
                        <button type="submit">
                            <h3>Firmware update</h3>
                        </button>
                    </form>
                    <br>
                </td>
            </tr>
            <tr>
                <td>
                    <form action="/status">
                        <button type="submit">
                            <h3>Hardware status</h3>
                        </button>
                    </form>
                    <br>
                </td>
            </tr>
        </table>
    </div>
</body>

</html>
)rawliteral";