#include <ch32v00x.h>

#include "nectransmitter.h"

NECTransmitter necTransmitter;

void SetupInterrupts() {
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  EXTI_InitTypeDef EXTI_InitStructure = {0};
  NVIC_InitTypeDef NVIC_InitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD, ENABLE);

  // Configuration for GPIO Input on D3
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* GPIOD ----> EXTI_Line0 */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource3);
  EXTI_InitStructure.EXTI_Line = EXTI_Line3;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void Setup() {
  necTransmitter.Init(GPIOD, RCC_APB2Periph_GPIOD, GPIO_Pin_0);
  SetupInterrupts();
}

int main() {
  Setup();
  while (true) {
  }
}
