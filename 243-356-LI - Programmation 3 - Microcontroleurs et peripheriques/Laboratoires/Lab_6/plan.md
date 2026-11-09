STM32F103	PCA9685	Fonction
PB6	SCL	Horloge I²C
PB7	SDA	Données I²C
3.3V	VCC	Alimentation logique
GND	GND	Masse commune

Analyseur logique	À connecter sur
CH0	PB6 (SCL)
CH1	PB7 (SDA)
GND	GND commun

Analyseur logique	Connexion	Mesure
CH0	PB6 / SCL	Horloge I²C
CH1	PB7 / SDA	Données I²C
CH2	PWM0 du PCA9685	Signal PWM
GND	GND commun	Référence électrique

SUR SALAE
branchement physique sur analyseur logique
CH1: SCL
CH2: SDA

faire une focntione servo angle