#include <memory>

#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/linear.h"
#include "sensesp/ui/config_item.h"
#include "sensesp_app_builder.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/transforms/frequency.h"
#include "sensesp_onewire/onewire_temperature.h"

using namespace reactesp;
using namespace sensesp;
using namespace sensesp::onewire;

void setup() {
  SetupLogging();

  // Create the global SensESPApp() object.
  SensESPAppBuilder builder;
  sensesp_app = builder.get_app();

  /*
     Find all the sensors and their unique addresses. Then, each new instance
     of OneWireTemperature will use one of those addresses. You can't specify
     which address will initially be assigned to a particular sensor, so if you
     have more than one sensor, you may have to swap the addresses around on
     the configuration page for the device. (You get to the configuration page
     by entering the IP address of the device into a browser.)
  */

  /*
     Tell SensESP where the sensor is connected to the board
     ESP32 pins are specified as just the X in GPIOX
  */
  uint8_t pin =25;

  DallasTemperatureSensors* dts = new DallasTemperatureSensors(pin);

  // Define how often SensESP should read the sensor(s) in milliseconds
  uint read_delay = 500;

  // Below are temperatures sampled and sent to Signal K server
  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html

  // Measure coolant temperature
  auto coolant_temp =
      new OneWireTemperature(dts, read_delay, "/coolantTemperature/oneWire");

  ConfigItem(coolant_temp)
      ->set_title("Coolant Temperature")
      ->set_description("Temperature of the engine coolant")
      ->set_sort_order(110);

  auto coolant_temp_calibration =
      new Linear(1.0, 0.0, "/coolantTemperature/linear");

  ConfigItem(coolant_temp_calibration)
      ->set_title("Coolant Temperature Calibration")
      ->set_description("Calibration for the coolant temperature sensor")
      ->set_sort_order(120);

  auto coolant_temp_sk_output = new SKOutputFloat(
      "propulsion.engine.coolantTemperature", "/coolantTemperature/skPath");

  ConfigItem(coolant_temp_sk_output)
      ->set_title("Coolant Temperature Signal K Path")
      ->set_description("Signal K path for the coolant temperature")
      ->set_sort_order(130);

  coolant_temp->connect_to(coolant_temp_calibration)
      ->connect_to(coolant_temp_sk_output);

  // Measure SeaWaterIn temperature
  auto* seaWaterInTemperature =
      new OneWireTemperature(dts, read_delay, "/seaWaterInTemperature/oneWire");
  auto* seaWaterInTemperature_calibration =
      new Linear(1.0, 0.0, "/seaWaterInTemperature/linear");
  auto* seaWaterInTemperature_k_output = new SKOutputFloat(
      "propulsion.engine.seaWaterInTemperature", "/seaWaterInTemperature/skPath");

  seaWaterInTemperature->connect_to(seaWaterInTemperature_calibration)
      ->connect_to(seaWaterInTemperature_k_output);

  // Measure SeaWaterOut temperature
  auto* seaWaterOutTemperature =
      new OneWireTemperature(dts, read_delay, "/seaWaterOutTemperature/oneWire");
  auto* seaWaterOutTemperature_calibration =
      new Linear(1.0, 0.0, "/seaWaterOutTemperature/linear");
  auto* seaWaterOutTemperature_k_output = new SKOutputFloat(
      "propulsion.engine.seaWaterOutTemperature", "/seaWaterOutTemperature/skPath");

  seaWaterOutTemperature->connect_to(seaWaterOutTemperature_calibration)
      ->connect_to(seaWaterOutTemperature_k_output);
      
// ********** RPM Application **************//
  const char* config_path_calibrate = "/Engine RPM/calibrate";
  const char* config_path_skpath = "/Engine RPM/sk_path";
  const float multiplier = 1.0;

  auto* engineRPM = new DigitalInputCounter(16, INPUT_PULLUP, RISING, 500, config_path_calibrate);

    ConfigItem(engineRPM)
        ->set_title("Engine RPM")
        ->set_description("Revolutions of the Engine")
        ->set_sort_order(200);

  engineRPM->connect_to(new Frequency(multiplier, config_path_calibrate))
  // connect the output of sensor to the input of Frequency()
          ->connect_to(new SKOutputFloat("propulsion.engine.revolutions", config_path_skpath));  
          // connect the output of Frequency() to a Signal K Output as a number

  auto engine_rpm_sk_output = new SKOutputFloat("propulsion.engine.revolutions", config_path_skpath);          

  ConfigItem(engine_rpm_sk_output)
      ->set_title("Engine RPM Signal K Path")
      ->set_description("Signal K path for the RPM of engine")
      ->set_sort_order(210);          
}

// main program loop
void loop() {
  static auto event_loop = sensesp_app->get_event_loop();
  event_loop->tick();
}
