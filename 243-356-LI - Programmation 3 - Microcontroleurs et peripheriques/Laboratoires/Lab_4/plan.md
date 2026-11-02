1. Objectif
Lire la tension analogique du potentiomètre R1-1 sur PA0 / ADC1_IN0, la convertir avec l'ADC1 12 bits et afficher en continu sur le LCD le format exact ADC1:X.XXXV. L'affichage doit correspondre à la tension mesurée au multimètre, rester dans la plage réelle Vmin–Vmax, ne pas clignoter et rester stable.
Capacités visées
- Interpréter les spécifications techniques des microcontrôleurs et périphériques.
- Utiliser des fonctionnalités spécifiques d'un microcontrôleur.
- Mettre en œuvre des communications avec des périphériques.
2. Sources de référence
1. TEP_Prog_Lab4.pdf, p. 3 : exigences et signatures des fonctions; p. 4 : plan de test; p. 5 : règles de programmation; p. 6 : remise et barème.
2. notes_Prog3.pdf, p. 3 : carte mémoire; p. 4 : RCC et arbre d'horloges; p. 6 : registres GPIO; p. 9 : schéma de la carte (R1-1 → PA0, LCD → PB9 à PB15); p. 13-14 : accès bare-metal aux registres.
3. Code existant du Lab 3 : main.c, LCD.c/.h, gpio.c/.h, Timer2.c/.h et SysTick.c/.h ont été examinés.
4. RM0008 a été consulté en ligne pour les bits et séquences ADC. La fiche technique du modèle exact monté sur la carte et ses limites électriques restent à confirmer après identification physique de la puce.
3. Matériel et connexions
- [ ] Kit STM32F103 TEP.
- [ ] Multimètre.
- [ ] Analyseur logique, si nécessaire pour le diagnostic des signaux numériques; il ne remplace pas le multimètre pour mesurer PA0.
- [ ] Alimentation et connexion de programmation/débogage habituelles.
- [x] D'après le schéma de la carte, R1-1 est un potentiomètre de 10 kΩ entre 3,3 V et GND, curseur vers PA0, soit ADC1 canal 0.
- [x] Le LCD existant utilise PB9 RS, PB10 RW, PB11 E, PB12–PB15 D4–D7.
- [ ] Confirmer sur la carte physique les points de mesure PA0 et GND et la référence de la puce (les documents mentionnent CBT6 et le laboratoire fait référence à un kit STM32F103).
- [ ] Ne jamais appliquer plus que les limites électriques admissibles sur PA0.
4. Mesures obligatoires avant programmation
1. Mettre la carte sous tension normalement.
2. Brancher le multimètre en tension continue : noir à GND, rouge à PA0 (point réellement connecté au curseur).
3. Tourner R1-1 complètement vers la position de tension minimale; attendre la stabilisation; noter la valeur.
4. Tourner R1-1 complètement vers la position maximale; noter la valeur.
5. Noter la tension d'alimentation analogique ou la référence si accessible, pour comprendre les écarts de conversion.
6. Mesurer ensuite une valeur intermédiaire pour préparer le test de précision.
Mesure	Valeur	État
Vmin au multimètre	À mesurer	[ ]
Vmax au multimètre	À mesurer	[ ]
V à mi-course	À mesurer	[ ]
VDDA / référence réelle, si accessible	À vérifier	[ ]


Important : inscrire Vmin et Vmax comme constantes dans Inc/ADC.h avec un commentaire contenant les valeurs réellement mesurées. Ne pas inventer 0,000 V et 3,300 V.
5. État initial du projet et arborescence
Dossier de travail :
/Users/matisserheaume-viale/Code/TEP-Session-3-2026-2027/
243-356-LI - Programmation 3 - Microcontroleurs et peripheriques/
Laboratoires/Lab_4/
Lab_4/
├── Inc/
│   ├── ADC.h            # créé, à remplir personnellement
│   ├── gpio.h
│   ├── LCD.h
│   ├── SysTick.h
│   └── Timer2.h
├── Src/
│   ├── ADC.c            # créé, à remplir personnellement
│   ├── gpio.c
│   ├── LCD.c
│   ├── main.c
│   ├── SysTick.c
│   └── Timer2.c
├── build/               # produits générés, ne pas éditer
├── startup_stm32f103xb.s
├── stm32f103xb_flash.ld
└── plan.md
Il n'y a pas de CMakeLists.txt dans la copie. L'inspection a confirmé que `build/qc1` et sa sauvegarde pointent vers des sources du Lab 2/3, donc ils ne valident pas le Lab 4. La configuration QC1 provient de l'extension VS Code; une configuration neuve a été créée dans `build/lab4-check` avec les sources, le fichier startup et le linker du Lab 4.
Inventaire et décisions
Fichier	Action prévue	Justification
Src/main.c	Adapter	Actuellement initialisation LCD + horloge TIM2 et affichage du temps.
Src/LCD.c	Réutiliser et nettoyer	Les trois fonctions LCD requises existent; retirer l'affichage de l'heure si devenu inutile.
Inc/LCD.h	Conserver les signatures obligatoires	Supprimer seulement les déclarations devenues inutiles.
Src/ADC.c	Implémenter	Initialisation ADC1, lecture et affichage du potentiomètre.
Inc/ADC.h	Compléter	Garde d'inclusion, prototypes et constantes Vmin/Vmax.
gpio.c/.h	Aucun appel par main/LCD/ADC; laissés dans la copie	QC1 inclut tous les fichiers Src/*.c; le linker élimine les sections non utilisées.
Timer2.c/.h	Aucun appel par main/LCD; fichiers laissés dans la copie	L'horloge du Lab 3 n'est plus utilisée pour l'affichage; le startup conserve le vecteur TIM2_IRQHandler.
SysTick.c/.h	SysTick_GetMs() n'est pas appelé	LCD.c configure directement le compteur SysTick et le startup conserve SysTick_Handler.
startup_*.s, *.ld	Préserver	Nécessaires au démarrage et à l'édition de liens.
build/	Nettoyer/reconstruire avec prudence	Contient des artefacts du Lab 3 et une sauvegarde QC1.


6. Architecture logicielle
main.c
- [x] Garder SystemInit() : HSI à 8 MHz, bus APB2 sans division.
- [x] Initialiser les périphériques une seule fois.
- [x] Initialiser le LCD puis ADC1; cet ordre lance SysTick avant les délais ADC.
- [x] Faire un premier affichage dès le démarrage.
- [x] Utiliser while (1) pour actualiser la lecture et l'affichage.
- [x] Garder la conversion ADC et le formatage hors de main.
- [x] Retirer l'affichage de l'heure et les valeurs de test 23:59:55.
ADC.h
- [x] Garde d'inclusion #ifndef / #define / #endif.
- [x] Types fixes nécessaires (uint16_t, etc.).
- [ ] Remplacer les emplacements VMIN/VMAX par les valeurs mesurées et commenter les mesures.
- [x] Déclarations exactes :
  - void ADC1_Init(void)
  - uint16_t ADC1_Lire(void)
  - void LCD_Affichage_Potentiometre(void)
- [x] Aucune définition de variable ou fonction dans l'en-tête.
ADC.c
- [x] Accès aux registres matériels en volatile.
- [x] Définir les adresses et masques avec des noms explicites.
- [x] Garder les fonctions auxiliaires privées static.
- [x] Séparer l'initialisation, la conversion et le formatage.
- [x] Ne pas utiliser HAL ni bibliothèque de périphériques.
LCD.c/.h
Les signatures doivent rester exactement :
void LCD_Init(void);
void LCD_Commande(bool RW, bool RS, uint8_t Data);
void LCD_Ecrire_Char(unsigned char caractere, uint8_t ligne, uint8_t colonne);
- [x] Les trois fonctions existent dans le code fourni.
- [x] Retirer LCD_Affichage_Temps() et sa dépendance Timer2.h.
- [x] Garder le pilote 4 bits et les temporisations existantes.
- [x] Ne pas appeler LCD_CMD_CLEAR à chaque nouvelle mesure.
- [x] Écrire seulement les positions qui changent; garder l'affichage au format constant.
7. Initialisation ADC1 : étapes à comprendre et réaliser
Broche : PA0 = ADC1_IN0. L'ADC du STM32F103 est un convertisseur SAR de 12 bits, donnant un code de 0 à 4095.
1. RCC : activer l'horloge de GPIOA et d'ADC1 via RCC_APB2ENR.
2. Horloge ADC : choisir ADCPRE dans RCC_CFGR de façon à rester à 14 MHz maximum. Avec la configuration système actuelle à HSI 8 MHz et APB2 sans division, une division ADC par 2 donne 4 MHz; confirmer l'horloge réelle avant d'utiliser cette hypothèse.
3. GPIOA : configurer PA0 en entrée analogique (MODE0=00, CNF0=00 dans GPIOA_CRL), sans pull-up/pull-down numérique.
4. ADC1 : déterminer les champs nécessaires dans ADC1_CR1 et ADC1_CR2 pour la conversion régulière, l'alignement et le déclenchement logiciel.
5. Échantillonnage : configurer ADC1_SMPR2 pour le canal 0. Un temps suffisamment long facilite la mesure stable du potentiomètre; calculer le temps total de conversion en cycles ADC.
6. Séquence : configurer la longueur de séquence régulière dans ADC1_SQR1 (une conversion) et le premier rang dans ADC1_SQR3 (canal 0).
7. Mise sous tension : suivre la séquence exacte du manuel STM32F1 pour activer l'ADC, attendre sa stabilisation et effectuer la calibration (reset de calibration puis calibration, avec attente des bits correspondants).
8. Conversion : configurer correctement la sélection de déclenchement logiciel, lancer la conversion selon la séquence documentée et attendre EOC.
9. Résultat : lire ADC1_DR, ne garder que les 12 bits pertinents si l'alignement est à droite.
10. Robustesse : envisager une stratégie de diagnostic si un bit de fin de conversion ne change jamais; ne pas masquer une mauvaise configuration par un délai arbitraire.
Registres à documenter
Registre	Fonction
RCC_APB2ENR	Horloge GPIOA et ADC1
RCC_CFGR	Prescaler ADC
GPIOA_CRL	PA0 en mode analogique
ADC1_SR	État et fin de conversion EOC
ADC1_CR1	Configuration de conversion
ADC1_CR2	Activation, calibration, déclenchement
ADC1_SMPR2	Durée d'échantillonnage canal 0
ADC1_SQR1	Longueur de séquence régulière
ADC1_SQR3	Canal du premier rang
ADC1_DR	Donnée convertie


Ne pas coder les valeurs de bits de mémoire : les confirmer dans RM0008 et noter leur nom, leur position et la valeur voulue.
Repères vérifiés dans RM0008 et utilisés dans le code : ADCPRE[15:14]=00 sélectionne PCLK2/2; SMP0[2:0]=111 donne 239,5 cycles d'échantillonnage; une conversion prend 239,5 + 12,5 = 252 cycles ADC, soit environ 63 µs à 4 MHz. Dans ADC_CR2, EXTSEL[19:17]=111 et EXTTRIG=1 sélectionnent le déclenchement logiciel; ADON=1 active l'ADC; RSTCAL (bit 3) puis CAL (bit 2) lancent les deux étapes de calibration; SWSTART (bit 22) démarre une lecture. EOC est le bit 1 d'ADC_SR et la lecture d'ADC_DR l'efface.
8. Lecture ADC1
- [x] La fonction ADC1_Lire(void) retourne un uint16_t.
- [x] Déclencher une conversion régulière sur le canal 0.
- [x] Attendre la fin de conversion EOC.
- [x] Lire le registre de données ADC1_DR.
- [x] Retourner un code numérique entre 0 et 4095.
- [ ] Vérifier que la valeur augmente lorsque la tension sur PA0 augmente.
- [x] Vérifier dans le code qu'aucun autre module appelé ne reconfigure ADC1 ou PA0.
9. Conversion numérique → tension
La relation idéale avec référence réelle est :
\[
V_{PA0}\simeq\frac{N\,V_{REF}}{4095},\qquad 0\leq N\leq4095.
\]
Cette formule est une approximation de mise à l'échelle usuelle; la caractéristique exacte du convertisseur et sa précision dépendent notamment de la référence et des erreurs de mesure. Vmin/Vmax sont des limites de la tension réelle du potentiomètre, pas automatiquement les références électriques de l'ADC. Ne pas remplacer VREF par Vmax sans justification.
- [ ] Identifier la tension de référence réellement utilisée (VDDA / VREF+ selon la carte); 3,300 V reste provisoire.
- [ ] Comparer les valeurs calculées avec les mesures du multimètre.
- [ ] Corriger une éventuelle erreur systématique seulement avec une méthode justifiée et expliquée.
- [ ] Activer la borne [Vmin, Vmax] après les mesures réelles; elle reste désactivée tant que ces valeurs sont inconnues.
- [x] Convertir en millivolts entiers pour faciliter l'affichage à trois décimales.
- [x] Arrondir au millivolt le plus proche et utiliser un accumulateur assez large.
- [ ] Si le potentiomètre ne couvre pas 0–3,3 V, conserver ses limites mesurées sans fausser la lecture intermédiaire.
Exemple de vérification mathématique (non lié à la carte mesurée)
Si VREF = 3,300 V et N = 2048, la formule idéale donne environ 1,650 V. Cela ne signifie pas que la carte possède exactement cette référence ou cette valeur à mi-course.
10. Format LCD
Format exigé : ADC1:X.XXXV (exemple : ADC1:3.300V).

Positions sur la première ligne :
Colonne	Caractère
1–5	A, D, C, 1, :
6	chiffre des volts
7	.
8–10	trois chiffres décimaux
11	V


- [x] Utiliser LCD_Ecrire_Char() pour afficher les caractères.
- [x] Afficher un chiffre avant le point et exactement trois après.
- [x] Effacer l'écran une fois à l'initialisation et écrire le texte complet au premier affichage.
- [x] Ne pas effacer tout le LCD à chaque lecture.
- [x] Garder le format fixe ADC1:X.XXXV.
- [x] Ne pas afficher de signe négatif.
- [x] Rafraîchir à 20 Hz et ne réécrire que les caractères qui changent.
11. Précision, filtrage et stabilité
Le plan de test impose :
- Tension minimale : écart de ±0,020 V maximum.
- Tension maximale : écart de ±0,020 V maximum, sans jamais afficher plus que Vmax.
- Mi-course : écart maximal de 0,050 V avec le multimètre.
- Potentiomètre immobile pendant 10 s : variation d'au plus ±0,010 V et aucun clignotement.
- Rotation lente : évolution progressive, sans saut ni blocage.
Pistes à évaluer après avoir validé une conversion simple :
1. Faire plusieurs conversions successives.
2. Calculer une moyenne sur un nombre fixe d'échantillons, avec accumulateur assez large.
3. Évaluer le compromis entre lissage et réactivité lors de la rotation.
4. Ne mettre à jour les chiffres que lorsque la valeur affichée change, si utile.
5. Contrôler la cadence de rafraîchissement.
6. Ne pas masquer une erreur de référence ou de calibration avec un filtre.
7. Vérifier expérimentalement que le filtrage choisi respecte les tests 6 et 7.
12. Plan de réalisation, étape par étape
Phase A — Préparer le projet
- [x] Copier le Lab 3 dans Lab_4.
- [x] Créer Inc/ADC.h et Src/ADC.c.
- [x] Examiner main.c, LCD.c/.h, gpio.c/.h.
- [x] Examiner Timer2.c/.h, SysTick.c/.h et les fichiers de build.
- [x] Vérifier le contrôle de version, les fichiers ignorés et l'état de la copie avant modifications.
- [ ] Faire une sauvegarde ou un commit du point de départ si le dossier est sous Git.
Phase B — Mesures et documentation
- [x] Identifier PA0/ADC1_IN0 dans le schéma du kit.
- [ ] Mesurer Vmin et Vmax.
- [ ] Mesurer ou confirmer VDDA/VREF; le code utilise provisoirement la valeur nominale 3,300 V du schéma.
- [x] Vérifier dans le code les fréquences HSI=8 MHz, PCLK2=8 MHz et ADCCLK=4 MHz.
- [x] Repérer dans RM0008 les registres et séquences ADC.
Phase C — Module ADC
- [x] Compléter ADC.h avec les prototypes et emplacements Vmin/Vmax.
- [x] Configurer les horloges RCC.
- [x] Configurer PA0 en analogique.
- [x] Configurer canal, séquence et temps d'échantillonnage.
- [x] Activer et calibrer ADC1.
- [x] Implémenter ADC1_Lire().
- [ ] Tester les valeurs brutes aux deux extrémités et à mi-course.
Phase D — Affichage
- [x] Adapter le code LCD sans casser les trois fonctions obligatoires.
- [x] Implémenter le calcul de tension et l'arrondi.
- [ ] Appliquer Vmin/Vmax.
- [x] Implémenter LCD_Affichage_Potentiometre().
- [x] Modifier main.c pour initialiser et rafraîchir.
- [x] Retirer l'affichage de l'heure et les appels Timer2 dans main.c et LCD.c.
Phase E — Qualité et essais
- [x] Compiler avec ARM GCC et analyser les avertissements.
- [ ] Compilation sans aucun avertissement : les fichiers C sont silencieux, mais le linker signale les stubs `_close`, `_lseek`, `_read` et `_write` de `nosys.specs`.
- [ ] Flasher la carte.
- [ ] Exécuter les sept tests dans l'ordre, sans modifier ni recompiler entre eux.
- [ ] Vérifier chaque règle de qualité du code.
- [ ] Préparer une explication orale de chaque registre, calcul et fonction.

Résultat du build du 23 octobre 2026 : compilation dédiée dans `build/lab4-check`, configurée avec les sources, le fichier startup et le linker du Lab 4. Les compilations C avec `-Wall -Wextra` ne signalent aucun avertissement. L'édition de liens produit `firmware.elf`, mais émet les avertissements des appels système non implémentés de `nosys.specs`; le programme n'utilise pas d'E/S système. La carte n'a pas été flashée et les mesures réelles restent à faire.
Phase F — Remise
- [ ] Préparer le ZIP Remise_Laboratoire_VOTRENOM.zip avec les fichiers de code demandés.
- [ ] Vérifier que le ZIP contient bien les sources nécessaires, sans artefacts build/ superflus.
- [ ] Faire la démonstration en séance.
- [ ] Déposer sur Omnivox avant le 10 novembre 2026.
13. Plan de test officiel et fiche de résultats
Règle : tests dans cet ordre, sans modification ni recompilation entre les tests; démonstration à l'enseignant.
#	Test	Procédure	Critère de réussite	Points	Résultat
1	Démarrage	Appuyer sur RESET	Affichage immédiat, aucun caractère parasite	5	[ ]
2	Format	Placer le potentiomètre à une position quelconque	Exactement ADC1:X.XXXV	10	[ ]
3	Minimum	Tourner au minimum, mesurer au multimètre	Vmin à ±0,020 V	5	[ ]
4	Maximum	Tourner au maximum, mesurer au multimètre	Vmax à ±0,020 V, jamais au-dessus	5	[ ]
5	Précision	Mesurer à mi-course	Écart affichage/multimètre ≤ 0,050 V	10	[ ]
6	Variation	Aller lentement min → max → min	Variation progressive, sans saut ni blocage	10	[ ]
7	Stabilité	Ne pas bouger pendant 10 s	Pas de clignotement, variation ≤ ±0,010 V	5	[ ]


Feuille de mesures :
Position	Multimètre (V)	ADC brut	LCD (V)	Erreur absolue (V)	Conforme ?
Minimum	—	—	—	—	[ ]
Mi-course	—	—	—	—	[ ]
Maximum	—	—	—	—	[ ]


14. Liste de contrôle qualité (14 règles du document)
- [ ]
  1. Chaque module possède sa paire .c/.h; main.c contient seulement l'initialisation et la boucle principale.
- [ ]
  2. Chaque .h possède une garde d'inclusion.
- [ ]
  3. Les .h contiennent seulement déclarations, types et constantes, aucune définition de fonction ou variable.
- [ ]
  4. Les éléments privés d'un module sont static.
- [ ]
  5. Noms explicites, fonctions publiques préfixées par le module.
- [ ]
  6. Constantes en majuscules (#define ou const).
- [ ]
  7. Aucun nombre magique (broches, masques, champs de registres, conversions).
- [ ]
  8. Indentation uniforme, accolades toujours présentes.
- [ ]
  9. Types de taille fixe et bool pour les données matérielles; pas de int pour registres/données matérielles.
- [ ]
  10. Si ISR utilisée : courte, acquitte le drapeau, met à jour un compteur/drapeau, termine.
- [ ]
  11. Variables partagées ISR / code principal déclarées volatile.
- [ ]
  12. Initialisation une seule fois, puis while (1).
- [ ]
  13. Variables globales limitées aux partages avec ISR; sinon locales ou static.
- [ ]
  14. Compilation sans avertissement, aucun code mort ou commenté.
Attention à la reprise du Lab 3 : main.c utilise actuellement unsigned int pour des compteurs et LCD.c contient un affichage de temps; revoir ces éléments pour éviter les violations ou le code inutile.
15. Risques techniques et prévention
Risque	Symptôme	Vérification / action
Mauvais canal	Lecture fixe ou incohérente	Vérifier PA0 et SQR3 canal 0
GPIO non analogique	Bruit ou lecture anormale	Vérifier GPIOA_CRL
Horloge ADC absente	ADC inactif	Vérifier RCC_APB2ENR
Horloge ADC trop élevée	Mesures incorrectes	Vérifier ADCPRE, PCLK2 et limite 14 MHz
Séquence calibration incorrecte	Erreur de mesure	Suivre RM0008
Mauvais déclenchement	Attente infinie de EOC	Vérifier configuration du déclenchement logiciel
Mauvaise référence	Erreur systématique en volts	Mesurer/identifier VDDA et VREF
Mauvaise échelle Vmin/Vmax	Extrémités correctes mais milieu faux	Ne pas confondre limites du potentiomètre et référence ADC
Arrondi ou dépassement	Valeur LCD fausse	Vérifier arithmétique et bornes
Trop de rafraîchissements	LCD clignote / valeurs instables	Éviter clear répété, limiter cadence, moyenner si nécessaire
Dépendance héritée Lab 3	Erreur de compilation	Retirer proprement les appels/liaisons Timer2 inutiles
Build obsolète	Ancien firmware flashé	Recompiler à partir des sources du Lab 4 et vérifier le binaire utilisé


16. Commandes utiles (inspection seulement)
Depuis le dossier Lab_4 :
pwd
find Inc Src -maxdepth 2 -type f | sort
cat Src/main.c
cat Inc/LCD.h
cat Inc/ADC.h
cat Src/ADC.c
cat Src/Timer2.c Inc/Timer2.h
cat Src/SysTick.c Inc/SysTick.h
find . -maxdepth 3 \( -name CMakeLists.txt -o -name Makefile -o -name '*.cmake' -o -name compile_commands.json \) -print
find . -maxdepth 2 -name '.gitignore' -o -name '.git'
Compilation/flash : reprendre la commande et le système de build effectivement utilisés dans le Lab 3 après inspection. Ne pas supposer que cmake ou QC1 utilise automatiquement ADC.c sans vérification.
17. Barème et remise
Catégorie	Points
Démonstration (7 tests)	50
Respect des exigences	25
Qualité du code	25
Total	100


- Le laboratoire représente 10 % de la note finale.
- Chaque exigence manquante ou incomplète : −5 points.
- Chaque non-respect du standard de qualité : −3 points.
- Chaque mauvaise réponse aux questions de compréhension : −5 points sur le résultat final.
- Retard : 10 % par jour, fins de semaine incluses.
- Travail individuel, démonstration en séance, remise ZIP sur Omnivox.
18. Questions de compréhension à préparer
- Pourquoi faut-il activer l'horloge de GPIOA et d'ADC1 ?
- Pourquoi PA0 doit-il être en mode analogique ?
- Quel canal ADC correspond à PA0 ?
- Que signifie 12 bits et pourquoi la valeur maximale est-elle 4095 ?
- Quelle est la différence entre temps d'échantillonnage et temps de conversion ?
- Pourquoi l'ADC nécessite-t-il une calibration ?
- Quel rôle jouent ADC1_SR, ADC1_CR2 et ADC1_DR ?
- Comment convertir le résultat brut en volts ?
- Pourquoi Vmin/Vmax ne sont-ils pas nécessairement 0 V et 3,3 V ?
- Pourquoi Vmax n'est-il pas forcément égal à VREF ?
- Comment obtenir exactement trois décimales sans clignotement ?
- Comment vérifier la précision au multimètre ?
- Quel est l'effet d'une moyenne sur la stabilité et la réactivité ?
- Pourquoi garder le code en modules séparés ?
19. Critères de fin de travail
Le Lab 4 est terminé seulement lorsque :
- [ ] Toutes les fonctions et signatures exigées sont présentes.
- [ ] Les mesures réelles Vmin et Vmax figurent dans ADC.h.
- [ ] Le LCD affiche ADC1:X.XXXV dès le démarrage.
- [ ] Les 7 tests officiels sont réussis dans l'ordre sans recompiler.
- [ ] Les 14 règles de programmation sont respectées.
- [ ] La compilation est sans avertissement.
- [ ] Le fonctionnement est compris et explicable sans aide.
- [ ] Le ZIP demandé est vérifié et remis avant l'échéance.
