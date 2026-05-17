/*
 * ================================================================
 *  jeu.c  —  PARTIE 2 / 2   (Mathis)
 *  Contient :
 *    - La boucle principale while(!fini)
 *    - Gestion des evenements clavier
 *    - Logique de jeu (scrolling, vagues, tirs, collisions)
 *    - Rendu a l'ecran (dessiner tout)
 *    - Nettoyage memoire a la fin du niveau
 *  L'initialisation est dans la PARTIE 1 (Mathieu)
 * ================================================================
 */

    /* ================================================================
       BOUCLE PRINCIPALE — tourne 60 fois par seconde
       A chaque iteration : on attend un evenement Allegro,
       on le traite, puis on dessine si la file est vide.
       ================================================================ */
    while (!fini) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(q, &ev); /* attend le prochain evenement */

        /* --- Fermeture de la fenetre (croix ou Alt+F4) --- */
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            res = MENU;
            fini = true;
        }

        /* --- Evenements clavier --- */
        if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            int k = ev.keyboard.keycode;

            /* P ou ECHAP : met le jeu en pause */
            if (k == ALLEGRO_KEY_P || k == ALLEGRO_KEY_ESCAPE)
                pause = true;

            /* R : reprendre depuis la pause */
            if (pause && k == ALLEGRO_KEY_R)
                pause = false;

            /* M : retour au menu depuis la pause */
            if (pause && k == ALLEGRO_KEY_M) {
                res  = MENU;
                fini = true;
            }

            /* SPACE : tirer (cooldown de 8 frames entre deux tirs) */
            if (!pause && k == ALLEGRO_KEY_SPACE && cd <= 0) {
                tirer(tj, j->x + 200, j->y, i_tj);
                jouer_effet(s->tir); /* son du tir sans couper la musique */
                cd = 8;
            }
        }

        /* --- Tick du timer : logique de jeu (seulement si pas en pause) --- */
        if (ev.type == ALLEGRO_EVENT_TIMER && !pause) {

            /* Tir maintenu : on peut garder SPACE appuye */
            ALLEGRO_KEYBOARD_STATE ks;
            al_get_keyboard_state(&ks);
            if (al_key_down(&ks, ALLEGRO_KEY_SPACE) && cd <= 0) {
                tirer(tj, j->x + 200, j->y, i_tj);
                jouer_effet(s->tir);
                cd = 8;
            }
            if (cd > 0) cd--; /* decremente le cooldown a chaque frame */

            /* Scrolling du fond : -3 pixels par frame
             * Quand le fond sort completement (fx <= -LARGEUR),
             * on le remet a 0 pour creer une boucle infinie.
             * On dessine 2 fois le fond cote a cote pour eviter le noir. */
            if (!pboss) {
                fx -= 3;
                if (fx <= -LARGEUR) fx = 0;
            }

            /* Spawn des vagues d'ennemis
             * tv = timer qui decremente chaque frame
             * Quand il atteint 0 on fait apparaitre la prochaine vague
             * Les donnees des vagues viennent du tableau V dans niveaux.c */
            if (!pboss && --tv <= 0) {
                if (vague < 8) {
                    float (*v)[5] = V[num-1][vague]; /* vague du niveau num */
                    for (int k = 0; k < 6; k++)
                        if (v[k][0] > 0) /* 0 = pas d'ennemi a cette position */
                            spawn(en, v[k][0], v[k][1], (bool)v[k][2],
                                  (int)v[k][3], (int)v[k][4], i_e, i_t);
                    vague++;
                }
                /* Verifie si toutes les vagues sont passees ET tous morts */
                bool tous = true;
                for (int i = 0; i < NB_ENNEMIS; i++)
                    if (en[i].actif) { tous = false; break; }
                if (vague >= 8 && tous) {
                    if (num == 3) pboss = true;       /* niveau 3 : boss */
                    else { res = VICTOIRE; fini = true; } /* sinon : victoire */
                }
                tv = 400; /* remet le timer a 400 frames (~6.6 secondes) */
            }

            /* Deplacement et tirs des ennemis
             * ENNEMI_VOLANT : avance vers la gauche (vx = -12)
             * ENNEMI_FIXE   : ne bouge pas (vx = 0)
             * Chaque ennemi a un timer_tir qui decremente.
             * Quand il atteint 0, l'ennemi tire vers le joueur. */
            for (int i = 0; i < NB_ENNEMIS; i++) {
                if (!en[i].actif) continue;

                /* Deplacement si volant */
                if (!en[i].fixe) {
                    en[i].x += en[i].vx;
                    if (en[i].x < -160) en[i].actif = false; /* sorti de l'ecran */
                }

                /* Tir vers le joueur */
                if (--en[i].timer_tir <= 0) {
                    /* Calcul de la direction : vecteur ennemi -> joueur */
                    float dx  = j->x - en[i].x;
                    float dy  = j->y - en[i].y;
                    float dst = sqrt(dx*dx + dy*dy);
                    /* Normalisation : divise par la distance pour obtenir
                     * un vecteur de longueur 1, puis multiplie par 20
                     * pour obtenir une vitesse constante quelle que soit
                     * la distance entre l'ennemi et le joueur */
                    if (dst > 0) { dx /= dst; dy /= dst; }
                    tir_e(te, en[i].x, en[i].y + 60, dx * 20, dy * 20, i_te);
                    en[i].timer_tir = en[i].cadence; /* remet le timer */
                }
            }

            /* Comportement du boss (niveau 3 uniquement)
             * Phase 1 : entre depuis la droite jusqu'a LARGEUR-600
             * Phase 2 : apparu == true, tire 3 projectiles en eventail */
            if (pboss && boss.actif) {
                if (!boss.apparu) {
                    boss.x -= 8;
                    if (boss.x <= LARGEUR - 600) boss.apparu = true;
                }
                if (boss.apparu && --boss.timer_tir <= 0) {
                    /* 3 tirs : un droit, un en haut, un en bas */
                    tir_e(te, boss.x, boss.y+150, -24,   0, i_te);
                    tir_e(te, boss.x, boss.y+150, -20,  12, i_te);
                    tir_e(te, boss.x, boss.y+150, -20, -12, i_te);
                    boss.timer_tir = 55;
                }
            }

            /* Mouvement des tirs joueur : avancent vers la droite */
            for (int i = 0; i < NB_TIRS; i++)
                if (tj[i].actif) {
                    tj[i].x += tj[i].vx;
                    if (tj[i].x > LARGEUR) tj[i].actif = false; /* sorti */
                }

            /* Mouvement des tirs ennemis : avancent dans leur direction */
            for (int i = 0; i < NB_TIRS_ENE; i++)
                if (te[i].actif) {
                    te[i].x += te[i].vx;
                    te[i].y += te[i].vy;
                    if (te[i].x < 0 || te[i].x > LARGEUR ||
                        te[i].y < 0 || te[i].y > HAUTEUR)
                        te[i].actif = false;
                }

            /* Timer des explosions : elles durent 35 frames puis disparaissent */
            for (int i = 0; i < NB_EXPS; i++)
                if (ex[i].actif && --ex[i].timer <= 0)
                    ex[i].actif = false;

            /* Deplacement du joueur (lecture clavier dans joueur.c) */
            maj_joueur(j);

            /* Collisions dans l'ordre (5 fonctions de collision.c)
             * L'ordre est important : on traite les tirs joueur en premier
             * pour qu'un ennemi detruit ne puisse plus toucher le joueur */
            col_tirs_ennemis(tj, en, ex, j, i_ex);  /* 1. tirs joueur vs ennemis */
            if (pboss) col_tirs_boss(tj, &boss, ex, j, i_ex); /* 2. tirs joueur vs boss */
            col_tirs_joueur(te, j, ex, i_ex);         /* 3. tirs ennemis vs joueur */
            col_joueur_ennemis(j, en, ex, i_ex);      /* 4. contact joueur/ennemi */
            if (pboss) col_joueur_boss(j, &boss, ex, i_ex);   /* 5. contact joueur/boss */

            /* Mort du joueur : respawn si vies restantes, sinon game over */
            if (!j->vivant) {
                if (j->vies > 0) respawn_joueur(j);
                else { res = GAME_OVER; fini = true; }
            }

            /* Boss vaincu : victoire du niveau 3 */
            if (pboss && !boss.actif) { res = VICTOIRE; fini = true; }
        }

        /* ================================================================
           RENDU — dessine tout a l'ecran si plus d'evenements en attente
           On dessine dans cet ordre : fond -> ennemis -> boss -> tirs
           -> explosions -> joueur -> HUD (toujours le joueur par-dessus)
           ================================================================ */
        if (al_is_event_queue_empty(q)) {
            al_clear_to_color(al_map_rgb(0, 0, 0));

            /* Fond en double pour eviter le noir pendant le scrolling */
            al_draw_bitmap(i_fond, fx,           0, 0);
            al_draw_bitmap(i_fond, fx + LARGEUR, 0, 0);

            /* Ennemis */
            for (int i = 0; i < NB_ENNEMIS; i++)
                if (en[i].actif)
                    al_draw_bitmap(en[i].img, en[i].x, en[i].y, 0);

            /* Boss + barre de vie */
            if (pboss && boss.actif) {
                al_draw_bitmap(boss.img, boss.x, boss.y, 0);
                int bw = (boss.pv * 800) / PV_BOSS; /* largeur proportionnelle aux PV */
                al_draw_filled_rectangle(40, 40, 840, 88, al_map_rgb(80,  0,  0)); /* fond rouge sombre */
                al_draw_filled_rectangle(40, 40, 40+bw, 88, al_map_rgb(220, 40, 40)); /* barre rouge */
                al_draw_rectangle(40, 40, 840, 88, al_map_rgb(255, 255, 255), 4);  /* contour blanc */
            }

            /* Tirs joueur */
            for (int i = 0; i < NB_TIRS; i++)
                if (tj[i].actif)
                    al_draw_bitmap(tj[i].img, tj[i].x, tj[i].y, 0);

            /* Tirs ennemis */
            for (int i = 0; i < NB_TIRS_ENE; i++)
                if (te[i].actif)
                    al_draw_bitmap(te[i].img, te[i].x, te[i].y, 0);

            /* Explosions avec transparence alpha
             * ALLEGRO_ALPHA = respecte le canal alpha du PNG
             * On remet le blender par defaut apres chaque explosion */
            for (int i = 0; i < NB_EXPS; i++) {
                if (ex[i].actif) {
                    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
                    al_draw_bitmap(ex[i].img, ex[i].x, ex[i].y, 0);
                    al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE,   ALLEGRO_INVERSE_ALPHA);
                }
            }

            /* Joueur (avec clignotement si invincible — gere dans joueur.c) */
            dessiner_joueur(j);

            /* HUD : vies, score, barre de progression */
            hud(j, font, vague, pboss);

            /* Ecran de pause semi-transparent par-dessus tout */
            if (pause) {
                al_draw_filled_rectangle(0, 0, LARGEUR, HAUTEUR,
                                         al_map_rgba(0, 0, 0, 170));
                al_draw_text(font, al_map_rgb(0,   200, 255),
                             LARGEUR/2, HAUTEUR/2 - 200,
                             ALLEGRO_ALIGN_CENTRE, "PAUSE");
                al_draw_text(font, al_map_rgb(255, 255, 255),
                             LARGEUR/2, HAUTEUR/2,
                             ALLEGRO_ALIGN_CENTRE, "R  -  Reprendre");
                al_draw_text(font, al_map_rgb(255, 180, 100),
                             LARGEUR/2, HAUTEUR/2 + 150,
                             ALLEGRO_ALIGN_CENTRE, "M  -  Menu");
            }

            /* Envoie le rendu a l'ecran (echange les buffers) */
            al_flip_display();
        }
    } /* fin de la boucle while(!fini) */

    /* Animation de transition si victoire et pas dernier niveau */
    if (res == VICTOIRE && num < 3)
        transition(font, num + 1);

    /* Nettoyage : liberation de toute la memoire du niveau
     * Chaque al_destroy_bitmap libere la memoire de l'image.
     * Sans ca, la memoire resterait occupee entre les niveaux. */
    al_destroy_bitmap(i_fond); al_destroy_bitmap(i_j);
    al_destroy_bitmap(i_e);    al_destroy_bitmap(i_t);
    al_destroy_bitmap(i_b);    al_destroy_bitmap(i_tj);
    al_destroy_bitmap(i_te);   al_destroy_bitmap(i_ex);
    al_destroy_event_queue(q);
    al_destroy_timer(tmr);
    if (font) al_destroy_font(font);

    return res; /* VICTOIRE, GAME_OVER ou MENU */
}
