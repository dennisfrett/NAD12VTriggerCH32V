#include "debug.h"
#include <ch32v00x.h>

#include "nectransmitter.h"

// #define DEBUG_MODE

volatile bool overridden = false;

NECTransmitter necTransmitter;

extern "C" void EXTI7_0_IRQHandler(void)
    __attribute__((interrupt("WCH-Interrupt-fast")));

extern "C" void EXTI7_0_IRQHandler(void) {

  // Override button interrupt.
  if (EXTI_GetITStatus(EXTI_Line1) != RESET) {
    EXTI_ClearITPendingBit(EXTI_Line1); // Clear flag

    overridden = true;
  }

  // 12V trigger interrupt.
  if (EXTI_GetITStatus(EXTI_Line2) != RESET) {
    EXTI_ClearITPendingBit(EXTI_Line2); // Clear flag
  }
}

// Configure all pins on peripheral A and D to input pulldown to save maximum
// power.
void SetupOtherPins() {
  GPIO_InitTypeDef GPIO_InitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD, ENABLE);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;

  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}

// Initialize pins C1 and C2 as external interrupt.
void InitializeInterrupt() {
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  EXTI_InitTypeDef EXTI_InitStructure = {0};
  NVIC_InitTypeDef NVIC_InitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC, ENABLE);

  // Configure all pins as input pull down.
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  // Override button.
  // GPIOC1 ----> EXTI_Line1
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource1);
  EXTI_InitStructure.EXTI_Line = EXTI_Line1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  // 12V trigger pin.
  // GPIOC2 ----> EXTI_Line2
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);
  EXTI_InitStructure.EXTI_Line = EXTI_Line2;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void Setup() {
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  SystemCoreClockUpdate();

  Delay_Init();

#ifdef DEBUG_MODE
  USART_Printf_Init(115200);
  printf("Starting up, waiting 5s...\r\n");
#endif

  // Wait 5s after first startup to allow for easy flashing.
  Delay_Ms(5000);

  // TODO: order of these 2 seems to matter. Why?
  InitializeInterrupt();
  SetupOtherPins();

  necTransmitter.Init(GPIOD, RCC_APB2Periph_GPIOD, GPIO_Pin_2);
}

// Goes into standby and returns when interrupt is triggered.
void Standby() {
#ifdef DEBUG_MODE
  printf("Going into standby\r\n");
#endif

  // TODO: Not sure yet which calls are needed for standby.
  RCC_LSICmd(ENABLE);
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
    ;

  PWR_AWU_SetPrescaler(PWR_AWU_Prescaler_10240);
  PWR_EnterSTANDBYMode(PWR_STANDBYEntry_WFE);

#ifdef DEBUG_MODE
  USART_Printf_Init(115200);
  printf("Woken up\r\n");
#endif
}

/// Returns whether the trigger pin has the same value 300ms apart.
bool DebounceValue() {

  const auto val1 = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2);

  Delay_Ms(300);

  const auto val2 = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2);

  return val1 == val2;
}

int main(void) {
  Setup();

  while (true) {
    // Reset overriden.
    overridden = false;

    Standby();

    if (overridden || DebounceValue()) {

      // Toggle NAD.
      necTransmitter.SendExtendedNEC(0x877C, 0x80);

      // Wait a bit for stuff to settle down.
      Delay_Ms(500);
    }
  }
}
