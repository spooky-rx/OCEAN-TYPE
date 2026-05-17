#include "jeu.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <math.h>
#include <stdio.h>

/* Declarations externes */
void maj_joueur(Joueur *j);
void dessiner_joueur(Joueur *j);
void respawn_joueur(Joueur *j);
void col_tirs_ennemis(Tir tj[],Ennemi en[],Explosion ex[],Joueur *j,ALLEGRO_BITMAP *i);
void col_tirs_boss(Tir tj[],Boss *b,Explosion ex[],Joueur *j,ALLEGRO_BITMAP *i);
void col_tirs_joueur(Tir te[],Joueur *j,Explosion ex[],ALLEGRO_BITMAP *i);
void col_joueur_ennemis(Joueur *j,Ennemi en[],Explosion ex[],ALLEGRO_BITMAP *i);
void col_joueur_boss(Joueur *j,Boss *b,Explosion ex[],ALLEGRO_BITMAP *i);
extern float V[3][8][6][5];
void spawn(Ennemi en[],float x,float y,bool fixe,int pv,int cad,ALLEGRO_BITMAP *iv,ALLEGRO_BITMAP *it);

/* Charge une image ou cree un carre couleur si fichier manquant */
static ALLEGRO_BITMAP *charger(const char *f,int w,int h,ALLEGRO_COLOR c){
    ALLEGRO_BITMAP *b=al_load_bitmap(f);
    if(b)return b;
    b=al_create_bitmap(w,h);al_set_target_bitmap(b);al_clear_to_color(c);
    al_set_target_backbuffer(al_get_current_display());return b;
}

/* Place 2 tirs cote a cote */
static void tirer(Tir t[],float x,float y,ALLEGRO_BITMAP *i){
    int ok=0;
    for(int j=0;j<NB_TIRS&&ok<2;j++)
        if(!t[j].actif){t[j]=(Tir){x,y+ok*60,55,0,true,i};ok++;}
}

/* Place un tir ennemi dans un slot libre */
static void tir_e(Tir t[],float x,float y,float vx,float vy,ALLEGRO_BITMAP *i){
    for(int j=0;j<NB_TIRS_ENE;j++)
        if(!t[j].actif){t[j]=(Tir){x,y,vx,vy,true,i};return;}
}

/* HUD : vies, score, barre de progression */
static void hud(Joueur *j,ALLEGRO_FONT *f,int vague,bool boss){
    char buf[64]; sprintf(buf,"VIES: %d    SCORE: %06d",j->vies,j->score);
    al_draw_text(f,al_map_rgb(255,255,255),40,HAUTEUR-100,0,buf);
    float p=boss?1.0f:(float)vague/8.0f;
    al_draw_filled_rectangle(LARGEUR-700,HAUTEUR-90,LARGEUR-100,HAUTEUR-60,al_map_rgb(40,40,80));
    al_draw_filled_rectangle(LARGEUR-700,HAUTEUR-90,LARGEUR-700+(int)(600*p),HAUTEUR-60,al_map_rgb(50,200,100));
    al_draw_rectangle(LARGEUR-700,HAUTEUR-90,LARGEUR-100,HAUTEUR-60,al_map_rgb(255,255,255),3);
}

/* Fondu noir + texte de transition entre niveaux */
static void transition(ALLEGRO_FONT *f,int n){
    char msg[32]; sprintf(msg,"NIVEAU %d",n);
    for(int a=0;a<=60;a++){al_draw_filled_rectangle(0,0,LARGEUR,HAUTEUR,al_map_rgba(0,0,0,(a*255)/60));al_flip_display();al_rest(1.0/60);}
    al_clear_to_color(al_map_rgb(0,0,0));
    al_draw_text(f,al_map_rgb(0,200,255),LARGEUR/2,HAUTEUR/2-80,ALLEGRO_ALIGN_CENTRE,msg);
    al_draw_text(f,al_map_rgb(200,200,200),LARGEUR/2,HAUTEUR/2+60,ALLEGRO_ALIGN_CENTRE,"Preparez-vous !");
    al_flip_display();al_rest(2.0);
    for(int a=60;a>=0;a--){al_clear_to_color(al_map_rgb(0,0,0));al_draw_text(f,al_map_rgb(0,200,255),LARGEUR/2,HAUTEUR/2-80,ALLEGRO_ALIGN_CENTRE,msg);al_draw_filled_rectangle(0,0,LARGEUR,HAUTEUR,al_map_rgba(0,0,0,(a*255)/60));al_flip_display();al_rest(1.0/60);}
}

/* ================================================================
   BOUCLE PRINCIPALE D'UN NIVEAU
   ================================================================ */
int lancer_niveau(ALLEGRO_DISPLAY *d,int num,Joueur *j,Sons *s){
    ALLEGRO_BITMAP *i_fond=charger(num==1?"background1.png":num==2?"background2.png":"background3.png",LARGEUR,HAUTEUR,al_map_rgb(0,20,60));
    ALLEGRO_BITMAP *i_j  =charger("player.png",      200,160,al_map_rgb(0,150,255));
    ALLEGRO_BITMAP *i_e  =charger("enemy.png",       160,160,al_map_rgb(255,100,0));
    ALLEGRO_BITMAP *i_t  =charger("enemy_turret.png",160,160,al_map_rgb(200,50,50));
    ALLEGRO_BITMAP *i_b  =charger("boss.png",        400,400,al_map_rgb(150,0,200));
    ALLEGRO_BITMAP *i_tj =charger("bullet.png",       60, 20,al_map_rgb(255,220,50));
    ALLEGRO_BITMAP *i_te =charger("enemy_bullet.png", 50, 20,al_map_rgb(255,60,60));
    ALLEGRO_BITMAP *i_ex =charger("explosion.png",   250,250,al_map_rgb(255,140,0));
    ALLEGRO_FONT   *font =al_load_ttf_font("font.ttf",60,0);
    if(!font)font=al_create_builtin_font();

    j->img=i_j;
    Tir tj[NB_TIRS]={0},te[NB_TIRS_ENE]={0};
    Ennemi en[NB_ENNEMIS]={0}; Explosion ex[NB_EXPS]={0};
    Boss boss={LARGEUR+100,HAUTEUR/2-200,PV_BOSS,90,true,false,i_b};
    float fx=0; int vague=0,tv=200,cd=0,res=VICTOIRE;
    bool pboss=false,pause=false,fini=false;

    jouer_musique(s,s->niveaux[num-1]);

    ALLEGRO_EVENT_QUEUE *q=al_create_event_queue();
    ALLEGRO_TIMER *tmr=al_create_timer(1.0/60.0);
    al_register_event_source(q,al_get_display_event_source(d));
    al_register_event_source(q,al_get_timer_event_source(tmr));
    al_register_event_source(q,al_get_keyboard_event_source());
    al_start_timer(tmr);

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
            for(int i=0;i<NB_EXPS;i++) if(ex[i].actif){
                al_set_blender(ALLEGRO_ADD,ALLEGRO_ALPHA,ALLEGRO_INVERSE_ALPHA);
                al_draw_bitmap(ex[i].img,ex[i].x,ex[i].y,0);
                al_set_blender(ALLEGRO_ADD,ALLEGRO_ONE,ALLEGRO_INVERSE_ALPHA);
            }
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
