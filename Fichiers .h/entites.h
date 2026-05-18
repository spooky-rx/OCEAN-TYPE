#ifndef ENTITES_H
#define ENTITES_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdbool.h>
#include "globals.h"

typedef struct {
    float x, y;
    int   vies, score;
    bool  vivant, invincible;
    int   timer_inv;
    ALLEGRO_BITMAP *img;
} Joueur;

typedef struct {
    float x, y, vx;
    int   pv, timer_tir, cadence;
    bool  actif, fixe;
    ALLEGRO_BITMAP *img;
} Ennemi;

typedef struct {
    float x, y, vx, vy;
    bool  actif;
    ALLEGRO_BITMAP *img;
} Tir;

typedef struct {
    float x, y;
    int   timer;
    bool  actif;
    ALLEGRO_BITMAP *img;
} Explosion;

typedef struct {
    float x, y;
    int   pv, timer_tir;
    bool  actif, apparu;
    ALLEGRO_BITMAP *img;
} Boss;

typedef struct {
    ALLEGRO_SAMPLE    *menu;
    ALLEGRO_SAMPLE    *niveaux[3];
    ALLEGRO_SAMPLE    *tir;
    ALLEGRO_SAMPLE_ID  id;
    bool               joue;
} Sons;

#endif
