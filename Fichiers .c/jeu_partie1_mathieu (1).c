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
ALLEGRO_BITMAP *charger(const char *f,int w,int h,ALLEGRO_COLOR c){
    ALLEGRO_BITMAP *b=al_load_bitmap(f);
    if(b)return b;
    b=al_create_bitmap(w,h);al_set_target_bitmap(b);al_clear_to_color(c);
    al_set_target_backbuffer(al_get_current_display());return b;
}

/* Place 2 tirs cote a cote */
void tirer(Tir t[],float x,float y,ALLEGRO_BITMAP *i){
    int ok=0;
    for(int j=0;j<NB_TIRS&&ok<2;j++)
        if(!t[j].actif){t[j]=(Tir){x,y+ok*60,55,0,true,i};ok++;}
}

/* Place un tir ennemi dans un slot libre */
void tir_e(Tir t[],float x,float y,float vx,float vy,ALLEGRO_BITMAP *i){
    for(int j=0;j<NB_TIRS_ENE;j++)
        if(!t[j].actif){t[j]=(Tir){x,y,vx,vy,true,i};return;}
}

/* HUD : vies, score, barre de progression */
void hud(Joueur *j,ALLEGRO_FONT *f,int vague,bool boss){
    char buf[64]; sprintf(buf,"VIES: %d    SCORE: %06d",j->vies,j->score);
    al_draw_text(f,al_map_rgb(255,255,255),40,HAUTEUR-100,0,buf);
    float p=boss?1.0f:(float)vague/8.0f;
    al_draw_filled_rectangle(LARGEUR-700,HAUTEUR-90,LARGEUR-100,HAUTEUR-60,al_map_rgb(40,40,80));
    al_draw_filled_rectangle(LARGEUR-700,HAUTEUR-90,LARGEUR-700+(int)(600*p),HAUTEUR-60,al_map_rgb(50,200,100));
    al_draw_rectangle(LARGEUR-700,HAUTEUR-90,LARGEUR-100,HAUTEUR-60,al_map_rgb(255,255,255),3);
}

/* Fondu noir + texte de transition entre niveaux */
void transition(ALLEGRO_FONT *f,int n){
    char msg[32]; sprintf(msg,"NIVEAU %d",n);
    for(int a=0;a<=60;a++){al_draw_filled_rectangle(0,0,LARGEUR,HAUTEUR,al_map_rgba(0,0,0,(a*255)/60));al_flip_display();al_rest(1.0/60);}
    al_clear_to_color(al_map_rgb(0,0,0));
    al_draw_text(f,al_map_rgb(0,200,255),LARGEUR/2,HAUTEUR/2-80,ALLEGRO_ALIGN_CENTRE,msg);
    al_draw_text(f,al_map_rgb(200,200,200),LARGEUR/2,HAUTEUR/2+60,ALLEGRO_ALIGN_CENTRE,"Preparez-vous !");
    al_flip_display();al_rest(2.0);
    for(int a=60;a>=0;a--){al_clear_to_color(al_map_rgb(0,0,0));al_draw_text(f,al_map_rgb(0,200,255),LARGEUR/2,HAUTEUR/2-80,ALLEGRO_ALIGN_CENTRE,msg);al_draw_filled_rectangle(0,0,LARGEUR,HAUTEUR,al_map_rgba(0,0,0,(a*255)/60));al_flip_display();al_rest(1.0/60);}
}

/*BOUCLE PRINCIPALE D'UN NIVEAU*/
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

    /* ---- LA SUITE EST DANS LA PARTIE 2 (Mathis) ---- */
}
