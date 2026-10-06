---
title: "Laboratoire 3 — Affichage de l’heure sur LCD avec TIM2"
author: "Matisse Rhéaume-Viale"
date: "6 octobre 2026"
lang: fr-CA
geometry: margin=2cm
fontsize: 10pt
header-includes:
  - \usepackage{graphicx}
---

**Cours :** 243-356-LI — Programmation 3 — Microcontrôleurs et périphériques  
**Enseignant :** Gabriel Fortin-Bélanger  
**Groupe :** 0001

\newpage

# 1. Objectif

Le but est d’afficher l’heure au format `HH:MM:SS` sur l’écran LCD. Le Timer 2 déclenche une interruption chaque seconde. L’interruption avance l’heure et la boucle principale met l’écran à jour.

Le programme utilise les registres du STM32F103 directement. Il n’utilise pas HAL. L’écran est piloté en mode 4 bits avec les broches `DB4` à `DB7`.

# 2. Branchement de l’écran

Le plan du laboratoire omet la broche `E` (Enable). Le pilote LCD utilisé au Lab 1 confirme que `E` est reliée à PB11. Il faut donc aussi brancher cette broche.

| Signal LCD | Broche STM32F103 | Rôle |
|---|---:|---|
| RS | PB9 | Sélection commande ou donnée |
| R/W | PB10 | Sélection lecture ou écriture |
| E | PB11 | Validation du transfert |
| DB4 | PB12 | Donnée, bit 4 |
| DB5 | PB13 | Donnée, bit 5 |
| DB6 | PB14 | Donnée, bit 6 |
| DB7 | PB15 | Donnée, bit 7 |

\begin{center}
\includegraphics[width=0.75\textwidth]{images/doc_tc2004a_pins.png}

{\small\textit{Figure 1 — Brochage du module TC2004A. Tinsharp, p. 5.}}
\end{center}

Le code écrit seulement dans l’écran. Il garde donc `R/W` à 0. La fonction `LCD_Commande()` ne fait rien si `RW` vaut `true`, car son prototype ne permet pas de retourner la valeur lue et les lignes de données sont configurées en sorties.

**À confirmer avant le branchement :** la fiche du module TC2004A indique une alimentation logique de 5 V et un niveau haut minimal qui dépend de cette alimentation. Il faut vérifier la fiche du module exact et la compatibilité entre ses entrées et les sorties 3,3 V du STM32. Le câblage électrique n’a pas été vérifié sur la carte pendant ce travail.

\newpage

# 3. Fonctionnement du programme

Au démarrage, `SystemInit()` sélectionne le HSI à 8 MHz et laisse les préscalers AHB et APB à 1. `LCD_Init()` configure les broches et démarre l’écran en mode 4 bits. Le programme affiche ensuite `00:00:00`, puis démarre TIM2.

TIM2 compte une seconde par période. Son interruption met à jour le nombre de secondes depuis minuit et envoie l’événement `SEV`. La boucle principale attend avec `WFE`. Au réveil, elle vérifie si le compteur de mise à jour a changé, puis rafraîchit l’écran. Les écritures LCD restent donc dans le programme principal et ne ralentissent pas l’interruption.

Le temps est gardé sous forme d’un nombre de secondes entre 0 et 86 399. Après `23:59:59`, il revient à zéro. Cette représentation rend le passage à la minute et à minuit simple à gérer.

# 4. Plan de test

Les essais ci-dessous viennent des consignes du Lab 3. Aucun résultat Saleae n’est présenté comme mesuré : les essais sur carte et les captures de l’analyseur logique restent à faire.

| Cas | Vérification | Résultat attendu | Résultat obtenu |
|---|---|---|---|
| 1 | Capturer une commande LCD avec l’analyseur logique | Pour `0x28`, `RS=0`, `R/W=0`, puis les nibbles `0010` et `1000`, chacun validé par une impulsion sur `E`. | **NON TESTÉ** — capture matérielle absente. |
| 2 | Mesurer la précision de l’affichage des secondes | Le début de chaque rafraîchissement de l’heure est espacé d’environ 1 s. Noter les périodes minimale, moyenne et maximale mesurées. | **NON TESTÉ** — capture matérielle absente. |
| 3 | Vérifier le retour à zéro | En partant de `23:59:55`, l’écran doit afficher `00:00:00` après cinq interruptions d’une seconde. | **NON TESTÉ** — capture matérielle absente. |

## 4.1 Procédures proposées

**Cas 1 — Transmission d’une commande.** Brancher les voies de l’analyseur sur `RS`, `R/W`, `E` et `DB4` à `DB7`, puis redémarrer la carte. Capturer l’initialisation et repérer la commande `0x28`. Vérifier `RS=0`, `R/W=0`, les deux nibbles dans le bon ordre et une impulsion `E` par nibble.

**Cas 2 — Intervalle des secondes.** Capturer plusieurs rafraîchissements consécutifs sur `E`. Pour chaque rafraîchissement de l’heure, noter l’instant du premier front montant de `E`. Calculer les intervalles entre ces instants et rapporter le minimum, la moyenne et le maximum. La valeur attendue est proche de 1 s; la précision réelle dépend notamment de l’horloge de la carte.

**Cas 3 — Passage de minuit.** Pour cet essai seulement, remplacer l’initialisation à zéro dans `main()` par :

```c
(void)Timer2_SetTime(23U, 59U, 55U);
```

Après la compilation et le redémarrage, vérifier la suite `23:59:55`, `23:59:56`, `23:59:57`, `23:59:58`, `23:59:59`, puis `00:00:00`. Remettre ensuite l’initialisation normale à zéro. Cette procédure n’a pas été exécutée sur le matériel.

\newpage

# 5. Analyse des fonctions demandées

## 5.1 `LCD_Init()`

### Extrait du code source

```c
void LCD_Init(void)
{
    LCD_GPIO_Init();

    /* SysTick sert ici de compteur libre pour les délais du LCD. */
    SYST_CSR = 0UL;
    SYST_RVR = SYST_RELOAD;
    SYST_CVR = 0UL;
    SYST_CSR = SYST_CLKSOURCE | SYST_ENABLE;

    LCD_Delai_Ms(50UL);

    LCD_Envoyer_Nibble(0x03U);
    LCD_Delai_Us(4500UL);
    LCD_Envoyer_Nibble(0x03U);
    LCD_Delai_Ms(1UL);
    LCD_Envoyer_Nibble(0x03U);
    LCD_Delai_Us(150UL);
    LCD_Envoyer_Nibble(0x02U);
    LCD_Delai_Us(150UL);

    LCD_Commande(false, false, LCD_CMD_FUNCTION_SET);
    LCD_Commande(false, false, LCD_CMD_DISPLAY_OFF);
    LCD_Commande(false, false, LCD_CMD_CLEAR);
    LCD_Commande(false, false, LCD_CMD_ENTRY_MODE);
    LCD_Commande(false, false, LCD_CMD_DISPLAY_ON);
}
```

### Explication

`LCD_GPIO_Init()` active l’horloge de GPIOB, configure PB9 à PB15 comme sorties push-pull et met les sorties à zéro. SysTick est démarré sans interruption; il sert de compteur libre pour mesurer les délais. À 8 MHz, chaque microseconde correspond à 8 cycles.

Après 50 ms, le code envoie les nibbles `0011`, `0011`, `0011`, puis `0010`. Cette séquence place le contrôleur LCD en mode 4 bits. Les commandes suivantes configurent l’écran, effacent son contenu et activent l’affichage. `0x28` choisit le transfert 4 bits et le format de caractère 5 × 8 points.

| Registre | Adresse | Utilisation dans le pilote |
|---|---:|---|
| `RCC_APB2ENR` | `0x40021018` | Bit 3 à 1 pour activer GPIOB. |
| `GPIOB_CRH` | `0x40010C04` | Configure PB9 à PB15 en sorties push-pull à 2 MHz (`MODE=10`, `CNF=00`). |
| `GPIOB_BSRR` | `0x40010C10` | Met les broches de commande et de données à 0 sans opération de lecture-modification-écriture. |
| `SysTick_CSR` | `0xE000E010` | Sélectionne l’horloge processeur et démarre le compteur; `TICKINT` reste à 0. |
| `SysTick_RVR` | `0xE000E014` | Fixe le rechargement à la valeur maximale de 24 bits. |
| `SysTick_CVR` | `0xE000E018` | Fournit la valeur descendante utilisée pour calculer les délais. |

Les extraits du manuel STM32 ci-dessous montrent le bit d’activation de GPIOB, la configuration de `GPIOx_CRH` et le registre de sortie `GPIOx_BSRR`.

\begin{center}
\includegraphics[width=0.68\textwidth]{images/doc_rm0008_rcc_iopben.png}

{\small\textit{Figure 2 — Registre de configuration et activation de GPIOB. ST RM0008, p. 113.}}
\end{center}

\begin{center}
\includegraphics[width=0.65\textwidth]{images/doc_rm0008_gpio_crh.png}

{\small\textit{Figure 3 — Configuration des broches PB8 à PB15. ST RM0008, p. 172.}}
\end{center}

\begin{center}
\includegraphics[width=0.65\textwidth]{images/doc_rm0008_gpio_bsrr.png}

{\small\textit{Figure 4 — Registre de mise à 1 et de remise à 0. ST RM0008, p. 173.}}
\end{center}

La fiche HD44780U montre le transfert en deux nibbles ainsi que les délais d’initialisation. Le schéma de séquence ci-dessous vient de la documentation du cours utilisée au Lab 1.

\begin{center}
\includegraphics[width=0.48\textwidth]{images/doc_hd44780_init_sequence.png}

{\small\textit{Figure 5 — Séquence de démarrage en mode 4 bits. Documentation du cours, Lab 1.}}
\end{center}

\begin{center}
\includegraphics[width=0.62\textwidth]{images/doc_hd44780_4bit.png}

{\small\textit{Figure 6 — Transfert et timing de l’interface 4 bits. Hitachi HD44780U, p. 22.}}
\end{center}

\newpage

## 5.2 `LCD_Commande()`

### Extrait du code source

```c
void LCD_Commande(bool RW, bool RS, uint8_t Data)
{
    if (RW)
    {
        return;
    }

    LCD_SetPin(LCD_RS_PIN, RS);
    LCD_SetPin(LCD_RW_PIN, false);

    LCD_Envoyer_Nibble((uint8_t)(Data >> 4U));
    LCD_Envoyer_Nibble((uint8_t)(Data & LCD_DATA_MASK));

    if ((!RS) && ((Data == LCD_CMD_CLEAR) || (Data == LCD_CMD_HOME)))
    {
        LCD_Delai_Ms(2UL);
    }
    else
    {
        LCD_Delai_Us(50UL);
    }
}
```

### Explication

`RS=0` sélectionne une commande; `RS=1` sélectionne un caractère. Le pilote écrit toujours avec `R/W=0`. L’octet est séparé en deux groupes de quatre bits : le nibble de poids fort est transmis en premier, puis celui de poids faible. `LCD_Envoyer_Nibble()` place les quatre bits sur PB12 à PB15 et produit une impulsion sur PB11.

Le registre `GPIOB_BSRR` met chaque broche à 1 ou à 0. Pour éviter de toucher aux autres sorties, le code écrit les bits de mise à 1 ou de remise à 0 dans une seule écriture au registre. Les commandes `CLEAR` et `HOME` ont un délai de 2 ms; les autres opérations attendent 50 µs.

La figure du fabricant confirme l’ordre des nibbles dans une interface 4 bits. Pour `0x28`, le bus présente d’abord `0010`, puis `1000`.

\begin{center}
\includegraphics[width=0.62\textwidth]{images/doc_hd44780_4bit.png}

{\small\textit{Figure 7 — Ordre des transferts en mode 4 bits. Hitachi HD44780U, p. 22.}}
\end{center}

\newpage

## 5.3 `LCD_Ecrire_Char()`

### Extrait du code source

```c
void LCD_Ecrire_Char(unsigned char caractere, uint8_t ligne, uint8_t colonne)
{
    uint8_t adresse;

    if ((ligne < 1U) || (ligne > LCD_ROWS)
        || (colonne < 1U) || (colonne > LCD_COLUMNS))
    {
        return;
    }

    adresse = (uint8_t)(LCD_CMD_SET_DDRAM
                      | adresseLigne[ligne - 1U]
                      | (uint8_t)(colonne - 1U));
    LCD_Commande(false, false, adresse);
    LCD_Commande(false, true, (uint8_t)caractere);
}
```

### Explication

Les numéros de ligne et de colonne commencent à 1. La fonction vérifie qu’ils sont dans les limites d’un écran 20 × 4. Elle ajoute le décalage de la colonne à l’adresse de base de la ligne. Les adresses utilisées sont `0x00`, `0x40`, `0x14` et `0x54`.

Le bit 7 de l’instruction est mis à 1 pour choisir une adresse DDRAM. La fonction envoie d’abord cette instruction avec `RS=0`, puis le caractère avec `RS=1`. Le tableau d’adresses correspond à la disposition mémoire du contrôleur pour un écran 20 × 4.

\begin{center}
\includegraphics[width=0.65\textwidth]{images/doc_hd44780_ddram.png}

{\small\textit{Figure 8 — Instruction et adresses DDRAM. Hitachi HD44780U, p. 29.}}
\end{center}

\newpage

## 5.4 `LCD_Affichage_Temps()`

### Extrait du code source

```c
void LCD_Affichage_Temps(void)
{
    uint32_t temps;
    uint8_t heures;
    uint8_t minutes;
    uint8_t secondes;
    unsigned char caracteres[8];
    uint8_t colonne;

    temps = Timer2_GetSecondsSinceMidnight();
    heures = (uint8_t)(temps / 3600UL);
    minutes = (uint8_t)((temps / 60UL) % 60UL);
    secondes = (uint8_t)(temps % 60UL);

    caracteres[0] = (unsigned char)('0' + (heures / 10U));
    caracteres[1] = (unsigned char)('0' + (heures % 10U));
    caracteres[2] = ':';
    caracteres[3] = (unsigned char)('0' + (minutes / 10U));
    caracteres[4] = (unsigned char)('0' + (minutes % 10U));
    caracteres[5] = ':';
    caracteres[6] = (unsigned char)('0' + (secondes / 10U));
    caracteres[7] = (unsigned char)('0' + (secondes % 10U));

    for (colonne = 0U; colonne < 8U; colonne++)
    {
        LCD_Ecrire_Char(caracteres[colonne], 1U, (uint8_t)(colonne + 1U));
    }
}
```

### Explication

La fonction lit le compteur de secondes, puis calcule les heures, les minutes et les secondes. Chaque chiffre est converti en caractère ASCII en ajoutant sa valeur à `'0'`. Elle envoie ensuite les huit caractères sur la première ligne, de la colonne 1 à la colonne 8.

Cette fonction ne configure pas de registre matériel elle-même. Les transferts se font par `LCD_Ecrire_Char()` et `LCD_Commande()`. La figure du fabricant ci-dessous montre comment les caractères passent au LCD par son interface 4 bits.

\begin{center}
\includegraphics[width=0.62\textwidth]{images/doc_hd44780_4bit.png}

{\small\textit{Figure 9 — Transfert d’un octet vers l’écran en mode 4 bits. Hitachi HD44780U, p. 22.}}
\end{center}

# 6. Réglage de TIM2

Le HSI est à 8 MHz. Comme le préscaler APB1 vaut 1, l’horloge de TIM2 est aussi à 8 MHz. Le code utilise `PSC=7999` et `ARR=999` :

$$
f_{\mathrm{update}} = \frac{8\,000\,000}{(7999+1)(999+1)} = 1\ \text{Hz}
$$

TIM2 produit donc une interruption chaque seconde. Le registre `TIM2_EGR` reçoit `UG` pour appliquer le préscaler. Le code efface ensuite les indicateurs en attente, active `UIE` dans `TIM2_DIER`, puis active l’IRQ 28 dans le NVIC.

| Registre | Adresse | Valeur ou rôle |
|---|---:|---|
| `RCC_APB1ENR` | `0x4002101C` | Bit 0 à 1 pour activer TIM2. |
| `TIM2_CR1` | `0x40000000` | `URS` choisit les demandes Update du compteur; `CEN` démarre le timer. |
| `TIM2_DIER` | `0x4000000C` | `UIE` autorise l’interruption Update. |
| `TIM2_SR` | `0x40000010` | L’indicateur `UIF` signale une période terminée. |
| `TIM2_EGR` | `0x40000014` | `UG` applique immédiatement les valeurs de base du timer. |
| `TIM2_PSC` | `0x40000028` | Divise l’horloge par `PSC + 1 = 8000`. |
| `TIM2_ARR` | `0x4000002C` | Compte `ARR + 1 = 1000` impulsions avant Update. |
| `NVIC_ISER0` | `0xE000E100` | Bit 28 à 1 pour activer l’IRQ TIM2. |

\begin{center}
\includegraphics[width=0.73\textwidth]{images/doc_rm0008_clock_tree.png}

{\small\textit{Figure 10 — Arbre d’horloge STM32F103. ST RM0008, p. 93.}}
\end{center}

\begin{center}
\includegraphics[width=0.73\textwidth]{images/doc_rm0008_tim2_psc_arr.png}

{\small\textit{Figure 11 — Définition des préscaler et auto-reload de TIM2. ST RM0008, p. 419.}}
\end{center}

# 7. Vérification logicielle et limites

La compilation et l’édition de liens croisées du firmware avec `arm-none-eabi-gcc` ont réussi. L’image obtenue contient 2 004 octets de code et 1 576 octets de BSS. L’éditeur de liens a affiché des avertissements de stubs système newlib (`_close`, `_read`, `_write`, `_lseek`); il s’agit des appels système non fournis pour cette application bare metal. Aucun résultat de compilation ne prouve le fonctionnement sur une carte.

Les trois essais avec l’analyseur logique, l’affichage réel du LCD, la précision du HSI et le passage physique à minuit restent **NON TESTÉS**. Il faudra ajouter les captures de l’analyseur et les observations de la carte après les essais.

Le dossier `build/qc1` présent dans Lab 3 a été copié du Lab 2 et son cache CMake référence toujours les sources du Lab 2. Il n’a pas été utilisé pour la compilation ci-dessus. Le firmware a été compilé dans un répertoire temporaire séparé afin de préserver ces fichiers existants.

# 8. Conclusion

Le code du Lab 3 configure le LCD en mode 4 bits, place les caractères à l’adresse DDRAM voulue et convertit le compteur de secondes en affichage `HH:MM:SS`. TIM2 est configuré pour une interruption d’une seconde et le passage de `23:59:59` à `00:00:00` est prévu dans le code. La compilation croisée a réussi. Il reste à vérifier le branchement électrique, exécuter les trois essais sur la carte et ajouter leurs captures réelles.

# Références

- Consignes du laboratoire : `TEP_Prog_Lab3.pdf`, fourni avec le travail.
- Plan du laboratoire : `Laboratoires/Lab_3/plan.md`.
- Documentation du contrôleur : Hitachi, *HD44780U Dot Matrix Liquid Crystal Display Controller/Driver*, fichier `../../../Documentation/Datashit/HD44780.pdf`, pages 22 et 29.
- Documentation du module : Tinsharp, *TC2004A*, fichier `../../../Documentation/Datashit/TC2004A-01.pdf`, page 5.
- Documentation STM32 : STMicroelectronics, *RM0008 Reference Manual*, fichier RM0008 dans `../../../Documentation/Datashit/`; pages 93, 113, 172–173 et 418–419.
- Rapport du Lab 1 : `../../../Documentation/RapportLab1.pdf`; utilisé pour confirmer le branchement de `E` sur PB11.
- Rapport du Lab 2 : `Laboratoires/Lab_2/RapportLab2.md`; utilisé comme référence pour la structure et la présentation du rapport.
