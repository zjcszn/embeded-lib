const ledCheckbox = document.getElementById('led-checkbox');
const ledParamsForm = document.getElementById('led-params-form');
const ledImage = document.getElementById('led-img');
const sensorCanvas = document.getElementById('canvas');
const ctx = sensorCanvas.getContext('2d');
const sensorCheckbox = document.getElementById('sensor-checkbox');
const inputOnTime = document.getElementById('on-time');
const inputOffTime = document.getElementById('off-time');
const inputCycles = document.getElementById('cycles');

inputOnTime.oninput = inputOffTime.oninput = inputCycles.oninput = (e) => {
    e.target.value = e.target.value.replace(/[-eE]/g, '');
}

const defaultValue = '0';
inputOnTime.onblur = inputOffTime.onblur = inputCycles.onblur = (e) => {
    if (!e.target.value) {
        e.target.value = defaultValue;
        e.target.style.color = '#999'
    } else {
        e.target.style.color = '#000'
    }
}

inputOnTime.onfocus = inputOffTime.onfocus = inputCycles.onfocus = (e) => {
    if (e.target.value === defaultValue) {
        e.target.value = '';
        e.target.style.color = '#000'
    }
}

ledCheckbox.onchange = (e) => {
    ledImage.src = ledCheckbox.checked ? './img/led_on.png' : './img/led_off.png';

    let xhr = new XMLHttpRequest();
    xhr.onload = function (e) {
        if (this.status === 200) {
            let json = JSON.parse(this.responseText);
            if (json.status === "No Permission") {
                window.location.replace("./index.shtml");
            }
        }
    }
    const urlParam = ledCheckbox.checked ? 'led=on' : "led=off";
    xhr.open("GET", "./led.cgi?" + urlParam, true);
    xhr.send();
}

let timerID;
sensorCheckbox.onchange = (e) => {
    if (sensorCheckbox.checked) {
        timerID = setInterval('drawCharts(ctx,config);', 100);
    } else {
        clearInterval(timerID);
    }
}


/* 将HTML表单转化为JSON */
const formToJson = (form) => {
    let keyPairs = {};
    let formData = new FormData(form);
    for (let entry of formData.entries()) {
        keyPairs[entry[0]] = entry[1];
    }
    return JSON.stringify(keyPairs);
}

const setLedConfig = () => {
    if (!inputOnTime.value || !inputOffTime.value || !inputCycles) {
        console.log('one of the params is empty');
        return;
    }
    let ledConfig = formToJson(ledParamsForm);
    let xhr = new XMLHttpRequest();
    xhr.open('POST', './led.cgi', true);
    console.log(ledConfig);
    xhr.send(ledConfig);
}

/* 初始化画布 */
const initCanvas = () => {
    /* 解决高分屏上画布绘制模糊的问题 */
    let dpr = window.devicePixelRatio;
    let {width: cssWidth, height: cssHeight} = sensorCanvas.getBoundingClientRect();
    sensorCanvas.style.width = sensorCanvas.width + "px";
    sensorCanvas.style.height = sensorCanvas.height + "px";

    sensorCanvas.width = dpr * cssWidth;
    sensorCanvas.height = dpr * cssHeight;
    ctx.scale(dpr, dpr);
}

/* 绘制坐标轴 */
const drawAxis = (ctx, config) => {
    ctx.save();

    const {origin, xAxis, yAxis} = config;
    const yAxisHeight = 5;
    ctx.lineWidth = 1;

    ctx.beginPath();
    ctx.strokeStyle = '#000000';
    // draw xAxis
    ctx.moveTo(origin.x, origin.y);
    ctx.lineTo(origin.x + xAxis.length, origin.y);
    // draw yAxis
    ctx.moveTo(origin.x, origin.y);
    ctx.lineTo(origin.x, origin.y - yAxis.length);
    ctx.stroke();
    ctx.closePath();

    ctx.beginPath();
    ctx.strokeStyle = 'rgba(175,175,175,0.5)';
    for (let i = 1; i < yAxis.length / yAxis.gap; i++) {
        ctx.moveTo(origin.x + xAxis.length, origin.y - yAxis.gap * i);
        ctx.lineTo(origin.x, origin.y - yAxis.gap * i);
        ctx.fillText((Math.ceil((i / (yAxis.length / yAxis.gap)) * 100)).toString(), origin.x - 20, origin.y - yAxis.gap * i);
    }
    for (let i = 1; i < xAxis.length / xAxis.gap; i++) {
        ctx.moveTo(origin.x + xAxis.gap * i, origin.y - yAxisHeight);
        ctx.lineTo(origin.x + xAxis.gap * i, origin.y);
    }
    ctx.stroke();
    ctx.closePath();

    ctx.restore();
}

/* 绘制线条 */
const drawLine = (ctx, config, points) => {
    ctx.save();

    ctx.lineWidth = 1;
    const {origin, xAxis, yAxis} = config;

    ctx.beginPath();
    ctx.strokeStyle = '#0064FFFF';

    ctx.moveTo(origin.x, origin.y - points[0].y * (yAxis.length / 100));
    for (let i = 1; i < points.length; i++) {
        ctx.lineTo(origin.x + i * xAxis.gap, origin.y - points[i].y * (yAxis.length / 100));
    }
    ctx.stroke();
    ctx.closePath();

    ctx.restore();
}

/* 画布属性配置 */
const config =
    {
        origin: {
            x: 25.5,
            y: 325.5
        },
        xAxis: {
            length: 400,
            gap: 20
        },
        yAxis: {
            length: 300,
            gap: 30
        }
    }

/* 容纳数据的数组 */
let points = [];
let max = 0, min = 100;


/* 绘制波形图（模拟数据）*/
const drawCharts = (ctx, config) => {
    let val = 40 + (Math.ceil(Math.random() * 100) % 31);
    if (val < min) min = val;
    if (val > max) max = val;
    const point = {y: val};
    if (points.length > 20) {
        points.shift();
    }
    points.push(point);
    ctx.clearRect(0, 0, sensorCanvas.width, sensorCanvas.height);
    drawAxis(ctx, config);
    drawLine(ctx, config, points);
    document.getElementById('max-val').innerHTML = max;
    document.getElementById('min-val').innerHTML = min;
}


initCanvas();
drawAxis(ctx, config);


