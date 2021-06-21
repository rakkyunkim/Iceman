#include "StudentWorld.h"
#include <string>
#include <queue>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

int StudentWorld::move() {
	setDisplayText();
	if (player->isDead() == true)
		return GWSTATUS_PLAYER_DIED;
	player->doSomething();
	for (int i = 0; i != actors.size(); i++)
		if (actors[i]->isDead()) {
			delete actors[i];
			actors.erase(actors.begin() + i);
			i--;
		}
	for (int i = 0; i != actors.size(); i++) {
		actors[i]->doSomething();
	}
	//if oil all picked up, go to next level.
	if (getNumOilLeftToPickUp() == 0)
		return GWSTATUS_FINISHED_LEVEL;
	//G = current_level_number * 25 + 300
	//1/G chance Goodies appear, and 1/5 chance sonar appears and 4/5 chance gold nugget appears.
	//1 = SonarKit
	//2 = water
	//if there is no sonar already, spawn sonarkit by chance.
	int chance = chanceGoodiesAppear();
	if (chance == 1) {
		if (!isThereSonar()) {
			actors.push_back(new SonarKits(this, 0, 60));
		}
	}
	else if (chance == 2) {
		int x, y;
		getWaterPosition(x, y);
		actors.push_back(new WaterPool(this, x, y));
	}
	int maxProtesters = std::min(15.0, 2 + getLevel() * 1.5);
	if (numProtesters <= maxProtesters && ticksTillNextSpawn == 0) {
		int probHardcore = std::min((unsigned)90, getLevel() * 10 + 30);
		int randNum = rand() % 100;
		if (randNum < probHardcore) 
			actors.push_back(new HardcoreProtesters(this, 60, 60));		
		else
			actors.push_back(new RegularProtesters(this, 60, 60));
		numProtesters++;
	}
	//reset spawn tick
	if(ticksTillNextSpawn == 0)
		ticksTillNextSpawn = std::max((unsigned int)25, 200 - getLevel());
	ticksTillNextSpawn--;

	return GWSTATUS_CONTINUE_GAME;
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp
int StudentWorld::init() {
	//summon ICEMAN
	player = new Iceman(this, 30, 60);
	//summon Boulders
	for (int b = 0; b < numBoulders(); b++) {
		int x, y;
		getBoulderPosition(x, y);
		//add boulder to vector of actors
		actors.push_back(new Boulder(this, x, y));
	}
	//randomly spwan one protester
	int randNum = rand() % 2;
	if(randNum == 0)
		actors.push_back(new RegularProtesters(this, 60, 60));
	else if(randNum == 1)
		actors.push_back(new HardcoreProtesters(this, 60, 60));
	numProtesters = 1;
	ticksTillNextSpawn = std::max((unsigned int)25, 200 - getLevel());
	//summon gold nuggets
	for (int g = 0; g < numGolds(); g++) {
		int x, y;
		getGoodiesPosition(x, y);
		actors.push_back(new GoldNuggets(this, x, y, false));
	}
	//summon oil barrels
	for (int p = 0; p < numBarrels(); p++) {
		int x, y;
		getGoodiesPosition(x, y);
		//push back oil barrels.
		actors.push_back(new OilBarrel(this, x, y));
	}
	//summon ice field
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			bool isThereBoulder = false;
			if (j >= 60) {
				//no ice above 60
				IceField[i][j] = nullptr;
				//pathBFS[i][j] = 'G';
				continue;
			}
			if (i <= 33 && i >= 30 && j >= 4 && j <= 59) {
				//no ice in ice shaft
				IceField[i][j] = nullptr;
				//pathBFS[i][j] = 'G';
				continue;
			}
			else {
				for (int k = 0; k != actors.size(); k++) {
					//iterate through actor to find boulder
					if (actors[k]->isBoulder()) {
						//save boulder position.
						int x = actors[k]->getX();
						int y = actors[k]->getY();
						//avoiding boulder when creating ice.
						if (i <= (x + 3) && j <= (y + 3) && i >= x && j >= y) {
							isThereBoulder = true;
							IceField[i][j] = nullptr;
							//pathBFS[i][j] = 'B';
						}
					}
				}
				//check if the position is occupied by boulder.
				if (isThereBoulder == false) {
					IceField[i][j] = new Ice(this, i, j);
				}
			}
		}
	}
	

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp() {
	//called when Iceman lost life or completed current level.
	//every actor in the entire oil field
	//(the Iceman and every Protester, Goodies like Nuggets, Sonar Kits and Water, Barrels of oil, Boulders, Ice, etc.) must be deleted
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			if (IceField[i][j] != nullptr) {
				delete IceField[i][j];
				IceField[i][j] = nullptr;
			}
		}
	}
	while (actors.size() != 0) {
		delete actors[0];
		actors.erase(actors.begin());
	}
	delete player;
}
StudentWorld::~StudentWorld() {
	cleanUp();
}
string StudentWorld::formatDisplayText(int level, int lives, int health, int score, int oil, int sonar, int gold, int squirt) {
	string s = "Lvl: " + to_string(level) + " Lives: " + to_string(lives) + " Hlth: " + to_string(health) + "%" + " Score: " + to_string(score) + " Squirt: " + to_string(squirt) + " Sonar: " + to_string(sonar) + " Gold: " + to_string(gold) + " Oil Left: " + to_string(oil);
	return s;
}
void StudentWorld::setDisplayText()
{
	int level = getLevel();
	int lives = getLives();
	int health = 10 * getCurrentHealth();
	int squirts = getSquirtStocks();
	int gold = getGoldStocks();
	int barrelsLeft = getNumOilLeftToPickUp(); 
	int sonar = getSonarStocks();
	int score = getScore();
	// Next, create a string from your statistics, of the form:
	// Lvl: 52 Lives: 3 Hlth: 80% Wtr: 20 Gld: 3 Oil Left: 2 Sonar: 1 Scr: 321000
	string s = formatDisplayText(level, lives, health, score, barrelsLeft, sonar, gold, squirts);
		//squirts, gold, barrelsLeft, sonar, score);
	// Finally, update the display text at the top of the screen with your
	// newly created stats
	setGameStatText(s); // calls our provided GameWorld::setGameStatText
}



void StudentWorld::breakIce(int x, int y) {
	int hasDug = false;
	for (int i = 0; i != 4; i++) {
		for (int j = 0; j != 4; j++) {
			Ice* ice = IceField[x + i][y + j];
			if (ice != nullptr) {
				delete ice;
				IceField[x + i][y + j] = nullptr;
				//pathBFS[x + i][y + j] = 'G';
				hasDug = true;
			}
		}
	}
	if(hasDug)
		playSound(SOUND_DIG);
	
}
//check if squirt is blocked by ice or boulder.
bool StudentWorld::canSquirtMoveTo(int x, int y) {
	//1) blocked by ice.
	for (int i = 0; i != 4; i++) {
		for (int j = 0; j != 4; j++) {
			Ice* ice = IceField[x + i][y + j];
			if (ice != nullptr) {
				return false;
			}
		}
	}
	//2)blocked by boulder OR protester
	for (int i = 0; i < actors.size(); i++) {
		if (actors[i]->isBoulder() ) {
			if (checkDistance(actors[i], x, y, 3))
				return false;
		}
	}
	return true;
}
//check to see if there is ice numIce below the player.
bool StudentWorld::canBoulderFall(int numIce, int x, int y) {

	for(int i = 0; i <= numIce; i++){
		for (int j = 0; j != 4; j++) {
			Ice* ice = IceField[x + j][y - i];
			if (ice != nullptr)
				return true;
		}
	}
	return false;

}

//***no actors can go out of scope (60 by 60)****
//Iceman: can move thru ice but not thru boulders
//same for protesters
bool StudentWorld::isPlayerMovableTo(Actor* a, int x, int y) {
	//out of bounds.
	if (x > 60 || x < 0 || y > 60 || y < 0)
		return false;
	//hit boulder
	for (int i = 0; i < actors.size(); i++) {
		if (actors[i]->isBoulder()) {
			if (checkDistance(actors[i], x, y, 3))
				return false;
		}
	}
	//protester hit ice
	if (a->isProtester()) {
		for (int i = 0; i != 4; i++) {
			for (int j = 0; j != 4; j++) {
				if (IceField[x + i][y + j] != nullptr) {
					return false;
				}
			}
		}
		if (checkDistance(player, x, y, 3)) {
			return false;
		}
	}
	return true;
}
bool StudentWorld::isPlayerMovableTo2(Actor* a, int x, int y) {
	//out of bounds.
	if (x > 60 || x < 0 || y > 60 || y < 0)
		return false;
	//hit boulder
	for (int i = 0; i < actors.size(); i++) {
		if (actors[i]->isBoulder()) {
			if (checkDistance(actors[i], x, y, 3))
				return false;
		}
	}
	//protester hit ice
	if (a->isProtester()) {
		for (int i = 0; i != 4; i++) {
			for (int j = 0; j != 4; j++) {
				if (IceField[x + i][y + j] != nullptr) {
					return false;
				}
			}
		}
	}
	return true;
}

//pass by reference actor and save boulder
bool StudentWorld::isThereBoulderNearIceman(Actor*& a, int x, int y) {
	for (int i = 0; i < actors.size(); i++) {
		if (actors[i]->isBoulder()) {
			a = actors[i];
			if (checkDistance(actors[i], x, y, 3))
				return true;
		}
	}
	return false;
}

bool StudentWorld::isBoulderMovableTo(Actor* a, int x, int y) {
	//stop when:
	//1)out of bound
	//2)hit ice
	//3)hit top of another boulder
	//4)hit iceman.(iceman dies instantly).
	//must disappear after falling.

	//out of bound
	if (x > 60 || x < 0 || y > 60 || y < 0) {
		a->setDead();
		return false;
	}
	//hit ice'
	//canBoulderFall(0, x, y)
	else if (canBoulderFall(0, x, y)) {
		a->setDead();
		return false;
	}
	//hit another boulder
	else if (isthereAnotherBoulder(a, x, y)) {
		a->setDead();
		return false;
	}
	else {
		return true;
	}

}
bool StudentWorld::checkDistance(Actor* a, int x, int y, int distance) const {
	//distance formula
	int dist = sqrt(pow(a->getX() - x, 2) + pow(a->getY() - y, 2));
	return (dist <= distance);
}

//check to see if there is a boulder in a given coordinate.
//used to check if boulders collide when falling.
bool StudentWorld::isthereAnotherBoulder(Actor* a, int x, int y) {
	for (int i = 0; i != actors.size(); i++) {
		//if it is a different boulder
		if (actors[i]->isBoulder() && a != actors[i]) {
			//if boulders meet, return false. otherwise return true.
			if (checkDistance(actors[i], x, y, 3))
				return true;
		}
	}
	return false;
}
//Boulder Spawn req: X = 0 to 60
//					 Y = 20 to 56
//at least 6 distance away from other actors
void StudentWorld::getBoulderPosition(int& x, int& y) {
	x = rand() % 61;
	y = rand() % 37 + 20; // 20 to 56

	if (x >= 27 && x <= 33) {
		getBoulderPosition(x, y);
		return;
	}
	for (int i = 0; i < actors.size(); i++) {
		//loop through actors and check if the random position is greater than 6 positions away.
		if (checkDistance(actors[i], x, y, 6)) {
			getBoulderPosition(x, y);
			return;
		}
	}
}
//Goodies Spawn req: X = 0 to 60
//					 Y = 0 to 56
//at least 6 distance away from other actors
void StudentWorld::getGoodiesPosition(int& x, int& y) {
	x = rand() % 61;  //0 to 60
	y = rand() % 57; // 0 to 56
	if (x >= 27 && x <= 33) {
		getGoodiesPosition(x, y);
		return;
	}
	for (int i = 0; i < actors.size(); i++) {
		//loop through actors and check if the random position is greater than 6 positions away.
		//if less than 6, call this function recurrently.
		if (checkDistance(actors[i], x, y, 6)) {
			getGoodiesPosition(x, y);
			return;
		}
	}
}
//generate position for water where ice is not present.
void StudentWorld::getWaterPosition(int& x, int& y) {
	x = rand() % 61;
	y = rand() % 61;
	for (int i = 0; i != 4; i++) {
		for (int j = 0; j != 4; j++) {
			if (IceField[x + i][y + j] != nullptr) {
				getWaterPosition(x, y);
				return;
			}
		}
	}
}

//reveal the radius of 12 near iceman.
//set the actors state to "revealed" so it wont go invisible again.
void StudentWorld::revealAreaWithSonar() {
	int x = player->getX();
	int y = player->getY();
	for (int i = 0; i != actors.size(); i++) {
		if (checkDistance(actors[i], x, y, 12)) {
			actors[i]->setVisible(true);
			actors[i]->setRevealed();
		}
	}
}
int StudentWorld::chanceGoodiesAppear() {
	//G = current_level_number * 25 + 300
	//1/G chance Goodies appear, and 1/5 chance sonar appears and 4/5 chance water appears.
	int chance = (1/(getLevel() * 25 + 300)) * 100; //percent chance of goodies spawn
	if ((rand() % 100) < chance) {
		if ((rand() % 100) < 20) {// 1/5
			//sonar
			return 1;
		}
		else //water pool
			return 2;
	}
}
bool StudentWorld::isThereSonar() {

	for (int i = 0; i < actors.size(); i++) {
		if (!actors[i]->isBoulder()) {
			if (actors[i]->getX() == 0 && actors[i]->getY() == 60)
				return true;
		}
	}
	return false;
}
void StudentWorld::dropGold(int x, int y) {
	actors.push_back(new GoldNuggets(this, x, y, true));
}
void StudentWorld::pushBackActor(Actor* a) {
	actors.push_back(a);
}
GraphObject::Direction StudentWorld::pickRandomDir() {
	int i = rand() % 4 + 1; // 1 to 4

	if (i == 1) {
		return GraphObject::up;
	} else if (i == 2) {
		return GraphObject::down;
	}
	else if (i == 3) {
		return GraphObject::right;
	}
	else if (i == 4) {
		return GraphObject::left;
	}
}
void StudentWorld::moveInTheDirectionFacing(Actor* a) {
	GraphObject::Direction dir = a->getDirection();
	int x = a->getX();
	int y = a->getY();

	if (dir == GraphObject::up && isPlayerMovableTo(a, x, y + 1)) {
		a->moveTo(x, y + 1);
	}
	else if (dir == GraphObject::down && isPlayerMovableTo(a, x, y - 1)) {
		a->moveTo(x, y - 1);
	}
	else if (dir == GraphObject::left && isPlayerMovableTo(a, x - 1, y)) {
		a->moveTo(x - 1, y);
	}
	else if (dir == GraphObject::right && isPlayerMovableTo(a, x + 1, y)) {
		a->moveTo(x + 1, y);
	}
	else if (dir == GraphObject::none)
		a->moveTo(x, y);
}
void StudentWorld::moveInTheDirectionFacingWithoutChecking(Actor* a) {
	GraphObject::Direction dir = a->getDirection();
	int x = a->getX();
	int y = a->getY();

	if (dir == GraphObject::up) {
		a->moveTo(x, y + 1);
	}
	else if (dir == GraphObject::down) {
		a->moveTo(x, y - 1);
	}
	else if (dir == GraphObject::left) {
		a->moveTo(x - 1, y);
	}
	else if (dir == GraphObject::right) {
		a->moveTo(x + 1, y);
	}
}
//return true if protester can move to dir. Return false otherwise.
bool StudentWorld::isProtesterMovableTo(Actor* a, GraphObject::Direction dir) {

	int x = a->getX();
	int y = a->getY();

	if (dir == GraphObject::up && isPlayerMovableTo(a, x, y + 1)) {
		a->setDirection(GraphObject::up);
		return true;
	}
	else if (dir == GraphObject::down && isPlayerMovableTo(a, x, y - 1)) {
		a->setDirection(GraphObject::down);
		return true;
	}
	else if (dir == GraphObject::left && isPlayerMovableTo(a, x - 1, y)) {
		a->setDirection(GraphObject::left);
		return true;
	}
	else if (dir == GraphObject::right && isPlayerMovableTo(a, x + 1, y)) {
		a->setDirection(GraphObject::right);
		return true;
	}
	else
		return false;
}
//return true if can move to perpendicular direction. Save the direction in dir.
bool StudentWorld::isPerpenDirMovable(Actor* a, GraphObject::Direction& dir) {
	int x = a->getX();
	int y = a->getY();
	if (a->getDirection() == GraphObject::up || a->getDirection() == GraphObject::down) {
		//if both directions are movable, pick random direction out of those two.
		if (isPlayerMovableTo(a, x + 1, y) && isPlayerMovableTo(a, x - 1, y)) {
			int randNum = rand() % 2;
			if (randNum == 0)
				dir = GraphObject::left;
			else if (randNum == 1)
				dir = GraphObject::right;
			return true;
		}
		if (isPlayerMovableTo(a, x + 1, y)) {
			dir = GraphObject::right;
			return true;
		}
		if (isPlayerMovableTo(a, x - 1, y)) {
			dir = GraphObject::left;
			return true;
		}
	}
	else if (a->getDirection() == GraphObject::right || a->getDirection() == GraphObject::left) {
		//if both directions are movable, pick random direction out of those two.
		if (isPlayerMovableTo(a, x, y + 1) && isPlayerMovableTo(a, x, y - 1)) {
			int randNum = rand() % 2;
			if (randNum == 0)
				dir = GraphObject::up;
			else if (randNum == 1)
				dir = GraphObject::down;
			return true;
		}
		if (isPlayerMovableTo(a, x, y + 1)) {
			dir = GraphObject::up;
			return true;
		}
		if (isPlayerMovableTo(a, x, y - 1)) {
			dir = GraphObject::down;
			return true;
		}
	}
	return false;
}
bool StudentWorld::annoyProtesters(Actor* a) {
	for (int i = 0; i < actors.size(); i++) {
		if (actors[i]->isProtester()) {
			if (checkDistance(actors[i], a->getX(), a->getY(), 4)) {
				actors[i]->setAnnoyed(true);
				return true;
			}
		}
	}
	return false;
}
GraphObject::Direction StudentWorld::trackIceman(Actor* a) {
	//protester's x and y
	int px = a->getX();
	int py = a->getY();
	//iceman's x and y
	int ix = player->getX();
	int iy = player->getY();

	//checking if iceman and protester is in perpendicular to each other
	//then, if iceman y is bigger, that means
	//iceman is above the protester.
	if (abs(px - ix) <= 4 && py < iy) {
		for (int i = 0; i <= (abs(py - iy)); i++) {
			if (!isPlayerMovableTo2(a, px, py + i))
				return GraphObject::none;
		}
		return GraphObject::up;
	}
	//iceman is below the protester.
	else if (abs(px - ix) <= 4 && py > iy) {
		for (int i = 0; i <= (abs(py - iy)); i++) {
			if (!isPlayerMovableTo2(a, px, py - i))
				return GraphObject::none;
		}
		return GraphObject::down;
	}
	//iceman on the right
	else if (abs(py - iy) <= 4 && px < ix) {
		for (int i = 0; i <= abs(px - ix); i++) {
			if (!isPlayerMovableTo2(a, px + i, py))
				return GraphObject::none;
		}
		return GraphObject::right;
	}
	//iceman on the left
	else if (abs(py - iy) <= 3 && px > ix) {
		for (int i = 0; i <= abs(px - ix); i++) {
			if (!isPlayerMovableTo2(a, px - i, py))
				return GraphObject::none;
		}
		return GraphObject::left;
	}
	return GraphObject::none;

}
//if protester found gold, return true.
bool StudentWorld::foundPickableGold(Actor* a, Actor*& b) {
	for (int i = 0; i < actors.size(); i++) {
		if (actors[i]->isProtester()) {
			if (checkDistance(a, actors[i]->getX(), actors[i]->getY(), 3)) {
				b = actors[i];
				return true;
			}
		}
	}
	return false;
}

//this function will fill up the degreeTracker array with the appropriate degree.
void StudentWorld::bfs_queue(Actor* a, int sx, int sy) {
	setDegreeTracker();
	int x;
	int y;
	int curX;
	int curY;
	int dx[4] = { -1, 1, 0, 0 };
	int dy[4] = { 0, 0, 1, -1 };
	int finalX = a->getX();
	int finalY = a->getY();
	bool reached_end = false;
	int move_count = 0;

	queue<pair<int, int>> que;

	que.push({ sx, sy });
	degreeTracker[sx][sy] = 0;


	while (!que.empty()) {
		//track parent nodes.
		x = que.front().first;
		y = que.front().second;
		que.pop();

		if (x == finalX && y == finalY) {
			reached_end = true;
		}
		move_count = degreeTracker[x][y];
		move_count++;
		//check neighboring nodes.
		for (int i = 0; i < 4; i++) {
			curX = x + dx[i];
			curY = y + dy[i];
			//out of bounds
			if (curX < 0 || curY < 0)
				continue;
			if (curX > 60 || curY > 60)
				continue;
			//check visited
			if (degreeTracker[curX][curY] != 1000)
				continue;
			//check blocked cells
			if (!isPlayerMovableTo2(a, curX, curY))
				continue;
			//push x and y if passed all these.
			que.push({ curX, curY });
			//track corresponding degrees.
			degreeTracker[curX][curY] = move_count;
		}
	}
}

//start from (x,y) in degreeTracker and 
//fill up path vector according to degreeTracker,
//following minimum degrees.
void StudentWorld::directionToPath(int sx, int sy, int ex, int ey, GraphObject::Direction& dir) {
	//from protester's position, go back to start position, comparing neighboring values in degreeTracker.
	int minDegree;
	minDegree = std::min(min(degreeTracker[ex - 1][ey], degreeTracker[ex + 1][ey]), min(degreeTracker[ex][ey + 1], degreeTracker[ex][ey - 1]));
	if (minDegree == degreeTracker[ex + 1][ey]) {
		degreeTracker[ex + 1][ey] = 1000;
		dir = GraphObject::right;
	}
	else if (minDegree == degreeTracker[ex][ey + 1]) {
		degreeTracker[ex][ey + 1] = 1000;
		dir = GraphObject::up;
	}
	else if (minDegree == degreeTracker[ex - 1][ey]) {
		degreeTracker[ex - 1][ey] = 1000;
		dir = GraphObject::left;
	}
	else {
		degreeTracker[ex][ey - 1] = 1000;
		dir = GraphObject::down;
	}
}

void StudentWorld::setDegreeTracker() {
	//set trackSteps to a large number to make every space unmoveable. We'll change the moveable ones to lower numbers so we can use comparison to decide which direction to go
	for (int i = 0; i < 61; i++) {
		for (int j = 0; j < 61; j++) {
			degreeTracker[i][j] = 1000;
		}
	}
}
