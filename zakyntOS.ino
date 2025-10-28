#include <LiquidCrystal.h>
#include <EEPROM.h>

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

//for OS
uint8_t bright = 85;
uint8_t frames = 0;
uint8_t delay_time = 25;
uint16_t delay_change = 0;
bool buttons[5] = {false}; //sequence left-up-right-down-any
bool last_btns[4] = {false};
uint8_t btn_pins[4] = {6, 5, 2, 4};
uint8_t num_click = 0;
String states[11] = {"menu", "flappy", "snake", "RPS", "pong", "game", "game", "game", "game", "game", "scores"};
int data[9] = {0};
uint8_t num_states = 11;
int8_t selected = 0;
int8_t display_offset = 0;
byte icons[13][8] = {
  {0, 0, 0, 4, 10, 12, 0, 0}, //flappy
  {0, 12, 8, 10, 8, 10, 14, 0}, //snake
  {0, 0, 25, 26, 4, 26, 25, 0}, //scissors
  {1, 17, 21, 17, 1, 1, 1, 1}, // pong
  {0, 0, 14, 8, 12, 2, 12, 0}, // 5
  {0, 0, 4, 8, 12, 10, 4, 0}, // 6
  {0, 0, 14, 2, 4, 8, 8, 0}, // 7
  {0, 0, 4, 10, 4, 10, 4, 0}, // 8
  {0, 0, 4, 10, 6, 2, 4, 0}, // 9
  {0, 0, 23, 0, 23, 0, 23, 0}, // scores
  {0, 0, 31, 17, 10, 4, 0, 0}, // empty ↓
  {0, 0, 31, 31, 14, 4, 0, 0}, // full ↓
  {0, 0, 4, 10, 17, 31, 0, 0} // ↑
};
String state = states[0];
bool alive = true;

//flappy
uint8_t steps[4] = {11, 3, 8, 0};
uint8_t lengths[13] = {0, 0, 0, 0, 0, 0, 1, 3, 7, 14, 28, 24, 16}; //rotation of pipe widths
uint8_t heights[5] = {8, 8, 8, 4, 1}; 

//snake
uint8_t dir = 2;
uint8_t nextdir;
bool can_change = true;
uint8_t binary[5] = {16, 8, 4, 2, 1};
uint8_t body[240][2] = {0};
uint8_t fruit[2];
bool free_pixel;

//Rock, Paper, Scissors
byte play[3][8] = {
  {0, 0, 6, 15, 31, 31, 14, 0},
  {0, 0, 15, 15, 31, 30, 30, 0},
  {0, 0, 25, 26, 4, 26, 25, 0}
};
uint8_t player = 1;

//pong
int8_t vector[2] = {1, 1};

//for multiple games
uint16_t length;
uint16_t score = 0;
byte characters[2][4][8];
int8_t byte_n = 0;
uint8_t tiles[4][16] = {0}; //4 slots with 16 lines each

void gameover(int score, int game) {
  delay(300);
  //Blink display 3 times
  for(int i = 0; i < 3; i++) {
    lcd.noDisplay();
    delay(400);
    lcd.display();
    delay(400);
  }

  lcd.clear();
  delay(100);
  
  lcd.setCursor(0, 0);
  lcd.print("Game Over");
  if (score > data[game]) {
    storeData(score, game);
    getData(game);

    lcd.setCursor(0, 1);
    lcd.print("New best: ");
    lcd.print(score);
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(score);
    lcd.setCursor(11, 0);
    lcd.print("Best:");
    lcd.setCursor(11, 1);
    lcd.print(data[game]);
  }
  delay_time = 25;
  alive = false;
}

void displayScore(int num) {
  int i = 0;
  if (num >= 0) {
    i = num;
  }
  lcd.setCursor(8, 0);
  lcd.print("Score:");
  lcd.setCursor(8, 1);
  lcd.print(i);
}

void displayTiles(int offset) {
  for(int i = 0; i < 4; i++) {
    for(int n = 0; n < 8; n++) {
      characters[0][i][n] = tiles[i][n];
      characters[1][i][n] = tiles[i][n + 8];
    }
  }
  byte_n = 0;
  for(int i = 0; i < 4; i++) {
    for(int n = 0; n < 2; n++) {
      lcd.createChar(byte_n, characters[n][i]);
      byte_n++;
    }
  }
  byte_n = 0;
  for(int i = 0; i < 4; i++) {
    for(int n = 0; n < 2; n++) {
      lcd.setCursor(i + offset, n);
      lcd.write(byte(byte_n));
      byte_n++;
    }
  }
}

void changeDelay(int min, int interval) {
  if ((delay_time > min) and (score % interval == 0)) {
    if (delay_change != score) {
      delay_time--;
      delay_change = score;
    }
  }
}

void RPSPlay(int human, int player) {
  lcd.setCursor(0, 0);
  lcd.write(byte(human));
  lcd.print(" X ");
  lcd.write(byte(player));
}

void RPSState(String state, int score) {
  if (score > data[2]) {
    storeData(score, 2);
    getData(2);
  }
  lcd.setCursor(0, 1);
  lcd.print(state);
  lcd.setCursor(8, 0);
  lcd.print("Row: ");
  lcd.print(score);
  lcd.setCursor(8, 1);
  lcd.print("Max: ");
  lcd.print(data[2]);
  alive = false;
}

void RPSAnim() {
  lcd.clear();
  delay(50);
  for(int i = 0; i < 3; i++) {
    RPSPlay(i, i);
    delay(300);
    lcd.setCursor(0, 0);
    lcd.write("  X  ");
    delay(50);
  }
  lcd.clear();
  delay(200);
}

void OSAnim() {
  lcd.setCursor(4, 0);
  lcd.print("Z");
  delay(450);
  lcd.print("a");
  delay(330);
  lcd.print("k");
  delay(230);
  lcd.print("y");
  delay(150);
  lcd.print("n");
  delay(100);
  lcd.print("t");
  delay(50);
  lcd.print("OS");
  delay(1400);
  lcd.clear();
  delay(200);
}

void storeData(int value, int slot) { // allocate 2 bytes per int
  EEPROM.put(slot * sizeof(int), value);
}

void getData(int index) {
  EEPROM.get(index * sizeof(int), data[index]);
  if (data[index] == -1 || data[index] == 0xFFFF) {
    data[index] = 0; // default score
  }
}

void setup() {
  pinMode (2, INPUT_PULLUP);
  pinMode (3, OUTPUT);
  pinMode (4, INPUT_PULLUP);
  pinMode (5, INPUT_PULLUP);
  pinMode (6, INPUT_PULLUP);
  pinMode (A0, INPUT_PULLUP);
  pinMode (A1, INPUT_PULLUP);

  for (int i = 0; i < sizeof(body)/sizeof(body[0]); i++) {
    body[i][0] = 0;
    body[i][1] = 0;
  }
  
  for (int i = 0; i < 4; i++) {
    getData(i);
  }
  
  // initialize LCD and set up the number of columns and rows:
  lcd.begin(16, 2);
  analogWrite(3, (bright));
  OSAnim();
}

void loop() { // COPIED
  //adjast brightness
  analogWrite(3, (bright));
  if(!digitalRead(A0)) {
    if (bright < 255) {
      bright++;
    }
  } 
  if (!digitalRead(A1)) {
    if (bright > 0) {
      bright--;
    }
  }

  //set button states
  buttons[4] = false;
  for(int i = 0; i < 4; i++) {
    if (!digitalRead(btn_pins[i])) {
      if (!last_btns[i]) {
        buttons[i] = true;
        last_btns[i] = true;
        buttons[4] = true;
      } else {
        buttons[i] = false;
      }
      num_click++;
    } else {
      last_btns[i] = false;
      buttons[i] = false;
    }
  }

  //menu view
  if (state == "menu") { //COPYED
    byte_n = 0;
    for(int i = display_offset; i < display_offset + 6; i++) {
      lcd.createChar(byte_n, icons[i]); // chars 0 - 5
      byte_n++;
    }
    lcd.createChar(6, icons[num_states - 1]);
    lcd.createChar(7, icons[num_states]);
    lcd.setCursor(0, 0);
    lcd.write("Menu");
    for (int i = 1; i < 7; i++) {
      lcd.setCursor((i*2 + 3), 0);
      if (selected - display_offset == i - 1) {
        lcd.write(byte(7));
      } else {
        lcd.write(byte(6));
      }
      lcd.setCursor((i*2 + 3), 1);
      lcd.write(byte(i - 1));
    }

    if (buttons[0]) {
      selected--;
      if(display_offset > selected) {
        display_offset = selected;
      }
      if(selected < 0) {
        selected = num_states - 2;
        display_offset = num_states - 7;
      }
    }
    if (buttons[2]) {
      selected++;
      if(display_offset < selected - 5) {
        display_offset++;
      }
      if(selected > num_states - 2) {
        selected = 0;
        display_offset = 0;
      }
    }

    if (buttons[3]) {
      state = states[selected + 1];
      alive = true;
      score = 0;
      delay_time = 25;
      randomSeed(frames + num_click);
      if (state == "flappy") {
        length = 2;
        for(int i = 0; i < 3; i++) {
          heights[i] = 8;
        }
        heights[3] = random(8);
        heights[4] = random(8);
        steps[0] = 11;
        steps[1] = 3;
        steps[2] = 8;
        steps[3] = 0;
        score = -1;
      } else if (state == "snake") {
        length = 2;
        dir = 2;
        nextdir = 4;
        body[0][0] = 1;
        body[0][1] = 2;
        fruit[0] = random(19) + 2;
        fruit[1] = random(15) + 2;
      } else if (state == "RPS") {
        player = 1;
        lcd.createChar(0, play[0]);
        lcd.createChar(1, play[1]);
        lcd.createChar(2, play[2]);
        lcd.createChar(3, icons[num_states - 1]);
        lcd.createChar(4, icons[num_states]);
        lcd.createChar(5, icons[num_states + 1]);
      } else if (state == "pong") {
        length = 1;
        vector[0] = 1;
        vector[1] = 1 - random(2) * 2;
        body[0][0] = 11;
        body[0][1] = random(14) + 1;
        delay_time = 20;
      } else if (state == "scores") {
        player = 0;
        length = (num_states - 2) / 4;
        if ((num_states - 2) % 4 == 0) {
          length--;
        }
      } 
      lcd.clear();
    }
  } else if (state == "flappy") { //RE-DONE
    if (alive) {
      if (buttons[4]) {
        if (length >= 2) {
          length = length - 2;
        }
      }

      //clearing tiles
      for(int i = 0; i < 4; i++) {
        for(int n = 0; n < 16; n++) {
          tiles[i][n] = 0;
        }
      }

      //creating all 4 tiles
      for(int i = 0; i < 4; i++) {
        if (heights[i] > 7) {
          for(int n = 0; n < 16; n++) {
            tiles[i][n] = 0;
          }
        } else {
          tiles[i][0] = lengths[steps[i]];
          tiles[i][15] = lengths[steps[i]];
          for(int n = 1; n < 15; n++) {
            if (heights[i] >= n) {
              tiles[i][n] = lengths[steps[i]];
            } else if (heights[i] + 7 < n) {
              tiles[i][n] = lengths[steps[i]];
            }
          }
        }
      }
      
      for(int i = 0; i < length + 3; i++) {
        if (length == i) {
          tiles[1][i] = tiles[1][i] + 4;
        } else if (length + 1 == i) {
          tiles[1][i] = tiles[1][i] + 10;
        } else if (length + 2 == i) {
          tiles[1][i] = tiles[1][i] + 12;
        }
      }
      
      if (frames % 10 == 0) {
        for(int i = 0; i < 4; i++) {
          steps[i]++;
          if (steps[i] >= 13) {
            steps[i] = 0;
            heights[i] = heights[i + 1];
          }
        }
        if (steps[1] == 12) {
          score++;
        }
      }
      
      //rand height of pipe
      if (heights[3] == heights[4]) {
        randomSeed(num_click);
        heights[4] = random(8);
      }

      displayTiles(0);
      displayScore(score);
      
      //death check
      if (length >= 13) {
        gameover(score, 0);
      } else if(tiles[1][length + 1] == 11) {
        gameover(score, 0);
      } else if ((tiles[1][length - 1] & 4) == 4) {
        if (length != 0) {
          gameover(score, 0);
        }
      } else if (((tiles[1][length + 3] & 4) == 4) or ((tiles[1][length + 3] & 8) == 8)) {
        if (length < 12) {
          gameover(score, 0);
        }
      } else if (tiles[1][length + 2] == 15) {
        gameover(score, 0);
      }

      if(frames % 10 == 0) {
        length++;
      }

      changeDelay(3, 2);
    } else if (buttons[4]) {
      for(int i = 0; i < 4; i++) {
        for(int n = 0; n < 16; n++) {
          tiles[i][n] = 0;
        }
      }
      state = states[0];
      lcd.clear();
    }
  } else if (state == "snake") { //RE-WRITTEN
    if (alive) {
      for(int i = 0; i < 4; i++) {
        if (buttons[i] and dir != (i - 2) and dir != (i + 2)) {
          if (can_change) {
            dir = i;
            can_change = false;
          } else if ((nextdir == 4) and (dir != i)) {
            nextdir = i;
          }
        } else if ((nextdir < 4) and (can_change)) {
          dir = nextdir;
          nextdir = 4;
        }
      }
      
      for(int i = 0; i < 4; i++) {
        for(int n = 0; n < 16; n++) {
          tiles[i][n] = 0;
        }
      }

      

      if (frames % 10 == 0) {
        for(int i = length; i > 0; i--) {
          body[i][0] = body[i - 1][0];
          body[i][1] = body[i - 1][1];
        }
        can_change = true;
        if (dir % 2 == 0) {
          body[0][0] += (dir - 1);
        } else {
          body[0][1] += (dir - 2);
        }

        if (body[0][0] == fruit[0] and body[0][1] == fruit[1]) {
          if (length < 239) {
            length++;
          }
          score++;
          if ((delay_time > 1) and (score % 1 == 0)) {
            if (delay_change != score) {
              delay_time--;
              delay_change = score;
            }
          }
          free_pixel = false;
          randomSeed(num_click);
          while (!free_pixel) {
            free_pixel = true;
            fruit[0] = random(20) + 1;
            fruit[1] = random(16) + 1;
            for (int i = 0; i < length + 1; i++) {
              if (body[i][0] == fruit[0] and body[i][1] == fruit[1]) {
                free_pixel = false;
              }
            }
          }
        }
      }

      for(int i = 0; i < length + 1; i++) {
        tiles[(body[i][0] - 1) / 5][body[i][1] - 1] += binary[(body[i][0] - 1) % 5];
      }
      
      tiles[(fruit[0] - 1) / 5][fruit[1] - 1] += binary[(fruit[0] - 1) % 5];
     
      free_pixel = true;
      for (int i = 0; i < length + 1; i++) {
        if (body[i + 1][0] == body[0][0] and body[i + 1][1] == body[0][1]) {
          free_pixel = false;
        }
      }
      if ((body[0][0] < 1) or (body[0][0] > 20 or body[0][1] < 1) or (body[0][1] > 16) or !free_pixel) {
        for (int i = 0; i < length + 1; i++) {
          body[i][0] = 0;
          body[i][1] = 0;
        }
        gameover(score, 1);
      } else {
        lcd.setCursor(0, 0);
        lcd.write(255);
        lcd.setCursor(0, 1);
        lcd.write(255);

        displayTiles(1);
        
        lcd.setCursor(5, 0);
        lcd.write(255);
        lcd.setCursor(5, 1);
        lcd.write(255);

        displayScore(score);
      }

    } else if (buttons[4]) {
      for(int i = 0; i < 4; i++) {
        for(int n = 0; n < 16; n++) {
          tiles[i][n] = 0;
        }
      }
      state = states[0];
      lcd.clear();
    }
  } else if (state == "RPS") { //RE-DONE
    if (alive) {
      if (buttons[0]) {
        player--;
        if(player < 1) {
          player = 3;
        }
      }
      if (buttons[2]) {
        player++;
        if(player > 3) {
          player = 1;
        }
      }
      
      for (int i = 1; i < 4; i++) {
        lcd.setCursor(i*2 - 2, 0);
        if (player == i) {
          lcd.write(byte(4));
        } else {
          lcd.write(byte(3));
        }
        lcd.setCursor(i*2 - 2, 1);
        lcd.write(byte(i-1));
      }
      lcd.setCursor(10, 0);
      lcd.print("Back ");
      lcd.write(byte(5));

      if (buttons[3]) {
        randomSeed(frames);
        length = random(3) + 1;
        RPSAnim();
        RPSPlay(player - 1, length - 1);
        if (length == player) {
          RPSState("Draw", score);
        } else if ((length == player + 1) or (player == 3 and length == 1)) {
          score = 0;
          RPSState("Lose", score);
        } else {
          score++;
          RPSState("Win", score);
        }
      } else if (buttons[1]) { //back to menu
        state = states[0];
        lcd.clear();
      }
    } else if (buttons[4]) {
      alive = true;
      lcd.clear();
    }
  } else if (state == "pong") { //RE-DONE
    if (alive) {
      if (buttons[1]) {
        if (player > 0) {
          player--;
        }
      }

      if (buttons[3]) {
        if (player < 13) {
          player++;
        }
      }

      if (frames % 4 == 0) {
        body[0][0] += vector[0];
        body[0][1] += vector[1];

        body[1][0] = body[0][0] + 1;
        body[1][1] = body[0][1];
        body[2][0] = body[0][0];
        body[2][1] = body[0][1] + 1;
        body[3][0] = body[0][0] + 1;
        body[3][1] = body[0][1] + 1;
        
        if (body[0][0] > 18) {
          vector[0] = vector[0] * -1;
        }
        if (body[0][0] == 3 and -1 < body[0][1] - player and body[0][1] - player < 4) {
          vector[0] = vector[0] * -1;
          score++;
        }
        if (body[0][1] < 2 or body[0][1] > 14) {
          vector[1] = vector[1] * -1;
        }
      }
      
      for(int i = 0; i < 4; i++) {
        for(int n = 0; n < 16; n++) {
          tiles[i][n] = 0;
        }
      }

      for(int i = 0; i < 4; i++) {
        tiles[(body[i][0] - 1) / 5][body[i][1] - 1] += binary[(body[i][0] - 1) % 5];
      }
      for(int i = 0; i < 3; i++) {
        if ((tiles[0][player + i] & 8) != 8) {
          tiles[0][player + i] += 8;
        }
      }

      displayTiles(0);
      lcd.setCursor(4, 0);
      lcd.write(255);
      lcd.setCursor(4, 1);
      lcd.write(255);
      displayScore(score);

      changeDelay(10, 1);

      if (body[0][0] == 1) {
        gameover(score, 3);
        for (int i = 0; i < 4; i++) {
          body[i][0] = 0;
          body[i][1] = 0;
        }
      }
    } else if (buttons[4]) {
      state = states[0];
      lcd.clear();
    }
  } else if (state == "scores") {
    if (buttons[0]) {
      player--;
      if (player == 255) {
        player = length;
      }
      lcd.clear();
    }
    if (buttons[2]) {
      player++;
      if (player > length) {
        player = 0;
      }
      lcd.clear();
    }

    for (int i = 0; i < 4; i++) {
      lcd.createChar(i, icons[player * 4 + i]);
      if (player * 4 + i + 3 == num_states) {
        i = 4;
      }
    }

    for (int i = 0; i < 4; i++) {
      lcd.setCursor((i / 2) * 8, (i % 2));
      lcd.write(i);
      lcd.print(" ");
      lcd.print(data[player * 4 + i]);
      if (player * 4 + i + 3 == num_states) {
        i = 4;
      }
    }
    
    if (buttons[1]) {
      state = states[0];
      lcd.clear();
    }
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Game error");
    if (buttons[4]) {
      state = states[0];
      lcd.clear();
    }
  }
  frames++;
  delay(delay_time);
}