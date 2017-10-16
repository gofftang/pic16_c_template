/*
This is an example procedure for using the ADC to perform an Analog-to-Digital conversion:
    1. Configure Port:
        ? Disable pin output driver (Refer to the TRIS register)
        ? Configure pin as analog (Refer to the ANSEL register)
    2. Configure the ADC module:
        ? Select ADC conversion clock
        ? Configure voltage reference
        ? Select ADC input channel
        ? Turn on ADC module
    3. Configure ADC interrupt (optional):
        ? Clear ADC interrupt flag
        ? Enable ADC interrupt
        ? Enable peripheral interrupt
        ? Enable global interrupt(1)
    4. Wait the required acquisition time(2).
    5. Start conversion by setting the GO/DONE bit.
    6. Wait for ADC conversion to complete by one of the following:
        ? Polling the GO/DONE bit (polling way)
        ? Waiting for the ADC interrupt (interrupts enabled)
    7. Read ADC Result.
    8. Clear the ADC interrupt flag (required if interrupt is enabled).
 */

#include <xc.h>

//#define ADC_INTERRUPT
#define ADC_MAX         12
#define ADC_REF_VOLT    3.0f //REF_VDD

static unsigned int adc_value[ADC_MAX];

void adc_init(unsigned char chn)
{
    switch (chn) {
    case 0: //A0
        TRISA0 = 1; //Disable pin output driver
        ANSA0 = 1; //Configure pin as analog
        break;
    case 1: //A1
        TRISA1 = 1;
        ANSA1 = 1;
        break;
    case 2: //A2
        TRISA2 = 1;
        ANSA2 = 1;
        break;
    case 3: //A4
        TRISA4 = 1;
        ANSA4 = 1;
        break;
    case 4: //C0
        TRISC0 = 1;
        ANSC0 = 1;
        break;
    case 5: //C1
        TRISC1 = 1;
        ANSC1 = 1;
        break;
    case 6: //C2
        TRISC2 = 1;
        ANSC2 = 1;
        break;
    case 7: //C3
        TRISC3 = 1;
        ANSC3 = 1;
        break;
    case 8: //C6
        TRISC6 = 1;
        ANSC6 = 1;
        break;
    case 9: //C7
        TRISC7 = 1;
        ANSC7 = 1;
        break;
    case 10: //B4
        TRISB4 = 1;
        ANSB4 = 1;
        break;
    case 11: //B5
        TRISB5 = 1;
        ANSB5 = 1;
        break;
    default:
        break;
    }
    ADCON0bits.CHS = chn; //Select ADC input channel
    ADON = 1; //Turn on ADC module
#ifdef ADC_INTERRUPT
    ADIF = 0; //Clear ADC interrupt flag
    ADIE = 1; //Enable ADC interrupt
    PEIE = 1; //Enable peripheral interrupt
    GIE = 1; //Enable global interrupt(1)
#endif
}

void adc_isr(void)
{
#ifdef ADC_INTERRUPT
    if (ADCON0bits.CHS < ADC_MAX) {
        unsigned int reg_val = ADRESH << 8 | ADRESL;
        adc_value[ADCON0bits.CHS] = (unsigned int)(reg_val /1023 * ADC_REF_VOLT);
    }
#endif
}

#ifndef ADC_INTERRUPT
static void adc_polling(unsigned char chn)
{
    unsigned int reg_val = 0;

    ADGO = 1;
    while (ADGO) ;

    reg_val = ADRESH << 8 | ADRESL;
    adc_value[chn] = (unsigned int)(reg_val /1023 * ADC_REF_VOLT);
}
#endif

unsigned int adc_get_value(unsigned char chn)
{
    if (chn >= ADC_MAX) {
        return 0;
    }
    
#ifndef ADC_INTERRUPT
    adc_polling(chn);
#endif

    return adc_value[chn];
}
