#include <Arduino.h>

#include "sensesp_app.h"
#include "signalk/signalk_output.h"
#include "transforms/linear.h"
#include "transforms/moving_average.h"
#include <sensesp_app_builder.h>
#include "ultrasonic_level.h"

#define TX_PIN D3
#define RX_PIN D2

//#define SERIAL_DEBUG_DISABLED


// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([]() {

// Some initialization boilerplate when in debug mode...
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif



  // Create the global SensESPApp() object.
  // sensesp_app = new SensESPApp();

  SensESPAppBuilder builder;


  sensesp_app = builder.set_standard_sensors(ALL)
            ->set_hostname("fuelsensor")
            // ->set_sk_server("10.10.10.1", 3000) // Don't specify server address or port
            // ->set_wifi("SSID", "Password")
            ->get_app(); 

  // The "Signal K path" identifies this sensor to the Signal K server. Leaving
  // this blank would indicate this particular sensor (or transform) does not
  // broadcast Signal K data.
  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html
  //const char *sk_path = "tanks.freshWater.port.currentLevel";
  //const char *sk_path = "tanks.freshWater.starbord.currentLevel";
  const char *sk_path = "tanks.fuel.0.currentLevel";


  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the MCU board. It should
  // ALWAYS start with a forward slash if specified. If left blank,
  // that indicates this sensor or transform does not have any
  // configuration to save, or that you're not interested in doing
  // run-time configuration.

  const char *ultrasonic_in_config_path =
      "/dieselTank/ultrasonic_in";
  const char *linear_config_path = "/dieselTank/linear";
  const char *ultrasonic_ave_samples = "/dieselTank/samples";

  uint read_delay = 1000;

  auto *ultrasonic_sensor = new UltrasonicLevel(
      TX_PIN, RX_PIN, read_delay, ultrasonic_in_config_path);



// see: https://github.com/SignalK/SensESP/wiki/Using-AnalogInput-to-Calculate-a-Tank-Level

  const float empty_value = 0; // in mm 
  const float full_value = 400; // in mm  

  float scale = 100* 1 / (full_value - empty_value); // 0.25;
  // 0mm = 0%
  // 400mm = 100%

  // Wire up the output of the analog input to the Linear transform,
  // and then output the results to the Signal K server.
  ultrasonic_sensor
      ->connect_to(new Linear(scale, 0.0, linear_config_path))
      // ->connect_to(new MovingAverage(10, scale, ultrasonic_ave_samples))
      ->connect_to(new SKOutputNumber(sk_path));

  // also set tank capacity
  // int tank_capacity = 27;
  // auto *constant_sensor = new IntegerProducer();
  // constant_sensor->connect_to(
      // new SKOutputNumber("tanks.fuel.0.capacity")
  // );

  // Start the SensESP application running
  sensesp_app->enable();
});