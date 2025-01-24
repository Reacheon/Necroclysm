export module funcVar; 

import Player;

export int PlayerX() { return Player::ins()->getGridX(); }
export int PlayerY() { return Player::ins()->getGridY(); }
export int PlayerZ() { return Player::ins()->getGridZ(); }