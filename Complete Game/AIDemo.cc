#include "Player.hh"

#include <cmath>

using namespace std;



/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Demo



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
    vector<Dir> dirs;


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

        // initialization
        if (round() == 0) {
            dirs = vector<Dir>(nb_ghosts() + 1);
            for (int i=0; i <= nb_ghosts(); ++i) dirs[i] = rand_dir(None);
        }

        // move pacman
        Pos p = pacman(me()).pos;
        if (not pac_can_move(p, dirs[0]) or rand()%1000==0) dirs[0] = rand_dir(dirs[0]);
        move_my_pacman(dirs[0]);

        // move ghosts
        for (int i=0; i < nb_ghosts(); ++i) {
            Pos q = ghost(me(), i).pos;
            if (not ghost_can_move(q, dirs[i+1]) or rand()%1000==0) dirs[i+1] = rand_dir( dirs[i+1]);
            move_my_ghost(i, dirs[i+1]);
        }
    }


    inline bool pac_can_move (Pos p, Dir d) {
        CType t = cell(dest(p, d)).type;
        if (t == Wall or t == Gate) return false;        
        if (pacman(me()).type == PacMan and cell(dest(p, d)).id != -1) return false;
        return true;
    }
        
    inline bool ghost_can_move (Pos p, Dir d) {
        CType t = cell(dest(p, d)).type;
        if (t == Wall) return false;        
        int r = cell(dest(p, d)).id;
        if (r == -1) return true;
        if (robot(r).type==Ghost) return false;
        return true;
    }
        

    Dir rand_dir (Dir notd) {
        Dir a[] = {Left, Right, Bottom, Top};
        while (true) {
            Dir d = a[rand() % 4];
            if (d != notd) return d;
        }
    }

};



/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);

