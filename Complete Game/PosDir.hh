#ifndef PosDir_hh
#define PosDir_hh


#include <cstdlib>
#include "Utils.hh"

using namespace std;


/**
 * Enum to encode directions.
 */
enum Dir {
    None, Top, Bottom, Left, Right
};


/**
 * Conversion from Dir to char.
 */
inline char d2c (Dir d) {
    switch (d) {
        case None:      return 'n';
        case Top:       return 't';
        case Bottom:    return 'b';
        case Left:      return 'l';
        case Right:     return 'r';
    }
    _unreachable();
}


/**
 * Conversion from char to Dir.
 */
inline Dir c2d (char c) {
    switch (c) {
        case 'n':   return None;
        case 't':   return Top;
        case 'b':   return Bottom;
        case 'l':   return Left;
        case 'r':   return Right;
    }
    _unreachable();
}


/**
 * Simple struct to handle positions.
 */
struct Pos {

    /**
     * Plain old data.
     */
    int i, j;


    /**
     * Default constructor (0,0).
     */
    inline Pos ()
    :   i(0),
        j(0)
    {   }

    /**
     * Given constructor.
     */
    inline Pos (int i, int j)
    :   i(i),
        j(j)
    {   }

    /**
     * Put operator .
     */
    inline friend ostream& operator<< (ostream& os, const Pos& p) {
        return os << "(" << p.i << "," << p.j << ")";
    }

    /**
     * Comparison operator.
     */
    inline friend bool operator< (const Pos& a, const Pos& b) {
        if (a.i != b.i) return a.i < b.i;
        return a.j < b.j;
    }

    /**
     * Comparison operator.
     */
    inline friend bool operator== (const Pos& a, const Pos& b) {
        return a.i == b.i and a.j == b.j;
    }

    /**
     * Comparison operator.
     */
    inline friend bool operator!= (const Pos& a, const Pos& b) {
        return not (a == b);
    }

    /**
     * Comparison operator.
     */
    inline friend bool operator<= (const Pos& a, const Pos& b) {
        return a < b or a == b;
    }

    /**
     * Comparison operator.
     */
    inline friend bool operator>= (const Pos& a, const Pos& b) {
        return not (a < b);
    }

    /**
     * Comparison operator.
     */
    inline friend bool operator> (const Pos& a, const Pos& b) {
        return not (a <= b);
    }

    /**
     * Increment operator: moves a position according to a direction.
     */
    inline Pos& operator+= (Dir d) {
        switch (d) {
            case None:           break;
            case Top:       --i; break;
            case Bottom:    ++i; break;
            case Left:      --j; break;
            case Right:     ++j; break;
        }
        return *this;
    }

    /**
     * Addition operator: Returns a position by adding a direction.
     */
    inline Pos operator+ (Dir d) const {
        switch (d) {
            case None:      return Pos(i    , j    );
            case Top:       return Pos(i - 1, j    );
            case Bottom:    return Pos(i + 1, j    );
            case Left:      return Pos(i    , j - 1);
            case Right:     return Pos(i    , j + 1);
        }
        _unreachable();
    }

};


#endif
