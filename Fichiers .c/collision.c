#include "entites.h"
#include <stdbool.h>

/* Test AABB : renvoie true si les deux rectangles se touchent */
bool collision(float ax, float ay, float aw, float ah,
               float bx, float by, float bw, float bh) {
    return ax < bx+bw && ax+aw > bx && ay < by+bh && ay+ah > by;
}

/* Cree une explosion dans le premier slot libre */
void spawn_exp(Explosion exps[], float x, float y, ALLEGRO_BITMAP *img) {
    for (int i = 0; i < NB_EXPS; i++) {
        if (!exps[i].actif) {
            exps[i] = (Explosion){x, y, 35, true, img};
            return;
        }
    }
}

/* Tirs joueur vs ennemis */
void col_tirs_ennemis(Tir tj[], Ennemi en[], Explosion ex[],
                      Joueur *j, ALLEGRO_BITMAP *img_exp) {
    for (int t = 0; t < NB_TIRS; t++) {
        if (!tj[t].actif) continue;
        for (int e = 0; e < NB_ENNEMIS; e++) {
            if (!en[e].actif) continue;
            if (collision(tj[t].x,tj[t].y,60,20, en[e].x,en[e].y,160,160)) {
                tj[t].actif = false;
                if (--en[e].pv <= 0) {
                    spawn_exp(ex, en[e].x, en[e].y, img_exp);
                    en[e].actif = false;
                    j->score += 100;
                }
                break;
            }
        }
    }
}

/* Tirs joueur vs boss */
void col_tirs_boss(Tir tj[], Boss *b, Explosion ex[],
                   Joueur *j, ALLEGRO_BITMAP *img_exp) {
    if (!b->actif) return;
    for (int t = 0; t < NB_TIRS; t++) {
        if (!tj[t].actif) continue;
        if (collision(tj[t].x,tj[t].y,60,20, b->x,b->y,400,400)) {
            tj[t].actif = false;
            j->score += 10;
            if (--b->pv <= 0) {
                spawn_exp(ex, b->x+200, b->y+200, img_exp);
                b->actif = false;
                j->score += 1000;
            }
        }
    }
}

/* Tirs ennemis vs joueur - ignore si invincible */
void col_tirs_joueur(Tir te[], Joueur *j, Explosion ex[],
                     ALLEGRO_BITMAP *img_exp) {
    if (!j->vivant || j->invincible) return;
    for (int t = 0; t < NB_TIRS_ENE; t++) {
        if (!te[t].actif) continue;
        if (collision(te[t].x,te[t].y,50,20, j->x,j->y,200,160)) {
            te[t].actif = false;
            spawn_exp(ex, j->x, j->y, img_exp);
            j->vivant = false;
            j->vies--;
        }
    }
}

/* Contact direct joueur vs ennemi */
void col_joueur_ennemis(Joueur *j, Ennemi en[], Explosion ex[],
                        ALLEGRO_BITMAP *img_exp) {
    if (!j->vivant || j->invincible) return;
    for (int e = 0; e < NB_ENNEMIS; e++) {
        if (!en[e].actif) continue;
        if (collision(j->x,j->y,200,160, en[e].x,en[e].y,160,160)) {
            spawn_exp(ex, j->x, j->y, img_exp);
            en[e].actif = false;
            j->vivant = false;
            j->vies--;
        }
    }
}

/* Contact direct joueur vs boss */
void col_joueur_boss(Joueur *j, Boss *b, Explosion ex[],
                     ALLEGRO_BITMAP *img_exp) {
    if (!j->vivant || j->invincible || !b->actif || !b->apparu) return;
    if (collision(j->x,j->y,200,160, b->x,b->y,400,400)) {
        spawn_exp(ex, j->x, j->y, img_exp);
        j->vivant = false;
        j->vies--;
    }
}
