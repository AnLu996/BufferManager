#ifndef FRAME_H
#define FRAME_H

#include <iostream>
#include <string>

#include "Page.h"

using namespace std;

class Frame {
    private:
        int id;
        Page* pagina;
        bool dirtyFlag;
        int pinCount;
        int lastUsed;
        bool refBit;
    public:
        Frame(int id): dirtyFlag(0), pinCount(0), lastUsed(0), refBit(false) {
            this->id = id;
            pagina = nullptr;
        }

        int getId(){ return id;  }
        
        int getPinCount(){  return pinCount;    }
        
        void resetFrame() {
            delete pagina;
            pagina = nullptr;
            dirtyFlag = false;
            pinCount = 0;
            lastUsed = 0;
        }

        Page* getPagina() { return pagina; }
        void setPage(Page *pag) { this->pagina = pag; }
        void incrementPinCount() { pinCount++; }
        void decrementPinCount() { if (pinCount > 0) pinCount--; }
        void setLastUsed(int lused) { this->lastUsed = lused; }
        bool getDirty() { return dirtyFlag; }
        void setDirty(bool accion) { dirtyFlag = accion; }
        void saveChanges() { if (pagina) pagina->save(); }
        void incrementLastUsed(){   lastUsed++; }
        int getLastUsed(){  return lastUsed;    }
        bool getRefBit(){  return refBit;    }
        void setRefBit(bool state){  this->refBit = state;      }

};

#endif //FRAME_H