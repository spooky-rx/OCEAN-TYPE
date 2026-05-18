#include "entites.h"
#include <allegro5/allegro_primitives.h>

/* Deplace le joueur selon le clavier et borne a l'ecran */
void maj_joueur(Joueur *j) {
    ALLEGRO_KEYBOARD_STATE ks;
    if (!j->vivant) return;
    al_get_keyboard_state(&ks);
    if (al_key_down(&ks, ALLEGRO_KEY_UP))    j->y -= 18;
    if (al_key_down(&ks, ALLEGRO_KEY_DOWN))  j->y += 18;
    if (al_key_down(&ks, ALLEGRO_KEY_LEFT))  j->x -= 18;
    if (al_key_down(&ks, ALLEGRO_KEY_RIGHT)) j->x += 18;
    if (j->x < 0)             j->x = 0;
    if (j->x > LARGEUR - 200) j->x = LARGEUR - 200;
    if (j->y < 0)             j->y = 0;
    if (j->y > HAUTEUR - 160) j->y = HAUTEUR - 160;
    if (j->invincible) {
        j->timer_inv--;
        if (j->timer_inv <= 0) j->invincible = false;
    }
}

/* Dessine le joueur - clignote si invincible */
void dessiner_joueur(Joueur *j) {
    if (!j->vivant) return;
    if (j->invincible && (j->timer_inv / 5) % 2 == 0) return;
    al_draw_bitmap(j->img, j->x, j->y, 0);
}

/* Replace le joueur et active l'invincibilite */
void respawn_joueur(Joueur *j) {
    j->x = 400; j->y = HAUTEUR / 2 - 80;
    j->vivant = true;
    j->invincible = true;
    j->timer_inv = INVINCIBLE;
}
