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
        <h3>Motor configuration</h3>
        <p>The parameters are labelled in the same way as the ones in the official documentation for the TMC5160.
            Parameters for most velocities, acceleration and thresholds have been converted to RPM to make them more
            tangible, these are noted with an _RPM suffix.
            <a
                href="https://www.analog.com/media/en/technical-documentation/data-sheets/TMC5160A_datasheet_rev1.18.pdf">
                Link to TMC5160 documentation pdf</a>
        </p>
        <form action="/update_motor_config">
            <table>
                <tr>
                    <th>Parameter</th>
                    <th>Value</th>
                    <th>Description</th>
                </tr>
                <tr>
                    <td>
                        <h4>Micro steps</h4>
                    </td>
                    <td>
                        %MICRO_STEPS_PLACEHOLDER%
                    </td>
                    <td>
                        The number of micro steps to divide a full step of the motor into.
                    </td>
                </tr>
                <tr>
                    <td>
                        <h4>Steps per rotation</h4>
                    </td>
                    <td>
                        %STEPS_PER_ROT_PLACEHOLDER%
                    </td>
                    <td>
                        The number of full steps for one rotation of the motor.
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
    <script>
        function toggleCheckbox(element) {
            var xhr = new XMLHttpRequest();
            if (element.checked) { xhr.open("GET", "/update_motor_config?" + element.id + "=1", true); }
            else { xhr.open("GET", "/update_motor_config?" + element.id + "=0", true); }
            xhr.send();
        }
    </script>
</body>

</html>
)rawliteral";