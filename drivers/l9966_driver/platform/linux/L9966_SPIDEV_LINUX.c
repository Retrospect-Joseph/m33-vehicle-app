#include "L9966_SPIDEV_LINUX.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static bool configure_spi_mode(int fd)
{
    uint8_t mode = SPI_MODE_1;

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
        printf("SPI_IOC_WR_MODE failed: %s\n", strerror(errno));
        return false;
    }

    if (ioctl(fd, SPI_IOC_RD_MODE, &mode) < 0) {
        printf("SPI_IOC_RD_MODE failed: %s\n", strerror(errno));
        return false;
    }

    if (mode != SPI_MODE_1) {
        printf("SPI mode readback mismatch. Expected=%u Got=%u\n",
               (unsigned int)SPI_MODE_1,
               (unsigned int)mode);
        return false;
    }

    return true;
}

static bool configure_spi_bits_per_word(int fd, uint8_t bits_per_word)
{
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0) {
        printf("SPI_IOC_WR_BITS_PER_WORD failed: %s\n", strerror(errno));
        return false;
    }

    if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits_per_word) < 0) {
        printf("SPI_IOC_RD_BITS_PER_WORD failed: %s\n", strerror(errno));
        return false;
    }

    return true;
}

static bool configure_spi_speed(int fd, uint32_t speed_hz)
{
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz) < 0) {
        printf("SPI_IOC_WR_MAX_SPEED_HZ failed: %s\n", strerror(errno));
        return false;
    }

    if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed_hz) < 0) {
        printf("SPI_IOC_RD_MAX_SPEED_HZ failed: %s\n", strerror(errno));
        return false;
    }

    return true;
}

bool l9966_linux_spidev_open(
    l9966_linux_spidev_t *transport,
    const l9966_linux_spidev_config_t *config
)
{
    if (transport == 0 || config == 0 || config->device_path == 0) {
        return false;
    }

    transport->fd = -1;
    transport->opened = false;

    transport->speed_hz =
        (config->speed_hz != 0u)
        ? config->speed_hz
        : L9966_LINUX_SPIDEV_DEFAULT_SPEED_HZ;

    transport->bits_per_word =
        (config->bits_per_word != 0u)
        ? config->bits_per_word
        : L9966_LINUX_SPIDEV_DEFAULT_BITS_PER_WORD;

    transport->delay_usecs = config->delay_usecs;

    int fd = open(config->device_path, O_RDWR);

    if (fd < 0) {
        printf("Failed to open SPI device %s: %s\n",
               config->device_path,
               strerror(errno));
        return false;
    }

    if (!configure_spi_mode(fd)) {
        close(fd);
        return false;
    }

    if (!configure_spi_bits_per_word(fd, transport->bits_per_word)) {
        close(fd);
        return false;
    }

    if (!configure_spi_speed(fd, transport->speed_hz)) {
        close(fd);
        return false;
    }

    transport->fd = fd;
    transport->opened = true;

    printf("Opened L9966 Linux spidev transport: %s\n", config->device_path);
    printf("SPI mode=%u bits=%u speed=%u Hz delay=%u us\n",
           (unsigned int)SPI_MODE_1,
           (unsigned int)transport->bits_per_word,
           (unsigned int)transport->speed_hz,
           (unsigned int)transport->delay_usecs);

    return true;
}

void l9966_linux_spidev_close(
    l9966_linux_spidev_t *transport
)
{
    if (transport == 0) {
        return;
    }

    if (transport->opened && transport->fd >= 0) {
        close(transport->fd);
    }

    transport->fd = -1;
    transport->opened = false;
}

bool l9966_linux_spidev_transfer(
    const uint8_t *tx,
    uint8_t *rx,
    size_t len,
    void *user
)
{
    l9966_linux_spidev_t *transport = (l9966_linux_spidev_t *)user;

    if (transport == 0 || tx == 0 || rx == 0) {
        return false;
    }

    if (!transport->opened || transport->fd < 0) {
        printf("l9966_linux_spidev_transfer failed: SPI device is not open\n");
        return false;
    }

    if (len == 0u) {
        return false;
    }

    /*
     * The L9966 register protocol uses 32-bit SPI frames.
     *
     * The core driver currently calls this with len = 4.
     *
     * This transport is written generically enough to transfer any byte count,
     * but normal L9966 register reads/writes should be 4 bytes.
     */
    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));

    tr.tx_buf = (uintptr_t)tx;
    tr.rx_buf = (uintptr_t)rx;
    tr.len = (uint32_t)len;
    tr.speed_hz = transport->speed_hz;
    tr.bits_per_word = transport->bits_per_word;
    tr.delay_usecs = transport->delay_usecs;

    int ret = ioctl(
        transport->fd,
        SPI_IOC_MESSAGE(1),
        &tr
    );

    if (ret < 1) {
        printf("SPI_IOC_MESSAGE failed: %s\n", strerror(errno));
        return false;
    }

    return true;
}

bool l9966_linux_spidev_is_open(
    const l9966_linux_spidev_t *transport
)
{
    if (transport == 0) {
        return false;
    }

    return transport->opened && transport->fd >= 0;
}