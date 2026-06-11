#include "board_spi.h"

#include <string.h>

#include "fsl_lpspi.h"

#include "board_pins.h"

void BOARD_SPI_Init(void)
{
    lpspi_master_config_t config;

    LPSPI_MasterGetDefaultConfig(&config);
    config.baudRate = BOARD_L9966_SPI_BAUDRATE_HZ;
    config.whichPcs = BOARD_L9966_SPI_PCS;
    config.cpol = kLPSPI_ClockPolarityActiveHigh;
    config.cpha = kLPSPI_ClockPhaseSecondEdge;
    config.bitsPerFrame = 8U;
    config.direction = kLPSPI_MsbFirst;

    LPSPI_MasterInit(BOARD_L9966_SPI_BASE, &config, BOARD_L9966_SPI_SRC_CLOCK_HZ);
}

bool BOARD_SPI_L9966_Transfer(const uint8_t *tx, uint8_t *rx, size_t length)
{
    lpspi_transfer_t transfer;

    if ((tx == NULL) || (length == 0U))
    {
        return false;
    }

    memset(&transfer, 0, sizeof(transfer));
    transfer.txData = tx;
    transfer.rxData = rx;
    transfer.dataSize = length;
    transfer.configFlags = (uint32_t)BOARD_L9966_SPI_PCS | (uint32_t)kLPSPI_MasterPcsContinuous;

    BOARD_L9966_SetCs(false);
    const status_t status = LPSPI_MasterTransferBlocking(BOARD_L9966_SPI_BASE, &transfer);
    BOARD_L9966_SetCs(true);

    return (status == kStatus_Success);
}