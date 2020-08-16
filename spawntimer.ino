// pins
int buzzer = 0;
int button = 4;
int internalBuzzer = 2;
int artificialLoad = 3;

unsigned long CONFIG_TIMEOUT = 3000;
unsigned long lastKeyPress = millis();

boolean configured = false;
boolean timerRunning = false;

int clickCount = 0;
int clickValue = 30; // seconds to add to the timer for each click

unsigned long timerSetting = 0; // ms between spawn events
unsigned long nextSpawn = millis();
unsigned long spawnSignalLength = 3000;
boolean buttonWasPressed = false;

void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(internalBuzzer, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  pinMode(artificialLoad, OUTPUT);
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
  digitalWrite(artificialLoad, HIGH);
  delay(ms);
  digitalWrite(artificialLoad, LOW);
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
  beep(spawnSignalLength);
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
      bleedingDelay(1000);
    }
  }
}

void weakBeep(int ms) {
  for (int i = 0; i < ms/2; ++i) {
    digitalWrite(internalBuzzer, HIGH);
    delay(1);
    digitalWrite(internalBuzzer, LOW);
    delay(1);
  }
}

void setConfig() {
  unsigned long sToMs = 1000;
  timerSetting = sToMs * clickValue * clickCount;
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
  return digitalRead(button) == HIGH;
}

void beep(int ms) {
  digitalWrite(buzzer, HIGH);
  delay(ms);
  digitalWrite(buzzer, LOW);
}
