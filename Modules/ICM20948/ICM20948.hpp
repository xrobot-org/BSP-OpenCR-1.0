#pragma once

// clang-format off
/* === MODULE MANIFEST V2 ===
module_description: TDK ICM-20648/ICM-20948 SPI 6-axis IMU driver
constructor_args:
  - data_rate: ICM20948::DataRate::DATA_RATE_1KHZ
  - accl_range: ICM20948::AcclRange::RANGE_16G
  - gyro_range: ICM20948::GyroRange::DPS_2000
  - rotation:
      w: 1.0
      x: 0.0
      y: 0.0
      z: 0.0
  - gyro_topic_name: "icm20948_gyro"
  - accl_topic_name: "icm20948_accl"
  - task_stack_depth: 2048
  - spi_name: "spi1"
  - cs_pin_name: "IMU_CS"
  - int_pin_name: "IMU_INT"
template_args: []
required_hardware: spi1 IMU_CS IMU_INT
depends: []
=== END MANIFEST === */
// clang-format on

#include <array>
#include <cmath>
#include <cstdint>

#include "app_framework.hpp"
#include "gpio.hpp"
#include "message.hpp"
#include "spi.hpp"
#include "thread.hpp"
#include "transform.hpp"

class ICM20948 : public LibXR::Application {
 public:
  static constexpr float M_DEG2RAD_MULT = 0.01745329251f;
  static constexpr float STANDARD_GRAVITY = 9.80665f;

  enum class DataRate : uint8_t {
    DATA_RATE_4KHZ = 1,
    DATA_RATE_1KHZ = 4,
    DATA_RATE_500HZ = 8,
    DATA_RATE_250HZ = 16,
    DATA_RATE_125HZ = 32,
  };

  enum class GyroRange : uint8_t {
    DPS_250 = 0,
    DPS_500 = 1,
    DPS_1000 = 2,
    DPS_2000 = 3,
  };

  enum class AcclRange : uint8_t {
    RANGE_2G = 0,
    RANGE_4G = 1,
    RANGE_8G = 2,
    RANGE_16G = 3,
  };

  ICM20948(LibXR::HardwareContainer &hw, LibXR::ApplicationManager &app,
           DataRate data_rate, AcclRange accl_range, GyroRange gyro_range,
           LibXR::Quaternion<float> &&rotation, const char *gyro_topic_name,
           const char *accl_topic_name, size_t task_stack_depth,
           const char *spi_name = "spi1", const char *cs_pin_name = "IMU_CS",
           const char *int_pin_name = "IMU_INT")
      : data_rate_(data_rate),
        accl_range_(accl_range),
        gyro_range_(gyro_range),
        topic_gyro_(gyro_topic_name, sizeof(gyro_data_)),
        topic_accl_(accl_topic_name, sizeof(accl_data_)),
        cs_(hw.template FindOrExit<LibXR::GPIO>({cs_pin_name})),
        int_(hw.template FindOrExit<LibXR::GPIO>({int_pin_name})),
        spi_(hw.template FindOrExit<LibXR::SPI>({spi_name})),
        rotation_(std::move(rotation)),
        op_spi_(sem_spi_) {
    app.Register(*this);

    cs_->Write(true);
    int_->DisableInterrupt();
    int_->SetConfig(
        {LibXR::GPIO::Direction::RISING_INTERRUPT, LibXR::GPIO::Pull::NONE});

    auto int_cb = LibXR::GPIO::Callback::Create(
        [](bool in_isr, ICM20948 *self) {
          auto now = LibXR::Timebase::GetMicroseconds();
          self->dt_ = now - self->last_int_time_;
          self->last_int_time_ = now;
          self->new_data_.PostFromCallback(in_isr);
        },
        this);
    int_->RegisterCallback(int_cb);

    while (!Init()) {
      XR_LOG_ERROR("ICM20948: Init failed, who_am_i=0x%02X. Retry...", who_am_i_);
      LibXR::Thread::Sleep(100);
    }
    XR_LOG_PASS("ICM20948: Init success, who_am_i=0x%02X.", who_am_i_);

    thread_.Create(this, ThreadFunc, "icm20948_thread", task_stack_depth,
                   LibXR::Thread::Priority::REALTIME);
  }

  void OnMonitor() override {
    if (std::isinf(gyro_data_.x()) || std::isinf(gyro_data_.y()) ||
        std::isinf(gyro_data_.z()) || std::isinf(accl_data_.x()) ||
        std::isinf(accl_data_.y()) || std::isinf(accl_data_.z()) ||
        std::isnan(gyro_data_.x()) || std::isnan(gyro_data_.y()) ||
        std::isnan(gyro_data_.z()) || std::isnan(accl_data_.x()) ||
        std::isnan(accl_data_.y()) || std::isnan(accl_data_.z())) {
      XR_LOG_WARN("ICM20948: NaN data detected. gyro: %f %f %f, accl: %f %f %f",
                  gyro_data_.x(), gyro_data_.y(), gyro_data_.z(),
                  accl_data_.x(), accl_data_.y(), accl_data_.z());
    }
  }

 private:
  static constexpr uint8_t WHO_AM_I_ICM20948 = 0xEA;
  static constexpr uint8_t WHO_AM_I_ICM20648 = 0xE0;
  static constexpr uint8_t REG_BANK_SEL = 0x7F;
  static constexpr uint8_t BANK0_WHO_AM_I = 0x00;
  static constexpr uint8_t BANK0_USER_CTRL = 0x03;
  static constexpr uint8_t BANK0_LP_CONFIG = 0x05;
  static constexpr uint8_t BANK0_PWR_MGMT_1 = 0x06;
  static constexpr uint8_t BANK0_PWR_MGMT_2 = 0x07;
  static constexpr uint8_t BANK0_INT_PIN_CFG = 0x0F;
  static constexpr uint8_t BANK0_INT_ENABLE_1 = 0x11;
  static constexpr uint8_t BANK0_INT_STATUS_1 = 0x1A;
  static constexpr uint8_t BANK0_ACCEL_XOUT_H = 0x2D;
  static constexpr uint8_t BANK2_GYRO_SMPLRT_DIV = 0x00;
  static constexpr uint8_t BANK2_GYRO_CONFIG_1 = 0x01;
  static constexpr uint8_t BANK2_ACCEL_SMPLRT_DIV_1 = 0x10;
  static constexpr uint8_t BANK2_ACCEL_SMPLRT_DIV_2 = 0x11;
  static constexpr uint8_t BANK2_ACCEL_CONFIG = 0x14;
  static constexpr uint8_t READ_LEN = 14;
  static constexpr uint32_t CHIP_SELECT_SETTLE_US = 5;

  bool Init() {
    ConfigureBus();
    SelectBank(0);
    WriteSingle(BANK0_PWR_MGMT_1, 0x80);
    LibXR::Thread::Sleep(100);

    SelectBank(0);
    who_am_i_ = ReadSingle(BANK0_WHO_AM_I);
    if (who_am_i_ != WHO_AM_I_ICM20948 && who_am_i_ != WHO_AM_I_ICM20648) {
      return false;
    }

    WriteSingle(BANK0_PWR_MGMT_1, 0x01);
    LibXR::Thread::Sleep(10);
    WriteSingle(BANK0_PWR_MGMT_2, 0x00);
    WriteSingle(BANK0_LP_CONFIG, 0x00);
    WriteSingle(BANK0_USER_CTRL, 0x10);

    SelectBank(2);
    WriteSingle(BANK2_GYRO_SMPLRT_DIV, static_cast<uint8_t>(data_rate_) - 1U);
    WriteSingle(BANK2_GYRO_CONFIG_1,
                static_cast<uint8_t>((static_cast<uint8_t>(gyro_range_) << 1U) | 0x01U));
    WriteSingle(BANK2_ACCEL_SMPLRT_DIV_1, 0x00);
    WriteSingle(BANK2_ACCEL_SMPLRT_DIV_2, static_cast<uint8_t>(data_rate_) - 1U);
    WriteSingle(BANK2_ACCEL_CONFIG,
                static_cast<uint8_t>((static_cast<uint8_t>(accl_range_) << 1U) | 0x01U));

    SelectBank(0);
    WriteSingle(BANK0_INT_ENABLE_1, 0x00);
    // The onboard IMU uses the default active-high pulse interrupt path.
    WriteSingle(BANK0_INT_PIN_CFG, 0x00);
    ReadSingle(BANK0_INT_STATUS_1);
    int_->EnableInterrupt();
    WriteSingle(BANK0_INT_ENABLE_1, 0x01);
    return true;
  }

  static void ThreadFunc(ICM20948 *self) {
    while (true) {
      if (self->new_data_.Wait(50) == LibXR::ErrorCode::OK) {
        self->ReadSingle(BANK0_INT_STATUS_1);
        self->Read(BANK0_ACCEL_XOUT_H, READ_LEN);
        self->ReadSingle(BANK0_INT_STATUS_1);
        self->Parse();
        self->topic_gyro_.Publish(self->gyro_data_);
        self->topic_accl_.Publish(self->accl_data_);
      } else {
        XR_LOG_WARN("ICM20948 wait timeout.");
      }
    }
  }

  void ConfigureBus() {
    LibXR::SPI::Configuration config;
    config.clock_polarity = LibXR::SPI::ClockPolarity::HIGH;
    config.clock_phase = LibXR::SPI::ClockPhase::EDGE_2;
    config.prescaler = LibXR::SPI::Prescaler::DIV_32;
    spi_->SetConfig(config);
  }

  void SelectBank(uint8_t bank) { WriteSingle(REG_BANK_SEL, static_cast<uint8_t>(bank << 4U)); }

  void WriteSingle(uint8_t reg, uint8_t data) {
    AssertChipSelect();
    spi_->MemWrite(reg, data, op_spi_);
    DeassertChipSelect();
  }

  uint8_t ReadSingle(uint8_t reg) {
    uint8_t data = 0;
    Read(reg, &data, 1);
    return data;
  }

  void Read(uint8_t reg, uint8_t len) { Read(reg, buffer_, len); }

  void Read(uint8_t reg, uint8_t *data, uint8_t len) {
    AssertChipSelect();
    spi_->MemRead(reg, {data, static_cast<size_t>(len)}, op_spi_);
    DeassertChipSelect();
  }

  void AssertChipSelect() {
    cs_->Write(false);
    LibXR::Timebase::DelayMicroseconds(CHIP_SELECT_SETTLE_US);
  }

  void DeassertChipSelect() {
    LibXR::Timebase::DelayMicroseconds(CHIP_SELECT_SETTLE_US);
    cs_->Write(true);
  }

  void Parse() {
    std::array<int16_t, 3> accl_raw_i16;
    std::array<int16_t, 3> gyro_raw_i16;
    std::array<float, 3> accl_raw;
    std::array<float, 3> gyro_raw;

    for (int i = 0; i < 3; i++) {
      accl_raw_i16[i] = static_cast<int16_t>(buffer_[i * 2] << 8 | buffer_[i * 2 + 1]);
      accl_raw[i] = static_cast<float>(accl_raw_i16[i]) * GetAcclLSB();
      gyro_raw_i16[i] = static_cast<int16_t>(buffer_[i * 2 + 8] << 8 | buffer_[i * 2 + 9]);
      gyro_raw[i] = static_cast<float>(gyro_raw_i16[i]) * GetGyroLSB();
    }

    int16_t temp_raw = static_cast<int16_t>(buffer_[6] << 8 | buffer_[7]);
    temperature_ = static_cast<float>(temp_raw) / 333.87f + 21.0f;

    accl_data_ = rotation_ * Eigen::Matrix<float, 3, 1>(
                                 accl_raw[0], accl_raw[1], accl_raw[2]);
    gyro_data_ = rotation_ * Eigen::Matrix<float, 3, 1>(
                                 gyro_raw[0], gyro_raw[1], gyro_raw[2]);
  }

  float GetAcclLSB() const {
    switch (accl_range_) {
      case AcclRange::RANGE_2G:
        return 2.0f * STANDARD_GRAVITY / 32768.0f;
      case AcclRange::RANGE_4G:
        return 4.0f * STANDARD_GRAVITY / 32768.0f;
      case AcclRange::RANGE_8G:
        return 8.0f * STANDARD_GRAVITY / 32768.0f;
      case AcclRange::RANGE_16G:
        return 16.0f * STANDARD_GRAVITY / 32768.0f;
      default:
        ASSERT(false);
        return 0.0f;
    }
  }

  float GetGyroLSB() const {
    float dps = 0.0f;
    switch (gyro_range_) {
      case GyroRange::DPS_250:
        dps = 250.0f;
        break;
      case GyroRange::DPS_500:
        dps = 500.0f;
        break;
      case GyroRange::DPS_1000:
        dps = 1000.0f;
        break;
      case GyroRange::DPS_2000:
        dps = 2000.0f;
        break;
      default:
        ASSERT(false);
        break;
    }
    return dps / 32768.0f * M_DEG2RAD_MULT;
  }

  DataRate data_rate_;
  AcclRange accl_range_;
  GyroRange gyro_range_;
  float temperature_ = 0.0f;
  uint8_t who_am_i_ = 0;

  LibXR::MicrosecondTimestamp last_int_time_ = 0;
  LibXR::MicrosecondTimestamp::Duration dt_ = 0;

  uint8_t buffer_[READ_LEN] = {};
  Eigen::Matrix<float, 3, 1> gyro_data_, accl_data_;

  LibXR::Topic topic_gyro_, topic_accl_;
  LibXR::GPIO *cs_, *int_;
  LibXR::SPI *spi_;
  LibXR::Quaternion<float> rotation_;
  LibXR::Semaphore sem_spi_, new_data_;
  LibXR::SPI::OperationRW op_spi_;
  LibXR::Thread thread_;
};


