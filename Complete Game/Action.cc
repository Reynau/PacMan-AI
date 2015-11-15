#include "Action.hh"
#include "Board.hh"


using namespace std;


Action::Action (istream& is) {

    // Warning: all read operations must be checked for SecGame.
    
    u_ = set<int>();
    q_ = queue<Movement>();
    string s;
    if (getline(is, s)) {
        istringstream iss(s);    
        int r;
        iss >> r;
        while (r != -1) {
            char c;
            iss >> c;
            q_.push(Movement(r, c2d(c)));
            u_.insert(r);
            iss >> r;        
}   }   }


void Action::print (ostream& os) const {
    queue<Movement> qq = q_;
    while (not qq.empty()) {
        Movement m = qq.front(); qq.pop();
        os << m.r << " " << d2c(m.d) << " ";
    }
    os << -1 << endl;
}

