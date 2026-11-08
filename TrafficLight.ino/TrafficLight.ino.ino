/*
  ======================================================================
   TRAFFIC LIGHT + 7-SEGMENT COUNTDOWN + NIGHT MODE
  ======================================================================

  WHAT THE PROGRAM DOES
  ---------------------
  Normal mode: four phases repeat forever. In every phase the active LED(s)
  blink (1 second on, 1 second off) while the 7-segment display counts down.

      Phase 0   RED             5 s    display shows 5 4 3 2 1
      Phase 1   RED + YELLOW    3 s    display shows 3 2 1   (both blink together)
      Phase 2   YELLOW          3 s    display shows 3 2 1
      Phase 3   GREEN           5 s    display shows 5 4 3 2 1
      ...then it goes back to phase 0.

  Night mode: while the switch is in the night position the display is
  blank and only the YELLOW LED blinks. When the switch is moved back,
  the normal cycle starts again from the beginning of phase 0 (red).

  WIRING (what each pin is connected to)
  --------------------------------------
    Pins 2..8   segments a, b, c, d, e, f, g of the 7-segment display, in
                this order. The code assumes a COMMON-CATHODE display
                (HIGH = segment ON). For a common-anode display the
                patterns would have to be inverted.
    Pin 9       RED LED     (HIGH = on)
    Pin 10      YELLOW LED  (HIGH = on)
    Pin 11      GREEN LED   (HIGH = on)
    Pin 12      night-mode switch; the other side of the switch goes to GND

  Segment names, for reading the digit table below:

        --a--
       f|   |b
        --g--
       e|   |c
        --d--

  HOW THE TIMING WORKS
  --------------------
  The code never uses delay(). loop() runs thousands of times per second
  and each time asks: "have 1000 ms passed since the last tick?". Once a
  second it does one "tick": update the display and toggle the LEDs.
  Because the loop is never blocked, the switch is checked all the time,
  so night mode reacts immediately.
*/


// ---------------------------------------------------------------------
//  7-segment digit patterns
// ---------------------------------------------------------------------
// digits[n][s] says whether segment s must be ON (1) or OFF (0) to show
// the digit n. The columns are the segments in this order: a b c d e f g
int digits[10][7] = {
                     {1, 1, 1, 1, 1, 1, 0},                       //0   a b c d e f
                     {0, 1, 1, 0, 0, 0, 0},                       //1   b c
                     {1, 1, 0, 1, 1, 0, 1},                       //2   a b d e g
                     {1, 1, 1, 1, 0, 0, 1},                       //3   a b c d g
                     {0, 1, 1, 0, 0, 1, 1},                       //4   b c f g
                     {1, 0, 1, 1, 0, 1, 1},                       //5   a c d f g
                     {1, 0, 1, 1, 1, 1, 1},                       //6   a c d e f g
                     {1, 1, 1, 0, 0, 0, 0},                       //7   a b c
                     {1, 1, 1, 1, 1, 1, 1},                       //8   all segments
                     {1, 1, 1, 1, 0, 1, 1}                        //9   a b c d f g
};


// ---------------------------------------------------------------------
//  Pins and settings
// ---------------------------------------------------------------------
// segmentPins[s] = Arduino pin connected to segment s (a, b, c, d, e, f, g).
// The order matches the columns of the digits table above.
const int segmentPins[7] = {2, 3, 4, 5, 6, 7, 8};

// ledPins[i] = Arduino pin of LED number i.
// The LED numbering is used everywhere (ledState[] and Blink() too):
// 0 = red, 1 = yellow, 2 = green.
const int ledPins[3] = {9, 10, 11};

// The level on pin 12 that means "night mode".
// Pin 12 uses INPUT_PULLUP, so it reads HIGH when the switch is open and
// LOW when the switch connects it to GND.
const int NIGHT_LEVEL = HIGH;

// How long each phase lasts, in seconds (phase 0, 1, 2, 3).
const int phaseSeconds[4] = {5, 3, 3, 5};

// Which LED(s) blink in each phase. Letters: r = red, y = yellow, g = green.
// "ry" means red and yellow blink together. Blink() reads these letters.
const char *phaseLeds[4]    = {"r", "ry", "y", "g"};


// ---------------------------------------------------------------------
//  Program state (variables that change while the program runs)
// ---------------------------------------------------------------------
// ledState[i] remembers if LED i is ON (HIGH) or OFF (LOW).
// Blink() needs it to know what to switch the LED to next.
int ledState[3] = {LOW , LOW, LOW};

// The millis() time of the last one-second tick.
unsigned long previousMillis = 0;

// currentPhase: 0 = red, 1 = red+yellow, 2 = yellow, 3 = green,
//               4 = night mode (only yellow blinks)
// second:       the number the display will show on the NEXT tick. It
//               counts down. It starts at 5 because the program starts
//               in phase 0 (red, 5 seconds).
int currentPhase = 0, second = 5;


// Function declarations (prototypes), so loop() can call the functions
// that are written further down in the file.
void displayDigit(int number);
void clearDisplay();
void Blink(const char *blink);
void turnOffAllLeds();


// ---------------------------------------------------------------------
//  setup() runs once when the Arduino starts
// ---------------------------------------------------------------------
void setup() {
  // Pins 2..8: the seven display segments, all outputs
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);

  // Pins 9..11: the three LEDs, all outputs
  pinMode(9, OUTPUT);       //RED
  pinMode(10, OUTPUT);      //YELLOW
  pinMode(11, OUTPUT);      //GREEN

  // Pin 12: the night-mode switch. INPUT_PULLUP turns on the Arduino's
  // internal resistor, so no external resistor is needed: the pin reads
  // HIGH when the switch is open and LOW when it connects to GND.
  pinMode(12, INPUT_PULLUP);

  // Start with every LED and the display switched off
  turnOffAllLeds();
  clearDisplay();

}

//_____________________________________________________________________________________________________________________________________________________________________________________________________

// ---------------------------------------------------------------------
//  loop() runs again and again, forever
// ---------------------------------------------------------------------
void loop() {
  // Time since the Arduino started, in milliseconds
  unsigned long currentMillis = millis();

  // STEP 1: read the switch.  true = switch is in the night position.
  bool night = (digitalRead(12) == NIGHT_LEVEL);

  // STEP 2: check whether the switch position and the current mode disagree.
  // The transition happens only when the program needs to enter or exit Night Mode.

  // The switch was just turned to night mode:
  if (night && currentPhase != 4){
    currentPhase = 4;                         // remember that we are in night mode
    turnOffAllLeds();                         // start from a clean state
    clearDisplay();                           // night mode shows no number
    previousMillis = currentMillis - 1000;    // pretend a full second has already passed,
                                              // so the first yellow blink happens right now
                                              // instead of after a second of darkness
  }
  // The switch was just turned back to normal mode:
  else if (!night && currentPhase == 4){
    turnOffAllLeds();
    second = 5;                               // red phase lasts 5 seconds
    currentPhase = 0;                         // restart the cycle from RED
    previousMillis = currentMillis - 1000;    // same trick: red starts right now
  }

  // STEP 3: the one-second tick. Everything inside runs once per second.
  // (Unsigned subtraction also stays correct when millis() overflows.)
  if (currentMillis - previousMillis >= 1000){
    previousMillis = currentMillis;           // remember when this tick happened

    // Night mode: only toggle the yellow LED, the display stays blank
    if (currentPhase == 4){
      Blink("y");
    }

    // Normal mode
    else {
      // second == 0 means the last phase already showed its final number (1),
      // so it is time to move on to the next phase. That is why 0 is never
      // displayed: this tick shows the first number of the new phase instead.
      if (second == 0){
        currentPhase = (currentPhase + 1) % 4;   // 0 -> 1 -> 2 -> 3 -> back to 0
        second = phaseSeconds[currentPhase];     // load the length of the new phase
        turnOffAllLeds();                        // every phase starts with all LEDs off,
                                                 // so the first blink is always "on"
      }

      displayDigit(second);                      // show the current countdown number
      Blink(phaseLeds[currentPhase]);            // toggle the LED(s) of this phase
      second--;                                  // the next tick shows one number less
    }
  }

}


//_____________________________________________________________________________________________________________________________________________________________________________________________________

// Shows a digit (0..9) on the 7-segment display.
// Any other value (for example -1) clears the display instead.
void displayDigit(int number){

  // Safety check: only 0..9 can be shown
  if (number > 9 || number < 0){
    clearDisplay();
    return;
  }

  // Go through the 7 segments (a..g) and switch each one ON or OFF
  // according to the pattern stored in the digits table.
  for(int column=0 ; column < 7 ; column++){
    digitalWrite(segmentPins[column], digits[number][column]);
  }
}

//_____________________________________________________________________________________________________________________________________________________________________________________________________

// Turns all 7 segments off, so the display is blank.
// Pins 2..8 are the segment pins.
void clearDisplay(){

  for(int pin=2 ; pin<=8 ; pin++){
    digitalWrite(pin, LOW);
  }

}

//_____________________________________________________________________________________________________________________________________________________________________________________________________

// Toggles (ON -> OFF, or OFF -> ON) every LED named in the text.
// Example: Blink("y") toggles yellow, Blink("ry") toggles red and yellow
// at the same time. Called once per second, so each LED stays on for one
// second, then off for one second, and so on.
void Blink(const char *blink) {   // R - Y - G

  // Go through the text letter by letter, until the end of the text ('\0')
  for (int i = 0; blink[i] != '\0'; i++) {

    // Turn the letter into an LED number (the index in ledPins / ledState)
    int index;
    if (blink[i] == 'r') {
      index = 0;
    }
    else if (blink[i] == 'y') {
      index = 1;
    }
    else if (blink[i] == 'g') {
      index = 2;
    }
    else{
      continue;            // unknown letter: ignore it and go to the next one
    }

    ledState[index] = !ledState[index];            // flip the remembered state
    digitalWrite(ledPins[index], ledState[index]); // apply it to the real LED
  }

}
//_____________________________________________________________________________________________________________________________________________________________________________________________________

// Turns all three LEDs off and resets their remembered state,
// so the next Blink() call turns them ON.
void turnOffAllLeds() {

    for (int i = 0; i < 3; i++) {
        ledState[i] = LOW;
        digitalWrite(ledPins[i], LOW);
    }
}
