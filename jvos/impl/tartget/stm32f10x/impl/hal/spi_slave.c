#include <stdint.h>

#include "spi_api.h"
#include "spi_ex_api.h"

#include "mos.h"
#include "mhal/spi_slave.h"
#include "ringbuffer.h"

enum
{
    MXOS_SPI_1,
    MXOS_SPI_MAX
};

typedef struct
{
    spi_t spi;
    PinName mosi;
    PinName miso;
    PinName sclk;
    PinName ssel;
    ringbuffer_t ringbuf;
    uint32_t rxlen;
    mos_semphr_id_t txsem;
    mos_semphr_id_t rxsem;
} spi_slave_dev_t;

static spi_slave_dev_t spi_dev[] = {
    [MXOS_SPI_1] = {
        .spi.spi_idx = MBED_SPI0,
        .mosi = PA_23,
        .miso = PA_22,
        .sclk = PA_18,
        .ssel = PA_19,
    },
};

static void spi_bus_tx_done_callback(void *pdata, SpiIrq event);
static void slave_rx_handler(void *pdata, uint8_t *data, uint8_t n);

merr_t mhal_spi_slave_open(mhal_spi_t spi, uint32_t bufsize)
{
    uint8_t *buf;
    spi_slave_dev_t *dev = &spi_dev[spi];

    // Initialise Rx ringbuffer
    if ((buf = (uint8_t *)malloc(bufsize)) == NULL)
    {
        return kNoMemoryErr;
    }
    ringbuffer_init(&dev->ringbuf, buf, bufsize);
    dev->txsem = mos_semphr_new(1);
    dev->rxsem = mos_semphr_new(1);

    spi_init(&dev->spi, dev->mosi, dev->miso, dev->sclk, dev->ssel);

    spi_slave_rx_hook(&dev->spi, slave_rx_handler, &dev->spi);
    spi_bus_tx_done_irq_hook(&dev->spi, spi_bus_tx_done_callback, &dev->spi);

    return kNoErr;
}

merr_t mhal_spi_slave_format(mhal_spi_t spi, uint8_t mode, uint8_t bits)
{
    spi_slave_dev_t *dev = &spi_dev[spi];

    spi_format(&dev->spi, bits, mode, 1);

    return kNoErr;
}

merr_t mhal_spi_slave_write(mhal_spi_t spi, uint8_t *buf, uint32_t len)
{
    spi_slave_dev_t *dev = &spi_dev[spi];

    spi_slave_write_stream(&dev->spi, buf, len);

    mos_semphr_acquire(dev->txsem, 0xFFFFFFFF);

    return kNoErr;
}

merr_t mhal_spi_slave_read(mhal_spi_t spi, uint8_t *data, uint32_t *size, uint32_t timeout)
{
    uint32_t expect_len, frag_len;
    uint32_t ringbuf_total;
    uint32_t start_point, expired_time;

    spi_slave_dev_t *dev = &spi_dev[spi];

    expect_len = *size;
    start_point = mos_time();
    expired_time = 0;

    *size = 0;
    ringbuf_total = ringbuffer_total_size(&dev->ringbuf);

    while (expect_len > 0)
    {
        frag_len = expect_len > ringbuf_total ? ringbuf_total : expect_len;

        if (ringbuffer_used_space(&dev->ringbuf) < frag_len)
        {
            dev->rxlen = frag_len;
        }

        if (dev->rxlen > 0)
        {
            if (expired_time > timeout || mos_semphr_acquire(dev->rxsem, timeout - expired_time) != kNoErr)
            {
                dev->rxlen = 0;
                frag_len = ringbuffer_used_space(&dev->ringbuf);
                ringbuffer_read(&dev->ringbuf, data, frag_len, &frag_len);
                *size += frag_len;
                return kTimeoutErr;
            }
        }

        ringbuffer_read(&dev->ringbuf, data, frag_len, &frag_len);

        data += frag_len;
        *size += frag_len;
        expect_len -= frag_len;

        expired_time = mos_time() - start_point;
    }

    return kNoErr;
}

merr_t mhal_spi_slave_close(mhal_spi_t spi)
{
    spi_slave_dev_t *dev = &spi_dev[spi];

    spi_free(&dev->spi);
    mos_semphr_delete(dev->rxsem);
    mos_semphr_delete(dev->txsem);
    free(dev->ringbuf.buffer);

    return kNoErr;
}

static void spi_bus_tx_done_callback(void *pdata, SpiIrq event)
{
    spi_slave_dev_t *dev = (spi_slave_dev_t *)pdata;

    mos_semphr_release(dev->txsem);
}

static void slave_rx_handler(void *pdata, uint8_t *data, uint8_t n)
{
    spi_slave_dev_t *dev = (spi_slave_dev_t *)pdata;

    ringbuffer_write(&dev->ringbuf, data, n);

    // Notify thread if sufficient data are available
    if (dev->rxlen > 0 && ringbuffer_used_space(&dev->ringbuf) >= dev->rxlen)
    {
        mos_semphr_release(dev->rxsem);
        dev->rxlen = 0;
    }
}