/*
Blue
by dimitdim

LeadMe firmware
for hardware v2.0

Settings below:
- Pin numbers: should be self-explanatory.  Digital Arduino pin numbers unless noted.  Mostly for testing
- batt_low: when should the LED starting blinking?
- num_vib: number of vibrators.  Mostly or testing
- LED_enable: turn LED off it it proves annoying

Serial commands:
- angle@[angle]!: vibrate in [angle] direction, or turn off all motors if [angle]<0
- all!: vibrate all motors
- test!: demo mode, rotate motors
- batt!: reutn battery level in percent
- motor@[val]! vibrate a specific motor at a specfic speed.  0-255 vibrates motor 0, 256-511 vibrates motor 1, etc. Mostly for testing
All commands also return current PWM values.  Mostly for testing

*/

/*Pin Definitions*/
const int M0 = 5; //Leftmost bird's-eye-view.  Increasing clockwise...
const int M1 = 6;
const int M2 = 9;
const int M3 = 10;
const int M4 = 11;

const int BatLvl = 0; //Analog
const int Butt = 12;
const int Charge = 8; //The IC outputs tri-state, so I can change this to an ADC input, stick a voltage divider on there and differentiate between "Not charging" and "Done charging". On popular demand.
const int BTStat = 7;
const int LED = 13;

/*Settings*/
const int batt_low = 20; //In percent
const int num_vib = 5; //Legacy, pretty sure we're set on 5.  Helpful for testing though...
const boolean LED_enable = true;

/*Serial*/
String inputString=""; //Initiate Serial input command variables
String cmd="";
int val=0;
boolean stringComplete=false; //Don't run debuging info until a command is entered

/*Misc Definitions*/
int duty[num_vib]={0};
int angle=-1;
int base=0;
int add=0;
int i;
int battery_level;
int bl_raw;
boolean demo=false;
int time10;
int demo_time;

void all(int d) {
  for(i=0;i<num_vib;i++) {
    duty[i]=d;
  }
}

void update_bl() {
  bl_raw=analogRead(BatLvl);
  battery_level = 100*((BatLvl-574)/62); //Apologies for the random hard-coded numbers.  We'll get an expirimental curve up in this bitch later
  battery_level = max(0,min(100,battery_level)); //Some limiting
}

void update_LED() { if(LED_enable) {
  if(!digitalRead(Charge)) digitalWrite(LED,(time10%2000)>1000); //Can change to pulsing on popular demand.  But the LED won't be D13, and that sucks cuz it's usefull for SPI
  else digitalWrite(LED,digitalRead(BTStat));
  if(battery_level<=batt_low && time10%5000<600) {
    if((time10%200)<100) digitalWrite(LED,HIGH);
    else digitalWrite(LED,LOW);
  }
}}

void communicate() {
  cmd=inputString.substring(0,inputString.indexOf('@')); //Split the input into command and value. The only reason this isn't '=' is because I like '@'
  val=inputString.substring(inputString.indexOf('@')+1).toInt();
  demo=false;
  
  if (cmd.equals(String("angle"))) {
    angle=(max(-1,val))%360;
    all(0);
    if(angle>=0) {
      base=angle/(360/num_vib);
      add=angle%(360/num_vib);
      duty[base%num_vib]=(255*((360/num_vib)-add))/(360/num_vib);
      duty[(base+1)%num_vib]=(255*add)/(360/num_vib);
    }
  }

  else if (cmd.equals(String("motor"))) {
    duty[(val/255)%num_vib]=val%255;
  }
  
  else if (cmd.equals(String("test"))) {
    demo=true;
    Serial.println("Demo Time!");
  }
  
  else if(cmd.equals(String("all"))) {
    all(255);
  }

  else if(cmd.equals(String("batt"))) {
    Serial.println(battery_level);
  }

  else {
    Serial.println("Try again.");
  }

  Serial.print("PWM Values: ");
  for(i=0;i<num_vib;i++){
    Serial.print(duty[i])&&Serial.print(',');
  }
  Serial.println();

  stringComplete=false;
  inputString = "";
  
}
void setup() {
  Serial.begin(9600);
  while (!Serial) {;} //some Arduinos take a while...  Leonardo, I'm looking at you...
  inputString.reserve(128); //allows adding one byte at a time (it needs to be a buffer of sorts)
  pinMode(M0,OUTPUT);
  pinMode(M1,OUTPUT);
  pinMode(M2,OUTPUT);
  pinMode(M3,OUTPUT);
  pinMode(M4,OUTPUT);
  pinMode(BatLvl,INPUT);
  pinMode(Charge,INPUT);
  pinMode(LED,OUTPUT);
  Serial.println("Welcome!");
  for(i=0;i<3;i++) { //Welcome!
    all(255);
    digitalWrite(LED,HIGH);
    delay(250);
    all(0);
    digitalWrite(LED,LOW);
    delay(250);
  }
}

void loop() {
  time10=millis()%10000;
  update_bl();
  update_LED();
  if(stringComplete) {communicate();} //update PWM duty cycles
  if(demo) {
    demo_time=(time10%1000)/4;
    duty[0]=(demo_time+0*255/num_vib)%255;
    duty[1]=(demo_time+1*255/num_vib)%255;
    duty[2]=(demo_time+2*255/num_vib)%255;
    duty[3]=(demo_time+3*255/num_vib)%255;
    duty[4]=(demo_time+4*255/num_vib)%255;
  }
  analogWrite(M0,duty[0]);
  analogWrite(M1,duty[1]);
  analogWrite(M2,duty[2]);
  analogWrite(M3,duty[3]);
  analogWrite(M4,duty[4]);
}

void serialEvent() { //This gets run every time after loop
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        if (inChar == '!') {
            inputString.trim(); //Get rid of the \n
            stringComplete = true; //The loop will run next time!
        }
        else { //Put all the input bytes in a buffer
            inputString += inChar;
        }
    }
}
