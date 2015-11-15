
#ifndef Action_hh
#define Action_hh

#include "Utils.hh"
#include "PosDir.hh"

using namespace std;




/**
 * Struct that stores a basic movement: robot and direction.
 */

struct Movement {
    int r;
    Dir d;

    Movement (int r, Dir d)
    :   r(r),
        d(d)
    {   }
};



/**
 * Class that stores the actions requested by a player in a round.
 */

class Action {

    friend class Game;
    friend class SecGame;
    friend class Board;

    set<int> u_;
    queue<Movement> q_;

    /**
     * Constructor reading one action from a stream.
     */
    Action (istream& is);

    /**
     * Print the action to a stream.
     */
    void print (ostream& os) const;


public:

    /**
     * Creates an empty action.
     */
    Action ()
    {   }

    /**
     * Moves the pacman of the player in a given direction.
     */
    inline void move_my_pacman (Dir d) {
        if (u_.find(0) == u_.end()) {
            q_.push(Movement(0, d));
            u_.insert(0);
        } else {
            cerr << "warning: action alread requested for this robot." << endl;
    }   }
    

    /**
     * Moves the ghost id of the player in a given direction.
     */
    inline void move_my_ghost (int ghost, Dir d) {
        if (u_.find(1 + ghost) == u_.end()) {
            q_.push(Movement(1 + ghost, d));
            u_.insert(1 + ghost);
        } else {
            cerr << "warning: action alread requested for this robot." << endl;
    }   }

};


#endif
