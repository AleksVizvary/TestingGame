#include <SFML/Graphics.hpp>
#include <optional>
#include <utility>

enum class Faza {czekanie, picie, celowanie, bieg, powrot};
enum class Zdarzenie {przeciwnikBiegnie, przeciwnikCzeka, trafiłeś, podniosles, wrociles};

struct Cel {
    float polozenieX, polozenieY;
    Cel (float polozenieX, float polozenieY);
    
    void ustawPolozenie(float x, float y){
        polozenieX = x;
        polozenieY = y;
    }
    
    std::pair<float, float> zwrocPolozenie() {
    return {polozenieX, polozenieY};
}
};

struct Postac {

    Faza fazaPostaci = Faza::czekanie;

    Postac();
        
    Faza zmienStan(Zdarzenie zdarzenie){

         if (fazaPostaci == Faza::czekanie  && zdarzenie == Zdarzenie::przeciwnikBiegnie) fazaPostaci = Faza::picie;
    else if (fazaPostaci == Faza::picie     && zdarzenie == Zdarzenie::przeciwnikCzeka)   fazaPostaci = Faza::celowanie;
    else if (fazaPostaci == Faza::celowanie && zdarzenie == Zdarzenie::trafiłeś)          fazaPostaci = Faza::bieg;
    else if (fazaPostaci == Faza::bieg      && zdarzenie == Zdarzenie::podniosles)        fazaPostaci = Faza::powrot;
    else if (fazaPostaci == Faza::powrot    && zdarzenie == Zdarzenie::wrociles)          fazaPostaci = Faza::czekanie;

    return fazaPostaci;
    }

    void czekanie() {
    }
    void picie() {
    }
    void celowanieRzut() {
    } 
    void bieg() {
    }
    void podnos() {
    }
    void powrot() {
    }
};


struct GameLoop
{       
    Postac Gracz1;
    Postac Gracz2;

    GameLoop();

    void obsluzPostac(Postac& Gracz1, Postac& Gracz2){
        switch (Gracz1.fazaPostaci)
        {
        case Faza::czekanie:
            Gracz1.czekanie();
            Gracz2.celowanieRzut();
            break;
            
        case Faza::picie:
            Faza fazaGracz2 = Gracz2.fazaPostaci;

            Gracz1.picie();
            if (fazaGracz2 == Faza::bieg)
                Gracz2.bieg();
            
            else if (fazaGracz2 == Faza::powrot)
                Gracz2.powrot();

            break;
            
        case Faza::celowanie:
            Gracz1.celowanieRzut();
            Gracz2.czekanie();
            break;

        case Faza::bieg:
            Gracz1.bieg();
            Gracz2.picie();
            break;

        case Faza::powrot:
            Gracz1.powrot();
            Gracz2.picie();
            break;
            
        default:
            break;
        }
    };

};
