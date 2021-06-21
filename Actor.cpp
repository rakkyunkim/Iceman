#include "Actor.h"
#include "StudentWorld.h"

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp
Actor::Actor(StudentWorld* w, int initX, int initY, Direction startDir, bool visible, int imageID, double size, int depth)
	: GraphObject(imageID, initX, initY, startDir, size, depth) {
	setDirection(startDir);
	if(visible)
		setVisible(visible);
    world = w;
    imgID = imageID;
    heisDead = false;
    revealed = false;
    isAnnoyed = false;
    hasPickedUpGold = false;
}
void Actor::doSomething() {}
Humans::Humans(StudentWorld* w, int initX, int initY, Direction initDir, int imageID, unsigned int hp)
	:Actor(w, initX, initY, initDir, true, imageID, 1.0, 0)
{
    health = hp;
}

Goodies::Goodies(StudentWorld* w, int initX, int initY, Direction initDir, int imgID, bool visible)
    : Actor(w, initX, initY, initDir, visible, imgID, 1.0, 2)
{
    ticks = 0;

}
Squirts::Squirts(StudentWorld* w, int initX, int initY, Direction initDir)
    : Actor(w, initX, initY, initDir, true, IID_WATER_SPURT, 1.0, 1)
{
    travelDistance = 4;
}

WaterPool::WaterPool(StudentWorld* w, int initX, int initY) 
    : Goodies(w, initX, initY, right, IID_WATER_POOL, true) 
{
    ticks = std::max((unsigned int)100, 300 - 10 * getWorld()->getLevel());
}
OilBarrel::OilBarrel(StudentWorld* w, int initX, int initY) 
    : Goodies(w, initX, initY, right, IID_BARREL, false) 
{
}
// There is a 1 in G chance that a new Water Pool or Sonar Kit Goodie will be added 
// to the oil field during any particular tick, where :
// int G = current_level_number * 25 + 300
// Assuming a new Goodie should be added, there is a 1 / 5 chance that you should
// add a new Sonar Kit, and a 4 / 5 chance you should add a Water Goodie.
// Each new Sonar Kit must be added at x = 0, y = 60 on the screen.
SonarKits::SonarKits(StudentWorld* w, int initX, int initY)
    : Goodies(w, initX, initY, right, IID_SONAR, true)
{
    ticks = std::max((unsigned int)100, 300 - ((10 * getWorld()->getLevel())));

}
GoldNuggets::GoldNuggets(StudentWorld* w, int initX, int initY, bool dropped) 
    : Goodies(w, initX, initY, right, IID_GOLD, false) 
{
    ticks = 100;
    isDropped = dropped;
}
Protester::Protester(StudentWorld* w, int initX, int initY, int imgID, unsigned int health, int score)
    :Humans(w, initX, initY, left, imgID, health)
{
    ticks = std::max((unsigned)0, 3 - getWorld()->getLevel() / 4);
    tickCounter = ticks;
    isLeavingField = false;
    shoutTicks = 15;
    vertMoveTicks = 20;
    numMoves = rand() % 53 + 8;
    scoreWhenDead = score;
    hasSoundPlayed = false;
    ranBFS = false;
}
RegularProtesters::RegularProtesters(StudentWorld* w, int initX, int initY) 
    : Protester(w, initX, initY, IID_PROTESTER, 5, 100) 
{
}
HardcoreProtesters::HardcoreProtesters(StudentWorld* w, int initX, int initY)
    : Protester(w, initX, initY, IID_HARD_CORE_PROTESTER, 20, 250)
{
}
//For causing a Regular Protester to give up: 100 points
//For causing a Hardcore Protester to give up : 250 point
//For causing a Boulder to bonk a Protester: 500 points
//For bribing a Regular Protester with a Gold Nugget: 25 points
//For bribing a Hardcore Protester with a Gold Nugget : 50 points
//Tick till next Protester spawn: int T = max(25, 200 – current_level_number)
//max # of protesters: int P = min(15, 2 + current_level_number * 1.5)
void Protester::doSomething() {
    int x = getX();
    int y = getY();
    Iceman* player = getWorld()->getPlayer();
    //number of moves 8 to 60
    int numSquaresToMoveInCurrentDirection = rand() % 53 + 8;
    int M = 16 + getWorld()->getLevel() * 2;
    //check dead or alive.
    if (isDead())
        return;

    //ran out of health
    if (hasSoundPlayed == false) {
        if (getHealth() <= 0) {
            isLeavingField = true;
            getWorld()->decNumProtester();
            getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
            hasSoundPlayed = true;
            tickCounter = 0;
        }
    }


    //if protester is hit by squirt, stun and annoy by 2.
    if (isheAnnoyed() && !isLeavingField) {
        getAnnoyed(2);
        getWorld()->increaseScore(scoreWhenDead);
        setAnnoyed(false);
        tickCounter = std::max((unsigned)50, 100 - getWorld()->getLevel() * 10);
    }

    //if not done counting ticks, return.
    tickCounter--;
    if (tickCounter > 0)
        return;

    tickCounter = ticks;
    //shout tick = 15
    shoutTicks--;
    vertMoveTicks--;

    //leave-the-oil-field state
    if (isLeavingField) {
        if (x == 60 && y == 60) {
            setDead();
            return;
        }
        else {
            if (ranBFS == false) {
                getWorld()->bfs_queue(this, 60, 60);
                ranBFS = true;
            }
            Direction direction;
            getWorld()->directionToPath(60, 60, getX(), getY(), direction);
            setDirection(direction);
            getWorld()->moveInTheDirectionFacingWithoutChecking(this);
        }
        //TODO: determine path to leaving the field.
    }
    else {
        Direction dir;
        //vertical movement
        if (vertMoveTicks <= 0) {
            if (getWorld()->isPerpenDirMovable(this, dir)) {
                setDirection(dir);
                numMoves = rand() % 53 + 8;
                vertMoveTicks = 200;
            }
        }
        //spot iceman
        Direction toIceman = getWorld()->trackIceman(this);
        if (toIceman != Direction::none) {
            setDirection(toIceman);
        }
        
        //move in the direction facing until can't OR ran out of moves.
        if (numMoves > 0 && getWorld()->isProtesterMovableTo(this, getDirection())) {
            getWorld()->moveInTheDirectionFacing(this);
            numMoves--;

        }//if protester can't move in the direction facing, assign random direction.
        else if ((numMoves == 0 || !getWorld()->isProtesterMovableTo(this, getDirection())) && !getWorld()->checkDistance(player, x, y, 4)) {
            //assign random direction to protester
            dir = getWorld()->pickRandomDir();
            getWorld()->isProtesterMovableTo(this, dir);
            numMoves = rand() % 53 + 8;
        }

        //if close to iceman, shout and annoy
        if (shoutTicks <= 0 && getWorld()->checkDistance(player, x, y, 4)) {
            
            getWorld()->playSound(SOUND_PROTESTER_YELL);
            //get shouted at by protester
            getWorld()->annoyHuman(player, 2);
            //reset shout tick.
            shoutTicks = 15;
        }
        //get hit by boulder
        Actor* b;
        if (getWorld()->isThereBoulderNearIceman(b, x, y) && b->isheFalling()) {
            getWorld()->increaseScore(500);
            getAnnoyed(getHealth());
        }
        if (hasGold()) {
            getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
            //regular protester
            if (scoreWhenDead == 100)
                isLeavingField = true;
            else {
                tickCounter = std::max((unsigned)50, 100 - getWorld()->getLevel() * 10);
                setHasPickedUpGold(false);
            }
        }
    }
}

Iceman::Iceman(StudentWorld* world, int initX, int initY) : Humans(world, initX, initY, right, IID_PLAYER, 10) {
    // water, gold, sonar stocks
    oilStock = getWorld()->numBarrels();
    sonarStock = 1;
    goldStock = 0;
    waterStock = 5;
}
Boulder::Boulder(StudentWorld* w, int initX, int initY) : 
    Actor(w, initX, initY, down, true, IID_BOULDER, 1.0, 1) 
{
    ticksBeforeDrop = 30;
    isFalling = false;
    playsound = false;
}
bool Actor::countTicks(int& ticks) {
    ticks--;
    if (ticks == 0)
        return true;
    return false;
}

//////doSomething functions for actors///////////
void Boulder::doSomething() {
    if (isDead())
        return;
    int x = getX();
    int y = getY();

    if (isFalling == true) {
        //falling state
        if(getWorld()->isBoulderMovableTo(this, x, y))
            moveTo(x, y - 1);
        if (!playsound) {
            getWorld()->playSound(SOUND_FALLING_ROCK);
            playsound = true;
        }
    }
    else if (!getWorld()->canBoulderFall(4, x, y)) {
        //waiting state
        if (countTicks(ticksBeforeDrop)) {
            isFalling = true;
            moveTo(x, y - 1);
        }
    }
}
void SonarKits::doSomething() {
    if (isDead())
        return;
    int x = getX();
    int y = getY();
    Actor* player = getWorld()->getPlayer();
    if (!countTicks(ticks)) {
        if (getWorld()->checkDistance(player, x, y, 3)) {
            setDead();
            getWorld()->playSound(SOUND_GOT_GOODIE);
            getWorld()->incSonarCount();
            getWorld()->increaseScore(75);
            return;
        }
    }
    //setDead after certain ticks.
    else {
        setDead();
    }

}
void WaterPool::doSomething() {
    if (isDead())
        return;
    int x = getX();
    int y = getY();
    Actor* player = getWorld()->getPlayer();
    if (!countTicks(ticks)) {
        if (getWorld()->checkDistance(player, x, y, 3)) {
            setDead();
            getWorld()->playSound(SOUND_GOT_GOODIE);
            getWorld()->increaseScore(100);
            getWorld()->refillWater();
            return;
        }
    }
    else {
        //setDead after certain ticks.
        setDead();
    }
}
void GoldNuggets::doSomething() {
    if (isDead())
        return;
    Actor* player = getWorld()->getPlayer();
    if (!isVisible() && getWorld()->checkDistance(player, getX(), getY(), 4)) {
        setVisible(true);
    } 
    else if (getWorld()->checkDistance(player, getX(), getY(), 3) && !isDropped) {
        setDead();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->increaseScore(10);
        getWorld()->incGoldCount();
    }
    else if (!getWorld()->checkDistance(player, getX(), getY(), 4) && !isRevealed() && !isDropped) {
        setVisible(false);
    }
    else if (isDropped) {
        setVisible(true);
        if (countTicks(ticks))
            setDead();
    }
    Actor* protester;
    if (isDropped && getWorld()->foundPickableGold(this, protester)) {
        setDead();
        protester->setHasPickedUpGold(true);
        getWorld()->increaseScore(25);
    }
}
void Iceman::doSomething() {
    int x = getX();
    int y = getY();
    int ch = KEY_PRESS_DOWN;
    //take user input
    if (getWorld()->getKey(ch) == true)
    {
        switch (ch)
        {
        case KEY_PRESS_ESCAPE:
            setDead();
            break;
        case KEY_PRESS_UP:
            setDirection(up);
            if (getWorld()->isPlayerMovableTo(this, x, y + 1)) {
                getWorld()->breakIce(x, y + 1);
                moveTo(x, y + 1);
            }
            else
                moveTo(x, y);
            break;
        case KEY_PRESS_DOWN:
            setDirection(down);
            if (getWorld()->isPlayerMovableTo(this, x, y - 1)) {
                getWorld()->breakIce(x, y - 1);
                moveTo(x, y - 1);
            }
            else
                moveTo(x, y);
            break;
        case KEY_PRESS_LEFT:
            setDirection(left);
            if (getWorld()->isPlayerMovableTo(this, x - 1, y)) {
                getWorld()->breakIce(x - 1, y);
                moveTo(x - 1, y);
            }
            else
                moveTo(x, y);
            break;
        case KEY_PRESS_RIGHT:
            setDirection(right);
            if (getWorld()->isPlayerMovableTo(this, x + 1, y)) {
                getWorld()->breakIce(x + 1, y);
                moveTo(x + 1, y);
            }
            else
                moveTo(x, y);
            break;
        case'Z':
        case'z':
            if (sonarStock > 0) {
                getWorld()->revealAreaWithSonar();
                getWorld()->playSound(SOUND_SONAR);
                sonarStock--;
            }
            break;
        case KEY_PRESS_TAB:
            if (goldStock > 0) {
                getWorld()->dropGold(x, y);
                goldStock--;
            }
            break;
        case KEY_PRESS_SPACE:
            if (waterStock > 0) {
                shootSquirt(getDirection());
            }
                break;
        default:
            break;
            }
        }
        Actor* a;
        //if there is a boulder that is falling within the range of iceman
        //set the iceman dead.
        if (getWorld()->isThereBoulderNearIceman(a, x, y) && a->isheFalling()) {
            getAnnoyed(10);
            setDead();
        }

        if (getHealth() == 0) {
            //decrease life if dead.
            getWorld()->decLives();
            setDead();
        }
}
void Iceman::shootSquirt(Direction dir) {
    int x = getX();
    int y = getY();
    Squirts* squirt;

    switch (dir) {
    case up:
        squirt = new Squirts(getWorld(), x, y , getDirection());
        squirt->moveTo(x, y + 4);
        break;
    case down:
        squirt = new Squirts(getWorld(), x, y , getDirection());
        squirt->moveTo(x, y - 4);
        break;
    case left:
        squirt = new Squirts(getWorld(), x , y, getDirection());
        squirt->moveTo(x - 4, y);
        break;
    case right:
        squirt = new Squirts(getWorld(), x , y, getDirection());
        squirt->moveTo(x + 4, y);
        break;
    default:
        break;
    }
    getWorld()->pushBackActor(squirt);
    waterStock--;
    getWorld()->playSound(SOUND_PLAYER_SQUIRT);

}
void Squirts::doSomething() {
    int x = getX();
    int y = getY();
    Actor* protester;
    if (isDead())
        return;
    if(travelDistance == 4)
        getWorld()->annoyProtesters(this);
    //setdead if traveled 4 squares OR ran into ice or boulder
    if (travelDistance == 0 || !getWorld()->canSquirtMoveTo(x, y))
        setDead();
    travelDistance--;
}

void OilBarrel::doSomething() {
    if (isDead())
        return;
    Actor* player = getWorld()->getPlayer();
    int x = getX();
    int y = getY();
    if (!isVisible() && getWorld()->checkDistance(player, x, y, 4)) {
        setVisible(true);
    }
    else if(getWorld()->checkDistance(player, x, y, 3)){
        setDead();
        getWorld()->playSound(SOUND_FOUND_OIL);
        getWorld()->increaseScore(1000);
        getWorld()->incBarrelCount();
        return;
    }
    //if its out of range AND if its not yet revealed by sonar, make it invisible.
    else if(!getWorld()->checkDistance(player, x, y, 4) && !isRevealed()){
        setVisible(false);
    }
}