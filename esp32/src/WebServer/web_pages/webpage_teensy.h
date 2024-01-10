const char teensy_html[] PROGMEM = R"rawliteral(
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
        <h3>Teensy status</h3>
        <h4>Teensy crash report</h4>
        <p>
            %TEENSY_CRASH_REPORT%
        </p>
        <table>
            <tr>
                <td>
                    <form action="/refresh_teensy_crash_report">
                        <button type="submit">
                            <h3>Refresh crash report</h3>
                        </button>
                    </form>
                </td>
                <td>
                    <form action="/reboot_teensy">
                        <button type="submit">
                            <h3>Reboot Teensy</h3>
                        </button>
                    </form>
                </td>
            </tr>
        </table>


    </div>
</body>

</html>
)rawliteral";