// VARIABLE DEFINITIONS
#include <WiFi.h>

#define ACCESSPOINT_MODE // switch on to create Network with the ESP, switch
// off to log into an existing network

// Variables for automatic mode
bool automatic_mode = false; // false = manual mode, true = automatic mode

// Speed Variables
unsigned int single_speed = 10;         // Speed of the single turn in rpm default is 10
unsigned int max_single_speed = 100;    // Max Speed of the single turn in rpm default is 100

unsigned int infinite_speed = 10;       // Speed of the infinite turn in rpm
unsigned int max_infinite_speed = 100;  // Max Speed of the infinite turn in rpm default is 100

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

const int DNS_PORT = 53;

uint8_t dns_answer[] = {
    0xc0, 0x0c,          // Point to the name in the DNS question
    0,    1,             // 2 bytes - record type, A
    0,    1,             // 2 bytes - address class, INET
    0,    0,    0, 120,  // 4 bytes - TTL
    0,    4,             // 2 bytes - address length
  192,  168,    4,   1   // 4 bytes - IP address
};

// MQTT Broker settings
const char *mqtt_broker = "x6e1f6a7.ala.eu-central-1.emqxsl.com";
const char *motor_state_topic = "esp32/motorState";
const char *motor_speed_topic = "esp32/motorSpeed";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 8883;

// Load DigiCert Global Root CA ca_cert, which is used by EMQX Cloud Serverless Deployment
const char* ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";

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
