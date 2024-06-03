const progressLabel = document.getElementById("g-label");
const progressContainer = document.getElementById("g-container");
const progressBar = document.getElementById("g-progress");
const inputFile = document.getElementById("input-file");
const inputLabel = document.getElementById("input-label");

inputFile.onchange = (e) => {
    let fileName = {};
    fileName = e.target.value.split('\\').pop();
    console.log('File Name:' + fileName);
    inputLabel.innerHTML = fileName ? fileName : "No Firmware Selected";
};

const uploadFirmware = () => {
    // init progress bar & label
    progressLabel.innerHTML = '';
    progressBar.style.width = '0';

    progressLabel.style.visibility = "visible";
    progressContainer.style.visibility = "visible";
    if (inputFile.files.length === 0) {
        progressLabel.innerHTML = "<span style='color:red'>No File Selected!</span>";
        return;
    }

    let uploadData = new FormData();
    uploadData.append('FileName', inputFile.files[0].name);
    uploadData.append('FileSize', inputFile.files[0].size);
    uploadData.append('FileData', inputFile.files[0]);

    let xhr = new XMLHttpRequest();
    xhr.open("POST", './upload', true);
    xhr.onload = (e) => {
        if (this.status === 200 && (JSON.parse(this.responseText)).status === 'OK') {
            progressLabel.innerHTML = "<span style='color: #4cd24c'>Upload Success!</span>"
            console.log('upload success');
            return;
        }
        progressLabel.innerHTML = "<span style='color: red'>Upload Failed!</span>"
        console.log('upload failed');
    }
    xhr.upload.onprogress = (e) => {
        if (e.lengthComputable) {
            let percent = Math.floor(e.loaded / e.total * 100).toString();
            progressLabel.innerHTML = "<span>Upload Progress:" + percent + "%</span>";
            progressBar.style.width = percent + "%";
        }
    }
    xhr.send(uploadData);
}

const updateFirmware = () => {
    let xhr = new XMLHttpRequest();
    xhr.onload = function () {
        if (this.status === 200 && (JSON.parse(this.responseText)).status === 'OK') {
            console.log("send update instruction success");
        } else {
            console.log("send update instruction failed");
        }
    }
    xhr.open("GET", "./update.cgi", true);
    xhr.send();
}

