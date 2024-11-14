let infinite_start = false;
let singleSpeed, maxSingleSpeed;
let infiniteSpeed, maxInfiniteSpeed;

// Function to get the current speeds values
function fetchSpeeds() {
    fetch('/speeds')
    .then(response => response.json())
    .then(data => {
        singleSpeed = data.single_speed;
        document.getElementById(`single-speed`).value = singleSpeed;
        document.getElementById(`single-speed-value`).textContent = singleSpeed;
        document.getElementById(`current-single-speed`).textContent = singleSpeed;

        maxSingleSpeed = data.max_single_speed;
        document.getElementById(`max-single-speed`).value = maxSingleSpeed;
        document.getElementById(`max-single-speed-value`).textContent = maxSingleSpeed;
        document.getElementById(`current-max-single-speed`).textContent = maxSingleSpeed;

        infiniteSpeed = data.infinite_speed;
        document.getElementById(`infinite-speed`).value = infiniteSpeed;
        document.getElementById(`infinite-speed-value`).textContent = infiniteSpeed;
        document.getElementById(`current-infinite-speed`).textContent = singleSpeed;

        maxInfiniteSpeed = data.max_infinite_speed;
        document.getElementById(`max-infinite-speed`).value = maxInfiniteSpeed;
        document.getElementById(`max-infinite-speed-value`).textContent = maxInfiniteSpeed;
        document.getElementById(`current-max-infinite-speed`).textContent = maxInfiniteSpeed;
    })
    .catch(error => {
    console.error('Error fetching speeds values', error);
    });
}

fetchSpeeds();

// Function to update speed values
function updateSpeed(type) {
    let speedValue = document.getElementById(`${type}-speed`).value;
    document.getElementById(`${type}-speed-value`).textContent = speedValue;
    document.getElementById(`current-${type}-speed`).textContent = speedValue;
    if (type === "single") {
        sendSliderValueToESP32("setSingleSpeed", speedValue);
        singleSpeed = speedValue;
    } else if (type === "infinite") {
        sendSliderValueToESP32("setInfiniteSpeed", speedValue);
        infiniteSpeed = speedValue;
    }
}

// Function to update max speed values
function updateMaxSpeed(type) {
    let maxSpeedValue = document.getElementById(`max-${type}-speed`).value;
    document.getElementById(`max-${type}-speed-value`).textContent = maxSpeedValue;
    document.getElementById(`current-max-${type}-speed`).textContent = maxSpeedValue;
    document.getElementById(`${type}-speed`).max = maxSpeedValue;
}

// Open settings dialog
function openSettingsDialog() {
    document.getElementById('settingsDialog').style.display = 'flex';
}

// Close settings dialog
function closeSettingsDialog() {
    document.getElementById('settingsDialog').style.display = 'none';
}

// Open login dialog
function openLoginDialog() {
    document.getElementById('loginDialog').style.display = 'flex';
}

// Close settings dialog
function closeLoginDialog() {
    document.getElementById('loginDialog').style.display = 'none';
}

// Close the modal if user clicks outside of it
window.onclick = function(event) {
    let modal = document.getElementById('settingsDialog');
    if (event.target == modal) {
        closeSettingsDialog();
    }
    modal = document.getElementById('loginDialog');
    if (event.target == modal) {
        closeLoginDialog();
    }
};

// Tab functionality
function openTab(evt, tabName) {
    var i, x, tablinks;
    x = document.getElementsByClassName("tab-content");
    for (i = 0; i < x.length; i++) {
        x[i].style.display = "none";
    }
    tablinks = document.getElementsByClassName("tab-link");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" w3-white", "");
    }
    document.getElementById(tabName).style.display = "flex";
    evt.currentTarget.className += " w3-white";
}

function sendSliderValueToESP32(endpoint, value) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/" + endpoint + "?value=" + value, true);
    xhr.onreadystatechange = function () {
        if (xhr.readyState == 4 && xhr.status == 200) {
            console.log("Response: " + xhr.responseText); // Optional: Handle response
        }
    };
    xhr.send();
}

function toggleCheckbox(x) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/" + x, true);
    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            console.log("Response: " + xhr.responseText); // Optional: Handle response
            // if (loggingEnabled) {
                // if (x === 'single_turn_start' || x === 'infinite_turn_start' || x === 'infinite_turn_stop') {
                //     logData.push({"timestamp": getCurrentFormattedTime(), "command": x, "single_speed": singleSpeed, "infinite_speed": infiniteSpeed});
                // }
            // }
        }
    };
    xhr.send();
}

// function to enable the toggling of the start/stop button
function toggleStart_Stop_infinite() {
    infinite_start = !infinite_start;
    const infiniteTurnButton = document.getElementById('infiniteTurnButton')
    if (infinite_start) {
        toggleCheckbox("infinite_turn_start");
        infiniteTurnButton.style.backgroundColor = "#c1121f";
        infiniteTurnButton.textContent = "Stop";
    } else {
        toggleCheckbox("infinite_turn_stop");
        infiniteTurnButton.style.backgroundColor = "#28A745";
        infiniteTurnButton.textContent = "Start Infinite Turn";
    }
}

// function to toggle Half and Full Button
function setActiveButton(mode) {
    const halfButton = document.getElementById('halfButton');
    const fullButton = document.getElementById('fullButton');
    console.log(mode);
    if (mode === "half") {
        toggleCheckbox("half");
        halfButton.disabled = true;
        fullButton.disabled = false;
    }
    if (mode === "full") {
        toggleCheckbox("full");
        halfButton.disabled = false;
        fullButton.disabled = true;
    }
}
