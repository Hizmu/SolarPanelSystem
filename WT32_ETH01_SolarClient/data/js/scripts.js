document.addEventListener('DOMContentLoaded', () => {

    // API Endpoints
    const API_CONTROL_PANEL = "/api/panel/control";
    const API_PANEL_STATUS = "/api/panel/data";
    const API_NETWORK = "/api/network/data";
    const API_NETWORK_SAVE = "/api/network/save";
    const API_SENSOR_DATA = "/api/sensor/data"; 
    const API_PLC_CONNECTION_STATUS = "/api/plc/connectionstatus";

    const API_DEBUG_LOGS = "/logs";
    const API_ESP_STATUS = "/esp/status"; 
    // Navigation Elements
    const navButtons = document.querySelectorAll('.nav-button:not(.menu)');
    const menuButton = document.getElementById('menu-button');
    const navBtnsContainer = document.getElementById('nav-btns');

    // Panels
    const currentPanel = document.getElementById('current-panel');
    const controlPanel = document.getElementById('control-panel');
    const networkSettingsPanel = document.getElementById('network-settings');
    const noConnectionPanel = document.getElementById('no-connection-panel'); 
    
    // Status Elements
    const currentTimeElement = document.getElementById('current-time');
    const onlineStatus = document.getElementById('online-status');

    // Control Buttons
    const startButton = document.getElementById('start-button');
    const stopButton = document.getElementById('stop-button');
    const manualButton = document.getElementById('manual-button');
    const resetPositionButton = document.getElementById('reset-position-button');
    const zeroPositionButton = document.getElementById('zero-position-button');
    const autoStartCheckbox = document.getElementById('auto-start-checkbox');
    const speedInput = document.getElementById('speed-input');
    const moveUpBtn = document.getElementById('move-up');
    const moveDownBtn = document.getElementById('move-down');
    const moveLeftBtn = document.getElementById('move-left');
    const moveRightBtn = document.getElementById('move-right');

    // Display Elements
    const state_value = document.getElementById('state-value');
    const mode_value = document.getElementById('mode-value');
    const voltage_value = document.getElementById('voltage-value');
    const current_value = document.getElementById('current-value');
    const temperature_value = document.getElementById('temperature-value');
    const humidity_value = document.getElementById('humidity-value');
    const horizontal_position = document.getElementById('horizontal-value');
    const vertical_position = document.getElementById('vertical-value');
    const currentHorizontal = document.getElementById('current-horizontal');
    const currentVertical = document.getElementById('current-vertical');
    const SunSensor = document.getElementById('sunsensor-value');
    const horizontal_state = document.getElementById('horizontal-state');
    const vertical_state = document.getElementById('vertical-state'); 

    // Network Inputs
    const saveNetworkButton = document.getElementById('save-network-button');
    const ssidInput = document.getElementById('ssid-input');
    const passwordInput = document.getElementById('password-input');
    const espstatus = document.getElementById('esp-status');
    const espip = document.getElementById('esp-ip');

    // Additional Display Elements
    const speedValue = document.getElementById('speed-value'); // Додано визначення

    // Movement State Variables
    let up_pressed = false;
    let down_pressed = false;
    let left_pressed = false;
    let right_pressed = false;
    
    // Classes
    class SolarPanelStatus {
        constructor() {
            this.Started = false;
            this.AutoStart = false;
            this.ManualMode = false;
            this.H_MoveDone = false;
            this.H_Moving = false;
            this.V_MoveDone = false;
            this.V_Moving = false;
            this.Sunlight_Founded = false;
            this.H_Position = 0;
            this.V_Position = 0;
            this.Velocity = 0;
            this.SunSensor = 0;
        }
    }
    
    class SensorData {
        constructor() {
            this.temperature = 0;
            this.humidity = 0;
            this.current = 0;
            this.voltage = 0;
            this.power = 0; // Додано для потужності
        }
    }

    // Instances
    const panelStatus = new SolarPanelStatus();
    const sensorData = new SensorData();
    let timeSended = false;
    let isPLCConnected = false;

    // Function to update current time
    function updateTime() {
        const now = new Date();
        const hours = String(now.getHours()).padStart(2, '0');
        const minutes = String(now.getMinutes()).padStart(2, '0');
        currentTimeElement.textContent = `${hours}:${minutes}`; // Використано шаблонний рядок
    }

    // Main Update Function
    function Update() {
        checkConnectionStatus();
        changeOnlineStatus();
        displayData();
        controlButtonsState();

        if(isPLCConnected) {   
            if(!timeSended) {
                controlPanelAPI("set_datetime");
                timeSended = true;
            }
            fetchPanelStatus();
            
        }
    }

    // Initial Calls and Intervals
    setInterval(updateTime, 60000);
    setInterval(Update, 1000);
    setInterval(getLogs, 2000);
    setInterval(handleEspStatus, 2000);
    setInterval(fetchSensorData, 1000);


    checkConnectionStatus();
    updateTime();
    
    // Обробка навігаційних кнопок
    navButtons.forEach(button => {
        button.addEventListener('click', () => {

            navButtons.forEach(btn => btn.classList.remove('active'));
            button.classList.add('active');

            document.querySelectorAll('main > section').forEach(section => {
                section.style.display = 'none';
            });

            if (window.innerWidth <= 480) {
                navBtnsContainer.classList.remove('open');
            }
            const target = button.getAttribute('data-target');
            const targetSection = document.getElementById(target);
            if (targetSection) {
                if (target === 'control-panel' && !isPLCConnected) {
                    noConnectionPanel.style.display = "block";
                } else {
                    noConnectionPanel.style.display = "none";
                }
                targetSection.style.display = "block";
                if(target === 'charts-panel'){
                    initializeCharts();
                }
            }

        });
    });

    // Обробка кнопки меню
    menuButton.addEventListener('click', () => {
        navBtnsContainer.classList.toggle('open');
    });

    // Функція для відображення даних
    function displayData() {
        state_value.textContent = panelStatus.Started ? "Запущений" : "Зупинений";
        mode_value.textContent = panelStatus.ManualMode ?  "Ручний" : "Авто";
        autoStartCheckbox.checked = panelStatus.AutoStart;
        speedInput.value = panelStatus.Velocity;
        voltage_value.textContent = sensorData.voltage;
        current_value.textContent = sensorData.current;
        temperature_value.textContent = sensorData.temperature;
        humidity_value.textContent = sensorData.humidity;
        horizontal_position.textContent = panelStatus.H_Position;
        vertical_position.textContent = panelStatus.V_Position;
        currentHorizontal.textContent = panelStatus.H_Position;
        currentVertical.textContent = panelStatus.V_Position;
        SunSensor.textContent = panelStatus.SunSensor;

        // Обчислення потужності
        sensorData.power = sensorData.voltage * sensorData.current;

        if (panelStatus.H_Moving) {
            horizontal_state.textContent = "Moving";
        } else if (panelStatus.H_MoveDone) {
            horizontal_state.textContent = "Move done";
        } else{
            horizontal_state.textContent = "Stop";
        }
        if (panelStatus.V_Moving) {
            vertical_state.textContent = "Moving";
        } else if (panelStatus.V_MoveDone) {
            vertical_state.textContent = "Move done";
        } else{
            vertical_state.textContent = "Stop";
        }
    }

    // Функція для зміни статусу онлайн
    function changeOnlineStatus() {
        if(isPLCConnected) {
            onlineStatus.classList.remove("offline");
            onlineStatus.classList.add("online");
            onlineStatus.textContent = "PLC Online";
            return;
        }
        onlineStatus.classList.remove("online");
        onlineStatus.classList.add("offline");
        onlineStatus.textContent = "PLC Offline";
    }

    // Функція для контролю стану кнопок
    function controlButtonsState() {
        
        if(isPLCConnected) {
            if (panelStatus.Started) {
                startButton.classList.add('green');
            } else {
                startButton.classList.remove('green');
            }

            if (!panelStatus.Started) {
                stopButton.classList.add('red');
            } else {
                stopButton.classList.remove('red');
            }
            if(panelStatus.ManualMode) {
                manualButton.classList.add("green");
            } else{
                manualButton.classList.remove("green");
            }
            startButton.classList.remove("gray");
            stopButton.classList.remove("gray");
            manualButton.classList.remove("gray");
        } else{
            startButton.classList.add("gray");
            stopButton.classList.add("gray");
            manualButton.classList.add("gray");
        }

        if(isPLCConnected && panelStatus.ManualMode)
        {
            if(panelStatus.V_Moving) {
                if(up_pressed) {
                    moveUpBtn.classList.add("green");
                } else if(down_pressed){
                    moveDownBtn.classList.add("green");
                }
            } else{
                down_pressed = false;
                up_pressed = false;
                moveUpBtn.classList.remove("green");
                moveDownBtn.classList.remove("green");
            }
            if(panelStatus.H_Moving) {
                if(left_pressed) {
                    moveLeftBtn.classList.add("green");
                } else if(right_pressed){
                    moveRightBtn.classList.add("green");
                }
            } else{
                moveLeftBtn.classList.remove("green");
                moveRightBtn.classList.remove("green");
            }
            resetPositionButton.classList.remove("gray");
            zeroPositionButton.classList.remove("gray");
            moveUpBtn.classList.remove("gray");
            moveDownBtn.classList.remove("gray");
            moveLeftBtn.classList.remove("gray");
            moveRightBtn.classList.remove("gray");
            speedInput.classList.remove("gray");
        } else{
            resetPositionButton.classList.add("gray");
            zeroPositionButton.classList.add("gray");
            moveUpBtn.classList.add("gray");
            moveDownBtn.classList.add("gray");
            moveLeftBtn.classList.add("gray");
            moveRightBtn.classList.add("gray");
            speedInput.classList.add("gray");
        }
        return
        startButton.disabled = !isPLCConnected;
        stopButton.disabled =  !isPLCConnected;
        manualButton.disabled =  !isPLCConnected;
        resetPositionButton.disabled =  !isPLCConnected;
        zeroPositionButton.disabled =  !isPLCConnected;
        moveUpBtn.disabled =   !isPLCConnected;
        moveDownBtn.disabled =  !isPLCConnected;
        moveLeftBtn.disabled =   !isPLCConnected;
        moveRightBtn.disabled =  !isPLCConnected;
        speedInput.disabled = !isPLCConnected;
    }

    // Слухачі подій для кнопок управління
    startButton.addEventListener('click', () => {
        if (!isPLCConnected) {
            console.error('Cannot send command start: No connection to PLC.');
            return;
        }
        controlPanelAPI('start');
    });

    stopButton.addEventListener('click', () => {
        if (!isPLCConnected) {
            console.error('Cannot send command stop: No connection to PLC.');
            return;
        }
        controlPanelAPI('stop');
    });

    manualButton.addEventListener('click', () => {
        if (isPLCConnected) { 
            if (!panelStatus.ManualMode) {
                controlPanelAPI('remoteOn');
            } else {
                controlPanelAPI('remoteOff');
            }
        } else {
            console.error('Cannot send command remote: No connection to PLC.');
        }
    });

    resetPositionButton.addEventListener('click', () => {
        if (!isPLCConnected) {
            console.error('Cannot send command reset_position: No connection to PLC.');
            return;
        }
        if(!panelStatus.ManualMode)
        {
            blinkButton(manualButton, "red", 3, 500);
            return; 
        }
        controlPanelAPI('reset_position');
    });

    zeroPositionButton.addEventListener('click', () => {
        if (!isPLCConnected) {
            console.error('Cannot send command zero_position: No connection to PLC.');
            return;
        }
        if(!panelStatus.ManualMode)
        {
            blinkButton(manualButton, "red", 3, 500);
            return;
        }
        controlPanelAPI('zero_position');
    });

    autoStartCheckbox.addEventListener('change', () => {
        if (!isPLCConnected) {
            console.error('Cannot send command autostart: No connection to PLC.');
            return;
        }
        const autoStart = autoStartCheckbox.checked;
        if (autoStart) {
            controlPanelAPI("autostartOn");
        } else {
            controlPanelAPI("autostartOff");
        }
    });

    moveUpBtn.addEventListener('click', () => movePanel('up'));
    moveDownBtn.addEventListener('click', () => movePanel('down'));
    moveLeftBtn.addEventListener('click', () => movePanel('left'));
    moveRightBtn.addEventListener('click', () => movePanel('right'));

    // Функція для миготіння кнопки
    function blinkButton(button, color, times, interval) {
        let count = 0;
        const originalColor = button.style.backgroundColor; 

        const blinkInterval = setInterval(() => {
            if (count < times * 2) { 
                button.style.backgroundColor = 
                    button.style.backgroundColor === color ? originalColor : color;
                count++;
            } else {
                clearInterval(blinkInterval); 
                button.style.backgroundColor = originalColor; 
            }
        }, interval);
    }

    // Функція для руху панелі
    function movePanel(direction) {
        if (!isPLCConnected) {
            console.error('Cannot send command move_panel: No connection to PLC.');
            return;
        }
        let value = '';
        if (direction === 'up') {
            up_pressed = true;
            value = 'move_up';
        } else if (direction === 'down') {
            down_pressed = true;
            value = 'move_down';
        } else if (direction === 'left') {
            left_pressed = true;
            value = 'move_left';
        } else if (direction === 'right') {
            right_pressed = true;
            value = 'move_right';
        }
        controlPanelAPI('direction', value);
    }

    // Обробка зміни швидкості
    speedInput.addEventListener('input', () => {
        let value = parseInt(speedInput.value, 10);
        if (isNaN(value) || value < 0) {
            value = 0;
        } else if (value > 10) {
            value = 10;
        }
        speedInput.value = value;
        speedValue.textContent = value;
        controlPanelAPI('change_velocity', value);
    });

    // Обробка збереження мережевих налаштувань
    saveNetworkButton.addEventListener('click', () => {
        const ssid = ssidInput.value.trim();
        const password = passwordInput.value.trim();
        if (ssid === '' || password === '') {
            alert('Будь ласка, заповніть всі поля.');
            return;
        }
        const credentials = {
            ssid: ssid,
            password: password
        };
        fetch(API_NETWORK_SAVE, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(credentials)
        })
        .then(response => {
            if (response.ok) {
                alert('Налаштування мережі збережено успішно!');
                ssidInput.value = '';
                passwordInput.value = '';
            } else {
                alert('Помилка при збереженні налаштувань мережі.');
            }
        })
        .catch(error => {
            console.error('Error:', error);
            alert('Сталася помилка при збереженні налаштувань мережі.');
        });
    });
    
    function getLogs(){
        fetch(API_DEBUG_LOGS)
        .then(response => {
            if (!response.ok) {
                console.error(`HTTP error! Status: ${response.status}`);
                return;
            }
            return response.json();
        })
        .then(data => {
            if (!data) return;
            if(data.logs != "")
                console.log(data.logs)

        })
    }
    function handleEspStatus(){
        fetch(API_ESP_STATUS)
        .then(response => {
            if (!response.ok) {
                console.error(`HTTP error! Status: ${response.status}`);
                return;
            }
            return response.json();
        })
        .then(data => {
            if (!data) return;
            espip.innerHTML = data.sStatus
            espip.innerHTML = data.ip
        })
    }
    // Функція для отримання статусу панелі
    function fetchPanelStatus() {
        fetch(API_PANEL_STATUS)
            .then(response => {
                if (!response.ok) {
                    console.error(`HTTP error! Status: ${response.status}`);
                    return;
                }
                return response.json();
            })
            .then(data => {
                if (!data) return;
                panelStatus.Started = data.started;
                panelStatus.AutoStart = data.autoStart;
                panelStatus.ManualMode = data.manualMode;             
                panelStatus.H_MoveDone = data.hMoveDone;
                panelStatus.H_Moving = data.hMoving;      
                panelStatus.V_MoveDone = data.vMoveDone;
                panelStatus.V_Moving = data.vMoving;       
                panelStatus.Sunlight_Founded = data.sunlightFounded; 
                panelStatus.H_Position = data.hPosition;
                panelStatus.V_Position = data.vPosition; 
                panelStatus.Velocity = data.velocity;
                panelStatus.SunSensor = data.sunSensor; 
            })
            .catch(error => {
                console.error('Error fetching panel status:', error);
            });
    }

    // Функція для отримання даних сенсорів
    function fetchSensorData() {
        fetch(API_SENSOR_DATA)
            .then(response => {
                if (!response.ok) {
                    console.error(`HTTP error! Status: ${response.status}`);
                    return;
                }
                return response.json();
            })
            .then(data => {
                if (!data) return;
                console.log(data);
                sensorData.voltage = data.voltage;
                sensorData.humidity = data.humidity;
                sensorData.temperature = data.temperature;
                sensorData.current = data.current;
                sensorData.power = data.power || (data.voltage * data.current);
            })
            .catch(error => {
                console.error('Error fetching sensor data:', error);
            });
    }

    // Функція для взаємодії з API контролю панелі
    function controlPanelAPI(command, value = null) {
        const now = new Date();

        const body = {
            start: command === "start",
            stop: command === "stop",
            autostartOn: command === "autostartOn",
            autostartOff: command === "autostartOff",
            remoteOn: command === "remoteOn",
            remoteOff: command === "remoteOff",
            reset_position: command === "reset_position",
            zero_position: command === "zero_position",
            direction: command === "direction" ? value : '',
            set_datetime: command === "set_datetime",
            weekday: command === "set_datetime" ? now.getDay() : 0,
            day: command === "set_datetime" ? now.getDate() : 0,
            month: command === "set_datetime" ? now.getMonth() : 0,
            year: command === "set_datetime" ? now.getFullYear() : 0,
            hours: command === "set_datetime" ? now.getHours() : 0,
            minutes: command === "set_datetime" ? now.getMinutes() : 0,
            seconds: command === "set_datetime" ? now.getSeconds() : 0,
            change_velocity: command === "change_velocity",
            velocity: command === "change_velocity" ? value : 0
        };

        fetch(API_CONTROL_PANEL, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(body)
        })
        .catch(error => {
            console.error(`Error sending command ${command}:`, error);
        });
    }

    // Функція для отримання мережевих налаштувань
    function fetchNetworkSettings() {
        fetch(API_NETWORK)
            .then(response => {
                if (!response.ok) {
                    console.error(`HTTP error! Status: ${response.status}`);
                    return;
                }
                return response.json();
            })
            .then(data => {
                if (!data) return;
                ssidInput.value = data.ssid || '';
                passwordInput.value = data.password || '';
            })
            .catch(error => {
                console.error('Error fetching network settings:', error);
            });
    }

    // Функція для перевірки статусу з'єднання з PLC
    function checkConnectionStatus() {
        fetch(API_PLC_CONNECTION_STATUS, 
            {
                method: "GET",
                headers: {
                    "Content-Type": "application/json",
                },
            }
        )
            .then(response => {
                if (!response.ok) {
                    console.error(`HTTP error! Status: ${response.status}`);
                    isPLCConnected = false;
                    return;
                }
                const contentType = response.headers.get('Content-Type');
                if (!contentType || !contentType.includes('application/json')) {
                    console.error("Expected JSON, got " + contentType);
                    isPLCConnected = false;
                    return;
                }
                return response.json();
            })
            .then(data => {
                if (!data) return;
                isPLCConnected = data.isPlcConnected;
            })
            .catch(error => {
                console.error('Error checking connection status:', error);
                isPLCConnected = false;
            });
    }

    fetchNetworkSettings();



    window.addEventListener('online', () => {
        console.log('You are online.');
        onlineStatus.classList.remove("offline");
        onlineStatus.classList.add("online");
        onlineStatus.textContent = "Інтернет доступний";
        sendSensorData(); 
    });

    window.addEventListener('offline', () => {
        console.log('You are offline.');
        onlineStatus.classList.remove("online");
        onlineStatus.classList.add("offline");
        onlineStatus.textContent = "Інтернет недоступний";
    });



});
