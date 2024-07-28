// VARIABLE DEFINITIONS
#include <WiFi.h>

#define ACCESSPOINT_MODE // switch on to create Network with the ESP, switch
// off to log into an existing network

// Variables for automatic mode
bool automatic_mode = false; // false = manual mode, true = automatic mode

// Speed Variables
unsigned int single_speed = 10; // Speed of the single turn in rmp default is 10
unsigned int infinite_speed = 10; // Speed of the infinite turn in rmp

unsigned int last_single_speed = 0;
unsigned int last_infinite_speed = 0;

bool front_pos_defined = false;
bool rear_pos_defined = false;

int front_pos = 0; // defining the position where the tool in half rev mode hits
int rear_pos = 0;  // defines the idle position

double speed_multi =
    5; // the speed slider in the html always delivers a value between 10 and
       // 100 rmp --> to increase this range increase the mulitplicator

// variables for timing serial prints
long int last_stepper_position_printT = 0;

bool resetRamp = false;
int hallSensPinFront = 13;
int hallSensPinRamp = 14;
int hallSensPinRear = 15;

int hallSensOffset = 30;

// Variables for sin generator:
double base_sin = 0; // Base Force in dN
double f_sin = 0;    // Frequency in pbm
double amp_sin = 0;  // Amplitude Force in dN
double y_sin = 0;    // Sin output variable

// Variables for timing the code in the loop:
unsigned long previousMillis = 0; // will store last time
unsigned long currentMillis = 0;
const long interval1 = 100; // interval at which to execute (milliseconds)
long int timeout = 2000;    // Timeout for the Turn in ms

// Variables for buttons:
volatile bool startTurn = 0; // Start permanent Turn
bool calibrationDone = false;

// Variables for network credentials:
#ifndef ACCESSPOINT_MODE
const char *ssid = "FalschesHaus";
const char *password = "ehdOb!sSi";
#endif

// Variables for network credentials:
const int HTTP_PORT = 80;
const int HTTPS_PORT = 443;
const int WS_PORT = 81;
const int WSS_PORT = 82;

const char *ap_ssid = "Dorsi-ESP-AP";
const char *ap_password = "Dorsi2024";

const char *ssid = "DorsiWifi";
const char *password = "Dorsi2024";

// Variables to save values from input on main page:
const char *PARAM_INPUT_1 = "input1";
const char *PARAM_INPUT_2 = "input2";
const char *PARAM_INPUT_3 = "input3";

// Arrays for video mode
// first row contains the time in s, second row the speed in % of the max speed
double video_1[20][2] = {{0, 30},  {3, 50},  {5, 40},   {7, 60},  {9, 70},
                         {11, 80}, {13, 90}, {15, 100}, {17, 90}, {19, 80},
                         {21, 70}, {23, 60}, {25, 50},  {27, 40}, {29, 30},
                         {31, 20}, {33, 10}, {35, 0},   {37, 0},  {39, 0}};
