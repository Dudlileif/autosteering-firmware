const char motor_html[] PROGMEM = R"rawliteral(
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
        <h3>Motor configuration</h3>
        <p>The parameters are labelled in the same way as the ones in the official documentation for the TMC5160.
            <a
                href="https://www.analog.com/media/en/technical-documentation/data-sheets/TMC5160A_datasheet_rev1.18.pdf">
                Link to pdf</a>
        </p>
        <form action="/update_motor_config">
            <table>
                <tr>
                    <th>Parameter</th>
                    <th>Value</th>
                </tr>
                <tr>
                    <td>
                        <h4>Micro steps</h4>
                    </td>
                    <td>
                        %MICRO_STEPS_PLACEHOLDER%
                    </td>
                </tr>
                <tr>
                    <td>
                        <h4>Steps per rotation</h4>
                    </td>
                    <td>
                        %STEPS_PER_ROT_PLACEHOLDER%
                    </td>
                </tr>
                %MOTOR_CONFIG_PLACEHOLDER%
                <tr>
                    <td colspan="2">
                        <button type="submit" value="Submit">
                            <h4>Submit</h4>
                        </button>
                    </td>
                </tr>

            </table>
        </form>
    </div>
</body>

</html>
)rawliteral";