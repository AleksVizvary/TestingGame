#include <optional>
#include <utility>
#include <ctime>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

enum class Faza {czekanie, picie, celowanie, bieg, powrot};
enum class Zdarzenie {przeciwnikBiegnie, przeciwnikCzeka, trafiłeś, podniosles, wrociles, start};

struct Napoj 
{
    Napoj();

    void pij()
    {
        return;
    }

};



struct Cel {
    float polozenie;
    Cel(float polozenie)
        : polozenie(polozenie)
    {
    }
    
    void ustawPolozenie(float x){
        polozenie = x;
    }
    
    float zwrocPolozenie() {
    return polozenie;
    }
};

struct Pocisk
{
    float x = 0.f;
    float y = 0.f;

    float vx = 0.f;
    float vy = 0.f;

    bool leci = false;

    Pocisk(float startX) : x(startX) 
    {
    }

    void start(float startX, float angle, float speed)
    {
        x = startX;
        y = 0.f;

        vx = std::cos(angle) * speed;
        vy = std::sin(angle) * speed;

        leci = true;
    }

    void update(float dt)
    {
        if (!leci) return;

        const float g = 9.81f;

        vy -= g * dt;
        x  += vx * dt;
        y  += vy * dt;
    }

    bool wyladowal()
    {
        return leci && y <= 0.f;
    }
};

struct Postac {
    float predkosc = 10.f;
    float pozycja;
    Faza fazaPostaci = Faza::czekanie;
    Pocisk pocisk;

    Postac(float pozycja)
        : pocisk(pozycja)
    {
    }

    Faza zmienStan(Zdarzenie zdarzenie){
            
             if (fazaPostaci == Faza::czekanie  && zdarzenie == Zdarzenie::przeciwnikBiegnie) fazaPostaci = Faza::picie;
        else if (fazaPostaci == Faza::picie     && zdarzenie == Zdarzenie::przeciwnikCzeka)   fazaPostaci = Faza::celowanie;
        else if (fazaPostaci == Faza::celowanie && zdarzenie == Zdarzenie::trafiłeś)          fazaPostaci = Faza::bieg;
        else if (fazaPostaci == Faza::bieg      && zdarzenie == Zdarzenie::podniosles)        fazaPostaci = Faza::powrot;
        else if (fazaPostaci == Faza::powrot    && zdarzenie == Zdarzenie::wrociles)          fazaPostaci = Faza::czekanie;
        
        return fazaPostaci;
    }
    // ================================================================
    void czekanie() {

    }

    void picie() {
        
    }   
    void startRzut(float angle)
    {
        std::cout << "[startRzut] angle=" << angle << "\n";

        if (pocisk.leci) return;

        pocisk.start(pozycja, angle, predkosc);
    }

    std::optional<Zdarzenie> updateRzut(float dt, float celX)
    {
        pocisk.update(dt);
        
        if (pocisk.wyladowal())
        {
            pocisk.leci = false;

            if (std::abs(pocisk.x - celX) < 0.5f)
                return Zdarzenie::trafiłeś;
        }

        return std::nullopt;
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
    Cel cel;
    Postac Gracz1;
    Postac Gracz2;

    GameLoop(float pozcycja1, float pozycja2, float pozycjaCelu)
        : Gracz1(pozcycja1),
          Gracz2(pozycja2),
          cel(pozycjaCelu)
    {
    }

    void obsluzPostac1(
        float dt = 0, 
        float move = 0, 
        float drink = 0, 
        float pick = 0,
        float kąt = 0
    )

    {
        switch (Gracz1.fazaPostaci)
        {
        case Faza::czekanie:   Gracz1.czekanie(); break;
        case Faza::picie:      Gracz1.picie(); break;
        case Faza::celowanie:  Gracz1.startRzut(kąt); break;
        case Faza::bieg:       Gracz1.bieg(); break;
        case Faza::powrot:     Gracz1.powrot(); break;
        }
    }

    void obsluzPostac2(
        float dt = 0, 
        float move = 0, 
        float drink = 0, 
        float pick = 0,
        float kąt = 0
    )
    {
        switch (Gracz2.fazaPostaci)
        {
        case Faza::czekanie:   Gracz2.czekanie(); break;
        case Faza::picie:      Gracz2.picie(); break;
        case Faza::celowanie:  Gracz2.startRzut(kąt); break;
        case Faza::bieg:       Gracz2.bieg(); break;
        case Faza::powrot:     Gracz2.powrot(); break;
        }
    }

    void start(){
        Gracz1.fazaPostaci = Faza::celowanie;
        Gracz2.fazaPostaci = Faza::czekanie;
    }

    void update(float dt)
    {
        // 1. update lotu (jeśli leci)
        if (auto zd = Gracz1.updateRzut(dt, cel.zwrocPolozenie()))
        {
            Gracz1.zmienStan(*zd);
        }

        // 2. zachowanie wg fazy
        obsluzPostac1(dt);
    }
    
};

