#include STM32_HAL_H

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "irq.h"
#include "spi.h"
#include "timer.h"

SPI_HandleTypeDef spi;
static DMA_HandleTypeDef hdma_tx;
static DMA_HandleTypeDef hdma_rx;

static uint8_t *recv_buf = (uint8_t *)0x30040000;
static uint8_t *send_buf = (uint8_t *)0x30040040;
static int32_t volatile spi_rx_event = 0;
static int32_t volatile spi_tx_event = 0;
static int32_t volatile spi_abort_event = 0;
ChannelType host_channel = CHANNEL_NULL;

uint8_t spi_data_in[SPI_BUF_MAX_IN_LEN];
uint8_t spi_data_out[SPI_BUF_MAX_OUT_LEN];

trans_fifo spi_fifo_in = {.p_buf = spi_data_in,
                          .buf_size = SPI_BUF_MAX_IN_LEN,
                          .over_pre = false,
                          .read_pos = 0,
                          .write_pos = 0,
                          .lock_pos = 0};

secbool spi_can_write(void) {
  if (spi_tx_event == 0)
    return sectrue;
  else
    return secfalse;
}

int32_t wait_spi_rx_event(int32_t timeout) {
  int32_t tickstart = HAL_GetTick();

  while (spi_rx_event == 1) {
    if ((HAL_GetTick() - tickstart) > timeout) {
      return -1;
    }
  }
  return 0;
}

int32_t wait_spi_tx_event(int32_t timeout) {
  int32_t tickstart = HAL_GetTick();

  while (spi_tx_event == 1) {
    if ((HAL_GetTick() - tickstart) > timeout) {
      return -1;
    }
  }
  return 0;
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
  SET_RX_BUS_BUSY();
  if (spi_rx_event) {
    spi_rx_event = 0;
  }
  if (!fifo_write_no_overflow(&spi_fifo_in, recv_buf, hspi->RxXferSize)) {
    memset(recv_buf, 0, SPI_PKG_SIZE);
  }

  HAL_SPI_Receive_DMA(&spi, recv_buf, SPI_PKG_SIZE);
  SET_RX_BUS_IDEL();
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (spi_tx_event) {
    spi_tx_event = 0;
  }

  HAL_SPI_Receive_DMA(&spi, recv_buf, SPI_PKG_SIZE);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
  ensure(secfalse, "spi ovr err");
}

void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi) { spi_abort_event = 0; }

void SPI2_IRQHandler(void) { HAL_SPI_IRQHandler(&spi); }

int32_t spi_slave_init() {
  GPIO_InitTypeDef gpio;

  __HAL_RCC_DMA1_FORCE_RESET();
  __HAL_RCC_DMA1_RELEASE_RESET();

  __HAL_RCC_SPI2_FORCE_RESET();
  __HAL_RCC_SPI2_RELEASE_RESET();

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_SPI2_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;

  gpio.Pin = GPIO_PIN_5 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOK, &gpio);
  HAL_GPIO_WritePin(GPIOK, GPIO_PIN_5, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOK, GPIO_PIN_6, GPIO_PIN_SET);

  if (PCB_VERSION_1_0_0 == pcb_version) {
    // SPI2: PB12(NSS),PB13(SCK)
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF5_SPI2;
    gpio.Pin = GPIO_PIN_12 | GPIO_PIN_13;
    HAL_GPIO_Init(GPIOB, &gpio);
  } else {
    // SPI2: PA11(NSS),PA9(SCK)
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF5_SPI2;
    gpio.Pin = GPIO_PIN_9 | GPIO_PIN_11;
    HAL_GPIO_Init(GPIOA, &gpio);
  }

  // SPI2: PC2(MISO), PC3(MOSI)
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio.Alternate = GPIO_AF5_SPI2;
  gpio.Pin = GPIO_PIN_2 | GPIO_PIN_3;
  HAL_GPIO_Init(GPIOC, &gpio);

  spi.Instance = SPI2;
  spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  spi.Init.Direction = SPI_DIRECTION_2LINES;
  spi.Init.CLKPhase = SPI_PHASE_1EDGE;
  spi.Init.CLKPolarity = SPI_POLARITY_LOW;
  spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  spi.Init.CRCPolynomial = 7;
  spi.Init.DataSize = SPI_DATASIZE_8BIT;
  spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
  spi.Init.NSS = SPI_NSS_HARD_INPUT;
  spi.Init.TIMode = SPI_TIMODE_DISABLE;
  spi.Init.Mode = SPI_MODE_SLAVE;
  spi.Init.FifoThreshold = SPI_FIFO_THRESHOLD_16DATA;

  if (HAL_OK != HAL_SPI_Init(&spi)) {
    return -1;
  }

  /*##-3- Configure the DMA ##################################################*/
  /* Configure the DMA handler for Transmission process */
  hdma_tx.Instance = SPIx_TX_DMA_STREAM;
  hdma_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  hdma_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  hdma_tx.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;
  hdma_tx.Init.Request = SPIx_TX_DMA_REQUEST;
  hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_tx.Init.Mode = DMA_NORMAL;
  hdma_tx.Init.Priority = DMA_PRIORITY_LOW;

  HAL_DMA_Init(&hdma_tx);

  /* Associate the initialized DMA handle to the the SPI handle */
  __HAL_LINKDMA(&spi, hdmatx, hdma_tx);

  /* Configure the DMA handler for Transmission process */
  hdma_rx.Instance = SPIx_RX_DMA_STREAM;

  hdma_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  hdma_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  hdma_rx.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_rx.Init.PeriphBurst = DMA_PBURST_SINGLE;
  hdma_rx.Init.Request = SPIx_RX_DMA_REQUEST;
  hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_rx.Init.Mode = DMA_NORMAL;
  hdma_rx.Init.Priority = DMA_PRIORITY_HIGH;

  HAL_DMA_Init(&hdma_rx);

  /* Associate the initialized DMA handle to the the SPI handle */
  __HAL_LINKDMA(&spi, hdmarx, hdma_rx);

  /*##-4- Configure the NVIC for DMA #########################################*/
  /* NVIC configuration for DMA transfer complete interrupt (SPI1_TX) */
  NVIC_SetPriority(SPIx_DMA_TX_IRQn, IRQ_PRI_DMA);
  HAL_NVIC_EnableIRQ(SPIx_DMA_TX_IRQn);

  /* NVIC configuration for DMA transfer complete interrupt (SPI1_RX) */
  NVIC_SetPriority(SPIx_DMA_RX_IRQn, IRQ_PRI_DMA);
  HAL_NVIC_EnableIRQ(SPIx_DMA_RX_IRQn);

  NVIC_SetPriority(SPI2_IRQn, IRQ_PRI_SPI);
  HAL_NVIC_EnableIRQ(SPI2_IRQn);

  memset(recv_buf, 0, SPI_PKG_SIZE);
  spi_rx_event = 1;
  SET_RX_BUS_IDEL();
  /* start SPI receive */
  if (HAL_SPI_Receive_DMA(&spi, recv_buf, SPI_PKG_SIZE) != HAL_OK) {
    return -1;
  }

  return 0;
}

int32_t spi_slave_send(uint8_t *buf, uint32_t size, int32_t timeout) {
  uint32_t msg_size;
  int32_t ret = 0;
  msg_size = size < SPI_PKG_SIZE ? SPI_PKG_SIZE : size;
  memcpy(send_buf, buf, msg_size);

  spi_abort_event = 1;
  if (HAL_SPI_Abort_IT(&spi) != HAL_OK) {
    return 0;
  }
  while (spi_abort_event)
    ;

  if (HAL_SPI_Transmit_DMA(&spi, send_buf, SPI_PKG_SIZE) != HAL_OK) {
    goto END;
  }

  SET_COMBUS_LOW();

  spi_tx_event = 1;

  if (wait_spi_tx_event(timeout) != 0) {
    goto END;
  }
  ret = msg_size;
END:
  if (ret == 0) {
    HAL_SPI_Abort(&spi);
    HAL_SPI_Receive_DMA(&spi, recv_buf, SPI_PKG_SIZE);
  }
  SET_COMBUS_HIGH();

  return ret;
}

uint32_t spi_slave_poll(uint8_t *buf) {
  volatile uint32_t total_len, len, ret;

  if (buf == NULL) return 0;

  total_len = fifo_lockdata_len(&spi_fifo_in);
  if (total_len == 0) {
    return 0;
  }

  len = total_len > SPI_PKG_SIZE ? SPI_PKG_SIZE : total_len;
  ret = fifo_read_lock(&spi_fifo_in, buf, len);
  return ret;
}

uint32_t spi_read_retry(uint8_t *buf) {
  spi_rx_event = 1;

  for (int retry = 0;; retry++) {
    int r = wait_spi_rx_event(500);
    if (r == -1) {  // reading failed
      if (r == -1 && retry < 10) {
        // only timeout => let's try again
      } else {
        // error
        error_shutdown("Error reading", "from SPI.", "Try to", "reset.");
      }
    }

    if (r == 0) {
      return spi_slave_poll(buf);
    }
  }
}

void SPIx_DMA_RX_IRQHandler(void) { HAL_DMA_IRQHandler(spi.hdmarx); }

void SPIx_DMA_TX_IRQHandler(void) { HAL_DMA_IRQHandler(spi.hdmatx); }
