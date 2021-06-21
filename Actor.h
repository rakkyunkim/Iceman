#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
class StudentWorld;

//Actors: Iceman, Regular Protesters, Hardcore Protesters, 
//		  Boulders, Gold Nuggets, Barrels of oil, Water, 
//		  Squirts from the Iceman’s squirt gun, and Sonar Kits.


// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp
class Actor : public GraphObject {
public:
	Actor(StudentWorld* w, int startX, int startY, Direction startDir, bool visible, int imageID, double size, int depth);
	// Action to perform each tick.
	virtual void doSomething();
	bool isDead() { return heisDead; }
	void setDead() { heisDead = true; }
	StudentWorld* getWorld() const { return world; }
	virtual bool canDigIce() { return false; }
	virtual int getImgID() { return imgID; }
	virtual bool isBoulder() const { return false; }
	virtual bool isheFalling() { return false; }
	//when revealed by sonar
	void setRevealed() { revealed = true; }
	bool isRevealed() { return revealed; }
	virtual bool countTicks(int& t);
	virtual bool isProtester() const { return false; }
	bool isheAnnoyed() { return isAnnoyed; }
	void setAnnoyed(bool annoyed) { isAnnoyed = annoyed; }
	bool hasGold() { return hasPickedUpGold; }
	void setHasPickedUpGold(bool gold) { hasPickedUpGold = gold; }
private:
	bool revealed;
	int imgID;
	StudentWorld* world;
	bool heisDead;
	bool isAnnoyed;
	bool hasPickedUpGold;
};

class Humans : public Actor {
public:
	Humans(StudentWorld* w, int initX, int initY, Direction startDir, int imageID, unsigned int hp);
	virtual void getAnnoyed(int hp) { health -= hp; }
	int getHealth() { return health; }
	//annoy the actor by certain amount.
	//Initial Health: Iceman 10, Protester 5, Hardcore Protester 20
private:
	int health;
};
//things that can be picked up.
class Goodies : public Actor {
public:
	Goodies(StudentWorld* w, int initX, int initY, Direction initDir, int imgID, bool visible);

private:
	int ticks;

};
class Squirts :public Actor {
public:
	Squirts(StudentWorld* w, int initX, int initY, Direction initDir);
	virtual void doSomething();
	virtual bool isSquirt() { return true; }
private:
	int travelDistance;
};
class SonarKits :public Goodies {
public:
	SonarKits(StudentWorld* w, int initX, int initY);
	virtual void doSomething();
private:
	int ticks;
};
class GoldNuggets :public Goodies {
public:
	//invisible if spawned by the field.
	//visible if dropped by Iceman.
	GoldNuggets(StudentWorld* w, int initX, int initY, bool dropped);
	virtual void doSomething();
private:
	int ticks;
	bool isDropped;
};
class WaterPool :public Goodies {
public:
	WaterPool(StudentWorld* w, int initX, int initY);
	virtual void doSomething();
private:
	int ticks;
};
class OilBarrel :public Goodies {
	//starts out invisible.
public:
	OilBarrel(StudentWorld* w, int initX, int initY);
	virtual void doSomething();

};
class Boulder :public Actor {
public:
	Boulder(StudentWorld* w, int initX, int initY);
	virtual void doSomething();
	virtual bool isBoulder() const { return true; };
	virtual bool isheFalling() { return isFalling; }
private:
	int ticksBeforeDrop;
	bool isFalling;
	bool playsound;
};
class Iceman :public Humans {
public:
	Iceman(StudentWorld* w, int initX, int initY);
	virtual void doSomething();
	bool canDigIce() { return true; }
	int numOilStock() { return oilStock; }
	int numSonarStock() { return sonarStock; }
	int numGoldStock() { return goldStock; }
	int numSquirtStock() { return waterStock; }
	void incOilStock() { oilStock--; }
	void incSonarStock() { sonarStock++; }
	void incGoldStock() { goldStock++; }
	void fillWater() { waterStock += 5; }
	void shootSquirt(Direction dir);

private:
	int oilStock;
	int goldStock;
	int sonarStock;
	int waterStock;
};
class Protester :public Humans {
public:
	Protester(StudentWorld* w, int initX, int initY, int imgID, unsigned int health, int score);
	virtual void doSomething();
	bool isProtester() const{ return true; }
	virtual bool isHardcore() { return false; }
private:
	int shoutTicks;
	int vertMoveTicks;
	int ticks;
	int tickCounter;
	bool isLeavingField;
	int numMoves;
	int scoreWhenDead;
	bool hasSoundPlayed;
	bool ranBFS;
};
class RegularProtesters : public Protester {
public:
	RegularProtesters(StudentWorld* w, int initX, int initY);

};

class HardcoreProtesters :public Protester {
public:
	HardcoreProtesters(StudentWorld* w, int initX, int initY);
	virtual bool isHardcore() { return true; }

};

//ICE CLASS
class Ice : public Actor
{
public:
	Ice(StudentWorld* w, int startX, int startY) : Actor(w, startX, startY, right, true, IID_ICE, 0.25, 3) {}

};
#endif // ACTOR_H_
