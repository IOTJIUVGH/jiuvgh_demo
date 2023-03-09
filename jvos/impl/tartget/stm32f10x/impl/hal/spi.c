#include <stdint.h>

#include "spi_api.h"
#include "spi_ex_api.h"

#include "mos.h"
#include "mhal/spi.h"

#include "peripheral_remap.h"

enum
{
    MXOS_SPI_1,
    MXOS_SPI_MAX
};

typedef struct
{
    spi_t spi;
    mos_semphr_id_t txsem;
} spi_dev_t;

static spi_dev_t spi_dev[M_SPI_NUM];

static void irq_handler(void *pdata, SpiIrq event);

merr_t mhal_spi_open(int spi, mhal_spi_pinmux_t *pinmux)
{
    spi_dev_t *dev = &spi_dev[spi];

    dev->txsem = mos_semphr_new(1);

    dev->spi.hal_ssi_adaptor.index = spi_remap[spi];

    spi_init(&dev->spi, gpio_remap[pinmux->mosi], gpio_remap[pinmux->miso], gpio_remap[pinmux->clk], NC);

    spi_irq_hook(&dev->spi, irq_handler, dev);

    return kNoErr;
}

merr_t mhal_spi_format(int spi, uint32_t speed, uint8_t mode, uint8_t bits)
{
    spi_dev_t *dev = &spi_dev[spi];

    spi_frequency(&dev->spi, speed);

    spi_format(&dev->spi, bits, mode, 0);

    return kNoErr;
}

merr_t mhal_spi_write_and_read(int spi, uint8_t *txbuf, uint8_t *rxbuf, uint32_t len)
{
    spi_dev_t *dev = &spi_dev[spi];

    if (txbuf == NULL && rxbuf == NULL)
    {
        return kParamErr;
    }

    if (txbuf == NULL)
    {
        spi_master_read_stream_dma(&dev->spi, rxbuf, len);
    }
    else if (rxbuf == NULL)
    {
        spi_master_write_stream_dma(&dev->spi, txbuf, len);
    }
    else
    {
        spi_master_write_read_stream_dma(&dev->spi, txbuf, rxbuf, len);
    }

    mos_semphr_acquire(dev->txsem, 0xFFFFFFFF);

    return kNoErr;
}

merr_t mhal_spi_close(int spi)
{
    spi_dev_t *dev = &spi_dev[spi];

    spi_free(&dev->spi);

    return kNoErr;
}

static void irq_handler(void *pdata, SpiIrq event)
{
    spi_dev_t *dev = (spi_dev_t *)pdata;

    if (event == SpiRxIrq)
    {
        mos_semphr_release(dev->txsem);
    }
}