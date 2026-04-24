#pragma once

#include <cstdint>

#include "stm32_flash.hpp"

namespace OpenCR
{

static constexpr uint32_t APP_BASE = 0x08020000u;
static constexpr uint32_t APP_SIZE = 0x000E0000u;
static constexpr uint32_t APP_SEAL_OFFSET = 0x000C0000u;
static constexpr size_t APP_START_SECTOR = 5u;
static constexpr uint32_t BOOT_MAGIC = 0x4F435241u;  // "ARCO"

static constexpr LibXR::FlashSector FLASH_SECTORS[] = {
    {0x08000000u, 0x00008000u},
    {0x08008000u, 0x00008000u},
    {0x08010000u, 0x00008000u},
    {0x08018000u, 0x00008000u},
    {0x08020000u, 0x00020000u},
    {0x08040000u, 0x00040000u},
    {0x08080000u, 0x00040000u},
    {0x080C0000u, 0x00040000u},
};

}  // namespace OpenCR
