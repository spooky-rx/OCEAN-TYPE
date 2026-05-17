#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include "globals.h"
#include "menu.h"
#include "sons.h"

int main(void) {
    al_init(); al_init_font_addon(); al_init_ttf_addon();
    al_init_image_addon(); al_init_primitives_addon();
    al_install_keyboard(); al_install_audio();
    al_init_acodec_addon(); al_reserve_samples(8);

    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    ALLEGRO_DISPLAY *d = al_create_display(LARGEUR, HAUTEUR);
    if (!d) {
        al_set_new_display_flags(ALLEGRO_WINDOWED);
        d = al_create_display(LARGEUR, HAUTEUR);
    }
    al_set_window_title(d, "Ocean Type");

    Sons s = charger_sons();

    /* On passe bien par jouer_musique pour que son_actif soit mis a jour */
    al_rest(0.3);
    jouer_musique(&s, s.menu);

    afficher_menu(d, &s);
    liberer_sons(&s);
    al_destroy_display(d);
    return 0;
}
