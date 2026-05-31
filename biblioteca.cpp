#include <iostream>
#include <string>
#include <conio.h>

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
    cout<< "-----SISTEMA DE BIBLIOTECA (MENU)-----"<<endl;

    getch();
    return 0;
}