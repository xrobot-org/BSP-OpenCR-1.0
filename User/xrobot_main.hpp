#pragma once
// xrobot-stamp: config=xrobot.yaml sha256=caef887f9746f674521ab1ed6a0263b0b62678e1c3c506023497834b88e569e8
// xrobot-stamp: lock=../xrobot.lock sha256=82695426ce6203c2862d2f2f141c4f4a4aca76ffd1c445a9c7e1bdee7f704edd
// xrobot-stamp: tool=xrobot 0.3.1

#include <memory>
#include <type_traits>
#include <utility>
#include "libxr.hpp"
#include "thread.hpp"
#include "BlinkLED.hpp"
#include "BuzzerAlarm.hpp"
#include "ICM20948.hpp"
#include "MadgwickAHRS.hpp"

namespace xrobot_generated {
template <typename...> struct TypeList {};
template <typename Source, typename... Views>
struct RegistrationMatches
    : std::bool_constant<(!std::is_reference<Views>::value && ...) &&
                         (std::is_convertible<Source*, Views*>::value && ...)> {};

}  // namespace xrobot_generated

// Force only this entry inline in optimized Clang builds.
#if defined(__clang__) && defined(__OPTIMIZE__) && !defined(LIBXR_DEBUG_BUILD) && \
    ((defined(XROBOT_OPTIMIZED_BUILD) && XROBOT_OPTIMIZED_BUILD) || \
     (!defined(XROBOT_OPTIMIZED_BUILD) && defined(NDEBUG)))
#define XR_XROBOT_MAIN_INLINE [[gnu::always_inline]] inline
#else
#define XR_XROBOT_MAIN_INLINE inline
#endif

[[noreturn]] XR_XROBOT_MAIN_INLINE void XRobotMain(
    LibXR::GPIO& IMU_INT,
    LibXR::GPIO& IMU_CS,
    LibXR::GPIO& LED1,
    LibXR::SPI& spi1,
    LibXR::RamFS& ramfs,
    LibXR::PWM& pwm_buzzer,
    LibXR::Database& database) {
  // modules[0]: blink_led
  static BlinkLED blink_led(
      static_cast<LibXR::GPIO&>(LED1)
      , 250
  );
  // modules[1]: buzzer_alarm
  static BuzzerAlarm buzzer_alarm(
      static_cast<LibXR::PWM&>(pwm_buzzer)
      , 1500
      , 80
      , 80
  );
  static const ICM20948::Param& xr_arg_imu_param =
      {
.data_rate = ICM20948::DataRate::DATA_RATE_1KHZ
, .accl_range = ICM20948::AcclRange::RANGE_16G
, .gyro_range = ICM20948::GyroRange::DPS_2000
, .rotation = {1.0f, 0.0f, 0.0f, 0.0f}
, .gyro_topic_name = "icm20948_gyro"
, .accl_topic_name = "icm20948_accl"
, .task_stack_depth = 2048
}
  ;
  // modules[2]: imu
  static ICM20948 imu(
      static_cast<LibXR::GPIO&>(IMU_CS)
      , static_cast<LibXR::GPIO&>(IMU_INT)
      , static_cast<LibXR::SPI&>(spi1)
      , static_cast<LibXR::Database&>(database)
      , static_cast<LibXR::RamFS&>(ramfs)
      , xr_arg_imu_param
  );
  static const MadgwickAHRS::Param& xr_arg_ahrs_param =
      {
.beta = 0.05f
, .gyro_topic_name = "icm20948_gyro"
, .accl_topic_name = "icm20948_accl"
, .quaternion_topic_name = "ahrs_quaternion"
, .euler_topic_name = "ahrs_euler"
, .task_stack_depth = 2048
}
  ;
  // modules[3]: ahrs
  static MadgwickAHRS ahrs(
      static_cast<LibXR::RamFS&>(ramfs)
      , xr_arg_ahrs_param
  );
  static_assert(std::is_void_v<decltype(blink_led.OnMonitor())>, "blink_led.OnMonitor() must return void");
  static_assert(std::is_void_v<decltype(buzzer_alarm.OnMonitor())>, "buzzer_alarm.OnMonitor() must return void");
  static_assert(std::is_void_v<decltype(imu.OnMonitor())>, "imu.OnMonitor() must return void");
  static_assert(std::is_void_v<decltype(ahrs.OnMonitor())>, "ahrs.OnMonitor() must return void");
  for (;;) {
    blink_led.OnMonitor();
    buzzer_alarm.OnMonitor();
    imu.OnMonitor();
    ahrs.OnMonitor();
    LibXR::Thread::Sleep(1000);
  }
}

#undef XR_XROBOT_MAIN_INLINE

/* User Code Begin XRobotMain */
/* User Code End XRobotMain */
// clang-format off
// NOLINTBEGIN
#define XR_REGISTER_DETAIL_power_manager(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::PowerManager>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(power_manager)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_IMU_INT(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::GPIO>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(IMU_INT)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_IMU_CS(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::GPIO>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(IMU_CS)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_KEY2(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::GPIO>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(KEY2)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_DXL_DIR(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::GPIO>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(DXL_DIR)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_LED3(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::GPIO>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(LED3)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_LED2(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::GPIO>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(LED2)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_SW2(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::GPIO>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(SW2)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_BUZZER_SIG(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::GPIO>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(BUZZER_SIG)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_DXL_PWR_EN(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::GPIO>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(DXL_PWR_EN)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_LED4(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::GPIO>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(LED4)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_SW1(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::GPIO>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(SW1)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_LED1(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::GPIO>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(LED1)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_KEY1(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::GPIO>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(KEY1)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_LED_RUN(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::GPIO>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(LED_RUN)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_adc1_adc_channel_vrefint(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::ADC>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(adc1_adc_channel_vrefint)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_adc1_adc_channel_vbat(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::ADC>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(adc1_adc_channel_vbat)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_adc3_adc_channel_10(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::ADC>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(adc3_adc_channel_10)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_spi1(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::SPI>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(spi1)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_usart2(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::UART>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(usart2)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_usart3(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::UART>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(usart3)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_can2(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::CAN>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(can2)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_iwdg(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::Watchdog>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(iwdg)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_usb_otg_fs_cdc(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::UART>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(usb_otg_fs_cdc)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_ramfs(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::RamFS>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(ramfs)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_pwm_buzzer(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::PWM>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(pwm_buzzer)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_database(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::Database>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(database)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER(name, ...) XR_REGISTER_DETAIL_##name(__VA_ARGS__)

#define XROBOT_MAIN() ::XRobotMain(IMU_INT, IMU_CS, LED1, spi1, ramfs, pwm_buzzer, database)

// NOLINTEND
// clang-format on
