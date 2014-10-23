/*
Serial command is "angle@[angle]" where [angle] is 0-359 (or more, it's moded, don't worry).
Once given, the approriate thingies vibrate.  They are faded if the angle is not a multiple of 45.
The first 6 are the 6 PWM pins, in increasing number order.
For the next 2, we'll serial over a comand to a second chip.  It'll be good.
If you send a negative number, they all stop.
Enjoy.
D
*/

int duty[8]={0};
int angle=-1;
int base=0;
int add=0;
int i;

/*Serial*/
String inputString=""; //Initiate Serial input command variables
String cmd="";
int val=0;
boolean stringComplete=false; //Don't run debuging info until a command is entered

void communicate() {
  cmd=inputString.substring(0,inputString.indexOf('@')); //Split the input into command and value. The only reason this isn't '=' is because I like '@'
  val=inputString.substring(inputString.indexOf('@')+1).toInt();
  
  if (cmd.equals(String("angle"))) {
    angle=(max(-1,val))%360;
    for(i=0;i<8;i++) {
      duty[i]=0;
    }
    if(angle>=0) {
      base=angle/45;
      add=angle%45;
      duty[base%8]=(255*(45-add))/45;
      duty[(base+1)%8]=(255*add)/45;
    }
    for(i=0;i<8;i++){
    Serial.print(duty[i])&&Serial.print(',');
    }
    Serial.println();
  }
  
  stringComplete=false;
  inputString = "";
  
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {;} //some Arduinos take a while...
  inputString.reserve(128); //allows adding one byte at a time (it needs to be a buffer of sorts)
  for(i=0;i<14;i++) { //set them all to outputs
    pinMode(i,OUTPUT);
  }
}

void loop() {
  if(stringComplete) {communicate();} //update PWM duty cycles
  analogWrite(3,duty[0]);
  analogWrite(5,duty[1]);
  analogWrite(6,duty[2]);
  analogWrite(9,duty[3]);
  analogWrite(10,duty[4]);
  analogWrite(11,duty[5]);
}

void serialEvent() { //This gets run every time after loop
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        if (inChar == '\n') {
            inputString.trim(); //Get rid of the \n
            stringComplete = true; //The loop will run next time!
        }
        else { //Put all the input bytes in a buffer
            inputString += inChar;
        }
    }
}
