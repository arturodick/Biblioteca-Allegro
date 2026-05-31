#include <iostream>
#include <string>
#include <conio.h>
#include <allegro5/allegro.h> //libreria base de allegro

using namespace std;

//usaremos un ENUM para que solo acepte las palabras q le digamos
enum EstadoObjeto{
    DISPONIBLE,
    PRESTADO
};

//usamos struct en la ficha de cada articulo de la biblioteca
struct Articulo{
    int id;
    string titulo;
    string Autor;
    int anio;
    string genero;
    EstadoObjeto estado; //este guarda si esta disponible o prestado
};

int main(){
    
    //hay que inicializar el motor de allegro
    if(!al_init()){
        cout<<"ERROR AL INICIAR"<<endl;
        return -1;
    }

    //vamos a crear la ventana (ancho de 800, alto de 700)
    ALLEGRO_DISPLAY *ventana = al_create_display(800,700);
    if(!ventana){
        cout<<"ERROR al crear VENTANA"<<endl;
        return -1;
    }
    al_set_window_title(ventana, "BIBLIOTECA PERSONAL"); //titulo de la ventana

    //vamos a crear la cola de eventos, esto detecta clics, teclas, etc
    ALLEGRO_EVENT_QUEUE *cola_de_eventos = al_create_event_queue();

    //temporizador para q corra a 60 cuadros x segundo
    ALLEGRO_TIMER *reloj= al_create_timer(1.0 / 60.0);

    //la cola vigilara las cosas o acciones que le pasen a la ventana. COLA DE EVENTOS
    al_register_event_source(cola_de_eventos, al_get_display_event_source(ventana));
    al_register_event_source(cola_de_eventos, al_get_timer_event_source(reloj));

    //enciende el reloj para que corra
    al_start_timer(reloj);


    bool corriendo = true;
    bool dibujar = true; //aqui avisa cuando toca pintar la pantalla

    //AQUI esta el bucle PRINCIPAL
    while(corriendo){
        ALLEGRO_EVENT evento;

        //aqui el programa se quedara hasta q pase algo
        al_wait_for_event(cola_de_eventos, &evento);

        //el evento es cada tic del reloj, entonces pinta
        if(evento.type == ALLEGRO_EVENT_TIMER){
            dibujar = true;
        }

        //si el usuario cierra la ventana con la X
        if(evento.type ==ALLEGRO_EVENT_DISPLAY_CLOSE){
            corriendo = false; //cierra ciclo para salir
        }

        //si el reloj da permiso y ya no hay eventos pendientes
        if(dibujar && al_is_event_queue_empty(cola_de_eventos)){
            dibujar = false; //reseteamos bandera

        //en esta parte se uso ia para guiarnos en como seleccionar colores, se explico
        //el color en formato RGB: Rojo, Verde, Azul. los valores son de 0 a 255
        al_clear_to_color(al_map_rgb(34, 40, 49));

        //mostrar en la pantalla todo lo pintado
        al_flip_display();
        }
    }

    //vamos a limpiar todo, al cerrar borramos lo creado para limpiar la memoria
    al_destroy_event_queue(cola_de_eventos);
    al_destroy_display(ventana);
    al_destroy_timer(reloj);

    cout<<"VENTANA CERRADA"<<endl;

    getch();
    return 0;
} 