const char main_html[] PROGMEM = R"rawliteral(
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
        <h2>Tractor Autosteering</h2>
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
            <tr>
                <td>
                    <form action="/motor">
                        <button type="submit">
                            <h3>Motor configuration</h3>
                        </button>
                    </form>
                    <br>
                </td>
            </tr>
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
                    <form action="/teensy">
                        <button type="submit">
                            <h3>Teensy status</h3>
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