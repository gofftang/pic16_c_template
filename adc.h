#ifndef ADC_H
#define	ADC_H

#ifdef	__cplusplus
extern "C" {
#endif

void adc_init(unsigned char chn);
void adc_isr(void);
unsigned int adc_get_value(unsigned char chn);

#ifdef	__cplusplus
}
#endif

#endif	/* ADC_H */

