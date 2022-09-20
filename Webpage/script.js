/******
 * file upload and list
 */
function isTextFile(path) {
  var ext = /(?:\.([^.]+))?$/.exec(path)[1];
  if (typeof ext !== undefined) {
    switch (ext) {
      case 'txt':
      case 'htm':
      case 'html':
      case 'js':
      case 'json':
      case 'c':
      case 'h':
      case 'cpp':
      case 'css':
      case 'xml':
        return true;
    }
  }
  return false;
}

var b_printing= false;
var str_upload_file = "";

function updateUserBtn(){
  var tt_url = '/getfmdname';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
  console.log("test send btn");
};


function createTreeLeaf(path, name, size) {
  var td = document.createElement('td');
  td.id = ((path == '/' ? '' : path) + '/' + name).toLowerCase();
  td.innerHTML = name.toLowerCase();
  return td;
}
function convertToShortName(fullName) {
  var f_name = fullName.replace('/', '');
  var base_name = f_name.replace(/\.[^/.]+$/, '');
  if (base_name.length >= 6) {
    base_name = base_name.substring(0, 6) + '~1';
  } else {
    base_name = base_name + '~1';
  }
  var full_name = base_name + '.GCO';
  return full_name;
}

function makeid(length) {
  var result = '';
  var characters =
    'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
  var charactersLength = characters.length;
  for (var i = 0; i < length; i++) {
    result += characters.charAt(Math.floor(Math.random() * charactersLength));
  }
  return result;
}

function addList(parent, path, items) {
  var ll = items.length;
  for (var i = 0; i < ll; i++) {
    var list = document.createElement('tr');
    parent.appendChild(list);
    var item = items[i];
    var itemEl;
    if (item.type === 'file') {
      if (item.name.endsWith('.gcode')) {
        itemEl = createTreeLeaf(path, item.name, item.size);
        console.log(item.name);
        list.appendChild(itemEl);
        var td_print = document.createElement('td');
        td_print.innerHTML =
          "<button class='btn btn-default btn-xs' type='button'><span class='glyphicon glyphicon-print' aria-hidden='true'></span>print</button>";
        td_print.className = item.name;
        td_print.onclick = function () {
          xmlHttp = new XMLHttpRequest();
          xmlHttp.onreadystatechange = function () {
            var resp = xmlHttp.responseText;
            if (
              resp.startsWith('NO') ||
              resp.startsWith('SD') ||
              resp.startsWith('PRINTER')
            ) {
                if(resp.startsWith('NOT DIR')){
                  alert("Node init SD card failed, please click 'Mount SD' and update filelist again");
                }else if(resp.startsWith('PRINTER BUSY')){
                  alert("Printer is printing, or not finish job normally, click 'Cancel' can get printer again.");
                }else{
                  alert(resp);
                }
            }
          };
          var short_name = '/' + convertToShortName(this.className);
          xmlHttp.open('GET', '/print?filename=' + short_name, true);
          xmlHttp.send();
          b_printing = true;

        };
        list.appendChild(td_print);

        var td = document.createElement('td');
        td.innerHTML =
          "<button class='btn btn-default btn-xs' type='button'><span class='glyphicon glyphicon-trash' aria-hidden='true'></span>delete</button>";
        td.className = item.name;
        td.onclick = function () {
          console.log('delete: %s', this.className);
          xmlHttp = new XMLHttpRequest();
          xmlHttp.onload = function () {
            creatTree();
          };
          xmlHttp.onreadystatechange = function () {
            var resp = xmlHttp.responseText;
            if (
              resp.startsWith('NO') ||
              resp.startsWith('SD') ||
              resp.startsWith('PRINTER')
            ) {
              if(resp.startsWith('NOT DIR')){
                alert("Node init SD card failed, please click 'Mount SD' and update filelist again");
              }else if(resp.startsWith('PRINTER BUSY')){
                alert("Printer is printing, or not finish job normally, click 'Cancel' can get printer again.");
              }else{
                alert(resp);
              }
            }
          };
          xmlHttp.open('GET', '/remove?path=' + this.className, true);
          xmlHttp.send();
        };
        list.appendChild(td);
      }
    }
  }
}

function httpGet(parent, path) {
  xmlHttp = new XMLHttpRequest(parent, path);
  xmlHttp.onreadystatechange = function () {
    var resp = xmlHttp.responseText;
    if (xmlHttp.readyState == 4) {
      //clear loading
      if (xmlHttp.status == 200)
        addList(parent, path, JSON.parse(xmlHttp.responseText));
        console.log(xmlHttp.responseText);
    }
    if (
      resp.startsWith('NO') ||
      resp.startsWith('SD') ||
      resp.startsWith('PRINTER')
    ) {
        if(resp.startsWith('NOT DIR')){
          alert("Node init SD card failed, please click 'Mount SD' and update filelist again");
        }else if(resp.startsWith('PRINTER BUSY')){
          alert("Printer is printing, or not finish job normally, click 'Cancel' can get printer again.");
        }else{
          alert(resp);
        }
      
    }
  };
  xmlHttp.open('GET', '/list?dir=' + path, true);
  xmlHttp.send(null);
  //start loading
}

function creatTree() {
  var fileListM = document.getElementById('file-list-frame');
  while (fileListM.hasChildNodes()) {
    fileListM.removeChild(fileListM.lastChild);
  }

  httpGet(fileListM, '/');
}

const updateButton = document.getElementById('update-list');
const unmountButton = document.getElementById('release-sd');
const mountButton = document.getElementById('get-sd');
const uploadButton = document.getElementById('upload-file');

updateButton.onclick = () => {
  creatTree();
};

unmountButton.onclick = () => {
  var tt_url = '/operate?op=RELEASESD';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.onreadystatechange = function () {
    var resp = xmlHttp.responseText;
    if (
      resp.startsWith('NO') ||
      resp.startsWith('SD') ||
      resp.startsWith('PRINTER')
    ) {
      if(resp.startsWith('PRINTER BUSY')){
        alert("Printer is printing, or not finish job normally, click 'Cancel' can get printer again.");
      }else{
        alert(resp);
      }
    }
  };
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
};

mountButton.onclick = () => {
  var tt_url = '/operate?op=GETSD';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.onreadystatechange = function () {
    var resp = xmlHttp.responseText;
    if (
      resp.startsWith('NO') ||
      resp.startsWith('SD') ||
      resp.startsWith('PRINTER')
    ) {
      if(resp.startsWith('NOT DIR')){
        alert("Node init SD card failed, please click 'Mount SD' and update filelist again");
      }else if(resp.startsWith('PRINTER BUSY')){
        alert("Printer is printing, or not finish job normally, click 'Cancel' can get printer again.");
      }else{
        alert(resp);
      }
    }
  };
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
};

function uploadComplete(evt) {
  var progressBar = document.getElementById('progressbar');
  progressBar.value = 0;
  document.getElementById('probar').style.display="none";
  // document.getElementById('file_msg').style.display="block";
  document.getElementById('file_msg').innerHTML=str_upload_file;
  alert('Upload finish!');
  creatTree();
}
//上传失败
function uploadFailed(evt) {
  alert('Upload failed!');
}

function progressFunction(evt) {
  var progressBar = document.getElementById('progressbar');
  if (evt.lengthComputable) {
    //
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
  var input = document.getElementById('choose-file');
  if (input.files.length === 0) {
    return;
  }
  document.getElementById('probar').style.display="block";
  str_upload_file = input.files[0].name;
  document.getElementById('file_msg').innerHTML=" ";
  var fileListM = document.getElementById('file-list-frame');
  while (fileListM.hasChildNodes()) {
    fileListM.removeChild(fileListM.lastChild);
  }
  xmlHttp = new XMLHttpRequest();
  // xmlHttp.onreadystatechange = httpPostProcessRequest;
  xmlHttp.onload = uploadComplete;
  xmlHttp.onerror = uploadFailed;
  xmlHttp.upload.onprogress = progressFunction;
  var formData = new FormData();
  var savePath = '';
  var random_start = makeid(3);
  if(input.files[0].name.endsWith('gcode'))  {
    savePath = '/' + random_start + '-' + input.files[0].name;
  }
  else{
    savePath = '/' + input.files[0].name;
  }
  formData.append('data', input.files[0], savePath);
  xmlHttp.open('POST', '/edit');
  xmlHttp.send(formData);
};

/**
 * Gcode send and read, text display
 */
const sendGcodedButton = document.getElementById('btn-send-gcode');
const sendCleardButton = document.getElementById('btn-send-clear');
const copySelectButton = document.getElementById('btn-sel-copy');
const autoCheckButton = document.getElementById('btn-auto-check');
const sendGcdoeInput = document.getElementById('gcode-lineedit');

const user1 = document.getElementById('user-1');
const user2 = document.getElementById('user-2');
const user3 = document.getElementById('user-3');

const muser1 = document.getElementById('muser-1');
const muser2 = document.getElementById('muser-2');
const muser3 = document.getElementById('muser-3');


sendGcodedButton.onclick = () => {
  var cmdLineEdit = document.getElementById('gcode-lineedit');
  var cmd = cmdLineEdit.value;
  var tt_url = '/gcode?gc=' + cmd;
  xmlHttp = new XMLHttpRequest();
  xmlHttp.onreadystatechange = function () {
    var resp = xmlHttp.responseText;
    if (
      resp.startsWith('NO') ||
      resp.startsWith('SD') ||
      resp.startsWith('PRINTER')
    ) {
      if(resp.startsWith('NOT DIR')){
        alert("Node init SD card failed, please click 'Mount SD' and update filelist again");
      }else if(resp.startsWith('PRINTER BUSY')){
        alert("Printer is printing, or not finish job normally, click 'Cancel' can get printer again.");
      }else{
        alert(resp);
      }
    }
  };
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
};
sendCleardButton.onclick = () => {
  document.getElementById('serial-console').value = 'clean\r\n';
};

document.getElementById("btn-sel-copy").addEventListener("click", function (e) {
  var textarea = document.getElementById("serial-console");
  e.target.download = "log-" + (new Date()).toLocaleTimeString() + ".txt";
  console.log(textarea.value);
  e.target.href = "data:image/svg+xml;,"+encodeURIComponent(textarea.value);
  textarea.select();
  document.execCommand("copy");

}, false);  


//enter button
sendGcdoeInput.onkeydown = function (e) {
  if (e.keyCode == 13) {
    var cmd = this.value;
    var tt_url = '/gcode?gc=' + cmd;
    xmlHttp = new XMLHttpRequest();
    xmlHttp.open('GET', tt_url);
    xmlHttp.send();
  }
};

user1.onclick = () => {
  var tt_url = '/user1';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
  console.log("user1 button");
};

user2.onclick = () => {
  var tt_url = '/user2';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
  console.log("user2 button");
};

user3.onclick = () => {
  var tt_url = '/user3';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
  console.log("user3 button");
};

muser1.onclick = () => {
  var tt_url = '/user1';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
  console.log("user1 button");
};

muser2.onclick = () => {
  var tt_url = '/user2';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
  console.log("user2 button");
};

muser3.onclick = () => {
  var tt_url = '/user3';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
  console.log("user3 button");
};




var source = new EventSource('/events');
source.addEventListener(
  'gcode_cli',
  function (e) {
    //console.log("gyro_readings", e.data);
    var obj = e.data;
    var scrollText = document.getElementById('serial-console');
    var show_msg = true;
    if(obj.startsWith('##')&&obj.endsWith('&&'))
    {
      var user1 = document.getElementById('user-1');
      var user2 = document.getElementById('user-2');
      var user3 = document.getElementById('user-3');
      // var user4 = document.getElementById('user-4');
      var users = new Array();
      users[0]=user1;
      users[1]=user2;
      users[2]=user3;
      // users[3]=user4;
      var muser1 = document.getElementById('muser-1');
      var muser2 = document.getElementById('muser-2');
      var muser3 = document.getElementById('muser-3');
      // var user4 = document.getElementById('user-4');
      var musers = new Array();
      musers[0]=muser1;
      musers[1]=muser2;
      musers[2]=muser3;

      var str_sub = obj.slice(2, -2).split(';');
      var ary_len = str_sub.length;
      for(var i=0; i<3;i++){
        if(str_sub[i].startsWith('/fmd')&&str_sub[i].endsWith('txt')){
          users[i].innerHTML = str_sub[i].slice(5,-6);
          musers[i].innerHTML = str_sub[i].slice(5,-6);
        }
      }
      // console.log(str_sub);
    }

    //get file name, progress and tempture
    var reg_t = /T:([0-9]*\.[0-9]*) *\/([0-9]*\.[0-9]*)/g;
    var reg_b = /B:([0-9]*\.[0-9]*) *\/([0-9]*\.[0-9]*)/g;
    var reg_p = /SD printing byte ([0-9]*)\/([0-9]*)/g;
    var reg_f = /Current file:([\S\s]*).GCO/g;
    var reg_af = /Current file:([\S\s]*).GCO ([\S\s]*).gcode/g;
    var reg_prusa = /:([\S\s]*).gcode/g;
    var reg_short_low = /:([\S\s]*).gco/g;
    var reg_end = /Finish/g;

    var reg_chip=/Start @/g

    // refer marlin language.h  STR_SD_XXX
    var reg_sd_err_init = /No SD card/g;
    var reg_sd_err_subdir = /Cannot open subdir/g;
    var reg_sd_err_volinit = /volume.init failed/g;
    var reg_sd_err_root = /openRoot failed/g;

    var b_start = obj.match(reg_chip);
    if(b_start){
      //setTimeout for update fmd button
      setTimeout(updateUserBtn,1000);
    }

    var checktempture = document.getElementById('easymode').checked;

    var heater = obj.match(reg_t);
    if (heater) {
      var header_tmp = RegExp.$1;
      var header_targ = RegExp.$2;
      var display_header = header_tmp + '/' + header_targ;
      var header_ele = document.getElementById('hothead-display');
      header_ele.innerHTML = display_header;
      if(checktempture){
        show_msg = false;
        console.log("not show the tempture");
        console.log(checktempture);
      }
    }

    var beder = obj.match(reg_b);
    if (beder) {
      var beder_tmp = RegExp.$1;
      var beder_targ = RegExp.$2;
      var display_bed = beder_tmp + '/' + beder_targ;
      var bed_ele = document.getElementById('hotbed-display');
      bed_ele.innerHTML = display_bed;
      if(checktempture){
        show_msg = false;
      }
        
    }

    var prog = obj.match(reg_p);
    if (prog) {
      var current_line = parseFloat(RegExp.$1);
      var total_lines = parseFloat(RegExp.$2);
      var percent = Math.round((current_line / total_lines) * 100);
      var percent_ele = document.getElementById('print-progess');
      percent_ele.innerHTML = percent.toString();
	  if(checktempture){
        show_msg = false;
      }
    }

    var print_file = obj.match(reg_f);
    if (print_file) {
      var printFileElement = document.getElementById('print-file');
      printFileElement.innerHTML = RegExp.$1 + '.GCO';
	  if(checktempture){
        show_msg = false;
      }
    }

    var print_ful_name = obj.match(reg_af);
    if (print_ful_name) {
        var printFileElement = document.getElementById('print-file');
        printFileElement.innerHTML = RegExp.$2 + '.gcode';
		if(checktempture){
        show_msg = false;
      }
    }

    // process SD card error
    if(b_printing)
    {
      if(obj.match(reg_sd_err_init) || obj.match(reg_sd_err_subdir) ||obj.match(reg_sd_err_volinit) ||obj.match(reg_sd_err_root) )
      {
        alert('Printer init SD card failed! Can not start printing.');
        var tt_url = '/operate?op=CANCLE';
        xmlHttp = new XMLHttpRequest();
        xmlHttp.open('GET', tt_url);
        xmlHttp.send();
      
        var percent_ele = document.getElementById('print-progess');
        var printFileElement = document.getElementById('print-file');
        percent_ele.innerHTML = 0;
        printFileElement.innerHTML = 'no file';
        b_printing = false;
      }
    }

    if(obj.includes("no file")){

      if(b_printing)
      {
        var tt_url = '/operate?op=CANCLE';
        xmlHttp = new XMLHttpRequest();
        xmlHttp.open('GET', tt_url);
        xmlHttp.send();
      
        var percent_ele = document.getElementById('print-progess');
        var printFileElement = document.getElementById('print-file');
        percent_ele.innerHTML = 0;
        printFileElement.innerHTML = 'no file';
       
        b_printing = false;
      }

    } 

    var print_prusa_short = obj.match(reg_short_low);
    if (print_prusa_short) {
      var printFileElement = document.getElementById('print-file');
      printFileElement.innerHTML = RegExp.$1 + '.GCO';
    }

    var print_prusa_name = obj.match(reg_prusa);
    if (print_prusa_name) {
      var printFileElement = document.getElementById('print-file');
      printFileElement.innerHTML = RegExp.$1 + '.gcode';
    }

    var b_finish = obj.match(reg_end);
    var b_done = obj.match(/Done/g);
    if (b_finish || b_done) {
      var percent_ele = document.getElementById('print-progess');
      var printFileElement = document.getElementById('print-file');
      percent_ele.innerHTML = 0;
      printFileElement.innerHTML = 'no file';
      alert('Print job finish!');
    }


    if(show_msg){
      obj += '\r\n';
      scrollText.value += obj;
      scrollText.scrollTop = scrollText.scrollHeight;
    }


  },
  false
);

var int = self.setInterval('updateStatus()', 8000);
function updateStatus() {
  var tt_url = '/status';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
}

/***
 * Gcode control
 *
 */

function getRadioValue() {
  var radios = document.getElementsByName('stepRadio');
  var value = 0;
  for (var i = 0; i < radios.length; i++) {
    if (radios[i].checked == true) {
      value = radios[i].value;
    }
  }
  return value;
}

function sendGcode(gcode) {
  var tt_url = '/gcode?gc=' + gcode;
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
}

function sendUrlRequest(url, time) {
  var tt_url = url;
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
}

var x_pos = 0;
var y_pos = 0;
var z_pos = 0;
var e_pos = 0;
const xpButton = document.getElementById('x-p');
const xdButton = document.getElementById('x-d');
const ypButton = document.getElementById('y-p');
const ydButton = document.getElementById('y-d');
const zpButton = document.getElementById('z-p');
const zdButton = document.getElementById('z-d');
const epButton = document.getElementById('e-p');
const edButton = document.getElementById('e-d');

const haButton = document.getElementById('home');
const hxButton = document.getElementById('home-x');
const hyButton = document.getElementById('home-y');
const hzButton = document.getElementById('home-z');

const setHeadButton = document.getElementById('set-head');
const setBedButton = document.getElementById('set-bed');
const setHeadLineedit = document.getElementById('temp-head');
const setBedLineedit = document.getElementById('temp-bed');

const pauseButton = document.getElementById('btn-pause');
const cancelButton = document.getElementById('btn-cancel');
const restartButton = document.getElementById('btn-restart');
const resethostButton = document.getElementById('btn-resethost');
const setSPIButton = document.getElementById('btn-setspi');
const setSDIOButton = document.getElementById('btn-setsdio');

const mresethostButton = document.getElementById('mbtn-resethost');
const msetSPIButton = document.getElementById('mbtn-setspi');
const msetSDIOButton = document.getElementById('mbtn-setsdio');

xpButton.onclick = () => {
  var step = getRadioValue();
  x_pos += parseFloat(step);
  var cmd = 'G0 X' + x_pos.toFixed().toString();
  sendGcode(cmd);
};

xdButton.onclick = () => {
  var step = getRadioValue();
  x_pos -= parseFloat(step);
  if (x_pos < 0) x_pos = 0.0;
  var cmd = 'G0 X-' + x_pos.toFixed().toString();
  sendGcode(cmd);
};

ypButton.onclick = () => {
  var step = getRadioValue();
  y_pos += parseFloat(step);
  var cmd = 'G0 Y' + y_pos.toFixed().toString();
  sendGcode(cmd);
};

ydButton.onclick = () => {
  var step = getRadioValue();
  y_pos -= parseFloat(step);
  if (y_pos < 0) y_pos = 0.0;
  var cmd = 'G0 Y-' + y_pos.toFixed().toString();
  sendGcode(cmd);
};

zpButton.onclick = () => {
  var step = getRadioValue();
  z_pos += parseFloat(step);
  var cmd = 'G0 Z' + z_pos.toFixed().toString();
  sendGcode(cmd);
};

zdButton.onclick = () => {
  var step = getRadioValue();
  z_pos -= parseFloat(step);
  if (z_pos < 0) z_pos = 0.0;
  var cmd = 'G0 Z-' + z_pos.toFixed().toString();
  sendGcode(cmd);
};

epButton.onclick = () => {
  var step = getRadioValue();
  e_pos += parseFloat(step);
  var cmd = 'G0 E' + e_pos.toFixed().toString();
  sendGcode(cmd);
};

edButton.onclick = () => {
  var step = getRadioValue();
  e_pos -= parseFloat(step);
  if (e_pos < 0) e_pos = 0.0;
  var cmd = 'G0 E-' + e_pos.toFixed().toString();
  sendGcode(cmd);
};

haButton.onclick = () => {
  var cmd = 'G28';
  x_pos = 0;
  y_pos = 0;
  z_pos = 0;
  sendGcode(cmd);
};

hxButton.onclick = () => {
  var cmd = 'G28 X';
  x_pos = 0;
  sendGcode(cmd);
};

hyButton.onclick = () => {
  var cmd = 'G28 Y';
  y_pos = 0;
  sendGcode(cmd);
};

hzButton.onclick = () => {
  var cmd = 'G28 Z';
  z_pos = 0;
  sendGcode(cmd);
};

setHeadButton.onclick = () => {
  var target_head = document.getElementById('temp-head').value;
  var cmd = 'M104 S' + target_head;
  sendGcode(cmd);
};

setBedButton.onclick = () => {
  var target_bed = document.getElementById('temp-bed').value;
  var cmd = 'M140 S' + target_bed;
  sendGcode(cmd);
};

// setHeadLineedit.onclick
// setBedLineedit = document.getElementById('temp-bed');
setHeadLineedit.onkeydown = function (e) {
  if (e.keyCode == 13) {
    var target_head = document.getElementById('temp-head').value;
    var cmd = 'M104 S' + target_head;
    sendGcode(cmd);
  }
};

setBedLineedit.onkeydown = function (e) {
  if (e.keyCode == 13) {
    var target_bed = document.getElementById('temp-bed').value;
    var cmd = 'M140 S' + target_bed;
    sendGcode(cmd);
  }
};

pauseButton.onclick = () => {
  var tt_url = '/operate?op=PAUSE';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
};
cancelButton.onclick = () => {
  var tt_url = '/operate?op=CANCLE';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();

  var percent_ele = document.getElementById('print-progess');
  var printFileElement = document.getElementById('print-file');
  percent_ele.innerHTML = 0;
  printFileElement.innerHTML = 'no file';
  b_printing = false;
};

restartButton.onclick = () => {
  var tt_url = '/operate?op=RECOVER';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
};

resethostButton.onclick = () => {
  var tt_url = '/resetusb';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
};

setSPIButton.onclick = () => {
  alert(
    'After switching the SD data reading method, please must repower your Node and 3D printer.'
  );
  var tt_url = '/setsdtype?type=SPI';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
};

setSDIOButton.onclick = () => {
  alert(
    'After switching the SD data reading method, please must repower your Node and 3D printer.'
  );
  var tt_url = '/setsdtype?type=SDIO';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
};

mresethostButton.onclick = () => {
  var tt_url = '/resetusb';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
};

msetSPIButton.onclick = () => {
  alert(
    'After switching the SD data reading method, please must repower your Node and 3D printer.'
  );
  var tt_url = '/setsdtype?type=SPI';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
};

msetSDIOButton.onclick = () => {
  alert(
    'After switching the SD data reading method, please must repower your Node and 3D printer.'
  );
  var tt_url = '/setsdtype?type=SDIO';
  xmlHttp = new XMLHttpRequest();
  xmlHttp.open('GET', tt_url);
  xmlHttp.send();
};

autoCheckButton.onclick = () => {
  // auto check step
  sendUrlRequest('/resetusb',200);
  //1. homeimg test
  var i = 0;
  for(i=0;i<3;i++){
      sendGcode('M118 $Fiberpunk echo\n');
  }
  setTimeout(function() {
    sendGcode('M115\n');
  },100);

  sendUrlRequest('/operate?op=GETSD', 1300);
  creatTree();
  sendUrlRequest('/operate?op=RELEASESD', 300);
  sendGcode('M20\n');
  sendUrlRequest('/operate?op=GETSD', 300);
};







