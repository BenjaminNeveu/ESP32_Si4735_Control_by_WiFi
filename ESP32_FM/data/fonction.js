function mode() {
    var mode = document.getElementById("mode").value;
    if(mode == "fm"){
        $("#button").children("#bandwidth").remove();
        document.getElementById("info").innerHTML = "MHz";
    }else{
        document.getElementById("info").innerHTML = "KHz";
        $( "#button" ).append( $( "<button onclick='bandwidth()' id='bandwidth'>Filtre</button>" ) );
    }
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "mode", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('mode=' + mode);

    getFrequence();
}

function getFrequence() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function()
    {
        if(this.readyState == 4 && this.status == 200)
        {
            document.getElementById("frequence").innerHTML = this.responseText;
        }
    };
    xhr.open("GET", "getFrequence", true);
    xhr.send();
}

function getSnr() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function()
    {
        if(this.readyState == 4 && this.status == 200)
        {
            document.getElementById("snr").innerHTML = this.responseText;
        }
    };
    xhr.open("GET", "getSnr", true);
    xhr.send();
}

function getRssi() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function()
    {
        if(this.readyState == 4 && this.status == 200)
        {
            document.getElementById("rssi").innerHTML = this.responseText;
        }
    };
    xhr.open("GET", "getRssi", true);
    xhr.send();
}

function getPilot() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function()
    {
        if(this.readyState == 4 && this.status == 200)
        {
            document.getElementById("pilot").innerHTML = this.responseText;
        }
    };
    xhr.open("GET", "getPilot", true);
    xhr.send();
}

/*
function getStatus() {
    getFrequence();
    getPilot();
    getSnr();
    getRssi();
}
*/

setInterval(function getData() {
    getFrequence();
    getPilot();
    getSnr();
    getRssi();
}, 1000);

function freqButton() {
    var freq = prompt("Fréquence :");
    if (document.getElementById("mode").value == "fm" ){
        freq = freq * 100;
    }
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "frequence", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('frequence=' + freq);
    getFrequence();
}

function downButton() {
    var step = document.getElementById("step").value;
    if (document.getElementById("mode").value == "fm" ){
    step = step * 100;    
    }
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "down", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('step=' + step);
    getFrequence();
}

function upButton() {
    var step = document.getElementById("step").value;
    if (document.getElementById("mode").value == "fm" ){
        step = step * 100;    
    }
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "up", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('step=' + step);
    getFrequence();
}

function seekDown() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "seekdown", true);
    xhr.send();
    getFrequence();
}

function seekUp() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "seekup", true);
    xhr.send();
    getFrequence();
}

function bandwidth() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "bandwidth", true);
    xhr.send();
}