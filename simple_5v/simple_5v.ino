/*
Simple sketch that outputs 5V to an arbitrary pin.
Start it by serialing "start@[pin]" at 9600 baud, where [pin] is the pin you want to start.
Same with stopping.
*/

int values[14]={0};
int i;

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
    values[val]=1;
    Serial.print("Pin ") && Serial.print(val) && Serial.println(" enabled.");
  }
  if (cmd.equals(String("stop"))) {
    val=max(0,min(13,val));
    values[val]=0;
    Serial.print("Pin ") && Serial.print(val) && Serial.println(" disabled.");
  }
  
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
  if(stringComplete) {communicate();} //update values
  for(i=0;i<14;i++) { //write values
    digitalWrite(i,values[i]);
  }
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
