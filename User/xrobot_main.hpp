#include "app_framework.hpp"
#include "libxr.hpp"

// Module headers
#include "BlinkLED.hpp"
#include "BuzzerAlarm.hpp"
#include "ICM20948.hpp"
#include "MadgwickAHRS.hpp"

static void XRobotMain(LibXR::HardwareContainer &hw) {
  using namespace LibXR;
  ApplicationManager appmgr;

  // Auto-generated module instantiations
  static BlinkLED blink_led(hw, appmgr, 250);
  static BuzzerAlarm buzzer_alarm(hw, appmgr, 1500, 80, 80);
  static ICM20948 imu(hw, appmgr, ICM20948::DataRate::DATA_RATE_1KHZ, ICM20948::AcclRange::RANGE_16G, ICM20948::GyroRange::DPS_2000, {1.0, 0.0, 0.0, 0.0}, "icm20948_gyro", "icm20948_accl", 2048, "spi1", "IMU_CS", "IMU_INT");
  static MadgwickAHRS ahrs(hw, appmgr, 0.05, "icm20948_gyro", "icm20948_accl", "ahrs_quaternion", "ahrs_euler", 2048);

  while (true) {
    appmgr.MonitorAll();
    Thread::Sleep(1000);
  }
}