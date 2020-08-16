/**
Spawn Timer

Silently counts down for a user configurable amount of time
with increasingly rapid "countdown" style beeping starting when there is
one minute remaining, ending with a loud beep. The timer resets itself,
resulting in a signal exactly X seconds apart.

Install on any Arduino compatible device (tested on Adafruit Trinket).

Schematic:
- Button connected between Vcc pin and BUTTON pin.
- Signal buzzer connected between GND and BUZZER pin. Current implementation assumes an active buzzer, edit beep() if passive buzzer is to be used.
- Smaller warning/signal buzzer between GND and WEAK_BUZZER pin. Current implementation assumes a passive buzzer, edit weakBeep() if active buzzer is to be used.
- (Optional) artificial load resistor (Potentiometer set to approx 50-200 Ohms recommended) between GND and ARTIFICIAL_LOAD pin 
if the device is to be powered through a USB power bank. Without this artificial load any half decent power bank will shut off
after 20 seconds because the device draws too little current.

Behavior:
- After completed bootup the device will trigger a short beep on both buzzers. At this point the device is waiting
for the user to set the timer.
- Pressing the button increases the timer by 30 seconds each press (pressing the button 3 times sets the timer for 1m30s, 6 presses sets it to 3m, etc.)
- After CONFIG_TIMEOUT ms of no user input the weak buzzer will repeat the setting back to the user in the following way:
	a long beep for each full minute, followed by a short beep if it is also set to 30sec 
	(timer set to 1m30s results in one long beep and one short, 3m setting results in 3 long beeps, etc.).
- At this time the device is fully configured and is waiting to be started.
- Pressing the button starts the timer, which is confirmed by a short beep on the weak buzzer.
	
- When there is less than one minute remaining the weak buzzer will give increasingly more rapid, short beeps.
- When the timer har reached 0 the main buzzer will give a constant signal for SPAWN_SIGNAL_LENGTH milliseconds.
- The timer will then reset itself and repeat the countdown.


MIT License

Copyright (c) 2020 Lukas Mattsson

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

**/

// pins
const int BUZZER = 0;
const int WEAK_BUZZER = 2;
const int BUTTON = 4;
const int ARTIFICIAL_LOAD = 3;

const unsigned long CONFIG_TIMEOUT = 3000;
const unsigned long SPAWN_SIGNAL_LENGTH = 3000;

unsigned long lastKeyPress = millis();
boolean configured = false;
boolean timerRunning = false;

int clickCount = 0;
const int clickValue = 30; // seconds to add to the timer for each click

unsigned long timerSetting = 0; // ms between spawn events
unsigned long nextSpawn = millis();

boolean buttonWasPressed = false;

void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(WEAK_BUZZER, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(ARTIFICIAL_LOAD, OUTPUT);
  weakBeep(100);
  bleedingDelay(500);
  beep(100);
}

void loop() {
  if (!configStarted() || buttonState()) {
    postponeTimeout();
  }
  if (userDoneConfiguring() && !configCompleted()) {
    setConfig();
    repeatConfigToUser();
  }
  
  if (timerRunning) {
    awaitRespawn();
    signalSpawn();
    setNextSpawnTime();
  } else if (configCompleted() && buttonPressed()) {
    signalTimerStarted();
    startTimer();
  } else if (buttonPressed()) {
    incrementTimer();
  }
  buttonWasPressed = buttonState();
  bleedingDelay(50);
}


void bleedingDelay(int ms) {
  for (int i = 0; i < ms/50; ++i) {
    bleedCurrent(25);
    delay(25);
  }
}

void bleedCurrent(int ms) {
  digitalWrite(ARTIFICIAL_LOAD, HIGH);
  delay(ms);
  digitalWrite(ARTIFICIAL_LOAD, LOW);
}

void incrementTimer() {
    clickCount++;
}

boolean userDoneConfiguring() {
  return configStarted() && timeoutReached();
}

void signalTimerStarted() {
  weakBeep(100);
}

void startTimer() {
  setFirstSpawnTime();
  timerRunning = true;
}

void setNextSpawnTime() {
  nextSpawn += timerSetting;
}

void setFirstSpawnTime() {
  nextSpawn = millis() + timerSetting;
}

void signalSpawn() {
  beep(SPAWN_SIGNAL_LENGTH);
}

void awaitRespawn() {
  unsigned long now = millis();
  unsigned long spawnNow = nextSpawn - 500ul;
  unsigned long spawnImminent = nextSpawn - 1500ul;
  unsigned long spawnReallyReallyClose = nextSpawn - 3000ul;
  unsigned long spawnReallyClose = nextSpawn - 7000ul;
  unsigned long spawnCloser = nextSpawn - 15000ul;
  unsigned long spawnClose = nextSpawn - 30000ul;
  unsigned long spawnSomewhatClose = nextSpawn - 60000ul;

  while ((now = millis()) < nextSpawn) {

    if (now >= spawnSomewhatClose) {
      weakBeep(50);
    }

    if (now >= spawnNow) {
      delay(25);
    } else if (now >= spawnImminent) {
      bleedingDelay(50);
    } else if (now >= spawnReallyReallyClose) {
      bleedingDelay(250);
    } else if (now >= spawnReallyClose) {
      bleedingDelay(500);
    } else if (now >= spawnCloser) {
      bleedingDelay(1000);
    } else if (now >= spawnClose) {
      bleedingDelay(2000);
    } else if (now >= spawnSomewhatClose) {
      bleedingDelay(10000);
    } else {
      bleedingDelay(500);
    }
  }
}

void weakBeep(int ms) {
  for (int i = 0; i < ms/2; ++i) {
    digitalWrite(WEAK_BUZZER, HIGH);
    delay(1);
    digitalWrite(WEAK_BUZZER, LOW);
    delay(1);
  }
}

void setConfig() {
  timerSetting = 1000ul * clickValue * clickCount;
}

boolean configStarted() {
  return clickCount > 0;
}

void postponeTimeout() {
  lastKeyPress = millis();
}

boolean timeoutReached() {
  return millis() - lastKeyPress > CONFIG_TIMEOUT;
}

void repeatConfigToUser() {
  int configAsSeconds = clickCount * clickValue;

  int wholeMinutes = configAsSeconds / 60;
  int leftoverSeconds = configAsSeconds % 60;

  for (int i = 0; i < wholeMinutes; ++i) {
    beepMinute();
  }

  if (leftoverSeconds > 0) {
    beepHalfMinute();
  }
}

void beepMinute() {
  weakBeep(1000);
  bleedingDelay(500);
}

void beepHalfMinute() {
  weakBeep(300);
}

boolean configCompleted() {
  return timerSetting > 0;
}

boolean buttonPressed() {
  return buttonState() && !buttonWasPressed;
}

boolean buttonState() {
  return digitalRead(BUTTON) == HIGH;
}

void beep(int ms) {
  digitalWrite(BUZZER, HIGH);
  delay(ms);
  digitalWrite(BUZZER, LOW);
}
