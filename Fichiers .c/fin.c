#include "jeu.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>

void ecran_fin(ALLEGRO_DISPLAY *d, int score, bool victoire) {
    ALLEGRO_EVENT_QUEUE *q = al_create_event_queue();
    ALLEGRO_TIMER *t = al_create_timer(1.0/60.0);
    ALLEGRO_FONT *f1 = al_load_ttf_font("font.ttf",160,0);
    ALLEGRO_FONT *f2 = al_load_ttf_font("font.ttf", 65,0);
    if (!f1) f1 = al_create_builtin_font();
    if (!f2) f2 = al_create_builtin_font();

    al_register_event_source(q, al_get_display_event_source(d));
    al_register_event_source(q, al_get_timer_event_source(t));
    al_register_event_source(q, al_get_keyboard_event_source());
    al_start_timer(t);

    bool fini = false;
    while (!fini) {
        ALLEGRO_EVENT ev; al_wait_for_event(q, &ev);
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) fini = true;
        if (ev.type == ALLEGRO_EVENT_KEY_DOWN &&
           (ev.keyboard.keycode == ALLEGRO_KEY_ENTER ||
            ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)) fini = true;

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            char buf[64];
            al_clear_to_color(al_map_rgb(0,10,40));
            if (victoire) {
                al_draw_text(f1,al_map_rgb(50,200,255),LARGEUR/2,500,
                    ALLEGRO_ALIGN_CENTRE,"L'OCEAN EST SAUVE !");
                al_draw_text(f2,al_map_rgb(150,230,255),LARGEUR/2,900,
                    ALLEGRO_ALIGN_CENTRE,"Vous avez vaincu les envahisseurs !");
            } else {
                al_draw_text(f1,al_map_rgb(255,60,60),LARGEUR/2,500,
                    ALLEGRO_ALIGN_CENTRE,"LES PROFONDEURS VOUS ONT ENGLOUTI...");
                al_draw_text(f2,al_map_rgb(200,100,100),LARGEUR/2,900,
                    ALLEGRO_ALIGN_CENTRE,"La mer vous offre une seconde chance.");
            }
            sprintf(buf,"Score : %06d", score);
            al_draw_text(f2,al_map_rgb(255,220,0),LARGEUR/2,1300,ALLEGRO_ALIGN_CENTRE,buf);
            al_draw_text(f2,al_map_rgb(160,160,200),LARGEUR/2,1700,
                ALLEGRO_ALIGN_CENTRE,"ENTREE pour revenir au menu");
            al_flip_display();
        }
    }
    al_destroy_font(f1); al_destroy_font(f2);
    al_destroy_event_queue(q); al_destroy_timer(t);
}
