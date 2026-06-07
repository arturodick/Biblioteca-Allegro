/*
============================================================
 SISTEMA DE GESTION DE BIBLIOTECA PERSONAL 
============================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#define ANCHO_VENTANA  900
#define ALTO_VENTANA   600
#define COL_FONDO      al_map_rgb(34, 40, 49)

/* Variables Globales de Control de Entorno */
ALLEGRO_DISPLAY     *display     = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;

int iniciarAllegro(void) {
    if (!al_init()) return 0;
    if (!al_install_keyboard()) return 0;
    if (!al_init_primitives_addon()) return 0;

    al_set_new_display_flags(ALLEGRO_WINDOWED);
    display = al_create_display(ANCHO_VENTANA, ALTO_VENTANA);
    if (!display) return 0;

    event_queue = al_create_event_queue();
    if (!event_queue) return 0;

    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_display_event_source(display));

    return 1;
}

int main(void) {
    if (!iniciarAllegro()) {
        fprintf(stderr, "Error al inicializar la interfaz gráfica.\n");
        return -1;
    }

    int ejecucion = 1;
    ALLEGRO_EVENT ev;

    while (ejecucion) {
        al_clear_to_color(COL_FONDO);
        al_flip_display();

        al_wait_for_event(event_queue, &ev);
        if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                ejecucion = 0;
            }
        } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            ejecucion = 0;
        }
    }

    al_destroy_event_queue(event_queue);
    al_destroy_display(display);
    return 0;
}