function mode() {
    var mode = document.getElementById("selectMode").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "mode", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('mode=' + mode);
    getFrequence();
}

function getFrequence() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function ()
    {
        if (this.readyState == 4 && this.status == 200)
        {
            document.getElementById("frequence").innerHTML = this.responseText;
        }
    };
    xhr.open("GET", "getFrequence", true);
    xhr.send();
}

function getSnr() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function ()
    {
        if (this.readyState == 4 && this.status == 200)
        {
            document.getElementById("snr").innerHTML = this.responseText;
        }
    };
    xhr.open("GET", "getSnr", true);
    xhr.send();
}

function getRssi() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function ()
    {
        if (this.readyState == 4 && this.status == 200)
        {
            document.getElementById("rssi").innerHTML = this.responseText;
        }
    };
    xhr.open("GET", "getRssi", true);
    xhr.send();
}

function getAgc() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function ()
    {
        if (this.readyState == 4 && this.status == 200)
        {
            document.getElementById("agcText").innerHTML = this.responseText;
        }
    };
    xhr.open("GET", "getAgc", true);
    xhr.send();
}

function getBfo() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function ()
    {
        if (this.readyState == 4 && this.status == 200)
        {
            document.getElementById("bfo").innerHTML = this.responseText;
        }
    };
    xhr.open("GET", "getBfo", true);
    xhr.send();
}

setInterval(function getData() {
    getFrequence();
    getSnr();
    getRssi();
    getAgc();
    getBfo();
}, 1000);

function freqButton() {
    var freq = prompt("Fréquence :");
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "frequence", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('frequence=' + freq);
    getFrequence();
}

function downFreq() {
    var step = document.getElementById("stepFreq").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "downFreq", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('stepF=' + step);
    getFrequence();
}

function upFreq() {
    var step = document.getElementById("stepFreq").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "upFreq", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('stepF=' + step);
    getFrequence();
}

function downBfo() {
    var step = document.getElementById("stepBfo").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "downBfo", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('stepB=' + step);
    getBfo();
}

function upBfo() {
    var step = document.getElementById("stepBfo").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "upBfo", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('stepB=' + step);
    getBfo();
}



function bandwidth() {
    var filtre = document.getElementById("selectFiltre").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "bandwidth", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('filtre=' + filtre);
}

function ham() {
    var band = document.getElementById("selectBand").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "ham", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('band=' + band);
    getFrequence();
}

function agc() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "agc", true);
    xhr.send();
    getAgc();
}

function zero() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "reset", true);
    xhr.send();
    getBfo();
}
  
function openMenu(evt, select) {
    var i, tabcontent, tablinks;
    tabcontent = document.getElementsByClassName("tabcontent");
    for (i = 0; i < tabcontent.length; i++) {
      tabcontent[i].style.display = "none";
    }
    tablinks = document.getElementsByClassName("tablinks");
    for (i = 0; i < tablinks.length; i++) {
      tablinks[i].className = tablinks[i].className.replace(" active", "");
    }
    document.getElementById(select).style.display = "block";
    evt.currentTarget.className += " active";
}

