/*
Simple sketch that output a 50% duty cycle PWM to an arbitrary pin.
Start it by serialing "start@[pin]" at 9600 baud, where [pin] is the pin you want to start.
Same with stopping.
No idea if it actually works, don't have an Anal to test it in.
*/

/*PWM*/
int cycle=127; //I'm assuming 50% PWM is nice
int duty[14]={0}; //I'm leaving space for every each pin.  Fuck switches.

/*Serial*/
String inputString=""; //Initiate Serial input command variables
String cmd="";
int val=0;
//float val=0;
//char valtemp[33]; //Reserve space for intermediate step in converting String to float
boolean stringComplete=false; //Don't run debuging info until a command is entered

void communicate() {
  cmd=inputString.substring(0,inputString.indexOf('@')); //Split the input into command and value. The only reason this isn't '=' is because I like '@'
  val=inputString.substring(inputString.indexOf('@')+1).toInt();
  //These are for float input instead.  Since I'm using it as an index, this is less useful for now:
  //inputString.substring(inputString.indexOf('@')+1).toCharArray(valtemp, sizeof(valtemp)); //Jankety conversion to floats
  //val=atof(valtemp);
  
  if (cmd.equals(String("start"))) {
    val=max(0,min(13,val)); //Hopefully this isn't needed..
    duty[val]=cycle;
    Serial.print("PWM ") && Serial.print(val) && Serial.println(" started.");
  }
  if (cmd.equals(String("stop"))) {
    val=max(0,min(13,val));
    duty[val]=0;
    Serial.print("PWM ") && Serial.print(val) && Serial.println(" stopped.");
  }
  
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {;} //some Arduinos take a while...
  inputString.reserve(128); //allows adding one byte at a time (it needs to be a buffer of sorts)
}

void loop() {
  if(stringComplete) {communicate();} //update PWM duty cycles
  analogWrite(3,duty[3]);
  analogWrite(5,duty[5]);
  analogWrite(6,duty[6]);
  analogWrite(9,duty[9]);
  analogWrite(10,duty[10]);
  analogWrite(11,duty[11]);
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
