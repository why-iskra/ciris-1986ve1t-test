//
// Created by whyiskra on 2025-05-28.
//

#pragma once

// #define asm_disable_irq() __asm("cpsid i")
// #define asm_enable_irq()  __asm("cpsie i")
#define asm_disable_irq() 
#define asm_enable_irq()  
#define asm_nop()         __asm("nop")

