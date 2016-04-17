
// Firmware for MultispeQ 1.0 hardware.   Part of the PhotosynQ project.

// setup() and support routines
// main loop is in loop.cpp

// update DAC and get lights working in [{}]
// once lights work, comparison test old and new adc routines, with timing
//

/*

  + test do we need to calibrate offsets (like we did with the betas?)

  Discuss meringing other eeprom floats into userdef[] with #define
  Create API for read_userdef, save_userdev, and reset_eeprom, delete all other eeprom commands, clean up all 1000… calls.
  Using bluetooth ID as ID -
  Firmware needs api call to write code to eeprom (unique ID).
  Get rid of user_enter… and replace with new user enter, then delete main function
  Pull out the alert/confirm/prompt into a subroutine.
  If averages == 1, then print data in real time
  Reimplement Actinic background light… make sure to update (currently set to 13)
  reimplement print_offset and get_offset

  Make the “environmental” a separate subroutine and pass the before or after 0,1 to it.

  I would like to set the micro-einstein level for the lights in my measurements rather than a raw (unitless) 12 bit value.

  Hardware
  Noise in detector - big caps help, but not completely.  ANy better ideas?
  Low DAC values (eg, LED5 read from main) cause much higher stdev
  Detector has a DC offset (which causes ADC to read zero)
  Pulse width vs detector output is non-linear - caused by the DC filter?

  x Switch to combined ISR for LED pulses (no glitches)

  Convert all possible into an array to make designing protocols more user friendly
  x turn pulse_distance and pulse_size → into an array

  x Greg - find suitable small magnet to embed, talk with Geoff

  Android to check for empty ID (if all 0s, or all 1s, then set api call to make unique ID == BLE mac address.
  Check protocol routines (produce error codes if fail):
   Battery check: Calculate battery output based on flashing the 4 IR LEDs at 250 mA each for 10uS.  This should run just before any new protocol - if it’s too low, report to the user
   What's a reasonable value for minimum voltage while pulsing?
   (greg) Overheat LED check: adds up time + intensity + pulsesize and length of pulses, and calculates any overages.  Report overages - do not proceed if over.  Also needs a shutoff which is available through a 1000 call.
   Syntax check: make sure that the structure of the JSON is correct, report corrupted
   x Jon - CRC check: so we expect CRC on end of protocol JSON, and check to make sure it’s valid.  Report corrupted
   LED intensity range check?  Ie, certain LEDs can only go up to a certain intensity

  Define and then code 1000+ calls for all of the sensors (for chrome app to call)
  Implement application of magnetometer/compass calibrations
  Sebastian - can we change the sensor read commands 1000 over to normal protocols - then you output as per normal?)
  Check with sebastian about adding comments to protocols (even the inventor of json thinks there is a place for them)
  Clean up the protocols - light intensity (make into a single 1000+ call, see old code to bring it in)
  Check to make sure “averages” works in the protocols
  Clean up the pulsesize = 0 and all that stuff… meas_intensity…
  Attach par sensor to USB-C breakout, make calibration routine.
  Look up and see why iOS doesn’t connect to BLE.
  Test the power down feature from the bluetooth.  Determine how long it takes when powered down to come back to life (from bluetooth).  Include auto power off in the main while loop - auto-off after 10 seconds.
  Troubleshoot issues with bluetooth between protocols.


  Start documenting the commands + parameters to pass…
  And the eeprom commands

   Next to do:
   Add calibration commands back in
   consolidate commands to end up with:
   get rid of droop in dac values at <10.
   fix detector mv offset (prevents reading low light levels)

   First thing - go through eeprom.h and set all of the variables in top and bottom of file...
   expose only a portion to users via 1000+ commands

   I suggest that all userdefs be a single float - JZ
   read_userdef - option to get a single userdef or all userdefs.  include the long arrays (like calibration_slope) as well as the offset and other variables which are not currently saved as userdefs.  All saved values are saveable in get_userdef.  This should have a print option when someone wants to also print it, and a get all option to print all userdefs (as JSON follow existing structure).
   replaces get_calibration, get_calibration_userdef, call_print_calibration, print_sensor_calibration, print_offset, set_device_info

   save_userdef - saves a value to EEPROM.  The size of the saved array here is defined by the userdef number.  So maybe number 1- 20 are user definable (some doubles, some triples, some long arrays), then 21 - 60 are 'reserved' for us developers (ie we don't expose them). is device info, 2 - 45 are 2 arrays, 45 - 50 are 3 array, and 50 - 60 are 25 array (or something).  Should include a standard print option to show success (as JSON).
   replaces add_calibration, add_userdef, save_calibration_slope, save_calibration_yint, calibrate_offset

   reset_eeprom - option to reset all, or just reset the exposed user values (1 - 20)
   replaces reset_all

   Note: code for each function should have some intial comments describing what it does

   Note: eeprom writing needs refactoring - eliminate all EEPROMxAnything()

   Here's the remaining commands in this file which should be moved.  I've organized them by function.  Actually this will be nice as some of them need to be renamed and cleaned up anyway.

  // power off
  pwr_off

  // Calibration commands
  add_calibration             // this adds a single element to the calibrations stored as an array (containing all of the LED pins)
  get_calibration             // this gets and prints to serial a saved calibration value
  print_cal                   // print a single calibration value
  add_userdef                 // this adds a set of userdefs (may be 2, 3, or 4 objects in length)
  get_calibration_userdef     // this gets and prints to serial a saved userdef value
  print_get_userdef           // this prints all of the saved userdef values
  call_print_calibration      // call or call + print all calibration values
  save_calibration_slope      // get rid of these - integrate into rest of normal save system
  save_calibration_yint       // get rid of these - integrate into rest of normal save system
  reset_all                   // reset all calibration values, or only all except the device info.

  // Calculation of PAR from PAR sensor, and from MultispeQ LEDs, and calibration of light sensor
  Light_Intensity
  calculate_intensity
  calculate_intensity_background
  lux_to_uE
  uE_to_intensity
  calibrate_light_sensor

  // Calculate frequency of the timers
  reset_freq

  // CoralspeQ related functions
  readSpectrometer
  print_data

  // Commands related to the detector offset
  calibrate_offset
  calculate_offset
  print_sensor_calibration
  print_offset

  // Device info command
  set_device_info

  // EEPROMxAnything - delete this and use new eeprom.h

  NEXT STEPS:

  - CHANGE THE WAY WE UPTAKE THE ACTNINIC AND MEASURING LIGHTS - ACT_LIGHTS, MEAS_LIGHTS, DETECTORS 1/2/3/4, SEE HOW IT'S ITERPRETED IN THE CODE.

  a_lights : [[3,4,5],[3],[3]]
  a_intensities : [[254,450,900],[254],[254]]
  meas_lights : [[2,3],[2,3],[3]]
  meas_intensities[[500,100],[500,100],[100]]
  detectors : [[2, 1],[2, 1],[1]]
  pulses : [100,100,200]
  pulse_distance : [10000,10000,1000]
  pulse_size :  [10,100,10]
  message : [["alert":"alert message"],["prompt":"prompt message"],["0","0"]]


  A flat structure is also possible... where each pulse set is a separate object.  In that case I'd have to interpret each pulse set separately using the JSON interpret tool.  This takes time, and it's preferable to completely
  interpret the JSON prior to starting the measurement.  Interpreting it between pulses would likely cause delays and increase the shortest possible measurement pulse distance.
  The other option is to interpret each JSON and rebuild the arrays at the beginning of the measurement - this would be quite tedious and hard to do.

*/

/*
  ////////////////////// HARDWARE NOTES //////////////////////////

  The detector operates with an high pass filter, so only pulsing light (<100us pulses) passes through the filter.  Permanent light sources (like the sun or any other constant light) is completely
  filtered out.  So in order to measure absorbance or fluorescence a pulsing light must be used to be detectedb by the detector.  Below are notes on the speed of the measuring lights
  and actinic lights used in the MultispeQ, and the noise level of the detector:

  Optical:
  RISE TIME, Measuring: 0.4us
  FALL TIME, Measuring: 0.2us
  RISE TIME Actinic (bright): 2us
  FALL TIME Actinic (bright): 2us
  RISE TIME Actinic (dim): 1.5us
  FALL TIME Actinic (dim): 1.5us

  Electrical:
  RISE TIME, Measuring: .4us
  FALL TIME, Measuring: .2us
  RISE TIME Actinic: 1.5us
  FALL TIME Actinic: 2us
  NOISE LEVEL, detector 1: ~300ppm or ~25 detector units from peak to peak
  OVERALL: Excellent results - definitely good enough for the spectroscopic measurements we plant to make (absorbance (ECS), transmittance, fluorescence (PMF), etc.)

  //////////////////////  I2C Addresses //////////////////////
  TCS34725 0x29

*/


// includes
#include "defines.h"
#include <i2c_t3.h>
#include <Time.h>                                                             // enable real time clock library
#include "utility/Adafruit_Sensor.h"
#include "json/JsonParser.h"
#include "utility/mcp4728.h"              // delete this once PAR is fixed
#include "DAC.h"
#include "AD7689.h"               // external ADC
#include "utility/Adafruit_BME280.h"      // temp/humidity/pressure sensor
#define EXTERN
#include "eeprom.h"
//#include <ADC.h>                  // internal ADC
#include "serial.h"
#include "utility/crc32.h"
#include <SPI.h>    // include the new SPI library:


// function definitions used in this file
int MAG3110_init(void);           // initialize compass
int MMA8653FC_init(void);         // initialize accelerometer
void MLX90615_init(void);         // initialize contactless temperature sensor
void PAR_init(void);               // initialize PAR and RGB sensor


//////////////////////PIN DEFINITIONS FOR CORALSPEQ////////////////////////
#define SPEC_GAIN      28
//#define SPEC_EOS       NA
#define SPEC_ST        26
#define SPEC_CLK       25
#define SPEC_VIDEO     A10
//#define LED530         15
//#define LED2200k       16
//#define LED470         20
//#define LED2200K       2
#define SPEC_CHANNELS    256
uint16_t spec_data[SPEC_CHANNELS];
unsigned long spec_data_average[SPEC_CHANNELS];            // saves the averages of each spec measurement
int idx = 0;

#if 0
#define NUM_PINS 26

// MCU pins that are controllable by the user
// More explanation???

float all_pins [NUM_PINS] = {
  15, 16, 11, 12, 2, 20, 14, 10, 34, 35, 36, 37, 38, 3, 4, 9, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33
};
float calibration_slope [NUM_PINS] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
float calibration_yint [NUM_PINS] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
float calibration_slope_factory [NUM_PINS] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
float calibration_yint_factory [NUM_PINS] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
float calibration_baseline_slope [NUM_PINS] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
float calibration_baseline_yint [NUM_PINS] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
float calibration_blank1 [NUM_PINS] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
float calibration_blank2 [NUM_PINS] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
float calibration_other1 [NUM_PINS] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
float calibration_other2 [NUM_PINS] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
#endif


// pressure/temp/humidity sensors
Adafruit_BME280 bme1;        // I2C sensor
Adafruit_BME280 bme2;       // I2C sensor

// This routine is called first

void setup() {

  delay(500);

  // set up serial ports (Serial and Serial1)
  Serial_Set(4);
  Serial_Begin(57600);

#ifdef DEBUGSIMPLE
  Serial_Print_Line("serial works");
#endif

  // Set up I2C bus
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_INT, I2C_RATE_400);  // using alternative wire library
  //  Serial_Print_Line("I2C works");

  // initialize SPI bus
  SPI.begin ();
  //  Serial_Print_Line("SPI works");

  // initialize DACs
  DAC_init();

  // set up MCU pins

  // set up LED on/off pins
  for (unsigned i = 1; i < NUM_LEDS + 1; ++i)
    pinMode(LED_to_pin[i], OUTPUT);

  pinMode(HALL_OUT, INPUT);                                                       // set hall effect sensor to input so it can be read
  pinMode(DEBUG_DC, INPUT);                                                       // leave floating
  pinMode(DEBUG_DD, INPUT);                                                       // leave floating
  //  pinMode(BLERESET, OUTPUT);                                                  // leave floating - deprecated
  //  digitalWriteFast(BLERESET, HIGH);
  //  delay(100);
  //  digitalWriteFast(BLERESET, LOW);                                                // set BLE reset low for the bluetooth to be active
  pinMode(ISET, OUTPUT);                                                          // set USB charge level - deprecated
  digitalWriteFast(ISET, HIGH);
  //  pinMode(ACTINICLIGHT_INTENSITY_SWITCH, OUTPUT);
  //  digitalWriteFast(ACTINICLIGHT_INTENSITY_SWITCH, HIGH);                     // preset the switch to the actinic (high) preset position, DAC channel 0

  // pins used to turn on/off detector integration/discharge
  pinMode(HOLDM, OUTPUT);
  digitalWriteFast(HOLDM, HIGH);                  // discharge cap
  pinMode(HOLDADD, OUTPUT);
  digitalWriteFast(HOLDADD, HIGH);                // discharge cap

  pinMode(BLANK, OUTPUT);                                                            // used as a blank pin to pull high and low if the meas lights is otherwise blank (set to 0)

#if 0
  // ???
  float default_resolution = 488.28;
  int timer0 [8] = {
    5, 6, 9, 10, 20, 21, 22, 23
  };
  int timer1 [2] = {
    3, 4
  };
  int timer2 [2] = {
    25, 32
  };
#endif

  // ??
  //analogWriteFrequency(3, 187500);                                              // Pins 3 and 5 are each on timer 0 and 1, respectively.  This will automatically convert all other pwm pins to the same frequency.
  //analogWriteFrequency(5, 187500);

#if CORALSPEQ == 1
  // Set pinmodes for the coralspeq
  //pinMode(SPEC_EOS, INPUT);
  pinMode(SPEC_GAIN, OUTPUT);
  pinMode(SPEC_ST, OUTPUT);
  pinMode(SPEC_CLK, OUTPUT);

  digitalWrite(SPEC_GAIN, LOW);
  digitalWrite(SPEC_ST, HIGH);
  digitalWrite(SPEC_CLK, HIGH);
  digitalWrite(SPEC_GAIN, HIGH); //High Gain
  //digitalWrite(SPEC_GAIN, LOW); //LOW Gain
#endif

  MAG3110_init();           // initialize compass
  MMA8653FC_init();         // initialize accelerometer

  // ADC config
  analogReference(EXTERNAL);
  analogReadResolution(16);
  analogReadAveraging(4); 
  {
    uint32_t x = analogRead(39) >> 4;  // forumla needs 12 bits, not 16
    uint32_t mv = (178 * x * x + 2688757565 - 1184375 * x) / 372346; // milli-volts input to MCU, clips at ~3500
    assert(mv > 3400);      // voltage is too low for proper operation
  }
  analogReference(INTERNAL);

#ifdef BME280
  // pressure/humidity/temp sensors
  // note: will need 0x76 or 0x77 to support two chips
  assert (bme.begin(0x76) && bme2.begin(0x77));

#ifdef DEBUGSIMPLE
  Serial_Print("BME280 Temperature = ");
  Serial_Print(bme.readTemperature());
  Serial_Print_Line(" *C");
#endif
#endif

  PAR_init();               // color sensor

#undef DEBUGSIMPLE

  // test expressions - works! pass it a string and it is evaluated
  //double expr(const char s[]);
  //Serial_Printf("expr = %f\n",expr("userdef1/2"));  // userdef1 is from eeprom

  assert(sizeof(eeprom_class) < 2048);                    // check that we haven't exceeded eeprom space

  Serial_Print(DEVICE_NAME);
  Serial_Print_Line(" Ready");


}  // setup()


// ??
void reset_freq() {
  analogWriteFrequency(5, 187500);                                               // reset timer 0
  analogWriteFrequency(3, 187500);                                               // reset timer 1
  analogWriteFrequency(25, 488.28);                                              // reset timer 2
  /*
    Teensy 3.0              Ideal Freq:
    16      0 - 65535       732 Hz          366 Hz
    15      0 - 32767       1464 Hz         732 Hz
    14      0 - 16383       2929 Hz         1464 Hz
    13      0 - 8191        5859 Hz         2929 Hz
    12      0 - 4095        11718 Hz        5859 Hz
    11      0 - 2047        23437 Hz        11718 Hz
    10      0 - 1023        46875 Hz        23437 Hz
    9       0 - 511         93750 Hz        46875 Hz
    8       0 - 255         187500 Hz       93750 Hz
    7       0 - 127         375000 Hz       187500 Hz
    6       0 - 63          750000 Hz       375000 Hz
    5       0 - 31          1500000 Hz      750000 Hz
    4       0 - 15          3000000 Hz      1500000 Hz
    3       0 - 7           6000000 Hz      3000000 Hz
    2       0 - 3           12000000 Hz     6000000 Hz

  */
}


// read/write device_id and manufacture_date to eeprom

void set_device_info(const int _set) {
  Serial_Printf("{\"device_name\":\"%s\",\"device_version\":\"%s\",\"device_id\":\"%ld\",\"device_firmware\":\"%s\",\"device_manufacture\":\"%d\"}", DEVICE_NAME, DEVICE_VERSION, eeprom->device_id, FIRMWARE_VERSION, eeprom->manufacture_date);
  Serial_Print_CRC();

  if (_set == 1) {
    long val;

    // please enter new device ID (lower 4 bytes of BLE MAC address) followed by '+'
    val =  Serial_Input_Long("+", 0);              // save to eeprom
    if (eeprom->device_id != val) {
      eeprom->device_id = val;              // save to eeprom
      delay(1);
    }

    // please enter new date of manufacture (yyyymm) followed by '+'
    val = Serial_Input_Long("+", 0);
    if (eeprom->manufacture_date != val) {
      eeprom->manufacture_date = val;
      delay(1);
    }

    // print again for verification

  Serial_Printf("{\"device_name\":\"%s\",\"device_version\":\"%s\",\"device_id\":\"%ld\",\"device_firmware\":\"%s\",\"device_manufacture\":\"%d\"}", DEVICE_NAME, DEVICE_VERSION, eeprom->device_id, FIRMWARE_VERSION, eeprom->manufacture_date);
    Serial_Print_CRC();
  } // if

  return;

} // set_device_info()

#if CORAL_SPEQ == 1

void readSpectrometer(int intTime, int delay_time, int read_time, int accumulateMode)
{
  /*
    //int delay_time = 35;     // delay per half clock (in microseconds).  This ultimately conrols the integration time.
    int delay_time = 1;     // delay per half clock (in microseconds).  This ultimately conrols the integration time.
    int idx = 0;
    int read_time = 35;      // Amount of time that the analogRead() procedure takes (in microseconds)
    int intTime = 100;
    int accumulateMode = false;
  */

  // Step 1: start leading clock pulses
  for (int i = 0; i < SPEC_CHANNELS; i++) {
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
  }

  // Step 2: Send start pulse to signal start of integration/light collection
  digitalWrite(SPEC_CLK, LOW);
  delayMicroseconds(delay_time);
  digitalWrite(SPEC_CLK, HIGH);
  digitalWrite(SPEC_ST, LOW);
  delayMicroseconds(delay_time);
  digitalWrite(SPEC_CLK, LOW);
  delayMicroseconds(delay_time);
  digitalWrite(SPEC_CLK, HIGH);
  digitalWrite(SPEC_ST, HIGH);
  delayMicroseconds(delay_time);

  // Step 3: Integration time -- sample for a period of time determined by the intTime parameter
  int blockTime = delay_time * 8;
  int numIntegrationBlocks = (intTime * 1000) / blockTime;
  for (int i = 0; i < numIntegrationBlocks; i++) {
    // Four clocks per pixel
    // First block of 2 clocks -- measurement
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);

    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
  }


  // Step 4: Send start pulse to signal end of integration/light collection
  digitalWrite(SPEC_CLK, LOW);
  delayMicroseconds(delay_time);
  digitalWrite(SPEC_CLK, HIGH);
  digitalWrite(SPEC_ST, LOW);
  delayMicroseconds(delay_time);
  digitalWrite(SPEC_CLK, LOW);
  delayMicroseconds(delay_time);
  digitalWrite(SPEC_CLK, HIGH);
  digitalWrite(SPEC_ST, HIGH);
  delayMicroseconds(delay_time);

  // Step 5: Read Data 2 (this is the actual read, since the spectrometer has now sampled data)
  idx = 0;
  for (int i = 0; i < SPEC_CHANNELS; i++) {
    // Four clocks per pixel
    // First block of 2 clocks -- measurement
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, LOW);

    // Analog value is valid on low transition
    if (accumulateMode == false) {
      spec_data[idx] = analogRead(SPEC_VIDEO);
      spec_data_average[idx] += spec_data[idx];
    } else {
      spec_data[idx] += analogRead(SPEC_VIDEO);
    }
    idx += 1;
    if (delay_time > read_time) delayMicroseconds(delay_time - read_time);   // Read takes about 135uSec

    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);

    // Second block of 2 clocks -- idle
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
  }

  // Step 6: trailing clock pulses
  for (int i = 0; i < SPEC_CHANNELS; i++) {
    digitalWrite(SPEC_CLK, LOW);
    delayMicroseconds(delay_time);
    digitalWrite(SPEC_CLK, HIGH);
    delayMicroseconds(delay_time);
  }
}

void print_data()
{
  Serial_Print("\"data_raw\":[");
  for (int i = 0; i < SPEC_CHANNELS; i++)
  {
    Serial_Print((int)spec_data[i]);
    if (i != SPEC_CHANNELS - 1) {               // if it's the last one in printed array, don't print comma
      Serial_Print(",");
    }
  }
  Serial_Print("]");
}
#endif

#if 0
// give it a channel, outputs outputs x,y,z 3 axis data comes out (16 bit)
// perhaps add an option to return raw values or values with calibration applied
void compass (uint16_t array[]) {};

// give it a channel, outputs outputs x,y,z  3 axis data comes out (16 bit)
// perhaps add an option to return raw values or values with calibration applied
void accel (uint16_t array[]) {};

// read this channel from ??, replys with one number (16 bit)
// maybe best to take 33 readings and return the median
uint16_t atod (int channel) {
  return 0;
};
#endif

// qsort uint16_t comparison function (tri-state) - needed for median16()

static int uint16_cmp(const void *a, const void *b)
{
  const uint16_t *ia = (const uint16_t *)a; // casting pointer types
  const uint16_t *ib = (const uint16_t *)b;

  if (*ia == *ib)
    return 0;

  if (*ia > *ib)
    return 1;
  else
    return -1;
}

// return the median value of an array of 16 bit unsigned values
// note: this also sorts the array
// percentile is normally .50

uint16_t median16(uint16_t array[], const int n, const float percentile)
{
  qsort(array, (size_t) n, sizeof(uint16_t), uint16_cmp);
  return (array[(int) roundf(n * percentile)]);
}


// return the stdev value of an array of 16 bit unsigned values

float stdev16(uint16_t val[], const int count)
{
  // calc stats
  double mean = 0, delta = 0, m2 = 0, variance = 0, stdev = 0, n = 0;

  for (int i = 0; i < count; i++) {
    ++n;
    delta = val[i] - mean;
    mean += delta / n;
    m2 += (delta * (val[i] - mean));
  } // for
  variance = m2 / (count - 1);  // (n-1):Sample Variance  (n): Population Variance
  stdev =  sqrt(variance);        // Calculate standard deviation
  //Serial_Printf("single pulse stdev = %.2f, mean = %.2f AD counts\n", stdev, mean);
  //Serial_Printf("bits (95%%) = %.2f\n", (15 - log(stdev * 2) / log(2.0))); // 2 std dev from mean = 95%
  return stdev;
} // stdev16()


// check a json protocol for validity (matching [] and {})
// return 1 if OK, otherwise 0
// also check CRC value if present

int check_protocol(char *str)
{
  int bracket = 0, curly = 0;
  char *ptr = str;

  while (*ptr != 0) {
    switch (*ptr) {
      case '[':
        ++bracket;
        break;
      case ']':
        --bracket;
        break;
      case '{':
        ++curly;
        break;
      case '}':
        --curly;
        break;
    } // switch
    ++ptr;
  } // while

  if (bracket != 0 || curly != 0)  // unbalanced - can't be correct
    return 0;

  // check CRC - 8 hex digits immediately after the closing ]
  ptr = strrchr(str, ']'); // find last ]
  if (!ptr)                        // no ] found - how can that be?
    return 0;

  ++ptr;                      // char after last ]

  if (!isxdigit(*(ptr)))      // hex digit follows last ] ?
    return 1;                    // no CRC so report OK

  // CRC is there - check it

  crc32_init();     // find crc of json (from first [ to last ])
  crc32_buf (str, ptr - str);

  // note: must be exactly 8 upper case hex digits
  if (strncmp(int32_to_hex (crc32_value()), ptr, 8) != 0) {
    return 0;                 // bad CRC
  }

  return 1;                   // CRC is OK
} // check_protocol()


const unsigned long SHUTDOWN = 10000;   // power down after X ms of inactivity
static unsigned long last_activity = millis();
// if there hasn't been any activity for x seconds, then power down
// also power down if the battery is too low
void powerdown() {
  // send command to BLE module by setting XX low
  // only the BLE module can power up/down the MCU
#if 0
  if (millis() - last_activity > SHUTDOWN) {
    Serial_Print_Line("powerdown");
    // send request to BLE module to power down this MCU
    for (;;) {}
  } // if
#endif
}
// record that we have seen serial port activity (used with powerdown())
void activity() {
  last_activity = millis();
}

// Battery check: Calculate battery output based on flashing the 4 IR LEDs at 250 mA each for 10uS.
// This should run just before any new protocol - if it’s too low, report to the user

int battery_check()
{
  return 0;
}




