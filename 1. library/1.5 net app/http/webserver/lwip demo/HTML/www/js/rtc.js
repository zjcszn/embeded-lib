setInterval(getTime, 1000);

function getTime() {
    let xhr = new XMLHttpRequest();
    xhr.onload = function (e) {
        if (this.status === 200) {
            document.getElementById("rtc").textContent = this.responseText;
        }
    }
    xhr.open("GET", "/ssi/rtc.ssi", true);
    xhr.send();
}

function loginOut() {

}