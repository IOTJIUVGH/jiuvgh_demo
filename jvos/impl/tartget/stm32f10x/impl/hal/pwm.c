#include <stdint.h>

#include "device.h"
#include "pwmout_api.h"

#include "mos.h"
#include "mhal/pwm.h"

#include "peripheral_remap.h"

static pwmout_t pwm_dev[M_PWM_NUM];

merr_t mhal_pwm_open(int pwm, int polarity, int pin)
{
    pwmout_t *dev = &pwm_dev[pwm];

    pwmout_init(dev, gpio_remap[pin]);

    return kNoErr;
}

merr_t mhal_pwm_set_freq(int pwm, uint32_t frequency)
{
    pwmout_t *dev = &pwm_dev[pwm];

    pwmout_period(dev, 1.0 / frequency);

    return kNoErr;
}

merr_t mhal_pwm_set_duty(int pwm, float dutycycle)
{
    pwmout_t *dev = &pwm_dev[pwm];

    pwmout_write(dev, dutycycle / 100.0);

    return kNoErr;
}

merr_t mhal_pwm_close(int pwm)
{
    pwmout_t *dev = &pwm_dev[pwm];

    pwmout_stop(dev);
    pwmout_free(dev);

    return kNoErr;
}
