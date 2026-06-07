/*
============================================================
 SISTEMA DE GESTION DE BIBLIOTECA PERSONAL 
============================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Allegro 5 */
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#ifdef _WIN32
   #include <conio.h>
#else
   #include <termios.h>
   #include <unistd.h>
   static int _getch(void) {
       struct termios oldt, newt;
       int ch;
       tcgetattr(STDIN_FILENO, &oldt);
       newt = oldt;
       newt.c_lflag &= ~(ICANON | ECHO);
       tcsetattr(STDIN_FILENO, TCSANOW, &newt);
       ch = getchar();
       tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
       return ch;
   }
#endif

typedef enum { DISPONIBLE, PRESTADO } EstadoObjeto;

typedef struct {
    int            id;
    char           titulo[100];
    char           autor[100];
    int            anio;
    char           genero[50];
    EstadoObjeto   estado;
} Articulo;

Articulo *inventario = NULL;
int       totalArticulos = 0;

ALLEGRO_DISPLAY       *display      = NULL;
ALLEGRO_EVENT_QUEUE   *event_queue  = NULL;
ALLEGRO_FONT          *font_titulo  = NULL;
ALLEGRO_FONT          *font_normal  = NULL;
ALLEGRO_FONT          *font_small   = NULL;

#define ANCHO_VENTANA  900
#define ALTO_VENTANA   600

#define COL_FONDO       al_map_rgb(34,  40,  49)
#define COL_PANEL       al_map_rgb(44,  51,  63)
#define COL_ACENTO      al_map_rgb(255,211,105)
#define COL_TEXTO       al_map_rgb(238,238,238)
#define COL_SUBTEXTO    al_map_rgb(170,180,195)
#define COL_BORDE       al_map_rgb(80,  90, 110)
#define COL_DISPONIBLE  al_map_rgb(90, 200, 120)
#define COL_PRESTADO    al_map_rgb(230, 90,  90)
#define COL_BARRA_BG    al_map_rgb(55,  65,  80)
#define COL_BARRA_FG    al_map_rgb(255,211,105)

#define ARCHIVO_BINARIO  "biblioteca.dat"
#define ARCHIVO_REPORTE  "reporte.txt"

/* --- PROTOTIPOS COMPLETOS --- */
void cargarArchivoBinario(Articulo **inv, int *total);
void guardarArchivoBinario(Articulo *inv, int total);
void generarReporteTexto(Articulo *inv, int total);
void agregarArticulo(Articulo **inv, int *total);
void buscarArticulo(Articulo *inv, int total);
void mostrarTodos(Articulo *inv, int total);
void modificarArticulo(Articulo *inv, int total);
void eliminarArticulo(Articulo **inv, int *total);
int  generarNuevoID(Articulo *inv, int total);
int  iniciarAllegro(void);
void cerrarAllegro(void);
void dibujarFondo(void);
void dibujarMenu(int seleccion);
void dibujarTabla(Articulo *inv, int total, int pagina);
void dibujarMensaje(const char *linea1, const char *linea2, const char *linea3);
void barraProgreso(const char *etiqueta, float progreso);
void dibujarResultadoBusqueda(Articulo *art);
void limpiarVentana(void);
const char *estadoStr(EstadoObjeto e);
void limpiarBuffer(void);
void pausaConsola(void);

const char *estadoStr(EstadoObjeto e) {
    return (e == DISPONIBLE) ? "DISPONIBLE" : "PRESTADO";
}

void limpiarBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pausaConsola(void) {
    printf("\n  Presiona cualquier tecla para continuar...");
    _getch();
}

void barraProgreso(const char *etiqueta, float progreso) {
    if (!display) return;
    dibujarFondo();
    al_draw_text(font_titulo, COL_ACENTO, ANCHO_VENTANA/2, ALTO_VENTANA/2 - 60, ALLEGRO_ALIGN_CENTRE, etiqueta);
    
    float bw = 400, bh = 30;
    float bx = ANCHO_VENTANA/2 - bw/2;
    float by = ALTO_VENTANA/2 - bh/2;
    
    al_draw_filled_rectangle(bx, by, bx+bw, by+bh, COL_BARRA_BG);
    al_draw_filled_rectangle(bx, by, bx+(bw*progreso), by+bh, COL_BARRA_FG);
    al_draw_rectangle(bx, by, bx+bw, by+bh, COL_BORDE, 2.0f);
    al_flip_display();
}

void cargarArchivoBinario(Articulo **inv, int *total) {
    FILE *fp;
    int leidos = 0;
    barraProgreso("Cargando biblioteca...", 0.0f);
    fp = fopen(ARCHIVO_BINARIO, "rb");
    if (fp == NULL) {
        barraProgreso("Cargando biblioteca...", 1.0f);
        al_rest(0.5);
        return;
    }
    fread(&leidos, sizeof(int), 1, fp);
    if (leidos > 0) {
        *inv = (Articulo *)malloc(leidos * sizeof(Articulo));
        for (int i = 0; i < leidos; i++) {
            fread(&((*inv)[i]), sizeof(Articulo), 1, fp);
            barraProgreso("Cargando biblioteca...", (float)(i + 1) / leidos);
            al_rest(0.03);
        }
        *total = leidos;
    }
    fclose(fp);
    barraProgreso("Cargando biblioteca...", 1.0f);
    al_rest(0.4);
}

void guardarArchivoBinario(Articulo *inv, int total) {
    FILE *fp = fopen(ARCHIVO_BINARIO, "wb");
    if (fp == NULL) return;
    fwrite(&total, sizeof(int), 1, fp);
    for (int i = 0; i < total; i++) {
        fwrite(&inv[i], sizeof(Articulo), 1, fp);
        barraProgreso("Guardando biblioteca...", (float)(i + 1) / (total > 0 ? total : 1));
        al_rest(0.04);
    }
    fclose(fp);
    barraProgreso("Guardando biblioteca...", 1.0f);
    al_rest(0.5);
}

void generarReporteTexto(Articulo *inv, int total) {
    FILE *fp = fopen(ARCHIVO_REPORTE, "w");
    if (fp == NULL) return;
    fprintf(fp, "============================================================\n");
    fprintf(fp, "         REPORTE - SISTEMA DE BIBLIOTECA PERSONAL\n");
    fprintf(fp, "============================================================\n\n");
    fprintf(fp, "Total de articulos: %d\n\n", total);
    for (int i = 0; i < total; i++) {
        fprintf(fp, "------------------------------------------------------------\n");
        fprintf(fp, "ID     : %d\n", inv[i].id);
        fprintf(fp, "Titulo : %s\n", inv[i].titulo);
        fprintf(fp, "Autor  : %s\n", inv[i].autor);
        fprintf(fp, "Anio   : %d\n", inv[i].anio);
        fprintf(fp, "Genero : %s\n", inv[i].genero);
        fprintf(fp, "Estado : %s\n\n", estadoStr(inv[i].estado));
    }
    fclose(fp);
    printf("\n  [OK] Reporte generado en '%s'\n", ARCHIVO_REPORTE);
}

int generarNuevoID(Articulo *inv, int total) {
    int maxID = 0;
    for (int i = 0; i < total; i++) {
        if (inv[i].id > maxID) maxID = inv[i].id;
    }
    return maxID + 1;
}

void agregarArticulo(Articulo **inv, int *total) {
    Articulo nuevo;
    int opEstado;
    dibujarMensaje("INGRESANDO NUEVO ARTICULO", "Revisa la terminal para ingresar los datos.", "Presiona ENTER en la terminal cuando termines.");
    
    printf("\n  ============ NUEVO ARTICULO ============\n");
    nuevo.id = generarNuevoID(*inv, *total);
    printf("  ID asignado automaticamente: %d\n\n", nuevo.id);
    printf("  Titulo  : ");
    limpiarBuffer();
    fgets(nuevo.titulo, sizeof(nuevo.titulo), stdin);
    nuevo.titulo[strcspn(nuevo.titulo, "\n")] = '\0';
    printf("  Autor   : ");
    fgets(nuevo.autor, sizeof(nuevo.autor), stdin);
    nuevo.autor[strcspn(nuevo.autor, "\n")] = '\0';
    printf("  Anio    : ");
    scanf("%d", &nuevo.anio);
    limpiarBuffer();
    printf("  Genero  : ");
    fgets(nuevo.genero, sizeof(nuevo.genero), stdin);
    nuevo.genero[strcspn(nuevo.genero, "\n")] = '\0';
    printf("  Estado  (0=DISPONIBLE, 1=PRESTADO): ");
    scanf("%d", &opEstado);
    limpiarBuffer();
    nuevo.estado = (opEstado == 1) ? PRESTADO : DISPONIBLE;

    Articulo *temp = (Articulo *)realloc(*inv, (*total + 1) * sizeof(Articulo));
    if (temp == NULL) return;
    *inv = temp;
    (*inv)[*total] = nuevo;
    (*total)++;
    printf("\n  [OK] Articulo '%s' agregado con ID %d.\n", nuevo.titulo, nuevo.id);
    pausaConsola();
}

void buscarArticulo(Articulo *inv, int total) {
    int opcion, id, encontrado = 0;
    char titulo[100];
    dibujarMensaje("BUSCAR ARTICULO", "Revisa la terminal para ingresar la busqueda.", "");
    printf("\n  ============ BUSCAR ARTICULO ============\n");
    printf("  1. Buscar por ID\n  2. Buscar por Titulo\n  Opcion: ");
    scanf("%d", &opcion);
    limpiarBuffer();

    if (opcion == 1) {
        printf("  ID a buscar: ");
        scanf("%d", &id);
        limpiarBuffer();
        for (int i = 0; i < total; i++) {
            if (inv[i].id == id) {
                encontrado = 1;
                dibujarResultadoBusqueda(&inv[i]);
                break;
            }
        }
    } else if (opcion == 2) {
        printf("  Titulo a buscar: ");
        fgets(titulo, sizeof(titulo), stdin);
        titulo[strcspn(titulo, "\n")] = '\0';
        for (int i = 0; i < total; i++) {
            if (strstr(inv[i].titulo, titulo) != NULL) {
                encontrado = 1;
                dibujarResultadoBusqueda(&inv[i]);
                break;
            }
        }
    }
    if (!encontrado) printf("\n  [!] No se encontro el articulo.\n");
    pausaConsola();
}

void dibujarResultadoBusqueda(Articulo *art) {
    printf("\n  --- Resultado ---\n  ID     : %d\n  Titulo : %s\n  Autor  : %s\n  Estado : %s\n", art->id, art->titulo, art->autor, estadoStr(art->estado));
    char l2[150];
    snprintf(l2, sizeof(l2), "%s - %s (%d)", art->titulo, art->autor, art->anio);
    dibujarMensaje("ARTICULO ENCONTRADO", l2, estadoStr(art->estado));
}

void mostrarTodos(Articulo *inv, int total) {
    int pagina = 0, continuar = 1;
    ALLEGRO_EVENT ev;
    if (total == 0) {
        dibujarMensaje("MOSTRAR TODOS", "No hay articulos en la biblioteca.", "Presiona cualquier tecla para continuar.");
        _getch(); return;
    }
    dibujarTabla(inv, total, pagina);
    while (continuar) {
        while (al_get_next_event(event_queue, &ev)) {
            if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) continuar = 0;
                else if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                    int totalPaginas = (total + 9) / 10;
                    if (pagina < totalPaginas - 1) { pagina++; dibujarTabla(inv, total, pagina); }
                } else if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                    if (pagina > 0) { pagina--; dibujarTabla(inv, total, pagina); }
                }
            }
            if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) continuar = 0;
        }
    }
}

void dibujarTabla(Articulo *inv, int total, int pagina) {
    int inicio = pagina * 10, fin = inicio + 10;
    if (fin > total) fin = total;
    dibujarFondo();
    al_draw_filled_rectangle(20, 20, ANCHO_VENTANA-20, 65, COL_PANEL);
    al_draw_text(font_titulo, COL_ACENTO, ANCHO_VENTANA/2, 25, ALLEGRO_ALIGN_CENTRE, "INVENTARIO COMPLETO");
    
    int cx[] = { 30,  90, 310, 510, 620, 760 };
    const char *ch[] = { "ID", "TITULO", "AUTOR", "ANIO", "GENERO", "ESTADO" };
    int hy = 75;
    al_draw_filled_rectangle(20, hy, ANCHO_VENTANA-20, hy+28, COL_PANEL);
    for (int i = 0; i < 6; i++) {
        al_draw_text(font_small, COL_ACENTO, cx[i], hy+6, ALLEGRO_ALIGN_LEFT, ch[i]);
    }
    
    char buf[120];
    for (int i = inicio; i < fin; i++) {
        int fy = hy + 30 + (i - inicio) * 38;
        al_draw_filled_rectangle(20, fy, ANCHO_VENTANA-20, fy+35, (i % 2 == 0) ? al_map_rgb(40,47,58) : al_map_rgb(36,43,54));
        snprintf(buf, sizeof(buf), "%d", inv[i].id); al_draw_text(font_small, COL_TEXTO, cx[0], fy+10, 0, buf);
        snprintf(buf, sizeof(buf), "%.24s", inv[i].titulo); al_draw_text(font_small, COL_TEXTO, cx[1], fy+10, 0, buf);
        snprintf(buf, sizeof(buf), "%.22s", inv[i].autor); al_draw_text(font_small, COL_TEXTO, cx[2], fy+10, 0, buf);
        snprintf(buf, sizeof(buf), "%d", inv[i].anio); al_draw_text(font_small, COL_TEXTO, cx[3], fy+10, 0, buf);
        snprintf(buf, sizeof(buf), "%.14s", inv[i].genero); al_draw_text(font_small, COL_TEXTO, cx[4], fy+10, 0, buf);
        al_draw_text(font_small, (inv[i].estado == DISPONIBLE)?COL_DISPONIBLE:COL_PRESTADO, cx[5], fy+10, 0, estadoStr(inv[i].estado));
    }
    al_flip_display();
}

void modificarArticulo(Articulo *inv, int total) {
    int id, encontrado = 0, opEstado; char buffer[100];
    dibujarMensaje("MODIFICAR ARTICULO", "Revisa la terminal para ingresar el ID.", "");
    printf("\n  ID del articulo a modificar: "); scanf("%d", &id); limpiarBuffer();
    for (int i = 0; i < total; i++) {
        if (inv[i].id == id) {
            encontrado = 1;
            printf("  Nuevo titulo: "); fgets(buffer, 100, stdin); buffer[strcspn(buffer, "\n")] = '\0';
            if (strlen(buffer) > 0) strcpy(inv[i].titulo, buffer);
            printf("  Nuevo estado (0=DISPONIBLE, 1=PRESTADO): "); scanf("%d", &opEstado); limpiarBuffer();
            inv[i].estado = (opEstado == 1) ? PRESTADO : DISPONIBLE;
            break;
        }
    }
    if (!encontrado) printf(" No encontrado.\n");
    pausaConsola();
}

void eliminarArticulo(Articulo **inv, int *total) {
    int id, pos = -1;
    dibujarMensaje("ELIMINAR ARTICULO", "Revisa la terminal para ingresar el ID.", "");
    printf("\n  ID a eliminar: "); scanf("%d", &id); limpiarBuffer();
    for (int i = 0; i < *total; i++) {
        if ((*inv)[i].id == id) { pos = i; break; }
    }
    if (pos != -1) {
        for (int i = pos; i < *total - 1; i++) (*inv)[i] = (*inv)[i + 1];
        (*total)--;
        if (*total > 0) *inv = (Articulo *)realloc(*inv, *total * sizeof(Articulo));
        else { free(*inv); *inv = NULL; }
        printf(" Eliminado con exito.\n");
    }
    pausaConsola();
}

int iniciarAllegro(void) {
    if (!al_init() || !al_install_keyboard() || !al_init_font_addon() || !al_init_ttf_addon() || !al_init_primitives_addon()) return 0;
    display = al_create_display(ANCHO_VENTANA, ALTO_VENTANA);
    if (!display) return 0;
    event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_display_event_source(display));
    font_titulo = al_create_builtin_font();
    font_normal = al_create_builtin_font();
    font_small  = al_create_builtin_font();
    return 1;
}

void cerrarAllegro(void) {
    if (font_small) al_destroy_font(font_small);
    if (font_normal) al_destroy_font(font_normal);
    if (font_titulo) al_destroy_font(font_titulo);
    if (event_queue) al_destroy_event_queue(event_queue);
    if (display) al_destroy_display(display);
}

void dibujarFondo(void) { al_clear_to_color(COL_FONDO); }
void limpiarVentana(void) { dibujarFondo(); al_flip_display(); }

void dibujarMensaje(const char *linea1, const char *linea2, const char *linea3) {
    dibujarFondo();
    int px = ANCHO_VENTANA/2 - 280, py = ALTO_VENTANA/2 - 90;
    al_draw_filled_rectangle(px, py, px+560, py+180, COL_PANEL);
    al_draw_text(font_titulo, COL_ACENTO, ANCHO_VENTANA/2, py+16, ALLEGRO_ALIGN_CENTRE, linea1);
    al_draw_text(font_normal, COL_TEXTO, ANCHO_VENTANA/2, py+64, ALLEGRO_ALIGN_CENTRE, linea2);
    if (linea3) al_draw_text(font_small, COL_SUBTEXTO, ANCHO_VENTANA/2, py+96, ALLEGRO_ALIGN_CENTRE, linea3);
    al_flip_display();
}

void dibujarMenu(int seleccion) {
    dibujarFondo();
    al_draw_rectangle(20, 20, ANCHO_VENTANA-20, ALTO_VENTANA-20, COL_BORDE, 2.0f);
    al_draw_text(font_titulo, COL_ACENTO, ANCHO_VENTANA/2, 35, ALLEGRO_ALIGN_CENTRE, "BIBLIOTECA PERSONAL");
    
    const char *opciones[] = {
        "1. AGREGAR ARTICULO", "2. BUSCAR ARTICULO", "3. MOSTRAR TODOS",
        "4. MODIFICAR ARTICULO", "5. ELIMINAR ARTICULO", "6. GENERAR REPORTE", "7. GUARDAR Y SALIR"
    };
    for (int i = 0; i < 7; i++) {
        ALLEGRO_COLOR col = (seleccion == i+1) ? COL_ACENTO : COL_TEXTO;
        al_draw_text(font_normal, col, ANCHO_VENTANA/2 - 100, 150 + i*40, ALLEGRO_ALIGN_LEFT, opciones[i]);
    }
    al_flip_display();
}

int main(void) {
    if (!iniciarAllegro()) return -1;
    cargarArchivoBinario(&inventario, &totalArticulos);
    
    int opcion = 0;
    while (opcion != 7) {
        dibujarMenu(opcion);
        printf("\n Selecciona una opcion (1-7): ");
        if (scanf("%d", &opcion) != 1) { limpiarBuffer(); continue; }
        
        switch(opcion) {
            case 1: agregarArticulo(&inventario, &totalArticulos); break;
            case 2: buscarArticulo(inventario, totalArticulos); break;
            case 3: mostrarTodos(inventario, totalArticulos); break;
            case 4: modificarArticulo(inventario, totalArticulos); break;
            case 5: eliminarArticulo(&inventario, &totalArticulos); break;
            case 6: generarReporteTexto(inventario, totalArticulos); break;
            case 7: guardarArchivoBinario(inventario, totalArticulos); break;
        }
    }
    
    if (inventario) free(inventario);
    cerrarAllegro();
    return 0;
}