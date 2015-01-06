/*
Blue
by dimitdim

LeadMe firmware
for hardware v2

Settings below:
- pin numbers: should be self-explanatory.  Digital Arduino pin numbers unless noted.  Mostly for testing
- batt_low: when should the LED starting blinking?
- num_vib: number of motors.  Mostly for testing
- LED_enable: turn LED off if it proves annoying
- v0: LiPo voltage equivelent to 0% (linear model)
- v100: LiPo voltage equivelent to 100% (linear model)

Serial commands:
- angle@[angle]!: vibrate in [angle] direction (with fading, 0 is purely motor 0), or turn off all motors if [angle]<0
- all!: vibrate all motors
- test!: demo mode, rotate motors
- batt!: return battery level in percent
- motor@[val]! vibrate a specific motor at a specific speed.  0-255 vibrates motor 0, 256-511 vibrates motor 1, etc. Mostly for testing
All commands also return current PWM values.  Mostly for testing
*/

/*Pin Definitions*/
const int M0 = 5; //Leftmost, bird's-eye-view.  Increasing clockwise
const int M1 = 6;
const int M2 = 9;
const int M3 = 10;
const int M4 = 11;

const int BatLvl = 0; //Analog
const int BTStat = 2;
const int Charge = 8; //The IC outputs tri-state, so I can change this to an ADC input, stick a voltage divider on there and differentiate between "Not charging" and "Done charging". On popular demand.
const int Button = 12;
const int LED = 13;

/*Settings*/
const int batt_low = 20; //In percent
const int num_vib = 5; //Legacy, pretty sure we're set on 5.  Helpful for testing though...
const boolean LED_enable = true;

const int v0=3.7; //For a linear estimation of LiPo capacity (very innacurate, I know).  We'll get an expirimental curve up in this bitch later
const int v100=4.1;

/*Serial*/
String inputString; //Initiate Serial input command variables
String cmd;
int val;
boolean stringComplete=false; //Don't run until a command is entered

/*Misc Variables*/
int duty[num_vib];
int angle;
int base;
int add;
int i;
int battery_level;
int time10;
int demo_time;
boolean demo=false;

void update_LED() { if(LED_enable) {
  if(!digitalRead(Charge)) digitalWrite(LED,(time10%2000)>1000); //Can change to pulsing on popular demand.  But the LED won't be D13, and that sucks cuz it's usefull for SPI
  else digitalWrite(LED,digitalRead(BTStat));
  if(battery_level<=batt_low && time10%5000<600) digitalWrite(LED,(time10%200)<100);
}}

void all(int d) {
  for(i=0;i<num_vib;i++) {
    duty[i]=d;
  }
}

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
    duty[(val/255)%num_vib-1]=val%256;
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
  for(i=0;i<num_vib;i++) {
    Serial.print(duty[i])&&Serial.print(',');
  }
  Serial.println();
  stringComplete=false;
  inputString = "";
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {;} //some Arduinos take a while...  Leonardo, I'm looking at you...
  inputString.reserve(32); //allows adding one byte at a time (it needs to be a buffer of sorts)
  
  analogReference(EXTERNAL);

  pinMode(M0,OUTPUT);
  pinMode(M1,OUTPUT);
  pinMode(M2,OUTPUT);
  pinMode(M3,OUTPUT);
  pinMode(M4,OUTPUT);
  pinMode(BatLvl,INPUT);
  pinMode(BTStat,INPUT);
  pinMode(Charge,INPUT);
  pinMode(LED,OUTPUT);

  /*Welcome!*/
  Serial.println("Welcome!");
  for(i=0;i<3;i++) {
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
  battery_level=max(0,min(100,map(analogRead(BatLvl), v0*1023/6.6, v100*1023/6.6, 0, 100))); //Again, innacurate linear model
  update_LED();
  if(stringComplete) communicate(); //update PWM duty cycles
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
            stringComplete = true; //The update will run next time!
        }
        else { //Put all the input bytes in a buffer
            inputString += inChar;
        }
    }
}
