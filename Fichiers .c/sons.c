#include "sons.h"
#include <stdio.h>

static ALLEGRO_SAMPLE_ID id_courant;
static bool son_actif = false;

Sons charger_sons(void) {
    Sons s = {0};
    s.menu       = al_load_sample("snd_menu.wav");
    s.niveaux[0] = al_load_sample("snd_niveau1.wav");
    s.niveaux[1] = al_load_sample("snd_niveau2.wav");
    s.niveaux[2] = al_load_sample("snd_niveau3.wav");
    s.tir        = al_load_sample("snd_tir.wav");
    return s;
}

/* Arrete le son en cours puis joue le nouveau */
void jouer_musique(Sons *s, ALLEGRO_SAMPLE *nouveau) {
    /* Arrete proprement le son precedent */
    if (son_actif) {
        al_stop_sample(&id_courant);
        son_actif = false;
    }
    if (nouveau == NULL) return;
    son_actif = al_play_sample(nouveau, 1.0, 0.0, 1.0,
                               ALLEGRO_PLAYMODE_ONCE, &id_courant);
}

/* Effet court - ne coupe pas la musique */
void jouer_effet(ALLEGRO_SAMPLE *s) {
    if (s) al_play_sample(s, 0.7, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
}

void liberer_sons(Sons *s) {
    if (son_actif) { al_stop_sample(&id_courant); son_actif = false; }
    if (s->menu)       al_destroy_sample(s->menu);
    if (s->niveaux[0]) al_destroy_sample(s->niveaux[0]);
    if (s->niveaux[1]) al_destroy_sample(s->niveaux[1]);
    if (s->niveaux[2]) al_destroy_sample(s->niveaux[2]);
    if (s->tir)        al_destroy_sample(s->tir);
}
