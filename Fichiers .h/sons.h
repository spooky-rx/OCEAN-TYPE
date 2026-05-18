#ifndef SONS_H
#define SONS_H
#include "entites.h"
Sons charger_sons(void);
void jouer_musique(Sons *s, ALLEGRO_SAMPLE *nouveau);
void jouer_effet(ALLEGRO_SAMPLE *s);
void liberer_sons(Sons *s);
#endif
