#ifndef Board_hh
#define Board_hh

#include "Utils.hh"
#include "PosDir.hh"

using namespace std;




/**
 * Enum type for all possible contents of a cell.
 */

enum CType {
    Empty, Wall, Gate, Dot, Pill, Bonus, Hammer, Mushroom
};


/**
 * Enum type for all possible types of robots.
 */

enum RType {
    PacMan, PowerPacMan, Ghost
};



/**
 * The description of a cell stores its position,
 * its type (Empty|Wall|Gate|Dot|Pill|Bonus|Hammer|Mushroom),
 * and a robot identifier (none when id==-1).
 */

struct Cell {
    Pos pos;
    CType type;
    int id;
};


/**
 * The description of a robot stores its id,
 * its robot type (PacMan|PowerPacMan|Ghost)
 * alive state, player, position and time.
 */

struct Robot {
    int id;
    RType type;
    bool alive;
    int player;
    Pos pos;
    int time;
};


/**
 * Forward declarations.
 */
class Action;


/**
 * The board.
 */

class Board {

    // Allow access to the private part of Board.
    friend class Game;
    friend class SecGame;

    // Game settings
    int nb_rounds_;
    int nb_players_;
    int nb_ghosts_;
    int nb_robots_;
    int regen_time_;
    int power_time_;
    int pn_dot_;
    int pn_pill_;
    int pn_bonus_;
    int pn_ghost_;
    int pn_pacman_;
    int rows_;
    int cols_;
    Pos cage_;
    vector<string> names_;

    // Game state
    int round_;
    vector< vector<Cell> > cells_;
    vector<Robot> robots_;
    vector<int> score_;
    vector<double> status_;     // cpu status. <0: died, 0..1: %of cpu time limit
    set<Pos> cageset_;          // set of positions inside the cage



    /**
     * Construct a board by reading first round from a stream.
     */
    Board (istream& is);

    /**
     * Print the board preamble to a stream.
     */
    void print_preamble (ostream& os) const;

    /**
     * Print the board to a stream.
     */
    void print (ostream& os) const;

    /**
     * Print simplified board to a stream.
     * Just for debug, no interest.
     */
    void print_debug (ostream& os) const;

    /**
     * Returns a reference to the current cpu status of a player.
     */
    inline double& status (int player) {
        return status_[player];
    }

    /**
     * Computes the next board aplying the given actions as to the current board.
     * It also returns the actual actions performed.
     * b0 is the start board.
     */
    Board next (const vector<Action>& as, Action& actions_done, const Board& b0) const;

    /**
     * Applies a basic move.
     */
    bool move (int pl, int id, Dir dir, const Board& b0);



public:

    /**
     * Empty constructor.
     */
    Board ()
    {   }


    /**
     * Returns the number of rounds in the game.
     */
    inline int nb_rounds () const {
        return nb_rounds_;
    }

    /**
     * Returns the number of players in the game.
     */
    inline int nb_players () const {
        return nb_players_;
    }

    /**
     * Returns the number of ghosts for each player in the game.
     */
    inline int nb_ghosts () const {
        return nb_ghosts_;
    }

    /**
     * Returns the total number of robots in the game.
     */
    inline int nb_robots () const {
        return nb_robots_;
    }

    /**
     * Returns the regeneration time (as rounds) in the game.
     */
    inline int regen_time () const {
        return regen_time_;
    }

    /**
     * Returns the power time (as rounds) in the game.
     */
    inline int power_time () const {
        return power_time_;
    }

    /**
     * Returns the number of rows of the maze in the game.
     */
    inline int rows () const {
        return rows_;
    }

    /**
     * Returns the number of columns of the maze in the game.
     */
    inline int cols () const {
        return cols_;
    }

    /**
     * Returns the cage position.
     */
    inline Pos cage () const {
        return cage_;
    }

    /**
     * Returns the current round.
     */
    inline int round () const {
        return round_;
    }

    /**
     * Returns the number of points for each dot.
     */
    inline int pn_dot () const {
        return pn_dot_;
    }

    /**
     * Returns the number of points for each pill.
     */
    inline int pn_pill () const {
        return pn_pill_;
    }

    /**
     * Returns the number of points for each bonus.
     */
    inline int pn_bonus () const {
        return pn_bonus_;
    }

    /**
     * Returns the number of points for each pacman.
     */
    inline int pn_pacman () const {
        return pn_pacman_;
    }

    /**
     * Returns the number of points for each ghost.
     */
    inline int pn_ghost () const {
        return pn_ghost_;
    }

    /**
     * Return whether player is a valid player identifier.
     */
    inline bool player_ok (int player) const {
        return player >= 0 and player < nb_players();
    }

    /**
     * Return whether id is a valid robot identifier.
     */
    inline bool robot_ok (int id) const {
        return id >= 0 and id < nb_robots();
    }

    /**
     * Return whether ghost is a valid ghost identifier.
     */
    inline bool ghost_ok (int ghost) const {
        return ghost >= 0 and ghost < nb_ghosts();
    }

    /**
     * Return whether (i,j) is a position inside the board.
     */
    inline bool pos_ok (int i, int j) const {
        return i >= 0 and i < rows() and j >= 0 and j < cols();
    }

    /**
     * Return whether p is a position inside the board.
     * Does not handle tunnels.
     */
    inline bool pos_ok (const Pos& p) const {
        return pos_ok(p.i, p.j);
    }

    /**
     * Return whether (i,j) + d is a position inside the board.
     * Does not handle tunnels.
     */
    inline bool pos_ok (int i, int j, Dir d) const {
        return pos_ok(Pos(i, j) + d);
    }

    /**
     * Return whether p+d is a position inside the board.
     * Does not handle tunnels.
     */
    inline bool pos_ok (const Pos& p, Dir d) const {
        return pos_ok(p + d);
    }

    /**
     * Returns the position resulting from moving from
     * position p towards direction d taking tunnels into
     * account. Only checks geometry, not walls.
     */
    inline Pos dest (const Pos& p, Dir d) const {
        Pos q = p + d;

             if (q.i == -1) q.i = rows() - 1;
        else if (q.j == -1) q.j = cols() - 1;
        else if (q.i == rows()) q.i = 0;
        else if (q.j == cols()) q.j = 0;

        return q;
    }

    /**
     * Returns the name of a player.
     */
    inline string name (int player) const {
        check(player_ok(player));
        return names_.at(player);
    }

    /**
     * Returns the current score of a player.
     */
    inline int score (int player) const {
        check(player_ok(player));
        return score_.at(player);
    }

    /**
     * Returns the current cpu status of a player.
     * This is a value from 0 (0%) to 1 (100%) or negative if died.
     */
    inline double status (int player) const {
        check(player_ok(player));
        return status_.at(player);
    }

    /**
     * Returns the cell at (i, j).
     */
    inline const Cell& cell (int i, int j) const {
        check(pos_ok(i, j));
        return cells_[i][j];
    }

    /**
     * Returns the cell at p.
     */
    inline const Cell& cell (const Pos& p) const {
        return cell(p.i, p.j);
    }

    /**
     * Returns the robot id.
     */
    inline const Robot& robot (int id) const {
        check(robot_ok(id));
        return robots_.at(id);
    }

    /**
     * Return the pacman of player player.
     */
    inline const Robot& pacman (int player) const {
        check(player_ok(player));
        return robot(player * (nb_ghosts() + 1));
    }

    /**
     * Return the ghost ghost of player player.
     */
    inline const Robot& ghost (int player, int ghost) const {
        check(player_ok(player));
        check(ghost_ok(ghost));
        return robot(player * (nb_ghosts() + 1) + ghost + 1);
    }

};

#endif
