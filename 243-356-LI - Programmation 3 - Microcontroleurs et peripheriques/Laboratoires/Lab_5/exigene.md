# Guide d’exigences et de préparation — Lab 5

**Projet visé :** Lab 5 — I²C avec STM32F103 et PCA9685  
**Révision des sources :** 19 novembre 2026  
**Date de remise indiquée dans le PDF :** 24 novembre 2026  
**État de validation de ce guide :** revue statique seulement; aucune nouvelle compilation ni mesure matérielle effectuée pendant sa rédaction.

## 1. Objectif du laboratoire

Le STM32F103 agit comme contrôleur I²C. Il doit communiquer sans bibliothèque avec la carte PCA9685 et commander le servomoteur du canal 0 selon la position du potentiomètre R1-1.

Résultats attendus :

- Le bus I²C1 fonctionne en mode Standard à 100 kHz.
- Le potentiomètre commande progressivement un angle de 0° à 180°.
- Le servo est branché au canal 0 du PCA9685.
- L’écran LCD affiche l’angle au format `ANGLE:090`, avec trois chiffres.
- Le code n’utilise pas de bibliothèque de périphériques.
- Les fonctions imposées par les laboratoires précédents gardent leurs noms et leurs paramètres.

## 2. Organisation et interfaces exigées

Chaque module possède un fichier source `.c` et un fichier d’en-tête `.h`. Les fonctions publiques ont le préfixe de leur module. Les prototypes doivent être identiques entre le `.h`, la définition du `.c` et les appels des autres modules.

### LCD et ADC à réutiliser

- `LCD_Init(void)`
- `LCD_Commande(bool RW, bool RS, uint8_t Data)`
- `LCD_Ecrire_Char(unsigned char caractere, uint8_t ligne, uint8_t colonne)`
- `ADC1_Init(void)`
- `ADC1_Lire(void)`, qui retourne un `uint16_t`

### Fonctions I²C à fournir

- `I2C1_Init(void)`, sans valeur de retour.
- `I2C1_Ecrire_Registre(uint8_t adresse, uint8_t registre, uint8_t donnee)`, qui retourne `bool`.
- `I2C1_Lire_Registre(uint8_t adresse, uint8_t registre, uint8_t *donnee)`, qui retourne `bool`.

Pour les deux échanges, `true` indique une opération réussie. `false` indique notamment un NACK ou un délai dépassé. Le paramètre adresse est une adresse I²C de 7 bits; le test du laboratoire utilise l’adresse `0x40` du PCA9685.

### Fonctions PCA9685 à fournir

- `PCA9685_Init(void)`, qui retourne `bool`.
- `PCA9685_Definir_PWM(uint8_t canal, uint16_t debut, uint16_t fin)`, qui retourne `bool`.
- `PCA9685_Definir_Angle(uint8_t canal, uint8_t angle)`, qui retourne `bool`.

Les erreurs des transferts I²C doivent remonter jusqu’aux fonctions PCA9685 et être traitées par le programme principal. Une fonction qui retourne un état ne doit pas voir son résultat ignoré.

### Responsabilités des modules

| Module | Responsabilité attendue |
|---|---|
| `I2C.c/.h` | Configuration de I²C1 et échanges de lecture/écriture de registres; attente bornée et statut d’erreur. |
| `PCA9685.c/.h` | Adresse et registres du PCA9685, initialisation PWM, conversion de l’angle et écriture du canal. |
| `LCD.c/.h` | Broches, commandes et écriture LCD; seules les fonctions publiques nécessaires dans l’en-tête. |
| `ADC.c/.h` | Configuration et lecture de l’ADC1; les registres ADC restent dans le module ADC. |
| `gpio.c/.h` | Configuration des broches GPIO réutilisées par les autres périphériques. |
| `SysTick.c/.h` | Registres SysTick, compteur et délais utilisés par les modules. |
| `main.c` | Initialisations une fois, puis boucle principale selon la règle du PDF. |

## 3. Tests de démonstration obligatoires

Les tests se font dans l’ordre et sans modifier ni recompiler le programme entre les tests.

| # | Test | Procédure | Critère de réussite | Preuve à noter |
|---:|---|---|---|---|
| 1 | Démarrage | Appuyer sur RESET. | L’angle apparaît immédiatement sans caractère parasite; le servo prend la position du potentiomètre. | Résultat observé au démarrage. |
| 2 | Fréquence I²C | Mesurer SCL pendant un échange avec l’analyseur logique. | 100 kHz ±10 %, soit 90 à 110 kHz. | Fréquence mesurée et capture. |
| 3 | Trame d’écriture | Décoder un échange avec le PCA9685. | START, adresse `0x40` en écriture, ACK, registre, ACK, donnée, ACK, STOP. | Capture décodée avec ACK visibles. |
| 4 | Fréquence PWM | Mesurer la sortie du canal 0. | 50 Hz ±2 Hz, soit 48 à 52 Hz. | Fréquence mesurée. |
| 5 | Positions extrêmes | Mettre le potentiomètre aux deux extrêmes et mesurer l’impulsion. | Affichage `ANGLE:000` puis `ANGLE:180`; impulsion 1,00 ms puis 2,00 ms, chacune à ±0,05 ms. | Deux angles et deux largeurs mesurées. |
| 6 | Variation continue | Tourner lentement le potentiomètre dans les deux sens. | Angle et servo varient progressivement, sans saut ni blocage. | Observation de la démonstration. |

Ne pas inventer de mesures. Une valeur calculée ou prévue n’est pas une mesure à l’analyseur logique.

## 4. Règles de programmation du PDF

Cocher chaque point après revue des sources finales :

- [ ] Chaque module possède sa paire `.c/.h`; `main.c` contient les initialisations et la boucle principale.
- [ ] Chaque `.h` possède une garde d’inclusion.
- [ ] Les `.h` contiennent uniquement des déclarations, constantes et types; aucune définition de fonction ou de variable.
- [ ] Les fonctions et variables utilisées dans un seul module sont `static`.
- [ ] Les noms sont explicites; les fonctions publiques portent le préfixe du module.
- [ ] Les constantes sont en majuscules.
- [ ] Les broches, masques et valeurs de registre sont des constantes nommées; éviter les nombres magiques.
- [ ] L’indentation est uniforme et toutes les structures de contrôle ont des accolades.
- [ ] Les données matérielles utilisent des types de taille fixe (`uint8_t`, `uint16_t`, `uint32_t`) et `bool`; pas `int` pour les registres ou les données matérielles.
- [ ] Les ISR sont courtes : acquittement du drapeau, mise à jour minimale, puis retour.
- [ ] Toute donnée partagée avec une ISR est déclarée `volatile`.
- [ ] Les périphériques sont initialisés une seule fois avant la boucle infinie.
- [ ] Les variables globales sont limitées aux données partagées avec une ISR; les autres sont locales ou privées au module.
- [ ] La compilation finale ne produit aucun avertissement; aucun code mort ou commenté ne reste dans la remise.


### Bloqueurs de conformité à l’énoncé

- [ ] Les noms et signatures I²C actuels diffèrent des interfaces imposées. Le fichier actuel n’expose pas l’API de lecture de registre demandée.
- [ ] Les noms et signatures PCA9685 actuels diffèrent des interfaces imposées.
- [ ] `main.c` appelle les anciennes fonctions et ignore le résultat de l’initialisation PCA9685 et du réglage d’angle.
- [ ] Les constantes servo sont actuellement commentées comme environ 0,5 à 2,5 ms; le test demande 1 à 2 ms. Le résultat réel doit être réglé et mesuré.
- [ ] La règle sur le contenu de `main.c` doit être revue : le fichier courant calcule l’angle et prépare le texte LCD dans la boucle.

### ADC et affichage de tension ajoutés

- [ ] `ADC1_VMIN_MV` et `ADC1_VMAX_MV` sont encore à zéro; le drapeau `ADC1_LIMITES_MESUREES` est aussi à zéro.
- [ ] La conversion affichée utilise 3 300 mV et le code maximal 4 095; elle ne consulte pas Vmin/Vmax.
- [ ] La ligne de tension ADC n’est pas exigée par le PDF. Elle entraîne une deuxième lecture ADC par tour de boucle, en plus de celle qui sert à calculer l’angle.
- [ ] Si la mesure de Vmax de 3,1964 V correspond réellement à la butée maximale et si Vref vaut 3,300 V, une conversion linéaire de 0–4095 vers 0–180 donnerait environ 174° à cette butée. C’est une estimation conditionnelle; relever les codes ADC réels aux deux butées avant de décider d’une calibration.
- [ ] 0,0003 V correspond à 0,3 mV, donc s’arrondit à 0 mV si le stockage se fait en millivolts entiers.

### Nettoyage et preuves à terminer

- [ ] Vérifier les fonctions non appelées dans le projet, notamment les fonctions Timer2, les fonctions de test/duty-cycle PCA9685, l’ancienne écriture d’octet I²C et l’alias d’affichage LCD. Retirer le code réellement inutile ou expliquer pourquoi il est conservé.
- [ ] `plan.md` donne des affectations différentes pour les canaux de l’analyseur logique. Corriger les notes en fonction du branchement physique réel avant la démonstration.
- [ ] L’ancienne erreur `ADC1_OFFSET_ARRONDI` semble résolue dans le snapshot : la constante et son utilisation sont maintenant ensemble dans `LCD.c`. Cela reste à confirmer par une nouvelle compilation.
- [ ] Une revue de texte ne prouve ni une compilation réussie, ni un ACK du PCA9685, ni les fréquences ou impulsions réelles.

## 6. Revue complémentaire des normes demandées

Cette section est un aide-mémoire de vérification. Elle ne constitue pas une déclaration de conformité ni une certification. L’application exacte de ces normes exige leurs éditions officielles, le périmètre applicable, des résultats de vérification et une gestion documentée des écarts.

### DO-178C

- [ ] Distinguer une simple revue de code d’un processus de certification logiciel aéronautique.
- [ ] Pour une revendication réelle de conformité, identifier le niveau d’assurance et les données de cycle de vie applicables.
- [ ] Vérifier l’existence des exigences, de leur traçabilité vers le code et les tests, des revues, de la gestion de configuration et des preuves de vérification.
- [ ] Ne pas présenter la compilation ou les six démonstrations comme une certification DO-178C.

### MISRA C:2023

- [ ] Confirmer que l’édition demandée est bien MISRA C:2023 et obtenir le texte/licence et le profil d’analyse appropriés.
- [ ] Exécuter un analyseur configuré pour cette édition et examiner chaque règle obligatoire, requise et consultative selon le profil retenu.
- [ ] Documenter les écarts justifiés et leurs approbations; ne pas déclarer « conforme MISRA » sur la seule base d’un style propre.
- [ ] L’édition MISRA C:2025 est maintenant publiée; ne pas substituer cette édition à celle demandée par le projet sans accord explicite.

### NASA JPL C Coding Standard

- [ ] Vérifier le code contre la version officielle applicable du standard JPL D-60411; la référence NASA indique la version 1.0 datée du 3 mars 2009.
- [ ] Revoir en priorité les limites vérifiables des boucles d’attente, la vérification des retours d’erreur, les conversions et accès mémoire, les variables de portée minimale et le code inutilisé.
- [ ] Examiner séparément l’assembleur en ligne et les accès mémoire-mappés; documenter leur nécessité et leur revue.
- [ ] Ne pas conclure à la conformité si la copie du standard ou certaines règles ne sont pas disponibles.

### NXP UM10204 — I²C

- [ ] Vérifier les séquences START, adresse 7 bits et bit R/W, ACK/NACK, données, STOP et répétition START pour les lectures.
- [ ] Mesurer SCL à 100 kHz ±10 % pour satisfaire le test du laboratoire.
- [ ] Vérifier à l’analyseur logique le niveau électrique, les fronts de montée et les résistances de rappel selon le montage réel; une configuration logicielle seule ne le prouve pas.
- [ ] Vérifier que PB6/PB7 sont les broches utilisées, que les lignes sont open-drain et que la masse est commune.
- [ ] Garder les preuves du bus et des signaux séparées des résultats de compilation.

Les normes ont des portées différentes : UM10204 porte sur le bus I²C; MISRA et le standard JPL donnent des règles de développement C; DO-178C porte sur un processus d’assurance logiciel. Les satisfaire ne se résume pas à renommer les fonctions ou à compiler sans avertissement.

## 7. Ordre de travail conseillé

1. Clarifier avec l’enseignant la règle sur les modifications de code assistées par IA.
2. Comparer les interfaces publiques de `I2C.h` et `PCA9685.h` au PDF; relever les différences sans modifier les autres laboratoires.
3. Réaliser soi-même les ajustements des interfaces, implémentations et appels; vérifier que les erreurs I²C/PCA9685 remontent jusqu’au programme principal.
4. Vérifier la lecture R1-1, l’affichage `ANGLE:ddd`, la correspondance des deux butées et la sortie du canal 0.
5. Décider si l’affichage de tension additionnel reste; s’il reste, vérifier Vref, les mesures ADC et les constantes Vmin/Vmax.
6. Régler les extrêmes servo selon le test 1–2 ms et les mesurer.
7. Revoir les 14 règles, supprimer le code mort et corriger les notes d’analyseur logique.
8. Dans QC1, confirmer que le projet/source sélectionné est le Lab 5; effectuer une compilation propre et sans avertissement. Ne pas se fier à un ancien binaire.
9. Faire les six tests matériels dans l’ordre, sans modifier le programme entre eux, et inscrire uniquement les résultats observés.
10. Expliquer chaque module, chaque retour d’erreur, l’adresse `0x40`, le calcul de l’angle et les mesures lors de la démonstration.
11. Préparer le ZIP demandé après validation.

## 8. Remise

Le PDF demande un ZIP nommé `Remise_Laboratoire_VOTRENOM.zip`, contenant les fichiers de code. Remplacer `VOTRENOM` par le nom demandé par l’enseignant.

Avant de créer le ZIP :

- [ ] N’inclure que les fichiers nécessaires au projet : sources, en-têtes, démarrage et script linker si requis pour reconstruire le firmware.
- [ ] Exclure `build/qc1`, les objets `.o/.obj`, les exécutables `.elf`, `.bin`, `.hex`, `.map`, les caches et fichiers temporaires.
- [ ] Ne pas inclure ce guide de révision ni `plan.md`, sauf demande explicite de l’enseignant.
- [ ] Ouvrir le ZIP et vérifier la liste des fichiers avant le dépôt Omnivox.
- [ ] Vérifier que le nom final correspond exactement à la consigne.

## 9. Questions de compréhension à savoir expliquer

- Quelle est la différence entre une adresse I²C 7 bits et l’octet adresse plus bit R/W transmis sur le bus?
- Pourquoi une transaction de lecture de registre commence-t-elle par sélectionner le registre?
- À quoi servent ACK, NACK, START, répétition START et STOP?
- Pourquoi une attente matérielle doit-elle avoir une limite de temps?
- Quelle différence y a-t-il entre le code brut ADC, la tension de référence et la tension mesurée au potentiomètre?
- Comment les valeurs ADC des butées donnent-elles les angles 0° et 180°?
- Comment le prescaler du PCA9685 règle-t-il la fréquence PWM, et comment mesurer la largeur d’impulsion?
- Quelle preuve vient du compilateur, quelle preuve vient de l’analyseur logique et quelle preuve vient du servo réel?

## 10. Références

- [Énoncé officiel TEP_Prog_Lab5.pdf](/Users/matisserheaume-viale/Documents/PDF/TEP_Prog_Lab5.pdf)
- [RTCA — DO-178C](https://www.rtca.org/do-178/)
- [MISRA — MISRA C:2023](https://misra.org.uk/product/misra-c2023/)
- [MISRA — conformité et écarts](https://misra.org.uk/compliance/)
- [NASA Software Engineering Handbook — référence JPL C Coding Standard D-60411](https://swehb.nasa.gov/spaces/SITE/pages/16450595/JPL%2BC%2BCoding%2BStandard)
- [NXP UM10204 — I²C-bus specification and user manual, Rev. 7.0](https://www.nxp.com/docs/en/user-guide/UM10204.pdf)
