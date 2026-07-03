/* rcc_clock.c
 *
 * Purpose :
 * - Handles the PLL Multiplication to reach the External clock's maximum clock
 * of 180 Mhz from 8 Mhz.
 * - Safety Switching of the clocks.
 *
 * Functions:
 * -
 */

// Include the Header File
#include "rcc_clock.h"

/* ==============================================================================
 * RCC_BASE: Reset and Clock Control Base Address
 * Full Form: Reset and Clock Control
 * Purpose  : The starting memory address (0x40023800) for the entire RCC
 * hardware block on the Advanced High-performance Bus 1 (AHB1). All other RCC
 * registers are calculated as offsets from this base.
 * ==============================================================================
 */
#define RCC_BASE 0x40023800

/* ==============================================================================
 * RCC_CR: Clock Control Register (Offset 0x00)
 * Full Form: Reset and Clock Control - Clock Control Register
 * Purpose  : The main power switches and status flags for the raw clock
 * sources. Used to turn ON/OFF the internal 16MHz clock (HSI), the external
 * 8MHz crystal (HSE), and the main PLL engine. Also contains the
 * "Ready" flags to check if these hardware clocks have stabilized.
 * ==============================================================================
 */
#define RCC_CR (*(volatile unsigned int *)(RCC_BASE + 0x00))

/* ==============================================================================
 * RCC_PLLCFGR: PLL Configuration Register (Offset 0x04)
 * Full Form: Reset and Clock Control - Phase-Locked Loop Configuration Register
 * Purpose  : The math panel for the hardware multiplier. This register holds
 * the M, N, P, and Q divider/multiplier values used to transform the base clock
 * (e.g., 8MHz) up to maximum speed (e.g., 180MHz). Must be configured BEFORE
 * turning the PLL engine on in the RCC_CR.
 * ==============================================================================
 */
#define RCC_PLLCFGR (*(volatile unsigned int *)(RCC_BASE + 0x04))

/* ==============================================================================
 * RCC_CFGR: Clock Configuration Register (Offset 0x08)
 * Full Form: Reset and Clock Control - Clock Configuration Register
 * Purpose  : The master railway switch. Once the clocks are powered on and
 * stable, this register routes them. It selects which clock source actually
 * drives the CPU core (System Clock Switch) and sets the prescalers
 * (dividers) for the internal AHB, APB1, and APB2 peripheral buses.
 * ==============================================================================
 */
#define RCC_CFGR (*(volatile unsigned int *)(RCC_BASE + 0x08))

/* ==============================================================================
 * FLASH_BASE: Flash Interface Base Address
 * Full Form: Flash Memory Interface
 * Purpose  : The starting memory address (0x40023C00) for the hardware
 * controller that manages reading, writing, and erasing the permanent flash
 * memory.
 * ==============================================================================
 */
#define FLASH_BASE 0x40023C00

/* ==============================================================================
 * FLASH_ACR: Flash Access Control Register (Offset 0x00)
 * Full Form: Flash - Access Control Register
 * Purpose  : Controls how the CPU interacts with the flash memory. Its primary
 * job in clock configuration is setting "Wait States" (Latency). If
 * the CPU runs at 180MHz, the flash needs artificial delays (Wait States)
 * added so the CPU doesn't read data before the flash is ready to send it.
 * ==============================================================================
 */
#define FLASH_ACR (*(volatile unsigned int *)(FLASH_BASE + 0x00))

void clock_setup(void) {

  // Bit 16 turns on Crystal
  RCC_CR |= (1 << 16);
  while ((RCC_CR & (1 << 17)) == 0) {
    // Wait for Stability
  }

  // Bits 3:0 (LATENCY) control the wait states. We set it to 5.
  FLASH_ACR = (5 << 0);

  // Wiped the Registry Clean
  RCC_PLLCFGR = 0;

  // PLL Multiplication Operation
  // Bit 22: Set PLL Source to HSE (1)
  // Bits 5:0 (PLLM): Divide by 8
  // Bits 14:6 (PLLN): Multiply by 360
  // Bits 17:16 (PLLP): 00 means divide by 2
  RCC_PLLCFGR = (1 << 22) | (0 << 16) | (360 << 6) | (8 << 0);

  // Turning on the PLL
  RCC_CR |= (1 << 24);

  // Bit 25 == 1  Indicates stable upsampled frequency
  while ((RCC_CR & (1 << 25)) == 0) {
    // Wait untill Stablized
  }

  //  Binary 10 (Decimal 2) selects the PLL.
  RCC_CFGR |= (2 << 0);

  while ((RCC_CFGR & (3 << 2)) != (2 << 2)) {
    // wait until it shifted
  }
}
