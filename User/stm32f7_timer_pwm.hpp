#pragma once

#include "main.h"

#include "pwm.hpp"

namespace LibXR
{

class STM32F7TimerPWM : public PWM
{
 public:
  STM32F7TimerPWM(TIM_HandleTypeDef* htim, GPIO_TypeDef* gpio, uint16_t pin);

  ErrorCode SetDutyCycle(float value) override;
  ErrorCode SetConfig(Configuration config) override;
  ErrorCode Enable() override;
  ErrorCode Disable() override;

  void OnPeriodElapsed(TIM_HandleTypeDef* htim);

 private:
  TIM_HandleTypeDef* htim_;
  GPIO_TypeDef* gpio_;
  uint16_t pin_;
  bool enabled_ = false;

  static uint32_t GetTimerClockHz(TIM_TypeDef* instance);
};

}  // namespace LibXR

extern "C" void STM32F7TimerPWM_OnPeriodElapsed(TIM_HandleTypeDef* htim);

namespace
{
inline LibXR::STM32F7TimerPWM* g_instances[4] = {};
}

namespace LibXR
{

inline STM32F7TimerPWM::STM32F7TimerPWM(TIM_HandleTypeDef* htim, GPIO_TypeDef* gpio, uint16_t pin)
    : htim_(htim), gpio_(gpio), pin_(pin)
{
  for (auto*& instance : g_instances)
  {
    if (instance == nullptr)
    {
      instance = this;
      break;
    }
  }
}

inline ErrorCode STM32F7TimerPWM::SetDutyCycle(float value)
{
  UNUSED(value);
  return ErrorCode::OK;
}

inline ErrorCode STM32F7TimerPWM::SetConfig(Configuration config)
{
  if (htim_ == nullptr || gpio_ == nullptr || config.frequency == 0u)
  {
    return ErrorCode::ARG_ERR;
  }

  const uint32_t timer_clock_hz = GetTimerClockHz(htim_->Instance);
  if (timer_clock_hz == 0u || config.frequency > (UINT32_MAX / 2u))
  {
    return ErrorCode::INIT_ERR;
  }

  const uint32_t update_hz = config.frequency * 2u;
  uint32_t prescaler = 1u;
  uint32_t period = 0u;

  for (; prescaler <= 0x10000u; ++prescaler)
  {
    const uint32_t candidate = timer_clock_hz / (prescaler * update_hz);
    if (candidate > 0u && candidate <= 0x10000u)
    {
      period = candidate;
      break;
    }
  }

  if (period == 0u)
  {
    return ErrorCode::INIT_ERR;
  }

  const bool was_enabled = enabled_;
  if (was_enabled)
  {
    Disable();
  }

  htim_->Init.Prescaler = prescaler - 1u;
  htim_->Init.Period = period - 1u;
  htim_->Init.CounterMode = TIM_COUNTERMODE_UP;
  htim_->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

  if (HAL_TIM_Base_Init(htim_) != HAL_OK)
  {
    return ErrorCode::INIT_ERR;
  }

  __HAL_TIM_SET_COUNTER(htim_, 0u);
  __HAL_TIM_CLEAR_FLAG(htim_, TIM_FLAG_UPDATE);

  if (was_enabled)
  {
    return Enable();
  }

  return ErrorCode::OK;
}

inline ErrorCode STM32F7TimerPWM::Enable()
{
  if (htim_ == nullptr || gpio_ == nullptr)
  {
    return ErrorCode::ARG_ERR;
  }

  HAL_GPIO_WritePin(gpio_, pin_, GPIO_PIN_RESET);
  __HAL_TIM_SET_COUNTER(htim_, 0u);
  __HAL_TIM_CLEAR_FLAG(htim_, TIM_FLAG_UPDATE);

  if (HAL_TIM_Base_Start_IT(htim_) != HAL_OK)
  {
    return ErrorCode::FAILED;
  }

  enabled_ = true;
  return ErrorCode::OK;
}

inline ErrorCode STM32F7TimerPWM::Disable()
{
  if (htim_ == nullptr || gpio_ == nullptr)
  {
    return ErrorCode::ARG_ERR;
  }

  if (HAL_TIM_Base_Stop_IT(htim_) != HAL_OK)
  {
    return ErrorCode::FAILED;
  }

  enabled_ = false;
  HAL_GPIO_WritePin(gpio_, pin_, GPIO_PIN_RESET);
  return ErrorCode::OK;
}

inline void STM32F7TimerPWM::OnPeriodElapsed(TIM_HandleTypeDef* htim)
{
  if (enabled_ && htim == htim_)
  {
    HAL_GPIO_TogglePin(gpio_, pin_);
  }
}

inline uint32_t STM32F7TimerPWM::GetTimerClockHz(TIM_TypeDef* instance)
{
#if defined(TIM1) || defined(TIM8) || defined(TIM9) || defined(TIM10) || defined(TIM11)
  if (
#if defined(TIM1)
      instance == TIM1 ||
#endif
#if defined(TIM8)
      instance == TIM8 ||
#endif
#if defined(TIM9)
      instance == TIM9 ||
#endif
#if defined(TIM10)
      instance == TIM10 ||
#endif
#if defined(TIM11)
      instance == TIM11 ||
#endif
      false)
  {
    uint32_t clock_hz = HAL_RCC_GetPCLK2Freq();
#ifdef RCC_CFGR_PPRE2
    if ((RCC->CFGR & RCC_CFGR_PPRE2) != RCC_CFGR_PPRE2_DIV1)
    {
      clock_hz *= 2u;
    }
#endif
    return clock_hz;
  }
#endif

  uint32_t clock_hz = HAL_RCC_GetPCLK1Freq();
#ifdef RCC_CFGR_PPRE1
  if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
  {
    clock_hz *= 2u;
  }
#endif
  return clock_hz;
}

}  // namespace LibXR

extern "C" void STM32F7TimerPWM_OnPeriodElapsed(TIM_HandleTypeDef* htim)
{
  for (auto* instance : g_instances)
  {
    if (instance != nullptr)
    {
      instance->OnPeriodElapsed(htim);
    }
  }
}
