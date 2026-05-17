/*
 * ================================================================
 *  jeu.c  —  PARTIE 1 / 2   (Mathieu)
 *  Contient :
 *    - Chargement des images
 *    - Fonctions utilitaires (tirer, HUD, transition)
 *    - Initialisation de toutes les entites du niveau
 *  La boucle de jeu est dans la PARTIE 2 (Mathis)
 * ================================================================
 */

#include "jeu.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <math.h>
#include <stdio.h>

/* --- Declarations des fonctions definies dans d'autres fichiers --- */
void maj_joueur(Joueur *j);
void dessiner_joueur(Joueur *j);
void respawn_joueur(Joueur *j);
void col_tirs_ennemis(Tir tj[], Ennemi en[], Explosion ex[], Joueur *j, ALLEGRO_BITMAP *i);
void col_tirs_boss(Tir tj[], Boss *b, Explosion ex[], Joueur *j, ALLEGRO_BITMAP *i);
void col_tirs_joueur(Tir te[], Joueur *j, Explosion ex[], ALLEGRO_BITMAP *i);
void col_joueur_ennemis(Joueur *j, Ennemi en[], Explosion ex[], ALLEGRO_BITMAP *i);
void col_joueur_boss(Joueur *j, Boss *b, Explosion ex[], ALLEGRO_BITMAP *i);
extern float V[3][8][6][5]; /* tableau des vagues defini dans niveaux.c */
void spawn(Ennemi en[], float x, float y, bool fixe, int pv, int cad,
           ALLEGRO_BITMAP *iv, ALLEGRO_BITMAP *it);

/*
 * charger() — charge une image PNG depuis le disque.
 * Si le fichier n'existe pas, cree un rectangle de couleur c
 * de taille w x h comme image de remplacement.
 * Cela evite un crash si un asset est manquant.
 */
static ALLEGRO_BITMAP *charger(const char *f, int w, int h, ALLEGRO_COLOR c) {
    ALLEGRO_BITMAP *b = al_load_bitmap(f);
    if (b) return b; /* fichier trouve : on retourne l'image */
    /* fichier manquant : on cree un rectangle colore a la place */
    b = al_create_bitmap(w, h);
    al_set_target_bitmap(b);
    al_clear_to_color(c);
    al_set_target_backbuffer(al_get_current_display());
    return b;
}

/*
 * tirer() — place 2 tirs cote a cote dans le tableau tj[].
 * On cherche 2 slots libres (actif == false) et on les active.
 * Le 2eme tir est decale de 60px vers le bas pour l'effet double.
 * Si tous les slots sont pris, on ne tire pas (tableau plein).
 */
static void tirer(Tir t[], float x, float y, ALLEGRO_BITMAP *i) {
    int ok = 0; /* compte le nombre de tirs places */
    for (int j = 0; j < NB_TIRS && ok < 2; j++) {
        if (!t[j].actif) {
            t[j] = (Tir){x, y + ok * 60, 55, 0, true, i};
            ok++;
        }
    }
}

/*
 * tir_e() — place un tir ennemi dans le premier slot libre.
 * vx et vy definissent la direction et la vitesse du tir.
 * Ces valeurs sont calculees a partir de la normalisation
 * du vecteur ennemi -> joueur dans la boucle de jeu.
 */
static void tir_e(Tir t[], float x, float y, float vx, float vy,
                   ALLEGRO_BITMAP *i) {
    for (int j = 0; j < NB_TIRS_ENE; j++) {
        if (!t[j].actif) {
            t[j] = (Tir){x, y, vx, vy, true, i};
            return; /* on sort des qu'on a trouve un slot */
        }
    }
}

/*
 * hud() — affiche le HUD (Head-Up Display) a chaque frame.
 * Affiche les vies, le score, et une barre de progression verte.
 * La barre va de 0% (debut du niveau) a 100% (toutes vagues passees).
 * Si on est en phase boss, la barre reste a 100%.
 */
static void hud(Joueur *j, ALLEGRO_FONT *f, int vague, bool boss) {
    char buf[64];
    sprintf(buf, "VIES: %d    SCORE: %06d", j->vies, j->score);
    al_draw_text(f, al_map_rgb(255, 255, 255), 40, HAUTEUR - 100, 0, buf);

    /* Calcul du pourcentage de progression (0.0 a 1.0) */
    float p = boss ? 1.0f : (float)vague / 8.0f;

    /* Fond gris de la barre */
    al_draw_filled_rectangle(LARGEUR-700, HAUTEUR-90, LARGEUR-100, HAUTEUR-60,
                             al_map_rgb(40, 40, 80));
    /* Partie verte qui avance */
    al_draw_filled_rectangle(LARGEUR-700, HAUTEUR-90,
                             LARGEUR-700 + (int)(600 * p), HAUTEUR-60,
                             al_map_rgb(50, 200, 100));
    /* Contour blanc */
    al_draw_rectangle(LARGEUR-700, HAUTEUR-90, LARGEUR-100, HAUTEUR-60,
                      al_map_rgb(255, 255, 255), 3);
}

/*
 * transition() — animation entre deux niveaux.
 * Etape 1 : fondu progressif vers le noir (60 frames)
 * Etape 2 : affichage du texte "NIVEAU X" pendant 2 secondes
 * Etape 3 : fondu progressif depuis le noir (60 frames)
 * al_rest(1.0/60) = pause d'une frame pour garder 60fps
 */
static void transition(ALLEGRO_FONT *f, int n) {
    char msg[32];
    sprintf(msg, "NIVEAU %d", n);

    /* Fondu vers le noir */
    for (int a = 0; a <= 60; a++) {
        al_draw_filled_rectangle(0, 0, LARGEUR, HAUTEUR,
                                 al_map_rgba(0, 0, 0, (a * 255) / 60));
        al_flip_display();
        al_rest(1.0 / 60);
    }

    /* Ecran noir avec le texte du niveau suivant */
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_text(f, al_map_rgb(0, 200, 255),
                 LARGEUR/2, HAUTEUR/2 - 80, ALLEGRO_ALIGN_CENTRE, msg);
    al_draw_text(f, al_map_rgb(200, 200, 200),
                 LARGEUR/2, HAUTEUR/2 + 60, ALLEGRO_ALIGN_CENTRE, "Preparez-vous !");
    al_flip_display();
    al_rest(2.0); /* pause de 2 secondes */

    /* Fondu depuis le noir */
    for (int a = 60; a >= 0; a--) {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_text(f, al_map_rgb(0, 200, 255),
                     LARGEUR/2, HAUTEUR/2 - 80, ALLEGRO_ALIGN_CENTRE, msg);
        al_draw_filled_rectangle(0, 0, LARGEUR, HAUTEUR,
                                 al_map_rgba(0, 0, 0, (a * 255) / 60));
        al_flip_display();
        al_rest(1.0 / 60);
    }
}

/*
 * lancer_niveau() — point d'entree du niveau, appelee depuis menu.c.
 * PARTIE INITIALISATION (Mathieu) :
 *   - Charge toutes les images du niveau
 *   - Cree les tableaux d'entites (tirs, ennemis, explosions)
 *   - Cree le boss et initialise toutes les variables
 *   - Lance le timer Allegro a 60fps
 * La suite (boucle de jeu) est dans la PARTIE 2 — Mathis.
 */
int lancer_niveau(ALLEGRO_DISPLAY *d, int num, Joueur *j, Sons *s) {

    /* Chargement du fond selon le numero de niveau */
    ALLEGRO_BITMAP *i_fond = charger(
        num==1 ? "background1.png" : num==2 ? "background2.png" : "background3.png",
        LARGEUR, HAUTEUR, al_map_rgb(0, 20, 60));

    /* Chargement de tous les sprites du jeu */
    ALLEGRO_BITMAP *i_j  = charger("player.png",       200, 160, al_map_rgb(0,   150, 255));
    ALLEGRO_BITMAP *i_e  = charger("enemy.png",        160, 160, al_map_rgb(255, 100,   0));
    ALLEGRO_BITMAP *i_t  = charger("enemy_turret.png", 160, 160, al_map_rgb(200,  50,  50));
    ALLEGRO_BITMAP *i_b  = charger("boss.png",         400, 400, al_map_rgb(150,   0, 200));
    ALLEGRO_BITMAP *i_tj = charger("bullet.png",        60,  20, al_map_rgb(255, 220,  50));
    ALLEGRO_BITMAP *i_te = charger("enemy_bullet.png",  50,  20, al_map_rgb(255,  60,  60));
    ALLEGRO_BITMAP *i_ex = charger("explosion.png",    250, 250, al_map_rgb(255, 140,   0));

    /* Police d'ecriture pour le HUD et la pause */
    ALLEGRO_FONT *font = al_load_ttf_font("font.ttf", 60, 0);
    if (!font) font = al_create_builtin_font(); /* police de secours */

    /* Le joueur utilise le sprite charge ci-dessus */
    j->img = i_j;

    /* Tableaux statiques : pas de malloc, taille fixe, 0 fuite memoire */
    Tir       tj[NB_TIRS]     = {0}; /* tirs du joueur */
    Tir       te[NB_TIRS_ENE] = {0}; /* tirs des ennemis */
    Ennemi    en[NB_ENNEMIS]   = {0}; /* ennemis actifs */
    Explosion ex[NB_EXPS]      = {0}; /* explosions actives */

    /* Boss : demarre hors ecran a droite, entre progressivement */
    Boss boss = {LARGEUR + 100, HAUTEUR/2 - 200, PV_BOSS, 90, true, false, i_b};

    /* Variables de controle de la boucle */
    float fx     = 0;       /* position X du fond (scrolling) */
    int   vague  = 0;       /* numero de la vague en cours (0 a 7) */
    int   tv     = 200;     /* timer avant la prochaine vague */
    int   cd     = 0;       /* cooldown entre deux tirs joueur */
    int   res    = VICTOIRE;/* resultat du niveau */
    bool  pboss  = false;   /* true = phase boss activee */
    bool  pause  = false;   /* true = jeu en pause */
    bool  fini   = false;   /* true = on sort de la boucle */

    /* Lance la musique du niveau (coupe la precedente automatiquement) */
    jouer_musique(s, s->niveaux[num - 1]);

    /* Creation de la file d'evenements Allegro */
    ALLEGRO_EVENT_QUEUE *q = al_create_event_queue();
    ALLEGRO_TIMER *tmr = al_create_timer(1.0 / 60.0); /* 60 fps */
    al_register_event_source(q, al_get_display_event_source(d));
    al_register_event_source(q, al_get_timer_event_source(tmr));
    al_register_event_source(q, al_get_keyboard_event_source());
    al_start_timer(tmr);

    /* ---- LA SUITE EST DANS LA PARTIE 2 (Mathis) ---- */
