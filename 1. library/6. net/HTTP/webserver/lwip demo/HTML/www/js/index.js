const loginForm = document.getElementById('login-form')
const loginMsg = document.getElementById('login-msg');
const loginUser = document.getElementById('usr');
const loginPass = document.getElementById('pwd');


loginUser.onkeydown = (e) => {
    if (e.code === 'Enter') {
        loginPass.focus();
    }
}

loginPass.onkeydown = (e) => {
    if (e.code === 'Enter') {
        loginSubmit();
    }
}

const loginSubmit = () => {
    if (!loginUser.value || !loginPass.value) {
        loginMsg.innerHTML = 'username or password is empty!';
        return;
    }

    let object = {};
    let formData = new FormData(loginForm);
    for (let entry of formData.entries()) {
        object[entry[0]] = entry[1];
    }
    let loginData = JSON.stringify(object);
    console.log(loginData);

    let xhr = new XMLHttpRequest();
    xhr.open('POST', './login.cgi', true);
    xhr.onload = function (e) {
        if (this.status === 200) {
            let resp = JSON.parse(this.responseText);
            if (resp.status === 'OK') {
                window.location.href = resp.url;
            } else {
                loginMsg.innerHTML = 'username or password is invalid!'
            }
        } else {
            loginMsg.innerHTML = 'network error!'
        }
    }

    xhr.send(loginData);
}

const loginReset = () => {
    loginMsg.innerHTML = '';
}
