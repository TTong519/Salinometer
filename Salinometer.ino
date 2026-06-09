#include <Wire.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(11,10,2,3,4,9); //Pins

// --- Settings ---
int analogPin = 0;
int buttonPin = 5;
int dtime = 500; // Delay time
int samples = 100; //Measurement sample size
float Vin = 5.0;
float R1 = 1000.0;

// --- Variables ---
bool isRunning = false;      // false = Stopped, true = Measuring
int buttonState;             // Current reading from the input pin
int lastButtonState = HIGH;  // Previous reading (default HIGH for PULLUP)

unsigned long lastDebounceTime = 0;  
unsigned long debounceDelay = 50;    // 50ms to filter noise

void setup(){
  pinMode(8, OUTPUT); 
  pinMode(7, OUTPUT);
  pinMode(12, INPUT);
  pinMode(buttonPin, INPUT_PULLUP); 
  Serial.begin(9600); 
  lcd.begin(16, 2); 
  lcd.clear(); 
  lcd.print("Ready");
}

void loop(){
  // 1. READ BUTTON
  int reading = digitalRead(buttonPin);

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // Only check if enough time has passed (Debounce)
  if ((millis() - lastDebounceTime) > debounceDelay) {
    
    // If the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // If the new state is LOW, it means the button was just pressed
      if (buttonState == LOW) {
        isRunning = !isRunning;

        // Update LCD immediately upon press
        lcd.clear();
        if (isRunning) {
          lcd.print("Measuring...");
          Serial.println("STARTED");
        } else {
          lcd.print("Stopped");
          Serial.println("STOPPED");
          delay(500);
          lcd.clear();
          lcd.print("Ready");
        }
      }
    }
  }
  
  // Save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;


  // 2. MEASURE (Only if isRunning is true)
  if (isRunning) {
    float tot = 0;
    float avg = 0;
    int raw = 0;
    float Vout = 0;
    float R2 = 0;
    float buff = 0;

    // Measurement batch
    for (int i = 0; i < samples; i++) {
      digitalWrite(7, HIGH);
      digitalWrite(8, LOW);
      delayMicroseconds(dtime);
      digitalWrite(7, LOW);
      digitalWrite(8, HIGH);
      delayMicroseconds(dtime);
      
      raw = analogRead(analogPin);
      
      if(raw > 0){
        buff = raw * Vin;
        Vout = (buff)/1024.0;
        // Prevent division by zero
        if(Vout > 0.01) { 
           buff = (Vin/Vout) - 1;
           R2 = R1 * buff;
           tot = tot + R2;
        }
      }
    }
    
    // Calculate and print
    if (tot > 0) {
        avg = tot / samples;
        // Salt percent formula
        float calcVal = ((-151.00/860.00)*avg) + (514481.00/10750.00) + 0.8;
    
        lcd.setCursor(0, 0);
        lcd.print("R: ");
        lcd.print((int)avg);
        
        lcd.setCursor(0, 1);
        lcd.print("Val: ");
        lcd.print(calcVal);
    }
  }
}