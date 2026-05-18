#include "jeu.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <math.h>
#include <stdio.h>

/* Declarations externes */
void maj_joueur(Joueur *j);
    while(!fini){
        ALLEGRO_EVENT ev; al_wait_for_event(q,&ev);
        if(ev.type==ALLEGRO_EVENT_DISPLAY_CLOSE){res=MENU;fini=true;}
        if(ev.type==ALLEGRO_EVENT_KEY_DOWN){
            int k=ev.keyboard.keycode;
            if(k==ALLEGRO_KEY_P||k==ALLEGRO_KEY_ESCAPE) pause=true;
            if(pause&&k==ALLEGRO_KEY_R) pause=false;
            if(pause&&k==ALLEGRO_KEY_M){res=MENU;fini=true;}
            if(!pause&&k==ALLEGRO_KEY_SPACE&&cd<=0){tirer(tj,j->x+200,j->y,i_tj);jouer_effet(s->tir);cd=8;}
        }
        if(ev.type==ALLEGRO_EVENT_TIMER&&!pause){
            ALLEGRO_KEYBOARD_STATE ks; al_get_keyboard_state(&ks);
            if(al_key_down(&ks,ALLEGRO_KEY_SPACE)&&cd<=0){tirer(tj,j->x+200,j->y,i_tj);jouer_effet(s->tir);cd=8;}
            if(cd>0)cd--;
            if(!pboss){fx-=3;if(fx<=-LARGEUR)fx=0;}

            /* Spawn vague depuis niveaux.c */
            if(!pboss&&--tv<=0){
                if(vague<8){
                    float(*v)[5]=V[num-1][vague];
                    for(int k=0;k<6;k++) if(v[k][0]>0) spawn(en,v[k][0],v[k][1],(bool)v[k][2],(int)v[k][3],(int)v[k][4],i_e,i_t);
                    vague++;
                }
                bool tous=true;
                for(int i=0;i<NB_ENNEMIS;i++) if(en[i].actif){tous=false;break;}
                if(vague>=8&&tous){if(num==3)pboss=true;else{res=VICTOIRE;fini=true;}}
                tv=400;
            }

            /* Tirs ennemis */
            for(int i=0;i<NB_ENNEMIS;i++){
                if(!en[i].actif)continue;
                if(!en[i].fixe){en[i].x+=en[i].vx;if(en[i].x<-160)en[i].actif=false;}
                if(--en[i].timer_tir<=0){
                    float dx=j->x-en[i].x,dy=j->y-en[i].y,dst=sqrt(dx*dx+dy*dy);
                    if(dst>0){dx/=dst;dy/=dst;}
                    tir_e(te,en[i].x,en[i].y+60,dx*20,dy*20,i_te);
                    en[i].timer_tir=en[i].cadence;
                }
            }

            /* Boss */
            if(pboss&&boss.actif){
                if(!boss.apparu){boss.x-=8;if(boss.x<=LARGEUR-600)boss.apparu=true;}
                if(boss.apparu&&--boss.timer_tir<=0){
                    tir_e(te,boss.x,boss.y+150,-24,  0,i_te);
                    tir_e(te,boss.x,boss.y+150,-20, 12,i_te);
                    tir_e(te,boss.x,boss.y+150,-20,-12,i_te);
                    boss.timer_tir=55;
                }
            }

            /* Mouvement tirs + explosions */
            for(int i=0;i<NB_TIRS;i++) if(tj[i].actif){tj[i].x+=tj[i].vx;if(tj[i].x>LARGEUR)tj[i].actif=false;}
            for(int i=0;i<NB_TIRS_ENE;i++) if(te[i].actif){te[i].x+=te[i].vx;te[i].y+=te[i].vy;if(te[i].x<0||te[i].x>LARGEUR||te[i].y<0||te[i].y>HAUTEUR)te[i].actif=false;}
            for(int i=0;i<NB_EXPS;i++) if(ex[i].actif&&--ex[i].timer<=0)ex[i].actif=false;

            maj_joueur(j);
            col_tirs_ennemis(tj,en,ex,j,i_ex);
            if(pboss)col_tirs_boss(tj,&boss,ex,j,i_ex);
            col_tirs_joueur(te,j,ex,i_ex);
            col_joueur_ennemis(j,en,ex,i_ex);
            if(pboss)col_joueur_boss(j,&boss,ex,i_ex);

            if(!j->vivant){if(j->vies>0)respawn_joueur(j);else{res=GAME_OVER;fini=true;}}
            if(pboss&&!boss.actif){res=VICTOIRE;fini=true;}
        }

        /* Rendu */
        if(al_is_event_queue_empty(q)){
            al_clear_to_color(al_map_rgb(0,0,0));
            al_draw_bitmap(i_fond,fx,0,0); al_draw_bitmap(i_fond,fx+LARGEUR,0,0);
            for(int i=0;i<NB_ENNEMIS;i++) if(en[i].actif) al_draw_bitmap(en[i].img,en[i].x,en[i].y,0);
            if(pboss&&boss.actif){
                al_draw_bitmap(boss.img,boss.x,boss.y,0);
                int bw=(boss.pv*800)/PV_BOSS;
                al_draw_filled_rectangle(40,40,840,88,al_map_rgb(80,0,0));
                al_draw_filled_rectangle(40,40,40+bw,88,al_map_rgb(220,40,40));
                al_draw_rectangle(40,40,840,88,al_map_rgb(255,255,255),4);
            }
            for(int i=0;i<NB_TIRS;i++) if(tj[i].actif) al_draw_bitmap(tj[i].img,tj[i].x,tj[i].y,0);
            for(int i=0;i<NB_TIRS_ENE;i++) if(te[i].actif) al_draw_bitmap(te[i].img,te[i].x,te[i].y,0);
            for(int i=0;i<NB_EXPS;i++) if(ex[i].actif) al_draw_bitmap(ex[i].img,ex[i].x,ex[i].y,0);
            dessiner_joueur(j);
            hud(j,font,vague,pboss);
            if(pause){
                al_draw_filled_rectangle(0,0,LARGEUR,HAUTEUR,al_map_rgba(0,0,0,170));
                al_draw_text(font,al_map_rgb(0,200,255), LARGEUR/2,HAUTEUR/2-200,ALLEGRO_ALIGN_CENTRE,"PAUSE");
                al_draw_text(font,al_map_rgb(255,255,255),LARGEUR/2,HAUTEUR/2,   ALLEGRO_ALIGN_CENTRE,"R  -  Reprendre");
                al_draw_text(font,al_map_rgb(255,180,100),LARGEUR/2,HAUTEUR/2+150,ALLEGRO_ALIGN_CENTRE,"M  -  Menu");
            }
            al_flip_display();
        }
    }

    if(res==VICTOIRE&&num<3) transition(font,num+1);
    al_destroy_bitmap(i_fond);al_destroy_bitmap(i_j);al_destroy_bitmap(i_e);
    al_destroy_bitmap(i_t);al_destroy_bitmap(i_b);al_destroy_bitmap(i_tj);
    al_destroy_bitmap(i_te);al_destroy_bitmap(i_ex);
    al_destroy_event_queue(q);al_destroy_timer(tmr);
    if(font)al_destroy_font(font);
    return res;
}
