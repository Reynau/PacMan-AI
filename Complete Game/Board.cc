#include <algorithm>

#include "Board.hh"
#include "Action.hh"

#include <cctype>

using namespace std;



Board::Board (istream& is) {
    string s;

    string version;
    is >> s >> version;
    assert(s == "pacman");
    assert(version == "v1");

    is >> s >> nb_players_;
    assert(s == "nb_players");
    assert(nb_players_ >= 1);

    is >> s >> nb_rounds_;
    assert(s == "nb_rounds");
    assert(nb_rounds_ >= 1);

    is >> s >> nb_ghosts_;
    assert(s == "nb_ghosts");
    assert(nb_ghosts_ >= 0);

    is >> s >> regen_time_;
    assert(s == "regen_time");
    assert(regen_time_ >= 1);

    is >> s >> power_time_;
    assert(s == "power_time");
    assert(power_time_ >= 1);

    is >> s >> pn_dot_;
    assert(s == "pn_dot");
    assert(pn_dot_ >= 1);

    is >> s >> pn_pill_;
    assert(s == "pn_pill");
    assert(pn_pill_ >= 1);

    is >> s >> pn_bonus_;
    assert(s == "pn_bonus");
    assert(pn_bonus_ >= 1);

    is >> s >> pn_ghost_;
    assert(s == "pn_ghost");
    assert(pn_ghost_ >= 1);

    is >> s >> pn_pacman_;
    assert(s == "pn_pacman");
    assert(pn_pacman_ >= 1);

    is >> s >> cage_.i >> cage_.j;
    assert(s == "cage");

    is >> s >> rows_;
    assert(s == "rows");
    assert(rows_ >= 1);

    is >> s >> cols_;
    assert(s == "cols");
    assert(cols_ >= 1);

    names_ = vector<string>(nb_players_);
    nb_robots_ = nb_players_ * (1 + nb_ghosts_);
    cells_ = vector< vector<Cell> >(rows_, vector<Cell>(cols_));
    robots_ = vector<Robot>(nb_robots_);

    is >> s;
    assert(s == "names");
    for (int pl = 0; pl < nb_players_; ++pl) {
        is >> names_[pl];
    }

    is >> s >> round_;
    assert(s == "round");
    assert(round_ >= 0);
    assert(round_ < nb_rounds_);

    for (int i = 0; i < rows_; ++i) {
        for (int j = 0; j < cols_; ++j) {
            cells_[i][j].id = -1;
            cells_[i][j].pos = Pos(i, j);
            char c;
            is >> c;
            c = toupper(c);
            //cerr << c;
            switch (c) {
                case '-': cells_[i][j].type = Empty;   break;
                case 'X': cells_[i][j].type = Wall;    break;
                case '#': cells_[i][j].type = Gate;    break;
                case '.': cells_[i][j].type = Dot;     break;
                case '*': cells_[i][j].type = Pill;    break;
                case 'B': cells_[i][j].type = Bonus;   break;
                case 'C': cells_[i][j].type = Hammer;   break;
                case 'M': cells_[i][j].type = Mushroom; break;
                default:
                    assert(false);
                    _unreachable();
        }   }
        //cerr << endl;
    }

    is >> s;
    assert(s == "score");
    score_ = vector<int>(nb_players_);
    for (int i = 0; i < nb_players_; ++i) {
        is >> score_[i];
    }

    is >> s;
    assert(s == "status");
    status_ = vector<double>(nb_players_);
    for (int i = 0; i < nb_players_; ++i) {
        is >> status_[i];
    }

    for (int id = 0; id < nb_robots_; ++id) {
        int i, j, time;
        char status;
        is >> s >> i >> j >> time >> status;
        //cerr << i << " " << j << endl;
        assert(pos_ok(i, j));
        assert(status == 'a' or status == 'd');

        RType type;
             if (s == "pacman")         type = PacMan;
        else if (s == "ghost")          type = Ghost;
        else if (s == "powerpacman")    type = PowerPacMan;
        else assert(false);

        robots_[id].id = id;
        robots_[id].type = type;
        robots_[id].alive = status == 'a';
        robots_[id].player = id / (nb_ghosts_ + 1);
        robots_[id].pos = Pos(i, j);
        robots_[id].time = time;

        cells_[i][j].id = id;

        if (round_ == 0 and (type == PacMan or type == PowerPacMan)) {
            cells_[i][j].type = Empty;
        }
    }


    // compute cageset_
    cageset_.insert(cage_);
    while (true) {
        set<Pos> old = cageset_;
        for (set<Pos>::iterator it = old.begin(); it != old.end(); ++it) {
            Pos p1 = *it + Left;     if (cell(p1).type == Empty) cageset_.insert(p1);
            Pos p2 = *it + Right;    if (cell(p2).type == Empty) cageset_.insert(p2);
            Pos p3 = *it + Top;      if (cell(p3).type == Empty) cageset_.insert(p3);
            Pos p4 = *it + Bottom;   if (cell(p4).type == Empty) cageset_.insert(p4);
        }
        if (cageset_ == old) break;
    }
}



void Board::print_preamble (ostream& os) const {
    os << "pacman v1" << endl;
    os << "nb_players " << nb_players() << endl;
    os << "nb_rounds " << nb_rounds() << endl;
    os << "nb_ghosts " << nb_ghosts() << endl;
    os << "regen_time " << regen_time() << endl;
    os << "power_time " << power_time() << endl;
    os << "pn_dot " << pn_dot() << endl;
    os << "pn_pill " << pn_pill() << endl;
    os << "pn_bonus " << pn_bonus() << endl;
    os << "pn_ghost " << pn_ghost() << endl;
    os << "pn_pacman " << pn_pacman() << endl;
    os << "cage " << cage().i << " " << cage().j << endl;
    os << "rows " << rows() << endl;
    os << "cols " << cols() << endl;
    os << "names";
    for (int player = 0; player < nb_players(); ++player) os << " " << name(player);
    os << endl << endl;
}



void Board::print (ostream& os) const {
    os << endl;
    os << "round " << round() << endl;
    for (int i = 0; i < rows_; ++i) {
        for (int j = 0; j < cols_; ++j) {
            CType c = cell(i, j).type;
                 if (c == Empty) os << '-';
            else if (c == Wall ) os << 'X';
            else if (c == Gate ) os << '#';
            else if (c == Dot  ) os << '.';
            else if (c == Pill ) os << '*';
            else if (c == Bonus) os << 'B';
            else if (c == Hammer) os << 'C';
            else if (c == Mushroom) os << 'M';
            else assert(0);
        }
        os << endl;
    }
    os << "score";
    for (int i = 0; i < nb_players(); ++i) os << " " << score(i);
    os << endl;
    os << "status";
    for (int i = 0; i < nb_players(); ++i) os << " " << status(i);
    os << endl;
    for (int id = 0; id < nb_robots(); ++id) {
        string s;
        if (robot(id).type == PacMan) s = "pacman";
        else if (robot(id).type == PowerPacMan) s = "powerpacman";
        else if (robot(id).type == Ghost) s = "ghost";
        else assert(0);
        os  << s << " "
            << robot(id).pos.i << " "
            << robot(id).pos.j << " "
            << robot(id).time << " "
            << (robot(id).alive ? 'a' : 'd') << endl;
    }
    os << endl;
}




void Board::print_debug (ostream& os) const {
    os << endl;
    os << "round " << round() << endl;
    for (int i = 0; i < rows_; ++i) {
        for (int j = 0; j < cols_; ++j) {
            if (cell(i, j).type == Wall) {
                os << 'X';
            } else if (cell(i, j).type == Gate) {
                os << '#';
            } else {
                int id = cell(i, j).id;
                if (id != -1) {
                    os << char('a'+id);
                } else {
                    os << '.';
        }   }   }
        os << endl;
    }
    os << endl;
}




Board Board::next (const vector<Action>& as, Action& actions_done, const Board& b0) const {

    // b will be the new board we shall return
    Board b = *this;

    // increment the round
    ++b.round_;

    // randomize turns
    vector<int> turns(nb_players());
    for (int player = 0; player < nb_players(); ++player) {
        turns[player] = player;
    }
    random_shuffle(turns.begin(), turns.end());

    // move each robot
    vector<bool> moved(nb_robots(), false);
    for (int turn = 0; turn < nb_players(); ++turn) {
        int player = turns[turn];
        queue<Movement> q = as[player].q_;
        while (not q.empty()) {
            Movement m = q.front();  q.pop();
            int lid = m.r;
            Dir dir = m.d;
            if (lid < 0 or lid > nb_ghosts() + 1) continue;
            if (dir != None and dir != Left and dir != Right and dir != Top and dir != Bottom) continue;
            int id = player * (1 + nb_ghosts()) + lid;
            if (moved[id]) continue;
            moved[id] = true;
            bool ok = b.move(player, id, dir, b0);
            if (ok) {
                actions_done.q_.push(Movement(id, dir));
    }   }   }

    // decrement times
    for (int id = 0; id < nb_robots(); ++id) {
        Robot& r = b.robots_[id];

        // check for end of power time
        if (r.alive and r.type == PowerPacMan) {
            if (--r.time <= 0) r.type = PacMan;
        }

        // check for end of regeneration time of ghosts
        if (not r.alive and r.type == Ghost and b.status(r.player) >= 0) {
            if (--r.time <= 0) {
                bool fnd = false;
                vector<Pos> v;
                for (set<Pos>::iterator it = b.cageset_.begin();  it != b.cageset_.end(); ++it) {
                    v.push_back(*it);
                }
                cerr << v.size() << endl;
                random_shuffle(v.begin(), v.end());
                
                for (int i = 0; i <int(v.size()) and not fnd; ++i) {
                    if (b.cell(v[i]).id == -1) {
                        b.cells_[v[i].i][v[i].j].id = id;
                        b.robots_[id].pos = v[i];
                        r.alive = true;
                        fnd = true;
                }   }
                if (not fnd) cerr << "info: cannot place ghost in cage." << endl;
            }
        }
        
        // check for end of regeneration time of pacman
        if (not r.alive and r.type == PacMan and b.status(r.player) >= 0) {
            if (--r.time <= 0) {
                bool fnd = false;

                for (int reps = 0; reps < 100 and not fnd; ++reps) {
                    Pos p = Pos(random() % rows(), random() % cols());
                    fnd = b.cell(p).type == Empty;
                    for (int ii = -2; ii <= 2; ++ii) {
                        for (int jj = -2; jj <= 2; ++jj) {
                            fnd = fnd and (not b.pos_ok(p.i + ii, p.j + jj) or b.cell(p.i + ii, p.j + jj).id == -1);
                    }   }
                    if (fnd) {
                        b.cells_[p.i][p.j].id = id;
                        b.robots_[id].pos = p;
                        r.alive = true;
                }   }
                if (not fnd) cerr << "info: cannot place pacman in board." << endl;            
    }   }   }

    return b;
}



bool Board::move (int player, int id, Dir d, const Board& b0) {

    // aha, this is the difficult function!

    if (d == None) return false;

    Robot& r1 = robots_[id];
    if (not r1.alive) return false;
    if (round() % 2 == 0 and r1.type != PowerPacMan) return false;

    Pos p1 = r1.pos;
    if (not pos_ok(p1)) return false;

    Pos p2 = dest(p1, d);
    if (not pos_ok(p2)) return false;

    Cell& c1 = cells_[p1.i][p1.j];
    Cell& c2 = cells_[p2.i][p2.j];
    CType& t2 = c2.type;

    if (t2 == Wall) return false;
    if (t2 == Gate and r1.type != Ghost) return false;

    if (c2.id != -1) {
        Robot& r2 = robots_[c2.id];

        if (r2.player == player) {
            return false;
        } else if (r1.type == Ghost) {
            if (r2.type == Ghost) {
                return false;
            } else if (r2.type == PacMan) {
                cells_[r2.pos.i][r2.pos.j].id = -1;
                r2.alive = false;
                r2.time = regen_time();
                r2.pos = Pos(0, 0);
                score_[player] += pn_pacman();
            } else if (r2.type == PowerPacMan) {
                cells_[r1.pos.i][r1.pos.j].id = -1;
                r1.alive = false;
                r1.pos = Pos(0, 0);
                r1.time = regen_time();
                score_[r2.player] += pn_ghost();
                c1.id = -1;
                return true;
            } else assert(0);
        } else if (r1.type == PacMan) {
            if (r2.type == Ghost) {
                cells_[r1.pos.i][r1.pos.j].id = -1;
                r1.alive = false;
                r1.pos = Pos(0, 0);
                r1.time = regen_time();
                score_[r2.player] += pn_pacman();
                c1.id = -1;
                return true;
            } else if (r2.type == PacMan) {
                return false;
            } else if (r2.type == PowerPacMan) {
                return false;
            } else assert(0);
        } else if (r1.type == PowerPacMan) {
            if (r2.type == Ghost) {
                cells_[r2.pos.i][r2.pos.j].id = -1;
                r2.alive = false;
                r2.pos = Pos(0, 0);
                r2.time = regen_time();
                score_[player] += pn_ghost();
            } else if (r2.type == PacMan) {
                return false;
            } else if (r2.type == PowerPacMan) {
                return false;
            } else assert(0);
        } else assert(0);
    }


    if (t2 == Dot and r1.type != Ghost) {
        t2 = Empty;
        score_[player] += pn_dot();
    }

    if (t2 == Pill and r1.type != Ghost) {
        t2 = Empty;
        score_[player] += pn_pill();
        if (r1.type != Ghost) {
            r1.type = PowerPacMan;
            r1.time = power_time();
    }   }

    if (t2 == Bonus and r1.type != Ghost) {
        t2 = Empty;
        score_[player] += pn_bonus();
    }

    if (t2 == Hammer and r1.type != Ghost) {
        t2 = Empty;
        score_[player] += pn_bonus();
        for (int i = 0; i < nb_robots(); ++i) {
            Robot& r3 = robots_[i];
            if (r3.alive and r3.player != player and r3.type == Ghost) {
                cells_[r3.pos.i][r3.pos.j].id = -1;
                r3.alive = false;
                r3.pos = Pos(0, 0);
                r3.time = 3;
                score_[player] += pn_ghost();                
            }
        }
    }

    if (t2 == Mushroom and r1.type != Ghost) {
        t2 = Empty;
        score_[player] += pn_bonus();
        for (int i = 0; i < rows(); ++i) {
            for (int j = 0; j < cols(); ++j) {
                if (b0.cell(i, j).type == Dot) cells_[i][j].type = Dot;
                if (b0.cell(i, j).type == Pill) cells_[i][j].type = Pill;
                if (b0.cell(i, j).type == Bonus) cells_[i][j].type = Bonus;
                if (b0.cell(i, j).type == Hammer) cells_[i][j].type = Hammer;
    }   }   }

    c2.id = c1.id;
    c1.id = -1;
    r1.pos = p2;

    return true;
}




