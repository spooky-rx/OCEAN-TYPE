/* entites.h : notre fichier d'en-tete qui contient toutes les structs.
   On en a besoin ici pour connaitre la definition de Joueur
   (ses champs x, y, vivant, invincible, etc.). */
#include "entites.h"

/* allegro_primitives.h : module Allegro pour dessiner des formes.
   Pas directement utilise dans ce fichier mais necessaire
   pour que la compilation ne genere pas d'avertissement. */
#include <allegro5/allegro_primitives.h>


/* ================================================================
   FONCTION maj_joueur()
   Appelee a chaque frame depuis jeu.c.
   Lit l'etat du clavier et deplace le joueur en consequence.
   Empeche le joueur de sortir de l'ecran.
   Gere le decompte de l'invincibilite.
   ================================================================ */

/* void = ne retourne rien, fait juste une action.
   Joueur *j = pointeur vers la struct du joueur.
   On passe un pointeur pour modifier directement les champs
   (x, y, invincible...) sans copier toute la struct. */
void maj_joueur(Joueur *j) {

    /* ALLEGRO_KEYBOARD_STATE ks : variable qui va contenir
       l'etat de toutes les touches du clavier au moment de l'appel.
       C'est une "photo" instantanee du clavier. */
    ALLEGRO_KEYBOARD_STATE ks;

    /* if (!j->vivant) return :
       Si le joueur est mort (vivant == false), on ne fait rien.
       return quitte la fonction immediatement.
       Sans ca le joueur continuerait a se deplacer apres sa mort. */
    if (!j->vivant) return;

    /* al_get_keyboard_state(&ks) : remplit ks avec l'etat actuel
       de toutes les touches. Le & signifie "adresse de ks"
       car la fonction a besoin d'un pointeur pour modifier ks. */
    al_get_keyboard_state(&ks);

    /* al_key_down(&ks, ALLEGRO_KEY_UP) : renvoie true si la touche
       fleche haut est enfoncee EN CE MOMENT.
       j->y -= 18 : diminue Y de 18 pixels.
       En Allegro, Y=0 est en HAUT de l'ecran, donc diminuer Y
       fait monter le joueur. 18 pixels/frame = vitesse de deplacement. */
    if (al_key_down(&ks, ALLEGRO_KEY_UP))    j->y -= 18;

    /* Fleche bas : augmente Y de 18 pixels = descend sur l'ecran. */
    if (al_key_down(&ks, ALLEGRO_KEY_DOWN))  j->y += 18;

    /* Fleche gauche : diminue X de 18 pixels = va vers la gauche. */
    if (al_key_down(&ks, ALLEGRO_KEY_LEFT))  j->x -= 18;

    /* Fleche droite : augmente X de 18 pixels = va vers la droite. */
    if (al_key_down(&ks, ALLEGRO_KEY_RIGHT)) j->x += 18;

    /* Borne gauche : si le joueur depasse le bord gauche (x < 0)
       on le remet a 0. Il ne peut pas sortir de l'ecran a gauche. */
    if (j->x < 0)             j->x = 0;

    /* Borne droite : si le joueur depasse le bord droit
       on le bloque a LARGEUR-200.
       On soustrait 200 car le sprite du joueur fait 200px de large.
       Sans ca le bord droit du sprite sortirait de l'ecran. */
    if (j->x > LARGEUR - 200) j->x = LARGEUR - 200;

    /* Borne haute : si le joueur depasse le haut de l'ecran (y < 0)
       on le remet a 0. */
    if (j->y < 0)             j->y = 0;

    /* Borne basse : bloque le joueur a HAUTEUR-160.
       On soustrait 160 car le sprite fait 160px de haut. */
    if (j->y > HAUTEUR - 160) j->y = HAUTEUR - 160;

    /* Gestion de l'invincibilite apres un respawn.
       Si le joueur est invincible on decremente le timer. */
    if (j->invincible) {

        /* timer_inv-- : decremente le compteur de 1 a chaque frame.
           Comme la boucle tourne 60 fois/seconde,
           INVINCIBLE = 180 frames = 3 secondes d'invincibilite. */
        j->timer_inv--;

        /* Quand le timer atteint 0 : l'invincibilite se termine.
           Le joueur redevient vulnerable aux tirs et contacts. */
        if (j->timer_inv <= 0) j->invincible = false;
    }
}


/* ================================================================
   FONCTION dessiner_joueur()
   Appelee a chaque frame depuis jeu.c, apres maj_joueur().
   Dessine le sprite du joueur a sa position x, y.
   Si invincible : clignote en sautant 1 frame sur 2.
   ================================================================ */

/* Joueur *j : pointeur vers le joueur pour lire ses champs. */
void dessiner_joueur(Joueur *j) {

    /* Si le joueur est mort on ne le dessine pas du tout. */
    if (!j->vivant) return;

    /* Formule du clignotement :
       j->timer_inv / 5 : divise le timer par 5.
         Cela change de valeur toutes les 5 frames au lieu de chaque frame.
         Sans ca le clignotement serait trop rapide (invisible).
       % 2 : modulo 2, donne 0 ou 1 en alternance.
         timer=180 -> 180/5=36 -> 36%2=0 -> invisible
         timer=175 -> 175/5=35 -> 35%2=1 -> visible
         timer=170 -> 170/5=34 -> 34%2=0 -> invisible
         etc.
       == 0 : si le resultat est 0 on ne dessine pas le joueur.
       Effet final : le joueur clignote toutes les 5 frames. */
    if (j->invincible && (j->timer_inv / 5) % 2 == 0) return;

    /* al_draw_bitmap : dessine l'image du joueur a sa position.
       j->img = le sprite du joueur (charge dans lancer_niveau).
       j->x, j->y = position du coin haut-gauche du sprite.
       0 = pas de flags speciaux (pas de miroir, pas de rotation). */
    al_draw_bitmap(j->img, j->x, j->y, 0);
}


/* ================================================================
   FONCTION respawn_joueur()
   Appelee depuis jeu.c quand le joueur meurt et qu'il lui reste des vies.
   Replace le joueur a sa position de depart et l'immunise.
   ================================================================ */

/* Joueur *j : pointeur vers le joueur pour modifier ses champs. */
void respawn_joueur(Joueur *j) {

    /* Remet le joueur a sa position de depart :
       x = 400 : 400 pixels depuis la gauche.
       y = HAUTEUR/2 - 80 : a peu pres au centre vertical de l'ecran. */
    j->x = 400;
    j->y = HAUTEUR / 2 - 80;

    /* vivant = true : le joueur est de nouveau en vie.
       Sans ca dessiner_joueur() ne le dessinerait pas
       et les collisions seraient ignorees. */
    j->vivant = true;

    /* invincible = true : active l'immunite temporaire.
       Empeche le joueur de mourir instantanement en reapparaissant
       au milieu des tirs ennemis deja presents a l'ecran. */
    j->invincible = true;

    /* timer_inv = INVINCIBLE : remet le compteur a 180 frames.
       INVINCIBLE est defini dans globals.h.
       180 frames / 60fps = 3 secondes d'invincibilite. */
    j->timer_inv = INVINCIBLE;
}
