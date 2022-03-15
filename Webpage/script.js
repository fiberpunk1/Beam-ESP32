/******
 * file upload and list
 */
function isTextFile(path){
    var ext = /(?:\.([^.]+))?$/.exec(path)[1];
    if(typeof ext !== undefined){
      switch(ext){
        case "txt":
        case "htm":
        case "html":
        case "js":
        case "json":
        case "c":
        case "h":
        case "cpp":
        case "css":
        case "xml":
          return true;
      }
    }
    return false;
}

function createTreeLeaf(path, name, size){
    var td = document.createElement("td");
    td.id = (((path == "/")?"":path)+"/"+name).toLowerCase();
    td.innerHTML = name.toLowerCase();
    return td;
}
function convertToShortName(fullName){
    var f_name = fullName.replace('/','');
    var base_name = f_name.replace(/\.[^/.]+$/,'');
    if(base_name.length>=6){
        base_name = base_name.substring(0,6) + "~1";
    }else{
        base_name = base_name + "~1";
    }
    var full_name = base_name + ".GCO";
    return full_name;

}

function makeid(length) {
    var result           = '';
    var characters       = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
    var charactersLength = characters.length;
    for ( var i = 0; i < length; i++ ) {
      result += characters.charAt(Math.floor(Math.random() * charactersLength));
   }
   return result;
}

function addList(parent, path, items){
    var ll = items.length;
    for(var i = 0; i < ll; i++){
        var list = document.createElement("tr");
        parent.appendChild(list);
        var item = items[i];
        var itemEl;
        if(item.type === "file"){
            if(item.name.endsWith(".gcode")){
                itemEl = createTreeLeaf(path, item.name, item.size);
                console.log(item.name);
                list.appendChild(itemEl);
                var td_print = document.createElement("td");
                td_print.innerHTML = "<button class='btn btn-default btn-xs' type='button'><span class='glyphicon glyphicon-print' aria-hidden='true'></span>print</button>";
                td_print.className = item.name;
                td_print.onclick = function(){
                    xmlHttp = new XMLHttpRequest();
                    xmlHttp.onreadystatechange = function(){
                        var resp = xmlHttp.responseText;
                        if(resp.startsWith("NO")||resp.startsWith("SD")||resp.startsWith("PRINTER")){
                            alert(resp);
                        }
                    }
                    var short_name = "/"+convertToShortName(this.className);
                    xmlHttp.open("GET", "/print?filename="+short_name, true);
                    xmlHttp.send();
                };
                list.appendChild(td_print);


                var td = document.createElement("td");
                td.innerHTML = "<button class='btn btn-default btn-xs' type='button'><span class='glyphicon glyphicon-trash' aria-hidden='true'></span>delete</button>";
                td.className = item.name;
                td.onclick = function(){
                    console.log("delete: %s", this.className);
                    xmlHttp = new XMLHttpRequest();
                    xmlHttp.onload = function(){
                       creatTree();     
                    }
                    xmlHttp.onreadystatechange = function(){
                        var resp = xmlHttp.responseText;
                        if(resp.startsWith("NO")||resp.startsWith("SD")||resp.startsWith("PRINTER")){
                            alert(resp);
                        }
                    }
                    xmlHttp.open("GET", "/remove?path="+this.className, true);
                    xmlHttp.send();
                };
                list.appendChild(td);
            }


         

           
        } 
        
    }

}

function httpGet(parent, path){
    xmlHttp = new XMLHttpRequest(parent, path);
    xmlHttp.onreadystatechange = function(){
        var resp = xmlHttp.responseText;
        if (xmlHttp.readyState == 4){
            //clear loading
            if(xmlHttp.status == 200) addList(parent, path, JSON.parse(xmlHttp.responseText));
        }
        if(resp.startsWith("NO")||resp.startsWith("SD")||resp.startsWith("PRINTER")){
            alert(resp);
        }
    }
    xmlHttp.open("GET", "/list?dir="+path, true);
    xmlHttp.send(null);
    //start loading
}

function creatTree(){
    var fileListM = document.getElementById("file-list-frame");
    while(fileListM.hasChildNodes()){
        fileListM.removeChild(fileListM.lastChild)
    }

    httpGet(fileListM, "/");
}

const updateButton = document.getElementById("update-list");
const unmountButton = document.getElementById("release-sd");
const mountButton = document.getElementById("get-sd");
const uploadButton = document.getElementById("upload-file");

updateButton.onclick = () => {
    creatTree();
}

unmountButton.onclick = () => {
    var tt_url = "/operate?op=RELEASESD";
    xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function(){
        var resp = xmlHttp.responseText;
        if(resp.startsWith("NO")||resp.startsWith("SD")||resp.startsWith("PRINTER")){
            alert(resp);
        }
    }
    xmlHttp.open("GET", tt_url);
    xmlHttp.send();
}

mountButton.onclick = () => {
    var tt_url = "/operate?op=GETSD";
    xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function(){
        var resp = xmlHttp.responseText;
        if(resp.startsWith("NO")||resp.startsWith("SD")||resp.startsWith("PRINTER")){
            alert(resp);
        }
    }
    xmlHttp.open("GET", tt_url);
    xmlHttp.send();
}

function uploadComplete(evt) {
    var progressBar = document.getElementById("progressbar");
    progressBar.value = 0; 
    alert("Upload finish!");
    creatTree();
}
//上传失败
function uploadFailed(evt) {
    alert("Upload failed!");
}

function progressFunction(evt) {
    var progressBar = document.getElementById("progressbar");
    if (evt.lengthComputable) {//
        progressBar.max = evt.total;
        progressBar.value = evt.loaded;  
    }
}

// function httpPostProcessRequest(){
//     if (xmlHttp.readyState == 4){
//       if(xmlHttp.status != 200) alert("ERROR["+xmlHttp.status+"]: "+xmlHttp.responseText);
      
//     }
//   }

uploadButton.onclick = () => {
    var input = document.getElementById("choose-file");
    if(input.files.length === 0){
        return;
    }
    xmlHttp = new XMLHttpRequest();
    // xmlHttp.onreadystatechange = httpPostProcessRequest;
    xmlHttp.onload = uploadComplete;
    xmlHttp.onerror = uploadFailed;
    xmlHttp.upload.onprogress = progressFunction;
    var formData = new FormData();
    var random_start = makeid(3);
    var savePath = "/"+random_start+"-"+input.files[0].name;
    formData.append("data", input.files[0], savePath);
    xmlHttp.open("POST", "/edit");
    xmlHttp.send(formData);
}


/**
 * Gcode send and read, text display
 */
const sendGcodedButton = document.getElementById("btn-send-gcode");
const sendCleardButton = document.getElementById("btn-send-clear");
const sendGcdoeInput = document.getElementById("gcode-lineedit");



sendGcodedButton.onclick = () => {
    var cmdLineEdit = document.getElementById("gcode-lineedit");
    var cmd = cmdLineEdit.value;
    var tt_url = "/gcode?gc="+cmd;
    xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function(){
        var resp = xmlHttp.responseText;
        if(resp.startsWith("NO")||resp.startsWith("SD")||resp.startsWith("PRINTER")){
            alert(resp);
        }
    }
    xmlHttp.open("GET", tt_url);
    xmlHttp.send();

}
sendCleardButton.onclick = () => {
    document.getElementById("serial-console").value = "clean\r\n";

}
//enter button
sendGcdoeInput.onkeydown = function(e){
    if(e.keyCode == 13){
        var cmd = this.value;
        var tt_url = "/gcode?gc="+cmd;
        xmlHttp = new XMLHttpRequest();
        xmlHttp.open("GET", tt_url);
        xmlHttp.send();
    }
}

var source = new EventSource('/events');
source.addEventListener('gcode_cli', function(e) {
  //console.log("gyro_readings", e.data);
  var obj = e.data;
  var scrollText = document.getElementById("serial-console");
  obj += "\r\n";
  scrollText.value += obj;
  scrollText.scrollTop = scrollText.scrollHeight;

  //get file name, progress and tempture
   var reg_t = /T:([0-9]*\.[0-9]*) *\/([0-9]*\.[0-9]*)/g;
   var reg_b = /B:([0-9]*\.[0-9]*) *\/([0-9]*\.[0-9]*)/g;
   var reg_p = /SD printing byte ([0-9]*)\/([0-9]*)/g;
   var reg_af = /Current file:([\S\s]*).GCO/g;
   var reg_f = /Current file:([\S\s]*).GCO ([\S\s]*).gcode/g;
   var reg_end = /Finish/g;

   var heater = obj.match(reg_t);
   if(heater){
        var header_tmp = (RegExp.$1);
        var header_targ = (RegExp.$2);
        var display_header = header_tmp + "/" + header_targ;
        var header_ele = document.getElementById("hothead-display");
        header_ele.innerHTML = display_header;
   }

   var beder = obj.match(reg_b);
   if(beder){
        var beder_tmp = (RegExp.$1);
        var beder_targ = (RegExp.$2);
        var display_bed = beder_tmp + "/" + beder_targ;
        var bed_ele = document.getElementById("hotbed-display");
        bed_ele.innerHTML = display_bed;

    }

    var prog = obj.match(reg_p);
    if(prog){
        var current_line = parseFloat(RegExp.$1);
        var total_lines = parseFloat(RegExp.$2);
        var percent = Math.round((current_line/total_lines)*100);
        var percent_ele = document.getElementById("print-progess");
        percent_ele.innerHTML = percent.toString();
    }

    var print_file = obj.match(reg_f);
    if(print_file){
        var printFileElement = document.getElementById("print-file");
        printFileElement.innerHTML = RegExp.$1 + ".GCO";
    }

    var print_ful_name = obj.match(reg_af);
    if(print_ful_name){
        var printFileElement = document.getElementById("print-file");
        printFileElement.innerHTML = RegExp.$2 + ".gcode";
    }

    var b_finish = obj.match(reg_end);
    var b_done = obj.match(/Done/g);
    if(b_finish || b_done){
        var percent_ele = document.getElementById("print-progess");
        var printFileElement = document.getElementById("print-file");
        percent_ele.innerHTML = 0;
        printFileElement.innerHTML = "no file";
        alert("Print job finish!");
    }

}, false);

var int=self.setInterval("updateStatus()",8000);
function updateStatus(){
    var tt_url = "/status";
    xmlHttp = new XMLHttpRequest();
    xmlHttp.open("GET", tt_url);
    xmlHttp.send();
}

/***
 * Gcode control 
 * 
 */

function getRadioValue(){
    var radios = document.getElementsByName("stepRadio");
    var value = 0;
    for(var i = 0; i<radios.length; i++){
        if(radios[i].checked == true){
            value = radios[i].value;
        }
    }
    return value;
}

function sendGcode(gcode){
    var tt_url = "/gcode?gc="+gcode;
    xmlHttp = new XMLHttpRequest();
    xmlHttp.open("GET", tt_url);
    xmlHttp.send();
}
var x_pos=0;
var y_pos=0;
var z_pos=0;
var e_pos=0;
const xpButton = document.getElementById("x-p");
const xdButton = document.getElementById("x-d");
const ypButton = document.getElementById("y-p");
const ydButton = document.getElementById("y-d");
const zpButton = document.getElementById("z-p");
const zdButton = document.getElementById("z-d");
const epButton = document.getElementById("e-p");
const edButton = document.getElementById("e-d");

const haButton = document.getElementById("home");
const hxButton = document.getElementById("home-x");
const hyButton = document.getElementById("home-y");
const hzButton = document.getElementById("home-z");

const setHeadButton = document.getElementById("set-head");
const setBedButton = document.getElementById("set-bed");

const pauseButton = document.getElementById("btn-pause");
const cancleButton = document.getElementById("btn-cancle");
const restartButton = document.getElementById("btn-restart");
const resethostButton = document.getElementById("btn-resethost");

xpButton.onclick = () => {
    var step = getRadioValue();
    x_pos += parseFloat(step);
    var cmd = "G0 X"+x_pos.toFixed().toString();
    sendGcode(cmd);
}

xdButton.onclick = () => {
    var step = getRadioValue();
    x_pos -= parseFloat(step);
    if(x_pos<0) x_pos = 0.0;
    var cmd = "G0 X-"+x_pos.toFixed().toString();
    sendGcode(cmd);
}

ypButton.onclick = () => {
    var step = getRadioValue();
    y_pos += parseFloat(step);
    var cmd = "G0 Y"+y_pos.toFixed().toString();
    sendGcode(cmd);
}

ydButton.onclick = () => {
    var step = getRadioValue();
    y_pos -= parseFloat(step);
    if(y_pos<0) y_pos = 0.0;
    var cmd = "G0 Y-"+y_pos.toFixed().toString();
    sendGcode(cmd);
}

zpButton.onclick = () => {
    var step = getRadioValue();
    z_pos += parseFloat(step);
    var cmd = "G0 Z"+z_pos.toFixed().toString();
    sendGcode(cmd);
}

zdButton.onclick = () => {
    var step = getRadioValue();
    z_pos -= parseFloat(step);
    if(z_pos<0) z_pos = 0.0;
    var cmd = "G0 Z-"+z_pos.toFixed().toString();
    sendGcode(cmd);
}

epButton.onclick = () => {
    var step = getRadioValue();
    e_pos += parseFloat(step);
    var cmd = "G0 E"+e_pos.toFixed().toString();
    sendGcode(cmd);
}

edButton.onclick = () => {
    var step = getRadioValue();
    e_pos -= parseFloat(step);
    if(e_pos<0) e_pos = 0.0;
    var cmd = "G0 E-"+e_pos.toFixed().toString();
    sendGcode(cmd);
}

haButton.onclick = () => {
    var cmd = "G28";
    x_pos = 0;
    y_pos = 0;
    z_pos = 0;
    sendGcode(cmd);
}

hxButton.onclick = () => {
    var cmd = "G28 X";
    x_pos = 0;
    sendGcode(cmd);
}

hyButton.onclick = () => {
    var cmd = "G28 Y";
    y_pos = 0;
    sendGcode(cmd);
}

hzButton.onclick = () => {
    var cmd = "G28 Z";
    z_pos = 0;
    sendGcode(cmd);
}

setHeadButton.onclick = () => {
    var target_head = document.getElementById("temp-head").value;
    var cmd = "M104 S"+target_head;
    sendGcode(cmd);
}

setBedButton.onclick = () => {
    var target_bed = document.getElementById("temp-bed").value;
    var cmd = "M140 S"+target_bed;
    sendGcode(cmd);
}

pauseButton.onclick = () => {
    var tt_url = "/operate?op=PAUSE";
    xmlHttp = new XMLHttpRequest();
    xmlHttp.open("GET", tt_url);
    xmlHttp.send();
}
cancleButton.onclick = () => {
    var tt_url = "/operate?op=CANCLE";
    xmlHttp = new XMLHttpRequest();
    xmlHttp.open("GET", tt_url);
    xmlHttp.send();

    var percent_ele = document.getElementById("print-progess");
    var printFileElement = document.getElementById("print-file");
    percent_ele.innerHTML = 0;
    printFileElement.innerHTML = "no file";
}

restartButton.onclick = () => {
    var tt_url = "/esprestart";
    xmlHttp = new XMLHttpRequest();
    xmlHttp.open("GET", tt_url);
    xmlHttp.send();
}

resethostButton.onclick = () => {
    var tt_url = "/resetusb";
    xmlHttp = new XMLHttpRequest();
    xmlHttp.open("GET", tt_url);
    xmlHttp.send();
}