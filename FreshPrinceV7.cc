#include "Player.hh"

#include <cmath>
#include <list>

using namespace std;



/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME FreshPrinceV7



struct PLAYER_NAME : public Player {


  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  } 

  /**
   * Attributes for your player can be defined here.
   */
  
  //Struct to clasify the bonuses in the priority queue
  struct pVal {
    Pos pos;      //Position of the bonus
    int value;    //Value of the bonus
    int pdist;    //Distance between player and bonus
    CType type;   //Type of bonus
    
    pVal(CType ctype, Pos cpos, int cdist) {
      pos = cpos;
      type = ctype;
      pdist = cdist;
      
      //Depending of the map, the AI will assign different values to the different bonuses
      if(map == 2) {
        switch(ctype) {
          case Hammer:   value = 5; break;
          case Mushroom: value = 2; break;
          case Pill:     value = 3; break;
          case Bonus:    value = 2; break;
          case Dot:      value = 1; break;
          default:       value = 0; break;
        };
      }
      if(map == 1 or map == 7) {
        switch(ctype) {
          case Hammer:   value = 5; break;
          case Pill:     value = 4; break;
          case Bonus:    value = 4; break;
          case Mushroom: value = 2; break;
          case Dot:      value = 1; break;
          default:       value = 0; break;
        };
      }
      else {
        switch(ctype) {
          case Hammer:   value = 5; break;
          case Pill:     value = 4; break;
          case Bonus:    value = 3; break;
          case Mushroom: value = 2; break;
          case Dot:      value = 1; break;
          default:       value = 0; break;
        };
      }
    }
  };
  
  //Comparator function for priority queue
  class ComparePriors {
    public:
      bool operator()(const pVal& p1, const pVal& p2) {
        return (p1.value < p2.value or (p1.value == p2.value and p1.pdist > p2.pdist));
      }
  };
  
  //PacMan Position. Struct to save positions and last direction of pacmans. Is used for the camperMode of the ghosts.
  struct PPos {
    Pos pos;
    Dir dir;
    PPos() { pos = Pos(-1,-1); }
  };
  
  int map;
  vector<vector<int>> dist;
  int defaultDist;
  vector<PPos> pMans; //Vector de posiciones de pacmans con su ultima direccion.Se usa para el camperMode de los fantasmas.
  priority_queue<pVal, vector<pVal>, ComparePriors> P;
  queue<Pos> G; //Ghosts queue
  queue<int> C; //PacMans queue (posicion en el vector pMans)
  queue<Pos> S; //SuperPacMans queue
  queue<Pos> T; //Gates queue
  list<Pos>  H; //Lista de posiciones en las que aparece un Hammer
  
  int mushCount; //Mushroom counter
  int hammCount; //Hammer counter
  int pillCount; //Pill counter
  int bonusCount; //Bonus counter
  
  int timeForPills; //Tiempo a partir del cual empieza a buscar pills
  int timeForMushrooms; //Tiempo minimo para poder coger una seta
  int maxDistHammer; //Distancia maxima al spot de un hammer si hay mushrooms
  
  bool fuckThatShitNigga;

  /**
   * Play method.
   * 
   * This method will be invoked once per each round.
   * You have to read the board here to place your actions
   * for this round.
   *
   * In this example, each robot selects a random direction
   * and follows it as long as it does not hit a wall.
   */     
  virtual void play () {
    //Initializations
    map = mapReference();
    defaultDist = -1;
    dist = vector<vector<int>> (rows(), vector<int>(cols(),defaultDist));
    pMans = vector<PPos> (nb_players(), PPos());
    Pos root, newPos;
    mushCount = 0;
    hammCount = 0;
    pillCount = 0;
    timeForPills = 15;
    timeForMushrooms = 45;
    maxDistHammer = 14;
    
    fuckThatShitNigga = true;
    
    //If im alive, move the pacman
    if(pacman(me()).alive) {
      //If im powerpacman or if that turn is a pair turn, we move (odd turns does not move if you are not powerpacman)
      if(pacman(me()).type == PowerPacMan or (pacman(me()).type == PacMan and round()%2 == 0)) {
        root = pacman(me()).pos;  //Get position
        cleanPacMan();            //Clean all data structures of the pacman
        pacRadar(root);           //Scans all the map since the position of the pacman
        newPos = AI();            //Gets the new position to move (adjacent to the root position)
        moveMe(root, newPos);     //Moves to the new position
      }
    }
      
    //Move ghosts
    for(int i=0; i<nb_ghosts(); ++i) {
      if(ghost(me(), i).alive)
      {
        dist = vector<vector<int>> (rows(), vector<int>(cols(),defaultDist));
        cleanGhosts();                //Clean all data structures of the ghost
        root = ghost(me(), i).pos;    //Get position
        ghostRadar(root);             //Scans all the map since the position of the ghost
        newPos = gAI(i);              //Gets the new position to move (adjacent to the root position)
        moveMyGhost(i, root, newPos); //Moves to the new position
      }
    }
  }
    
  //Decides if the pacman can move to the position calculated by the position p + direction d
  inline bool pac_can_move (Pos p, Dir d) {
    CType t = cell(dest(p, d)).type;
    int id = cell(dest(p, d)).id;
    
    if(t == Wall or t == Gate) return false;
    if(map == 5 and id != -1 and robot(id).type == Ghost) return true;
    if(pacman(me()).type == PacMan and id != -1) return false;
    if(pacman(me()).type == PowerPacMan and id != -1) {
      if(robot(id).type == PacMan or robot(id).type == PowerPacMan) return false;
      if(robot(id).type == Ghost and robot(id).player == me()) return false;
    }
    return true;
  }    
  
  //Decides if the ghost can move to the position calculated by the position p + direction d
  inline bool ghost_can_move (Pos p, Dir d) {
    CType t = cell(dest(p, d)).type;
    int r = cell(dest(p, d)).id;
    
    if(t == Wall) return false;
    if(r == -1) return true;
    if(robot(r).type==Ghost) return false;
    if((robot(r).type==PacMan or robot(r).type==PowerPacMan) and robot(r).player==me()) return false;
    return true;
  }
  
  //Clean all data structures of the pacman
  inline void cleanPacMan() { 
    while(!P.empty()) P.pop();
    while(!G.empty()) G.pop();
    while(!T.empty()) T.pop();
  }
  
  //Clean all data structures of the ghost
  inline void cleanGhosts() {
    while(!C.empty()) C.pop();
    while(!S.empty()) S.pop();
    while(!T.empty()) T.pop();
  }
  
  //Return distance from root to p
  inline int mapDist(Pos p) {
    return dist[p.i][p.j];
  }
  
  //Cleans priority queue P
  inline void cleanP(CType type) {
    while(!P.empty() and P.top().type == type) P.pop();
  }
    
  //Refresh the pMans vector
  void refreshPPos(PPos& ppos, Pos newPos) {
    Pos aux;
    aux.i = newPos.i-ppos.pos.i;
    aux.j = newPos.j-ppos.pos.j;
    if(aux.i == -1 and aux.j == 0) ppos.dir = Top;
    if(aux.i == 1 and aux.j == 0)  ppos.dir = Bottom;
    if(aux.i == 0 and aux.j == 1)  ppos.dir = Right;
    if(aux.i == 0 and aux.j == -1) ppos.dir = Left;
    if(aux.i == 0 and aux.j == 0)  ppos.dir = None;
    ppos.pos = newPos;
  }
  
  //Return the map identifier
  int mapReference() {
    Pos pCage = cage();
    if(pCage == Pos(14, 29)) {
      if(cell(1,29).type == Gate) return 0;//Crises
      else return 1; //Default
    }
    if(pCage == Pos(1, 1))   return 2;//Cotolengo
    if(pCage == Pos(26, 27)) return 3;//Horta
    if(pCage == Pos(2, 28))  return 4;//Joseki
    if(pCage == Pos(23, 29)) return 5;//Manicomi
    if(pCage == Pos(12, 28)) return 6;//Terror
    if(pCage == Pos(23, 26)) return 7;//UPC
    if(pCage == Pos(4, 28))  return 8;//Smile
    return -1;
  }
    
  //Breadth-first search for PacMan
  void pacRadar(Pos p) {
    //Local vars
    CType priors[5] = {Hammer, Pill, Bonus, Mushroom, Dot};
    int maxDist = 999;
    bool dot = false;
    
    //BFS will use the p position to search in the matrix
    queue<Pos> Q; Q.push(p);
    dist[p.i][p.j] = 0;
    
    while(!Q.empty())
    {
      //Get the front position and pops the last
      p = Q.front(); Q.pop();
      
      //Get info from the position p
      Cell c = cell(p); Robot r;
      if(c.id != -1) r = robot(c.id);
      else r.type = PacMan;

      if(round() == 1 and c.type == Hammer) H.insert(H.end(), p);
      if(r.type == Ghost and r.player != me()) G.push(p);
      
      //Scans for bonus
      for(int i=0; i<5; ++i) {
        if(c.type == priors[i]) { //If the position contains a bonus...
          if(c.type == Dot) { //Adds a dot...
            if(!dot) {  //only if it is the nearest to the pacman (more dots would be unnecessary)
              P.push(pVal(c.type, p, dist[p.i][p.j]));
              dot = true;
            }
          }
          else { //Adds the bonus
            P.push(pVal(c.type, p, dist[p.i][p.j]));
            
            //Bonus counters
            if(c.type == Pill) ++pillCount;
            if(c.type == Hammer)   ++hammCount;
            if(c.type == Mushroom) ++mushCount;
            if(c.type == Bonus) ++bonusCount;
          }
        }
      }

      //Scans adjacent positions
      for(int i=1; i<5; ++i)
      {
        Pos aux = dest(p, (Dir)i);
        if(pos_ok(aux) and dist[aux.i][aux.j] == defaultDist) //If it is a correct position...
        {
          if(!pac_can_move(p, (Dir)i)) dist[aux.i][aux.j] = maxDist;
          else { Q.push(aux); dist[aux.i][aux.j] = dist[p.i][p.j]+1; } //If the position is valid, we add it to the queue and refresh distance
        }
      }
    }
  }
    
  //Artificial Intelligence by maps
  Pos AI() {
    switch(map) {
      case 0: return pacDefaultMap();   //Crises
      case 1: return pacDefaultMap();   //Default
      case 2: return pacCotolengoMap(); //Cotolengo
      case 3: return pacDefaultMap();   //Horta
      case 4: return pacDefaultMap();   //Joseki
      case 5: return pacManicomiMap();  //Manicomi
      case 6: return pacTerrorMap();    //Terror
      case 7: return pacUPCMap();       //UPC
      case 8: return pacDefaultMap();   //Smile
    }
    return pacDefaultMap();
  }
    
  //Map
  Pos pacDefaultMap() {
    if(pacman(me()).type == PowerPacMan) { //If i'm power pacman (Huh!)
      if(!P.empty() and P.top().type == Hammer) return P.top().pos; //Get all hammers
      if(needPills()) return P.top().pos; //Get pills if necessary
      if(canKillGhost()) return G.front(); //Goes to kill ghost if it's possible to get it
      else if(!P.empty()) return P.top().pos; //Nothing to do here... take anything
    }
    if(pacman(me()).type == PacMan and round()%2 == 0 and !P.empty()) {  //If im a pacman
      if(!P.empty() and P.top().type == Hammer) { //If a hammer is near the pacman
        if(mapDist(P.top().pos) <= 2) return P.top().pos; //(Near is 2 positions of distance ^^) goes for it
        else cleanP(Hammer);  //bye hammer
      }
      if(!P.empty()) return P.top().pos; //Anything
    }
    return Pos(-1,-1);
  }
  
  //Map
  Pos pacManicomiMap() {
    if(pacman(me()).pos.i > 20) return suicide(); //Yes, you read it well: SUICIDE!
    else {
      if(mushCount > 0) return Pos(11,11);
      else return P.top().pos;
    }
    /*
     * I explain why I did that: That was a map that if you appear in pos.i > 20, you can't do ANYTHING. 
     * So, I prefer to suicide before waste time and lost the game xD
    */
  }
  
  //Map
  Pos pacCotolengoMap() {
    if(pacman(me()).type == PowerPacMan) { //If i'm power pacman (Huh!)
      if(mapDist(Pos(1,1)) != -1 and !G.empty()) return G.front();
      if(!P.empty() and P.top().type == Hammer) return P.top().pos; //Get all hammers
      if(needPills()) return P.top().pos; //Get pills if necessary
      if(canKillGhost()) return G.front(); //Goes to kill ghost if it's possible to get it
      else if(!P.empty())  return P.top().pos; //Nothing to do here... take anything
    }
    if(pacman(me()).type == PacMan and round()%2 == 0 and !P.empty()) { //If im a pacman
      if(!P.empty() and P.top().type == Hammer) {  //If a hammer is near the pacman
        if(mapDist(P.top().pos) <= 2) return P.top().pos; //(Near is 2 positions of distance ^^) goes for it
        else cleanP(Hammer); //bye hammer
      }
      if(!P.empty()) return P.top().pos; //Anything
    }
    return Pos(-1,-1);
  } 
  
  //So similar to the other strategies
  Pos pacTerrorMap() {
    if(pacman(me()).type == PowerPacMan)
    {
      if(!P.empty() and P.top().type == Hammer) return P.top().pos;
      if(needPills()) return P.top().pos;
      if(canKillGhost()) return G.front();
      else if(!P.empty()) return P.top().pos;
    }
    if(pacman(me()).type == PacMan and round()%2 == 0 and !P.empty()) {
      if(!P.empty() and P.top().type == Hammer) { 
        if(mapDist(P.top().pos) <= 8) return P.top().pos;
        else cleanP(Hammer);
      }
      if(!P.empty()) return P.top().pos;
    }
    return Pos(-1,-1);
  }
  
  //So similar to the other strategies
  Pos pacUPCMap() {
    if(pacman(me()).type == PowerPacMan and !P.empty()) {
      if(!P.empty() and P.top().type == Hammer) {
        if(mapDist(P.top().pos) < 30) return P.top().pos;
        else P.pop();
      }
      if(canKillGhost())  G.front();

      return P.top().pos;
    }
    if(pacman(me()).type == PacMan and round()%2 == 0 and !P.empty()) {
      if(!P.empty() and P.top().type == Hammer) {
        if(mapDist(P.top().pos) < 30) return P.top().pos;
        else P.pop();
      }
      return P.top().pos;
    }
    return Pos(-1,-1);
  }
    
  //Suicide function. Goes direct to a ghost
  inline Pos suicide() {
    if(!G.empty()) return G.front();
    return Pos(27,29);
  }
  
  //Function that evaluate if the pacman needs a pill (that doesn't sound so good...)
  inline bool needPills() {
    return (!P.empty() and pacman(me()).time < timeForPills and P.top().type == Pill);
  }
  
  //Function that evaluates if the pacman can take a hammer before another pacman
  bool canTakeHammer() {
    while(!P.empty() and P.top().type == Hammer) {
      int enemyTime, myTime = pacman(me()).time;
      int enemyDist = hammerRadar(P.top().pos, enemyTime), myDist = mapDist(P.top().pos);

      //If I arrive to the hammer before another pacman, go for it
      return (myTime >= myDist and (enemyDist == -1 or (enemyTime < enemyDist) or (enemyTime >= enemyDist and enemyDist >= myDist)))
      P.pop();
    }
    return false;
  }
    
  //Returns distance between the nearest powerpacman and the hammer and his power time (Breadth-first search)
  int hammerRadar(Pos p, int& time) {
    int maxDist = 999;
    vector<vector<int>> eDist(rows(), vector<int>(cols(), defaultDist));
    time = -1;

    queue<Pos> Q; Q.push(p);
    eDist[p.i][p.j] = 0;
    while(!Q.empty()) {
      p = Q.front(); Q.pop();

      Cell c = cell(p); Robot r;
      if(c.id != -1) r = robot(c.id);
      else r.type = PacMan;

      if(r.type == PowerPacMan and r.player != me()) { time = robot(cell(p).id).time; return eDist[p.i][p.j]; }

      //Search in adjacent positions
      for(int i=1; i<5; ++i) {
        Pos aux = dest(p, (Dir)i);
        if(pos_ok(aux) and eDist[aux.i][aux.j] == defaultDist) {
          if(!ghost_can_move(p, (Dir)i)) eDist[aux.i][aux.j] = maxDist;
          else { Q.push(aux); eDist[aux.i][aux.j] = eDist[p.i][p.j]+1; } //Si es walkable, la metemos en la cola y actualizamos distancia
        }
      }
    }
    return -1;
  }
    
  //Decides if the pacman can kill a ghost by different parameters
  bool canKillGhost() {
    while(pacman(me()).type == PowerPacMan and !G.empty()) {
      Pos aux = G.front();
      int myDistance = dist[aux.i][aux.j];
      if(myDistance*2 < pacman(me()).time) { //If i can catch a ghost...
        //If distance between ghost and his nearest door is half or more than the powerpacman to the ghost, returns true
        int gateDist = -1, powerPacManDist = -1, powerPacPlayer = -1;
        enemyGhostSalvationDistance(aux, gateDist, powerPacManDist, powerPacPlayer);

        if(gateDist == -1 or gateDist*2 > dist[aux.i][aux.j]) //If there are not avaliable gates or i'm faster...
          if(powerPacManDist == -1 or (powerPacManDist*2 > myDistance*2) or !(powerPacManDist*2 < pacman(powerPacPlayer).time)) //If there are any powerpacman near the ghost that can catch him before...
            return true;
      }
      G.pop();
    }
    return false;
  }
    
  //Modifies gateDist and powerPacManDist with the distance of both nearest items, if exists. powerPacPlayer returns the player of the powerpacman, if exists
  void enemyGhostSalvationDistance(Pos p, int& gateDist, int& powerPacManDist, int& powerPacPlayer) {
    int maxDist = 999;
    vector<vector<int>> eDist(rows(), vector<int>(cols(), defaultDist));

    Cell c = cell(p); Robot r = robot(c.id);
    int gPlayer = r.player;

    queue<Pos> Q; Q.push(p);
    eDist[p.i][p.j] = 0;
    bool gateF = false, powerF = false;
    while(!Q.empty() and !gateF and !powerF) {
      p = Q.front(); Q.pop();

      c = cell(p);
      if(c.id != -1) r = robot(c.id);
      else r.type = PacMan;

      if(c.type == Gate and !gateF) { gateF = true; gateDist = eDist[p.i][p.j]; }
      if(r.type == PowerPacMan and !powerF and r.player != me() and r.player != gPlayer) { powerF = true; powerPacManDist = eDist[p.i][p.j]; powerPacPlayer = r.player; }

      for(int i=1; i<5 and !gateF and !powerF; ++i) {
        Pos aux = dest(p, (Dir)i);
        if(pos_ok(aux) and eDist[aux.i][aux.j] == defaultDist) {
          if(!ghost_can_move(p, (Dir)i)) eDist[aux.i][aux.j] = maxDist;
          else { Q.push(aux); eDist[aux.i][aux.j] = eDist[p.i][p.j]+1; }
        }
      }
    }
  }
    
  //Function that moves the pacman
  void moveMe(const Pos& rootPos, Pos p)
  {
    //Using the distances matrix we will get the new direction that the pacman will use to go to the objective p.
    int dst = (p.i == -1 and p.j == -1) ? 1 : dist[p.i][p.j];
    while(dst != 1)
    {
      //Compares the distances of the adjacent positions and gets the nearest position
      Pos pTop = dest(p, Top), pBottom = dest(p, Bottom), pRight = dest(p, Right), pLeft = dest(p, Left);

      Pos s1;
      if(pos_ok(pTop) and pos_ok(pBottom))
        s1 = (dist[pTop.i][pTop.j] < dist[pBottom.i][pBottom.j]) ? pTop : pBottom; //Compares top and bottom
      else if(pos_ok(pTop) and !pos_ok(pBottom))
        s1 = pTop;
      else if(!pos_ok(pTop) and pos_ok(pBottom))
        s1 = pBottom;

      Pos s2;
      if(pos_ok(pRight) and pos_ok(pLeft))
        s2 = (dist[pRight.i][pRight.j] < dist[pLeft.i][pLeft.j]) ? pRight : pLeft; //Compares right and left
      else if(pos_ok(pRight) and !pos_ok(pLeft))
        s2 = pRight;
      else if(!pos_ok(pRight) and pos_ok(pLeft))
        s2 = pLeft;

      p = (dist[s1.i][s1.j] <= dist[s2.i][s2.j]) ? s1 : s2; //Compares the two positions selected before
      dst = dist[p.i][p.j];
      if(dst == -1) dst = 1;
    }
    
    //Get the direction that our pacman will move
    Dir dir = None;
    if(dest(rootPos,Top) == p)    dir = Top;
    if(dest(rootPos,Bottom) == p) dir = Bottom;
    if(dest(rootPos,Right) == p)  dir = Right;
    if(dest(rootPos,Left) == p)   dir = Left;
    move_my_pacman(dir);
  }
   
  //Same that the other BFS on the AI
  void ghostRadar(Pos p)
  { 
    int defaultDist = -1;
    int maxDist = 999;
    
    queue<Pos> Q; Q.push(p);
    dist[p.i][p.j] = 0;
    
    while(!Q.empty()) {
      p = Q.front(); Q.pop();

      Cell c = cell(p); Robot r;
      if(c.id != -1) r = robot(c.id);
      else r.type = Ghost;

      if(r.type == PacMan and r.player != me()) {
        refreshPPos(pMans[r.player], p);
        C.push(r.player);
      }
      if(r.type == PowerPacMan and r.player != me()) {
        refreshPPos(pMans[r.player], p);
        S.push(p);
      }
      if(c.type == Gate) T.push(p);

      for(int i=1; i<5; ++i) {
        Pos aux = dest(p, (Dir)i);
        if(pos_ok(aux) and dist[aux.i][aux.j] == defaultDist) {
          if(!ghost_can_move(p, (Dir)i)) dist[aux.i][aux.j] = maxDist;
          else { Q.push(aux); dist[aux.i][aux.j] = dist[p.i][p.j]+1; }
        }
      }
    }
  }
    
  //Ghost artificial intelligence
  Pos gAI(int i)
  {
    switch(map)
    {
      case 0: return defaultGhost(i);    //Crises
      case 1: return ghostDefaultMap(i); //Default
      case 2: return defaultGhost(i);    //Cotolengo
      case 3: return defaultGhost(i);    //Horta
      case 4: return ghostJosekiMap(i);  //Joseki
      case 5: return defaultGhost(i);    //Manicomi
      case 6: return ghostTerrorMap(i);  //Terror
      case 7: return defaultGhost(i);    //UPC
      case 8: return defaultGhost(i);    //Smile
    }
    return ghostDefaultMap(i);
  }
    
  //Default strategy for a ghost
  Pos defaultGhost(int i) {
    if(!C.empty() and mapDist(pMans[C.front()].pos) == 1) return pMans[C.front()].pos;
    if(!S.empty() and !T.empty()) { //WARNING! DANGER!
      Pos aux = S.front();
      if(fuckThatShitNigga and hammCount == 0 and dist[aux.i][aux.j] > 4) return fuckTheSystemMode(i); //That's my secret weapon! :D Literally, blocks the enemies ^^
      else return defaultCamperMode(i); //Stays on a gate, waiting to will a pacman that moves near
    }
    else if(!C.empty()) return agressiveMode(i);
    else return Pos(-1,-1);
  }
  
  //Ghost map strategy
  Pos ghostDefaultMap(int i) {
    int vPosSize = 12;
    int followAtDistance = 1;
    Pos vPos[12] = {Pos(12,27), Pos(12,28), Pos(12,29), Pos(12,30), Pos(12,31), Pos(14,25), 
       Pos(14,33), Pos(16,27), Pos(16,28), Pos(16,29), Pos(16,30), Pos(16,31)}; //Positions to camp
    if(round() > 21) {
      if(!C.empty() and mapDist(pMans[C.front()].pos) == 1) return pMans[C.front()].pos;
      if(!S.empty() and !T.empty()) { //WARNING! DANGER! 
        if(fuckThatShitNigga and hammCount == 0 and mapDist(S.front()) > 4) return fuckTheSystemMode(i);
        else return camperMode(i, vPos, vPosSize, followAtDistance);
      }
      else if(!C.empty()) return agressiveMode(i);
      else return Pos(-1,-1);
    }
    else return Pos(-1,-1);
  }
  
//Ghost map strategy
  Pos ghostTerrorMap(int i) {
    int vPosSize = 2;
    int followAtDistance = 1;
    Pos vPos[2] = {Pos(22,25), Pos(22,29)/*, Pos(11,31), Pos(11, 23)*/}; //Posiciones para campear
    
    if(!C.empty() and mapDist(pMans[C.front()].pos) == 1) return pMans[C.front()].pos;
    if(!S.empty() and !T.empty()) { //WARNING! DANGER! 
      if(fuckThatShitNigga and hammCount == 0 and mapDist(S.front()) > 4) return fuckTheSystemMode(i);
      else return camperMode(i, vPos, vPosSize, followAtDistance);
    }
    else if(!C.empty()) return agressiveMode(i);
    else if(!T.empty()) return camperMode(i, vPos, vPosSize, followAtDistance);
    
    return Pos(-1,-1);
  }
  
  //Ghost map strategy
  Pos ghostJosekiMap(int i) {
    if(round() < 8 or (round() < 33 and pacman(me()).type == PowerPacMan)) return Pos(-1,-1);
    
    if(!C.empty() and mapDist(pMans[C.front()].pos) == 1) return pMans[C.front()].pos;
    if(!S.empty() and !T.empty()) { //WARNING! DANGER! 
      Pos aux = S.front();
      if(fuckThatShitNigga and hammCount == 0 and dist[aux.i][aux.j] > 4) return fuckTheSystemMode(i);
      else return defaultCamperMode(i);
    }
    else if(!C.empty()) return agressiveMode(i);
    return Pos(-1,-1);
  }
  
  //FATALITY. HELL IS HERE.
  Pos fuckTheSystemMode(int i) {
    Pos p = ghost(me(), i).pos;
    if(cell(p).type == Gate) { //If ghost is on a gate, he goes out
      if(!S.empty() and mapDist(S.front()) > 4) return S.front();
      return Pos(-1,-1);
    }
    return T.front(); //else, he goes to the gate again
    /*
     *  That makes that if people programmed his pacman like: 
     *      "If i'm power pacman, go directly to the ghosts" (90% of the other players)
     *  and im his objetive, they will be blocked (pacmans cant kill ghosts if they are on a gate)
    */
  }
    
  //Camper Mode: Stands on a gate until a pacman moves near, then the ghost attacks
  Pos defaultCamperMode(int i){ 
    Pos p = ghost(me(), i).pos;
    if(cell(p).type == Gate) {
      if(!C.empty()) {
        if(mapDist(pMans[C.front()].pos) == 1) return pMans[C.front()].pos;
        Pos aux = dest(pMans[C.front()].pos, pMans[C.front()].dir);
        if(dist[aux.i][aux.j] == 1) return aux;
      }
      return Pos(-1,-1);
    }
    return T.front();
  }
  
  //Another camper mode
  Pos camperMode(int i, Pos vPos[], int size, int distance){
    Pos aux = Pos(1, 1);
    for(int j=0; j<size; ++j) {
      if(ghost(me(), i).pos == vPos[j]) {
        if(!C.empty()) {
          Pos p = dest(pMans[C.front()].pos, pMans[C.front()].dir);
          if(mapDist(p) <= distance) return p;
        }
        return Pos(-1,-1);
      }
      if(mapDist(vPos[j]) < mapDist(aux) and cell(vPos[j]).id == -1 ) aux = vPos[j]; //Search for the nearest gate
    }
    //aux = Pos(1, 1) means no objective gates avaliable, so we move to the nearest gate avaliable or if we are already on a gate, we stay here
    if(aux == Pos(1,1)) {
      if(cell(ghost(me(), i).pos).type == Gate) return Pos(-1,-1);
      return T.front();
    }
    return aux;
  }
  
  //Wao, so much complexity. Goes directly to a pacman
  inline Pos agressiveMode(int i) {
    return pMans[C.front()].pos;
  }
    
  //Same that moveMe
  void moveMyGhost(int i, const Pos& rootPos, Pos p) {
    int dst = (p.i == -1 and p.j == -1) ? 1 : dist[p.i][p.j];
    while(dst != 1) {
      Pos pTop = dest(p, Top), pBottom = dest(p, Bottom), pRight = dest(p, Right), pLeft = dest(p, Left);

      Pos s1;
      if(pos_ok(pTop) and pos_ok(pBottom))
        s1 = (dist[pTop.i][pTop.j] < dist[pBottom.i][pBottom.j]) ? pTop : pBottom; //compares top and bottom
      else if(pos_ok(pTop) and !pos_ok(pBottom))
        s1 = pTop;
      else if(!pos_ok(pTop) and pos_ok(pBottom))
        s1 = pBottom;

      Pos s2;
      if(pos_ok(pRight) and pos_ok(pLeft))
        s2 = (dist[pRight.i][pRight.j] < dist[pLeft.i][pLeft.j]) ? pRight : pLeft; //compares right and left
      else if(pos_ok(pRight) and !pos_ok(pLeft))
        s2 = pRight;
      else if(!pos_ok(pRight) and pos_ok(pLeft))
        s2 = pLeft;

      p = (dist[s1.i][s1.j] < dist[s2.i][s2.j]) ? s1 : s2; //compares the positions selected before
      dst = dist[p.i][p.j];
      if(dst == -1) dst = 1;
    }
    
    //Get the new direction
    Dir dir = None;
    if(dest(rootPos,Top) == p)    dir = Top;
    if(dest(rootPos,Bottom) == p) dir = Bottom;
    if(dest(rootPos,Right) == p)  dir = Right;
    if(dest(rootPos,Left) == p)   dir = Left;
    move_my_ghost(i, dir);
  }
};



/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);

