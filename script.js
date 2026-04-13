function login() {
    let user = document.getElementById("username").value;
    let pass = document.getElementById("password").value;

    // Hardcoded users
    if (user === "manager" && pass === "123") {
        window.location = "manager.html";
    }
    else if (user === "customer" && pass === "123") {
        window.location = "customer.html";
    }
    else if (user === "staff" && pass === "123") {
        window.location = "staff.html";
    }
    else {
        alert("Invalid Login");
    }
    return false;
}

// Customer booking
function bookStyle(style) {
    alert("Booking sent to staff for: " + style);
}

// Staff action
function respond(status) {
    alert("Request " + status);
}