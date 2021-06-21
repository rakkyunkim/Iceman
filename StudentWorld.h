#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <vector>

class Ice;
class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir)
		: GameWorld(assetDir)
	{
	}
	~StudentWorld();
	virtual int init();
	virtual int move();
	virtual void cleanUp();
	void setDisplayText();
	Iceman* getPlayer() { return player; }
	std::string formatDisplayText(int level, int lives, int health, int score, int oil, int sonar, int gold, int squirt);
	int getCurrentHealth() { return player->getHealth(); };
	void incBarrelCount() { player->incOilStock(); }
	void incSonarCount() { player->incSonarStock(); }
	void incGoldCount() { player->incGoldStock(); }
	void refillWater() { player->fillWater(); }
	int getNumOilLeftToPickUp() { return player->numOilStock(); }
	int getSonarStocks() { return player->numSonarStock(); }
	int getGoldStocks() { return player->numGoldStock(); }
	int getSquirtStocks() { return player->numSquirtStock(); }
	void decNumProtester() { numProtesters--; }

	//drop gold.
	void dropGold(int x, int y);
	//check if there is already a sonar spawned.
	bool isThereSonar();
	//calculate the spawn chance of goodies(water, sonar)
	int chanceGoodiesAppear();
	//make visible goodies with sonar.
	void revealAreaWithSonar();
	//check if there is a boulder in the way of iceman and pass by reference that boulder to check if it's falling or not.
	bool isThereBoulderNearIceman(Actor*& a, int x, int y);
	//generate randomly the spawn coordinates of boulders.
	void getBoulderPosition(int& x, int& y);
	//generate randomly the spawn coordinates of goodies(gold, oil)
	void getGoodiesPosition(int& x, int& y);
	//return numbers of boulders, oil barrels, and golds each level.
	int numBoulders() { return std::min(getLevel() / 2 + 2, (unsigned int)9);}
	int numBarrels() { return std::min(2 + getLevel(), (unsigned int)21); }
	int numGolds() { return std::max(5 - getLevel() / 2, (unsigned int)2); }
	//check if there is ice under numIce at coordinate (x, y).
	//It's used to check if boulder should fall or not.
	bool canBoulderFall(int numIce, int x, int y);
	//check to see if two boulders hit each other
	bool isthereAnotherBoulder(Actor* a, int x, int y);
	//Iceman breaks ice
	void breakIce(int x, int y);
	//it checks whether Icaman can move to that location or not. (e.g. return false if there is a boulder or out of bound.)
	bool isPlayerMovableTo(Actor* a, int x, int y);
	bool isPlayerMovableTo2(Actor* a, int x, int y);
	//check to see until when boulders can drop. (e.g. return false if out of bound, hit another boulder or player or ice.)
	bool isBoulderMovableTo(Actor* a, int x, int y);
	//check euclidean distance between actor a and coordinate (x, y).
	//return true if the distance is less than radius, and false otherwise. 
	bool checkDistance(Actor* a, int x, int y, int radius) const;
	//push_back actor. Used to push_back squirt into vector of actors.
	void pushBackActor(Actor* a);
	bool canSquirtMoveTo(int x, int y);
	//get open position for water.
	void getWaterPosition(int& x, int& y);
	//generate random direction for protesters.
	GraphObject::Direction pickRandomDir();
	//move the actor in the direction facing if possible.
	void moveInTheDirectionFacing(Actor* a);
	void moveInTheDirectionFacingWithoutChecking(Actor* a);
	//return true if protester can move in the dir. false otherwise.
	bool isProtesterMovableTo(Actor* a, GraphObject::Direction dir);
	//return true if protester can move in perpendicular dir.
	bool isPerpenDirMovable(Actor* a, GraphObject::Direction& dir);
	void annoyHuman(Humans* a, int n) { a->getAnnoyed(n); }
	bool annoyProtesters(Actor* a);
	//return the direction towards iceman when in line of sight.
	GraphObject::Direction trackIceman(Actor* a);
	//return true if protester found gold. pass by reference actor and store protester.
	bool foundPickableGold(Actor* a, Actor*& b);
	//run bfs and fill in degreeTracker with appropriate degrees.
	void bfs_queue(Actor* a, int sx, int sy);
	//get direction by finding the minimum degree around its neighbors. (use degreeTracker)
	void directionToPath(int sx, int sy, int ex, int ey, GraphObject::Direction& dir);
	//initially set all degrees to high number to indicate its not been visited OR its not movable.
	void setDegreeTracker();

private:
	Ice* IceField[64][64];
	//tracks degree of each nodes
	int degreeTracker[61][61];
	//stores final path
	Iceman* player;
	int numProtesters;
	int ticksTillNextSpawn;
	std::vector<Actor*> actors;

};

#endif // STUDENTWORLD_H_
