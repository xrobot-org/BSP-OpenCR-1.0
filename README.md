# BSP-OpenCR-1.0

OpenCR 1.0（STM32F746ZGTx）的 LibXR / XRobot BSP，基于 FreeRTOS。可构建完整固件，或 USB DFU bootloader 与配套应用两个镜像。

## 目录

```text
OpenCR1.0.ioc             CubeMX 工程
Core/ Drivers/            CubeMX 生成的初始化代码与 ST HAL / CMSIS
Middlewares/              FreeRTOS、LibXR submodule
Modules/modules.yaml      需要的模块（`xrobot:` 固定 XRobot 版本）
Modules/sources.yaml      模块源
xrobot.lock               模块的精确 commit
User/app_main.cpp         入口：由代码生成器生成，注册硬件并调用 XROBOT_MAIN()
User/bootloader_main.cpp  DFU bootloader 入口（不使用 XRobot 模块）
User/libxr_config.yaml    LibXR 外设参数（`generator:` 固定代码生成器版本）
User/xrobot.yaml          产品配置（BlinkLED、BuzzerAlarm、ICM20948、MadgwickAHRS）
STM32F746XX_*.ld          完整固件 / bootloader / 应用的链接脚本
```

`Modules/<owner>/<Repo>/`、`Modules/CMakeLists.txt` 和 `User/xrobot_main.hpp` 由 `xrobot` 生成，不提交。

## 准备

```bash
git clone --recursive https://github.com/xrobot-org/BSP-OpenCR-1.0.git
cd BSP-OpenCR-1.0
pip install xrobot==1.0.0 libxr==6.0.0   # 与 xrobot: 和 generator: 一致
xrobot setup                             # 拉取模块、检查配置、生成入口头文件
```

已克隆的仓库先运行 `git submodule update --init --recursive`。三种镜像都需要先运行 `xrobot setup`。

构建需要 CMake、Ninja，以及 `starm-clang`（Preset 使用 `cmake/starm-clang.cmake`，picolibc 配置）或 `arm-none-eabi-gcc`（`cmake/gcc-arm-none-eabi.cmake`，CI 使用）在 `PATH` 中。

## 构建

`OPENCR_IMAGE` 选择镜像：

| `OPENCR_IMAGE` | 入口 | 链接脚本 | Preset |
| --- | --- | --- | --- |
| `default` | `app_main.cpp` | `STM32F746xx_FLASH.ld`（0x08000000） | `Debug`、`Release` |
| `bootloader` | `bootloader_main.cpp` | `STM32F746XX_BOOTLOADER.ld`（0x08000000，128 KiB） | `BootloaderDebug` |
| `app` | `app_main.cpp` | `STM32F746XX_APP.ld`（0x08040000） | `AppDebug` |

```bash
cmake --preset Debug
cmake --build --preset Debug
```

输出在 `build/<preset>/OpenCR1.0.elf`。不用 Preset 时（与 CI 相同）：

```bash
cmake . -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -DCMAKE_BUILD_TYPE=Release -DOPENCR_IMAGE=app -Bbuild -G Ninja
cmake --build build
```

bootloader 以 USB DFU 设备（"OpenCR App DFU"）接收应用镜像并写入 0x08040000 起的应用区，然后跳转到应用。

构建前 LibXR 检查 `User/xrobot_main.hpp` 是否比配置、锁文件、入口和模块头文件新，过期时构建失败并提示对应的 `xrobot gen -c <配置>`。修改 `User/xrobot.yaml` 或添加其他 `User/*.yaml` 产品配置的方法见 [项目管理（XRobot）](https://xrobot.work/docs/proj_man)。配置里的硬件名是 `User/app_main.cpp` 中 `XR_REGISTER` 注册的对象名（如 `LED1`、`spi1`、`can2`、`pwm_buzzer`、`database`）。

## 修改 CubeMX 配置后

在 CubeMX 中生成代码后，重新生成 BSP 对象（与 CI 相同的命令）：

```bash
libxr parse -d . -o .config.yaml
libxr gen -i .config.yaml -o User/app_main.cpp --xrobot --libxr-config User/libxr_config.yaml
```

`User/app_main.cpp` 中 `User Code` 区域的内容会保留。提交 `User/app_main.cpp`、`User/app_main.h`、`User/flash_map.hpp` 和 `User/libxr_config.yaml`；CI 会重新生成并检查它们与提交一致。

## CI

`.github/workflows/xrobot_stm32.yml` 在 `ghcr.io/xrobot-org/docker-image-stm32:main` 中：安装固定版本的工具，重新生成并检查 BSP 对象，运行 `xrobot format --check` 和 `xrobot setup --frozen --context-ref <被构建的分支> --release-ref <目标分支>`，然后用 `arm-none-eabi-gcc` 分别构建 `default`、`bootloader`、`app` 三个镜像。

分支：`dev` 接收修改；稳定线 `main` 只通过从 `dev` 发起的 PR 更新。目标为 `dev` 时锁定的模块提交必须在模块的 `dev` 上，目标为 `main` 或标签时必须在模块的 `master`（或 `main`）上。

## 许可

本仓库以 Apache-2.0 发布，见 [LICENSE](LICENSE)。随仓库分发的第三方代码保留各自的许可，见 [NOTICE](NOTICE)。
