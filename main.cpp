//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"

 //=====[Declaration and initialization of public global objects]===============
AnalogIn potentiometer(A0);
AnalogIn lm35(A1); // 10 mV/\xB0 C
PwmOut buzzer(D5);
AnalogIn mq2(A2);
DigitalOut led(LED3);
#define NUMBER_OF_KEYS                           4
#define BLINKING_TIME_GAS_ALARM               1000
#define BLINKING_TIME_OVER_TEMP_ALARM          500
#define BLINKING_TIME_GAS_AND_OVER_TEMP_ALARM  100
#define NUMBER_OF_AVG_SAMPLES                   100
#define OVER_TEMP_LEVEL                         50
#define TIME_INCREMENT_MS                       10
#define DEBOUNCE_KEY_TIME_MS                    40
#define KEYPAD_NUMBER_OF_ROWS                    4
#define KEYPAD_NUMBER_OF_COLS                    4
#define EVENT_MAX_STORAGE                      100
#define EVENT_NAME_MAX_LENGTH                   14
 UnbufferedSerial uartUsb(USBTX, USBRX, 115200);
//=====[Declaration and initialization of public global variables]=============
typedef enum {
    MATRIX_KEYPAD_SCANNING,
    MATRIX_KEYPAD_DEBOUNCE,
    MATRIX_KEYPAD_KEY_HOLD_PRESSED
} matrixKeypadState_t;
char receivedChar = '\0';
char str[100] = "";
char alrm1[100] = "";
char alrm2[100] = "";
char alrm3[100] = "";
int yo = 0;
char alrm4[100] = "";
char alrm5[100] = "";
char codeSequence[NUMBER_OF_KEYS]   = { '1', '5', '9', '0' };
char keyPressed[NUMBER_OF_KEYS] = { '0', '0', '0', '0' };
typedef struct systemEvent {
    time_t seconds;
    char typeOfEvent[EVENT_NAME_MAX_LENGTH];
} systemEvent_t;

DigitalOut keypadRowPins[KEYPAD_NUMBER_OF_ROWS] = {PB_3, PB_5, PC_7, PA_15};
DigitalIn keypadColPins[KEYPAD_NUMBER_OF_COLS]  = {PB_12, PB_13, PB_15, PC_6};

 bool quit = false;
     int cony = 0;
     int pas = 0;

float lm35Reading = 0.0; // Raw ADC input A0 value
float lm35TempC = 0.0;   // Temperature in Celsius degrees [\xB0 C]
float lm35TempF = 0.0;   // Temperature in Fahrenheit degrees [\xB0 F]
int numberOfHashKeyReleasedEvents = 0;
float gasread = 0.0;
float tempread = 0.0;
float potentiometerReading = 0.0;   // Raw ADC input A1 value
float potentiometerScaledToC = 0.0; // Potentiometer value scaled to Celsius degrees [\xB0 C]
float potentiometerScaledToF = 0.0; // Potentiometer value scaled to Fahrenheit degrees [\xB0 F]
int accumulatedDebounceMatrixKeypadTime = 0;
int matrixKeypadCodeIndex = 0;
int code = 0;
char matrixKeypadLastKeyPressed = '\0';
char matrixKeypadIndexToCharArray[] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D',
};
matrixKeypadState_t matrixKeypadState;

int eventsIndex            = 0;
systemEvent_t arrayOfStoredEvents[EVENT_MAX_STORAGE];

bool showKeypadInUart = true;
struct tm rtcTime;
            
//=====[Declarations (prototypes) of public functions]=========================
void time();
void timread();
void inputsInit();
 void uartTask();
void Alarm();
bool areEqual();
void matrixKeypadInit();
char matrixKeypadUpdate();
void keypadToUart();
void eventLogUpdate();
char matrixKeypadScan();
void systemElementStateUpdate( bool lastState,
                               bool currentState,
                               const char* elementName );
 float analogReadingScaledWithTheLM35Formula( float analogReading );;
 float potentiometerScaledToCelsius( float analogValue );
void pcSerialComStringWrite( const char* str );
char pcSerialComCharRead();
void pcSerialComStringRead( char* str, int strLength );
void pcSerialComCharWrite( char chr );
//=====[Main function, the program entry point after power on or reset]========

int main()
{   
    int n = 0; 
    inputsInit();
    while( true ) { 
                char str[100] = "";
                lm35Reading = lm35.read();                
                lm35TempC = analogReadingScaledWithTheLM35Formula(lm35Reading);
                delay(1000);
             gasread = mq2.read();
                delay(1000); 
                Alarm();                          
     }
    }
 
void timread(){
            pcSerialComStringWrite("\n 5 Past alarm dates:\r\n");
            pcSerialComStringWrite( alrm1 );
            pcSerialComStringWrite( alrm2 );
            pcSerialComStringWrite( alrm3 );
            pcSerialComStringWrite( alrm4 );
            pcSerialComStringWrite( alrm5 );
}
    int i = 0;
void timer(){
    time_t epochSeconds;
            epochSeconds = time(NULL);
            switch(i){
                case 1:
                alrm1[0] = '\0';
                break;
                case 2:
                alrm2[0] = '\0';
                break;
                case 3:
                alrm3[0] = '\0';
                break;
                case 4:
                alrm4[0] = '\0';
                break;
                case 5:
                alrm5[0] = '\0';
                i = 0;
                break;
            }
            if(alrm1[0] == '\0'){
            sprintf ( alrm1, "Date and Time = %s", ctime(&epochSeconds));
            } else if (alrm2[0] == '\0'){
                      sprintf ( alrm2, "Date and Time = %s", ctime(&epochSeconds));
            }else if (alrm3[0] == '\0'){
                      sprintf ( alrm3, "Date and Time = %s", ctime(&epochSeconds));
            }else if (alrm4[0] == '\0'){
                      sprintf ( alrm4, "Date and Time = %s", ctime(&epochSeconds));
            }else if (alrm5[0] == '\0'){
                      i = i + 1;
                      sprintf ( alrm5, "Date and Time = %s", ctime(&epochSeconds));       
            }
}//=====[Implementations of public functions]===================================

void Alarm() 
{
    gasread = mq2.read();
    lm35Reading = lm35.read();
    tempread = analogReadingScaledWithTheLM35Formula(lm35Reading);
   if(gasread >=0.2){
    pcSerialComStringWrite( "Gas Alarm\r\n");
   } else if(tempread >= 30.0) {
    pcSerialComStringWrite( "Temperature Alarm\r\n");
   }
    while(gasread >= 0.2 || tempread >= 30.0){
               gasread = mq2.read();
                lm35Reading = lm35.read();
    tempread = analogReadingScaledWithTheLM35Formula(lm35Reading);
                if (mq2 >= 0.2 || tempread >= 30.0) {
                       buzzer.period(1.0/200.0);
                       buzzer=5.0;
                      delay(1); 
                      buzzer.period(1.0/200.0);
                         buzzer=5.0;
                     delay(1);
                          buzzer=0.0;
                           led = HIGH;
                           code = 0;
                           timer();
                           pcSerialComStringWrite( "\nEnter code to deactivate alarm\r\n");
                           showKeypadInUart = true;
            while(code == 0){
                keypadToUart();
                if(pas == 4){
                        led = OFF;
                        code = 1;
                        pas = 0;
                        cony = 0;
                        pcSerialComStringWrite( "\nAlarm deactivated\r\n");
                      break;        
                    }
                if (cony == 4){
                    pcSerialComStringWrite( "\nWrong code\r\n");
                    cony = 0;
                }
                delay(TIME_INCREMENT_MS);
                           }
                } else {break;
                }
                delay(10);
  }
                    pcSerialComStringWrite( "Gas is not being detected\r\n");
                    pcSerialComStringWrite( "Temp is not high\r\n");
 }
 
float analogReadingScaledWithTheLM35Formula( float analogReading )
{
    return analogReading * 330.0;
}

float celsiusToFahrenheit( float tempInCelsiusDegrees )
{
    return 9.0/5.0 * tempInCelsiusDegrees + 32.0;
}

float potentiometerScaledToCelsius( float analogValue )
{
    return 148.0 * analogValue + 2.0;
}

float potentiometerScaledToFahrenheit( float analogValue )
{
    return celsiusToFahrenheit( potentiometerScaledToCelsius(analogValue) );
}
bool areEqual()
{
    int i;

    for (i = 0; i < NUMBER_OF_KEYS; i++) {
        if (codeSequence[i] != keyPressed[i]) {
            return false;
        }
    }

    return true;
}
void inputsInit()
{
    matrixKeypadInit();
}
void pcSerialComStringWrite( const char* str )
{
    uartUsb.write( str, strlen(str) );
}

char pcSerialComCharRead()
{
    char receivedChar = '\0';
    if( uartUsb.readable() ) {
        uartUsb.read( &receivedChar, 1 );
    }
    return receivedChar;
}
void matrixKeypadInit()
{
    matrixKeypadState = MATRIX_KEYPAD_SCANNING;
    int pinIndex = 0;
    for( pinIndex=0; pinIndex<KEYPAD_NUMBER_OF_COLS; pinIndex++ ) {
        (keypadColPins[pinIndex]).mode(PullUp);
    }
}

char matrixKeypadScan()
{
    int r = 0;
    int c = 0;
    int i = 0;

    for( r=0; r<KEYPAD_NUMBER_OF_ROWS; r++ ) {

        for( i=0; i<KEYPAD_NUMBER_OF_ROWS; i++ ) {
            keypadRowPins[i] = ON;
        }

        keypadRowPins[r] = OFF;

        for( c=0; c<KEYPAD_NUMBER_OF_COLS; c++ ) {
            if( keypadColPins[c] == OFF ) {
                return matrixKeypadIndexToCharArray[r*KEYPAD_NUMBER_OF_ROWS + c];
            }
        }
    }
    return '\0';
}

char matrixKeypadUpdate()
{
    char keyDetected = '\0';
    char keyReleased = '\0';

    switch( matrixKeypadState ) {

    case MATRIX_KEYPAD_SCANNING:
        keyDetected = matrixKeypadScan();
        if( keyDetected != '\0' ) {
            matrixKeypadLastKeyPressed = keyDetected;
            accumulatedDebounceMatrixKeypadTime = 0;
            matrixKeypadState = MATRIX_KEYPAD_DEBOUNCE;
        }
        break;

    case MATRIX_KEYPAD_DEBOUNCE:
        if( accumulatedDebounceMatrixKeypadTime >=
            DEBOUNCE_KEY_TIME_MS ) {
            keyDetected = matrixKeypadScan();
            if( keyDetected == matrixKeypadLastKeyPressed ) {
                matrixKeypadState = MATRIX_KEYPAD_KEY_HOLD_PRESSED;
            } else {
                matrixKeypadState = MATRIX_KEYPAD_SCANNING;
            }
        }
        accumulatedDebounceMatrixKeypadTime =
            accumulatedDebounceMatrixKeypadTime + TIME_INCREMENT_MS;
        break;

    case MATRIX_KEYPAD_KEY_HOLD_PRESSED:
        keyDetected = matrixKeypadScan();
        if( keyDetected != matrixKeypadLastKeyPressed ) {
            if( keyDetected == '\0' ) {
                keyReleased = matrixKeypadLastKeyPressed;
            }
            matrixKeypadState = MATRIX_KEYPAD_SCANNING;
        }
        break;

    default:
        matrixKeypadInit();
        break;
    }
    return keyReleased;
}


void pcSerialComStringRead( char* str, int strLength )
{
    int strIndex;
    for ( strIndex = 0; strIndex < strLength; strIndex++) {
        uartUsb.read( &str[strIndex] , 1 );
        uartUsb.write( &str[strIndex] ,1 );
    }
    str[strLength]='\0';
}

void pcSerialComCharWrite( char chr )
{
    char str[2] = "";
    sprintf (str, "%c", chr);
    uartUsb.write( str, strlen(str) );
}

void keypadToUart()
{
    int keyPressed;
    int i;
yo = 0;
    if ( showKeypadInUart ) {
        keyPressed = matrixKeypadUpdate();
        if ( keyPressed != 0 ) {
            cony++;
            if (keyPressed == codeSequence[cony - 1]){
               pas++;
            } else{
                pas = 0;
            }               
            if( keyPressed == '*'){
                timread();
            }
             pcSerialComCharWrite(keyPressed);

        }
    }
}