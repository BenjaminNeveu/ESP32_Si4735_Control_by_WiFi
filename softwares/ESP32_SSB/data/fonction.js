function mode() {
    var mode = document.getElementById("selectMode").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "mode", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('mode=' + mode);
}

function getFrequency() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function ()
    {
        if (this.readyState == 4 && this.status == 200)
        {
            document.getElementById("frequency").innerHTML = this.responseText;
        }
    };
    xhr.open("GET", "getFrequency", true);
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
    getSnr();
    getRssi();
    getAgc();
    getBfo();
}, 1000);

function freqButton() {
    var freq = prompt("Frequency (khz):");
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "frequency", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('frequency=' + freq);
    getFrequency();
}

function downFreq() {
    var stepF = document.getElementById("stepFreq").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "downFreq", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('stepF=' + stepF);
getFrequency();
}

function upFreq() {
    var stepF = document.getElementById("stepFreq").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "upFreq", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('stepF=' + stepF);
getFrequency();
}

function downBfo() {
    var stepB = document.getElementById("stepBfo").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "downBfo", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('stepB=' + stepB);
    getBfo();
}

function upBfo() {
    var stepB = document.getElementById("stepBfo").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "upBfo", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('stepB=' + stepB);
    getBfo();
}

function bandwidth() {
    var bandwidth = document.getElementById("selectBandw").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "bandwidth", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('bandwidth=' + bandwidth);
}

function ham() {
    var ham = document.getElementById("selectHam").value;
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "ham", true);
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send('ham=' + ham);
    getFrequency();
}

function agc() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "agc", true);
    xhr.send();
    getAgc();
}

function reset() {
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

