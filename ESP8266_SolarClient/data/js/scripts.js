document.addEventListener('DOMContentLoaded', () => {

    // API Endpoints
    const API_PANEL_STATUS = "/api/panel/data";
    const API_NETWORK = "/api/network/data";
    const API_NETWORK_SAVE = "/api/network/save";
    const API_SENSOR_DATA = "/api/sensor_data"; 
    const API_PLC_CONNECTION_STATUS = "/api/plc/connectionstatus";
    const API_DEBUG_LOGS = "/logs"
    const API_SERVER_DATA = "http://185.237.204.60:5000/api/period_sensor_data"; 

    // Navigation Elements
    const navButtons = document.querySelectorAll('.nav-button:not(.menu)');
    const menuButton = document.getElementById('menu-button');
    const navBtnsContainer = document.getElementById('nav-btns');

    // Panels
    const currentPanel = document.getElementById('current-panel');
    const chartsPanel = document.getElementById('charts-panel'); 

    // Status Elements
    const currentTimeElement = document.getElementById('current-time');
    const onlineStatus = document.getElementById('online-status');

    // Display Elements
    const state_value = document.getElementById('state-value');
    const mode_value = document.getElementById('mode-value');
    const voltage_value = document.getElementById('voltage-value');
    const current_value = document.getElementById('current-value');
    const temperature_value = document.getElementById('temperature-value');
    const humidity_value = document.getElementById('humidity-value');
    const horizontal_position = document.getElementById('horizontal-value');
    const vertical_position = document.getElementById('vertical-value');
    const SunSensor = document.getElementById('sunsensor-value');

    // Network Inputs
    const saveNetworkButton = document.getElementById('save-network-button');
    const ssidInput = document.getElementById('ssid-input');
    const passwordInput = document.getElementById('password-input');

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
    let ChartsInitialized = false;
    let temperatureChartInstance, currentChartInstance, voltageChartInstance, powerChartInstance;
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
        if(isPLCConnected)
        {
            fetchPanelStatus();
        }
    }

    // Initial Calls and Intervals
    setInterval(updateTime, 60000);
    setInterval(Update, 1000);
    setInterval(getLogs, 1000);
    setInterval(fetchSensorData,1500);
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
                targetSection.style.display = "block";

            }

        });
    });
    initializeCharts();
    // Обробка кнопки меню
    menuButton.addEventListener('click', () => {
        navBtnsContainer.classList.toggle('open');
    });

    // Функція для відображення даних
    function displayData() {
        state_value.textContent = panelStatus.Started ? "Запущений" : "Зупинений";
        mode_value.textContent = panelStatus.ManualMode ?  "Ручний" : "Авто";
        voltage_value.textContent = sensorData.voltage;
        current_value.textContent = sensorData.current;
        temperature_value.textContent = sensorData.temperature;
        humidity_value.textContent = sensorData.humidity;
        horizontal_position.textContent = panelStatus.H_Position;
        vertical_position.textContent = panelStatus.V_Position;
        SunSensor.textContent = panelStatus.SunSensor;

        // Обчислення потужності
        sensorData.power = sensorData.voltage * sensorData.current;
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
                sensorData.voltage = data.voltage;
                sensorData.humidity = data.humidity;
                sensorData.temperature = data.temperature;
                sensorData.current = data.current;
                sensorData.power = data.power || (data.voltage * data.current);
                updateCharts(); 
            })
            .catch(error => {
                console.error('Error fetching sensor data:', error);
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
            sensorData.voltage = data.voltage;

        })
    }
    function initializeCharts() {
        const ctxTemp = document.getElementById('temperatureChart').getContext('2d');
        const ctxCurrent = document.getElementById('currentChart').getContext('2d');
        const ctxVoltage = document.getElementById('voltageChart').getContext('2d');
        const ctxPower = document.getElementById('powerChart').getContext('2d');
        if (ChartsInitialized)
            return;
        
        ChartsInitialized = true;
        temperatureChartInstance = new Chart(ctxTemp, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Температура (°C)',
                    data: [],
                    borderColor: 'rgba(255, 99, 132, 1)',
                    backgroundColor: 'rgba(255, 99, 132, 0.2)',
                    fill: true,
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    x: {
                        title: {
                            display: true,
                            text: 'Час'
                        }
                    },
                    y: {
                        title: {
                            display: true,
                            text: 'Температура (°C)'
                        }
                    }
                }
            }
        });

        currentChartInstance = new Chart(ctxCurrent, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Струм (A)',
                    data: [],
                    borderColor: 'rgba(54, 162, 235, 1)',
                    backgroundColor: 'rgba(54, 162, 235, 0.2)',
                    fill: true,
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    x: {
                        title: {
                            display: true,
                            text: 'Час'
                        }
                    },
                    y: {
                        title: {
                            display: true,
                            text: 'Струм (A)'
                        }
                    }
                }
            }
        });

        voltageChartInstance = new Chart(ctxVoltage, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Напруга (V)',
                    data: [],
                    borderColor: 'rgba(255, 206, 86, 1)',
                    backgroundColor: 'rgba(255, 206, 86, 0.2)',
                    fill: true,
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    x: {
                        title: {
                            display: true,
                            text: 'Час'
                        }
                    },
                    y: {
                        title: {
                            display: true,
                            text: 'Напруга (V)'
                        }
                    }
                }
            }
        });

        powerChartInstance = new Chart(ctxPower, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Потужність (W)',
                    data: [],
                    borderColor: 'rgba(75, 192, 192, 1)',
                    backgroundColor: 'rgba(75, 192, 192, 0.2)',
                    fill: true,
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    x: {
                        title: {
                            display: true,
                            text: 'Час'
                        }
                    },
                    y: {
                        title: {
                            display: true,
                            text: 'Потужність (W)'
                        }
                    }
                }
            }
        });

        fetchAndPopulateCharts();
        setInterval(fetchAndPopulateCharts, 1000);
    }

    function fetchAndPopulateCharts() {
        fetch(`${API_SERVER_DATA}?period=week`)
            .then(response => {
                if (!response.ok) {
                    console.error(`HTTP error! Status: ${response.status}`);
                    return;
                }
                return response.json();
            })
            .then(data => {
                if (!data) return;
                const labels = data.map(entry => new Date(entry.created_at).toLocaleTimeString());
                const temperatures = data.map(entry => entry.temperature);
                const currents = data.map(entry => entry.current);
                const voltages = data.map(entry => entry.voltage);
                const powers = data.map(entry => entry.power || (entry.voltage * entry.current));

                updateChart(temperatureChartInstance, labels, temperatures);
                updateChart(currentChartInstance, labels, currents);
                updateChart(voltageChartInstance, labels, voltages);
                updateChart(powerChartInstance, labels, powers);
            })
            .catch(error => {
                console.error('Error fetching data for charts:', error);
            });
    }

    function updateChart(chart, labels, data) {
        chart.data.labels = labels;
        chart.data.datasets[0].data = data;
        chart.update();
    }

    function updateCharts() {
        const now = new Date().toLocaleTimeString();
        // Додаємо нові дані до графіків
        if (temperatureChartInstance) {
            temperatureChartInstance.data.labels.push(now);
            temperatureChartInstance.data.datasets[0].data.push(sensorData.temperature);
            if (temperatureChartInstance.data.labels.length > 50) { // Обмеження на кількість точок
                temperatureChartInstance.data.labels.shift();
                temperatureChartInstance.data.datasets[0].data.shift();
            }
            temperatureChartInstance.update();
        }
        if (currentChartInstance) {
            currentChartInstance.data.labels.push(now);
            currentChartInstance.data.datasets[0].data.push(sensorData.current);
            if (currentChartInstance.data.labels.length > 50) {
                currentChartInstance.data.labels.shift();
                currentChartInstance.data.datasets[0].data.shift();
            }
            currentChartInstance.update();
        }
        if (voltageChartInstance) {
            voltageChartInstance.data.labels.push(now);
            voltageChartInstance.data.datasets[0].data.push(sensorData.voltage);
            if (voltageChartInstance.data.labels.length > 50) {
                voltageChartInstance.data.labels.shift();
                voltageChartInstance.data.datasets[0].data.shift();
            }
            voltageChartInstance.update();
        }
        if (powerChartInstance) {
            powerChartInstance.data.labels.push(now);
            powerChartInstance.data.datasets[0].data.push(sensorData.power);
            if (powerChartInstance.data.labels.length > 50) {
                powerChartInstance.data.labels.shift();
                powerChartInstance.data.datasets[0].data.shift();
            }
            powerChartInstance.update();
        }
    }


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

    // ---------------- Додано для графіків ----------------

    // Перевірка, чи існує панель графіків перед ініціалізацією
    if (chartsPanel) {
        initializeCharts();
    }

});
