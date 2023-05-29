#include "myHeader.hpp"

void getBluetoothMessage() {
  if (mySerial.available()) {
    char c = mySerial.read();
    if (c == 'o') {
      triggerDoorUnlock();
    } 
    else if (c == 'c') {
      triggerDoorLock();
    }
    if (c == 's') {
      // get the slider value to know how long to sing
      uint8_t sliderValue = mySerial.read();
      Serial.println(sliderValue, DEC);
      // play the motor for the time of the slider value
      motorSinging(sliderValue);
    }
  }
}
// Records the timing of knocks.
void listenToSecretKnock(){
  int i = 0;
  memset(knockReadings, 0, maximumKnocks);
  
  int currentKnockNumber = 0;
  int startTime = millis();
  int now = millis();
  
  SET_LOW(PORTD, greenLED); // we blink the green LED for a bit as a visual indicator of the knock.
  if (programButtonPressed == true){
    SET_LOW(PORTD, redLED); // and the red one too if we're programming a new knock.
  }
  delay(knockFadeTime);// wait for this peak to fade before we listen to the next one.
  SET_HIGH(PORTD, greenLED);
  if (programButtonPressed == true){
    SET_HIGH(PORTD, redLED);
  }
  do {
    //listen for the next knock or wait for it to timeout. 
    knockSensorValue = analogRead(knockSensor);
    if (knockSensorValue >= threshold){                   //got another knock...
      //record the delay time.
      now = millis();
      knockReadings[currentKnockNumber] = now - startTime;
      currentKnockNumber ++;                             //increment the counter
      startTime = now;          
      // and reset our timer for the next knock
      SET_LOW(PORTD, greenLED);
      if (programButtonPressed == true){
        SET_LOW(PORTD, redLED);
      }
      delay(knockFadeTime);                              // again, a little delay to let the knock decay.
      SET_HIGH(PORTD, greenLED);
      if (programButtonPressed == true){
        SET_HIGH(PORTD, redLED);
      }
    }

    now = millis();
    
    //did we timeout or run out of knocks?
  } while ((now-startTime < knockComplete) && (currentKnockNumber < maximumKnocks));
  
  //we've got our knock recorded, lets see if it's valid
  if (programButtonPressed == false){             // only if we're not in progrmaing mode.
    if (validateKnock() == true){
      triggerDoorUnlock(); 
    } else {
      SET_LOW(PORTD, redLED); // We didn't unlock, so blink the red LED as visual feedback.
      for (i = 0; i < 4 ;i ++){					
        SET_HIGH(PORTD, redLED);
        delay(100);
        SET_LOW(PORTD, redLED);
        delay(100);
      }
      SET_HIGH(PORTD, greenLED);
    }
  } else { // if we're in programming mode we still validate the lock, we just don't do anything with the lock
    validateKnock();
    // and we blink the green and red alternately to show that program is complete.
    SET_LOW(PORTD, redLED);
    SET_HIGH(PORTD, greenLED);
    for (i=0;i<3;i++){
      delay(100);
      SET_HIGH(PORTD, redLED);
      SET_LOW(PORTD, greenLED);
      delay(100);
      SET_LOW(PORTD, redLED);
      SET_HIGH(PORTD, greenLED);
    }
  }
}

// Runs the motor to play the musical box
void motorSinging(int sliderValue) {
  SET_HIGH(PORTD, lockMotor);
  delay(sliderValue * 100);
  SET_LOW(PORTD, lockMotor);
}

// Runs the servos to unlock the door.
void triggerDoorUnlock(){
  int i=0;
  
  SET_HIGH(PORTD, greenLED); // turn on the green LED so we know we're unlocked.
  
  myServo1.attach(servoMotor1);                // Attach the servo to the pin
  myServo2.attach(servoMotor2);                // Attach the servo to the pin
  Serial.println(myServo1.read());
  Serial.println(myServo2.read());
  Serial.println("Writing 90");

  myServo1.write(65);               // Turn the servo to unlock the door
  myServo2.write(125);               // Turn the servo to unlock the door
  
  delay (lockTurnTime);                    // Wait a bit.

  myServo1.detach();                        // Detach the servo from the pin
  myServo2.detach();                        // Detach the servo from the pin
  isBoxLocked = false;
  
  // Blink the green LED a few times for more visual feedback.
  for (i = 0; i < 5; i ++){   
    SET_LOW(PORTD, greenLED);
    delay(100);
    SET_HIGH(PORTD, greenLED);
    delay(100);
  }
   
}

void triggerDoorLock() {
  myServo1.attach(servoMotor1);
  myServo2.attach(servoMotor2);

  Serial.println(myServo1.read());
  Serial.println(myServo2.read());
  myServo1.write(0);// Writes 0 to lock
  myServo2.write(0);

  isBoxLocked = true;
  delay (lockTurnTime);// Wait a bit.
  myServo1.detach(); 
  myServo2.detach();
}

// Sees if our knock matches the secret.
boolean validateKnock(){
  int i = 0;
 
  // simplest check first: Did we get the right number of knocks?
  int currentKnockCount = 0;
  int secretKnockCount = 0;
  int maxKnockInterval = 0; // We use this later to normalize the times.
  
  for (i = 0; i < maximumKnocks; i++){
    if (knockReadings[i] > 0){
      currentKnockCount ++;
    }
    if (secretCode[i] > 0){
      secretKnockCount ++;
    }
    
    if (knockReadings[i] > maxKnockInterval){ 	// collect normalization data while we're looping.
      maxKnockInterval = knockReadings[i];
    }
  }
  
  // If we're recording a new knock, save the info and get out of here.
  if (programButtonPressed == true){
      for (i = 0; i < maximumKnocks; i ++){ // normalize the times
        secretCode[i]= map(knockReadings[i], 0, maxKnockInterval, 0, 100); 
      }
      // And flash the lights in the recorded pattern to let us know it's been programmed.
      SET_LOW(PORTD, greenLED);
      SET_LOW(PORTD, redLED);
      delay(1000);
      SET_HIGH(PORTD, greenLED);
      SET_HIGH(PORTD, redLED);
      delay(50);
      for (i = 0; i < maximumKnocks ; i ++){
        SET_LOW(PORTD, greenLED);
        SET_LOW(PORTD, redLED);
        // only turn it on if there's a delay
        if (secretCode[i] > 0){                                   
          delay(map(secretCode[i],0, 100, 0, maxKnockInterval)); // Expand the time back out to what it was.  Roughly.
          SET_HIGH(PORTD, greenLED);
          SET_HIGH(PORTD, redLED);
        }
        delay(50);
      }
	  return false; 	// We don't unlock the door when we are recording a new knock.
  }
  
  if (currentKnockCount != secretKnockCount){
    return false; 
  }
  
  /*  Now we compare the relative intervals of our knocks, not the absolute time between them.
      (ie: if you do the same pattern slow or fast it should still open the door.)
      This makes it less picky, which while making it less secure can also make it
      less of a pain to use if you're tempo is a little slow or fast. 
  */
  int totaltimeDifferences = 0;
  int timeDiff = 0;
  for (i = 0; i < maximumKnocks; i++){ // Normalize the times
    knockReadings[i]= map(knockReadings[i], 0, maxKnockInterval, 0, 100);      
    timeDiff = abs(knockReadings[i]-secretCode[i]);
    if (timeDiff > rejectValue){ // Individual value too far out of whack
      return false;
    }
    totaltimeDifferences += timeDiff;
  }
  // It can also fail if the whole thing is too inaccurate.
  if (totaltimeDifferences / secretKnockCount > averageRejectValue){
    return false; 
  }
  
  return true;
}
//--------------------------------------------------------------------------------
void setup() {
  SET_OUTPUT(DDRD, lockMotor);
  SET_OUTPUT(DDRD, redLED);
  SET_OUTPUT(DDRD, greenLED);
  SET_INPUT(DDRD, programSwitch);
  SET_PULLUP(PORTD, programSwitch);
  SET_INPUT(DDRD, topSwitch);
  SET_PULLUP(PORTD, topSwitch);
  
  mySerial.begin(9600);// Start serial communication for the bluetooth module.
  
  SET_HIGH(PORTD, greenLED); // Green LED on, we're ready to go!
  myServo1.attach(servoMotor1);
  myServo2.attach(servoMotor2);
  myServo1.write(0);
  myServo2.write(0);
  myServo1.detach();
  myServo2.detach();  
}

void loop() {
  getBluetoothMessage();
  isSwitchOpen = READ(PIND, topSwitch);
  if (isSwitchOpen == HIGH) {
    if (isBoxLocked == false) {  // is the box unlocked?
      delay(lockTurnWait);     // wait for a bit before locking it back
      if (READ(PIND, topSwitch) == HIGH) { // is it still closed?
        triggerDoorLock();
        isBoxLocked = true;
      }
    }
  }
  knockSensorValue = analogRead(knockSensor);

  if (READ(PIND, programSwitch) == LOW){  // is the program button pressed?                                       // this button works in negative logic, so LOW means the button is pressed.  
    programButtonPressed = true;          // Yes, so lets save that state
    SET_HIGH(PORTD, redLED);              // and turn on the red light too so we know we're programming.
  } else {
    programButtonPressed = false;
    SET_LOW(PORTD, redLED);               // turn off the red light
  }
  if (knockSensorValue >= threshold){
    listenToSecretKnock();
  }
}
