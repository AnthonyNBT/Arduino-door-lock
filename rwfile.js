const fs = require('fs');

function readJSON() {
    let rawdata = fs.readFileSync('data.json');
    let data = JSON.parse(rawdata);
    return data;
}

function writeJSON(input) {
    var curData = readJSON();
    curData["data"].push(input);
    curData = JSON.stringify(curData);
    fs.writeFileSync('data.json', curData);
}

module.exports = {readJSON , writeJSON};