Sur le STM32F103, les périphériques n'ont pas tous leur horloge activée au démarrage.

TIM2 dépend du registre RCC_APB1ENR. Il faut activer son bit 0 avant de configurer ou d'utiliser le timer.
RCC_APB1ENR |= (1U << 0);


Sans cette activation, le programme peut compiler correctement, mais TIM2 ne fonctionne pas.

À retenir pour l'examen :
- RCC_APB1ENR : TIM2, I2C1, USART2.
- RCC_APB2ENR : GPIO, ADC1, USART1, AFIO.

L'instruction suivante met uniquement le bit 5 à 0, sans modifier les autres bits :
GPIOA_ODR &= ~(1U << 5);


Voici les quatre opérations essentielles à connaître :
Opération	Instruction	Effet
OU (\|=)	REG \|= (1U << 5);	Met le bit 5 à 1
ET (&=)	REG &= ~(1U << 5);	Met le bit 5 à 0
XOR (^=)	REG ^= (1U << 5);	Inverse le bit 5
Test (&)	REG & (1U << 5)	Vérifie si le bit 5 vaut 1

Piège d'examen : REG = ~(1U << 5) écrase tous les bits du registre. Il faut utiliser &= pour préserver les autres bits.

La formule pour calculer la fréquence de débordement d'un timer est :
\[
f_{\text{timer}}=\frac{f_{\text{horloge}}}{(PSC+1)(ARR+1)}
\]

Avec les valeurs de la question :
- Horloge : 8 000 000 Hz
- PSC : 7999
- ARR : 999
Étape 1 — Calculer la fréquence après le prescaler
\[
\frac{8\,000\,000}{7999+1}=1000\text{ Hz}
\]

Étape 2 — Calculer la fréquence de débordement
\[
\frac{1000}{999+1}=\boxed{1\text{ Hz}}
\]

Le timer déborde donc une fois par seconde.

Piège d'examen : Il faut toujours ajouter +1 au PSC et à l'ARR.
ATTENTION IL FAUT ASBOLUMENT 