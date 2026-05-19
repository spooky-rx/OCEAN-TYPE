/* jeu.h : notre fichier d'en-tete principal.
   Il inclut entites.h et sons.h, et declare lancer_niveau().
   On en a besoin ici pour les constantes LARGEUR, HAUTEUR,
   VICTOIRE, GAME_OVER et MENU definies dans globals.h. */
#include "jeu.h"

/* allegro_font.h : permet d'utiliser al_draw_text()
   et al_create_builtin_font() pour afficher du texte. */
#include <allegro5/allegro_font.h>

/* allegro_ttf.h : permet de charger des polices .ttf
   avec al_load_ttf_font(). */
#include <allegro5/allegro_ttf.h>

/* allegro_primitives.h : permet de dessiner des formes
   comme al_clear_to_color() pour effacer l'ecran. */
#include <allegro5/allegro_primitives.h>

/* stdio.h : necessaire pour sprintf() qui formate
   le texte du score. */
#include <stdio.h>


/* ================================================================
   FONCTION ecran_fin()
   Affiche l'ecran de fin apres une partie.
   Deux cas : victoire (L'OCEAN EST SAUVE) ou defaite (ENGLOUTI).
   Attend que le joueur appuie sur ENTREE ou ECHAP pour revenir au menu.
   ================================================================ */

/* void = ne retourne rien.
   ALLEGRO_DISPLAY *d = pointeur vers la fenetre du jeu.
   int score = score final du joueur a afficher.
   bool victoire = true si le joueur a gagne, false si game over. */
void ecran_fin(ALLEGRO_DISPLAY *d, int score, bool victoire) {

    /* Cree la file d'evenements pour recevoir
       les touches clavier, le timer et la fermeture de fenetre. */
    ALLEGRO_EVENT_QUEUE *q = al_create_event_queue();

    /* Cree un timer a 60fps pour cadencer le rafraichissement
       de l'ecran de fin. 1.0/60.0 = 0.0166 secondes entre chaque tick. */
    ALLEGRO_TIMER *t = al_create_timer(1.0 / 60.0);

    /* f1 : grande police en taille 160px pour le titre principal
       ("L'OCEAN EST SAUVE !" ou "LES PROFONDEURS VOUS ONT ENGLOUTI..."). */
    ALLEGRO_FONT *f1 = al_load_ttf_font("font.ttf", 160, 0);

    /* f2 : police plus petite en taille 65px pour les sous-titres
       et le score. */
    ALLEGRO_FONT *f2 = al_load_ttf_font("font.ttf", 65, 0);

    /* Si font.ttf est introuvable : utilise la police par defaut d'Allegro.
       Tres basique mais evite un crash. */
    if (!f1) f1 = al_create_builtin_font();
    if (!f2) f2 = al_create_builtin_font();

    /* Abonne la file aux evenements de fermeture de fenetre
       (croix ou Alt+F4). */
    al_register_event_source(q, al_get_display_event_source(d));

    /* Abonne la file aux ticks du timer (60 fois par seconde). */
    al_register_event_source(q, al_get_timer_event_source(t));

    /* Abonne la file aux evenements clavier (ENTREE, ECHAP). */
    al_register_event_source(q, al_get_keyboard_event_source());

    /* Demarre le timer. Sans ca il est cree mais n'envoie rien. */
    al_start_timer(t);

    /* fini = false : controle la boucle while.
       Mis a true quand le joueur appuie sur ENTREE ou ECHAP. */
    bool fini = false;

    /* ================================================================
       BOUCLE D'AFFICHAGE DE L'ECRAN DE FIN
       Tourne jusqu'a ce que le joueur appuie sur une touche.
       ================================================================ */
    while (!fini) {

        /* Attend le prochain evenement avant de continuer.
           Sans ca la boucle tournerait a vide en consommant 100% du CPU. */
        ALLEGRO_EVENT ev;
        al_wait_for_event(q, &ev);

        /* Si l'utilisateur ferme la fenetre : on sort de la boucle. */
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) fini = true;

        /* Si le joueur appuie sur ENTREE ou ECHAP : on sort.
           Les deux touchent font la meme chose ici : revenir au menu.
           ev.keyboard.keycode = code de la touche enfoncee. */
        if (ev.type == ALLEGRO_EVENT_KEY_DOWN &&
           (ev.keyboard.keycode == ALLEGRO_KEY_ENTER ||
            ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)) fini = true;

        /* A chaque tick du timer (60fps) : on redessine l'ecran.
           On le fait dans le timer et pas directement pour avoir
           un rafraichissement fluide et regulier. */
        if (ev.type == ALLEGRO_EVENT_TIMER) {

            /* buf[64] : tableau de caracteres pour le texte du score.
               64 caracteres suffisent pour "Score : 001500". */
            char buf[64];

            /* Efface l'ecran avec un bleu tres sombre (presque noir).
               al_map_rgb(0,10,40) = R=0, G=10, B=40. */
            al_clear_to_color(al_map_rgb(0, 10, 40));

            /* Affiche le texte selon le resultat de la partie. */
            if (victoire) {

                /* Victoire : titre en bleu cyan a y=500.
                   f1 = grande police 160px.
                   al_map_rgb(50,200,255) = bleu cyan.
                   LARGEUR/2 = centre horizontal de l'ecran.
                   ALLEGRO_ALIGN_CENTRE = centre le texte sur la position X. */
                al_draw_text(f1, al_map_rgb(50, 200, 255),
                             LARGEUR/2, 500,
                             ALLEGRO_ALIGN_CENTRE, "L'OCEAN EST SAUVE !");

                /* Sous-titre en bleu clair a y=900.
                   f2 = police plus petite 65px. */
                al_draw_text(f2, al_map_rgb(150, 230, 255),
                             LARGEUR/2, 900,
                             ALLEGRO_ALIGN_CENTRE, "Vous avez vaincu les envahisseurs !");

            } else {

                /* Game Over : titre en rouge vif a y=500.
                   al_map_rgb(255,60,60) = rouge. */
                al_draw_text(f1, al_map_rgb(255, 60, 60),
                             LARGEUR/2, 500,
                             ALLEGRO_ALIGN_CENTRE, "LES PROFONDEURS VOUS ONT ENGLOUTI...");

                /* Sous-titre en rouge sombre a y=900. */
                al_draw_text(f2, al_map_rgb(200, 100, 100),
                             LARGEUR/2, 900,
                             ALLEGRO_ALIGN_CENTRE, "La mer vous offre une seconde chance.");
            }

            /* sprintf(buf, "Score : %06d", score) :
               Formate le score dans buf.
               %06d = affiche sur 6 chiffres avec des zeros devant.
               Ex: score=1500 -> buf contient "Score : 001500". */
            sprintf(buf, "Score : %06d", score);

            /* Affiche le score en jaune a y=1300. */
            al_draw_text(f2, al_map_rgb(255, 220, 0),
                         LARGEUR/2, 1300,
                         ALLEGRO_ALIGN_CENTRE, buf);

            /* Affiche l'instruction en gris-bleu a y=1700.
               Indique au joueur comment revenir au menu. */
            al_draw_text(f2, al_map_rgb(160, 160, 200),
                         LARGEUR/2, 1700,
                         ALLEGRO_ALIGN_CENTRE, "ENTREE pour revenir au menu");

            /* Envoie tout ce qu'on vient de dessiner a l'ecran.
               Sans ca rien ne serait visible (double buffering). */
            al_flip_display();
        }
    }

    /* ================================================================
       NETTOYAGE MEMOIRE
       Libere toutes les ressources allouees dans cette fonction.
       Sans ca la memoire resterait occupee indefiniment.
       ================================================================ */

    /* Libere les deux polices de caracteres. */
    al_destroy_font(f1);
    al_destroy_font(f2);

    /* Libere la file d'evenements. */
    al_destroy_event_queue(q);

    /* Libere le timer. */
    al_destroy_timer(t);
}
