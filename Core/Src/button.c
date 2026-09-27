#include "button.h"

#define BUTTON_DEBOUNCE_SAMPLES 5U
#define BUTTON_DEBOUNCE_DELAY_MS 1U

uint16_t button(GPIO_TypeDef *port, uint16_t pin, uint16_t active_state)
{
    uint8_t consecutive_high = 0U;
    uint8_t consecutive_low = 0U;

    for (uint32_t sample = 0U; sample < BUTTON_DEBOUNCE_SAMPLES; ++sample)
    {
        if (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET)
        {
            ++consecutive_high;
            consecutive_low = 0U;
        }
        else
        {
            ++consecutive_low;
            consecutive_high = 0U;
        }

        HAL_Delay(BUTTON_DEBOUNCE_DELAY_MS);
    }

    const uint8_t threshold = BUTTON_DEBOUNCE_SAMPLES / 2U;
    const uint8_t stable = active_state ? consecutive_high : consecutive_low;
    return (stable > threshold) ? 0xFFU : 0U;
}

uint16_t button_release(GPIO_TypeDef *port, uint16_t pin, uint16_t active_state)
{
    static uint8_t waiting_for_release = 0U;

    if (!waiting_for_release)
    {
        if (!button(port, pin, active_state))
        {
            return 0U;
        }

        waiting_for_release = 1U;
        return 0U;
    }

    const GPIO_PinState state = HAL_GPIO_ReadPin(port, pin);
    const uint8_t still_active = active_state
        ? (state == GPIO_PIN_SET)
        : (state == GPIO_PIN_RESET);

    if (still_active)
    {
        return 0U;
    }

    waiting_for_release = 0U;
    return 0xFFU;
}
