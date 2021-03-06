#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "objectSpriteRectList.hpp"

// * 0 = up, 1 = down, 2 = left, 3 = right
namespace Direction_Spaces {
    int up = 0, down = 1, left = 2, right = 3, none = 4;
}; 

int width = 1200, height = 1200, frameRate = 30, electricitySpeed = 5;
bool debugClog = false; 
const int electricityOrbLifeSpan = 3;
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

struct Direction {
    bool up, down, left, right; 
}; 


struct ElectricityOrb {
    sf::CircleShape ElectricityOrb; 
    int direction; // * 0 = up, 1 = down, 2 = left, 3 = right, 4 = none, 5 = auto 
    XY destination;  
    int speed; 
    int destinationCellNumber; 
    int originCellNumber;
    std::vector<int> cellsTravelled; 
};

struct ObjectWithOrientation {
    int objNumber; // * like if it's 6 (power_detector), 7 (power_dispenser), 12 (normal block pusher unp), 14 (sticky block pusher unp) 
    // 13 normal power, 15 sticky power
    int cellNumber; 
    int objIsFacing; // default down (0 rotation) 
};

class Grid {
private: 
    int leverIndex = -1; 
    XY leverPosition = {-1, -1}; // * if correct, it'd be the top left of the cell 
    bool leverState = false; // false = off, true = on 
    int nonEmptyCellCount = 0; 
    std::vector<ObjectWithOrientation> objectsWithOrientation;
    int identifier = -1;  
public: 
    std::vector<rectangleForGrid> gridRectangleHolder; 
    int gridRows, gridColumns; 
    float eachRectWidth, eachRectHeight; 
    XY drawStartingPoint; 
    std::vector<int> gridMatrix; 
    const std::vector<int> listOfObjectsWithOrientation = {6, 7, 12, 13, 14, 15}; 

    void describeGrid(int gridRows, int gridColumns, float eachRectWidth, float eachRectHeight, XY drawStartingPoint, std::vector<int> gridMatrix)  {
        Grid::eachRectWidth = eachRectWidth; 
        Grid::eachRectHeight = eachRectHeight; 
        Grid::gridRows = gridRows; 
        Grid::gridColumns = gridColumns; 
        Grid::gridMatrix = gridMatrix; 
        Grid::drawStartingPoint = drawStartingPoint;

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
            if (gridMatrix[i] == 11) {
                leverIndex = i;  
                leverPosition = {xPos, yPos}; 
            } 
            if (std::count(listOfObjectsWithOrientation.begin(), listOfObjectsWithOrientation.end(), gridMatrix[i])) {
                objectsWithOrientation.push_back({gridMatrix[i], i, Direction_Spaces::down});
            }

            
            if (gridMatrix[i] != 16) nonEmptyCellCount++; 


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

    Direction checkDirection(int index) { // * for preventing seg faults, could be used in fix wires but i dont feel like changing it 
        Direction direction = {true, true, true, true}; 
        if (index <= (gridRows - 1)) direction.up = false; 
        else if (index >= (gridRows * (gridColumns - 1))) direction.down = false;   

        if (index % gridRows == 0) direction.left = false; 
        else if ( (index+1) % (gridRows) == 0) direction.right = false; 
        return direction; 
    }
    Direction checkForNonEmpty(Direction direction, int index) {
        if (direction.up) if (gridMatrix[index - gridRows] == 16 || gridMatrix[index - gridRows] == 11 || gridMatrix[index - gridRows] == 10) direction.up = false; 

        if (direction.down) if (gridMatrix[index + gridRows] == 16 || gridMatrix[index + gridRows] == 11 || gridMatrix[index + gridRows] == 10) direction.down = false;                    

        if (direction.left) if (gridMatrix[index - 1] == 16 || gridMatrix[index - 1] == 11 || gridMatrix[index - 1] == 10) direction.left = false; 
        
        if (direction.right) if (gridMatrix[index + 1] == 16 || gridMatrix[index + 1] == 11 || gridMatrix[index + 1] == 10) direction.right = false;  
        return direction; 

    }

    Direction electricityCheckForNonEmpty(Direction direction, int index) {
        std::vector<int> validObjects = {0, 1, 2, 3, 4, 6, 7, 8, 17}; // wires, detecotr, dispenser 
        std::vector<int> wires = {0, 1, 2, 3, 4, 17};
        if ( !std::count(wires.begin(), wires.end(), gridMatrix[index])) {
            Direction d = {false, false, false, false}; 
            return d; 
        }

        // if (direction.up) if (!std::count(validObjects.begin(), validObjects.end(), gridMatrix[index - gridRows])) direction.up = false;

        // if (direction.down) if (!std::count(validObjects.begin(), validObjects.end(), gridMatrix[index + gridRows])) direction.down = false;

        // if (direction.left) if (!std::count(validObjects.begin(), validObjects.end(), gridMatrix[index - 1])) direction.left = false;

        // if (direction.right) if (!std::count(validObjects.begin(), validObjects.end(), gridMatrix[index + 1])) direction.right = false;
        if (direction.up) if (gridMatrix[index - gridRows] == 16 || gridMatrix[index - gridRows] == 11 || gridMatrix[index - gridRows] == 10) direction.up = false; 

        if (direction.down) if (gridMatrix[index + gridRows] == 16 || gridMatrix[index + gridRows] == 11 || gridMatrix[index + gridRows] == 10) direction.down = false;                    

        if (direction.left) if (gridMatrix[index - 1] == 16 || gridMatrix[index - 1] == 11 || gridMatrix[index - 1] == 10) direction.left = false; 
        
        if (direction.right) if (gridMatrix[index + 1] == 16 || gridMatrix[index + 1] == 11 || gridMatrix[index + 1] == 10) direction.right = false;  

        
        return direction;  
    }

    void fixWires() {
        for (int i = 0; i < gridRectangleHolder.size(); i++) {
            // * How the logic works because my variable names are kinda bad 
            // * Each cell if it has a wire or repeaeter has to look up, down, left, right if there is anything that isn't empty 
            // * We first have to make sure we can actually access it to prevent seg faults 
            // * We also check left and right to make sure we're not at the most left/right so we don't have situations where the previous/next cell fits the critera (of not being empty) and then cell's sprite is rotated as if it was when visually it isn't 
            // * Sprites are rotated/changed accordingly based on what's up, down, left, right of them 

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
                if (down) if (gridMatrix[i + gridRows] == 16) down = false;                    
                if (left) if (gridMatrix[i - 1] == 16) left = false; 
                if (right) if (gridMatrix[i + 1] == 16) right = false; 
                       
                // * Breakoff if (Can assume we don't need to ask anymore questions so the compiler may now carry on to greener pastures like if i == 4) 
                // ! also maybe sources of bugs. man. if bugs, investigate every continue statement in this code 
                if (!up && !down && !left && !right) continue; 
        
                int howmany = 0; 
                if (up) howmany++; if (down) howmany++; if (left) howmany++; if (right) howmany++; 
                // * yes ik bad variable name but all u need to know is that if statements just ask a bit less 

                // std::cout << i << " UP: " << up << " DOWN: " << down << " LEFT: " << left << " RIGHT: " << right << " H: " << howmany << std::endl; 

                if (up || down) {
                    if (!left && !right) {
                        changeIntRectOfSprite(i, vertical_wire_2_way); 
                        continue; 
                    }
                } 
                if (left || right) {
                    if (!up && !down) {
                        changeIntRectOfSprite(i, horizontal_wire_2_way); 
                        continue; 
                    }
                } 
                
                 if (left && up && howmany == 2) {
                    changeIntRectOfSprite(i, bent_wire_2_way);
                    rotateSprite(i, 90); 
                    continue; 
                } 
                else if (right && down && howmany == 2) {
                    changeIntRectOfSprite(i, bent_wire_2_way);
                    rotateSprite(i, 270); 
                    continue; 
                } else if (left && down && howmany == 2) {
                    changeIntRectOfSprite(i, bent_wire_2_way); 
                    // rotateSprite(i, 270); 
                    continue; 
                } else if (right && up && howmany == 2) {
                    rotateSprite(i, 180); 
                    changeIntRectOfSprite(i, bent_wire_2_way); 
                    continue; 
                }
                
                if (howmany == 3) {
                    changeIntRectOfSprite(i, wire_3_way); 
                    if (down && right && up) rotateSprite(i, -90); // ! not sure if these are properly rotated 
                    else if (right && up && left) rotateSprite(i, 180);
                    else if (up && right && down) rotateSprite(i, 270); 
                    else if (up && left && down) rotateSprite(i, 90);
                }

            } else if (gridMatrix[i] == 4) { // power extender is 4 
                bool up = true, down = true, left = true, right = true; 

                // * copy pasted logic to make sure seg faults dont happen 
                if (i <= (gridRows - 1)) up = false; 
                else if (i >= (gridRows * (gridColumns - 1))) down = false;   

                if (i % gridRows == 0) left = false; 
                else if ( (i+1) % (gridRows) == 0) right = false; 
                
                // * more copy paste  
                if (up) if (gridMatrix[i - gridRows] == 16) up = false; 
                if (down) if (gridMatrix[i + gridRows] == 16) down = false;
                if (left) if (gridMatrix[i - 1] == 16) left = false; 
                if (right) if (gridMatrix[i + 1] == 16) right = false; 

                // ? no idea if this is enough logic lol 
                if (up || down) {
                    rotateSprite(i, 90); 
                }

            }
        }
    }
    
    void changeWhatObjectIsFacing(int cellNum, int isFacing) {
        bool correctlyTargetingObjectWithOrientation = false;
        int objIndex = -1; 
        for (int i = 0; i < objectsWithOrientation.size(); i++) {
            if (objectsWithOrientation[i].cellNumber == cellNum) {
                correctlyTargetingObjectWithOrientation = true; 
                objIndex = i;
                break; 
            }
        }
        if (!correctlyTargetingObjectWithOrientation) {
            if (debugClog) std::cout << "didnt happen 0001" << std::endl;
            return; 
        }

        objectsWithOrientation[objIndex].objIsFacing = isFacing;

        if (objectsWithOrientation[objIndex].objIsFacing == Direction_Spaces::up) {
            rotateSprite(objectsWithOrientation[objIndex].cellNumber , 180);
        } else if (objectsWithOrientation[objIndex].objIsFacing == Direction_Spaces::down) {
            rotateSprite(objectsWithOrientation[objIndex].cellNumber , 0);

        } else if (objectsWithOrientation[objIndex].objIsFacing == Direction_Spaces::left) {
            rotateSprite(objectsWithOrientation[objIndex].cellNumber , 90);

        } else if (objectsWithOrientation[objIndex].objIsFacing == Direction_Spaces::right) {
            rotateSprite(objectsWithOrientation[objIndex].cellNumber , -90);

        }
    }

    bool checkIfLeverClick(sf::Vector2f mousePosView) {
        return gridRectangleHolder[leverIndex].rect.getGlobalBounds().contains(mousePosView); 
    }

    // check for mouse click 
    
    void toggleLever() {
        if (!leverState) {
            changeIntRectOfSprite(leverIndex, lever_on);
            leverState = true; 
        } else if (leverState) {
            changeIntRectOfSprite(leverIndex, lever_off);
            leverState = !true; 
        }
    }

    bool isLeverOn() {
        return leverState; 
    }

    void checkCellForImportantObjects(int index, int directionAllowed, ElectricityOrb &eOrb, std::vector<ElectricityOrb>& Electricity) {
        if (debugClog) {
        std::cout << "****START CHECKFORIMPOR******\n"; 
        std::cout << "index: " << index << " direction: " << directionAllowed << std::endl;
        std::cout << "ORB INFO" << std::endl; 
        std::cout << "CELLS TRAVELED SIZE: " << eOrb.cellsTravelled.size() <<" \nDESTINATION: " << eOrb.destination.X << " " << eOrb.destination.Y << " DIRECTION: " << eOrb.direction << "\n DEST, ORG " << eOrb.destinationCellNumber << " " << eOrb.originCellNumber << std::endl;
        std::cout << "GRID MATRIX= " << gridMatrix[index] << std::endl;
        
        for (int i = 0; i < Electricity.size(); i++) {
            std::cout << "ELECTRICITY FOR LOOP"; 
            std::cout << "ORG, DEST, DIRE " << Electricity[i].originCellNumber << " " << Electricity[i].destinationCellNumber << " " << Electricity[i].direction << std::endl;
        }
        
        std::cout << "****END CHECKFORIMPOR******\n";
        } 

        if (eOrb.speed != electricitySpeed) eOrb.speed = electricitySpeed;

        int specialIndex = -1; 

        // * general checks on current cell  
        if (gridMatrix[index] == 8) { // neutral winning block
            changeIntRectOfSprite(index, activated_winning_block); 
        } else if (gridMatrix[index] == 4) { // power extender
            eOrb.cellsTravelled.clear(); 

            eOrb.speed = electricitySpeed / 1.5; 
        } else if (gridMatrix[index] == 7) { // power dispenser 
            int objIndex = -1;
            for (int i = 0; i < objectsWithOrientation.size(); i++) {
                if (objectsWithOrientation[i].cellNumber == index) {
                    objIndex = i;
                    break;
                }
            }

            // default circ 
            sf::CircleShape circ; 
            circ.setPosition(sf::Vector2f(eOrb.ElectricityOrb.getPosition().x , eOrb.ElectricityOrb.getPosition().y));
            circ.setFillColor(sf::Color(rand()%255, rand()%255, 255)); 
            circ.setRadius(eOrb.ElectricityOrb.getRadius());
            
            if (debugClog) std::cout << "CELLS TRAVELLED\n";
             std::vector<int> cellsTravelled = {identifier};
              cellsTravelled.reserve(electricityOrbLifeSpan);
            identifier--; 
            if (debugClog) std::cout << "CELLS TRAVELLED 0TH = " << identifier++ << std::endl;

            if (objectsWithOrientation[objIndex].objIsFacing == Direction_Spaces::up) {
                XY destination = {eOrb.ElectricityOrb.getPosition().x, eOrb.ElectricityOrb.getPosition().y - height}; 
                it_ iterator = 0, destinationCellNumber = -1; 
                for (int i = index; i < gridMatrix.size(); i-= gridRows) {
                    if (gridMatrix[i] != 16 && i != index) {
                        destination.Y = eOrb.ElectricityOrb.getPosition().y - (eachRectHeight * iterator); 
                        destinationCellNumber = i; 
                        break; 
                    }
                    if (i <= (gridRows - 1)) break; // * seg fault preventer 

                    iterator++; 
                }
                if (debugClog) {
                 std::cout << "****START IF OBJSWITHORIENTATION[OBJ******\n"; 
                 std::cout << "DEST, INDEX :" << destinationCellNumber << " " << index << std::endl; 
                 std::cout << "****END IF OBJSWITHORIENTATION[OBJ******\n"; 
                }
                Electricity.push_back({circ, Direction_Spaces::up, destination, electricitySpeed * 2, destinationCellNumber , index, cellsTravelled});

            } else if (objectsWithOrientation[objIndex].objIsFacing == Direction_Spaces::down) {
                XY destination = {eOrb.ElectricityOrb.getPosition().x, eOrb.ElectricityOrb.getPosition().y + height};

                it_ iterator = 0, destinationCellNumber = -1; 
                for (int i = index; i < gridMatrix.size(); i+= gridRows) {
                    if (gridMatrix[i] != 16 && i != index) {
                        destination.Y = eOrb.ElectricityOrb.getPosition().y + (eachRectHeight * iterator); 
                        destinationCellNumber = i;
                        break; 
                    }
                    if (i >= (gridRows * (gridColumns - 1))) break; 

                    iterator++; 
                }
                if (debugClog)
                std::cout << "DESTINATION: " << destination.X << " " << destination.Y << std::endl;
                
                Electricity.push_back({circ, Direction_Spaces::down, destination, electricitySpeed * 2, destinationCellNumber, index, cellsTravelled});
            } else if (objectsWithOrientation[objIndex].objIsFacing == Direction_Spaces::left) {
                XY destination = {eOrb.ElectricityOrb.getPosition().x - width, eOrb.ElectricityOrb.getPosition().y};
                it_ iterator = 0, destinationCellNumber = -1; 
                for (int i = index; i < gridMatrix.size(); i--) {
                    if (gridMatrix[i] != 16 &&  i != index) {
                        destination.X = eOrb.ElectricityOrb.getPosition().x - (eachRectWidth * (iterator));
                        destinationCellNumber = i; 
                        break; 
                    }

                    if (i % gridRows == 0) break; 

                    iterator++; 
                }

                Electricity.push_back({circ, Direction_Spaces::left, destination, electricitySpeed * 2, destinationCellNumber , index, cellsTravelled});
            } else if (objectsWithOrientation[objIndex].objIsFacing == Direction_Spaces::right) { 
                XY destination = {eOrb.ElectricityOrb.getPosition().x + width, eOrb.ElectricityOrb.getPosition().y};
                it_ iterator = 0, destinationCellNumber = -1; 
                for (int i = index; i < gridMatrix.size(); i++) {
                    
                    if (gridMatrix[i] != 16 &&  i != index) {
                        destination.X = eOrb.ElectricityOrb.getPosition().x + (eachRectWidth * (iterator));
                        destinationCellNumber = i; 
                        break; 
                    }

                    if ( (i+1) % (gridRows) == 0) break;

                    iterator++; 

                }

                Electricity.push_back({circ, Direction_Spaces::right, destination, electricitySpeed * 2, destinationCellNumber , index, cellsTravelled});
            }

        }   

        if (debugClog) std::cout << "DIRECTIONA LLOWED START" << std::endl;
        if (directionAllowed == Direction_Spaces::up) {
            specialIndex = index - gridRows; 

        } else if (directionAllowed == Direction_Spaces::down) {
            specialIndex = index + gridRows;

        } else if (directionAllowed == Direction_Spaces::left) { 
            specialIndex = index - 1;

        } else if (directionAllowed == Direction_Spaces::right) { 
            specialIndex = index + 1;
        }
        if (debugClog) std::cout << "DIRECTIONA LLOWED END" << std::endl;
    }

    void sendElectricityFromLever(std::vector<ElectricityOrb> &electricity, int eSpeed, float radius) { // * rework this to just be a general one 
        sf::CircleShape circ; 
        std::vector<int> cellsTravelled; 
         cellsTravelled.reserve(electricityOrbLifeSpan);
        circ.setPosition(sf::Vector2f(leverPosition.X + eachRectWidth / 2, leverPosition.Y + eachRectHeight / 2)); // adding eachrect... makes it go to the middle of the cell 
        circ.setFillColor(sf::Color(255, 255, 0)); 
        circ.setRadius(radius); 

        Direction direction = checkForNonEmpty(checkDirection(leverIndex), leverIndex); // tells us where to send the electricity 

        if (direction.up) {
            electricity.push_back({circ, Direction_Spaces::up, {circ.getPosition().x , circ.getPosition().y - eachRectHeight} , eSpeed, leverIndex - gridRows, leverIndex, cellsTravelled});
        }

        if (direction.down) {
            electricity.push_back({circ, Direction_Spaces::down, {circ.getPosition().x , circ.getPosition().y + eachRectHeight} , eSpeed, leverIndex + gridRows, leverIndex, cellsTravelled});
        }

        if (direction.left) {
            electricity.push_back({circ, Direction_Spaces::left, {circ.getPosition().x - eachRectWidth , circ.getPosition().y} , eSpeed, leverIndex - 1, leverIndex, cellsTravelled});
        }

        if (direction.right) {
            electricity.push_back({circ, Direction_Spaces::right, {circ.getPosition().x + eachRectWidth, circ.getPosition().y} , eSpeed, leverIndex + 1, leverIndex, cellsTravelled});
        }

    }

    void receiveElectricity(ElectricityOrb& eOrb, std::vector<ElectricityOrb>& Electricity, it_ eOrbIndex) {
        if (debugClog) {
        std::cout << "****START RECEIVEELECTRIICTY******\n"; 
        std::cout << "CELLS TRAVELED SIZE: " << eOrb.cellsTravelled.size() << " \nDESTINATION: " << eOrb.destination.X << " " << eOrb.destination.Y << " DIRECTION: " << eOrb.direction << "\n DEST, ORG" << eOrb.destinationCellNumber << " " << eOrb.originCellNumber << " ORBINDEX: " << eOrbIndex << std::endl;

        // * LIMTER B : limits # of orbs by how many objects there are 
        std::cout << "got " << eOrb.destinationCellNumber <<std::endl; 
        }
         bool removeOrb = false,  makeNewOrb = true; ;

        // handle neg index electricity ("rogue, fast electricity")
        if (eOrb.cellsTravelled.size() > 0) {
            // ! CODE AT BAD STATE, GETTING LLVM ERROR: out of memory ERRORS 
            if (debugClog) {
            std::cout << "CELLS TRAVELLED IS GREATER THAN 0 AND 0TH ELEMENT = ";
            std::cout << eOrb.cellsTravelled[0] << std::endl;
            }
            if (eOrb.cellsTravelled[0] < 0) {
                if (debugClog) std::cout << "EORB CELLSTRAVLLED LESS THAN 0" << std::endl;
                checkCellForImportantObjects(eOrb.destinationCellNumber, Direction_Spaces::none, eOrb, Electricity);
                if (debugClog) std::cout << "DONE CHECKING FOR IMPORTANT OBJECTS" << std::endl;

                for (int i = 0; i < Electricity.size(); i++) {
                    if (Electricity[i].cellsTravelled.size() > 0) {
                        if (Electricity[i].cellsTravelled[0] == eOrb.cellsTravelled[0]) {
                            if (debugClog) std::cout << "IDENTIFIED " << i << " TO BE DELETED" << std::endl;
                            Electricity.erase(Electricity.begin() + i); 
                            return; // ! NOT SURE IF RETURN STATEMENT SHOULD BE USED HERE 
                        }

                    }
                }
            }
        }
        
        if (debugClog) std::cout << "E SIZE\n"; 
        if (Electricity.size() >= nonEmptyCellCount) makeNewOrb = false; 
        if (debugClog) {
       std::cout << "EORB CELLSTRAVELED PUSHBACK\n"; 
       std::cout << "CELLS TRAVELLED SIZE IS " << eOrb.cellsTravelled.size() << std::endl;
        }
        eOrb.cellsTravelled.push_back(eOrb.originCellNumber);  // ! SEG FAULT CAUSER? YES MOST LIKELY 
        if (debugClog) std::cout << "PUSHED" << std::endl;

        if (debugClog) std::cout << "EORB CELLSTRAVELED SIZE\n"; 
        if (eOrb.cellsTravelled.size() == electricityOrbLifeSpan) removeOrb = true; 

        if (debugClog) {
        std::cout << "ORIGIN: " << eOrb.originCellNumber << " DESTINATION: " << eOrb.destinationCellNumber << " DIRECTION: " << eOrb.direction << std::endl;
        }
        Direction direction = electricityCheckForNonEmpty(checkDirection(eOrb.destinationCellNumber), eOrb.destinationCellNumber); // still being used to tell where the electricity will be going 
        int index = eOrb.destinationCellNumber; 

        if (eOrb.direction == Direction_Spaces::up) {
            direction.down = false; 
        } else if (eOrb.direction == Direction_Spaces::down) {
            direction.up = false;
        } else if (eOrb.direction == Direction_Spaces::left) {
            direction.right = false;
        } else if (eOrb.direction == Direction_Spaces::right) {
            direction.left = false;
        }

        // * LIMITER: ELECTRICITY WONT GO TO A CELL IT HAS ALREADY BEEN TO 

        if (std::count(eOrb.cellsTravelled.begin(), eOrb.cellsTravelled.end(), index - gridRows )) { // checks if vec contains num 
            direction.up = false;
        }
        if (std::count(eOrb.cellsTravelled.begin(), eOrb.cellsTravelled.end(), index + gridRows)) { 
            direction.down = false;
        }
        if (std::count(eOrb.cellsTravelled.begin(), eOrb.cellsTravelled.end(), index - 1)) { 
            direction.left = false;
        }
        if (std::count(eOrb.cellsTravelled.begin(), eOrb.cellsTravelled.end(), index + 1)) {
            direction.right = false;
        }
        // * another limter idea is to have all cells have max wire passing limit, like if 3 orbs passed a cell, an orb can no longer go on that wire until the lever is flicked again

        if (debugClog)
         std::cout << "UP: " << direction.up << " DOWN: " << direction.down << " LEFT: " << direction.left << " RIGHT: " << direction.right << std::endl;

         if (debugClog)
         std::cout << "ABOUT TO MAKE ORB COPY\n";
         ElectricityOrb cOrb = eOrb; // copy of eOrb because
         if (debugClog)
         std::cout << "MADE ORB COPY\n";

        bool eOrbCreated = false; 

        if (direction.up) {
             // * check for important objects 
             if (debugClog)
             std::cout << "CHECKPOINT BEFORE CHECKCELLFORIMPORTANT FOR DIRECTION UP\n";
             checkCellForImportantObjects(index, Direction_Spaces::up, eOrb, Electricity); 
             // * orb handling 
            eOrb.direction = Direction_Spaces::up;
            eOrb.destination = {cOrb.ElectricityOrb.getPosition().x , cOrb.ElectricityOrb.getPosition().y - eachRectHeight}; 
            eOrb.originCellNumber = cOrb.destinationCellNumber;
            eOrb.destinationCellNumber = cOrb.destinationCellNumber - gridRows;
            eOrbCreated = true; 
        }

        // default circ 
        sf::CircleShape circ; 
        circ.setPosition(sf::Vector2f(cOrb.ElectricityOrb.getPosition().x , cOrb.ElectricityOrb.getPosition().y));
        circ.setFillColor(sf::Color(rand()%255, rand()%255, 255)); 
        circ.setRadius(cOrb.ElectricityOrb.getRadius()); 
        std::vector<int> cellsTravelled; 
        cellsTravelled.reserve(electricityOrbLifeSpan);

        if (direction.down) {
            if (debugClog)
            std::cout << "CHECKPOINT BEFORE CHECKCELLFORIMPORTANT FOR DIRECTION DOWN\n";
            checkCellForImportantObjects(index, Direction_Spaces::down, eOrb, Electricity); // ! THIS MIGHT BE THE SEG FAULT CAUSER, JUST WHAT'S BEING PASSED INTO HERE 

            if (!eOrbCreated) {
                eOrb.direction = Direction_Spaces::down;
                eOrb.destination = {eOrb.ElectricityOrb.getPosition().x , eOrb.ElectricityOrb.getPosition().y + eachRectHeight}; 
                eOrb.originCellNumber = eOrb.destinationCellNumber;
                eOrb.destinationCellNumber = eOrb.destinationCellNumber + gridRows;
                eOrbCreated = true;
            } else if (eOrbCreated && makeNewOrb) {
                

                Electricity.push_back({circ, Direction_Spaces::down, {circ.getPosition().x , circ.getPosition().y + eachRectHeight} , cOrb.speed, cOrb.destinationCellNumber + gridRows,  cOrb.destinationCellNumber, cellsTravelled});
            }
        }

        if (direction.left) {
            checkCellForImportantObjects(index, Direction_Spaces::left, eOrb, Electricity); 

            if (!eOrbCreated) {
                eOrb.direction = Direction_Spaces::left;
                eOrb.destination = {eOrb.ElectricityOrb.getPosition().x - eachRectWidth , eOrb.ElectricityOrb.getPosition().y}; 
                eOrb.originCellNumber = eOrb.destinationCellNumber;
                eOrb.destinationCellNumber = eOrb.originCellNumber - 1;
                eOrbCreated = true;
            } else if (eOrbCreated && makeNewOrb) {

                Electricity.push_back({circ, Direction_Spaces::left, {circ.getPosition().x - eachRectWidth, circ.getPosition().y } , cOrb.speed, cOrb.destinationCellNumber - 1, cOrb.destinationCellNumber, cellsTravelled});
            }
        }

        if (direction.right) {
            checkCellForImportantObjects(index, Direction_Spaces::right, eOrb, Electricity); 

            if (!eOrbCreated) {
                eOrb.direction = Direction_Spaces::right;
                eOrb.destination = {eOrb.ElectricityOrb.getPosition().x + eachRectWidth , eOrb.ElectricityOrb.getPosition().y}; 
                eOrb.originCellNumber = eOrb.destinationCellNumber;
                eOrb.destinationCellNumber = eOrb.originCellNumber + 1;
                eOrbCreated = true; 
            } else if (eOrbCreated && makeNewOrb) { 

                Electricity.push_back({circ, Direction_Spaces::right, {circ.getPosition().x + eachRectWidth, circ.getPosition().y } , cOrb.speed, cOrb.destinationCellNumber + 1, cOrb.destinationCellNumber, cellsTravelled});
            }
        }

        if (removeOrb || !eOrbCreated)  {
            checkCellForImportantObjects(index, Direction_Spaces::none, eOrb, Electricity); 
            if (debugClog) {
            std::cout << "REMOVE ORB, ORBCREATED = " << removeOrb << " " << eOrbCreated << "AND ORBINDEX = " << eOrbIndex << std::endl;
            std::cout << "NOW LOGGING ELECITRICITIES\n";
            }

            int CorrectIndex = -1; 
            it_ iterator = 0; 
            for (ElectricityOrb &t : Electricity) {
                if (debugClog) std::cout << t.direction << std::endl;
                if (
                    t.originCellNumber == eOrb.originCellNumber &&
                    t.destinationCellNumber == eOrb.destinationCellNumber &&
                    t.direction == eOrb.direction && 
                    t.destination.X == eOrb.destination.X &&
                    t.destination.Y == eOrb.destination.Y && 
                    t.ElectricityOrb.getPosition().x == eOrb.ElectricityOrb.getPosition().x &&
                    t.ElectricityOrb.getPosition().y == eOrb.ElectricityOrb.getPosition().y

                ) {
                    CorrectIndex = iterator;
                    break; 
                }
                iterator++;
            }
            Electricity.erase(Electricity.begin() + CorrectIndex); // ! Seg fault causer maybe ? ?
        }
    }
};


class PowerSystem {
private:

public:
    // * idea: only use one electricity instance and just have vector that handles the deletion of the orbs 
    std::vector<ElectricityOrb> Electricity; 
    
    // create a class of pointer orb objects
    void createElectricity(XY position, XY destination, int direction, int speed, float radius) {
        sf::CircleShape circ; 
        std::vector<int> cellsTravelled; 
        circ.setPosition(sf::Vector2f(position.X, position.Y)); 
        circ.setFillColor(sf::Color(255, 255, 0)); 
        circ.setRadius(radius); 

        Electricity.push_back({circ, direction, destination , speed}); 
       
    } 
};

class Game {
private: 
    sf::RenderWindow* window; 
    sf::Event event; 
    sf::Text text;
    Timer timer;
    Grid grid; 
    PowerSystem powerSystem; 

    bool mouseHeld = false;
    sf::Vector2i mousePosWindow; /// vector2i = 2 integers
    sf::Vector2f mousePosView;

    std::vector<EntityandName<sf::Text>> textHolder;
    std::vector<EntityandName<sf::RectangleShape>> rectangleHolder; 
    std::vector<EntityandName<sf::Sprite>> spriteHolder; // * moe like tool/object holder in the context of the game 
    GAME_STATES GAME_STATE = debug; 
public:
    Game() {
        srand(time(0)); 
        window = nullptr; 
        window = new sf::RenderWindow(sf::VideoMode(width, height), "Wire Desire", sf::Style::Close | sf::Style::Titlebar); 
        window->setFramerateLimit(frameRate);
        if (!objectSheet.loadFromFile("./graphics/modv2ObjSheetinUse.png")) { // ! modified version
            std::cout << "Error loading object sheet png" << std::endl; 
        }

        // * Creating instance of class and deleting it. using pointers and manual allocations is messy to try to avoid  
        // Electricity* e = new Electricity; 
        // e->cellsTravelled.push_back(0); 
        // delete e; 

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
                16, 16, 16, 16, 16, 16, 16,
16, 16, 16, 16, 16, 16, 16,
16, 11, 01, 04, 01,  8, 16,
05, 16,  7, 16, 16, 16, 05,
16, 16,  7, 16, 16, 16, 16,
16, 16, 16, 16, 16, 16, 16,
16, 16,  6, 16, 16, 16, 16,
            };

            grid.describeGrid(7, 7, 150, 150, {80, 80}, gridMatrix);
            grid.fixWires();
            // grid.sendElectricityFromLever(powerSystem.Electricity, electricitySpeed, grid.eachRectHeight / 10 );
 
            // powerSystem.createElectricity({100, 100}, 1.f, grid.eachRectHeight / 10 ); // * rect radius is arbitrary idk what to put  
            grid.changeWhatObjectIsFacing(23, Direction_Spaces::right);  

        }
    }
    // * very not dry ????
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
            // ! move debug state to be later on in the else-if chain 
        } else if (GAME_STATE == debug) {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                if (!mouseHeld) {
                    if (grid.checkIfLeverClick(mousePosView)) {
                        grid.toggleLever();

                        if (grid.isLeverOn())
                        grid.sendElectricityFromLever(powerSystem.Electricity, electricitySpeed, grid.eachRectHeight / 10 ); // elecitricty radius is arbitrary idk 

                        mouseHeld = true; 
                    }
                }

            } else if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                mouseHeld = false; 
            }
            
        }
    }

    void moveElectricity() {
        // * iterates through all the orbs and moves them in the direction they're supposed to go and once they hit the center of a cell, they're deleted a new one spawns if applicable 
        it_ iterator = 0; // already spent the making this a for in loop dont want to redo it to a for i 
        for (ElectricityOrb &t : powerSystem.Electricity) {
           
            int distance = abs( (t.ElectricityOrb.getPosition().x + t.ElectricityOrb.getPosition().y) - (t.destination.X + t.destination.Y)); // manhattan distance, not euclidean  

            bool destinationXAxisReached = (distance <= electricitySpeed);
            bool destinationYAxisReached = (distance <= electricitySpeed); // not sure if the minium distance should just be the electricity speed. i think it's good tho 

            bool movedInXAxis = false; 
            static int doDelete = 0; // * this var is here so there's a bit of delay because when multiple electricity orbs are deleted at the same time, compiler be like "LOL *starts beat boxing*" 
            
            if (destinationXAxisReached && destinationYAxisReached) {
                // if (doDelete % 3 == 0) {
                    if (debugClog) std::cout << "DEST X Y HIT\n"; 

                    // int originCellNumber = t.originCellNumber, destinationCellNumber = t.destinationCellNumber, direction = t.direction;
                    
                    // * NOT GONNA DELETE YET, DELETE ONCE TRAVELLED 3 CELLS 
                    //  powerSystem.Electricity.erase(powerSystem.Electricity.begin() + iterator); 
                    // iterator--; // ! potential source of bug, really noticable because u will get Abort Trap error or something like that. this is here because (wordy explanation as to why seg fault happens) and doing this hopefully prevents it
                    // * dont need to do deleting here or slowing it down as it's handled elsewhere 

                    if (debugClog) std::cout << "GIVING GRID.RECEVIE ITERATOR= " << iterator << std::endl;
                    grid.receiveElectricity(t, powerSystem.Electricity, iterator); // ! THIS MIGHT BE THE SEG FAULT CAUSER ;;   considering removing iterator as it's not accurate somehow 
                    

                // }
                doDelete++; 
                iterator++; 
                continue; 
                
            }

            if (!destinationXAxisReached) {
                // for x axis, it's only 2 (left), 3 (right)
                if (t.direction == 2) { // * left 
                    t.ElectricityOrb.move(-1 * t.speed, 0);
                    movedInXAxis = true;

                } else if (t.direction == 3) { // * right
                    t.ElectricityOrb.move(1 * t.speed, 0);
                    movedInXAxis = true;
                } 
            }

            if (!destinationYAxisReached && !movedInXAxis) {
                // for y axis, it's only 0 (up), 1 (down)
                if (t.direction == 0) { // * up
                    t.ElectricityOrb.move(0, -1 * t.speed);
                } 
                else if (t.direction == 1) { // * down
                    t.ElectricityOrb.move(0, 1 * t.speed);
                }
            }
            iterator++; 

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

        for (ElectricityOrb &t : powerSystem.Electricity ) {
            window->draw(t.ElectricityOrb);
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
        powerSystem.Electricity.clear();
        powerSystem.Electricity.shrink_to_fit();
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
        game.moveElectricity(); 
        game.render(); 
    }
    game.onGameEnd(); 
    
    return 0; 
}