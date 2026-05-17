#include "jeu.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>

void ecran_fin(ALLEGRO_DISPLAY *d, int score, bool victoire);

void afficher_menu(ALLEGRO_DISPLAY *d, Sons *s) {
    ALLEGRO_FONT *f1 = al_load_ttf_font("font.ttf",160,0); if(!f1)f1=al_create_builtin_font();
    ALLEGRO_FONT *f2 = al_load_ttf_font("font.ttf", 65,0); if(!f2)f2=al_create_builtin_font();
    ALLEGRO_FONT *f3 = al_load_ttf_font("font.ttf", 48,0); if(!f3)f3=al_create_builtin_font();
    ALLEGRO_BITMAP *fond = al_load_bitmap("background1.png");
    if (!fond) {
        fond = al_create_bitmap(LARGEUR,HAUTEUR);
        al_set_target_bitmap(fond);
        al_clear_to_color(al_map_rgb(0,20,60));
        al_set_target_backbuffer(d);
    }

    ALLEGRO_EVENT_QUEUE *q = al_create_event_queue();
    ALLEGRO_TIMER *t = al_create_timer(1.0/60.0);
    al_register_event_source(q, al_get_display_event_source(d));
    al_register_event_source(q, al_get_timer_event_source(t));
    al_register_event_source(q, al_get_keyboard_event_source());
    al_start_timer(t);

    /* Joue le son du menu maintenant - le timer n'est pas necessaire */
    /* Son lance depuis main.c - on ne le relance pas ici */

    const char *opts[5]={"Nouvelle partie","Niveau 1","Niveau 2","Niveau 3","Quitter"};
    int sel=0, onglet=0;
    float fx=0;
    bool fin=false;

    while (!fin) {
        ALLEGRO_EVENT ev; al_wait_for_event(q, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) fin=true;

        if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            int k = ev.keyboard.keycode;
            if (k==ALLEGRO_KEY_ESCAPE) fin=true;
            if (k==ALLEGRO_KEY_TAB||k==ALLEGRO_KEY_LEFT||k==ALLEGRO_KEY_RIGHT) onglet=!onglet;
            if (onglet==0) {
                if (k==ALLEGRO_KEY_UP)   { sel--; if(sel<0) sel=4; }
                if (k==ALLEGRO_KEY_DOWN) { sel++; if(sel>4) sel=0; }
                if (k==ALLEGRO_KEY_ENTER) {
                    if (sel==4) { fin=true; }
                    else {
                        Joueur j={0}; j.vies=VIES_MAX; j.vivant=true;
                        int dep=(sel==0)?1:sel, res=VICTOIRE;
                        for (int n=dep; n<=3&&res==VICTOIRE; n++) {
                            res = lancer_niveau(d, n, &j, s);
                            /* Remet le rendu sur la fenetre apres chaque niveau */
                            al_set_target_backbuffer(d);
                        }
                        if (res != MENU) {
                            ecran_fin(d, j.score, res==VICTOIRE);
                            al_set_target_backbuffer(d);
                        }
                        /* Relance la musique du menu proprement */
                        jouer_musique(s, s->menu);
                    }
                }
            }
        }

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            fx -= 3; if(fx <= -LARGEUR) fx=0;
            al_clear_to_color(al_map_rgb(0,0,0));
            al_draw_bitmap(fond, fx, 0, 0);
            al_draw_bitmap(fond, fx+LARGEUR, 0, 0);
            al_draw_filled_rectangle(0,0,LARGEUR,HAUTEUR, al_map_rgba(0,0,0,150));

            al_draw_text(f1,al_map_rgb(0,200,255), LARGEUR/2,60,  ALLEGRO_ALIGN_CENTRE,"OCEAN TYPE");
            al_draw_text(f3,al_map_rgb(100,180,220),LARGEUR/2,260, ALLEGRO_ALIGN_CENTRE,"Defendez les profondeurs !");

            /* Onglets */
            al_draw_text(f2,onglet==0?al_map_rgb(0,200,255):al_map_rgb(80,80,120),LARGEUR/2-400,450,ALLEGRO_ALIGN_CENTRE,"[ JOUER ]");
            al_draw_text(f2,onglet==1?al_map_rgb(0,200,255):al_map_rgb(80,80,120),LARGEUR/2+400,450,ALLEGRO_ALIGN_CENTRE,"[ COMMANDES ]");
            al_draw_text(f3,al_map_rgb(60,60,100),LARGEUR/2,530,ALLEGRO_ALIGN_CENTRE,"TAB ou fleches gauche/droite");

            if (onglet==0) {
                for (int i=0;i<5;i++) {
                    ALLEGRO_COLOR c=(i==sel)?al_map_rgb(255,220,0):al_map_rgb(160,160,200);
                    if(i==sel) al_draw_filled_rectangle(LARGEUR/2-720,670+i*200-10,LARGEUR/2+720,670+i*200+90,al_map_rgba(0,80,150,120));
                    al_draw_text(f2,c,LARGEUR/2,670+i*200,ALLEGRO_ALIGN_CENTRE,opts[i]);
                }
            } else {
                float y=650, p=155;
                al_draw_text(f2,al_map_rgb(255,220,0),LARGEUR/2-600,y,    0,"Deplacer  :"); al_draw_text(f2,al_map_rgb(200,200,255),LARGEUR/2+50,y,    0,"Touches directionnelles");
                al_draw_text(f2,al_map_rgb(255,220,0),LARGEUR/2-600,y+p,  0,"Tirer     :"); al_draw_text(f2,al_map_rgb(200,200,255),LARGEUR/2+50,y+p,  0,"SPACE  (double munition)");
                al_draw_text(f2,al_map_rgb(255,220,0),LARGEUR/2-600,y+p*2,0,"Pause     :"); al_draw_text(f2,al_map_rgb(200,200,255),LARGEUR/2+50,y+p*2,0,"P  ou  ECHAP");
                al_draw_text(f2,al_map_rgb(255,220,0),LARGEUR/2-600,y+p*3,0,"Reprendre :"); al_draw_text(f2,al_map_rgb(200,200,255),LARGEUR/2+50,y+p*3,0,"R");
                al_draw_text(f2,al_map_rgb(255,220,0),LARGEUR/2-600,y+p*4,0,"Menu      :"); al_draw_text(f2,al_map_rgb(200,200,255),LARGEUR/2+50,y+p*4,0,"M  (depuis la pause)");
                al_draw_text(f3,al_map_rgb(100,180,255),LARGEUR/2,y+p*5+30,ALLEGRO_ALIGN_CENTRE,"Boss au niveau 3 uniquement !");
            }
            al_flip_display();
        }
    }

    al_destroy_bitmap(fond);
    al_destroy_event_queue(q);
    al_destroy_timer(t);
    al_destroy_font(f1);
    al_destroy_font(f2);
    al_destroy_font(f3);
}
