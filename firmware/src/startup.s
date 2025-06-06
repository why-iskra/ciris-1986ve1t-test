    .syntax	unified
    .arch	armv6-m

/* stack */

    .section .stack
    .align	3
    .equ	stack_size, 0x1000
    .globl	__stack_top__
    .globl	__stack_limit__
__stack_limit__:
    .space	stack_size
    .size	__stack_limit__, . - __stack_limit__
__stack_top__:

/* heap */

    .section .heap
    .align	3
    .equ	heap_size, 0x2000
    .globl	__heap_start__
    .globl	__heap_end__
__heap_start__:
    .if		heap_size
    .space	heap_size
    .endif
    .size	__heap_start__, . - __heap_start__
__heap_end__:

/* isr vector */

    .section .isr_vector
    .align 2
    .long  __stack_top__          /* Top of Stack */
    .long  __isr_reset         	  /* Reset Handler */
    .long  __isr_nmi           	  /* NMI Handler */
    .long  __isr_hardfault     	  /* Hard Fault Handler */
    .long  0                      /* Reserved */
    .long  0                      /* Reserved */
    .long  0                      /* Reserved */
    .long  0                      /* Reserved */
    .long  0                      /* Reserved */
    .long  0                      /* Reserved */
    .long  0                      /* Reserved */
    .long  __isr_svc  		      /* SVCall Handler */
    .long  0                      /* Reserved */
    .long  0                      /* Reserved */
    .long  __isr_pendsv   		  /* PendSV Handler */
    .long  __isr_systick 		  /* SysTick Handler */
    /* external */
    .long  __isr_mil_std_1553b2   /* IRQ0 */
    .long  __isr_mil_std_1553b1   /* IRQ1 */
    .long  __isr_usb              /* IRQ2 */
    .long  __isr_can1             /* IRQ3 */
    .long  __isr_can2             /* IRQ4 */
    .long  __isr_dma              /* IRQ5 */
    .long  __isr_uart1            /* IRQ6 */
    .long  __isr_uart2            /* IRQ7 */
    .long  __isr_ssp1             /* IRQ8 */
    .long  __isr_busy             /* IRQ9 */
    .long  __isr_arinc429r        /* IRQ10 */
    .long  __isr_power            /* IRQ11 */
    .long  __isr_wwdg             /* IRQ12 */
    .long  __isr_timer4           /* IRQ13 */
    .long  __isr_timer1           /* IRQ14 */
    .long  __isr_timer2           /* IRQ15 */
    .long  __isr_timer3           /* IRQ16 */
    .long  __isr_adc              /* IRQ17 */
    .long  __isr_ethernet         /* IRQ18 */
    .long  __isr_ssp3             /* IRQ19 */
    .long  __isr_ssp2             /* IRQ20 */
    .long  __isr_arinc429t1       /* IRQ21 */
    .long  __isr_arinc429t2       /* IRQ22 */
    .long  __isr_arinc429t3       /* IRQ23 */
    .long  __isr_arinc429t4       /* IRQ24 */
    .long  0                      /* IRQ25 reserved */
    .long  0                      /* IRQ26 reserved */
    .long  __isr_bkp              /* IRQ27 */
    .long  __isr_ext_int1         /* IRQ28 */
    .long  __isr_ext_int2         /* IRQ29 */
    .long  __isr_ext_int3         /* IRQ30 */
    .long  __isr_ext_int4         /* IRQ31 */
    
/* __isr_reset_handler */

    .text
    .thumb
    .align	1
    .thumb_func
    .globl	__isr_reset
    .type	__isr_reset, %function
__isr_reset:
    cpsid i
    bl __start
    .size	__isr_reset, . - __isr_reset

/* __isr_default_handler */

    .text
    .thumb
    .align	1
    .thumb_func
    .weak	__isr_default
    .type	__isr_default, %function
__isr_default:
    bx lr
    .size	__isr_default, . - __isr_default

/* define others handlers */

    .macro	def_irq_handler	handler_name
    .weak	\handler_name
    .set	\handler_name, __isr_default
    .endm

    def_irq_handler __isr_nmi
    def_irq_handler __isr_hardfault
    def_irq_handler __isr_svc
    def_irq_handler __isr_pendsv
    def_irq_handler __isr_systick

    def_irq_handler __isr_mil_std_1553b2
    def_irq_handler __isr_mil_std_1553b1
    def_irq_handler __isr_usb
    def_irq_handler __isr_can1
    def_irq_handler __isr_can2
    def_irq_handler __isr_dma
    def_irq_handler __isr_uart1
    def_irq_handler __isr_uart2
    def_irq_handler __isr_ssp1
    def_irq_handler __isr_busy
    def_irq_handler __isr_arinc429r
    def_irq_handler __isr_power
    def_irq_handler __isr_wwdg
    def_irq_handler __isr_timer4
    def_irq_handler __isr_timer1
    def_irq_handler __isr_timer2
    def_irq_handler __isr_timer3
    def_irq_handler __isr_adc
    def_irq_handler __isr_ethernet
    def_irq_handler __isr_ssp3
    def_irq_handler __isr_ssp2
    def_irq_handler __isr_arinc429t1
    def_irq_handler __isr_arinc429t2
    def_irq_handler __isr_arinc429t3
    def_irq_handler __isr_arinc429t4
    def_irq_handler __isr_bkp
    def_irq_handler __isr_ext_int1
    def_irq_handler __isr_ext_int2
    def_irq_handler __isr_ext_int3
    def_irq_handler __isr_ext_int4
