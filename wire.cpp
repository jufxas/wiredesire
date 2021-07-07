#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include "objectSpriteRectList.hpp"

int width = 1200, height = 1200, frameRate = 30;
float frameNumber = 0.f;
typedef short int it_;
struct numTime { int num; float time; }; 
struct RGB { float R, G, B; }; 
struct XY { float X; float Y; }; 
class Timer {
public:
    std::vector<numTime> timeCheck; 
    float getTime() {
        return frameNumber/frameRate; 
    }
    void timeStart(int n) {
        for (numTime &t : timeCheck) {
            if (t.num == n) {
                std::cout << "Counter :" << n << ": taken" << std::endl; 
                return; 
            }   
        }
        timeCheck.push_back({n, getTime()}); 
    }
    float getInBetweenTime(int n) { // ! havent tested it yet  
        for (numTime &t : timeCheck) {
            if (t.num == n) {
                return getTime() - t.time; 
            }
        }
        return -1.f; 
    }
    float timeEnd(int n) {
        it_ c = 0; 
        for (numTime &t : timeCheck) {
            if (t.num == n) {
                std::cout << "Counter :" << n << ": is up, time (s) is " << getTime() - t.time << std::endl; 
                timeCheck.erase(timeCheck.begin() + c); 
                return getTime() - t.time; 
            }
            c++; 
        }
        return -1.f; 
    }
};

sf::Font font; // * has to be global because sfml documentation said so
sf::Texture objectSheet; // * idk if this has to be global but it'll just be global 

enum GAME_STATES {
    // init via "GAME_STATES G = title; "
    title, 
    tutorial, 
    menu, 
    game, 
    debug
};


template <typename T> 
struct EntityandName { // * need an identifier so it can be deleted      look into hash maps o_o 
    std::string entityName;
    T entity; 
}; 

struct rectangleForGrid {
    sf::RectangleShape rect; 
    sf::Sprite sprite; 
};

class Grid {
public: 
    std::vector<rectangleForGrid> gridRectangleHolder; 
    int gridRows, gridColumns; 
    float eachRectWidth, eachRectHeight; 
    XY drawStartingPoint; 
    std::vector<int> gridMatrix; 

    void describeGrid(int gridRows, int gridColumns, float eachRectWidth, float eachRectHeight, XY drawStartingPoint, std::vector<int> gridMatrix)  {
        // ? not sure if need to do this for the rest of the variables,, probably
        Grid::eachRectWidth = eachRectWidth; 
        Grid::eachRectHeight = eachRectHeight; 
        Grid::gridRows = gridRows; 
        Grid::gridColumns = gridColumns; 
        Grid::gridMatrix = gridMatrix; 

        float xPos = drawStartingPoint.X; 
        float yPos = drawStartingPoint.Y; 
        sf::RectangleShape rect; 
        sf::Sprite sprite; 
        sprite.setTexture(objectSheet); 
        rect.setFillColor(sf::Color(255, 255, 255)); 
        rect.setOutlineColor(sf::Color(0, 0, 0)); 
        rect.setSize(sf::Vector2f(eachRectWidth, eachRectHeight)); 
        rect.setOutlineThickness(1);

        for (int i = 0; i < gridRows * gridColumns; i++) {
             // ! Put in sprite position and scaling  (given w / local bound w) 
             sprite.setTextureRect(  objPointHolder[ gridMatrix[i] ].rect ); 

            rect.setPosition(sf::Vector2f(xPos, yPos)); 
            sprite.setPosition(sf::Vector2f(xPos, yPos)); 
            sprite.setScale(sf::Vector2f(
                eachRectWidth / sprite.getLocalBounds().width, 
                eachRectHeight / sprite.getLocalBounds().height
            ));

            gridRectangleHolder.push_back({rect, sprite}); 

            xPos += eachRectWidth; 
            if ( ((i + 1) % gridRows) == 0) {
                xPos = drawStartingPoint.X; 
                yPos += eachRectHeight; 
            }
        }
    }

    void changeIntRectOfSprite (int n, sf::IntRect intRectChange) {
        gridRectangleHolder[n].sprite.setTextureRect(intRectChange);
        gridRectangleHolder[n].sprite.setScale(sf::Vector2f(
            eachRectWidth / gridRectangleHolder[n].sprite.getLocalBounds().width, 
            eachRectHeight / gridRectangleHolder[n].sprite.getLocalBounds().height
        ));
        // ! i < 18 because that's the size of objpointholder. if objpointholder changes pls update this value. 
        for (int i = 0; i < 18; i++) {
            if (objPointHolder[i].rect == intRectChange) {
                gridMatrix[n] = i; 
                return; 
            }
        }
    }
    void rotateSprite (int n, float degrees) {
        gridRectangleHolder[n].sprite.setOrigin(
                gridRectangleHolder[n].sprite.getLocalBounds().width / 2, 
            gridRectangleHolder[n].sprite.getLocalBounds().height / 2
            );

            gridRectangleHolder[n].sprite.setRotation(degrees); 
            gridRectangleHolder[n].sprite.move(sf::Vector2f(eachRectWidth / 2, eachRectHeight / 2)); // ! LIKELY SOURCE OF FUTURE BUG 
    }

    void fixWires() {
        for (int i = 0; i < gridRectangleHolder.size(); i++) {
            bool up = true, down = true , left = true , right = true; // * identifiers for if there's a block up, down, left, or right
            if (gridMatrix[i] == 1) { // 1 = wire 4 way
                if (i <= (gridRows - 1)) {
                    // * case making it so up is false 
                    // * it's at the roof 
                    up = false; 
                } else if (i >= (gridRows * (gridColumns - 1))) {
                    // * it's at the bottom 
                    // * down is false 
                    down = false;
                }
                // * 42 to 48 = bottom is false 
                // * if it's
                // * (0 or 6), (7 or 13), (14 or 20), (21 or 27), (28 or 34), (35 or 41), (42 to 48) then left and right = false  
                if (i % gridRows == 0) {
                    // * it's at the left 
                    // * left is false 
                    left = false; 
                }
                else if ( (i+1) % (gridRows) == 0) { // ! may be source of a bug idk yet 
                    // * it's at the right
                    // * right is false 
                    right = false; 
                }
                

                // * making sure it's not at the top or bottom so seg faults are prevented 
                if (up) {
                    if (gridMatrix[i - gridRows] == 16) {
                        // * 16 means it's empty 
                        up = false; 
                    }
                }
                if (down) {
                    if (gridMatrix[i + gridRows] == 16) {
                        down = false;
                    }
                }

                if (left) {
                    if (gridMatrix[i - 1] == 16) {
                        left = false; 
                    }
                }
                if (right) {
                    if (gridMatrix[i + 1] == 16) {
                        right = false; 
                       
                    }
                }

                int howmany= 0; 

                if (up) howmany++; 
                if (down) howmany++; 
                if (left) howmany++; 
                if (right) howmany++; 

                std::cout << i << " UP: " << up << " DOWN: " << down << " LEFT: " << left << " RIGHT: " << right << " H: " << howmany << std::endl; 

                if (up || down) {
                    if (!left && !right) {
                        changeIntRectOfSprite(i, vertical_wire_2_way); 
                    }
                } 
                if (left || right) {
                    if (!up && !down) changeIntRectOfSprite(i, horizontal_wire_2_way); 
                } 
                
                 if (left && up && howmany == 2) {
                    changeIntRectOfSprite(i, bent_wire_2_way);
                    rotateSprite(i, 90); 
                } 
                else if (right && down && howmany == 2) {
                    changeIntRectOfSprite(i, bent_wire_2_way);
                    rotateSprite(i, 270); 
                } else if (left && down && howmany == 2) {
                    changeIntRectOfSprite(i, bent_wire_2_way); 
                    // rotateSprite(i, 270); 
                } else if (right && up && howmany == 2) {
                    rotateSprite(i, 180); 
                    changeIntRectOfSprite(i, bent_wire_2_way); 
                }
                
                if (howmany == 3) {
                    changeIntRectOfSprite(i, wire_3_way); 
                    if (down && right && up) {
                        rotateSprite(i, 90);
                    } else if (right && up && left) {
                        rotateSprite(i, 180);
                    } else if (up && right && down) {
                        rotateSprite(i, 270); 
                    } else if (up && left && down) {
                        rotateSprite(i, 90);
                    }

                }

            }
        }
    }
    
};

class Game {
private: 
    sf::RenderWindow* window; 
    sf::Event event; 
    sf::Text text;
    Timer timer;
    Grid grid; 

    bool mouseHeld = false;
    sf::Vector2i mousePosWindow; /// vector2i = 2 integers
    sf::Vector2f mousePosView;

    std::vector<EntityandName<sf::Text>> textHolder;
    std::vector<EntityandName<sf::RectangleShape>> rectangleHolder; 
    std::vector<EntityandName<sf::Sprite>> spriteHolder; // * moe like tool/object holder in the context of the game 
    GAME_STATES GAME_STATE = debug; 
public:
    Game() {
        window = nullptr; 
        window = new sf::RenderWindow(sf::VideoMode(width, height), "Wire Desire", sf::Style::Close | sf::Style::Titlebar); 
        window->setFramerateLimit(frameRate);
        if (!objectSheet.loadFromFile("./graphics/modv2ObjSheetinUse.png")) { // ! modified version
            std::cout << "Error loading object sheet png" << std::endl; 
        } 

        // * init objPointerHolder
        // ? Might get rid of name later since we now have number to intrect equivalencies 
        objPointHolder[0] = {horizontal_wire_2_way, "horizontal_wire_2_way"};
        objPointHolder[1] = {wire_4_way, "wire_4_way"};
        objPointHolder[2] = {bent_wire_2_way, "bent_wire_2_way"};
        objPointHolder[3] = {vertical_wire_2_way , "vertical_wire_2_way"};
        objPointHolder[4] = { power_extender, "power_extender"};
        objPointHolder[5] = { power_block , "power_block"}; 
        objPointHolder[6] = {power_detector, "power_detector"}; 
        objPointHolder[7] = {power_dispenser, "power_dispenser"}; 
        objPointHolder[8] = {neutral_winning_block, "neutral_winning_block"}; 
        objPointHolder[9] = {activated_winning_block, "activated_winning_block"}; 
        objPointHolder[10] = {lever_on, "lever_on"}; 
        objPointHolder[11] = {lever_off, "lever_off"}; 
        objPointHolder[12] = {normal_block_pusher_unpowered, "normal_block_pusher_unpowered"}; 
        objPointHolder[13] = {normal_block_pusher_powered, "normal_block_pusher_powered"}; 
        objPointHolder[14] = {sticky_block_pusher_unpowered, "sticky_block_pusher_unpowered"}; 
        objPointHolder[15] = {sticky_block_pusher_powered, "sticky_block_pusher_powered"}; 
        objPointHolder[16] = {empty, "empty"}; 
        objPointHolder[17] = {wire_3_way, "wire_3_way"}; 
        // * init objPointerHolder
    }
    bool isRunning() {
        return window->isOpen(); 
    }
    void pollEvents() {
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) window->close(); 
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) window->close(); 
            }
        }
    }
    // TODO: have a get entity function to easily delete and modify stuff in vectors. Issue: there will be separate vectors 
    void initObjects() { // * ran once at start and at game state changes 
        if (GAME_STATE == title) {
            makeText("wire_text", "./graphics/fonts/Orbitron/Orbitron-VariableFont_wght.ttf", 300, {0, 0, 0}, "Wire", 60, 300);
            makeText("desire_text", "./graphics/fonts/Orbitron/Orbitron-VariableFont_wght.ttf", 300, {0, 0, 0}, "Desire", 60 + 30 , 300 + 255);
            makeRect("outline_rect", 25, 25, 1150, 1150, {255, 255, 255}, {0, 0, 0}, 2);
            makeRect("outline_rect2", 50, 50, 1100, 1100, {255, 255, 255}, {0, 0, 0}, 2);
            makeRect("outline_rect3", 75, 75, 1050, 1050, {255, 255, 255}, {0, 0, 0}, 2);

            makeRect("play_button",  300, 800, 600, 300, {0, 0, 0}, {0, 0, 0}, 0); 
            makeText("play_text", "./graphics/fonts/Orbitron/Orbitron-VariableFont_wght.ttf", 200, {255, 255, 255}, "Play", 350, 800 ); 
        } else if (GAME_STATE == tutorial) {
            makeRect("go_back_rect", 100, 100, 300, 100, {0, 0, 0}, {0, 0, 0}, 0); 
            makeText("go_back_text", "./graphics/fonts/Orbitron/Orbitron-VariableFont_wght.ttf", 50, {255, 255, 255}, "Go Back", 100, 100); 
        } else if (GAME_STATE == debug) { // ! debug game state should not appear as one of the 1st few to check for so ... fix it lol !

            // ! HAS to align with rows x columns put into grid.describegrid(...)
            std::vector<int> gridMatrix = {
                01, 01, 01, 01, 16, 16, 01,
                16, 16, 01, 01, 04, 01, 01,
                16, 16, 16, 16, 16, 16, 16,
                16, 16, 16, 01, 16, 16, 16,
                16, 01, 01, 01, 01, 01, 16,
                16, 16, 16, 01, 16, 01, 16,
                01, 01, 01, 01, 16, 16, 01,
            };

            grid.describeGrid(7, 7, 150, 150, {80, 80}, gridMatrix); 
            grid.changeIntRectOfSprite(20, power_dispenser);
            grid.rotateSprite(20, 180); 
            // * MANUALLY FOUND OUT U HAD TO MOVE "sticky_block_pusher_powered" BY 75, 75 TO GET IT BACK IN POSITION
            // * 75,75 MIGHT BE RECTWIDTH/2, RECTHEIGHT/2  
            // * MAY NOT BE A BAD IDEA TO DO IT LIKE THAT DOE 
            // * SOMEHOW IT WORKS PRETTY WELL FOR ALL THE OBJECTS THAT'D BE ROTATED 
            // * SO U HAVE TO SET ORIGIN TO LOCAL BOUNDS /2, ROTATE, THEN MOVE BY 75x75 

            grid.fixWires(); 

        }
    }
    // * very not dry 😪
    sf::RectangleShape getRectByName(std::string name) {
        for (EntityandName<sf::RectangleShape> &t : rectangleHolder) 
            if (t.entityName == name) return t.entity; 
        
        static bool coutErr = false; 
        if (!coutErr) 
            std::cout << "COULDN'T FIND RECTANGLE" << std::endl; 
        coutErr = true; 
        sf::RectangleShape filler; 
        return filler; 
    }

    void updateObjects() {
        // updating mouse pos
        mousePosWindow = sf::Mouse::getPosition(*window);
        mousePosView = window->mapPixelToCoords(mousePosWindow);

        // insert game logic and key presses and all that  
        if (GAME_STATE == title) {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                if (!mouseHeld) {

                    if ( getRectByName("play_button").getGlobalBounds().contains(mousePosView) ) {
                        GAME_STATE = tutorial; 
                        emptyAllVecsAndInitObjs(); 
                    }
                }

            } else if (mouseHeld) {
                mouseHeld = false; 
            }
            // * Note : % only for ints 
            
        } else if (GAME_STATE == tutorial) {

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                if (!mouseHeld) {

                    if ( getRectByName("go_back_rect").getGlobalBounds().contains(mousePosView) ) {
                        GAME_STATE = title; 
                        emptyAllVecsAndInitObjs(); 
                    }
                }

            } else if (mouseHeld) {
                mouseHeld = false; 
            }

        } 
    }
    void makeText(std::string textName, std::string fontDir, float charSize, RGB fillColor, std::string textPrinted, float x, float y) {
        sf::Text text;
        font.loadFromFile("./graphics/fonts/Orbitron/Orbitron-VariableFont_wght.ttf");
        text.setFont(font);
        text.setCharacterSize(charSize);
        text.setFillColor(sf::Color(fillColor.R, fillColor.G, fillColor.B));
        text.setString(textPrinted);
        text.setPosition(sf::Vector2f(x, y));
        textHolder.push_back({textName, text});        
    }
    void makeRect(std::string name, float x, float y, float w, float h, RGB fillColor, RGB outlineColor, float thickness) {    
        sf::RectangleShape rect; 
        rect.setPosition(sf::Vector2f(x, y)); 
        rect.setSize(sf::Vector2f(w, h)); 
        rect.setFillColor(sf::Color(fillColor.R, fillColor.G, fillColor.B)); 
        rect.setOutlineColor(sf::Color(outlineColor.R, outlineColor.G, outlineColor.B)); 
        rect.setOutlineThickness(thickness); 
        rectangleHolder.push_back({name, rect}); 
    }

    void makeStandAloneObjectSprite(std::string typeofObject,  std::string name, float x, float y, float w, float h) {
        // * "stand alone" specificed since creation of sprites would be used for the grid
        sf::Sprite sprite; 
        sprite.setTexture(objectSheet); 
        sprite.setPosition(sf::Vector2f(x, y)); 

        
        // TODO: 
        // * make this dry like have a map of all this info then just iterate (DONE)

        for (intRectAndName& t : objPointHolder) {
            if (typeofObject == t.name) {
                sprite.setTextureRect(t.rect);
                // std::cout << sprite.getLocalBounds().width << " " << sprite.getLocalBounds().height << std::endl; 
                sprite.setScale(sf::Vector2f(
                    w / sprite.getLocalBounds().width, 
                    h / sprite.getLocalBounds().height
                ));
            }
        }
        spriteHolder.push_back({name, sprite}); 
    }

    void render() {
        window->clear(sf::Color::White); 
        // then window->draw(whatever)
        for (EntityandName<sf::RectangleShape> &t : rectangleHolder) {
            window->draw(t.entity); 
        }
        for (EntityandName<sf::Sprite> &t : spriteHolder) {
            window->draw(t.entity); 

        }

        for (rectangleForGrid &t : grid.gridRectangleHolder) {
            window->draw(t.rect); 
            window->draw(t.sprite); 
        }  

        // then render text 
        for (EntityandName<sf::Text> &t : textHolder) {
            window->draw(t.entity); 
        }

        // now display 
        window->display(); 
        frameNumber++; 
    }
    void emptyAllVecsAndInitObjs() {
        textHolder.clear(); 
        textHolder.shrink_to_fit(); 
        rectangleHolder.clear(); 
        rectangleHolder.shrink_to_fit(); 
        spriteHolder.clear(); 
        spriteHolder.shrink_to_fit(); 
        grid.gridRectangleHolder.clear(); 
        grid.gridRectangleHolder.shrink_to_fit(); 
        initObjects(); 

    }
    void onGameEnd() {
        delete window; 
    }
};

int main() {
    Game game;
    game.initObjects();

    while (game.isRunning()) {
        game.pollEvents();
        game.updateObjects(); // * also handling user events 
        game.render(); 
    }
    game.onGameEnd(); 
    
    return 0; 
}