/* ================================================================
   jeu.c — PARTIE 1 / 2 — Mathieu
   Contient les includes, les declarations, les fonctions
   utilitaires et l'initialisation du niveau.
   La boucle de jeu est dans la PARTIE 2 (Mathis).
   ================================================================ */


/* --- INCLUDES --- */

/* jeu.h : notre propre fichier d'en-tete.
   Il inclut entites.h (les structs) et sons.h (les sons).
   Sans ca le compilateur ne connait pas Joueur, Ennemi, Tir, etc. */
#include "jeu.h"

/* allegro_primitives.h : permet de dessiner des formes geometriques.
   On en a besoin pour al_draw_filled_rectangle() dans hud() et transition(). */
#include <allegro5/allegro_primitives.h>

/* allegro_font.h : permet d'afficher du texte a l'ecran avec al_draw_text(). */
#include <allegro5/allegro_font.h>

/* allegro_ttf.h : permet de charger des polices .ttf avec al_load_ttf_font(). */
#include <allegro5/allegro_ttf.h>

/* math.h : bibliotheque mathematique standard.
   On en a besoin pour sqrt() dans le calcul de la visee des ennemis (partie 2). */
#include <math.h>

/* stdio.h : bibliotheque standard d'entrees/sorties.
   On en a besoin pour sprintf() qui formate le texte du HUD. */
#include <stdio.h>


/* ================================================================
   DECLARATIONS EXTERNES
   Ces fonctions sont definies dans d'autres fichiers .c
   mais utilisees dans ce fichier.
   On doit les declarer ici pour que le compilateur sache
   qu'elles existent avant qu'on les appelle.
   ================================================================ */

/* maj_joueur : definie dans joueur.c.
   Lit le clavier et deplace le joueur a chaque frame. */
void maj_joueur(Joueur *j);

/* dessiner_joueur : definie dans joueur.c.
   Dessine le sprite du joueur, gere le clignotement si invincible. */
void dessiner_joueur(Joueur *j);

/* respawn_joueur : definie dans joueur.c.
   Replace le joueur a sa position de depart apres une mort
   et active l'invincibilite pendant 180 frames. */
void respawn_joueur(Joueur *j);

/* col_tirs_ennemis : definie dans collision.c.
   Verifie si un tir joueur touche un ennemi.
   tj[] = tirs joueur, en[] = ennemis, ex[] = explosions,
   j = joueur (pour ajouter des points), i = image explosion. */
void col_tirs_ennemis(Tir tj[], Ennemi en[], Explosion ex[], Joueur *j, ALLEGRO_BITMAP *i);

/* col_tirs_boss : definie dans collision.c.
   Verifie si un tir joueur touche le boss. */
void col_tirs_boss(Tir tj[], Boss *b, Explosion ex[], Joueur *j, ALLEGRO_BITMAP *i);

/* col_tirs_joueur : definie dans collision.c.
   Verifie si un tir ennemi touche le joueur.
   Ignoree si le joueur est invincible. */
void col_tirs_joueur(Tir te[], Joueur *j, Explosion ex[], ALLEGRO_BITMAP *i);

/* col_joueur_ennemis : definie dans collision.c.
   Verifie si le vaisseau entre en contact direct avec un ennemi. */
void col_joueur_ennemis(Joueur *j, Ennemi en[], Explosion ex[], ALLEGRO_BITMAP *i);

/* col_joueur_boss : definie dans collision.c.
   Verifie si le vaisseau entre en contact direct avec le boss.
   Seulement si boss.apparu == true. */
void col_joueur_boss(Joueur *j, Boss *b, Explosion ex[], ALLEGRO_BITMAP *i);

/* extern float V[3][8][6][5] : tableau defini dans niveaux.c.
   "extern" = "cette variable existe ailleurs, pas ici".
   C'est le tableau 4D de toutes les vagues :
   V[niveau][vague][ennemi][propriete]
   3 niveaux, 8 vagues, 6 ennemis, 5 proprietes (x,y,fixe,pv,cadence). */
extern float V[3][8][6][5];

/* spawn : definie dans niveaux.c.
   Place un ennemi dans le premier slot libre du tableau en[].
   iv = image ennemi volant, it = image tourelle fixe. */
void spawn(Ennemi en[], float x, float y, bool fixe, int pv, int cad,
           ALLEGRO_BITMAP *iv, ALLEGRO_BITMAP *it);


/* ================================================================
   FONCTION charger()
   Charge une image depuis le disque.
   Si le fichier est manquant, cree un rectangle colore a la place.
   Cela evite un crash si un asset est absent.
   ================================================================ */

/* ALLEGRO_BITMAP * = retourne un pointeur vers une image Allegro.
   const char *f = nom du fichier (ex: "player.png").
   int w, int h = taille du rectangle de secours si fichier manquant.
   ALLEGRO_COLOR c = couleur du rectangle de secours. */
ALLEGRO_BITMAP *charger(const char *f, int w, int h, ALLEGRO_COLOR c) {

    /* al_load_bitmap(f) : essaie de charger l'image depuis le disque.
       Si reussi : b pointe vers l'image chargee.
       Si echoue (fichier manquant) : b vaut NULL. */
    ALLEGRO_BITMAP *b = al_load_bitmap(f);

    /* Si b n'est pas NULL : fichier charge avec succes, on le retourne. */
    if (b) return b;

    /* On arrive ici seulement si le fichier est manquant.
       al_create_bitmap(w, h) : cree un bitmap vide de taille w x h en memoire. */
    b = al_create_bitmap(w, h);

    /* al_set_target_bitmap(b) : dit a Allegro que les prochains dessins
       s'appliquent sur ce bitmap et non sur l'ecran. */
    al_set_target_bitmap(b);

    /* al_clear_to_color(c) : remplit tout le bitmap avec la couleur c.
       C'est ce qui cree le rectangle colore de remplacement. */
    al_clear_to_color(c);

    /* al_set_target_backbuffer(...) : remet la cible de dessin sur l'ecran.
       IMPORTANT : sans ca tout ce qu'on dessine ensuite va dans le bitmap
       et pas a l'ecran. */
    al_set_target_backbuffer(al_get_current_display());

    /* Retourne le bitmap de remplacement. */
    return b;
}


/* ================================================================
   FONCTION tirer()
   Place 2 tirs cote a cote dans le tableau t[].
   On cherche 2 slots libres (actif == false) et on les active.
   ================================================================ */

/* Tir t[] = tableau des tirs joueur (NB_TIRS = 20 slots).
   float x, y = position de depart des tirs (bout du vaisseau).
   ALLEGRO_BITMAP *i = image du projectile. */
void tirer(Tir t[], float x, float y, ALLEGRO_BITMAP *i) {

    /* ok = compteur de tirs places. On veut en placer exactement 2. */
    int ok = 0;

    /* Parcourt le tableau. La condition ok < 2 arrete la boucle
       des qu'on a place nos 2 tirs, sans parcourir tout le tableau inutilement. */
    for (int j = 0; j < NB_TIRS && ok < 2; j++) {

        /* Si ce slot est libre (actif == false) : on peut y mettre un tir. */
        if (!t[j].actif) {

            /* Initialise le tir avec une syntaxe de struct litterale :
               x         = position X (bout du vaisseau)
               y + ok*60 = position Y :
                           ok=0 -> y+0  = tir du haut
                           ok=1 -> y+60 = tir du bas (60 pixels en dessous)
               55        = vx : vitesse horizontale en pixels/frame vers la droite
               0         = vy : pas de deplacement vertical
               true      = actif : ce slot est maintenant occupe
               i         = image du projectile */
            t[j] = (Tir){x, y + ok * 60, 55, 0, true, i};

            /* On a place un tir de plus. Quand ok = 2 la boucle s'arrete. */
            ok++;
        }
    }
}


/* ================================================================
   FONCTION tir_e()
   Place un seul tir ennemi dans le premier slot libre.
   ================================================================ */

/* vx, vy = direction et vitesse du tir ennemi.
   Ces valeurs sont calculees par normalisation dans la partie 2. */
void tir_e(Tir t[], float x, float y, float vx, float vy, ALLEGRO_BITMAP *i) {

    /* Parcourt les 40 slots de tirs ennemis. */
    for (int j = 0; j < NB_TIRS_ENE; j++) {

        /* Premier slot libre trouve : on y place le tir. */
        if (!t[j].actif) {

            /* x, y  = position de depart (position de l'ennemi qui tire).
               vx,vy = direction normalisee * vitesse (calcule dans partie 2).
               true  = actif.
               i     = image du projectile ennemi. */
            t[j] = (Tir){x, y, vx, vy, true, i};

            /* return : on sort immediatement. Un seul tir par appel. */
            return;
        }
    }
}


/* ================================================================
   FONCTION hud()
   Affiche le HUD (Head-Up Display) a chaque frame :
   vies, score, et barre de progression verte.
   ================================================================ */

/* j = pointeur vers le joueur pour lire vies et score.
   f = police d'ecriture.
   vague = numero de la vague actuelle (0 a 8).
   boss = true si on est en phase boss. */
void hud(Joueur *j, ALLEGRO_FONT *f, int vague, bool boss) {

    /* buf[64] : tableau de 64 caracteres pour le texte du HUD.
       Largement suffisant pour "VIES: 5    SCORE: 001500". */
    char buf[64];

    /* sprintf formate le texte dans buf :
       %d   = affiche un entier (j->vies).
       %06d = affiche sur 6 chiffres avec zeros devant (j->score).
              ex: 150 devient "000150". */
    sprintf(buf, "VIES: %d    SCORE: %06d", j->vies, j->score);

    /* Affiche buf en blanc, 40px depuis la gauche, 100px depuis le bas. */
    al_draw_text(f, al_map_rgb(255, 255, 255), 40, HAUTEUR - 100, 0, buf);

    /* Calcul de la progression entre 0.0 et 1.0 :
       Si on est en phase boss -> 1.0 (barre pleine).
       Sinon -> vague / 8 (ex: vague 4 -> 0.5 -> 50%).
       (float) force la division en float sinon elle serait entiere. */
    float p = boss ? 1.0f : (float)vague / 8.0f;

    /* Fond bleu-gris de la barre (600px de large, 30px de haut). */
    al_draw_filled_rectangle(LARGEUR-700, HAUTEUR-90, LARGEUR-100, HAUTEUR-60,
                             al_map_rgb(40, 40, 80));

    /* Partie verte dont la largeur = 600 * p pixels.
       Quand p = 1.0 elle recouvre entierement le fond bleu-gris. */
    al_draw_filled_rectangle(LARGEUR-700, HAUTEUR-90,
                             LARGEUR-700 + (int)(600 * p), HAUTEUR-60,
                             al_map_rgb(50, 200, 100));

    /* Contour blanc de 3 pixels d'epaisseur autour de la barre. */
    al_draw_rectangle(LARGEUR-700, HAUTEUR-90, LARGEUR-100, HAUTEUR-60,
                      al_map_rgb(255, 255, 255), 3);
}


/* ================================================================
   FONCTION transition()
   Animation de passage entre deux niveaux :
   fondu vers le noir -> texte "NIVEAU X" -> fondu depuis le noir.
   ================================================================ */

/* f = police d'ecriture. n = numero du niveau suivant a afficher. */
void transition(ALLEGRO_FONT *f, int n) {

    /* msg contiendra "NIVEAU 2" ou "NIVEAU 3". */
    char msg[32];
    sprintf(msg, "NIVEAU %d", n);

    /* ETAPE 1 : fondu vers le noir en 60 frames.
       a va de 0 a 60, l'opacite du rectangle noir augmente progressivement :
       a=0  -> opacite 0   (transparent, on voit encore le jeu)
       a=30 -> opacite 127 (semi-transparent)
       a=60 -> opacite 255 (ecran completement noir) */
    for (int a = 0; a <= 60; a++) {
        al_draw_filled_rectangle(0, 0, LARGEUR, HAUTEUR,
                                 al_map_rgba(0, 0, 0, (a * 255) / 60));
        al_flip_display();    /* envoie le rendu a l'ecran */
        al_rest(1.0 / 60);   /* pause d'une frame pour garder 60fps */
    }

    /* ETAPE 2 : affiche le texte "NIVEAU X" pendant 2 secondes. */
    al_clear_to_color(al_map_rgb(0, 0, 0));   /* efface en noir */
    al_draw_text(f, al_map_rgb(0, 200, 255),
                 LARGEUR/2, HAUTEUR/2 - 80,
                 ALLEGRO_ALIGN_CENTRE, msg);  /* titre en bleu cyan */
    al_draw_text(f, al_map_rgb(200, 200, 200),
                 LARGEUR/2, HAUTEUR/2 + 60,
                 ALLEGRO_ALIGN_CENTRE, "Preparez-vous !"); /* sous-titre gris */
    al_flip_display();
    al_rest(2.0);   /* pause de 2 secondes */

    /* ETAPE 3 : fondu depuis le noir en 60 frames.
       a va de 60 a 0, l'opacite diminue progressivement.
       On redessine le texte a chaque frame sinon le voile noir
       couvrirait un ecran vide. */
    for (int a = 60; a >= 0; a--) {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_text(f, al_map_rgb(0, 200, 255),
                     LARGEUR/2, HAUTEUR/2 - 80,
                     ALLEGRO_ALIGN_CENTRE, msg);
        al_draw_filled_rectangle(0, 0, LARGEUR, HAUTEUR,
                                 al_map_rgba(0, 0, 0, (a * 255) / 60));
        al_flip_display();
        al_rest(1.0 / 60);
    }
}


/* ================================================================
   DEBUT DE lancer_niveau() — PARTIE MATHIEU
   Charge toutes les images, initialise toutes les variables.
   La boucle de jeu est dans la PARTIE 2 (Mathis).
   ================================================================ */

/* int = retourne VICTOIRE, GAME_OVER ou MENU a la fin.
   ALLEGRO_DISPLAY *d = pointeur vers la fenetre du jeu.
   int num = numero du niveau (1, 2 ou 3).
   Joueur *j = pointeur vers le joueur (persiste entre les niveaux).
   Sons *s = pointeur vers les sons charges dans main.c. */
int lancer_niveau(ALLEGRO_DISPLAY *d, int num, Joueur *j, Sons *s) {

    /* Charge le fond selon le niveau.
       Operateur ternaire imbrique : choisit le bon fichier selon num.
       Couleur de secours : bleu nuit si le fichier est manquant. */
    ALLEGRO_BITMAP *i_fond = charger(
        num==1 ? "background1.png" : num==2 ? "background2.png" : "background3.png",
        LARGEUR, HAUTEUR, al_map_rgb(0, 20, 60));

    /* Chargement de tous les sprites.
       Le 2eme et 3eme argument = taille du rectangle de secours.
       Le 4eme argument = couleur si fichier manquant. */
    ALLEGRO_BITMAP *i_j  = charger("player.png",       200, 160, al_map_rgb(0,   150, 255));
    ALLEGRO_BITMAP *i_e  = charger("enemy.png",        160, 160, al_map_rgb(255, 100,   0));
    ALLEGRO_BITMAP *i_t  = charger("enemy_turret.png", 160, 160, al_map_rgb(200,  50,  50));
    ALLEGRO_BITMAP *i_b  = charger("boss.png",         400, 400, al_map_rgb(150,   0, 200));
    ALLEGRO_BITMAP *i_tj = charger("bullet.png",        60,  20, al_map_rgb(255, 220,  50));
    ALLEGRO_BITMAP *i_te = charger("enemy_bullet.png",  50,  20, al_map_rgb(255,  60,  60));
    ALLEGRO_BITMAP *i_ex = charger("explosion.png",    250, 250, al_map_rgb(255, 140,   0));

    /* Charge la police en taille 60px.
       Si font.ttf est absent : utilise la police par defaut d'Allegro. */
    ALLEGRO_FONT *font = al_load_ttf_font("font.ttf", 60, 0);
    if (!font) font = al_create_builtin_font();

    /* j->img = i_j : assigne le sprite joueur charge ci-dessus au joueur.
       -> signifie qu'on accede au champ d'une struct via un pointeur. */
    j->img = i_j;

    /* Tableaux statiques : alloues sur la pile, pas de malloc.
       = {0} initialise tous les champs a zero (actif = false pour tous).
       20 slots pour les tirs joueur, 40 pour les tirs ennemis. */
    Tir tj[NB_TIRS] = {0}, te[NB_TIRS_ENE] = {0};

    /* 12 slots pour les ennemis, 16 pour les explosions. */
    Ennemi    en[NB_ENNEMIS] = {0};
    Explosion ex[NB_EXPS]    = {0};

    /* Initialisation du boss avec une struct litterale :
       LARGEUR+100   = x : demarre hors ecran a droite (invisible)
       HAUTEUR/2-200 = y : centre vertical moins 200px
       PV_BOSS       = pv : 30 points de vie (defini dans globals.h)
       90            = timer_tir : 90 frames entre chaque tir du boss
       true          = actif : le boss existe
       false         = apparu : pas encore visible, entre depuis la droite
       i_b           = img : sprite du boss */
    Boss boss = {LARGEUR+100, HAUTEUR/2-200, PV_BOSS, 90, true, false, i_b};

    /* fx = position X du fond pour le scrolling.
       Commence a 0, decremente de 3 par frame dans la partie 2. */
    float fx = 0;

    /* vague = numero de la vague en cours (0 a 7). */
    int vague = 0;

    /* tv = timer avant la prochaine vague.
       200 frames = ~3.3 secondes avant les premiers ennemis. */
    int tv = 200;

    /* cd = cooldown du tir joueur.
       Mis a 8 apres chaque tir, empeche de retirer pendant 8 frames. */
    int cd = 0;

    /* res = resultat du niveau. Initialise a VICTOIRE par defaut.
       Sera change en GAME_OVER ou MENU selon ce qui se passe. */
    int res = VICTOIRE;

    /* pboss = phase boss. false = vagues normales. true = boss actif. */
    bool pboss = false;

    /* pause = true quand le joueur appuie sur P ou ECHAP. */
    bool pause = false;

    /* fini = false tant qu'on reste dans la boucle de jeu.
       Mis a true pour sortir du while en cas de victoire/mort/menu. */
    bool fini = false;

    /* Lance la musique du niveau.
       num-1 convertit num (1,2,3) en indice de tableau (0,1,2).
       jouer_musique() coupe le son precedent avant de jouer le nouveau. */
    jouer_musique(s, s->niveaux[num - 1]);

    /* Cree la file d'evenements : recoit clavier, timer, fenetre. */
    ALLEGRO_EVENT_QUEUE *q = al_create_event_queue();

    /* Cree un timer qui envoie un evenement toutes les 1/60 secondes = 60fps. */
    ALLEGRO_TIMER *tmr = al_create_timer(1.0 / 60.0);

    /* Abonne la file aux evenements de fermeture de fenetre (croix, Alt+F4). */
    al_register_event_source(q, al_get_display_event_source(d));

    /* Abonne la file aux ticks du timer (60 fois par seconde). */
    al_register_event_source(q, al_get_timer_event_source(tmr));

    /* Abonne la file aux touches du clavier (KEY_DOWN etc.). */
    al_register_event_source(q, al_get_keyboard_event_source());

    /* Demarre le timer : il commence a envoyer des evenements.
       Sans ca le timer est cree mais inactif. */
    al_start_timer(tmr);

    /* ---- LA SUITE EST DANS LA PARTIE 2 (Mathis) ---- */
}
