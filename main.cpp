#include <iostream>
#include <string>

#include "BufferManager.h"
#include "Page.h"
#include "Frame.h"

BufferManager *bm;
vector<Page*> paginas;

void menu();

using namespace std;

void solicitarPagina(){
    cout << "\t> SOLICITUD\n";
    int idPagina;
    char accion;
    do{
        cout << "\tID de la pagina: ";
        cin >> idPagina;
    } while (idPagina > 100 || idPagina <= 0);
    
    cout << "\tLectura (R) o escritura (W)?: ";
    cin >> accion;
    if (accion == 'R'){
        accion = false;
    } else if (accion == 'W'){
        accion = true;
    }
    cout << endl;
    bm->pinPage(idPagina, paginas[idPagina-1], accion);
}

void liberarPagina() {
    int pageId;
    cout << "\tIngrese el ID de la pagina a liberar: ";
    cin >> pageId;

    if (bm->pageExists(pageId)) { 
        bm->unpinPage(pageId);
        cout << "\tPinCount de la página ha sido decrementado.\n";
    } else {
        cout << "\tLa página con ID " << pageId << " no existe en el buffer.\n";
    }
}

void menu()
{
    char opcion;
    cout << "---------------------------\n";
    cout << ">>>> BUFFER MANAGER\n";
    cout << "---------------------------\n";

    cout << "Numero de frames: ";
    int numFrames;
    cin >> numFrames;
    bm = new BufferManager(numFrames);

    do {
        cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><\n";
        cout << ">> OPTIONS\n";
        cout << "1. Solicitar una pagina\n";
        cout << "2. Imprimir Buffer Pool\n";
        cout << "3. Liberar una pagina\n";
        cout << "4. Imprimir Hit Rate\n";
        cout << "5. Salir\n";
        cout << "Opcion? ";
        cin >> opcion;
        
        switch(opcion)
        {
            case '1':
                solicitarPagina();
                break;
            case '2':
                bm->printFrame();
                break;
            case '3':
                liberarPagina();
                break;
            case '4':
                cout << "\t> HIT RATE: " << bm->hitRate() << endl;
                break;
            case '5':
                cout << "\t> ADIOS!\n";
                break;
            default:
                break;
        }
    } while (opcion != '5');
}

int main (){
    
    paginas.resize(100);
    for (int i = 0; i < 100; i++) {
        paginas[i] = new Page(i+1);
    }
    menu();

    return 0;
}