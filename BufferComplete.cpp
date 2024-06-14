#include <iostream>
#include <map>
#include <vector>

using namespace std;

class Page;
class Frame;
class BufferManager;
BufferManager* bm;
vector<Page*> paginas;

void menu();

class Page {
public:
    int id;
    Page(int id) : id(id) {}
    void save() {} 
};

class Frame {
    private:
        int id;
        Page* pagina;
        bool dirtyFlag;
        int pinCount;
        bool pin; //Pin o unnpin page

        int lastUsed;
        bool refBit;
    public:
        Frame(int id): dirtyFlag(0), pinCount(0), lastUsed(0), refBit(false), pin(false) {
            this->id = id;
            pagina = nullptr;
        }

        int getId(){ return id;  }
        
        int getPinCount(){  return pinCount;    }

        bool getPin(){  return pin;    }  //Pin o unnpin page

        int getLastUsed(){  return lastUsed;    }

        bool getRefBit(){  return refBit;    }

        void setRefBit(bool state){  this->refBit = state;      }
        
        void resetFrame() {
            delete pagina;
            pagina = nullptr;
            dirtyFlag = false;
            pinCount = 0;
            pin = 0; //Pin o unnpin page
            lastUsed = 0;
            refBit = false;
        }

        Page* getPagina() { return pagina; }
        void setPage(Page *pag) { this->pagina = pag; }
        void incrementPinCount() { pinCount++; }
        void decrementPinCount() { if (pinCount > 0) pinCount--; }
        void setLastUsed(int lused) { this->lastUsed = lused; }
        bool getDirty() { return dirtyFlag; }
        void setDirty(bool accion) { dirtyFlag = accion; }
        void saveChanges() { if (pagina) pagina->save(); }
        void incrementLastUsed() {   lastUsed++; }

        void pinPage() { this->pin = true; }  //Pin o unnpin page

        void unnpinPage() { this->pin = false; } //Pin o unnpin page

};

class BufferManager {
    private:
        map<int, int> pageTable;
        vector<Frame*> bufferPool;
        string methodReplace;
        int clockHand;
        int miss;
        int hits;
        int solicitudes;

    public:

        /*
        Constructor por defecto
            poolSize = indica el numero de frames   
        */
        BufferManager() : miss(0), hits(0), solicitudes(0), clockHand(0) {
            int poolSize = 4; 
            bufferPool.resize(poolSize);
            for (int i = 0; i < poolSize; i++) {
                bufferPool[i] = new Frame(i);
            }

        }

        /*
        Constructor con parametros
        */
        BufferManager(int poolSize) : miss(0), hits(0), solicitudes(0) {
            bufferPool.resize(poolSize);
            for (int i = 0; i < poolSize; i++) {
                bufferPool[i] = new Frame(i);
            }

            //MRU, LRU, CLOCK
            methodReplace = "LRU"; //INDICADOR DE MÉTODO DE REEMPLAZO
        }

        ~BufferManager() {
            for (auto& frame : bufferPool) {
                delete frame;
            }
        }

        /*
        AUTORES: Andrea Cuela
        Verifica que el bufferPool este lleno
        Itera en cada frame en bufferPool, si hay un frame que está vacío, el buffer no está lleno
        */

        bool isBufferFull() {
            for (auto& frame : bufferPool) {
                if (frame->getPagina() == nullptr) {
                    return false;
                }
            }
            return true;
        }


        /*
        AUTORES: Melany Cahuana
        Imprime los valores del page table (id pagina y id frame)
        */
        void printPageTable(){
            cout << "Page table: "<<endl;
            cout << "page_id  frame_id"<< endl;
            for (const auto &entry: pageTable) {
                cout << "{" << entry.first << ", " << entry.second << "}" << endl;
            }
        }

        /*
        AUTORES: Melany Cahuana
        Incrementa todos los last used de todos los frames
        */
        void aumentarLastUsed(){
            for (auto& frame: bufferPool){
                frame->incrementLastUsed();
            }
        }

        /*
        AUTORES: Andrea Cuela y Melany Cahuana
        Se imprime informacion del frame:
            frame_id, page_id, mode (dirty bit), pinCount, last_used    
        */
        void printFrame(){
            cout << "\t> Buffer Pool\n";
            cout << "\tframe_id    page_id    mode    pinCount    pin    last_used    refBit"<< endl;
            for (auto& frame: bufferPool){
                cout << '\t' << frame->getId() << "\t\t";
                if (frame->getPagina() == nullptr){
                    cout << "-" << "\t-" << "\t-" << "\t    -" << "\t\t-" << "\t-" << endl;
                } else {
                    cout << frame->getPagina()->id << "\t";
                    if (frame->getDirty()) cout << "W\t";
                    else cout << "R\t";
                    cout << frame->getPinCount() << "\t     " << static_cast<int>(frame->getPin()) << "\t\t" << frame->getLastUsed() << "\t" << static_cast<int>(frame->getRefBit());
                    cout << endl;
                }
            }

            if (methodReplace == "CLOCK")
                cout << "\n\t> clockHand -> " << clockHand << endl;
        }

        float hitRate() { return (float)hits/solicitudes; }

        /*
        AUTORES: Andrea Cuela, Melany Cahuana y Giomar Muñoz
        Maneja el ingreso de paginas, busca si la página está en el Buffer Pool
            Si esta, incrementa el hits, modifica el last used y ve en que modo esta (escritura o lectura)
            Si no está, encontra un frame libre y anexar la página ahi
        */
        void pinPage(int pageId, Page* page, bool accion){
            Frame* framePage = nullptr;

            if (pageTable.count(pageId)) {
                hits++;
                framePage = bufferPool[pageTable[pageId]];
                if (framePage->getDirty() && !accion) flushPage(pageId);
                framePage->setDirty(accion);    //si va a cambiar a modo lectura
                if (methodReplace != "CLOCK") {
                    framePage->setLastUsed(0);  //resetea lastUsed a 0 si se est agregando de nuevo esa pagina
                }      
            }
            else {
                newPage(pageId, page, accion);
                miss++;
                // Solo establece framePage si newPage fue exitoso.
                if (pageTable.count(pageId)) {
                    framePage = bufferPool[pageTable[pageId]];
                }
            }

            // Asegura que framePage no sea nulo antes de intentar usarlo.
            if (framePage) {
                framePage->incrementPinCount(); // Incrementa pinCount porque la página comenzará a ser usada.
                if (methodReplace == "CLOCK") {
                    if (framePage->getPin() == false) //Se verifica que el pin sea falso para poder cambiar el refBit en CLOCK
                        framePage->setRefBit(true); // Establece el bit de referencia a 1 para CLOCK       
                } else {
                    //f(framePage->getPin() == false && methodReplace != "LRU")
                    aumentarLastUsed();         // Aumenta lastUsed para todos los frames en uso.
                }
            } else {
                cout << "\tError: No se pudo encontrar o crear un frame para la pagin " << pageId << endl;
                return;
            }         
            
            solicitudes++;
            cout << "\t> INFORMACION DE SOLICITUD\n";
            cout << "\tNro de solicitud: "<< solicitudes << endl;
            cout << "\tMisses: " << miss << endl;
            cout << "\tHits: " << hits << endl;
        }

        /*
        AUTORES: Melany Cahuana
        Maneja la operacion de 'despinear' una pagina de acuerdo a su id, decrementando
        su pincount (indicando que se estan disminuyendo accesos a esa pagina)
        */
        void unpinPage(int pageId) {
            Frame* framePage;
            if (pageTable.count(pageId)) {
                framePage = bufferPool[pageTable[pageId]];
                framePage->decrementPinCount();
            }
        }
        
        /*
        AUTORES: Andrea Cuela, Melany Cahuana y Giomar Muñoz
        Encuentra un frame disponible donde anexar la pagina solicitada,
        sino encuentra reemplaza al frame con mayor tiempo de existencia
        */
        void newPage(int pageId, Page* page, bool accion){
            Frame* framePage = nullptr;
            
            bool bufferIsFull = isBufferFull(); // Verifica si el buffer está lleno

            int selectedLastUsed;
            
            // Verifica si el buffer está lleno y ajusta la búsqueda de frame libre
            for (auto& frame : bufferPool) {
                if (frame->getPin() == false && frame->getPinCount() == 0) { // Se añade la verificacion de pin (si está fijado o no)
                    if (!bufferIsFull) {
                        framePage = frame;
                        break;
                    } else {
                        // Si el buffer está lleno, aplica las políticas MRU y LRU correctamente
                        if ((methodReplace == "MRU" && frame->getLastUsed() < selectedLastUsed) || 
                            (methodReplace == "LRU" && frame->getLastUsed() > selectedLastUsed)) {
                            framePage = frame;
                            selectedLastUsed = frame->getLastUsed();  // Actualiza el valor de comparación
                        }
                    }
                }
            }

            // Si después del ciclo no se encontró ningún frame, maneja según la política
            if (!framePage) {
                framePage = replaceFrame();

                // Manejo de errores si aún no hay frame disponible
                if (!framePage) {
                    cout << "\tNo se pudo agregar la pagina porque no hay frames libres ni reemplazables." << endl;
                    return;                 // Salir si no se encuentra un frame reemplazable
                }
            }

            // Configura el frame con la nueva página
            framePage->setPage(page); 
            pageTable[pageId] = framePage->getId();
            framePage->setDirty(accion);    // Asigna si está en escritura o en lectura
            if (methodReplace == "CLOCK") {
                framePage->setRefBit(true); // Establece el bit de referencia a 1 para CLOCK       
            } else {
                framePage->setLastUsed(0);  // Resetea lastUsed a 0 para la nueva página
            }

        }

        /*
        AUTORES: Andrea Cuela
        Esta función se encarga de reconocer el método de reemplazo a usar por el 
        BufferManager indicado en el constructor
        */

        Frame* replaceFrame() {
            Frame* framePage = nullptr;
            if (methodReplace == "MRU") {
                framePage = replaceFrameMRU();
            } else if (methodReplace == "LRU") {
                framePage = replaceFrameLRU();
            } else if (methodReplace == "CLOCK") {
                framePage = replaceFrameCLOCK();
            } else {
                return nullptr;
            }

            return framePage;
        }

        /*
        AUTORES: Andrea Cuela, Melany Cahuana y Giomar Muñoz
        Esta funcion se encarga de indicar cual es el frame que se debe reemplazar de 
        acuerdo al last used mas alto (LRU)
        */
        Frame* replaceFrameLRU() {
            Frame* frameToReplace = nullptr;
            int highestLastUsed = -1;

            for (auto& frame : bufferPool) {
                cout << "Frame ID: " << frame->getId() << " Pin Count: " << frame->getPinCount() << " Last Used: " << frame->getLastUsed() << endl;
                if (frame->getPin() == false && frame->getPinCount() == 0 && frame->getLastUsed() > highestLastUsed ) { // Se añade la verificacion de pin (si está fijado o no)
                    frameToReplace = frame;
                    highestLastUsed = frame->getLastUsed();
                }
            }

            if (!frameToReplace) {
                cout << "\t\nNo hay frames disponibles para reemplazar. Todos están en uso." << endl;
                return nullptr; 
            }

            // Procede a reemplazar el frame seleccionado.
            if (frameToReplace->getDirty()) {
                flushPage(frameToReplace->getPagina()->id); // Guardar cambios si está dirty
            }   
            freePage(frameToReplace->getPagina()->id);      // Liberar la página actual del frame

            return frameToReplace;
        }

        /*
        AUTORES: Andrea Cuela, Melany Cahuana y Giomar Muñoz
        Esta funcion se encarga de indicar cual es el frame que se debe reemplazar de 
        acuerdo al last used mas bajo (MRU) 
        */
        Frame* replaceFrameMRU() {
            Frame* frameToReplace = nullptr;
            int lowestLastUsed = 2147483647; 

            for (auto& frame : bufferPool) {
                cout << "---------- Frame ID: " << frame->getId() << " Pin Count: " << frame->getPinCount() << " Last Used: " << frame->getLastUsed() << endl;
                if (frame->getPin() == false && frame->getPinCount() == 0 && frame->getLastUsed() < lowestLastUsed) {  // Se añade la verificacion de pin (si está fijado o no)
                    frameToReplace = frame;
                    lowestLastUsed = frame->getLastUsed();
                }
            }

            if (!frameToReplace) {
                cout << "\t\nNo hay frames disponibles para reemplazar. Todos están en uso." << endl;
                return nullptr;
            }

            // Procede a reemplazar el frame seleccionado.
            if (frameToReplace->getDirty()) {
                flushPage(frameToReplace->getPagina()->id); // Guardar cambios si está dirty
            }
            freePage(frameToReplace->getPagina()->id);      // Liberar la página actual del frame

            return frameToReplace;
        }

        /*
        AUTORES: Andrea Cuela, Melany Cahuana y Giomar Muñoz
        Esta funcion se encarga de indicar cual es el frame que se debe reemplazar de 
        acuerdo a la estrategia de reemplazo CLOCK. 
        */
        Frame* replaceFrameCLOCK() {
            int bufferPoolSize = bufferPool.size();
            int counter = 0;
            
            cout << "\n\t > SIMULANDO CLOCK" << endl;
            cout << "\t clockHand -> " << clockHand << endl;

            while (true) {
                Frame* frame = bufferPool[clockHand];

                if (frame->getPin() == false) {  // Se añade la verificacion de pin (si está fijado o no)
                    if (frame->getRefBit()) {
                        frame->setRefBit(false);
                        cout << "\t*** SECOND CHANCE! ***" << endl;
                    } else {
                        // Procede a reemplazar el frame seleccionado.
                        if (frame->getPinCount() == 0) {

                            if (frame->getDirty()) {
                                cout << endl;
                                flushPage(frame->getPagina()->id); // Guardar cambios si está dirty
                                cout << endl;
                            }
                            freePage(frame->getPagina()->id); // Liberar la página actual del frame
                            clockHand = (clockHand + 1) % bufferPoolSize;
                            cout << "\t clockHand -> " << clockHand << endl;
                            
                            return frame;
                        } 
                    }
                }

                clockHand = (clockHand + 1) % bufferPoolSize;
                counter++;
                cout << "\t clockHand -> " << clockHand << endl;

                // Si hemos recorrido todo el bufferPool y no hemos encontrado un frame para reemplazar
                if (counter == bufferPoolSize) {
                    cout << "\t\nNo hay frames disponibles para reemplazar. Todos están en uso." << endl;
                    return nullptr; 
                }
            }
        }

        /*
        AUTORES:Giomar Muñoz
        Resetea el frame a desalojar y lo elimina del Page Table
        */
        void freePage(int pageId){
            Frame* framePage;
            // Resetea el frame para usar otra vez
            if (pageTable.count(pageId)) {
                framePage = bufferPool[pageTable[pageId]];
                framePage->resetFrame();
            }
            // Eliminar la entrada de la página vieja en pageTable
            pageTable.erase(pageId);
        }

        /*
        AUTORES:Giomar Muñoz
        Guarda los cambios que se hicieron en el buffer a disco
        */
        void flushPage(int pageId) {
            Frame* framePage;
            if (pageTable.count(pageId)) {
                framePage = bufferPool[pageTable[pageId]];
                
                cout << "\tGuardar cambios de la pagina " << framePage->getPagina()->id << "? (true = 1 / false = 0)?: ";
                bool saveC;
                cin >> saveC;
                if (saveC){
                    framePage->saveChanges();
                    cout << "\tSe guardaron los datos con exito!\n";
                } else{
                    cout << "\tNo se guardaron los cambios\n";
                }
                
            }
        }

        /*
        AUTORES:Giomar Muñoz
        Función para verificar si una página existe en el buffer
        */
        bool pageExists(int pageId) {
            // Usa la pageTable para verificar si el ID de página está presente
            return pageTable.count(pageId) > 0;
        }

        /*
        AUTORES: Melany Cahuana y Andrea Cuela
        Función encargada de pinnear una pagina en un frame
        */
        void markPagePinned(int pageId) {
            Frame* framePage;
            if (pageTable.count(pageId)) {
                framePage = bufferPool[pageTable[pageId]];
                if (framePage->getPin()) {
                    cout << "\tERROR: La pagina " << pageId << " ya esta fijada!\n";
                } else {
                    framePage->pinPage();
                    cout << "\tPagina " << pageId << " fijada.\n";
                }
            }
        }

        /*
        AUTORES: Melany Cahuana y Andrea Cuela
        Función encargada de unpinnear una pagina en un frame
        */
        void markPageUnpinned(int pageId) {
            Frame* framePage;
            if (pageTable.count(pageId)) {
                framePage = bufferPool[pageTable[pageId]];
                if (!framePage->getPin())
                    cout << "\tERROR: La pagina " << pageId << " no esta fijada!\n";
                else {
                    framePage->unnpinPage();
                    cout << "\tPagina " << pageId << " desfijada.\n";
                }
            }
        }
};

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
        cout << "\tPinCount de la pagina ha sido decrementado.\n";
    } else {
        cout << "\tLa pagin con ID " << pageId << " no existe en el buffer.\n";
    }
}

void menu()
{
    char opcion;
    int pageId;
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
        cout << "2. Fijar una pagina\n";
        cout << "3. Desfijar una pagina\n";
        cout << "4. Liberar una pagina\n";        
        cout << "5. Imprimir Page Table (Buffer Pool)\n";
        cout << "6. Imprimir Hit Rate\n";
        cout << "7. Salir\n";
        cout << "Opcion? ";
        cin >> opcion;
        
        switch(opcion)
        {
            case '1':
                solicitarPagina();
                break;
            case '2': 
            {
                int pageId;
                cout << "\tIngrese el ID de la pagina a fijar: ";
                cin >> pageId;

                if (bm->pageExists(pageId))
                    bm->markPagePinned(pageId);
                else
                    cout << "\tLa pagina con ID " << pageId << " no existe en el buffer.\n";
                break;
            }
            case '3':
            {
                int pageId;
                cout << "\tIngrese el ID de la pagina a desfijar: ";
                cin >> pageId;

                if (bm->pageExists(pageId))
                    bm->markPageUnpinned(pageId);
                else
                    cout << "\tLa pagina con ID " << pageId << " no existe en el buffer.\n";
                break;
            }
            case '4':
                liberarPagina();
                break;
            case '5':
                bm->printFrame();
                break;
            case '6':
                cout << "\t> HIT RATE: " << bm->hitRate() << endl;
                break;
            case '7':
                cout << "\t> ADIOS!\n";
                break;
            default:
                break;
        }
    } while (opcion != '7');
}

int main (){
    
    paginas.resize(100);
    for (int i = 0; i < 100; i++) {
        paginas[i] = new Page(i+1);
    }
    menu();

    return 0;
}