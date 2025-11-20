#include <memory>
#include "onewire_helper.h"

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

// Board & project constants
// - Use GPIO numbers (ESP32): refer to pins as `ONEWIRE_PIN`, `RPM_PIN`.
// - Use named constants to avoid magic numbers scattered through the code.
static constexpr uint8_t ONEWIRE_PIN = 25;
static constexpr uint8_t RPM_PIN = 16;
static constexpr unsigned int RPM_READ_DELAY_MS = 500;
static constexpr unsigned int TEMPERATURE_READ_DELAY_MS = 2000;

// add_onewire_temp is implemented in src/onewire_helper.cpp

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
  DallasTemperatureSensors* dts = new DallasTemperatureSensors(ONEWIRE_PIN);

  // Define how often SensESP should read the sensor(s) in milliseconds
  uint temperature_read_delay = TEMPERATURE_READ_DELAY_MS;

  // Below are temperatures sampled and sent to Signal K server
  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html

  // Measure coolant temperature
    // Measure coolant temperature
    add_onewire_temp(dts, temperature_read_delay, "coolantTemperature",
             "propulsion.main.coolantTemperature",
             "Coolant Temperature", 110, 120, 130);

    // Measure SeaWaterIn temperature
    add_onewire_temp(dts, temperature_read_delay, "seaWaterInTemperature",
             "propulsion.main.seaWaterInTemperature",
             "Sea Water In Temperature", 140, 150, 160);

    // Measure SeaWaterOut temperature
    add_onewire_temp(dts, temperature_read_delay, "seaWaterOutTemperature",
             "propulsion.main.seaWaterOutTemperature",
             "Sea Water Out Temperature", 170, 180, 190);
      
// ********** RPM Application **************//
  const char* config_path_calibrate = "/engineRPM/calibrate";
  const char* config_path_skpath = "/engineRPM/sk_path";
  const float multiplier = 1.0;

  auto* engineRPM = new DigitalInputCounter(RPM_PIN, INPUT_PULLUP, RISING, RPM_READ_DELAY_MS, config_path_calibrate);

  ConfigItem(engineRPM)
      ->set_title("Engine RPM")
      ->set_description("Revolutions of the Engine")
      ->set_sort_order(200);

  // Create Frequency and SK output once and reuse them (avoid duplicate SKOutputFloat)
  auto* rpm_frequency = new Frequency(multiplier, config_path_calibrate);
  auto* engine_rpm_sk_output = new SKOutputFloat("propulsion.main.revolutions", config_path_skpath);

  // Connect sensor -> frequency -> SK output
  engineRPM->connect_to(rpm_frequency)->connect_to(engine_rpm_sk_output);

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
