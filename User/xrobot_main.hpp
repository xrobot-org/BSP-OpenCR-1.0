#include "app_framework.hpp"
#include "libxr.hpp"

// Module headers
#include "BlinkLED.hpp"
#include "BuzzerAlarm.hpp"

static void XRobotMain(LibXR::HardwareContainer &hw) {
  using namespace LibXR;
  ApplicationManager appmgr;

  // Auto-generated module instantiations
  static BlinkLED blink_led(hw, appmgr, 250);
  static BuzzerAlarm buzzer_alarm(hw, appmgr, 1500, 80, 80);

  while (true) {
    appmgr.MonitorAll();
    Thread::Sleep(1000);
  }
}