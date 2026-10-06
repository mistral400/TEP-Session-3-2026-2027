# Laboratoire 2 — Interruption, Timeur et PWM

## Programmation sur STM32F103

**Nom :** Matisse Rhéaume-Viale  
**Cours :** 243-356-LI - Programmation 3 - Microcontroleurs et peripheriques  
**Enseignant :** Gabriel Fortin-Bélanger  
**Date :** Lundi 21 Septembre  
**Groupe :** 0001

# 0. Note sur l'Intelligence Artificielle

Le code, les explications, les résultats et les réponses aux questions n’ont pas été générés par l’IA. Si vous avez un doute sur une section, j’aimerais mieux que vous veniez me voir avant de tirer une conclusion.

Je pense qu’il est aussi important de rester prudent avant de conclure qu’un travail a été généré par l’IA simplement parce que son style « ressemble à de l’IA » ou parce qu’un détecteur l’indique. Ces outils ne permettent pas de déterminer avec certitude l’origine d’un texte et peuvent produire des faux positifs. Un doute devrait donc surtout servir à commencer une vérification, et non à constituer une preuve à lui seul. Dans ce genre de situation, je préfère qu’on me demande d’expliquer une section, de montrer mes fichiers de travail ou de démontrer ma compréhension du code avant qu’une conclusion soit tirée. Cette approche me semble aussi plus cohérente avec la PIEA, qui indique qu’un jugement d’évaluation doit être fondé, documenté, raisonné et interprété avec prudence.

Ce problème est d’ailleurs documenté. Une étude de Stanford a montré que plusieurs détecteurs avaient classé à tort **61,22 %** de textes rédigés par des étudiants non anglophones comme étant générés par une IA. OpenAI avait aussi développé son propre détecteur, mais l’a retiré en juillet 2023 en raison de son faible taux de précision; lors de ses tests, il ne détectait correctement qu’environ **26 %** des textes générés par IA et classait aussi certains textes humains comme étant générés par IA.

Je comprends donc qu’un professeur puisse avoir des doutes devant certains éléments d’un travail, mais je pense qu’un détecteur ou une impression sur le style devrait surtout servir à soulever une question, et non à constituer une preuve en soi.

L’IAG est encore relativement nouvelle et évolue extrêmement vite. Même pour quelqu’un qui travaille ou enseigne dans un domaine lié à l’informatique ou à la technologie, ou même pour un utilisateur régulier ou un *power user*, ça devient de plus en plus difficile de déterminer avec certitude ce qui a été fait ou non avec de l’IA.

Je comprends que certains éléments de mon document puissent paraître « trop propres » ou faire penser à de l’IA, par exemple les tableaux bien structurés, la mise en page ou les encadrés de code. Par contre, je travaille directement en Markdown et je sais moi-même comment faire la mise en forme de mes blocs de code, par exemple avec ` ```c `.

Pour être transparent sur ma façon de travailler :

* **Mise en page et fichiers :** j’utilise parfois l’IA pour m’aider avec la mise en forme du Markdown afin d’obtenir un PDF propre, par exemple pour les sauts de ligne, l’indentation, le formatage du code ou certains problèmes de mise en page. Je peux aussi l’utiliser pour m’aider à organiser mes fichiers ou à nommer automatiquement mes captures d’écran.
* **Correction du français :** j’utilise **Antidote 12** pour corriger mes fautes et surtout pour améliorer certaines phrases quand elles sont difficiles à comprendre.

Tout le contenu technique, les démarches, les réflexions, le code et les réponses viennent de moi.

Si quelque chose vous semble douteux, je suis disponible pour vous montrer mes fichiers de travail, mon historique de modifications ou simplement vous expliquer en personne n’importe quelle partie du code ou de ma démarche.


# 1. Objectif du laboratoire

Faire clignoter la LED D1-1 sur PB6 selon 3 modes différents. Le bouton S1-3 sur PB5 sert à changer de mode.

- Mode 0 : LED allumée 500 ms et éteinte 500 ms avec SysTick.
- Mode 1 : PWM logiciel avec TIM2. Le duty cycle est réglé dans le code.
- Mode 2 : la LED change d'état à chaque interruption Update de TIM2.
- Chaque appui fait : Mode 0 → Mode 1 → Mode 2 → Mode 0.

Le projet est bare metal. Les registres sont utilisés directement, sans HAL.

Les fonctions créées dans ce laboratoire sont :

- `SystemInit()`
- `main()`
- `LED_Init()`
- `LED_Set()`
- `LED_Toggle()`
- `Button_Init()`
- `Button_Read()`
- `SysTick_Init()`
- `SysTick_GetMs()`
- `SysTick_Handler()`
- `Timer2_Stop()`
- `Timer2_Start()`
- `Timer2_PWMInit()`
- `Timer2_InterruptInit()`
- `TIM2_IRQHandler()`

# 2. Plan de test

## 2.1 Tableau des tests

| Cas de test | Test | Étapes | Résultat attendu | Résultat obtenu |
|---|---|---|---|---|
| 1 | Mode 0 avec SysTick | Démarrer le programme et mesurer PB6. | 500 ms ON / 500 ms OFF, donc une période de 1 s. | Période moyenne de 0,999 s, avec environ 500 ms ON / 500 ms OFF. |
| 2 | Mode 1 avec PWM logiciel | Appuyer une fois sur S1-3 et mesurer PB6. | PWM de 100 Hz. À 69 %, environ 6,9 ms ON / 3,1 ms OFF. | Fréquence moyenne de 100,11 Hz et environ 31 % à l'état haut sur PB6. |
| 3 | Mode 2 avec interruption TIM2 | Appuyer une deuxième fois sur S1-3 et mesurer PB6. | Changement d'état chaque 500 ms, donc une période de 1 s. | Période moyenne de 0,999 s, avec un changement d'état environ chaque 500 ms. |
| 4 | Retour au mode 0 | Appuyer une troisième fois sur S1-3. | Retour au clignotement 500 ms ON / 500 ms OFF. | Retour au mode 0 avec une période moyenne de 0,999 s. |


## 2.2 Cas de test 1 — Mode 0 avec SysTick

**Résultat attendu :** 500 ms allumée et 500 ms éteinte.

**Résultat mesuré :** 
ΔT	3.9967278333333334	s
Nfalling	4	
Nrising	5	
fmax	1.0009188434983316	Hz
fmean	1.0008187114067104	Hz
Tstd	0.00007835616540771647	s
TposMean	0.4996544999999999	
TposMin	0.4995924166666664	
TposMax	0.4996994583333334	
TnegMean	0.49952745833333345	
TnegMin	0.4994895833333335	
TnegMax	0.49956358333333334	
Dutymean	0.500063572598151	
Dutymin	0.5000514639105363	
Dutymax	0.5000791704752382	
fmean	1.0008187160229005	
fmin	1.0007375018414613	
fmax	1.0009188434983316	
Tavg	0.9991819583333332	s


### Capture de l'analyseur logique

![Mode 0 Saleae](images/mode0.png)

**Observation :** la LED change d'état environ toutes les 500 ms. La période complète est proche de 1 s.

## 2.3 Cas de test 2 — PWM logiciel

**Résultat attendu :** PWM de 100 Hz. Avec `PWM_DUTY_PERCENT` à 69, mais puisque le Saleae affiche environ 31 % parce que la LED est active LOW, donc un duty cycle de 69 % dans le code veut dire que PB6 est à 0 pendant 69 % du temps et à 1 pendant environ 31 %.

**Résultat mesuré :**
ΔT	0.18288966666666617	s
Nfalling	19	
Nrising	19	
fmin	100.10427528676009	Hz
fmax	100.12849823941522	Hz
fmean	100.11492358937019	Hz
Tstd	7.340696504253055e-7	s
TposMean	0.0030964013157894635	
TposMin	0.003096041666666679	
TposMax	0.003096916666667312	
TnegMean	0.0068921134259259005	
TnegMin	0.006891083333332063	
TnegMax	0.0068930833333333795	
Dutymean	0.30999659111567485	
Dutymin	0.30994986276911324	
Dutymax	0.3100352891121492	
fmean	100.11492410006282	
fmin	100.10427528676009	
fmax	100.12849823941522	
Tavg	0.009988520833333344	s


### Capture de l'analyseur logique

![Mode 1 PWM Saleae](images/modePWM.png)

**Observation :** PWM avec un duty cycle sur Saleae de 31 %.

## 2.4 Cas de test 3 — Interruption TIM2

**Résultat attendu :** la LED change d'état toutes les 500 ms.

**Résultat mesuré :** 
ΔT	3.4956066249999997	s
Nfalling	4	
Nrising	4	
fmin	1.0012351069406695	Hz
fmax	1.0012444216605711	Hz
fmean	1.0012409686500188	Hz
Tstd	0.000005090557253580597	s
TposMean	0.499403208333334	
TposMin	0.49939666666666743	
TposMax	0.49941262500000083	
TnegMean	0.49934673611111063	
TnegMin	0.49932491666666645	
TnegMax	0.49936149999999907	
Dutymean	0.500028271552949	
Dutymin	0.5000232998620494	
Dutymax	0.5000359209219057	
fmean	1.0012516206208861	
fmin	1.0012273796139843	
fmax	1.0012800531078934	
Tavg	0.9987605694444446	s


### Capture de l'analyseur logique

![Mode 2 Saleae](images/mode2.png)

**Observation :** la LED change d'état environ toutes les 500 ms selon l'interruption.

## 2.5 Cas de test 4 — Retour au mode 0

**Résultat attendu :** après le troisième appui, le programme revient au mode 0.

**Résultat mesuré :** 
ΔT	3.9967278333333334	s
Nfalling	4	
Nrising	5	
fmax	1.0009188434983316	Hz
fmean	1.0008187114067104	Hz
Tstd	0.00007835616540771647	s
TposMean	0.4996544999999999	
TposMin	0.4995924166666664	
TposMax	0.4996994583333334	
TnegMean	0.49952745833333345	
TnegMin	0.4994895833333335	
TnegMax	0.49956358333333334	
Dutymean	0.500063572598151	
Dutymin	0.5000514639105363	
Dutymax	0.5000791704752382	
fmean	1.0008187160229005	
fmin	1.0007375018414613	
fmax	1.0009188434983316	
Tavg	0.9991819583333332	s

### Capture de l'analyseur logique

![Retour au mode 0 Saleae](images/mode0.png)

**Observation :** Retour au mode 0.

# 3. Questions d'analyse

Les sections suivantes utilisent le code actuel du projet.

# 3.1 Fonction `SystemInit()`

## Capture du code de la fonction

```c
void SystemInit(void)
{
    RCC_CR |= 1u; // active HSI

    while ((RCC_CR & (1u << 1u)) == 0u)
    {
    }

    RCC_CFGR &= ~3u; // prend HSI

    while ((RCC_CFGR & (3u << 2u)) != 0u)
    {
    }

    RCC_CFGR &= ~((0xFu << 4u) | (7u << 8u) | (7u << 11u)); // tout reste à 8 MHz
}
```

## Explications du code et des configurations de registres utilisées

Cette fonction démarre le HSI et le choisit comme horloge système. Le HSI fonctionne à 8 MHz.

```c
RCC_CR |= 1u;
```

Cette ligne met le bit HSION à 1. Elle active le HSI.

```c
while ((RCC_CR & (1u << 1u)) == 0u)
```

La boucle attend que le bit HSIRDY passe à 1.

```c
RCC_CFGR &= ~3u;
```

Cette ligne met les bits SW à `00`. Le HSI est donc choisi comme horloge système.

```c
RCC_CFGR &= ~((0xFu << 4u) | (7u << 8u) | (7u << 11u));
```

Les prescalers AHB, APB1 et APB2 restent à 1. Les bus restent donc à 8 MHz.

### Registres utilisés

| Registre | Adresse | Utilité |
|---|---|---|
| `RCC_CR` | `0x40021000` | Active le HSI et indique quand il est prêt. |
| `RCC_CFGR` | `0x40021004` | Choisit l'horloge système et les prescalers. |

## Justification avec la documentation officielle

![Registre RCC_CR](images/doc_RCC_CR.png)

![Registre RCC_CFGR](images/doc_RCC_CFGR.png)

**Explication de la documentation :**

On voit les bits HSION et HSIRDY dans `RCC_CR`. Dans `RCC_CFGR`, la valeur `00` pour SW choisit le HSI. Les prescalers à `0000` ou `000` ne divisent pas l'horloge.

# 3.2 Fonction `main()`

## Capture du code de la fonction

```c
int main(void)
{
    unsigned char mode = MODE_0;
    unsigned char bouton;
    unsigned char ancienBouton;
    unsigned char boutonStable;

    unsigned int tempsBouton = 0u;
    unsigned int tempsLED = 0u;
    unsigned int temps;

    LED_Init();
    Button_Init();
    Timer2_Stop();
    SysTick_Init();

    bouton = Button_Read();
    ancienBouton = bouton;
    boutonStable = bouton;

    LED_Set(LED_ON); // commence allumé

    while (1)
    {
        temps = SysTick_GetMs();
        bouton = Button_Read();

        if (bouton != ancienBouton)
        {
            ancienBouton = bouton;
            tempsBouton = temps; // repart le debounce
        }

        if ((bouton != boutonStable) &&
            ((unsigned int)(temps - tempsBouton) >= DEBOUNCE))
        {
            boutonStable = bouton;

            if (boutonStable != 0u)
            {
                mode++; // prochain mode

                if (mode > MODE_2)
                {
                    mode = MODE_0;
                }

                Timer2_Stop();
                LED_Set(LED_OFF);

                if (mode == MODE_0)
                {
                    LED_Set(LED_ON);
                    tempsLED = temps;
                }
                else if (mode == MODE_1)
                {
                    Timer2_PWMInit();
                }
                else
                {
                    Timer2_InterruptInit();
                }
            }
        }

        if (mode == MODE_0)
        {
            if ((unsigned int)(temps - tempsLED) >= TEMPS_LED)
            {
                tempsLED = temps;
                LED_Toggle(); // change au 500 ms
            }
        }

        __asm volatile ("dsb\n\twfi" ::: "memory"); // attend prochaine interruption
    }
}
```

## Explications du code et des configurations de registres utilisées

`main()` initialise la LED sur PB6, le bouton sur PB5, TIM2 et SysTick.

Le bouton doit rester stable pendant 20 ms avant que l'appui soit accepté. À chaque appui, `mode` augmente. Après le mode 2, il revient au mode 0.

- En mode 0, SysTick donne le temps en millisecondes. La LED change d'état après 500 ms.
- En mode 1, `Timer2_PWMInit()` démarre le PWM logiciel.
- En mode 2, `Timer2_InterruptInit()` fait une interruption TIM2 chaque 500 ms.

La soustraction `temps - tempsLED` permet de vérifier le temps passé sans bloquer le programme.

### Registres utilisés

Aucun

## Justification avec la documentation officielle

**Explication de la documentation :** 

Les registres GPIO, SysTick et TIM2 utilisés par les trois modes sont expliqués dans les sections autre sections du rapport et puisque ce rapport n'est pas généré par l'IA, je n'ai pas le temps de remmettre les explications.

# 3.3 Fonction `LED_Init()`

## Capture du code de la fonction

```c
void LED_Init(void)
{
    RCC_APB2ENR |= (1u << 3u); // active GPIOB

    LED_Set(LED_OFF); // commence éteinte

    GPIOB_CRL &= ~(0xFu << 24u); // efface PB6
    GPIOB_CRL |= (0x2u << 24u); // sortie push-pull 2 MHz
}
```

## Explications du code et des configurations de registres utilisées

Cette fonction configure PB6 comme sortie pour la LED D1-1.

```c
RCC_APB2ENR |= (1u << 3u);
```

Le bit IOPBEN active l'horloge du port GPIOB.

```c
GPIOB_CRL &= ~(0xFu << 24u);
GPIOB_CRL |= (0x2u << 24u);
```

PB6 utilise les bits 27 à 24 de `GPIOB_CRL`. La valeur `0010` configure une sortie push-pull à 2 MHz. Les bits de PB5 ne sont pas changés.

### Registres utilisés

| Registre | Adresse | Utilité |
|---|---|---|
| `RCC_APB2ENR` | `0x40021018` | Active l'horloge de GPIOB. |
| `GPIOB_CRL` | `0x40010C00` | Configure PB6 en sortie push-pull à 2 MHz. |

## Justification avec la documentation officielle

![Registre RCC_APB2ENR](images/doc_RCC_APB2ENR.png)

![Registre GPIOx_CRL](images/doc_GPIO_CRL.png)

**Explication de la documentation :**

On voit que les broches 0 à 7 sont configurées dans `GPIOx_CRL`. Pour PB6, MODE6 vaut `10` et CNF6 vaut `00`.

# 3.4 Fonction `LED_Set()`

## Capture du code de la fonction

```c
void LED_Set(unsigned char etat)
{
#if LED_ACTIVE_LOW == 1u
    if (etat == LED_ON)
    {
        GPIOB_BSRR = (LED_MASK << 16u); // PB6 à 0
    }
    else
    {
        GPIOB_BSRR = LED_MASK; // PB6 à 1
    }
#else
    if (etat == LED_ON)
    {
        GPIOB_BSRR = LED_MASK; // PB6 à 1
    }
    else
    {
        GPIOB_BSRR = (LED_MASK << 16u); // PB6 à 0
    }
#endif

    etatLED = etat;
}
```

## Explications du code et des configurations de registres utilisées

Cette fonction allume ou éteint la LED. Dans le projet, `LED_ACTIVE_LOW` vaut 1. La LED est donc allumée quand PB6 est à 0.

```c
GPIOB_BSRR = (LED_MASK << 16u);
```

Écrire dans la partie haute de `BSRR` remet PB6 à 0.

```c
GPIOB_BSRR = LED_MASK;
```

Écrire dans la partie basse met PB6 à 1. `etatLED` garde aussi l'état demandé pour `LED_Toggle()`.

### Registres utilisés

| Registre | Adresse | Utilité |
|---|---|---|
| `GPIOB_BSRR` | `0x40010C10` | Met PB6 à 1 ou à 0 sans changer les autres broches. |

## Justification avec la documentation officielle

![Registre GPIOx_BSRR](images/doc_GPIO_BSRR.png)

**Explication de la documentation :**

La partie BS6 met PB6 à 1. La partie BR6, dans les bits 16 à 31 du même registre, remet PB6 à 0.

# 3.5 Fonction `LED_Toggle()`

## Capture du code de la fonction

```c
void LED_Toggle(void)
{
    if (etatLED == LED_OFF)
    {
        LED_Set(LED_ON);
    }
    else
    {
        LED_Set(LED_OFF);
    }
}
```

## Explications du code et des configurations de registres utilisées

Cette fonction inverse l'état gardé dans `etatLED`. Elle appelle ensuite `LED_Set()` pour changer PB6.

### Registres utilisés

| Registre | Adresse | Utilité |
|---|---|---|
| Aucun directement | — | Le registre `GPIOB_BSRR` est utilisé dans `LED_Set()`. |

## Justification avec la documentation officielle

**Explication de la documentation :**

Cette fonction ne configure pas de registre. La commande de PB6 est expliquée dans la section `LED_Set()`.

# 3.6 Fonction `Button_Init()`

## Capture du code de la fonction

```c
void Button_Init(void)
{
    RCC_APB2ENR |= (1u << 3u); // active GPIOB

    GPIOB_CRL &= ~(0xFu << 20u); // efface PB5

#if BUTTON_INTERNAL_PULL == 0u
    GPIOB_CRL |= (0x4u << 20u); // entrée flottante
#else
    GPIOB_CRL |= (0x8u << 20u); // entrée pull-up/down

#if BUTTON_INTERNAL_PULL == 1u
    GPIOB_BSRR = BUTTON_MASK; // pull-up
#else
    GPIOB_BSRR = (BUTTON_MASK << 16u); // pull-down
#endif
#endif
}
```

## Explications du code et des configurations de registres utilisées

Cette fonction configure PB5 pour lire le bouton S1-3.

```c
GPIOB_CRL &= ~(0xFu << 20u);
GPIOB_CRL |= (0x4u << 20u);
```

PB5 utilise les bits 23 à 20. Dans le projet, `BUTTON_INTERNAL_PULL` vaut 0. La valeur `0100` configure donc PB5 en entrée flottante.

Le reste du code permet aussi de choisir un pull-up ou un pull-down interne si la constante est changée.

### Registres utilisés

| Registre | Adresse | Utilité |
|---|---|---|
| `RCC_APB2ENR` | `0x40021018` | Active l'horloge de GPIOB. |
| `GPIOB_CRL` | `0x40010C00` | Configure PB5 en entrée. |
| `GPIOB_BSRR` | `0x40010C10` | Choisit le pull-up ou le pull-down seulement si cette option est activée. |

## Justification avec la documentation officielle

![Registre RCC_APB2ENR](images/doc_RCC_APB2ENR.png)

![Registre GPIOx_CRL](images/doc_GPIO_CRL.png)

**Explication de la documentation :**

Dans `GPIOx_CRL`, MODE5 à `00` donne une entrée. CNF5 à `01` donne une entrée flottante. CNF5 à `10` sert pour une entrée avec pull-up ou pull-down.

# 3.7 Fonction `Button_Read()`

## Capture du code de la fonction

```c
unsigned char Button_Read(void)
{
    unsigned char bouton;

    bouton = (GPIOB_IDR & BUTTON_MASK) ? 1u : 0u; // lit PB5

#if BUTTON_ACTIVE_LOW == 1u
    bouton = !bouton; // appuyé = 1
#endif

    return bouton;
}
```

## Explications du code et des configurations de registres utilisées

Cette fonction lit le bit 5 de `GPIOB_IDR`.

```c
bouton = (GPIOB_IDR & BUTTON_MASK) ? 1u : 0u;
```

Le masque garde seulement l'état de PB5. Comme le bouton est actif à 0, la valeur est inversée. La fonction retourne donc 1 quand le bouton est appuyé.

### Registres utilisés

| Registre | Adresse | Utilité |
|---|---|---|
| `GPIOB_IDR` | `0x40010C08` | Lit l'état logique de PB5. |

## Justification avec la documentation officielle

![Registre GPIOx_IDR](images/doc_GPIO_IDR.png)

**Explication de la documentation :**

La documentation montre que chaque bit de `GPIOx_IDR` donne l'état de la broche correspondante. Le bit 5 correspond à PB5.

# 3.8 Fonction `SysTick_Init()`

## Capture du code de la fonction

```c
void SysTick_Init(void)
{
    SYST_CSR = 0u; // arrête SysTick
    tempsMs = 0u;

    SYST_RVR = 7999u; // 1 ms avec 8 MHz
    SYST_CVR = 0u; // remet à zéro

    SYST_CSR = CLKSOURCE | TICKINT | ENABLE; // démarre SysTick
}
```

## Explications du code et des configurations de registres utilisées

Cette fonction configure SysTick comme base de temps de 1 ms.

```text
8 MHz / 1000 = 8000 cycles
Reload = 8000 - 1 = 7999
```

```c
SYST_RVR = 7999u;
```

Cette ligne donne une interruption chaque 8000 cycles.

```c
SYST_CSR = CLKSOURCE | TICKINT | ENABLE;
```

Cette ligne prend l'horloge du processeur, active l'interruption et démarre SysTick.

### Registres utilisés

| Registre | Adresse | Utilité |
|---|---|---|
| `SYST_CSR` | `0xE000E010` | Active SysTick, son interruption et sa source d'horloge. |
| `SYST_RVR` | `0xE000E014` | Contient la valeur de recharge 7999. |
| `SYST_CVR` | `0xE000E018` | Remet le compteur courant à zéro. |

## Justification avec la documentation officielle

![Registre SysTick STK_CTRL](images/doc_SysTick_CTRL.png)

![Registre SysTick STK_LOAD](images/doc_SysTick_LOAD.png)

![Registre SysTick STK_VAL](images/doc_SysTick_VAL.png)

**Explication de la documentation :**

Le manuel du Cortex-M3 montre les bits ENABLE, TICKINT et CLKSOURCE dans `SYST_CSR`. Il montre aussi que la période utilise la valeur de recharge plus 1.

# 3.9 Fonction `SysTick_GetMs()`

## Capture du code de la fonction

```c
unsigned int SysTick_GetMs(void)
{
    return tempsMs;
}
```

## Explications du code et des configurations de registres utilisées

Cette fonction retourne le compteur `tempsMs`. Il contient le nombre de millisecondes depuis le démarrage de SysTick.

### Registres utilisés

| Registre | Adresse | Utilité |
|---|---|---|
| Aucun directement | — | La fonction retourne seulement une variable mise à jour par l'interruption SysTick. |

## Justification avec la documentation officielle

**Explication de la documentation :**

Cette fonction ne lit pas directement un registre. La configuration de SysTick est expliquée dans `SysTick_Init()`.

# 3.10 Fonction `SysTick_Handler()`

## Capture du code de la fonction

```c
void SysTick_Handler(void)
{
    tempsMs++; // ajoute 1 ms
}
```

## Explications du code et des configurations de registres utilisées

Cette fonction est appelée à chaque interruption SysTick. Comme l'interruption arrive chaque 1 ms, elle ajoute 1 au compteur de millisecondes.

### Registres utilisés

| Registre | Adresse | Utilité |
|---|---|---|
| Aucun directement | — | Le drapeau SysTick est géré par le cœur. La fonction augmente seulement `tempsMs`. |

## Justification avec la documentation officielle

![Registre SysTick STK_CTRL](images/doc_SysTick_CTRL.png)

**Explication de la documentation :**

La documentation du Cortex-M3 indique que SysTick peut produire une exception quand le compteur arrive à zéro et que TICKINT est activé.

# 3.11 Fonction `Timer2_Stop()`

## Capture du code de la fonction

```c
void Timer2_Stop(void)
{
    NVIC_ICER0 = TIM2_IRQ; // coupe IRQ TIM2

    RCC_APB1ENR |= TIM2_EN; // active horloge TIM2

    TIM2_CR1 = 0u; // arrête timer
    TIM2_DIER = 0u; // coupe interruptions
    TIM2_SR = 0u; // efface flags
    TIM2_CNT = 0u; // remet compteur à zéro

    NVIC_ICPR0 = TIM2_IRQ; // efface IRQ en attente

    modeTimer = TIMER2_MODE_STOP;
}
```

## Explications du code et des configurations de registres utilisées

Cette fonction arrête TIM2 et enlève les interruptions qui pourraient encore être en attente.

```c
RCC_APB1ENR |= TIM2_EN;
```

Le bit TIM2EN active l'horloge de TIM2. Il faut l'horloge pour accéder au timer.

```c
TIM2_CR1 = 0u;
TIM2_DIER = 0u;
```

Ces lignes arrêtent le compteur et désactivent son interruption Update.

```c
NVIC_ICER0 = TIM2_IRQ;
NVIC_ICPR0 = TIM2_IRQ;
```

Le bit 28 désactive l'IRQ de TIM2 et efface une ancienne demande en attente.

### Registres utilisés

| Registre | Adresse | Utilité |
|---|---|---|
| `RCC_APB1ENR` | `0x4002101C` | Active l'horloge de TIM2. |
| `TIM2_CR1` | `0x40000000` | Arrête le compteur. |
| `TIM2_DIER` | `0x4000000C` | Désactive l'interruption Update. |
| `TIM2_SR` | `0x40000010` | Efface les drapeaux de TIM2. |
| `TIM2_CNT` | `0x40000024` | Remet le compteur à zéro. |
| `NVIC_ICER0` | `0xE000E180` | Désactive l'IRQ 28. |
| `NVIC_ICPR0` | `0xE000E280` | Efface l'IRQ 28 en attente. |

## Justification avec la documentation officielle

![Registre RCC_APB1ENR](images/doc_RCC_APB1ENR.png)

![Registre TIMx_CR1](images/doc_TIM2_CR1.png)

![Registre TIMx_SR](images/doc_TIM2_SR.png)

![Registres TIMx_CNT et TIMx_PSC](images/doc_TIM2_CNT_PSC.png)

![Registre NVIC_ICER](images/doc_NVIC_ICER.png)

![Registre NVIC_ICPR](images/doc_NVIC_ICPR.png)

**Explication de la documentation :**

On voit que le bit CEN de `TIM2_CR1` démarre le compteur. Le bit UIE de `TIM2_DIER` active l'interruption Update et le bit UIF de `TIM2_SR` indique une mise à jour.

# 3.12 Fonction `Timer2_Start()`

## Capture du code de la fonction

```c
static void Timer2_Start(void)
{
    TIM2_CR1 = URS; // IRQ seulement en fin de période

    TIM2_EGR = UG; // charge PSC et ARR
    TIM2_SR = 0u; // efface UIF
    TIM2_CNT = 0u; // repart de zéro

    NVIC_ICPR0 = TIM2_IRQ; // efface ancienne IRQ
    TIM2_DIER = UIE; // active update interrupt
    NVIC_ISER0 = TIM2_IRQ; // active IRQ TIM2

    TIM2_CR1 = URS | CEN; // démarre timer
}
```

## Explications du code et des configurations de registres utilisées

Cette fonction charge la configuration de TIM2 et démarre le compteur. Elle est `static`, donc elle sert seulement dans `Timer2.c`.

```c
TIM2_CR1 = URS;
TIM2_EGR = UG;
```

URS évite que le `UG` crée une interruption. `UG` charge les valeurs de PSC et ARR.

```c
TIM2_DIER = UIE;
NVIC_ISER0 = TIM2_IRQ;
```

Ces lignes activent l'interruption Update dans TIM2 et l'IRQ 28 dans le NVIC.

```c
TIM2_CR1 = URS | CEN;
```

Le bit CEN démarre le timer.

### Registres utilisés

| Registre | Adresse | Utilité |
|---|---|---|
| `TIM2_CR1` | `0x40000000` | Règle URS et démarre le timer avec CEN. |
| `TIM2_DIER` | `0x4000000C` | Active l'interruption Update. |
| `TIM2_SR` | `0x40000010` | Efface le drapeau UIF. |
| `TIM2_EGR` | `0x40000014` | Produit l'événement UG pour charger PSC et ARR. |
| `TIM2_CNT` | `0x40000024` | Remet le compteur à zéro. |
| `NVIC_ISER0` | `0xE000E100` | Active l'IRQ 28 de TIM2. |
| `NVIC_ICPR0` | `0xE000E280` | Efface une ancienne IRQ en attente. |

## Justification avec la documentation officielle

![Registre TIMx_CR1](images/doc_TIM2_CR1.png)

![Registre TIMx_EGR](images/doc_TIM2_EGR.png)

![Registre TIMx_SR](images/doc_TIM2_SR.png)

![Registres TIMx_CNT et TIMx_PSC](images/doc_TIM2_CNT_PSC.png)

![Registre NVIC_ISER](images/doc_NVIC_ISER.png)

![Registre NVIC_ICPR](images/doc_NVIC_ICPR.png)

**Explication de la documentation :**

La documentation montre les bits CEN et URS dans `TIM2_CR1`, UIE dans `TIM2_DIER` et UG dans `TIM2_EGR`. TIM2 correspond à l'IRQ 28 dans le NVIC.

# 3.13 Fonction `Timer2_PWMInit()`

## Capture du code de la fonction

```c
void Timer2_PWMInit(void)
{
    Timer2_Stop();

    TIM2_PSC = 79u; // tick de 10 us
    TIM2_ARR = 9u; // interruption chaque 100 us

    compteurPWM = 0u;
    modeTimer = TIMER2_MODE_PWM;

    if (PWM_DUTY_PERCENT > 0u)
    {
        LED_Set(LED_ON);
    }
    else
    {
        LED_Set(LED_OFF);
    }

    Timer2_Start(); // démarre PWM
}
```

## Explications du code et des configurations de registres utilisées

Cette fonction prépare TIM2 pour le PWM logiciel du mode 1.

```c
TIM2_PSC = 79u;
TIM2_ARR = 9u;
```

```text
PSC = 79
tick = 8 MHz / (79 + 1) = 100 kHz = 10 us
ARR = 9
Update = 10 us × (9 + 1) = 100 us
100 étapes = 10 ms
PWM = 100 Hz
à 69 % : 6,9 ms ON / 3,1 ms OFF
```

Le PWM est fait dans `TIM2_IRQHandler()`. Ce n'est pas la sortie PWM matérielle de TIM2.

### Registres utilisés

| Registre | Adresse | Utilité |
|---|---|---|
| `TIM2_PSC` | `0x40000028` | Divise l'horloge de 8 MHz par 80. |
| `TIM2_ARR` | `0x4000002C` | Produit un Update après 10 ticks. |

## Justification avec la documentation officielle

![Registres TIMx_CNT et TIMx_PSC](images/doc_TIM2_CNT_PSC.png)

![Registre TIMx_ARR](images/doc_TIM2_CNT_ARR.png)

**Explication de la documentation :**

Le registre PSC divise l'horloge par `PSC + 1`. Le compteur va de 0 jusqu'à ARR, donc il compte `ARR + 1` ticks avant l'Update.

# 3.14 Fonction `Timer2_InterruptInit()`

## Capture du code de la fonction

```c
void Timer2_InterruptInit(void)
{
    Timer2_Stop();

    TIM2_PSC = 7999u; // tick de 1 ms
    TIM2_ARR = 499u; // interruption chaque 500 ms

    modeTimer = TIMER2_MODE_INTERRUPT;

    Timer2_Start();
}
```

## Explications du code et des configurations de registres utilisées

Cette fonction prépare TIM2 pour le mode 2.

```c
TIM2_PSC = 7999u;
TIM2_ARR = 499u;
```

```text
PSC = 7999
tick = 8 MHz / (7999 + 1) = 1 kHz = 1 ms
ARR = 499
Update = 1 ms × (499 + 1) = 500 ms
```

TIM2 produit donc une interruption Update chaque 500 ms.

### Registres utilisés

| Registre | Adresse | Utilité |
|---|---|---|
| `TIM2_PSC` | `0x40000028` | Divise l'horloge de 8 MHz par 8000. |
| `TIM2_ARR` | `0x4000002C` | Produit un Update après 500 ticks. |

## Justification avec la documentation officielle

![Registres TIMx_CNT et TIMx_PSC](images/doc_TIM2_CNT_PSC.png)

![Registre TIMx_ARR](images/doc_TIM2_CNT_ARR.png)

**Explication de la documentation :**

La documentation montre que PSC règle le diviseur du timer et que l'événement Update arrive quand le compteur atteint ARR.

# 3.15 Fonction `TIM2_IRQHandler()`

## Capture du code de la fonction

```c
void TIM2_IRQHandler(void)
{
    if ((TIM2_SR & UIF) == 0u)
    {
        return;
    }

    TIM2_SR = 0u; // efface flag

    if (modeTimer == TIMER2_MODE_PWM)
    {
        if (compteurPWM < PWM_DUTY_PERCENT)
        {
            LED_Set(LED_ON);
        }
        else
        {
            LED_Set(LED_OFF);
        }

        compteurPWM++;

        if (compteurPWM >= PWM_STEPS)
        {
            compteurPWM = 0u;
        }
    }
    else if (modeTimer == TIMER2_MODE_INTERRUPT)
    {
        LED_Toggle(); // change LED à chaque interruption
    }
}
```

## Explications du code et des configurations de registres utilisées

Cette fonction est appélé par l'interruption de TIM2.

```c
if ((TIM2_SR & UIF) == 0u)
```

Cette ligne vérifie que l'interruption vient bien d'un événement Update.

En mode PWM, `compteurPWM` va de 0 à 99. La LED est allumée pendant le nombre d'étapes donné par `PWM_DUTY_PERCENT`, puis éteinte pour le reste. Avec 69, elle est ON pendant 69 étapes et OFF pendant 31 étapes.

En mode interruption, `LED_Toggle()` change la LED d'état toutes les 500 ms.

### Registres utilisés

| Registre | Adresse | Utilité |
|---|---|---|
| `TIM2_SR` | `0x40000010` | Vérifie et efface le drapeau UIF. |

## Justification avec la documentation officielle

![Registre TIMx_SR](images/doc_TIM2_SR.png)

**Explication de la documentation :**

Le bit UIF de `TIM2_SR` passe à 1 quand un événement Update arrive. Le programme l'efface avant de traiter le mode actif.

# 4. Conclusion

Le programme utilise maintenant PB6 pour la LED et PB5 pour le bouton. SysTick sert de base de temps au mode 0. TIM2 sert au PWM logiciel du mode 1 et aux interruptions du mode 2. Les mesures obtenues correspondent au fonctionnement attendu des trois modes.
