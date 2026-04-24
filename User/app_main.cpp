#include "app_main.h"

#include "cdc_uart.hpp"
#include "dfu/dfu_runtime.hpp"
#include "libxr.hpp"
#include "main.h"
#include "opencr_flash_layout.hpp"
#include "stm32_adc.hpp"
#include "stm32_can.hpp"
#include "stm32_canfd.hpp"
#include "stm32_dac.hpp"
#include "stm32_flash.hpp"
#include "stm32_gpio.hpp"
#include "stm32_i2c.hpp"
#include "stm32_power.hpp"
#include "stm32_pwm.hpp"
#include "stm32_spi.hpp"
#include "stm32_timebase.hpp"
#include "stm32_uart.hpp"
#include "stm32_usb_dev.hpp"
#include "stm32_watchdog.hpp"
#include "flash_map.hpp"

using namespace LibXR;

/* User Code Begin 1 */
/* User Code End 1 */
// NOLINTBEGIN
// clang-format off
/* External HAL Declarations */
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc3;
extern CAN_HandleTypeDef hcan2;
extern IWDG_HandleTypeDef hiwdg;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim13;
extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

/* DMA Resources */
static uint16_t adc1_buf[32] __attribute__((section(".dma_buffer")));
static uint16_t adc3_buf[16] __attribute__((section(".dma_buffer")));
static uint8_t usart2_tx_buf[128] __attribute__((section(".dma_buffer")));
static uint8_t usart2_rx_buf[128] __attribute__((section(".dma_buffer")));
static uint8_t usart3_tx_buf[128] __attribute__((section(".dma_buffer")));
static uint8_t usart3_rx_buf[128] __attribute__((section(".dma_buffer")));
static uint8_t usb_otg_fs_ep0_in_buf[64];
static uint8_t usb_otg_fs_ep0_out_buf[64];
static uint8_t usb_otg_fs_ep1_in_buf[128];
static uint8_t usb_otg_fs_ep1_out_buf[128];
static uint8_t usb_otg_fs_ep2_in_buf[16];


static void JumpToBootloader(void*) {
  NVIC_SystemReset();
  while (true) {}
}

extern "C" void app_main(void) {
  SCB->VTOR = OpenCR::APP_BASE;
  __DSB();
  __ISB();
  // clang-format on
  // NOLINTEND
  /* User Code Begin 2 */
  /* User Code End 2 */
  // clang-format off
  // NOLINTBEGIN
  STM32TimerTimebase timebase(&htim13);
  PlatformInit(2, 1024);
  STM32PowerManager power_manager;

  /* GPIO Configuration */
  STM32GPIO IMU_INT(IMU_INT_GPIO_Port, IMU_INT_Pin, EXTI1_IRQn);
  STM32GPIO IMU_CS(IMU_CS_GPIO_Port, IMU_CS_Pin);
  STM32GPIO KEY2(KEY2_GPIO_Port, KEY2_Pin, EXTI15_10_IRQn);
  STM32GPIO DXL_DIR(DXL_DIR_GPIO_Port, DXL_DIR_Pin);
  STM32GPIO LED3(LED3_GPIO_Port, LED3_Pin);
  STM32GPIO LED2(LED2_GPIO_Port, LED2_Pin);
  STM32GPIO SW2(SW2_GPIO_Port, SW2_Pin);
  STM32GPIO BUZZER_SIG(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin);
  STM32GPIO DXL_PWR_EN(DXL_PWR_EN_GPIO_Port, DXL_PWR_EN_Pin);
  STM32GPIO LED4(LED4_GPIO_Port, LED4_Pin);
  STM32GPIO SW1(SW1_GPIO_Port, SW1_Pin);
  STM32GPIO LED1(LED1_GPIO_Port, LED1_Pin);
  STM32GPIO KEY1(KEY1_GPIO_Port, KEY1_Pin, EXTI3_IRQn);
  STM32GPIO LED_RUN(LED_RUN_GPIO_Port, LED_RUN_Pin);
  STM32ADC adc1(&hadc1, adc1_buf, {ADC_CHANNEL_VREFINT, ADC_CHANNEL_VBAT}, 3.3);
  auto adc1_adc_channel_vrefint = adc1.GetChannel(0);
  UNUSED(adc1_adc_channel_vrefint);
  auto adc1_adc_channel_vbat = adc1.GetChannel(1);
  UNUSED(adc1_adc_channel_vbat);

  STM32ADC adc3(&hadc3, adc3_buf, {ADC_CHANNEL_10}, 3.3);
  auto adc3_adc_channel_10 = adc3.GetChannel(0);
  UNUSED(adc3_adc_channel_10);


  STM32SPI spi1(&hspi1, {nullptr, 0}, {nullptr, 0}, 3);

  STM32UART usart2(&huart2,
              usart2_rx_buf, usart2_tx_buf, 5);

  STM32UART usart3(&huart3,
              usart3_rx_buf, usart3_tx_buf, 5);

  STM32CAN can2(&hcan2, 5);

  static constexpr auto USB_OTG_FS_LANG_PACK = LibXR::USB::DescriptorStrings::MakeLanguagePack(LibXR::USB::DescriptorStrings::Language::EN_US, "XRobot", "OpenCR App", "XRUSB-DEMO-");
  LibXR::USB::CDCUart usb_otg_fs_cdc(128, 128, 3);
  LibXR::USB::DfuRuntimeClass usb_otg_fs_dfu_rt(JumpToBootloader, nullptr, 50, "OpenCR Runtime DFU");

  STM32USBDeviceOtgFS usb_fs(
      &hpcd_USB_OTG_FS,
      256,
      {usb_otg_fs_ep0_out_buf, usb_otg_fs_ep1_out_buf},
      {{usb_otg_fs_ep0_in_buf, 64}, {usb_otg_fs_ep1_in_buf, 128}, {usb_otg_fs_ep2_in_buf, 16}},
      USB::DeviceDescriptor::PacketSize0::SIZE_64,
      0x1D50, 0x6199, 0x100,
      {&USB_OTG_FS_LANG_PACK},
      {{&usb_otg_fs_cdc, &usb_otg_fs_dfu_rt}},
      {reinterpret_cast<void *>(UID_BASE), 12}
  );
  usb_fs.Init(false);
  usb_fs.Start(false);

  STM32Watchdog iwdg(&hiwdg, 1000, 250);
  iwdg.Feed();

  // clang-format on
  // NOLINTEND
  /* User Code Begin 3 */
  while (true) {
    iwdg.Feed();
    HAL_GPIO_TogglePin(LED_RUN_GPIO_Port, LED_RUN_Pin);
    Thread::Sleep(100);
  }
  /* User Code End 3 */
}

