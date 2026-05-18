#ifndef JEU_H
#define JEU_H

#include <allegro5/allegro.h>
#include <stdbool.h>
#include "entites.h"
#include "sons.h"

int  lancer_niveau(ALLEGRO_DISPLAY *d, int num, Joueur *j, Sons *s);
void afficher_menu(ALLEGRO_DISPLAY *d, Sons *s);

#endif
