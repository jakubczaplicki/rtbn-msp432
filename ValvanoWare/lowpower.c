// low power example, volume 2
#define LPMR__LPM35    (0x000000a0)          /* LPM3.5. Core voltage setting 0. */
#define LPMR__LPM45    (0x000000c0)          /* LPM4.5 */
int main(void){
  SysTick_Init(30000);   // set up SysTick for 100 Hz interrupts
  PCMCTL0 &= ~0xF0;      // clear any pending low-power requests
  SCB_SCR |= 0x00000004; // allow deep sleep
  PCMCTL0 = PCMKEY_M | LPMR__LPM35;  // request 3.5 on WFI
  EnableInterrupts();
  while(1){
    WaitForInterrupt();  // go to sleep
  }
}
