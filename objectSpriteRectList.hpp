#pragma once
// * grid size in object sheet is 204 x 187 about

struct intRectAndName { sf::IntRect rect; std::string name; };
intRectAndName objPointHolder[18]; 

sf::IntRect horizontal_wire_2_way(1, 5, 204, 183);               // * 0 
sf::IntRect wire_4_way(207, 2, 203, 186);                        // * 1 
sf::IntRect bent_wire_2_way(412, 5, 203, 183);                   // * 2 
sf::IntRect vertical_wire_2_way(618, 2, 203, 185);               // * 3
sf::IntRect power_extender(1, 190, 203, 183);                    // * 4
sf::IntRect power_block(206, 190, 203, 183);                     // * 5 
sf::IntRect power_detector(412, 190, 203, 183);                  // * 6 
sf::IntRect power_dispenser(617, 190, 203, 183);                 // * 7 
sf::IntRect neutral_winning_block(1, 398, 202, 153);             // * 8 
sf::IntRect activated_winning_block(208, 398, 202, 153);         // * 9 
sf::IntRect lever_on (412, 398, 202, 153);                       // * 10 
sf::IntRect lever_off (618, 398, 202, 153);                      // * 11
sf::IntRect normal_block_pusher_unpowered (1, 612, 202, 133);    // * 12
sf::IntRect normal_block_pusher_powered (208, 612, 202, 133);    // * 13
sf::IntRect sticky_block_pusher_unpowered (412, 612, 202, 133);  // * 14
sf::IntRect sticky_block_pusher_powered(618, 612, 202, 133);     // * 15
sf::IntRect empty(0, 0, 0 ,0);                                   // * 16 
sf::IntRect wire_3_way(1, 612 + 200, 202, 133 + 40);             // * 17