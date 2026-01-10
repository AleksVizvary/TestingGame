#include <SFML/Graphics.hpp>
#include "globals.hpp"
#include <iostream>
#pragma once
#include <cmath>
// tu wszystkie funkcje i klasy
static void updateViewViewport(const sf::RenderWindow& window, sf::View& view) {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    float targetW = mainWin.x;
    float targetH = mainWin.y;

    if (winW <= 0.f || winH <= 0.f) return;

    float scale = std::min(winW / targetW, winH / targetH);
    float viewportW = (targetW * scale) / winW;
    float viewportH = (targetH * scale) / winH;
    float viewportLeft = (1.f - viewportW) / 2.f;
    float viewportTop = (1.f - viewportH) / 2.f;

    view.setViewport(sf::FloatRect(sf::Vector2(viewportLeft, viewportTop), sf::Vector2(viewportW, viewportH)));
}

struct charLook {
    int hatID;
    int hairID;
    int faceID;
    sf::Color topColor;
    sf::Color pantsColor;
    sf::Color shoeColor;
    sf::Color hairColor;
};

class charSprite {
public:
    charSprite(const sf::Vector2f& position, charLook charLook = { 0, 0, 0, sf::Color(0, 0, 0), sf::Color(0, 0, 0), sf::Color(0, 0, 0), sf::Color(0, 0, 0) })
    : shoes(sf::Vector2f({ 157.0, 354.0 })),
      pants(sf::Vector2f({ 157.0, 354.0 })),
      shirt(sf::Vector2f({ 157.0, 354.0 })),
      fingers(sf::Vector2f({ 157.0, 354.0 })),
      handBG(sf::Vector2f({ 157.0, 354.0 })),
      handFG(sf::Vector2f({ 157.0, 354.0 })),
      head(sf::Vector2f({ 157.0, 354.0 })),
      hair(sf::Vector2f({ 157.0, 354.0 })),
      face(sf::Vector2f({ 157.0, 354.0 })),
      hat(sf::Vector2f({ 157.0, 354.0 })) {

        if (charLook.hairID > 0) {
            hasHair = true;
            hairTxt.loadFromFile(std::format("../assets/img/sprites/hair/hair{}.png", charLook.hairID));
            hair.setTexture(&hairTxt);
            hair.setFillColor(charLook.hairColor);
            hair.setPosition(position);
        }

        if (charLook.faceID > 0) {
            hasFace = true;
            faceTxt.loadFromFile(std::format("../assets/img/sprites/face/face{}.png", charLook.faceID));
            face.setTexture(&faceTxt);
            face.setPosition(position);
        }

        if (charLook.hatID > 0) {
            hasHat = true;
            hatTxt.loadFromFile(std::format("../assets/img/sprites/hat/hat{}.png", charLook.hatID));
            hat.setTexture(&hatTxt);
            hat.setPosition(position);
        }

        shoesTxt.loadFromFile("../assets/img/sprites/shoes.png");
        shoes.setTexture(&shoesTxt);
        shoes.setFillColor(charLook.shoeColor);
        shoes.setPosition(position);

        pantsTxt.loadFromFile("../assets/img/sprites/pants.png");
        pants.setTexture(&pantsTxt);
        pants.setFillColor(charLook.pantsColor);
        pants.setPosition(position);

        shirtTxt.loadFromFile("../assets/img/sprites/shirt.png");
        shirt.setTexture(&shirtTxt);
        shirt.setFillColor(charLook.topColor);
        shirt.setPosition(position);

        fingersTxt.loadFromFile("../assets/img/sprites/fingers.png");
        fingers.setTexture(&fingersTxt);
        fingers.setPosition(position);

        handBGTxt.loadFromFile("../assets/img/sprites/handbg.png");
        handBG.setTexture(&handBGTxt);
        handBG.setPosition(position);

        handFGTxt.loadFromFile("../assets/img/sprites/handfg.png");
        handFG.setTexture(&handFGTxt);
        handFG.setPosition(position);

        headTxt.loadFromFile("../assets/img/sprites/head.png");
        head.setTexture(&headTxt);
        head.setPosition(position);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shoes);
        window.draw(pants);
        window.draw(handBG);
        window.draw(shirt);
        window.draw(fingers);
        window.draw(handFG);
        window.draw(head);
        if (hasFace) window.draw(face);
        if (hasHair) window.draw(hair);
        if (hasHat) window.draw(hat);

    }
    void setPos(float x, float y) {
        shoes.setPosition(sf::Vector2f(x, y));
        pants.setPosition(sf::Vector2f(x, y));
        shirt.setPosition(sf::Vector2f(x, y));
        fingers.setPosition(sf::Vector2f(x, y));
        handBG.setPosition(sf::Vector2f(x, y));
        handFG.setPosition(sf::Vector2f(x, y));
        head.setPosition(sf::Vector2f(x, y));
        hair.setPosition(sf::Vector2f(x, y));
        face.setPosition(sf::Vector2f(x, y));
        hat.setPosition(sf::Vector2f(x, y));
    }
    // dir: -1 - facing left, 1 - facing right
    void flip(float dir = -1) {
        sf::Vector2f direction = { dir, 1.f };
        shoes.setScale(direction);
        pants.setScale(direction);
        shirt.setScale(direction);
        fingers.setScale(direction);
        handBG.setScale(direction);
        handFG.setScale(direction);
        head.setScale(direction);
        hair.setScale(direction);
        face.setScale(direction);
        hat.setScale(direction);
    }

    void PositionAtFeet(sf::Sprite& s) 
    {
        auto b = s.getLocalBounds();
        s.setOrigin({ b.size.x / 2.f, b.size.y });
    }

    void faceRight(sf::Sprite& s)
    {
        auto sc = s.getScale();
        s.setScale({ std::abs(sc.x), sc.y });
    }

    void faceLeft(sf::Sprite& s) 
    {
        auto sc = s.getScale();
        s.setScale({ -std::abs(sc.x), sc.y });
    }

private:
    sf::RectangleShape mainRect;
    sf::RectangleShape shoes;
    sf::RectangleShape pants;
    sf::RectangleShape shirt;
    sf::RectangleShape fingers;
    sf::RectangleShape handBG;
    sf::RectangleShape handFG;
    sf::RectangleShape head;
    sf::RectangleShape hair;
    sf::RectangleShape face;
    sf::RectangleShape hat;
    

    sf::Texture pantsTxt;
    sf::Texture shoesTxt;
    sf::Texture shirtTxt;
    sf::Texture fingersTxt;
    sf::Texture handBGTxt;
    sf::Texture handFGTxt;
    sf::Texture headTxt;
    sf::Texture hairTxt;
    sf::Texture faceTxt;
    sf::Texture hatTxt;

    bool hasHair = false;
    bool hasFace = false;
    bool hasHat = false;
};


class Button {
public:
    Button(const sf::Vector2f& size, 
        const sf::Vector2f& position, 
        const sf::Color& color, 
        const sf::Color& hoverColor, 
        const std::string& textString, 
        const sf::Font& font, 
        unsigned int charSize, 
        const std::string& texture = "")
        : shape(size), text(font, textString, charSize), baseColor(color), hoverColor(hoverColor) {

        shape.setPosition(position);
        if (!texture.empty() && b_texture.loadFromFile(texture)) {
            shape.setTexture(&b_texture);
            shape.setFillColor(color);
        }
        else {
            shape.setFillColor(color);
        }
        text.setPosition(sf::Vector2f(position.x + size.x / 2.0f - text.getLocalBounds().size.x / 2.0f, position.y + size.y / 2.0f - text.getLocalBounds().size.y / 2.0f - 5.0f));
    }
    void draw(sf::RenderWindow& window) {
        window.draw(shape);
        window.draw(text);
    }
    bool isMouseOver(const sf::Vector2f& mousePos) {
        return shape.getGlobalBounds().contains(mousePos);
    }
    void hover() {
        shape.setFillColor(hoverColor);
    }
    void unhover() {
        shape.setFillColor(baseColor);
    }
private:
    sf::RectangleShape shape;
    sf::Text text;
    sf::Texture b_texture;
    sf::Color baseColor;
    sf::Color hoverColor;
};




class QTEbar : public sf::Drawable, public sf::Transformable{

private:
    std::vector<sf::RectangleShape> m_rects;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states.transform *= getTransform();
        for (const auto& rect : m_rects) {
            target.draw(rect, states);
        }
    }
public:
    QTEbar(float width, float height, float spacing) {
        const sf::Color colors[] = {
            sf::Color(0,150,0),
            sf::Color(237, 214, 34),
            sf::Color(255,0,0)
        };

        for (int i = 0; i < 3; i++) {
            sf::RectangleShape rect(sf::Vector2f(width, height));
            rect.setFillColor(colors[i]);
            float yPos = (height + spacing) * static_cast<float>(i);
            rect.setPosition(sf::Vector2f(0.f, yPos));
            m_rects.push_back(rect);
        }
    }
};


class greyBar : public sf::Drawable, public sf::Transformable {

private:
    std::vector<sf::RectangleShape> m_rects;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states.transform *= getTransform();
        for (const auto& rect : m_rects) {
            target.draw(rect, states);
        }
    }
public:
    greyBar(float width, float height, float spacing) {
        
        

        //for (int i = 0; i < 3; i++) {
            sf::RectangleShape rect(sf::Vector2f(width, height));
            rect.setFillColor(sf::Color(128, 128, 128));
            float yPos = (height + spacing);
            rect.setPosition(sf::Vector2f(0.f, yPos));
            m_rects.push_back(rect);
        //}
    }
};