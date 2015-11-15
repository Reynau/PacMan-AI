

// *********************************************************************
// Utility functions
// *********************************************************************


function getURLParameter (name) {
    // http://stackoverflow.com/questions/1403888/get-url-parameter-with-jquery
    return decodeURI(
        (RegExp(name + '=' + '(.+?)(&|$)').exec(location.search)||[,null])[1]
    );
}


function int (s) {
    return parseInt(s);
}

function double (s) {
    return parseFloat(s);
}




// *********************************************************************
// Global variables
// *********************************************************************


// Canvas element
var canvas = document.getElementById('myCanvas');

// Game state

var dataLoaded = false;
var gameLoaded = false;
var gamePaused = true;
var gamePreview = false;
var gameDirection = 1;          // jpetit: -1 does not work
var gameAnim = true;

var speed = 20;
var actRound = 0;
var FRAME_TIME = 1000/speed;
var frames = 0;
var FRAMES_PER_TURN = 4;
var reduction_factor = 1.0;

//Object for storing all the game data
var data = new Object();

//String for storing the raw data
var raw_data_str;

// Assigment of colors
var colors = new Array();
colors[0] = "blue";
colors[1] = "green";
colors[2] = "orange";
colors[3] = "purple";






function loadGame (game) {
    var xmlhttp;

    if (window.XMLHttpRequest) {
        // code for IE7+, Firefox, Chrome, Opera, Safari
        xmlhttp = new XMLHttpRequest();
    } else {
        // code for IE6, IE5
        xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
    }
    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState==4) {
            raw_data_str = xmlhttp.responseText;
            dataLoaded = true;
        }
    }

    xmlhttp.open("GET", game, false);
    xmlhttp.send();
}


// jpetit: com saber que el fitxer no s'ha carregat???


//Parsing and initializing the game
function initGame () {

    // convert text to tokens

    var st = raw_data_str + "";
    var t = st.replace('\n', ' ').split(/\s+/);
    var p = 0;


    data.tileSize = 26;

    // read prelude

    s = t[p++];
    data.version = t[p++];
    if (s != "pacman") alert("Error at round " + round + ".\n" + "pacman");
    if (data.version != "v1") alert("Error at round " + round + ".\n" + "version");

    s = t[p++];
    data.nb_players = int(t[p++]);
    if (s != "nb_players") alert("Error at round " + round + ".\n" + "nb_players");

    s = t[p++];
    data.nb_rounds = int(t[p++]);
    if (s != "nb_rounds") alert("Error at round " + round + ".\n" + "nb_rounds");

    s = t[p++];
    data.nb_ghosts = int(t[p++]);
    if (s != "nb_ghosts") alert("Error at round " + round + ".\n" + "ghosts");

    s = t[p++];
    data.regen_time = int(t[p++]);
    if (s != "regen_time") alert("Error at round " + round + ".\n" + "regen_time");

    s = t[p++];
    data.power_time = int(t[p++]);
    if (s != "power_time") alert("Error at round " + round + ".\n" + "power_time");

    s = t[p++];
    data.pn_dot = int(t[p++]);
    if (s != "pn_dot") alert("Error at round " + round + ".\n" + "pn_dot");

    s = t[p++];
    data.pn_pill = int(t[p++]);
    if (s != "pn_pill") alert("Error at round " + round + ".\n" + "pn_pill");

    s = t[p++];
    data.pn_bonus = int(t[p++]);
    if (s != "pn_bonus") alert("Error at round " + round + ".\n" + "no bonus");

    s = t[p++];
    data.pn_ghost = int(t[p++]);
    if (s != "pn_ghost") alert("Error at round " + round + ".\n" + "wrong");

    s = t[p++];
    data.pn_pacman = int(t[p++]);
    if (s != "pn_pacman") alert("Error at round " + round + ".\n" + "pn_pacman");

    s = t[p++];
    data.cage_i = int(t[p++]);
    data.cage_j = int(t[p++]);
    if (s != "cage") alert("Error at round " + round + ".\n" + "cage");

    s = t[p++];
    data.rows = int(t[p++]);
    if (s != "rows") alert("Error at round " + round + ".\n" + "rows");

    s = t[p++];
    data.cols = int(t[p++]);
    if (s != "cols") alert("Error at round " + round + ".\n" + "cols");

    data.nb_robots = (data.nb_ghosts+1) * data.nb_players;

    data.pacman = new Array();
    for (var i = 0; i < data.nb_players; ++i) {
        data.pacman[i] = new Object();
    }

    s = t[p++];
    if (s != "names") alert("Error at round " + round + ".\n" + "names");
    data.names = new Array();
    for (var i = 0; i < data.nb_players; ++i) {
        data.names[i] = t[p++];
    }

    data.rounds = new Array();
    for (var round = 0; round < data.nb_rounds; ++round) {

        $("#debug").html(round);

        if (t[p++] != "round") alert("Error at round " + round + ".\n" + "round");
        if (int(t[p++]) != round) alert("Error at round " + round + ".\n" + "wrong round");

        // maze
        data.rounds[round] = new Object();
        data.rounds[round].rows = new Array();
        for (var i = 0; i < data.rows; ++i) {
            data.rounds[round].rows[i] = t[p++];
        }

        // score
        if (t[p++] != "score") alert("Error at round " + round + ".\n" + "score");
        data.rounds[round].score = new Array();
        for (var i = 0; i < data.nb_players; ++i) {
            data.rounds[round].score[i] = int(t[p++]);
        }

        // status
        if (t[p++] != "status") alert("Error at round " + round + ".\n" + "status");
        data.rounds[round].cpu = new Array();
        for (var i = 0; i < data.nb_players; ++i) {
            data.rounds[round].cpu[i] = double(t[p++]);
        }

        // robots
        data.rounds[round].team = new Array();
        for (var i = 0; i < data.nb_players; ++i) {
            // pacman
            s = t[p++];
            if (s != "pacman" && s != "powerpacman") alert("Error at round " + round + ".\n" + "pacman");
            data.rounds[round].team[i] = new Object();
            data.rounds[round].team[i].pacman = new Object();
            data.rounds[round].team[i].pacman.pos_x = int(t[p++]);
            data.rounds[round].team[i].pacman.pos_y = int(t[p++]);
            data.rounds[round].team[i].pacman.time = int(t[p++]);
            data.rounds[round].team[i].pacman.state = t[p++];

            if (s == "powerpacman") {
                data.rounds[round].team[i].pacman.pow = true;
            } else {
                data.rounds[round].team[i].pacman.pow = false;
            }

            // ghost
            data.rounds[round].team[i].ghosts = new Array();
            for (var j = 0; j < data.nb_ghosts; ++j) {
                if (t[p++] != "ghost") alert("Error at round " + round + ".\n" + "ghost");
                data.rounds[round].team[i].ghosts[j] = new Object();
                data.rounds[round].team[i].ghosts[j].pos_x = int(t[p++]);
                data.rounds[round].team[i].ghosts[j].pos_y = int(t[p++]);
                data.rounds[round].team[i].ghosts[j].time = int(t[p++]);
                data.rounds[round].team[i].ghosts[j].state = t[p++];
        }   }

        if (round != data.nb_rounds - 1) {
            // actions
            if (t[p++] != "actions") alert("Error at round " + round + ".\n" + "no actions");
            for (var i = 0; i < data.nb_players; ++i) {
                if (int(t[p++]) != i) alert("Error at round " + round + ".\n" + "wrong player");
                data.rounds[round].team[i].pacman.action = 'n';
                for (var j = 0; j < data.nb_ghosts; ++j) {
                    data.rounds[round].team[i].ghosts[j].action = 'n';
                }
                var code = int(t[p++]);
                while (code != -1) {
                    if (code == 0) data.rounds[round].team[i].pacman.action = t[p++];
                    else data.rounds[round].team[i].ghosts[code - 1].action = t[p++];
                    code = t[p++];
                }
            }

            // movements
            if (t[p++] != "movements") alert("Error at round " + round + ".\n" + "no movements");
            var movs = new Array();
            for (var i = 0; i < data.nb_robots; ++i) movs[i] = 'n';
            var code = int(t[p++]);
            while (code != -1) {
                var robot = code;
                var move = t[p++];
                movs[robot] = move;
                code = t[p++];
            }

            // write movements in round/team data
            var n = 0;
            for (var i = 0; i < data.nb_players; ++i) {
                data.rounds[round].team[i].pacman.move = movs[n];
                n++;
                for (var j = 0; j < data.nb_ghosts; ++j) {
                    data.rounds[round].team[i].ghosts[j].move = movs[n];
                    n++;
    }   }   }   }


    // load sprites
    preloadImages();

    // prepare state variables
    gameLoaded = true;
    if (getURLParameter("start") == "yes" || getURLParameter("game") == "jordi.pac") gamePaused = false;
    else gamePaused = true;


    gamePreview = true;

    // slider init
    $("#slider").slider( "option", "min", 0 );
    $("#slider").slider( "option", "max", data.nb_rounds );

    // turn things on
    $("#loading").hide();
    $("#canvas").show();
    $("#scoreboard").show();
}


function preloadImages () {

    data.img = new Array();

    //Tiles
    data.img.tile_floor = new Image();
    data.img.tile_floor.src = "img/tile_floor.png";

    data.img.tile_wall = new Image();
    data.img.tile_wall.src = "img/tile_wall.png";
    data.img.tile_wall1 = new Image();
    data.img.tile_wall1.src = "img/tile_wall1.png";
    data.img.tile_wall2 = new Image();
    data.img.tile_wall2.src = "img/tile_wall2.png";
    data.img.tile_wall3 = new Image();
    data.img.tile_wall3.src = "img/tile_wall3.png";
    data.img.tile_wall4 = new Image();
    data.img.tile_wall4.src = "img/tile_wall4.png";
    data.img.tile_wall5 = new Image();
    data.img.tile_wall5.src = "img/tile_wall5.png";
    data.img.tile_wall6 = new Image();
    data.img.tile_wall6.src = "img/tile_wall6.png";
    data.img.tile_wall7 = new Image();
    data.img.tile_wall7.src = "img/tile_wall7.png";
    data.img.tile_wall8 = new Image();
    data.img.tile_wall8.src = "img/tile_wall8.png";
    data.img.tile_wall9 = new Image();
    data.img.tile_wall9.src = "img/tile_wall9.png";
    data.img.tile_wall10 = new Image();
    data.img.tile_wall10.src = "img/tile_wall10.png";
    data.img.tile_wall11 = new Image();
    data.img.tile_wall11.src = "img/tile_wall11.png";
    data.img.tile_wall12 = new Image();
    data.img.tile_wall12.src = "img/tile_wall12.png";
    data.img.tile_wall13 = new Image();
    data.img.tile_wall13.src = "img/tile_wall13.png";
    data.img.tile_wall14 = new Image();
    data.img.tile_wall14.src = "img/tile_wall14.png";
    data.img.tile_wall15 = new Image();
    data.img.tile_wall15.src = "img/tile_wall15.png";
    data.img.tile_wall16 = new Image();
    data.img.tile_wall16.src = "img/tile_wall16.png";

    data.img.tile_pill = new Image();
    data.img.tile_pill.src = "img/tile_pill.png";
    data.img.tile_dot = new Image();
    data.img.tile_dot.src = "img/tile_dot.png";
    data.img.tile_bonus = new Image();
    data.img.tile_bonus.src = "img/bonus.png";
    data.img.tile_hammer = new Image();
    data.img.tile_hammer.src = "img/hammer.png";
    data.img.tile_mushroom = new Image();
    data.img.tile_mushroom.src = "img/mushroom.png";
    data.img.tile_gate_h = new Image();
    data.img.tile_gate_h.src = "img/tile_gate_h.png";
    data.img.tile_gate_v = new Image();
    data.img.tile_gate_v.src = "img/tile_gate_v.png";

    //Sprites
    data.img.spr_pacman = new Array();
    data.img.spr_ghost = new Array();
    for (var i = 0; i < data.nb_players; ++i) {
        //data.img.spr_pacman[i] = new Image();
        for (var j = 0; j < data.nb_ghosts; ++j) {
            //data.img.spr_ghost[i*nb_ghosts+j] = new Image();
        }
    }


    data.img.spr_pacman_t = new Array();
    data.img.spr_pacman_b = new Array();
    data.img.spr_pacman_l= new Array();
    data.img.spr_pacman_r = new Array();

    for (var i = 0; i < data.nb_players; ++i) {

        data.img.spr_pacman_l[i] = new Array();
        data.img.spr_pacman_l[i][0] = new Image();
        data.img.spr_pacman_l[i][0].src = "img/pacman-l1-"+colors[i]+".png";
        data.img.spr_pacman_l[i][1] = new Image();
        data.img.spr_pacman_l[i][1].src = "img/pacman-l1-"+colors[i]+".png";
        data.img.spr_pacman_l[i][2] = new Image();
        data.img.spr_pacman_l[i][2].src = "img/pacman-l2-"+colors[i]+".png";
        data.img.spr_pacman_l[i][3] = new Image();
        data.img.spr_pacman_l[i][3].src = "img/pacman-l2-"+colors[i]+".png";

        data.img.spr_pacman_r[i] = new Array();
        data.img.spr_pacman_r[i][0] = new Image();
        data.img.spr_pacman_r[i][0].src = "img/pacman-r1-"+colors[i]+".png";
        data.img.spr_pacman_r[i][1] = new Image();
        data.img.spr_pacman_r[i][1].src = "img/pacman-r1-"+colors[i]+".png";
        data.img.spr_pacman_r[i][2] = new Image();
        data.img.spr_pacman_r[i][2].src = "img/pacman-r2-"+colors[i]+".png";
        data.img.spr_pacman_r[i][3] = new Image();
        data.img.spr_pacman_r[i][3].src = "img/pacman-r2-"+colors[i]+".png";

        data.img.spr_pacman_b[i] = new Array();
        data.img.spr_pacman_b[i][0] = new Image();
        data.img.spr_pacman_b[i][0].src = "img/pacman-b1-"+colors[i]+".png";
        data.img.spr_pacman_b[i][1] = new Image();
        data.img.spr_pacman_b[i][1].src = "img/pacman-b1-"+colors[i]+".png";
        data.img.spr_pacman_b[i][2] = new Image();
        data.img.spr_pacman_b[i][2].src = "img/pacman-b2-"+colors[i]+".png";
        data.img.spr_pacman_b[i][3] = new Image();
        data.img.spr_pacman_b[i][3].src = "img/pacman-b2-"+colors[i]+".png";

        data.img.spr_pacman_t[i] = new Array();
        data.img.spr_pacman_t[i][0] = new Image();
        data.img.spr_pacman_t[i][0].src = "img/pacman-t1-"+colors[i]+".png";
        data.img.spr_pacman_t[i][1] = new Image();
        data.img.spr_pacman_t[i][1].src = "img/pacman-t1-"+colors[i]+".png";
        data.img.spr_pacman_t[i][2] = new Image();
        data.img.spr_pacman_t[i][2].src = "img/pacman-t2-"+colors[i]+".png";
        data.img.spr_pacman_t[i][3] = new Image();
        data.img.spr_pacman_t[i][3].src = "img/pacman-t2-"+colors[i]+".png";
   }

   data.img.spr_powerpac = new Image();
   data.img.spr_powerpac.src = "img/powerpac.png";

    // Ghosts
    data.img.spr_ghostA = new Array();
    data.img.spr_ghostB = new Array();
    for (var i=0; i<data.nb_players; ++i) {
        data.img.spr_ghostA[i] = new Image();
        data.img.spr_ghostA[i].src = "img/ghost-"+colors[i]+"-a.png";
        data.img.spr_ghostB[i] = new Image();
        data.img.spr_ghostB[i].src = "img/ghost-"+colors[i]+"-b.png";
    }
}



function updateGame () {
    if (actRound >= 0 && actRound < data.nb_rounds) {
        for (var i = 0; i < data.nb_players; ++i) {
            var f = (frames+i)%4;
            switch (data.rounds[actRound].team[i].pacman.action) {
                case 't': //Top
                    data.img.spr_pacman[i] = data.img.spr_pacman_t[i][f];
                    break;
                case 'b': //Bottom
                    data.img.spr_pacman[i] = data.img.spr_pacman_b[i][f];
                    break;
                case 'r': //Right
                    data.img.spr_pacman[i] = data.img.spr_pacman_r[i][f];
                    break;
                case 'l': //Left
                    data.img.spr_pacman[i] = data.img.spr_pacman_l[i][f];
                    break;
                default: //None
                    data.img.spr_pacman[i] = data.img.spr_pacman_r[i][f];
                    break;
    }   }   }

    $("#slider").slider("option", "value", actRound);
}


function drawGame () {

    if (canvas.getContext) {
        var context = canvas.getContext('2d');
        var rectSize = data.tileSize;


        // set canvas size
        offset = 20;
        canvas.width  = window.innerWidth - 2 * offset;
        canvas.height = window.innerHeight;
        canvas.style.marginLeft = offset;

        var s = canvas.width/(rectSize*data.cols);
        context.scale(s * reduction_factor, s * reduction_factor);

        //Outter Rectangle
        context.fillStyle = "rgb(0,0,0)";
        context.fillRect(0, 0, rectSize*data.cols, rectSize*data.rows);

        //Draw maze
        for (var i = 0; i < data.rows; i++) {
            drawRow(actRound, i);
        }

        //Draw robots
        for (var i = 0; i < data.nb_players; i++) {

            if (data.rounds[actRound].team[i].pacman.state == 'a') {

                if (! gameAnim) {
                    var x = data.rounds[actRound].team[i].pacman.pos_x;
                    var y = data.rounds[actRound].team[i].pacman.pos_y;
                    var img = data.img.spr_pacman[i];
                    var ctx = canvas.getContext('2d');

                    ctx.drawImage(img, y*data.tileSize, x*data.tileSize);

                } else {
                    var move = data.rounds[actRound].team[i].pacman.move;
                    if (frames < (FRAMES_PER_TURN/2))
                        move = data.rounds[actRound].team[i].pacman.action;

                    var x = data.rounds[actRound].team[i].pacman.pos_x;
                    x = x*data.tileSize;
                    x = setMovedX(x, move);

                    var y = data.rounds[actRound].team[i].pacman.pos_y;
                    y = y*data.tileSize;
                    y = setMovedY(y, move);

                    var img = data.img.spr_pacman[i];
                    var ctx = canvas.getContext('2d');

                    if (data.rounds[actRound].team[i].pacman.pow) {
                        ctx.beginPath();
                        ctx.strokeStyle = colors[i];
                        ctx.lineWidth = 2;
                        ctx.arc(y+14, x+13, 16, 0, Math.PI*2, true);        // mides???
                        ctx.closePath();
                        ctx.stroke();
                    }

                    if (actRound > 6 && data.rounds[actRound - 6].team[i].pacman.state == 'd') {
                        ctx.beginPath();
                        ctx.strokeStyle = colors[i];
                        ctx.lineWidth = 2;
                        ctx.arc(y+14, x+13, 64, 0, Math.PI*2, true);        // mides???
                        ctx.closePath();
                        ctx.stroke();
                    }

                    ctx.drawImage(img, y, x);

                }
            } else {

                    if (actRound < 594 && data.rounds[actRound + 6].team[i].pacman.state == 'a') {

                        var r = 6;
                        while (data.rounds[actRound + r].team[i].pacman.state != 'd') r--;
                        r++;
    
                        var x = data.rounds[actRound + r].team[i].pacman.pos_x;
                        x = x*data.tileSize;
                        x = setMovedX(x, move);
    
                        var y = data.rounds[actRound + r].team[i].pacman.pos_y;
                        y = y*data.tileSize;
                        y = setMovedY(y, move);

                        var ctx = canvas.getContext('2d');

                        ctx.beginPath();
                        ctx.strokeStyle = colors[i];
                        ctx.lineWidth = 2;
                        ctx.arc(y+14, x+13, 64-4*r, 0, Math.PI*2, true);        // mides???
                        ctx.closePath();
                        ctx.stroke();
                    }

            }

            
            for (var j = 0; j < data.nb_ghosts; ++j) {
                if (data.rounds[actRound].team[i].ghosts[j].state == 'a') {
                    if (! gameAnim) {

                        var x = data.rounds[actRound].team[i].ghosts[j].pos_x;
                        var y = data.rounds[actRound].team[i].ghosts[j].pos_y;
                        var img = data.img.spr_ghostA[i];
                        var ctx = canvas.getContext('2d');
                        ctx.drawImage(img, y*data.tileSize, x*data.tileSize);

                    } else {


                        var move = data.rounds[actRound].team[i].ghosts[j].move;
                        if (frames < (FRAMES_PER_TURN/2))
                            move = data.rounds[actRound].team[i].ghosts[j].action;

                        var x = data.rounds[actRound].team[i].ghosts[j].pos_x;
                        x = x*data.tileSize;
                        x = setMovedX(x, move);

                        var y = data.rounds[actRound].team[i].ghosts[j].pos_y;
                        y = y*data.tileSize;
                        y = setMovedY(y, move);

                        var img = data.img.spr_ghostA[i];
                        var ctx = canvas.getContext('2d');
                        ctx.drawImage(img, y, x);
                    }
                } else {
                }
            }
        }
    }
}

//Note: X and Y of the next functions are reversed
function setMovedX (x, action) {
    if (action == 'n') return x;
    if (action == 't') {
        return (x - ((frames*data.tileSize)/FRAMES_PER_TURN));
    }
    if (action == 'b') {
        return (x + ((frames*data.tileSize)/FRAMES_PER_TURN));
    }
    return x;
}
function setMovedY (y, action) {
    if (action == 'n') return y;
    if (action == 'l')
        return (y - ((frames*data.tileSize)/FRAMES_PER_TURN));
    if (action == 'r')
        return (y + ((frames*data.tileSize)/FRAMES_PER_TURN));
    return y;
}
//Funcions per la posicio del powerpacman: s'ha de moure el doble
function setPowerX (x, action) {
    if (action == 't') {
        return (x - ((frames*data.tileSize)/FRAMES_PER_TURN) - ((frames*data.tileSize)/FRAMES_PER_TURN));
    }
    if (action == 'b') {
        return (x + ((frames*data.tileSize)/FRAMES_PER_TURN) + ((frames*data.tileSize)/FRAMES_PER_TURN));
    }
    return x;
}
function setPowerY (y, action) {
    if (action == 'l')
        return (y - ((frames*data.tileSize)/FRAMES_PER_TURN) - ((frames*data.tileSize)/FRAMES_PER_TURN));
    if (action == 'r')
        return (y + ((frames*data.tileSize)/FRAMES_PER_TURN) + ((frames*data.tileSize)/FRAMES_PER_TURN));
    return y;
}


function drawRow (round, row) {
    var ctx = canvas.getContext('2d');
    var rectSize = data.tileSize;
    for (var i = 0; i < data.cols; ++i) {
        var type = data.rounds[round].rows[row][i];
        var img = selectTile(round, type, row, i);
        /* no queda del tot bé
        if (type == 'C' || type == 'B') {
            ctx.save();
            ctx.translate((i+0.5)*rectSize, (row+0.5)*rectSize);
            ctx.rotate((round % 180) / 3.1416);
            ctx.drawImage(img, 0, 0);
            ctx.restore();
        } else
        */
        {
            ctx.drawImage(img, i*rectSize, row*rectSize);
        }
    }
}


function selectTile (round, type, row, col) {
    switch(type) {
        case 'X': //Wall
            return selectWall(round, type, row, col);
            break;
        case '-': //Empty
            return data.img.tile_floor;
            break;
        case '#': //Ghost gate
            if (data.rounds[round].rows[row-1][col] == 'X' ||
                data.rounds[round].rows[row+1][col] == 'X')
                return data.img.tile_gate_v;
            return data.img.tile_gate_h;
            break;
        case '.': //Dot
            return data.img.tile_dot;
            break;
        case '*': //Pill
            return data.img.tile_pill;
            break;
        case 'B': //Bonus
            return data.img.tile_bonus;
            break;
        case 'C': //hammer
            return data.img.tile_hammer;
            break;
        case 'M': //mushroom
            return data.img.tile_mushroom;
            break;
        default:
        break;
    }
}


function selectWall (round, type, row, col) {
    var n = 0;
    var s = 0;
    var e = 0;
    var w = 0;

    if ((row-1) < 0) n = 0;
    else if (data.rounds[round].rows[row-1][col] == 'X') n = 1;
    if ((row+1) >= data.rows) s = 0;
    else if (data.rounds[round].rows[row+1][col] == 'X') s = 1;
    if ((col-1) < 0) e = 0;
    else if (data.rounds[round].rows[row][col-1] == 'X') e = 1;
    if ((col+1) >= data.cols) w = 0;
    else if (data.rounds[round].rows[row][col+1] == 'X') w = 1;

    if (n == 0 && s == 0 && w == 1 && e == 0) return data.img.tile_wall1;
    if (n == 0 && s == 0 && w == 1 && e == 1) return data.img.tile_wall2;
    if (n == 0 && s == 0 && w == 0 && e == 1) return data.img.tile_wall3;
    if (n == 1 && s == 0 && w == 0 && e == 0) return data.img.tile_wall4;
    if (n == 1 && s == 1 && w == 0 && e == 0) return data.img.tile_wall5;
    if (n == 0 && s == 1 && w == 0 && e == 0) return data.img.tile_wall6;
    if (n == 1 && s == 0 && w == 1 && e == 0) return data.img.tile_wall7;
    if (n == 1 && s == 0 && w == 0 && e == 1) return data.img.tile_wall8;
    if (n == 0 && s == 1 && w == 1 && e == 0) return data.img.tile_wall9;
    if (n == 0 && s == 1 && w == 0 && e == 1) return data.img.tile_wall10;
    if (n == 1 && s == 1 && w == 0 && e == 1) return data.img.tile_wall11;
    if (n == 1 && s == 0 && w == 1 && e == 1) return data.img.tile_wall12;
    if (n == 1 && s == 1 && w == 1 && e == 0) return data.img.tile_wall13;
    if (n == 0 && s == 1 && w == 1 && e == 1) return data.img.tile_wall14;
    if (n == 0 && s == 0 && w == 0 && e == 0) return data.img.tile_wall15;
    if (n == 1 && s == 1 && w == 1 && e == 1) return data.img.tile_wall16;
    return data.img.tile_wall;
}







// *********************************************************************
// Button events
// *********************************************************************

function playButton () {
    gamePaused = false;
}


function pauseButton () {
    gamePaused = true;
}


function startButton () {
    actRound = 0;
    gamePreview = true;
}


function endButton () {
    actRound = 99999;
    gamePreview = true;
}


function animButton () {
    gameAnim = !gameAnim;
}

function closeButton () {
    window.close();
}





// *********************************************************************
// Mourse events
// *********************************************************************

function onDocumentMouseWheel (event) {
    var speed = 1;
    // WebKit
    if ( event.wheelDeltaY ) {
        actRound -= event.wheelDeltaY * speed;
    // Opera / Explorer 9
    } else if ( event.wheelDelta ) {
        actRound -= event.wheelDelta * speed;
    // Firefox
    } else if ( event.detail ) {
        actRound -= event.detail * speed;
    }
    mainloop();
}



// *********************************************************************
// Key events
// *********************************************************************


function onDocumentKeyDown (event) {
}




function onDocumentKeyUp (event) {

    // http://www.webonweboff.com/tips/js/event_key_codes.aspx

    switch (event.keyCode) {

        case 36: // Start
            actRound = 0;
            gamePreview = true;
            break;

        case 35: // End
            actRound = 99999;
            gamePreview = true;
            break;

        case 33: // PageDown
            actRound -= 20;
            gamePreview = true;
            break;

        case 34: // PageUp
            actRound += 20;
            gamePreview = true;
            break;

        case 38: // ArrowUp
        case 37: // ArrowLeft
            gamePaused= true;
            frame = 0;
            --actRound;
            gamePreview = true;
            break;

        case 40: // ArrowDown
        case 39: // ArrowRight
            gamePaused = true;
            frame = 0;
            ++actRound;
            gamePreview = true;
            break;

        case 32: // Space
            gamePaused = !gamePaused;
            mainloop();
            break;

        case 27: // Escape
            gamePaused = true;
            mainloop();
            break;

        case 187: // "+"
            reduction_factor += 0.15;
            break;

        case 189: // "-"
            reduction_factor -= 0.15;
            break;

        case 72: // "h"
            help();
            break;

        default:
            $("#debug").html(event.keyCode);
            break;
    }
}




function onWindowResize (event) {
    drawGame();
}


function help () {
    // opens a new popup with the help page
    var win = window.open('help.html' , 'name', 'height=400, width=300');
	if (window.focus) win.focus();
	return false;
}



// *********************************************************************
// This function is called periodically.
// *********************************************************************

function mainloop () {

    // Configure buttons
    if (gamePaused) {
        $("#but_play").show();
        $("#but_pause").hide();
    } else {
        $("#but_play").hide();
        $("#but_pause").show();
    }

    if (dataLoaded) {
        $("#but_anim").show();
        $("#but_reverse").show();
    } else {
        $("#but_anim").hide();
        $("#but_reverse").hide();
    }

    // initialitzation
    if (dataLoaded && !gameLoaded) initGame();
    if (!gameLoaded) return;

    // check actRound. jpetit: check!!!!
    if (actRound < 0) actRound = 0;
    if (actRound >= data.nb_rounds - 1) actRound = data.nb_rounds - 1;


    if (!gamePaused || gamePreview) {
        if (gamePreview) {
            frames = 0;
            gamePreview = false;
        }
        frames += gameDirection;
        if (frames == FRAMES_PER_TURN || frames == 0) {
            if (actRound < data.nb_rounds) {
                actRound += gameDirection;
                if (actRound%2) actRound += gameDirection;
            }
            frames = 0;
        }

        // check actRound again. jpetit: check!!!!
        if (actRound < 0) actRound = 0;
        if (actRound >= data.nb_rounds - 1) actRound = data.nb_rounds - 1;

        updateGame();
        drawGame();
    }


    // write score board

    var score = "<table style='table-layout:fixed; width: 400; ' cellpadding=2>";
    score += "<col width='24'>";
    score += "<col width='5'>";
    for (var j = 0; j < data.nb_ghosts; ++j) {
        score += "<col width='24'>";
    }
    score += "<col width='5'>";
    score += "<col width='120'>";
    score += "<col width='50'>";
    score += "<col width='20'>";
    score += "<col width='50'>";

    for (var i = 0; i < data.nb_players; ++i) {

        score += "<tr style='line-height: 25px;'>";

        score += "<td>";
        if (data.rounds[actRound].team[i].pacman.state == 'a') {
            score += "<img width='24px' src='img/pacman-r1-"+colors[i]+".png'/>";
        } else {
                sz = Math.min(24, 24 - 24 * data.rounds[actRound].team[i].pacman.time / data.regen_time);
                ro = 180 * data.rounds[actRound].team[i].pacman.time / data.regen_time;
                if (i % 2) ro = -ro;
            score += "<center><img width='"+sz+"px' style='-webkit-transform: rotate("+ro+"deg);' src='img/pacman-r1-"+colors[i]+".png'/></center>";
        }
        score += "</td>";

        score += "<td></td>";
        for (var j = 0; j < data.nb_ghosts; ++j) {
            score += "<td>";
            if (data.rounds[actRound].team[i].ghosts[j].state == 'a') {
                score += "<img width='24px' src='img/ghost-"+colors[i]+"-a.png'/>";
            } else {
                sz = Math.min(24, 24 - 24 * data.rounds[actRound].team[i].ghosts[j].time / data.regen_time);
                ro = 180 * data.rounds[actRound].team[i].ghosts[j].time / data.regen_time;
                if ((i+j) % 2) ro = -ro;
                score += "<center><img width='"+sz+"px' style='-webkit-transform: rotate("+ro+"deg);'  src='img/ghost-"+colors[i]+"-a.png'/></center>";
            }
            score += "</td>";
        }
        score += "<td></td>";


        var c = int(data.rounds[actRound].cpu[i]*50);
        if (c > 50 || c < 0) c = 50;

        score += "<td style='color: "+colors[i]+"'><b>"+ data.names[i] + "</b></td>";

        score += "<td style='text-align: right; color: "+colors[i]+"'><b>"+ (data.rounds[actRound].score[i]) + "</b></td>";

        score += "<td>";
        if (data.rounds[actRound].cpu[i] < 0) {
            c = 0;
            score += "<img width='18px' src='img/bomb.png'/>";
        }
        score += "</td>";

        score += "<td>";
        score += "<span style='display: inline-block; background-color: red;   width: " +(c) + "; height: 5px;'/>";
        score += "</td>";



        score += "</tr>";
    }
    score += "</table>";
    document.getElementById("scoreBoard").innerHTML = score;

    // write round
    $("#round").html(actRound);
}





// *********************************************************************
// This function is called when the DOM is ready.
// *********************************************************************

function init () {

    $("#debug").hide();


    // round things off
    $("#loading").show();
    $("#canvas").hide();
    $("#scoreboard").hide();

    // prepare the slider
    $("#slider").slider({
        slide: function(event, ui) {
            var value = $("#slider").slider( "option", "value" );
            actRound = value;
        }
    });
    $("#slider").width(500);

    // get url parameters
    var game;
    if (getURLParameter("sub") != "null") {
        if (getURLParameter("nbr") != "null") {
            game = "https://pacman-fib.jutge.org/?cmd=lliuraments&sub="+getURLParameter("sub")+"&nbr="+getURLParameter("nbr")+"&download=partida";
        } else {
            game = "https://pacman-fib.jutge.org/?cmd=partida&sub="+getURLParameter("sub")+"&download=partida";
        }
    } else {
        game = getURLParameter("game");
    }
    
    var fac = getURLParameter("fac");
    if (fac != "null") reduction_factor = double(fac);
    
    // load the given game
    loadGame(game);

    // round all on
    $("#all").show();

    // set the listerners for interaction
    document.addEventListener('mousewheel', onDocumentMouseWheel, false);
    document.addEventListener('keydown', onDocumentKeyDown, false);
    document.addEventListener('keyup', onDocumentKeyUp, false);

    window.addEventListener('resize', onWindowResize, false);

    // periodically call mainloop
    setInterval(mainloop, FRAME_TIME);

};


