# 🌊 Ocean Type — Shoot'em Up en C avec Allegro 5

> *2157. L'Empire Bydo envahit les profondeurs. Vous êtes le dernier rempart de l'océan.*

Projet ING1 — ECE Bordeaux — 2025/2026

---

## 🎮 Présentation

**Ocean Type** est un shoot'em up à scrolling horizontal inspiré du classique **R-Type (1987)**, développé en **C** avec la bibliothèque graphique **Allegro 5**.

Le joueur pilote un vaisseau sous-marin et doit survivre à des vagues d'ennemis de plus en plus difficiles sur **3 niveaux**, avant d'affronter un **boss final** au niveau 3.

---

## 👥 Équipe

| Membre | Rôle |
|---|---|
| **Mathieu** | `jeu.c` (1ère moitié) · `fin.c` · `joueur.c` — Boucle de jeu, écran de fin, déplacement joueur |
| **Mathis** | `jeu.c` (2ème moitié) · `main.c` · `entites.h` — Rendu, boss, initialisation, structures |
| **Yanis** | `collision.c` · `menu.c/h` — Collisions AABB, menu principal, navigation |
| **Ali** | `niveaux.c` · `sons.c/h` · `entites.h` · `globals.h` — Vagues ennemis, sons, constantes |

---

## 🚀 Fonctionnalités

- ✅ Menu principal avec 2 onglets (Jouer / Commandes)
- ✅ Accès direct aux niveaux 1, 2 et 3
- ✅ 3 niveaux avec 8 vagues d'ennemis chacun
- ✅ 2 types d'ennemis : volant et tourelle fixe
- ✅ Tirs ennemis qui visent le joueur (normalisation vectorielle)
- ✅ Double munition à chaque tir
- ✅ Boss final au niveau 3 avec barre de vie
- ✅ 5 vies + respawn + invincibilité 180 frames
- ✅ Explosions visuelles
- ✅ Scrolling horizontal du décor
- ✅ Barre de progression du niveau
- ✅ Score affiché en temps réel
- ✅ Pause à tout moment
- ✅ Musiques par niveau + son du tir
- ✅ Écran Game Over et Victoire thème océan
- ✅ Résolution 4K (3840×2152)

---

## 🕹️ Contrôles

| Touche | Action |
|---|---|
| `↑ ↓ ← →` | Déplacer le vaisseau |
| `SPACE` | Tirer (double munition) |
| `P` ou `ECHAP` | Pause |
| `R` | Reprendre (depuis la pause) |
| `M` | Retour au menu (depuis la pause) |
| `ENTREE` | Valider dans le menu |
| `TAB` | Changer d'onglet dans le menu |

---

## 🗂️ Architecture du code

```
ShootEmUp/
├── globals.h       # Constantes partagées (tailles, limites)
├── entites.h       # Toutes les structs (Joueur, Ennemi, Tir…)
├── sons.h / .c     # Chargement et lecture des sons
├── joueur.c        # Déplacement, clignotement, respawn
├── collision.c     # 5 fonctions de détection AABB
├── jeu.h / .c      # Boucle principale, vagues, boss
├── menu.c          # Menu 2 onglets
├── fin.c           # Écran Game Over / Victoire
├── main.c          # Initialisation Allegro + lancement
└── Makefile
```

---

## ⚙️ Compilation

### Prérequis

- GCC
- [Allegro 5](https://liballeg.org/) installé

### Linux / macOS

```bash
make
./ShootEmUp
```

### Windows (MinGW / MSYS2)

```bash
make
./ShootEmUp.exe
```

---

## 🖼️ Assets requis

Placer ces fichiers dans le même dossier que l'exécutable :

**Images**
```
player.png          (200×160)
enemy.png           (160×160)
enemy_turret.png    (160×160)
boss.png            (400×400)
bullet.png          (60×20)
enemy_bullet.png    (50×20)
explosion.png       (250×250, fond transparent)
background1.png     (3840×2152)
background2.png     (3840×2152)
background3.png     (3840×2152)
font.ttf
```

**Sons** *(optionnels — le jeu fonctionne sans)*
```
snd_menu.wav
snd_niveau1.wav
snd_niveau2.wav
snd_niveau3.wav
snd_tir.wav
```

> ⚠️ Les fichiers WAV doivent être en format **PCM 16 bits, 44100 Hz** pour être lus par Allegro 5.

---

## 🔧 Détails techniques

### Collisions AABB

Chaque entité est entourée d'un rectangle invisible. La collision est détectée si les deux rectangles se chevauchent sur X **et** Y simultanément.

```c
bool collision(ax, ay, aw, ah, bx, by, bw, bh) {
    return ax < bx+bw && ax+aw > bx && ay < by+bh && ay+ah > by;
}
```

5 fonctions appelées dans l'ordre à chaque frame : tirs joueur → ennemis, tirs joueur → boss, tirs ennemis → joueur, contact joueur/ennemi, contact joueur/boss.

### Tableaux statiques

Aucun `malloc()` dans le code — tous les tableaux (tirs, ennemis, explosions) sont alloués sur la pile. **Zéro fuite mémoire possible.**

### Sons

Deux fonctions distinctes :
- `jouer_musique()` — coupe le son précédent avant d'en jouer un nouveau
- `jouer_effet()` — joue un son court sans couper la musique en cours

---

## 📄 Licence

Projet académique — ECE Paris ING1 2025/2026.
Les assets graphiques et sonores sont issus de sources libres de droits.
