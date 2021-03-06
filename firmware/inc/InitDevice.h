//=========================================================
// inc/InitDevice.h: generated by Hardware Configurator
//
// This file will be regenerated when saving a document.
// leave the sections inside the "$[...]" comment tags alone
// or they will be overwritten!
//=========================================================
#ifndef __INIT_DEVICE_H__
#define __INIT_DEVICE_H__

// USER CONSTANTS
// USER PROTOTYPES

// $[Mode Transition Prototypes]
extern void enter_DefaultMode_from_RESET(void);
// [Mode Transition Prototypes]$

// $[Config(Per-Module Mode)Transition Prototypes]
extern void EMU_enter_DefaultMode_from_RESET(void);
extern void LFXO_enter_DefaultMode_from_RESET(void);
extern void CMU_enter_DefaultMode_from_RESET(void);
extern void ADC0_enter_DefaultMode_from_RESET(void);
extern void ACMP0_enter_DefaultMode_from_RESET(void);
extern void ACMP1_enter_DefaultMode_from_RESET(void);
extern void IDAC0_enter_DefaultMode_from_RESET(void);
extern void RTCC_enter_DefaultMode_from_RESET(void);
extern void USART0_enter_DefaultMode_from_RESET(void);
extern void USART1_enter_DefaultMode_from_RESET(void);
extern void LEUART0_enter_DefaultMode_from_RESET(void);
extern void WDOG0_enter_DefaultMode_from_RESET(void);
extern void I2C0_enter_DefaultMode_from_RESET(void);
extern void GPCRC_enter_DefaultMode_from_RESET(void);
extern void LDMA_enter_DefaultMode_from_RESET(void);
extern void TIMER0_enter_DefaultMode_from_RESET(void);
extern void TIMER1_enter_DefaultMode_from_RESET(void);
extern void LETIMER0_enter_DefaultMode_from_RESET(void);
extern void CRYOTIMER_enter_DefaultMode_from_RESET(void);
extern void PCNT0_enter_DefaultMode_from_RESET(void);
extern void PRS_enter_DefaultMode_from_RESET(void);
extern void PORTIO_enter_DefaultMode_from_RESET(void);
// [Config(Per-Module Mode)Transition Prototypes]$

// $[User-defined pin name abstraction]

#if KAPULA_eka
	#define ENC1_PIN            (11)
	#define ENC1_PORT           (gpioPortD)
	#define ENC1_TIM1_CC0       TIMER_ROUTELOC0_CC0LOC_LOC19

	#define ENC2_PIN            (12)
	#define ENC2_PORT           (gpioPortD)
	#define ENC2_TIM1_CC1       TIMER_ROUTELOC0_CC1LOC_LOC19

	#define ENCP_PIN            (13)
	#define ENCP_PORT           (gpioPortD)

	#define PTT_PIN             (6)
	#define PTT_PORT            (gpioPortF)

	#define PWM_PIN             (10)
	#define PWM_PORT            (gpioPortD)
	#define PWM_TIM0_CC0        TIMER_ROUTELOC0_CC0LOC_LOC18

	#define MIC_APORT           adcPosSelAPORT4XCH7 // PD15

#elif KAPULA_v2
	#define ENC1_PIN            (5)
	#define ENC1_PORT           (gpioPortF)
	#define ENC1_TIM1_CC0       TIMER_ROUTELOC0_CC0LOC_LOC29

	#define ENC2_PIN            (6)
	#define ENC2_PORT           (gpioPortF)
	#define ENC2_TIM1_CC1       TIMER_ROUTELOC0_CC1LOC_LOC29

	#define ENCP_PIN            (7)
	#define ENCP_PORT           (gpioPortF)

	#define PTT_PIN             (14)
	#define PTT_PORT            (gpioPortD)

	#define PWM_PIN             (14)
	#define PWM_PORT            (gpioPortB)
	#define PWM_TIM0_CC0        TIMER_ROUTELOC0_CC0LOC_LOC9

	#define MIC_APORT           adcPosSelAPORT3XCH10 // PA2

	#define MIC_EN_PIN          (3)
	#define MIC_EN_PORT         (gpioPortA)
#else
	#error "Unsupported or missing KAPULA_model"
#endif



#define TFT_CS_PIN          (7)
#define TFT_CS_PORT         (gpioPortC)

#define TFT_DC_PIN          (9)
#define TFT_DC_PORT         (gpioPortC)

#define TFT_LED_PIN         (10)
#define TFT_LED_PORT        (gpioPortC)

#define TFT_EN_PIN          (11)
#define TFT_EN_PORT         (gpioPortC)

#define RX_EN_PIN           (15)
#define RX_EN_PORT          (gpioPortD)

#define TX_EN_PIN           (13)
#define TX_EN_PORT          (gpioPortD)

// [User-defined pin name abstraction]$

#endif

