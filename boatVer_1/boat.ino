#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Servo.h> 

const char* ssid = "RC_boat";
const char* password = "99887766";
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
IPAddress IP;
Servo rudder;

const int out_serv = 4;
const int out_motor1 = 14;
const int out_motor2 = 12;
const int led = 5;

String sliderValue1 = "90";
String sliderValue2 = "0";
String state;

const char* PARAM_INPUT_1 = "value";
const char* PARAM_INPUT_2 = "state";

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1 maximum-scale=1 user-scalable=no">
  <title>ESP Web Server</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 1.5rem; position: fixed; top: 0px; right: 40%;}
    p {font-size: 1rem;}
    body {max-width: 400px; margin:0px; padding-bottom: 25px;}
    .direction {position: fixed; bottom: 20px; right: 30px;}
    .motor {position: fixed; top: 20px; left: 50px;}
    .rev {position: fixed; top: 110px; right: 90px;}
    
    .sliderDir { -webkit-appearance: none; margin: 14px; width: 300px; height: 5px; background: #D3D3D3;
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .sliderDir::-webkit-slider-thumb { width: 45px; height: 45px; background: #003249; cursor: pointer;}
    .sliderDir::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; } 

    .sliderMotor { -webkit-appearance: slider-vertical; margin: 14px; width: 5px; height: 180px; background: #D3D3D3;
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .sliderMotor::-webkit-slider-thumb {-webkit-appearance: slider-vertical; appearance: slider-vertical; width: 50px; height: 50px; background: #003249; cursor: pointer;}
    .sliderMotor::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; }
    
    .switch {position: absolute; top: 0; left: 0; right: 0px; bottom: 0; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #9ACD32}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  
  </style>
</head>
<body>
  <h2>WiFi_Boat</h2>
  <div class="direction">
  <p><span id="cyrs">straight</span></p> 
  <p><input type="range" oninput="updateSliderDir(this)" id="dirSlider" min="0" max="180" step="30" value="90" class="sliderDir"></p>
  <p style="font-size: 0.8rem;">DIRECTION</p>
  </div>
  <div class="motor">
  <p><span id="textSliderValue2">%SLIDERVALUE2%</span></p> 
  <p><input type="range" oninput="updateSliderMotor(this)" id="motorSlider" min="0" max="1023" value="%SLIDERVALUE2%" step="10" class="sliderMotor"></p>
  <p style="font-size: 0.8rem; position:relative; top: -160px; right: 40px; -webkit-transform: rotate(270deg);">SPEED</p>
  </div>
  <div class="rev">
  <p style="font-size: 0.8rem; position:relative; top: -40px; left: 25px;">REVERSE</p>  
  <label class="switch">
  <input type="checkbox" id="type" onchange="reversMotor(this)"><span class="slider"></span>
  </label>
  
  </div>
<script>
function updateSliderDir(element) {
  var sliderValue1 = document.getElementById("dirSlider").value;
  if (parseInt(sliderValue1,10) == 90) { 
      document.getElementById("cyrs").innerHTML = "straight"; }
  if (parseInt(sliderValue1,10) > 90) { 
    document.getElementById("cyrs").innerHTML = "right"; }
  if (parseInt(sliderValue1,10) < 90) { 
    document.getElementById("cyrs").innerHTML = "left"; }

  console.log(sliderValue1);
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/direction?value="+sliderValue1, true);
  xhr.send();
}
function updateSliderMotor(element) {
  var sliderValue2 = document.getElementById("motorSlider").value;
  document.getElementById("textSliderValue2").innerHTML = sliderValue2;
  console.log(sliderValue2);  
  var xhr = new XMLHttpRequest();
  if (document.getElementById("type").checked) {xhr.open("GET", "/speed?value="+sliderValue2+"&state=1", true);}
  else {xhr.open("GET", "/speed?value="+sliderValue2+"&state=0", true);}
  xhr.send();
}
function reversMotor(element) {
  document.getElementById("motorSlider").value = "0";
  document.getElementById("textSliderValue2").innerHTML = "0";
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/revers", true);
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

String processor(const String& var){
  if (var == "SLIDERVALUE2"){
    return sliderValue2;
  }
  return String();
}

void setup(){
  Serial.begin(115200);
  rudder.attach(out_serv);
  pinMode(led, OUTPUT);
  pinMode(out_motor1, OUTPUT);
  pinMode(out_motor2, OUTPUT);
  analogWrite(out_serv, sliderValue1.toInt());
  analogWrite(out_motor1, sliderValue2.toInt());
  analogWrite(out_motor2, sliderValue2.toInt());
  digitalWrite(led, HIGH);

/*
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
*/
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  IP = WiFi.softAPIP();
  delay(100);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/direction", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam(PARAM_INPUT_1)) {
      sliderValue1 = request->getParam(PARAM_INPUT_1)->value(); 
      rudder.write(sliderValue1.toInt());
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/speed", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      sliderValue2 = request->getParam(PARAM_INPUT_1)->value();
      state = request->getParam(PARAM_INPUT_2)->value();      
      if (state.toInt() == 0) {
        analogWrite(out_motor1, sliderValue2.toInt());
        analogWrite(out_motor2, 0);
      }
      else {
        analogWrite(out_motor1, 0);
        analogWrite(out_motor2, sliderValue2.toInt() / 2);        
      }
    }  
    request->send(200, "text/plain", "OK");
  });
  
  server.on("/revers", HTTP_GET, [](AsyncWebServerRequest *request){
    analogWrite(out_motor1, 0);
    analogWrite(out_motor2, 0);
    sliderValue2 = "0";        
    request->send(200, "text/plain", "OK");
  });
  
  server.begin();
}

void loop() {

}
