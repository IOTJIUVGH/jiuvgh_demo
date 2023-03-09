#include <stdint.h>

#include "device.h"
#include "i2c_api.h"

#include "mos.h"
#include "mhal/i2c.h"

#include "peripheral_remap.h"

static i2c_t i2c_dev[M_I2C_NUM];

merr_t mhal_i2c_open(int i2c, mhal_i2c_addr_width_t addrwidth, int frequency, mhal_i2c_pinmux_t *pinmux)
{
    i2c_t *dev = &i2c_dev[i2c];

    i2c_init(dev, gpio_remap[pinmux->sda], gpio_remap[pinmux->scl]);
    i2c_frequency(dev, frequency);

    return kNoErr;
}

merr_t mhal_i2c_write(int i2c, uint8_t addr, uint8_t *buf, uint32_t n)
{
    i2c_t *dev = &i2c_dev[i2c];

    return i2c_write(dev, addr, buf, n, 1) == n ? kNoErr : kTimeoutErr;
}

merr_t mhal_i2c_read(int i2c, uint8_t addr, uint8_t *buf, uint32_t n)
{
    i2c_t *dev = &i2c_dev[i2c];

    return i2c_read(dev, addr, buf, n, 1) == n ? kNoErr : kTimeoutErr;
}

merr_t mhal_i2c_close(int i2c)
{
    i2c_t *dev = &i2c_dev[i2c];

    i2c_reset(dev);

    return kNoErr;
}
