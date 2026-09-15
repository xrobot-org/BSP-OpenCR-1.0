#include <cstdint>

#include "FreeRTOS.h"
#include "task.h"

#include "dfu/dfu.hpp"
#include "libxr.hpp"
#include "main.h"
#include "stm32_flash.hpp"
#include "stm32_timebase.hpp"
#include "stm32_usb_dev.hpp"

extern IWDG_HandleTypeDef hiwdg;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern TIM_HandleTypeDef htim13;

namespace
{

constexpr uint32_t RAM_BASE = 0x20000000u;
constexpr uint32_t RAM_END = 0x20050000u;
constexpr uint32_t APP_BASE = 0x08040000u;
constexpr uint32_t APP_SIZE = 0x00040000u;
constexpr uint32_t APP_SEAL_OFFSET = 0x0003FFF0u;
constexpr size_t APP_START_SECTOR = 6u;
constexpr uint32_t APP_FLASH_END = APP_BASE + APP_SEAL_OFFSET;
constexpr LibXR::FlashSector FLASH_SECTORS[] = {
    {0x08000000u, 0x00008000u}, {0x08008000u, 0x00008000u},
    {0x08010000u, 0x00008000u}, {0x08018000u, 0x00008000u},
    {0x08020000u, 0x00020000u}, {0x08040000u, 0x00040000u},
    {0x08080000u, 0x00040000u}, {0x080C0000u, 0x00040000u},
};
uint8_t ep0_in_buf[64];
uint8_t ep0_out_buf[64];

void JumpToAppThunk(void*);

void SetStatusLed(GPIO_PinState led1, GPIO_PinState led2, GPIO_PinState led3,
                  GPIO_PinState led4, GPIO_PinState status)
{
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, led1);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, led2);
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, led3);
  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, led4);
  HAL_GPIO_WritePin(LED_RUN_GPIO_Port, LED_RUN_Pin, status);
}

bool AppVectorIsValid()
{
  const auto stack = *reinterpret_cast<const uint32_t*>(APP_BASE);
  const auto reset = *reinterpret_cast<const uint32_t*>(APP_BASE + 4u);
  return (stack >= RAM_BASE && stack <= RAM_END && (stack % 4u) == 0u &&
          reset >= APP_BASE && reset < APP_FLASH_END && (reset & 1u) == 1u);
}

void BoardDeinit()
{
  // Keep the priority-zero HAL timebase alive, but prevent RTOS task switches.
  taskENTER_CRITICAL();
  HAL_PCD_Stop(&hpcd_USB_OTG_FS);
  HAL_PCD_DeInit(&hpcd_USB_OTG_FS);

  HAL_RCC_DeInit();
  HAL_DeInit();

  SCB_DisableICache();
  SCB_DisableDCache();

  __disable_irq();
  SysTick->CTRL = 0u;
  SysTick->LOAD = 0u;
  SysTick->VAL = 0u;
  SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;

  for (uint32_t i = 0u; i < 8u; ++i)
  {
    NVIC->ICER[i] = 0xFFFFFFFFu;
    NVIC->ICPR[i] = 0xFFFFFFFFu;
  }

  __DSB();
  __ISB();
}

// No compiler frame may be accessed after changing PSP/MSP selection.
[[noreturn]] __attribute__((naked, noinline)) void JumpToAppVector(uint32_t)
{
  asm volatile(
      "ldr r1, [r0, #0]    \n"
      "ldr r2, [r0, #4]    \n"
      "movs r3, #0         \n"
      "msr control, r3     \n"
      "msr basepri, r3     \n"
      "msr faultmask, r3   \n"
      "msr msp, r1         \n"
      "dsb                 \n"
      "isb                 \n"
      "cpsie i             \n"
      "bx r2               \n");
}

[[noreturn]] void JumpToAppNow()
{
  BoardDeinit();
  SCB->VTOR = APP_BASE;
  __DSB();
  __ISB();
  JumpToAppVector(APP_BASE);
}

void JumpToAppThunk(void*)
{
  JumpToAppNow();
}

}  // namespace

extern "C" void app_main(void)
{
  LibXR::STM32TimerTimebase timebase(&htim13);
  LibXR::PlatformInit();

  LibXR::STM32Flash app_flash(FLASH_SECTORS,
                              sizeof(FLASH_SECTORS) /
                                  sizeof(FLASH_SECTORS[0]),
                              APP_START_SECTOR);
  LibXR::USB::DfuBootloaderClassT<1024> dfu(app_flash, 0, APP_SIZE,
                                            APP_SEAL_OFFSET,
                                            JumpToAppThunk,
                                            nullptr, false, "OpenCR App DFU");

  static constexpr auto lang_pack = LibXR::USB::DescriptorStrings::MakeLanguagePack(
      LibXR::USB::DescriptorStrings::Language::EN_US, "XRobot", "OpenCR Bootloader",
      "OPENCR-BL-");

  LibXR::STM32USBDeviceOtgFS usb_fs(
      &hpcd_USB_OTG_FS, 256, {ep0_out_buf}, {{ep0_in_buf, 64}},
      LibXR::USB::DeviceDescriptor::PacketSize0::SIZE_64, 0x1D50, 0x619A, 0x100,
      {&lang_pack}, {{&dfu}}, {reinterpret_cast<void*>(UID_BASE), 12});

  usb_fs.Init(false);
  usb_fs.Start(false);

  while (true)
  {
    HAL_IWDG_Refresh(&hiwdg);
    dfu.Process();

    if (dfu.HasValidImage())
    {
      SetStatusLed(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET,
                   GPIO_PIN_SET);
    }
    else
    {
      SetStatusLed(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET,
                   GPIO_PIN_SET);
    }

    if (dfu.TryConsumeAppLaunch(HAL_GetTick()) && AppVectorIsValid())
    {
      JumpToAppNow();
    }

    LibXR::Thread::Sleep(10);
  }
}
