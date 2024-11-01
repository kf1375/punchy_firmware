let step_respo_mode = false;
let step_respo_Start = false;
let infinite_start = false;
// let loggingEnabled = false;
let singleSpeed = 0;
let infiniteSpeed = 0;

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
    document.getElementById('settingsDialog').style.display = 'block';
}

// Close settings dialog
function closeSettingsDialog() {
    document.getElementById('settingsDialog').style.display = 'none';
}

// Open login dialog
function openLoginDialog() {
    document.getElementById('loginDialog').style.display = 'block';
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
    tablinks = document.getElementsByClassName("tablink");
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

    if (infinite_start) {
        toggleCheckbox("infinite_turn_start");
        // document.getElementById(
        //     "infinite_start_stop_button"
        // ).style.backgroundColor = "#f14907";
        // document.getElementById("infinite_start_stop_button").style.borderColor =
        //     "#f14907";
    } else {
        toggleCheckbox("infinite_turn_stop");
        // document.getElementById(
        //     "infinite_start_stop_button"
        // ).style.backgroundColor = "#e3af88";
        // document.getElementById("infinite_start_stop_button").style.borderColor =
        //     "#e3af88";
    }
}

// function to toggle Half and Full Button
function setActiveButton(selectedButtonId) {
    if (selectedButtonId === "half") {
        toggleCheckbox("half");
        // document.getElementById("half").style.backgroundColor = "#f14907";
        // document.getElementById("half").style.borderColor = "#f14907";
        // document.getElementById("full").style.backgroundColor = "#e3af88";
        // document.getElementById("full").style.borderColor = "#e3af88";
    }
    if (selectedButtonId === "full") {
        toggleCheckbox("full");
        // document.getElementById("full").style.backgroundColor = "#f14907";
        // document.getElementById("full").style.borderColor = "#f14907";
        // document.getElementById("half").style.backgroundColor = "#e3af88";
        // document.getElementById("half").style.borderColor = "#e3af88";
    }
}
