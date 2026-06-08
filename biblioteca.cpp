
/* ============================================================
 *  LIBRERIAS
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

/* ============================================================
 *  ENUMS
 * ============================================================ */

typedef enum {
    DISPONIBLE,
    PRESTADO
} EstadoObjeto;

//🐸 Pantallas / vistas de la aplicacion
typedef enum {
    PANTALLA_MENU,
    PANTALLA_AGREGAR,
    PANTALLA_MOSTRAR,
    PANTALLA_BUSCAR,
    PANTALLA_RESULTADO_BUSQUEDA,
    PANTALLA_MODIFICAR,
    PANTALLA_ELIMINAR,
    PANTALLA_REPORTE,
    PANTALLA_PROGRESO,
    PANTALLA_MENSAJE
} Pantalla;

/* ============================================================
 *  ESTRUCTURAS
 * ============================================================ */

typedef struct {
    int          id;
    char         titulo[100];
    char         autor[100];
    int          anio;
    char         genero[50];
    EstadoObjeto estado;
} Articulo;

//🐸 Campo de texto grafico
typedef struct {
    float x, y, w, h;
    char  buffer[200];
    int   longitud;
    int   activo;
    char  etiqueta[60];
} CampoTexto;

/* ============================================================
 *  CONSTANTES
 * ============================================================ */

#define ANCHO_VENTANA   1000
#define ALTO_VENTANA     650
#define ARCHIVO_BINARIO "biblioteca.dat"
#define ARCHIVO_REPORTE "reporte.txt"
#define MAX_CAMPOS        6
#define FILAS_POR_PAG    10

/* ============================================================
 *  VARIABLES GLOBALES
 * ============================================================ */

//🐸 Inventario dinamico
Articulo *inventario   = NULL;
int       totalArt     = 0;

//🐸 Allegro
ALLEGRO_DISPLAY     *display     = NULL;
ALLEGRO_EVENT_QUEUE *ev_queue    = NULL;
ALLEGRO_TIMER       *timer       = NULL;
ALLEGRO_FONT        *fnt_grande  = NULL;
ALLEGRO_FONT        *fnt_normal  = NULL;
ALLEGRO_FONT        *fnt_small   = NULL;

//🐸 Estado de la aplicacion
Pantalla   pantallaActual  = PANTALLA_MENU;
int        campoActivo     = 0;  //🐸 indice del campo enfocado
int        paginaTabla     = 0;  //🐸 pagina actual en MOSTRAR
int        idBusqueda      = -1;  //🐸 resultado de busqueda
int        idOperacion     = -1;  //🐸 id para modificar/eliminar
int        redibujar       = 1;

//🐸 Campos del formulario de agregar/modificar
CampoTexto campos[MAX_CAMPOS];
int        numCampos = 0;

//🐸 Mensaje temporal
char mensajeTitulo[120];
char mensajeLinea1[200];
char mensajeLinea2[200];
Pantalla mensajeRetorno = PANTALLA_MENU;

//🐸 Barra de progreso
float  progresoValor  = 0.0f;
char   progresoLabel[80];

//🐸 Resultado de busqueda
Articulo articuloEncontrado;
int      hayResultado = 0;

//🐸 Cursor visible (parpadeo)
int cursorVisible = 1;
int cursorTick    = 0;


#define C_FONDO      al_map_rgb(  8,  15,  35)  //🐸 azul muy oscuro, casi negro
#define C_PANEL      al_map_rgb( 18,  32,  75)  //🐸 azul oscuro logo
#define C_PANEL2     al_map_rgb( 26,  45,  98)  //🐸 azul oscuro un tono mas claro
#define C_ACENTO     al_map_rgb(245, 166,  35)  //🐸 dorado del logo
#define C_ACENTO2    al_map_rgb(255, 195,  70)  //🐸 dorado mas claro hover
#define C_TEXTO      al_map_rgb(225, 230, 248)  //🐸 blanco azulado legible
#define C_SUBTEXTO   al_map_rgb(123, 143, 199)  //🐸 azul grisaceo del logo
#define C_BORDE      al_map_rgb( 40,  65, 130)  //🐸 azul medio para bordes
#define C_BORDE2     al_map_rgb( 60,  90, 160)  //🐸 azul medio mas brillante
#define C_CAMPO      al_map_rgb( 12,  22,  55)  //🐸 fondo de campo de texto
#define C_CAMPO_ACT  al_map_rgb( 20,  38,  85)  //🐸 campo activo, azul oscuro
#define C_OK         al_map_rgb(245, 166,  35)  //🐸 dorado para confirmaciones
#define C_ERROR      al_map_rgb(232,   0, 106)  //🐸 magenta del logo para errores
#define C_HOVER      al_map_rgba(245,166,35, 35)  //🐸 dorado semitransparente
#define C_BARRA_BG   al_map_rgb( 20,  35,  80)  //🐸 fondo barra de progreso
#define C_BARRA_FG   al_map_rgb(245, 166,  35)  //🐸 relleno barra: dorado
#define C_PRESTADO   al_map_rgb(232,   0, 106)  //🐸 magenta = prestado
#define C_DISPONIBLE al_map_rgb(245, 166,  35)  //🐸 dorado = disponible
#define C_BOTON      al_map_rgb( 26,  58, 143)  //🐸 azul oscuro logo para botones
#define C_BOTON_H    al_map_rgb( 40,  80, 175)  //🐸 azul medio al hacer hover

/* ============================================================
 *  PROTOTIPOS
 * ============================================================ */

//🐸 Archivos
void cargarArchivoBinario(Articulo **inv, int *total);
void guardarArchivoBinario(Articulo *inv, int total);
void generarReporteTexto(Articulo *inv, int total);

//🐸 Inventario
void agregarArticulo(Articulo **inv, int *total);
int  buscarPorID(Articulo *inv, int total, int id);
int  buscarPorTitulo(Articulo *inv, int total, const char *txt);
void mostrarTodos(Articulo *inv, int total);
void modificarArticulo(Articulo *inv, int total, int id);
void eliminarArticulo(Articulo **inv, int *total, int id);
int  generarNuevoID(Articulo *inv, int total);

//🐸 Graficos - dibujo
void dibujarFondo(void);
void dibujarEncabezado(const char *titulo);
void dibujarPie(const char *ayuda);
void dibujarBoton(float x, float y, float w, float h,
                  const char *texto, int resaltado);
void dibujarCampo(CampoTexto *c, int enfocado);
void dibujarTodosLosCampos(void);
void dibujarMenu(void);
void dibujarFormAgregar(void);
void dibujarTabla(void);
void dibujarFormBuscar(void);
void dibujarResultado(void);
void dibujarFormModificar(void);
void dibujarFormEliminar(void);
void dibujarPantallaReporte(void);
void dibujarProgreso(void);
void dibujarMensaje(void);
void dibujar(void);

//🐸 Campos
void inicializarCampos(void);
void inicializarCamposModificar(int idx);
void limpiarCampos(void);
int  campoEnPosicion(float mx, float my);

//🐸 Logica de eventos
void manejarTecla(ALLEGRO_EVENT *ev);
void manejarClick(float mx, float my);
void manejarCaracter(ALLEGRO_EVENT *ev);

//🐸 Auxiliares
const char *estadoStr(EstadoObjeto e);
void        mostrarMensaje(const char *titulo, const char *l1,
                           const char *l2, Pantalla retorno);
void        mostrarProgreso(const char *label, float v);

/* ============================================================
 *  FUNCIONES DE ARCHIVOS
 * ============================================================ */

void cargarArchivoBinario(Articulo **inv, int *total) {
    int leidos = 0;
    mostrarProgreso("Cargando biblioteca...", 0.0f);

    FILE *fp = fopen(ARCHIVO_BINARIO, "rb");
    if (!fp) {
        mostrarProgreso("Cargando biblioteca...", 1.0f);
        al_rest(0.4);
        return;
    }
    fread(&leidos, sizeof(int), 1, fp);
    if (leidos > 0) {
        *inv = (Articulo *)malloc(leidos * sizeof(Articulo));
        if (!(*inv)) { fclose(fp); return; }
        int i;
        for (i = 0; i < leidos; i++) {
            fread(&((*inv)[i]), sizeof(Articulo), 1, fp);
            mostrarProgreso("Cargando biblioteca...", (float)(i+1)/leidos);
            al_rest(0.02);
        }
        *total = leidos;
    }
    fclose(fp);
    mostrarProgreso("Cargando biblioteca...", 1.0f);
    al_rest(0.4);
}

void guardarArchivoBinario(Articulo *inv, int total) {
    mostrarProgreso("Guardando biblioteca...", 0.0f);
    FILE *fp = fopen(ARCHIVO_BINARIO, "wb");
    if (!fp) return;
    fwrite(&total, sizeof(int), 1, fp);
    int i;
    for (i = 0; i < total; i++) {
        fwrite(&inv[i], sizeof(Articulo), 1, fp);
        mostrarProgreso("Guardando biblioteca...", (float)(i+1)/(total>0?total:1));
        al_rest(0.03);
    }
    fclose(fp);
    mostrarProgreso("Guardando biblioteca...", 1.0f);
    al_rest(0.5);
}

void generarReporteTexto(Articulo *inv, int total) {
    FILE *fp = fopen(ARCHIVO_REPORTE, "w");
    if (!fp) {
        mostrarMensaje("ERROR", "No se pudo crear reporte.txt", "", PANTALLA_MENU);
        return;
    }
    fprintf(fp, "============================================================\n");
    fprintf(fp, "      REPORTE - SISTEMA DE BIBLIOTECA PERSONAL\n");
    fprintf(fp, "============================================================\n\n");
    fprintf(fp, "Total de articulos: %d\n\n", total);
    int i;
    for (i = 0; i < total; i++) {
        fprintf(fp, "------------------------------------------------------------\n");
        fprintf(fp, "ID     : %d\n",  inv[i].id);
        fprintf(fp, "Titulo : %s\n",  inv[i].titulo);
        fprintf(fp, "Autor  : %s\n",  inv[i].autor);
        fprintf(fp, "Anio   : %d\n",  inv[i].anio);
        fprintf(fp, "Genero : %s\n",  inv[i].genero);
        fprintf(fp, "Estado : %s\n\n",estadoStr(inv[i].estado));
    }
    fprintf(fp, "============================================================\n");
    fclose(fp);
}

/* ============================================================
 *  FUNCIONES DE INVENTARIO
 * ============================================================ */

int generarNuevoID(Articulo *inv, int total) {
    int mx = 0, i;
    for (i = 0; i < total; i++)
        if (inv[i].id > mx) mx = inv[i].id;
    return mx + 1;
}

//🐸 Convierte campos del formulario en un Articulo y lo agrega
void agregarArticulo(Articulo **inv, int *total) {
    Articulo nuevo;
    nuevo.id = generarNuevoID(*inv, *total);
    strncpy(nuevo.titulo, campos[0].buffer, 99);
    strncpy(nuevo.autor,  campos[1].buffer, 99);
    nuevo.anio = atoi(campos[2].buffer);
    strncpy(nuevo.genero, campos[3].buffer, 49);
    //🐸 campo 4: estado — "0" o "1"
    nuevo.estado = (campos[4].buffer[0] == '1') ? PRESTADO : DISPONIBLE;

    Articulo *tmp = (Articulo *)realloc(*inv, (*total+1)*sizeof(Articulo));
    if (!tmp) return;
    *inv = tmp;
    (*inv)[*total] = nuevo;
    (*total)++;
}

int buscarPorID(Articulo *inv, int total, int id) {
    int i;
    for (i = 0; i < total; i++)
        if (inv[i].id == id) return i;
    return -1;
}

int buscarPorTitulo(Articulo *inv, int total, const char *txt) {
    int i;
    for (i = 0; i < total; i++)
        if (strstr(inv[i].titulo, txt)) return i;
    return -1;
}

void modificarArticulo(Articulo *inv, int total, int id) {
    int idx = buscarPorID(inv, total, id);
    if (idx < 0) return;
    if (strlen(campos[0].buffer) > 0)
        strncpy(inv[idx].titulo, campos[0].buffer, 99);
    if (strlen(campos[1].buffer) > 0)
        strncpy(inv[idx].autor,  campos[1].buffer, 99);
    if (strlen(campos[2].buffer) > 0)
        inv[idx].anio = atoi(campos[2].buffer);
    if (strlen(campos[3].buffer) > 0)
        strncpy(inv[idx].genero, campos[3].buffer, 49);
    if (strlen(campos[4].buffer) > 0)
        inv[idx].estado = (campos[4].buffer[0]=='1') ? PRESTADO : DISPONIBLE;
}

void eliminarArticulo(Articulo **inv, int *total, int id) {
    int pos = buscarPorID(*inv, *total, id);
    if (pos < 0) return;
    int i;
    for (i = pos; i < *total-1; i++)
        (*inv)[i] = (*inv)[i+1];
    (*total)--;
    if (*total > 0) {
        Articulo *tmp = (Articulo *)realloc(*inv, *total*sizeof(Articulo));
        if (tmp) *inv = tmp;
    } else {
        free(*inv);
        *inv = NULL;
    }
}

/* ============================================================
 *  AUXILIARES GRAFICOS
 * ============================================================ */

const char *estadoStr(EstadoObjeto e) {
    return (e == DISPONIBLE) ? "DISPONIBLE" : "PRESTADO";
}

void mostrarMensaje(const char *titulo, const char *l1,
                    const char *l2, Pantalla retorno) {
    strncpy(mensajeTitulo, titulo, 119);
    strncpy(mensajeLinea1, l1,    199);
    strncpy(mensajeLinea2, l2,    199);
    mensajeRetorno  = retorno;
    pantallaActual  = PANTALLA_MENSAJE;
    redibujar = 1;
}

void mostrarProgreso(const char *label, float v) {
    strncpy(progresoLabel, label, 79);
    progresoValor  = v;
    pantallaActual = PANTALLA_PROGRESO;
    dibujar();  //🐸 forzar redibujado inmediato
}

/* ============================================================
 *  CAMPOS DE TEXTO GRAFICOS
 * ============================================================ */

void limpiarCampos(void) {
    int i;
    for (i = 0; i < MAX_CAMPOS; i++) {
        campos[i].buffer[0] = '\0';
        campos[i].longitud  = 0;
        campos[i].activo    = 0;
    }
    campoActivo = 0;
}

//🐸 Configura campos para el formulario de AGREGAR
void inicializarCampos(void) {
    limpiarCampos();
    numCampos = 5;
    float cx = 300, cy = 170, cw = 380, ch = 38, sep = 70;

    strncpy(campos[0].etiqueta, "Titulo",          59);
    strncpy(campos[1].etiqueta, "Autor",           59);
    strncpy(campos[2].etiqueta, "Anio",            59);
    strncpy(campos[3].etiqueta, "Genero",          59);
    strncpy(campos[4].etiqueta, "Estado (0=Disp, 1=Prest)", 59);

    int i;
    for (i = 0; i < numCampos; i++) {
        campos[i].x = cx;
        campos[i].y = cy + i * sep;
        campos[i].w = cw;
        campos[i].h = ch;
    }
    campos[0].activo = 1;
}

//🐸 Configura campos para MODIFICAR precargando valores actuales
void inicializarCamposModificar(int idx) {
    limpiarCampos();
    numCampos = 5;
    float cx = 300, cy = 170, cw = 380, ch = 38, sep = 70;

    strncpy(campos[0].etiqueta, "Titulo (vacio=no cambiar)",   59);
    strncpy(campos[1].etiqueta, "Autor  (vacio=no cambiar)",   59);
    strncpy(campos[2].etiqueta, "Anio   (0=no cambiar)",       59);
    strncpy(campos[3].etiqueta, "Genero (vacio=no cambiar)",   59);
    strncpy(campos[4].etiqueta, "Estado (0=Disp,1=Prest,-=nc)",59);

    int i;
    for (i = 0; i < numCampos; i++) {
        campos[i].x = cx;
        campos[i].y = cy + i * sep;
        campos[i].w = cw;
        campos[i].h = ch;
    }

    //🐸 Precargar con valores actuales
    strncpy(campos[0].buffer, inventario[idx].titulo, 199);
    campos[0].longitud = (int)strlen(campos[0].buffer);
    strncpy(campos[1].buffer, inventario[idx].autor,  199);
    campos[1].longitud = (int)strlen(campos[1].buffer);
    snprintf(campos[2].buffer, 199, "%d", inventario[idx].anio);
    campos[2].longitud = (int)strlen(campos[2].buffer);
    strncpy(campos[3].buffer, inventario[idx].genero, 199);
    campos[3].longitud = (int)strlen(campos[3].buffer);
    campos[4].buffer[0] = (inventario[idx].estado == DISPONIBLE) ? '0' : '1';
    campos[4].buffer[1] = '\0';
    campos[4].longitud  = 1;

    campos[0].activo = 1;
}

int campoEnPosicion(float mx, float my) {
    int i;
    for (i = 0; i < numCampos; i++) {
        if (mx >= campos[i].x && mx <= campos[i].x + campos[i].w &&
            my >= campos[i].y && my <= campos[i].y + campos[i].h)
            return i;
    }
    return -1;
}

/* ============================================================
 *  FUNCIONES DE DIBUJO
 * ============================================================ */

void dibujarFondo(void) {
    al_clear_to_color(C_FONDO);
    //🐸 Lineas decorativas sutiles
    int i;
    for (i = 0; i < ALTO_VENTANA; i += 40)
        al_draw_line(0, (float)i, (float)ANCHO_VENTANA, (float)i,
                     al_map_rgba(255,255,255,4), 1.0f);
}

void dibujarEncabezado(const char *titulo) {
    al_draw_filled_rectangle(0, 0, (float)ANCHO_VENTANA, 75, C_PANEL);
    al_draw_line(0, 75, (float)ANCHO_VENTANA, 75, C_ACENTO, 2.0f);
    //🐸 Bloque de color lateral
    al_draw_filled_rectangle(0, 0, 6, 75, C_ACENTO);
    al_draw_text(fnt_grande, C_ACENTO,
                 ANCHO_VENTANA/2, 18,
                 ALLEGRO_ALIGN_CENTRE, "BIBLIOTECA PERSONAL");
    al_draw_text(fnt_small, C_SUBTEXTO,
                 ANCHO_VENTANA/2, 50,
                 ALLEGRO_ALIGN_CENTRE, titulo);
}

void dibujarPie(const char *ayuda) {
    al_draw_line(0, (float)(ALTO_VENTANA-45),
                 (float)ANCHO_VENTANA, (float)(ALTO_VENTANA-45),
                 C_BORDE, 1.0f);
    al_draw_filled_rectangle(0, (float)(ALTO_VENTANA-45),
                             (float)ANCHO_VENTANA, (float)ALTO_VENTANA,
                             C_PANEL);
    al_draw_text(fnt_small, C_SUBTEXTO,
                 ANCHO_VENTANA/2, (float)(ALTO_VENTANA-28),
                 ALLEGRO_ALIGN_CENTRE, ayuda);
}

void dibujarBoton(float x, float y, float w, float h,
                  const char *texto, int resaltado) {
    ALLEGRO_COLOR bg = resaltado ? C_ACENTO   : C_BOTON;
    ALLEGRO_COLOR fg = resaltado ? C_FONDO    : C_TEXTO;
    ALLEGRO_COLOR br = resaltado ? C_ACENTO2  : C_BORDE2;
    al_draw_filled_rounded_rectangle(x, y, x+w, y+h, 6, 6, bg);
    al_draw_rounded_rectangle(x, y, x+w, y+h, 6, 6, br, 1.5f);
    al_draw_text(fnt_normal, fg, x+w/2, y+h/2-8,
                 ALLEGRO_ALIGN_CENTRE, texto);
}

void dibujarCampo(CampoTexto *c, int enfocado) {
    //🐸 Etiqueta
    al_draw_text(fnt_small, C_SUBTEXTO,
                 c->x, c->y - 20, 0, c->etiqueta);
    //🐸 Caja
    ALLEGRO_COLOR bg = enfocado ? C_CAMPO_ACT : C_CAMPO;
    ALLEGRO_COLOR br = enfocado ? C_ACENTO    : C_BORDE;
    al_draw_filled_rounded_rectangle(c->x, c->y,
                                     c->x+c->w, c->y+c->h,
                                     5, 5, bg);
    al_draw_rounded_rectangle(c->x, c->y,
                              c->x+c->w, c->y+c->h,
                              5, 5, br, enfocado ? 2.0f : 1.0f);
    //🐸 Texto ingresado
    //🐸 Mostrar ultimos caracteres que quepan
    char visible[200];
    strncpy(visible, c->buffer, 199);
    al_draw_text(fnt_normal, C_TEXTO,
                 c->x+10, c->y+8, 0, visible);
    //🐸 Cursor parpadeante
    if (enfocado && cursorVisible) {
        float tw = al_get_text_width(fnt_normal, visible);
        float cx2 = c->x + 10 + tw;
        al_draw_line(cx2+2, c->y+6, cx2+2, c->y+c->h-6,
                     C_ACENTO, 2.0f);
    }
}

void dibujarTodosLosCampos(void) {
    int i;
    for (i = 0; i < numCampos; i++)
        dibujarCampo(&campos[i], i == campoActivo);
}

/* Boton fijo "<  Volver al Menu" en esquina superior derecha.
   Se llama al final de cada pantalla secundaria. */
void dibujarBotonMenu(void) {
    float bx = (float)(ANCHO_VENTANA - 162);
    float by = 16.0f;
    float bw = 148.0f;
    float bh = 34.0f;
    al_draw_filled_rounded_rectangle(bx, by, bx+bw, by+bh,
                                     6, 6, al_map_rgb(50,62,80));
    al_draw_rounded_rectangle(bx, by, bx+bw, by+bh,
                              6, 6, C_ACENTO, 1.5f);
    al_draw_text(fnt_small, C_ACENTO,
                 bx + bw/2, by + 9,
                 ALLEGRO_ALIGN_CENTRE, "<  Volver al Menu");
}

//🐸 ---- Pantalla: MENU PRINCIPAL ----
void dibujarMenu(void) {
    dibujarFondo();
    dibujarEncabezado("Sistema de Gestion de Coleccion Personal");

    //🐸 Panel central
    float px = ANCHO_VENTANA/2 - 220;
    float py = 100;
    float pw = 440;
    float ph = 430;
    al_draw_filled_rounded_rectangle(px, py, px+pw, py+ph, 10, 10, C_PANEL);
    al_draw_rounded_rectangle(px, py, px+pw, py+ph, 10, 10, C_BORDE, 1.5f);
    al_draw_filled_rectangle(px, py, px+pw, py+36, C_PANEL2);
    al_draw_text(fnt_normal, C_ACENTO,
                 px+pw/2, py+9, ALLEGRO_ALIGN_CENTRE, "MENU PRINCIPAL");
    al_draw_line(px, py+36, px+pw, py+36, C_BORDE, 1.0f);

    const char *opciones[] = {
        "1   Agregar articulo",
        "2   Buscar articulo",
        "3   Mostrar todos",
        "4   Modificar articulo",
        "5   Eliminar articulo",
        "6   Generar reporte",
        "7   Guardar y salir"
    };
    int n = 7;
    int i;
    for (i = 0; i < n; i++) {
        float oy = py + 50 + i * 52;
        dibujarBoton(px+30, oy, pw-60, 38, opciones[i], 0);
    }

    //🐸 Total articulos
    char info[60];
    snprintf(info, 59, "Articulos en biblioteca: %d", totalArt);
    al_draw_text(fnt_small, C_SUBTEXTO,
                 ANCHO_VENTANA/2, py+ph+10,
                 ALLEGRO_ALIGN_CENTRE, info);

    dibujarPie("Haz clic en una opcion o presiona la tecla numerica correspondiente  |  ESC = salir");
}

//🐸 ---- Pantalla: AGREGAR / MODIFICAR ----
void dibujarFormulario(const char *titulo, const char *botonTexto) {
    dibujarFondo();
    dibujarEncabezado(titulo);

    //🐸 Panel
    float px = 220, py = 95, pw = 560, ph = 460;
    al_draw_filled_rounded_rectangle(px, py, px+pw, py+ph, 10, 10, C_PANEL);
    al_draw_rounded_rectangle(px, py, px+pw, py+ph, 10, 10, C_BORDE, 1.5f);

    //🐸 Nota de ID solo en agregar
    if (pantallaActual == PANTALLA_AGREGAR) {
        char idStr[60];
        snprintf(idStr, 59, "ID se asignara automaticamente: %d",
                 generarNuevoID(inventario, totalArt));
        al_draw_text(fnt_small, C_OK, px+pw/2, py+12,
                     ALLEGRO_ALIGN_CENTRE, idStr);
    }

    dibujarTodosLosCampos();

    //🐸 Botones
    dibujarBoton(px+30,      py+ph-56, 180, 38, botonTexto,  1);
    dibujarBoton(px+pw-210,  py+ph-56, 180, 38, "Cancelar",  0);

    dibujarBotonMenu();
    dibujarPie("TAB = siguiente campo  |  Clic en el campo para seleccionarlo  |  ESC = cancelar");
}

void dibujarFormAgregar(void) {
    dibujarFormulario("AGREGAR NUEVO ARTICULO", "Guardar");
}

void dibujarFormModificar(void) {
    char tit[80];
    snprintf(tit, 79, "MODIFICAR ARTICULO  (ID: %d)", idOperacion);
    dibujarFormulario(tit, "Actualizar");
}

//🐸 ---- Pantalla: MOSTRAR TODOS ----
void dibujarTabla(void) {
    dibujarFondo();
    dibujarEncabezado("INVENTARIO COMPLETO");

    if (totalArt == 0) {
        al_draw_text(fnt_normal, C_SUBTEXTO,
                     ANCHO_VENTANA/2, ALTO_VENTANA/2,
                     ALLEGRO_ALIGN_CENTRE,
                     "No hay articulos en la biblioteca.");
        dibujarBotonMenu();
        dibujarPie("ESC = volver al menu");
        return;
    }

    //🐸 Cabecera de columnas
    int cx[] = { 30,  90, 330, 540, 650, 790 };
    const char *ch[] = { "ID","TITULO","AUTOR","ANIO","GENERO","ESTADO" };
    int ncols = 6;
    int hy = 85;

    al_draw_filled_rectangle(0, (float)hy,
                             (float)ANCHO_VENTANA, (float)(hy+30),
                             C_PANEL2);
    int i;
    for (i = 0; i < ncols; i++) {
        al_draw_text(fnt_small, C_ACENTO, (float)cx[i], (float)(hy+7), 0, ch[i]);
        if (i > 0)
            al_draw_line((float)cx[i]-5, (float)hy,
                         (float)cx[i]-5, (float)(ALTO_VENTANA-50),
                         C_BORDE, 1.0f);
    }
    al_draw_line(0, (float)(hy+30),
                 (float)ANCHO_VENTANA, (float)(hy+30),
                 C_ACENTO, 1.5f);

    int inicio = paginaTabla * FILAS_POR_PAG;
    int fin    = inicio + FILAS_POR_PAG;
    if (fin > totalArt) fin = totalArt;

    char buf[120];
    for (i = inicio; i < fin; i++) {
        float fy = (float)(hy + 32 + (i-inicio)*46);
        ALLEGRO_COLOR rowBg = ((i-inicio)%2==0) ? C_PANEL : al_map_rgb(28,35,46);
        al_draw_filled_rectangle(0, fy, (float)ANCHO_VENTANA, fy+44, rowBg);

        snprintf(buf,sizeof(buf),"%d", inventario[i].id);
        al_draw_text(fnt_small, C_TEXTO, (float)cx[0], fy+13, 0, buf);

        snprintf(buf,sizeof(buf),"%.26s", inventario[i].titulo);
        al_draw_text(fnt_small, C_TEXTO, (float)cx[1], fy+13, 0, buf);

        snprintf(buf,sizeof(buf),"%.22s", inventario[i].autor);
        al_draw_text(fnt_small, C_TEXTO, (float)cx[2], fy+13, 0, buf);

        snprintf(buf,sizeof(buf),"%d", inventario[i].anio);
        al_draw_text(fnt_small, C_TEXTO, (float)cx[3], fy+13, 0, buf);

        snprintf(buf,sizeof(buf),"%.14s", inventario[i].genero);
        al_draw_text(fnt_small, C_TEXTO, (float)cx[4], fy+13, 0, buf);

        ALLEGRO_COLOR ce = (inventario[i].estado==DISPONIBLE)?C_DISPONIBLE:C_PRESTADO;
        al_draw_text(fnt_small, ce, (float)cx[5], fy+13, 0,
                     estadoStr(inventario[i].estado));

        al_draw_line(0, fy+44, (float)ANCHO_VENTANA, fy+44, C_BORDE, 0.5f);
    }

    //🐸 Paginacion
    int totalPag = (totalArt + FILAS_POR_PAG - 1) / FILAS_POR_PAG;
    char pagBuf[80];
    snprintf(pagBuf, 79, "Pagina %d / %d", paginaTabla+1, totalPag);

    dibujarBoton(30,          (float)(ALTO_VENTANA-42), 120, 32, "<< Anterior", 0);
    al_draw_text(fnt_small, C_SUBTEXTO,
                 (float)(ANCHO_VENTANA/2), (float)(ALTO_VENTANA-30),
                 ALLEGRO_ALIGN_CENTRE, pagBuf);
    dibujarBoton((float)(ANCHO_VENTANA-155), (float)(ALTO_VENTANA-42),
                 120, 32, "Siguiente >>", 0);

    dibujarBotonMenu();
    al_draw_line(0, (float)(ALTO_VENTANA-45),
                 (float)ANCHO_VENTANA, (float)(ALTO_VENTANA-45),
                 C_BORDE, 1.0f);
}

//🐸 ---- Pantalla: BUSCAR ----
void dibujarFormBuscar(void) {
    dibujarFondo();
    dibujarEncabezado("BUSCAR ARTICULO");

    float px = 280, py = 110, pw = 440, ph = 300;
    al_draw_filled_rounded_rectangle(px, py, px+pw, py+ph, 10, 10, C_PANEL);
    al_draw_rounded_rectangle(px, py, px+pw, py+ph, 10, 10, C_BORDE, 1.5f);

    al_draw_text(fnt_normal, C_TEXTO, px+pw/2, py+14,
                 ALLEGRO_ALIGN_CENTRE, "Ingresa el termino de busqueda:");

    dibujarCampo(&campos[0], 1);  //🐸 campo 0 = campo de busqueda
    dibujarCampo(&campos[1], 0);  //🐸 campo 1 = "por titulo o ID" selector

    //🐸 Opciones de tipo de busqueda
    al_draw_text(fnt_small, C_SUBTEXTO, px+30, py+130, 0,
                 "Tipo:  [0] Por ID     [1] Por Titulo");
    dibujarCampo(&campos[1], campoActivo==1);

    dibujarBoton(px+30,     py+ph-56, 180, 38, "Buscar",   1);
    dibujarBoton(px+pw-210, py+ph-56, 180, 38, "Cancelar", 0);

    dibujarBotonMenu();
    dibujarPie("Escribe el valor y haz clic en Buscar  |  ESC = cancelar");
}

//🐸 ---- Pantalla: RESULTADO BUSQUEDA ----
void dibujarResultado(void) {
    dibujarFondo();
    dibujarEncabezado("RESULTADO DE BUSQUEDA");

    float px = 250, py = 100, pw = 500, ph = 360;
    al_draw_filled_rounded_rectangle(px, py, px+pw, py+ph, 10, 10, C_PANEL);
    al_draw_rounded_rectangle(px, py, px+pw, py+ph, 10, 10,
                              hayResultado ? C_OK : C_ERROR, 2.0f);

    if (!hayResultado) {
        al_draw_text(fnt_grande, C_ERROR,
                     px+pw/2, py+ph/2-15,
                     ALLEGRO_ALIGN_CENTRE, "No encontrado");
    } else {
        Articulo *a = &articuloEncontrado;
        char buf[160];
        float oy = py + 30;
        float paso = 44;

        al_draw_text(fnt_normal, C_ACENTO, px+pw/2, oy,
                     ALLEGRO_ALIGN_CENTRE, "ARTICULO ENCONTRADO");
        al_draw_line(px, oy+28, px+pw, oy+28, C_BORDE, 1.0f);
        oy += 42;

        const char *labs[] = {"ID","Titulo","Autor","Anio","Genero","Estado"};
        snprintf(buf,159,"%d", a->id); int fi=0;
        al_draw_text(fnt_small, C_SUBTEXTO, px+20,  oy+fi*paso, 0, labs[0]);
        al_draw_text(fnt_normal, C_TEXTO,   px+130, oy+fi*paso, 0, buf);       fi++;
        al_draw_text(fnt_small, C_SUBTEXTO, px+20,  oy+fi*paso, 0, labs[1]);
        al_draw_text(fnt_normal, C_TEXTO,   px+130, oy+fi*paso, 0, a->titulo); fi++;
        al_draw_text(fnt_small, C_SUBTEXTO, px+20,  oy+fi*paso, 0, labs[2]);
        al_draw_text(fnt_normal, C_TEXTO,   px+130, oy+fi*paso, 0, a->autor);  fi++;
        snprintf(buf,159,"%d", a->anio);
        al_draw_text(fnt_small, C_SUBTEXTO, px+20,  oy+fi*paso, 0, labs[3]);
        al_draw_text(fnt_normal, C_TEXTO,   px+130, oy+fi*paso, 0, buf);       fi++;
        al_draw_text(fnt_small, C_SUBTEXTO, px+20,  oy+fi*paso, 0, labs[4]);
        al_draw_text(fnt_normal, C_TEXTO,   px+130, oy+fi*paso, 0, a->genero); fi++;
        al_draw_text(fnt_small, C_SUBTEXTO, px+20,  oy+fi*paso, 0, labs[5]);
        ALLEGRO_COLOR ce=(a->estado==DISPONIBLE)?C_DISPONIBLE:C_PRESTADO;
        al_draw_text(fnt_normal, ce, px+130, oy+fi*paso, 0, estadoStr(a->estado));
    }

    dibujarBotonMenu();
    dibujarBoton(px+pw/2-70, py+ph-52, 140, 38, "Volver", 1);
    dibujarPie("Haz clic en Volver o presiona ESC");
}

//🐸 ---- Pantalla: ELIMINAR ----
void dibujarFormEliminar(void) {
    dibujarFondo();
    dibujarEncabezado("ELIMINAR ARTICULO");

    float px = 300, py = 130, pw = 400, ph = 250;
    al_draw_filled_rounded_rectangle(px, py, px+pw, py+ph, 10, 10, C_PANEL);
    al_draw_rounded_rectangle(px, py, px+pw, py+ph, 10, 10, C_ERROR, 2.0f);

    al_draw_text(fnt_normal, C_TEXTO, px+pw/2, py+16,
                 ALLEGRO_ALIGN_CENTRE, "Ingresa el ID a eliminar:");

    dibujarCampo(&campos[0], 1);

    //🐸 Preview si ya hay un ID valido
    int id = atoi(campos[0].buffer);
    if (id > 0) {
        int idx = buscarPorID(inventario, totalArt, id);
        if (idx >= 0) {
            char prev[120];
            snprintf(prev,119,"Se eliminara: \"%s\"", inventario[idx].titulo);
            al_draw_text(fnt_small, C_ERROR, px+pw/2, py+180,
                         ALLEGRO_ALIGN_CENTRE, prev);
        } else {
            al_draw_text(fnt_small, C_SUBTEXTO, px+pw/2, py+180,
                         ALLEGRO_ALIGN_CENTRE, "ID no encontrado");
        }
    }

    dibujarBoton(px+20,      py+ph-52, 160, 38, "Eliminar", 1);
    dibujarBoton(px+pw-180,  py+ph-52, 160, 38, "Cancelar", 0);

    dibujarBotonMenu();
    dibujarPie("Escribe el ID y haz clic en Eliminar  |  ESC = cancelar");
}

//🐸 ---- Pantalla: REPORTE ----
void dibujarPantallaReporte(void) {
    dibujarFondo();
    dibujarEncabezado("REPORTE GENERADO");

    float px = 220, py = 100, pw = 560, ph = 400;
    al_draw_filled_rounded_rectangle(px, py, px+pw, py+ph, 10, 10, C_PANEL);
    al_draw_rounded_rectangle(px, py, px+pw, py+ph, 10, 10, C_OK, 2.0f);

    al_draw_text(fnt_grande, C_OK,
                 px+pw/2, py+20, ALLEGRO_ALIGN_CENTRE,
                 "Reporte generado exitosamente");
    al_draw_line(px, py+60, px+pw, py+60, C_BORDE, 1.0f);

    al_draw_text(fnt_normal, C_TEXTO,
                 px+pw/2, py+80, ALLEGRO_ALIGN_CENTRE,
                 "Archivo: reporte.txt");

    char info[80];
    snprintf(info, 79, "Total de articulos: %d", totalArt);
    al_draw_text(fnt_normal, C_SUBTEXTO,
                 px+pw/2, py+120, ALLEGRO_ALIGN_CENTRE, info);

    //🐸 Preview de los primeros articulos en el reporte
    al_draw_line(px, py+155, px+pw, py+155, C_BORDE, 1.0f);
    al_draw_text(fnt_small, C_ACENTO, px+20, py+165, 0, "Vista previa:");

    int i, maxPrev = (totalArt < 6) ? totalArt : 6;
    for (i = 0; i < maxPrev; i++) {
        char linea[120];
        snprintf(linea,119, "[%d]  %s  —  %s  (%d)  %s",
                 inventario[i].id,
                 inventario[i].titulo,
                 inventario[i].autor,
                 inventario[i].anio,
                 estadoStr(inventario[i].estado));
        al_draw_text(fnt_small, C_TEXTO, px+20, (float)(py+185+i*28), 0, linea);
    }
    if (totalArt > 6) {
        char mas[40];
        snprintf(mas, 39, "... y %d mas en el archivo.", totalArt-6);
        al_draw_text(fnt_small, C_SUBTEXTO, px+20,
                     (float)(py+185+6*28), 0, mas);
    }

    dibujarBotonMenu();
    dibujarBoton(px+pw/2-80, py+ph-52, 160, 38, "Volver al menu", 1);
    dibujarPie("Haz clic en Volver o presiona ESC");
}

//🐸 ---- Pantalla: PROGRESO ----
void dibujarProgreso(void) {
    al_clear_to_color(C_FONDO);

    al_draw_text(fnt_grande, C_ACENTO,
                 ANCHO_VENTANA/2, ALTO_VENTANA/2 - 70,
                 ALLEGRO_ALIGN_CENTRE, "BIBLIOTECA PERSONAL");
    al_draw_text(fnt_normal, C_TEXTO,
                 ANCHO_VENTANA/2, ALTO_VENTANA/2 - 24,
                 ALLEGRO_ALIGN_CENTRE, progresoLabel);

    float bx = ANCHO_VENTANA/2 - 220;
    float by = ALTO_VENTANA/2 + 10;
    float bw = 440, bh = 30;

    al_draw_filled_rounded_rectangle(bx, by, bx+bw, by+bh, 8, 8, C_BARRA_BG);
    float fill = bw * progresoValor;
    if (fill > 1)
        al_draw_filled_rounded_rectangle(bx, by, bx+fill, by+bh,
                                         8, 8, C_BARRA_FG);
    al_draw_rounded_rectangle(bx, by, bx+bw, by+bh, 8, 8, C_BORDE2, 1.5f);

    char pct[12];
    snprintf(pct, 11, "%d%%", (int)(progresoValor*100));
    al_draw_text(fnt_small, C_FONDO, bx+bw/2, by+6,
                 ALLEGRO_ALIGN_CENTRE, pct);

    al_flip_display();
}

//🐸 ---- Pantalla: MENSAJE ----
void dibujarMensaje(void) {
    dibujarFondo();

    float px = ANCHO_VENTANA/2-250, py = ALTO_VENTANA/2-130;
    float pw = 500, ph = 260;
    al_draw_filled_rounded_rectangle(px, py, px+pw, py+ph, 12, 12, C_PANEL);
    al_draw_rounded_rectangle(px, py, px+pw, py+ph, 12, 12, C_ACENTO, 2.0f);

    al_draw_text(fnt_grande, C_ACENTO,
                 px+pw/2, py+18, ALLEGRO_ALIGN_CENTRE, mensajeTitulo);
    al_draw_line(px, py+58, px+pw, py+58, C_BORDE, 1.0f);
    al_draw_text(fnt_normal, C_TEXTO,
                 px+pw/2, py+72, ALLEGRO_ALIGN_CENTRE, mensajeLinea1);
    if (strlen(mensajeLinea2) > 0)
        al_draw_text(fnt_small, C_SUBTEXTO,
                     px+pw/2, py+108, ALLEGRO_ALIGN_CENTRE, mensajeLinea2);

    dibujarBoton(px+pw/2-70, py+ph-56, 140, 38, "Aceptar", 1);
    dibujarPie("Haz clic en Aceptar o presiona ENTER / ESC");
}

//🐸 ---- Despachador principal ----
void dibujar(void) {
    switch (pantallaActual) {
        case PANTALLA_MENU:               dibujarMenu();            break;
        case PANTALLA_AGREGAR:            dibujarFormAgregar();     break;
        case PANTALLA_MOSTRAR:            dibujarTabla();           break;
        case PANTALLA_BUSCAR:             dibujarFormBuscar();      break;
        case PANTALLA_RESULTADO_BUSQUEDA: dibujarResultado();       break;
        case PANTALLA_MODIFICAR:          dibujarFormModificar();   break;
        case PANTALLA_ELIMINAR:           dibujarFormEliminar();    break;
        case PANTALLA_REPORTE:            dibujarPantallaReporte(); break;
        case PANTALLA_PROGRESO:           dibujarProgreso();        break;
        case PANTALLA_MENSAJE:            dibujarMensaje();         break;
    }
    if (pantallaActual != PANTALLA_PROGRESO)
        al_flip_display();
}

/* ============================================================
 *  MANEJO DE EVENTOS
 * ============================================================ */

//🐸 Escritura en el campo activo
void manejarCaracter(ALLEGRO_EVENT *ev) {
    if (pantallaActual != PANTALLA_AGREGAR &&
        pantallaActual != PANTALLA_MODIFICAR &&
        pantallaActual != PANTALLA_BUSCAR &&
        pantallaActual != PANTALLA_ELIMINAR) return;

    if (campoActivo < 0 || campoActivo >= numCampos) return;
    CampoTexto *c = &campos[campoActivo];
    int uni = ev->keyboard.unichar;

    if (uni >= 32 && uni < 127 && c->longitud < 198) {
        c->buffer[c->longitud++] = (char)uni;
        c->buffer[c->longitud]   = '\0';
        redibujar = 1;
    }
}

void manejarTecla(ALLEGRO_EVENT *ev) {
    int kc = ev->keyboard.keycode;

    //🐸 ENTER / ESC globales
    if (kc == ALLEGRO_KEY_ESCAPE) {
        switch (pantallaActual) {
            case PANTALLA_MENU:
                guardarArchivoBinario(inventario, totalArt);
                al_destroy_display(display);
                exit(0);
                break;
            default:
                pantallaActual = PANTALLA_MENU;
                limpiarCampos();
                redibujar = 1;
                break;
        }
        return;
    }

    //🐸 Teclas de menu principal
    if (pantallaActual == PANTALLA_MENU) {
        switch (kc) {
            case ALLEGRO_KEY_1:
                inicializarCampos();
                pantallaActual = PANTALLA_AGREGAR;
                redibujar = 1;
                break;
            case ALLEGRO_KEY_2:
                limpiarCampos();
                numCampos = 2;
                campos[0].x=300; campos[0].y=260; campos[0].w=380; campos[0].h=38;
                strncpy(campos[0].etiqueta,"Valor a buscar",59);
                campos[1].x=300; campos[1].y=330; campos[1].w=380; campos[1].h=38;
                strncpy(campos[1].etiqueta,"Tipo: 0=Por ID   1=Por Titulo",59);
                campos[1].buffer[0]='0'; campos[1].buffer[1]='\0'; campos[1].longitud=1;
                campoActivo = 0;
                pantallaActual = PANTALLA_BUSCAR;
                redibujar = 1;
                break;
            case ALLEGRO_KEY_3:
                paginaTabla = 0;
                pantallaActual = PANTALLA_MOSTRAR;
                redibujar = 1;
                break;
            case ALLEGRO_KEY_4:
                limpiarCampos();
                numCampos = 1;
                campos[0].x=300; campos[0].y=260; campos[0].w=380; campos[0].h=38;
                strncpy(campos[0].etiqueta,"ID del articulo a modificar",59);
                campoActivo = 0;
                /* Se usa pantalla eliminar temporalmente para pedir ID,
                   luego se cambia a modificar al confirmar */
                idOperacion = -1;
                pantallaActual = PANTALLA_ELIMINAR;  //🐸 reutilizamos logica de pedir ID
                //🐸 pero en realidad haremos la pantalla de eliminar que pide ID
                //🐸 Para modificar abrimos directamente la pantalla correcta:
                pantallaActual = PANTALLA_MODIFICAR;
                //🐸 inicializamos campos de modificar vacios; el ID se pedira en campo[0]
                limpiarCampos();
                numCampos = 1;
                campos[0].x=350; campos[0].y=280; campos[0].w=300; campos[0].h=38;
                strncpy(campos[0].etiqueta,"ID del articulo a modificar",59);
                campoActivo = 0;
                idOperacion = -2;  //🐸 -2 = esperando que se ingrese el ID
                redibujar = 1;
                break;
            case ALLEGRO_KEY_5:
                limpiarCampos();
                numCampos = 1;
                campos[0].x=300; campos[0].y=260; campos[0].w=380; campos[0].h=38;
                strncpy(campos[0].etiqueta,"ID del articulo a eliminar",59);
                campoActivo = 0;
                pantallaActual = PANTALLA_ELIMINAR;
                redibujar = 1;
                break;
            case ALLEGRO_KEY_6:
                generarReporteTexto(inventario, totalArt);
                pantallaActual = PANTALLA_REPORTE;
                redibujar = 1;
                break;
            case ALLEGRO_KEY_7:
                guardarArchivoBinario(inventario, totalArt);
                mostrarMensaje("GUARDADO", "Datos guardados correctamente.",
                               "La aplicacion se cerrara en un momento.", PANTALLA_MENU);
                al_rest(2.0);
                al_destroy_display(display);
                exit(0);
                break;
        }
        return;
    }

    //🐸 Navegacion en tabla
    if (pantallaActual == PANTALLA_MOSTRAR) {
        int totalPag = (totalArt + FILAS_POR_PAG - 1) / FILAS_POR_PAG;
        if (kc == ALLEGRO_KEY_RIGHT && paginaTabla < totalPag-1) {
            paginaTabla++; redibujar = 1;
        }
        if (kc == ALLEGRO_KEY_LEFT && paginaTabla > 0) {
            paginaTabla--; redibujar = 1;
        }
        return;
    }

    //🐸 TAB: avanzar campo
    if (kc == ALLEGRO_KEY_TAB) {
        if (numCampos > 0) {
            campoActivo = (campoActivo + 1) % numCampos;
            redibujar = 1;
        }
        return;
    }

    //🐸 BACKSPACE: borrar caracter
    if (kc == ALLEGRO_KEY_BACKSPACE) {
        if (campoActivo >= 0 && campoActivo < numCampos) {
            CampoTexto *c = &campos[campoActivo];
            if (c->longitud > 0) {
                c->buffer[--c->longitud] = '\0';
                redibujar = 1;
            }
        }
        return;
    }

    //🐸 ENTER: confirmar accion
    if (kc == ALLEGRO_KEY_ENTER) {
        //🐸 --- Mensaje: cerrar ---
        if (pantallaActual == PANTALLA_MENSAJE) {
            pantallaActual = mensajeRetorno;
            redibujar = 1;
            return;
        }
        //🐸 --- Resultado busqueda: volver ---
        if (pantallaActual == PANTALLA_RESULTADO_BUSQUEDA ||
            pantallaActual == PANTALLA_REPORTE) {
            pantallaActual = PANTALLA_MENU;
            redibujar = 1;
            return;
        }
        //🐸 --- Agregar: guardar ---
        if (pantallaActual == PANTALLA_AGREGAR) {
            if (strlen(campos[0].buffer) == 0) {
                mostrarMensaje("CAMPO REQUERIDO",
                               "El titulo no puede estar vacio.", "",
                               PANTALLA_AGREGAR);
                return;
            }
            agregarArticulo(&inventario, &totalArt);
            char msg[80];
            snprintf(msg, 79, "Articulo '%s' agregado con exito.", campos[0].buffer);
            mostrarMensaje("ARTICULO AGREGADO", msg, "", PANTALLA_MENU);
            limpiarCampos();
            return;
        }
        //🐸 --- Modificar: si idOperacion==-2 pedir ID, sino confirmar ---
        if (pantallaActual == PANTALLA_MODIFICAR) {
            if (idOperacion == -2) {
                //🐸 Estamos esperando el ID
                int id = atoi(campos[0].buffer);
                int idx = buscarPorID(inventario, totalArt, id);
                if (idx < 0) {
                    mostrarMensaje("NO ENCONTRADO",
                                   "No existe un articulo con ese ID.", "",
                                   PANTALLA_MENU);
                    return;
                }
                idOperacion = id;
                inicializarCamposModificar(idx);
                redibujar = 1;
                return;
            }
            //🐸 Confirmar modificacion
            int idx = buscarPorID(inventario, totalArt, idOperacion);
            if (idx >= 0) {
                modificarArticulo(inventario, totalArt, idOperacion);
                mostrarMensaje("ACTUALIZADO",
                               "Articulo modificado correctamente.", "",
                               PANTALLA_MENU);
            }
            limpiarCampos();
            return;
        }
        //🐸 --- Buscar: ejecutar busqueda ---
        if (pantallaActual == PANTALLA_BUSCAR) {
            int tipo = atoi(campos[1].buffer);
            int idx  = -1;
            if (tipo == 0) {
                int id = atoi(campos[0].buffer);
                idx = buscarPorID(inventario, totalArt, id);
            } else {
                idx = buscarPorTitulo(inventario, totalArt, campos[0].buffer);
            }
            hayResultado = (idx >= 0);
            if (hayResultado)
                articuloEncontrado = inventario[idx];
            pantallaActual = PANTALLA_RESULTADO_BUSQUEDA;
            redibujar = 1;
            return;
        }
        //🐸 --- Eliminar: confirmar ---
        if (pantallaActual == PANTALLA_ELIMINAR) {
            int id  = atoi(campos[0].buffer);
            int idx = buscarPorID(inventario, totalArt, id);
            if (idx < 0) {
                mostrarMensaje("NO ENCONTRADO",
                               "No existe un articulo con ese ID.", "",
                               PANTALLA_MENU);
                return;
            }
            char msg[100];
            snprintf(msg,99,"'%s' eliminado correctamente.", inventario[idx].titulo);
            eliminarArticulo(&inventario, &totalArt, id);
            mostrarMensaje("ELIMINADO", msg, "", PANTALLA_MENU);
            limpiarCampos();
            return;
        }
    }
}

//🐸 Retorna 1 si el click cayo sobre el boton "Volver al Menu"
static int clickEnBotonMenu(float mx, float my) {
    float bx = (float)(ANCHO_VENTANA - 162);
    float by = 16.0f;
    float bw = 148.0f;
    float bh = 34.0f;
    return (mx >= bx && mx <= bx+bw && my >= by && my <= by+bh);
}

void manejarClick(float mx, float my) {
    //🐸 ---- Boton "Volver al Menu" (esquina superior derecha) ----
    if (pantallaActual != PANTALLA_MENU &&
        pantallaActual != PANTALLA_PROGRESO &&
        pantallaActual != PANTALLA_MENSAJE) {
        if (clickEnBotonMenu(mx, my)) {
            pantallaActual = PANTALLA_MENU;
            limpiarCampos();
            redibujar = 1;
            return;
        }
    }

    //🐸 ---- MENU ----
    if (pantallaActual == PANTALLA_MENU) {
        float px = ANCHO_VENTANA/2 - 220;
        float py = 100;
        float pw = 440;
        int i;
        for (i = 0; i < 7; i++) {
            float oy = py + 50 + i * 52;
            if (mx >= px+30 && mx <= px+pw-30 &&
                my >= oy    && my <= oy+38) {
                //🐸 Simular presionar la tecla correspondiente
                ALLEGRO_EVENT fake;
                fake.type = ALLEGRO_EVENT_KEY_DOWN;
                fake.keyboard.keycode = ALLEGRO_KEY_1 + i;
                manejarTecla(&fake);
                return;
            }
        }
        return;
    }

    //🐸 ---- TABLA: botones de paginacion ----
    if (pantallaActual == PANTALLA_MOSTRAR) {
        int totalPag = (totalArt + FILAS_POR_PAG - 1) / FILAS_POR_PAG;
        //🐸 Anterior
        if (mx>=30 && mx<=150 && my>=(ALTO_VENTANA-42) && my<=(ALTO_VENTANA-10)) {
            if (paginaTabla > 0) { paginaTabla--; redibujar = 1; }
        }
        //🐸 Siguiente
        if (mx>=(ANCHO_VENTANA-155) && mx<=(ANCHO_VENTANA-35) &&
            my>=(ALTO_VENTANA-42) && my<=(ALTO_VENTANA-10)) {
            if (paginaTabla < totalPag-1) { paginaTabla++; redibujar = 1; }
        }
        return;
    }

    //🐸 ---- FORMULARIOS: click en campo ----
    if (pantallaActual == PANTALLA_AGREGAR ||
        pantallaActual == PANTALLA_MODIFICAR ||
        pantallaActual == PANTALLA_BUSCAR ||
        pantallaActual == PANTALLA_ELIMINAR) {
        int idx = campoEnPosicion(mx, my);
        if (idx >= 0) {
            campoActivo = idx;
            redibujar = 1;
            return;
        }

        //🐸 Botones de accion
        //🐸 Boton principal (Guardar / Buscar / Eliminar / Actualizar)
        float px, py, pw, ph;
        if (pantallaActual == PANTALLA_AGREGAR) {
            px=220; py=95; pw=560; ph=460;
            //🐸 Guardar
            if (mx>=px+30 && mx<=px+210 && my>=py+ph-56 && my<=py+ph-18) {
                ALLEGRO_EVENT fake; fake.type=ALLEGRO_EVENT_KEY_DOWN;
                fake.keyboard.keycode=ALLEGRO_KEY_ENTER;
                manejarTecla(&fake); return;
            }
            //🐸 Cancelar
            if (mx>=px+pw-210 && mx<=px+pw-30 && my>=py+ph-56 && my<=py+ph-18) {
                pantallaActual=PANTALLA_MENU; limpiarCampos(); redibujar=1; return;
            }
        }
        if (pantallaActual == PANTALLA_BUSCAR) {
            px=280; py=110; pw=440; ph=300;
            if (mx>=px+30 && mx<=px+210 && my>=py+ph-56 && my<=py+ph-18) {
                ALLEGRO_EVENT fake; fake.type=ALLEGRO_EVENT_KEY_DOWN;
                fake.keyboard.keycode=ALLEGRO_KEY_ENTER;
                manejarTecla(&fake); return;
            }
            if (mx>=px+pw-210 && mx<=px+pw-30 && my>=py+ph-56 && my<=py+ph-18) {
                pantallaActual=PANTALLA_MENU; limpiarCampos(); redibujar=1; return;
            }
        }
        if (pantallaActual == PANTALLA_ELIMINAR) {
            px=300; py=130; pw=400; ph=250;
            if (mx>=px+20 && mx<=px+180 && my>=py+ph-52 && my<=py+ph-14) {
                ALLEGRO_EVENT fake; fake.type=ALLEGRO_EVENT_KEY_DOWN;
                fake.keyboard.keycode=ALLEGRO_KEY_ENTER;
                manejarTecla(&fake); return;
            }
            if (mx>=px+pw-180 && mx<=px+pw-20 && my>=py+ph-52 && my<=py+ph-14) {
                pantallaActual=PANTALLA_MENU; limpiarCampos(); redibujar=1; return;
            }
        }
        if (pantallaActual == PANTALLA_MODIFICAR) {
            //🐸 Pantalla de pedir ID
            if (idOperacion == -2) {
                //🐸 solo hay un campo; ENTER al hacer click en "Buscar"
                if (my >= 350 && my <= 400) {
                    ALLEGRO_EVENT fake; fake.type=ALLEGRO_EVENT_KEY_DOWN;
                    fake.keyboard.keycode=ALLEGRO_KEY_ENTER;
                    manejarTecla(&fake); return;
                }
            } else {
                //🐸 Formulario con 5 campos
                px=220; py=95; pw=560; ph=460;
                if (mx>=px+30 && mx<=px+210 && my>=py+ph-56 && my<=py+ph-18) {
                    ALLEGRO_EVENT fake; fake.type=ALLEGRO_EVENT_KEY_DOWN;
                    fake.keyboard.keycode=ALLEGRO_KEY_ENTER;
                    manejarTecla(&fake); return;
                }
                if (mx>=px+pw-210 && mx<=px+pw-30 && my>=py+ph-56 && my<=py+ph-18) {
                    pantallaActual=PANTALLA_MENU; limpiarCampos(); redibujar=1; return;
                }
            }
        }
        return;
    }

    //🐸 ---- RESULTADO / REPORTE: boton volver ----
    if (pantallaActual == PANTALLA_RESULTADO_BUSQUEDA) {
        float px=250,py=100,pw=500,ph=360;
        if (mx>=px+pw/2-70 && mx<=px+pw/2+70 && my>=py+ph-52 && my<=py+ph-14) {
            pantallaActual=PANTALLA_MENU; redibujar=1;
        }
        return;
    }
    if (pantallaActual == PANTALLA_REPORTE) {
        float px=220,py=100,pw=560,ph=400;
        if (mx>=px+pw/2-80 && mx<=px+pw/2+80 && my>=py+ph-52 && my<=py+ph-14) {
            pantallaActual=PANTALLA_MENU; redibujar=1;
        }
        return;
    }

    //🐸 ---- MENSAJE: boton aceptar ----
    if (pantallaActual == PANTALLA_MENSAJE) {
        float px=ANCHO_VENTANA/2-250, py=ALTO_VENTANA/2-130, pw=500, ph=260;
        if (mx>=px+pw/2-70 && mx<=px+pw/2+70 && my>=py+ph-56 && my<=py+ph-18) {
            pantallaActual=mensajeRetorno; redibujar=1;
        }
        return;
    }
}

/* ============================================================
 *  PANTALLA DE MODIFICAR: dibujo especial cuando se pide el ID
 * ============================================================ */
//🐸 Sobreescribimos dibujarFormModificar para manejar los dos estados
//🐸 Ya declarado arriba; redefinimos con logica interna

/* ============================================================
 *  MAIN
 * ============================================================ */

int main(void) {
    //🐸 ---- Iniciar Allegro ----
    if (!al_init()) return 1;
    if (!al_install_keyboard()) return 1;
    if (!al_install_mouse()) return 1;
    if (!al_init_font_addon()) return 1;
    al_init_ttf_addon();
    if (!al_init_primitives_addon()) return 1;

    al_set_new_display_flags(ALLEGRO_WINDOWED);
    display = al_create_display(ANCHO_VENTANA, ALTO_VENTANA);
    if (!display) return 1;
    al_set_window_title(display, "Biblioteca Personal");

    //🐸 Timer para animacion / cursor
    timer = al_create_timer(1.0/30.0);  //🐸 30 FPS
    if (!timer) return 1;

    ev_queue = al_create_event_queue();
    if (!ev_queue) return 1;
    al_register_event_source(ev_queue, al_get_keyboard_event_source());
    al_register_event_source(ev_queue, al_get_mouse_event_source());
    al_register_event_source(ev_queue, al_get_display_event_source(display));
    al_register_event_source(ev_queue, al_get_timer_event_source(timer));

    //🐸 Fuentes
    //🐸 Se intenta cargar fuentes formales en orden de preferencia
    fnt_grande = al_load_ttf_font("Georgia Bold.ttf",      30, 0);
    if (!fnt_grande) fnt_grande = al_load_ttf_font("georgiab.ttf",        30, 0);
    if (!fnt_grande) fnt_grande = al_load_ttf_font("TimesNewRoman-Bold.ttf", 30, 0);
    if (!fnt_grande) fnt_grande = al_load_ttf_font("times new roman bold.ttf", 30, 0);
    if (!fnt_grande) fnt_grande = al_load_ttf_font("DejaVuSerif-Bold.ttf", 30, 0);
    if (!fnt_grande) fnt_grande = al_load_ttf_font("DejaVuSans-Bold.ttf",  30, 0);
    if (!fnt_grande) fnt_grande = al_create_builtin_font();

    fnt_normal = al_load_ttf_font("Georgia.ttf",           17, 0);
    if (!fnt_normal) fnt_normal = al_load_ttf_font("georgia.ttf",          17, 0);
    if (!fnt_normal) fnt_normal = al_load_ttf_font("TimesNewRoman.ttf",    17, 0);
    if (!fnt_normal) fnt_normal = al_load_ttf_font("times new roman.ttf",  17, 0);
    if (!fnt_normal) fnt_normal = al_load_ttf_font("DejaVuSerif.ttf",      17, 0);
    if (!fnt_normal) fnt_normal = al_load_ttf_font("DejaVuSans.ttf",       17, 0);
    if (!fnt_normal) fnt_normal = al_create_builtin_font();

    fnt_small  = al_load_ttf_font("Georgia.ttf",           14, 0);
    if (!fnt_small)  fnt_small  = al_load_ttf_font("georgia.ttf",          14, 0);
    if (!fnt_small)  fnt_small  = al_load_ttf_font("TimesNewRoman.ttf",    14, 0);
    if (!fnt_small)  fnt_small  = al_load_ttf_font("times new roman.ttf",  14, 0);
    if (!fnt_small)  fnt_small  = al_load_ttf_font("DejaVuSerif.ttf",      14, 0);
    if (!fnt_small)  fnt_small  = al_load_ttf_font("DejaVuSans.ttf",       14, 0);
    if (!fnt_small)  fnt_small  = al_create_builtin_font();

    //🐸 Cargar datos
    cargarArchivoBinario(&inventario, &totalArt);
    pantallaActual = PANTALLA_MENU;
    redibujar = 1;

    al_start_timer(timer);

    //🐸 ---- Bucle principal ----
    int corriendo = 1;
    while (corriendo) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(ev_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            guardarArchivoBinario(inventario, totalArt);
            corriendo = 0;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            manejarTecla(&ev);
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
            manejarCaracter(&ev);
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            manejarClick((float)ev.mouse.x, (float)ev.mouse.y);
        }
        else if (ev.type == ALLEGRO_EVENT_TIMER) {
            //🐸 Cursor parpadeante cada 15 ticks (~0.5 s)
            cursorTick++;
            if (cursorTick >= 15) {
                cursorTick    = 0;
                cursorVisible = !cursorVisible;
                redibujar = 1;
            }
        }

        //🐸 Redibujar solo cuando es necesario
        if (redibujar && al_is_event_queue_empty(ev_queue)) {
            dibujar();
            redibujar = 0;
        }
    }

    //🐸 ---- Liberar recursos ----
    if (inventario) { free(inventario); inventario = NULL; }
    al_destroy_font(fnt_small);
    al_destroy_font(fnt_normal);
    al_destroy_font(fnt_grande);
    al_destroy_timer(timer);
    al_destroy_event_queue(ev_queue);
    al_destroy_display(display);

    return 0;
}
