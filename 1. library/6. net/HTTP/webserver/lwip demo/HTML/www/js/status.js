var respLabel = document.getElementById("resp-label");
const respError =  "<span style='color:red'>Server response error</span>";

function serialOpen() {
    var formData = new FormData(document.getElementById("config-form"));
    var urlParams = new URLSearchParams(formData);
    var req = new XMLHttpRequest();
    req.onreadystatechange = function(e) {
        if (this.readyState == XMLHttpRequest.DONE) {
            if (this.status == 200) {
                /* todo */
            } else {
                respLabel.innerHTML = respError;
            }
        }
    }
    req.open("GET", "./serial_open.cgi?" + urlParams, true);
    req.send();
}

function serialClose() {
    var req = new XMLHttpRequest();
    req.onreadystatechange = function(e) {
        if (this.readyState == XMLHttpRequest.DONE) {
            if (this.status == 200) {
                /* todo */
            } else {
                respLabel.innerHTML = respError;
            }
        }
    }
    req.open("GET", "./serial_close.cgi", true);
    req.send();
}

function sendMessage() {
    var msg = document.getElementById("send_msg").value;
    if (msg == '') {
        return;
    }

    var newStr = msg.toString().replace(/\n/g, "\r\n");
    var req = new XMLHttpRequest();
    req.onreadystatechange = function(e) {
        if (this.readyState == XMLHttpRequest.DONE) {
            if (this.status == 200) {
                document.getElementById("recv_msg").append("[OK  ] > "+newStr+'\n');
            } else {
                document.getElementById("recv_msg").append("[FAIL] > "+newStr+'\n');
            }
        }
    }
    req.open("POST", "./serial_trans.html", true);
    req.send(newStr);
}