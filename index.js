const rwfile = require("./rwfile.js");
const express = require('express');
const { engine } = require('express-handlebars');

const app = express();
const port = 8080;

app.engine('hbs', engine({
    extname: '.hbs',
    helpers: {
        toLowerCase: (str) => str.toLowerCase()
    }
}));
app.set('view engine', 'hbs');
app.set("views", "./views");

app.get('/', (req, res) => {
    res.render('home', { 'data': rwfile.readJSON()["data"] });
});

app.get('/getdata', (req, res) => {
    var type = {
        "1" : "RFID",
        "2" : "KEYPAD"
    };

    var status = {
        "1" : "Access",
        "2" : "Deny",
        "3" : "Warning",
        "4" : "Reset"
    };

    var today = new Date();
    var date = today.getFullYear() + '-' + (today.getMonth() + 1) + '-' + today.getDate();
    var time = today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();
    var dateTime = date + ' ' + time;

    var dataInput = req.query['dataESP'];

    var inputData = {
        "type": type[dataInput[0]],
        "status": status[dataInput[1]],
        "time": dateTime,
        "nod": parseInt(dataInput[2])
    };

    rwfile.writeJSON(inputData);
    res.send("Success");
});

app.listen(port, () => {
    console.log(`Started at http://localhost:${port}`);
});