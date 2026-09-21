#ifndef TIMER2_H
#define TIMER2_H

#define TIMER2_MODE_STOP      0u
#define TIMER2_MODE_PWM       1u
#define TIMER2_MODE_INTERRUPT 2u

#ifndef PWM_DUTY_PERCENT
#define PWM_DUTY_PERCENT 50u // de 0 à 100
#endif
#if PWM_DUTY_PERCENT > 100u
#error "Le duty cycle doit etre compris entre 0 et 100"
#endif

void Timer2_Stop(void);
void Timer2_PWMInit(void); // PWM logiciel 100 Hz
void Timer2_InterruptInit(void); // Update toutes les 500 ms

#endif
