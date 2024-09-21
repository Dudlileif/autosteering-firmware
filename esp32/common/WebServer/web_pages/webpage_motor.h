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
        <form action="/update_motor_config_post" id="motor_config_form">
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
        <table>
            <tr>
                <td>
                    <h4>WAS reading</h4>
                </td>
                <td>
                    <p, id="was_reading">????</p>
                </td>
                <td>
                    <input type="range" min="0" max="4095" value="0" class="slider" id="was_reading_slider">
                </td>
            </tr>
            <tr>
                <td>
                    <h4>WAS target</h4>
                </td>
                <td>
                    <p, id="was_target">????</p>
                </td>
                <td>
                    <input type="range" min="0" max="4095" value="0" class="slider" id="was_target_slider" oninput="updateWasTargetSlider()">
                </td>
            </tr>
            <tr>
                <td>
                    <h4>WAS change rate</h4>
                </td>
                <td>
                    <p, id="was_change_rate">????</p>
                </td>
            </tr><tr>
                <td>
                    <h4>WAS error over time</h4>
                </td>
                <td>
                    <p, id="was_error_over_time">????</p>
                </td>
            </tr>
            <tr>
                <td>
                    <h4>Motor</h4>
                </td>
                <td>
                    Override <input type="checkbox" id="motor_override_control" onclick="toggleMotorOverride()">
                </td>
                <td>
                    <p, id="motor_enabled">????</p>
                </td>
                <td>
                    <input type="checkbox" id="motor_enabled_checkbox">
                </td>
                <td>
                    <button onclick="toggleMotor()" id="motor_toggle_button">Toggle Motor</button>
                </td>
            </tr>
            <tr>
                <td>
                    <h4>Motor stalled</h4>
                </td>
                <td>
                    <p, id="motor_stalled">????</p>
                </td>
                <td>
                    <input type="checkbox" id="motor_stalled_checkbox">
                </td>
            </tr>
            <tr>
                <td>
                    <h4>Motor RPM</h4>
                </td>
                <td>
                    <p, id="motor_rpm">????</p>
                </td>
            </tr>
            <tr>
                <td>
                    <h4>Motor CS</h4>
                </td>
                <td>
                    <p, id="motor_cs">????</p>
                </td>
            </tr>
            <tr>
                <td>
                    <h4>Motor StallGuard</h4>
                </td>
                <td>
                    <p, id="motor_sg">????</p>
                </td>
            </tr>
            <tr>
                <td>
                    <h4>Motor Position</h4>
                </td>
                <td>
                    <p, id="motor_pos">????</p>
                </td>
            </tr>
        </table>
    </div>
    <script>
        function toggleCheckbox(element) {
            var xhr = new XMLHttpRequest();
            if (element.checked) { xhr.open("GET", "/update_motor_config?" + element.id + "=1", true); }
            else { xhr.open("GET", "/update_motor_config?" + element.id + "=0", true); }
            xhr.send();
        }

        motor_config_form.onsubmit=async(e)=>{
            e.preventDefault();
            let response = await fetch('/update_motor_config_post', {
                method:'POST', 
                body:new FormData(motor_config_form)
            });
        };

        function updateWasTargetSlider() {
                document.getElementById('was_target').innerHTML = document.getElementById('was_target_slider').value;
        }
        
        var motorEnabled = false;
        var tryToEnableMotor = false;
        var overrideMotorControl = false;
        function toggleMotorOverride(){
            overrideMotorControl=!overrideMotorControl;
            document.getElementById('motor_override_control').checked=overrideMotorControl;
        }

        function toggleMotor() {
            tryToEnableMotor = !tryToEnableMotor;
            document.getElementById('motor_toggle_button').innerHTML = !tryToEnableMotor ? 'Enable' : 'Disable';
        }
        document.getElementById('motor_toggle_button').innerHTML = !tryToEnableMotor ? 'Enable' : 'Disable';

        setInterval(() => {
            if ((motorEnabled||tryToEnableMotor) && overrideMotorControl){
                var xhr = new XMLHttpRequest();
                data = {};
                data['enable_motor'] = tryToEnableMotor;
                data['was_target'] = document.getElementById('was_target_slider').value;
                xhr.open("GET", "/motor_control?data="+encodeURIComponent(JSON.stringify(data)), true);
                xhr.timeout=450;
                xhr.ontimeout=()=>{console.log('Motor control request failed.')};
                xhr.send();
            }
        }, 500);

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

            source.addEventListener('error', function (e) {
                if (e.target.readyState != EventSource.OPEN) {
                    console.log("Events Disconnected");
                }
            }, false);

            source.addEventListener('message', function (e) {
                console.log("message", e.data);
            }, false);

            source.addEventListener('data', function (e) {
                var data = JSON.parse(e.data);
                // console.log("data", data);
                document.getElementById('was_reading').innerHTML = data['was'];
                document.getElementById('was_reading_slider').value = data['was'];

                document.getElementById('was_target_slider').min = data['was_min'];
                document.getElementById('was_target_slider').max = data['was_max'];

                if (!overrideMotorControl){
                    document.getElementById('was_target').innerHTML = data['was_target'];
                    document.getElementById('was_target_slider').value = data['was_target'];
                }
                
                motorEnabled=data['motor_enabled'];

                document.getElementById('motor_enabled').innerHTML = motorEnabled ? 'Enabled' : 'Disabled';
                document.getElementById('motor_enabled_checkbox').checked = motorEnabled;

                document.getElementById('motor_stalled').innerHTML = data['motor_stalled'] ? 'Stalled' : '';
                document.getElementById('motor_stalled_checkbox').checked = data['motor_stalled'];

                document.getElementById('motor_rpm').innerHTML = data['motor_rpm'].toFixed(2);
                document.getElementById('motor_cs').innerHTML = data['motor_cs'];
                document.getElementById('motor_sg').innerHTML = data['motor_sg'];
                document.getElementById('motor_pos').innerHTML = data['motor_pos']; 
                document.getElementById('was_change_rate').innerHTML = data['was_change_rate']; 
                document.getElementById('was_error_over_time').innerHTML = data['was_error_over_time'];

            }, false);

            source.addEventListener('motor_config', function (e) {
                var data = JSON.parse(e.data);
                // console.log("motor_config", data);
                for (var key in data){
                    if (typeof(data[key])=='boolean'){
                        document.getElementById(key).checked=data[key];
                    }else if (key=='MICRO_STEPS'){
                        var values = [0,2,4,8,16,32,64,128,256];
                        for (var index in values){
                            document.getElementById('micro_steps_'+values[index]).checked = values[index] == data[key];
                        }
                    }
                    else if (key=='STEPS_PER_ROT'){
                        var values = [200,400];
                        for (var index in values) {
                            document.getElementById('steps_per_rot_'+values[index]).checked = values[index] == data[key];
                        }
                    }   
                    else{
                    document.getElementById(key).value=data[key];
                    }
                }
            }, false);
            
        }
    </script>
</body>

</html>
)rawliteral";