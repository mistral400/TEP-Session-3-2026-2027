md
# PLAN.md

## But

Faire afficher l'heure sur le LCD en format :

HH:MM:SS

Le temps est mis à jour avec une interruption du Timer 2.

Les foncitons a faires
void LCD_Init(void);
void LCD_Commande(bool RW, bool RS, uint8_t Data);
void LCD_Ecrire_Char(unsigned char caractere, uint8_t ligne, uint8_t colonne);
void LCD_Affichage_Temps(void);


---

## Connexions LCD

+--------+-------+
| LCD    | F103  |
+--------+-------+
| RS     | PB9   |
| R/W    | PB10  |
| DB4    | PB12  |
| DB5    | PB13  |
| DB6    | PB14  |
| DB7    | PB15  |
+--------+-------+

Le LCD est branché en 4 bits, donc il reçoit seulement `DB4 à DB7`.

---

## LCD_Commande()

void LCD_Commande(bool RW, bool RS, uint8_t Data);

`Data` contient 8 bits.

Comme le LCD est branché en 4 bits, il faut envoyer `Data` en 2 nibbles :

Data = 0x28

0010 1000
^^^^ ^^^^
haut bas


Ordre :

1. Mettre RS
2. Mettre R/W
3. Envoyer le nibble haut sur DB7 à DB4
4. Faire une impulsion sur Enable
5. Envoyer le nibble bas sur DB7 à DB4
6. Faire une impulsion sur Enable



bit 3 -> DB7 -> PB15
bit 2 -> DB6 -> PB14
bit 1 -> DB5 -> PB13
bit 0 -> DB4 -> PB12


---

## LCD_Init()

Au power on, le LCD attend encore du 8 bits.

Il faut donc faire la séquence spéciale pour le mettre en mode 4 bits.


Power ON
   |
15 ms
   |
0011
   |
4.5 ms
   |
0011
   |
1 ms
   |
0011
   |
0010
   |
Mode 4 bits


Les 4 premiers envois sont seulement des nibbles.

### Premier envoi

+--------+-----+------+-----+-----+-----+-----+
| Signal | RS  | R/W  | DB7 | DB6 | DB5 | DB4 |
+--------+-----+------+-----+-----+-----+-----+
| Valeur |  0  |  0   |  0  |  0  |  1  |  1  |
| F103   | PB9 | PB10 | PB15| PB14| PB13| PB12|
+--------+-----+------+-----+-----+-----+-----+

Attendre `4.5 ms`.

### Deuxième envoi

0011


Attendre `1 ms`.

### Troisième envoi

0011

### Passage en 4 bits

text
0010


Après ça, on peut utiliser `LCD_Commande()` normalement avec des commandes de 8 bits.

Exemple :


LCD_Commande(false, false, 0x28); // 4 bits, 2 lignes, 5x8
LCD_Commande(false, false, 0x08); // écran OFF
LCD_Commande(false, false, 0x01); // clear
LCD_Commande(false, false, 0x06); // curseur avance
LCD_Commande(false, false, 0x0C); // écran ON


---

## LCD_Ecrire_Char()


void LCD_Ecrire_Char(unsigned char caractere,
                     uint8_t ligne,
                     uint8_t colonne);


But :

text
1. Trouver l'adresse de la ligne et colonne
2. Placer le curseur
3. Envoyer le caractère


Adresse de base :

text
Ligne 1 -> 0x00
Ligne 2 -> 0x40


Pour écrire un caractère :

text
RS = 1
RW = 0


---

## LCD_Affichage_Temps()


void LCD_Affichage_Temps(void);


Doit afficher :

text
HH:MM:SS


Exemple :

text
13:42:07


Il faut convertir chaque chiffre en caractère ASCII.

Exemple :


'0' + (heures / 10)
'0' + (heures % 10)


Même logique pour les minutes et secondes.

---

## Timer 2

L'interruption du Timer 2 sert à mettre à jour le temps.

text
secondes++

si secondes == 60
    secondes = 0
    minutes++

si minutes == 60
    minutes = 0
    heures++

si heures == 24
    heures = 0


Ensuite :


LCD_Affichage_Temps();


affiche la nouvelle heure.
