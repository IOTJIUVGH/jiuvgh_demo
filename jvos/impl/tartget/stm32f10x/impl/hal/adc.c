#include <stdint.h>

#include "device.h"
#include "analogin_api.h"

#include "mos.h"
#include "mhal/adc.h"

#include "peripheral_remap.h"

//amebaz2 don't support adc

merr_t mhal_adc_open(int adc, int pin)
{
    return kNoErr;
}

float mhal_adc_read(int adc)
{
    return kNoErr;
}

merr_t mhal_adc_close(int adc)
{
    return kNoErr;
}