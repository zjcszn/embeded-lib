const fileTable = document.getElementById('file-explorer');
const pageIndicate = document.getElementById('page-ind');
const tableOpFirst = document.getElementById('op-first');
const tableOpPre = document.getElementById('op-pre');
const tableOpNext = document.getElementById('op-next');
const tableOpLast = document.getElementById('op-last');


const tableOpControl = () => {
    let enable = [];
    let disable = [];
    if (pageCount <= 1) {
        disable.push(tableOpFirst, tableOpLast, tableOpNext, tableOpPre);
    } else {
        if (pageIndex === 0) {
            disable.push(tableOpPre);
            enable.push(tableOpFirst, tableOpLast, tableOpNext);
        } else if (pageIndex === pageCount - 1) {
            disable.push(tableOpNext);
            enable.push(tableOpFirst, tableOpLast, tableOpPre);
        } else {
            enable.push(tableOpFirst, tableOpLast, tableOpNext, tableOpPre);
        }
    }
    for (let op of enable) {
        op.setAttribute("style", "pointer-events:auto;opacity:1;color:;");
    }
    for (let op of disable) {
        op.setAttribute("style", "pointer-events:none;opacity:0.5;color:black;");
    }
}

const tableDestroy = () => {
    let tBodies = fileTable.getElementsByTagName('tbody');
    for (let i = tBodies.length - 1; i >= 0; i--) {
        fileTable.removeChild(tBodies[i]);
    }
}


const recordsPerPage = 10;

let pageIndex = 0;
let tableData = [];
let pageCount = Math.ceil(tableData.length / recordsPerPage);


const tableUpdate = (pageIndex) => {
    tableDestroy();
    let tBody = fileTable.createTBody();
    let recordsLeft = tableData.length - pageIndex * recordsPerPage;
    let recordsToInsert = recordsLeft < recordsPerPage ? recordsLeft : recordsPerPage;
    for (let i = 0; i < recordsToInsert; i++) {
        let newRow = tBody.insertRow();
        newRow.insertCell().innerHTML = tableData[pageIndex * recordsPerPage + i].idx;
        newRow.insertCell().innerHTML = tableData[pageIndex * recordsPerPage + i].name;
        newRow.insertCell().innerHTML = tableData[pageIndex * recordsPerPage + i].size;
        newRow.insertCell().innerHTML = tableData[pageIndex * recordsPerPage + i].date;
    }
    pageIndicate.innerHTML = (pageIndex + 1) + '/' + pageCount;
    tableOpControl();
}


const firstPage = () => {
    pageIndex = 0;
    tableUpdate(0);
}

const lastPage = () => {
    pageIndex = pageCount - 1;
    tableUpdate(pageIndex);
}

const nextPage = () => {
    if (pageIndex !== pageCount - 1) {
        pageIndex++;
        tableUpdate(pageIndex);
    }
}

const prePage = () => {
    if (pageIndex !== 0) {
        pageIndex--;
        tableUpdate(pageIndex);
    }
}

const tableDataGenerate = () => {
    for (let i = 0; i < 108; i++) {
        let date = new Date();
        tableData.push(
            {
                idx: (i + 1).toString(),
                name: "<a href='./update" + i + ".bin' download='update" + i + ".bin'>update" + i + ".bin</a>",
                size: Math.ceil(Math.random() * 100000).toString(),
                date: date.toLocaleString(),
            }
        )
    }
    pageCount = Math.ceil(tableData.length / recordsPerPage);
    tableUpdate(0);
}

tableDataGenerate();


