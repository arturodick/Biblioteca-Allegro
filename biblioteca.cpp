/*
============================================================
 SISTEMA DE GESTION DE BIBLIOTECA PERSONAL 
 Estructura analítica inicial y prototipado.
============================================================
 */
#include <stdio.h>
#include <stdlib.h>

// Enumeraciones y estructuras
typedef enum {
    DISPONIBLE,
    PRESTADO
} EstadoObjeto;

typedef struct {
    int    id;
    char   titulo[100];
    char   autor[100];
    int    anio;
    char   genero[50];
    EstadoObjeto estado;
} Articulo;

// Variables globales auxiliares
Articulo *inventario = NULL;
int totalArticulos = 0;

// Prototipos de funciones 
void cargarArchivoBinario(Articulo **inv, int *total);
void guardarArchivoBinario(Articulo *inv, int total);
void agregarArticulo(Articulo **inv, int *total);
void buscarArticulo(Articulo *inv, int total);
void mostrarTodos(Articulo *inv, int total);
void modificarArticulo(Articulo *inv, int total);
void eliminarArticulo(Articulo **inv, int *total);

int main(void) {
    // Estructura para pruebas de compilacion 
    printf("Sistema de Biblioteca Personal - Estructura Inicial Cargada.\n");
    return 0;
}