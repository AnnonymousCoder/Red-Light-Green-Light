#include<Arduino.h>
#include<Servo.h>
#include<LiquidCrystal.h>

//---------------------------------------------------Constants--------------------------------------------------------------
#define START_BTN   23  // The pin the start button is connected to
#define RED_LED     25  // The pin the red light is connected to
#define GREEN_LED   27  // The pin the green light is connected to


#define ONE_SECOND                1000              // 1 second in miliseconds
#define RED_LIGHT_WAIT_TIME       ONE_SECOND * 3    // The time the robot should be on red light in seconds
#define GREEN_LIGHT_WAIT_TIME     ONE_SECOND * 2    // The time the robot should be on green light in seconds
#define PLAY_TIME   60

const struct BUZZER{
  char pin;
  
  int red_freq;
  int r_playTime;

  int green_freq;
  int g_playTime;

  int win_freq;
  int win_playTime;
}buzzer{.pin = 8, .red_freq = 4500, .r_playTime = 500, .green_freq = 2000, .g_playTime = 500, .win_freq=5000, .win_playTime=1000};

const struct LCD{
  char regSelect;
  char enable;
  char data[4]; // data[4] = [d4, d5, d6, d7]
}display{12, 11, {5, 4, 3, 2}};


//----------------------------------------------------Variables-----------------------------------------------------------
struct USONIC_SENSOR{
  const char trigger;
  const char echoPin;

  int findDistance() {return logDuration() * 0.034/2;}
  int logDuration()  {
    digitalWrite(trigger, LOW);
    delayMicroseconds(2);
    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);
    return pulseIn(echoPin, HIGH);
  }

}u_sensor{.trigger = 7, .echoPin = 6};

struct SERVO{
  char pin;
  short angle[2];

  SERVO(char a):pin(a), angle{0, 180} {}

}robot(9), eliminator(44);


LiquidCrystal lcd(display.regSelect, display.enable, display.data[0], display.data[1], display.data[2], display.data[3]);
Servo servo_1, servo_2;


float acceptedOffset  = 2;     // The accepted offset between the initial distance and the current measured distance of the player during red light.
float initialDistance = 0;     // The initial distance of the player when the robot is on red light.
float observeTime     = 100;   // The repeatable time the robot should observe the player for movement in miliseconds during red light.
float winnningDist    = 2;     // The distance the player should be from the robot to win the game.


int usCounter         = 0;            // microsecond counter
long gameCounter      = 0;            // Overall game counter in miliseconds
int redLightCounter   = 0;            // Counts the time the robot is on red light
int greenLightCounter = 0;            // Counts the time the robot is on green light
int gameCountDown     = PLAY_TIME;    // Counts down the time remaining in the game in seconds

String countDownText = "Time: " + String(gameCountDown); // Text to be displayed on the LCD about the time remaining in the game
String msg;

bool gameStart           = false;   // Indicates the start/end of the game
bool playerMoved         = false;   // Indicates if the player moved when the robot was on red light
bool clearField          = false;   // Indicates if the field should be cleared
bool redGreenLightToggle = false;   // Indicates if the robot is on red light or green light

bool lookOnlyOnce = true;           // Used to obtain the initial player position when the robot is on red light

//----------------------------------------------------Timer Interrupts-----------------------------------------------------------

ISR(TIMER1_COMPA_vect){
    OCR1A += 16;

    usCounter += 1;
}

ISR(TIMER1_COMPB_vect){
    OCR1B += 16000;

    gameCounter += 1;
    redLightCounter += 1;
    greenLightCounter += 1;
}

//------------------------------------------------------------------------------------------------------------------------------

/*
  @brief: Controls the movement of the robot located at the front of the play field.
          The robot consists of a servo motor with a ultrasonic sensor attached to it.
          The robot is a mimic of the female robot seen in the movie's red light green light scene. 

  @Params: None
  @Return: None
 */
void robotMovement(){
    if(!gameStart){return;} 

    if(!redGreenLightToggle && lookOnlyOnce){ // Obtains the initial player position when the robot is on red light
        delay(500);
        observeTime = u_sensor.logDuration();
        initialDistance = u_sensor.findDistance();
        lookOnlyOnce = false;
    }

    if(usCounter >= observeTime){ // Checks for player movement
        int currDist = u_sensor.findDistance(), currTime = u_sensor.logDuration();
        int distDiff = abs(currDist - initialDistance);
        int timeDiff = abs(currTime - observeTime    );

        if(!redGreenLightToggle && (distDiff != currDist) && (distDiff > acceptedOffset || distDiff < acceptedOffset)){
            Serial.println("Player Moved");
            playerMoved = true;
            clearField = true;
        }

        usCounter = 0;
    }

    if(!redGreenLightToggle){
        if(redLightCounter >= RED_LIGHT_WAIT_TIME){   //Prepare to switch to green light
            greenLightCounter = 0;
            digitalWrite(RED_LED, LOW);
            tone(buzzer.pin, buzzer.green_freq, buzzer.g_playTime);
            redLightCounter = 0;
            redGreenLightToggle = !redGreenLightToggle;
            digitalWrite(GREEN_LED, HIGH);
        }
    }else{
        redLightCounter = 0;
        lookOnlyOnce = true;
         if(greenLightCounter >= GREEN_LIGHT_WAIT_TIME){  //Prepare to switch to red light
            digitalWrite(GREEN_LED, LOW);
            tone(buzzer.pin, buzzer.red_freq, buzzer.r_playTime);
            greenLightCounter = 0;
            redGreenLightToggle = !redGreenLightToggle;
            digitalWrite(RED_LED, HIGH);
         }
    }

    servo_2.write(robot.angle[redGreenLightToggle]);
}

/*
  @brief: Tracks and updates the remaining game time.

  @params: None
  @return: None
*/
void showRemainingTime(){
  if(!gameStart){return;}


    if(gameCounter > ONE_SECOND){
        gameCountDown -= 1;
        gameCounter = 0;
        if(gameCountDown <= 0){
          gameCountDown = 60;
          gameCounter = 0;
          clearField = true;
        }
    }
  
  countDownText = "Time: " + String(gameCountDown);

  lcd.print(" ");
  lcd.home();
  lcd.print(countDownText);
}

/*
  @brief: Controls the movement of the eliminator located at the side of the play field.
          The eliminator consists of a servo motor that is used to eliminate players who move when the robot is on red light.

  @params: None
  @return: None
*/
void eliminatePlayer(){
  if(!gameStart) return;

  if(playerMoved || clearField){
    Serial.println("Elimination Started");
  }
  short angle = eliminator.angle[playerMoved || clearField];

  servo_1.write(angle);
  if((clearField || playerMoved)){
        clearField = false;
        playerMoved = false;
        gameStart = false;
        delay(500);
        servo_1.detach();
        Serial.println("GameStopped");
  }
}







void setup(){
  pinMode(u_sensor.trigger, OUTPUT);
  pinMode(u_sensor.echoPin, INPUT);
  pinMode(buzzer.pin, OUTPUT);


  pinMode(RED_LED, OUTPUT);
  pinMode(START_BTN, INPUT);
  pinMode(GREEN_LED, OUTPUT);


  Serial.begin(9600);

  servo_1.attach(eliminator.pin, 500, 2480);
  servo_2.attach(robot.pin);

  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(countDownText);

//--------TIMER1 Interrupt Setup--------
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= B00000001;
  OCR1A = 16;
  OCR1B = 16000;
  TIMSK1 |= B00000110;


  servo_1.write(eliminator.angle[0]);
  servo_2.write(robot.angle[0]);
}







void loop(){

    if(!gameStart && digitalRead(START_BTN)){ 
        if(!servo_1.attached()){ servo_1.attach(eliminator.pin);}
        
        gameStart = true;
        gameCountDown = 60;
        gameCounter = 0;
        greenLightCounter = 0;
        redLightCounter = 0;
        initialDistance = 0;
        observeTime = 0;
        redGreenLightToggle = false;

        digitalWrite(RED_LED, HIGH);
        tone(buzzer.pin, buzzer.red_freq, buzzer.r_playTime);
        
        Serial.println("Game Started"+String(gameCounter));
    }

    showRemainingTime();
    eliminatePlayer();
    robotMovement();
}