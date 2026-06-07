/*
============================================================
 SISTEMA DE GESTION DE BIBLIOTECA PERSONAL 
============================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { DISPONIBLE, PRESTADO } EstadoObjeto;

typedef struct {
    int    id;
    char   titulo[100];
    char   autor[100];
    int    anio;
    char   genero[50];
    EstadoObjeto estado;
} Articulo;

void agregarArticulo(Articulo **inv, int *total) {
    Articulo nuevo;
    nuevo.id = *total + 1; //🐸 Asignacion ID lineal basica 
    
    printf("Titulo: ");
    fgets(nuevo.titulo, 100, stdin);
    nuevo.titulo[strcspn(nuevo.titulo, "\n")] = '\0';
    
    printf("Anio: ");
    scanf("%d", &nuevo.anio);
    getchar(); //🐸 Limpieza basica del buffer
    
    nuevo.estado = DISPONIBLE;

    *inv = (Articulo *)realloc(*inv, (*total + 1) * sizeof(Articulo));
    (*inv)[*total] = nuevo;
    (*total)++;
}

void guardarArchivoBinario(Articulo *inv, int total) {
    FILE *fp = fopen("biblioteca.dat", "wb");
    if (fp) {
        fwrite(&total, sizeof(int), 1, fp);
        fwrite(inv, sizeof(Articulo), total, fp);
        fclose(fp);
    }
}

void cargarArchivoBinario(Articulo **inv, int *total) {
    FILE *fp = fopen("biblioteca.dat", "rb");
    if (fp) {
        fread(total, sizeof(int), 1, fp);
        *inv = (Articulo *)malloc((*total) * sizeof(Articulo));
        fread(*inv, sizeof(Articulo), *total, fp);
        fclose(fp);
    }
}

int main(void) {
    int total = 0;
    Articulo *inv = NULL;
    
    cargarArchivoBinario(&inv, &total);
    printf("Articulos cargados: %d\n", total);
    
    agregarArticulo(&inv, &total);
    guardarArchivoBinario(inv, total);
    
    free(inv);
    return 0;
}