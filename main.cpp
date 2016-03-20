/*
 * A purposefully barebones flappy bird clone for usage with NEAT (Neuroevolution for Augmenting Topologies),
 * as well as for Genetic Algorithms.
 *
 * The speed is currently really high, however the hope is that the bot will learn quicker this way.
 * You can change the settings if you want it to slow down, however i've not coded in a simple variable for this.
 * Good luck, have fun.
 * -Blake Edwards / Dark Element
*/

#include <SDL2/SDL.h>
#include <SDL/SDL.h>
#include <iostream>
#include <string>
#include <vector>
#include <random>

using namespace std;

vector<vector<string> > grid;

int height = 128;
int width = 128;

//Pipes
  int pipe_min_len = 16;//px
  int pipe_width = 6;//px
  //int gap_len = 16;//px
  int gap_len = 16;
  int pipe_dist = 32;//px
  int last_pipe_x = 0;

  int pipe1_start = 0;
  int pipe2_end = width-1;

  int pipe1_end;
  int pipe2_start;

//Physics
  float init_velocity = 4.5;//px/s
  float acceleration = -.9;//px/s

  float position;
  float jump_time = 1.0;
  int jump_x = 1;
  int jump_y = width/2;

//Misc
  int score;
  int score_interval = 16;
  int score_inc = 0;
  int high_score = 0;

std::random_device rseed;
std::mt19937 rgen(rseed()); // mersenne_twister
std::uniform_real_distribution<double> idist(0+pipe_min_len,width-pipe_min_len-gap_len);//use idist(rgen) to get rand

class Player{
  public:
    int x;
    int y;
};

class Game{
  public:
    enum states {idle, playing, over};
};

SDL_Rect new_rect(int x, int y, int w, int h){
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  return rect;
}


vector<vector<string> > move_background_west(vector<vector<string> > grid){
  for (int x=0;x<grid.size();x++){
    for (int y=0;y<grid.size();y++){
      if (grid[x][y] == "B"){
        if (x-1 > -1){
          grid[x][y] = "W";
          grid[x-1][y] = "B";
        }
      }
    }
  }
  return grid;
}

vector<vector<string> > gen_pipes(vector<vector<string> > grid, int last_pipe_x){
  //Make new set of pipes at right of screen
  pipe1_end = floor(idist(rgen));
  pipe2_start = pipe1_end + gap_len;
  for (int y=pipe1_start;y<=pipe1_end;y++){
    for (int w=0;w<=pipe_width;w++){
      grid[grid.size()-1-w][y] = "B";
    }
  }
  for (int y=pipe2_start;y<=pipe2_end;y++){
    for (int w=0;w<=pipe_width;w++){
      grid[grid.size()-1-w][y] = "B";
    }
    grid[grid.size()-1][y] = "B";
  }
  return grid;
}

void update_display(vector<vector<string> > grid, SDL_Renderer* renderer){
  // Update display
  for (int x=0;x<grid.size();x++){
    for (int y=0;y<grid[x].size();y++){
      if (grid[x][y] == "W"){
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      }else if(grid[x][y] == "B"){
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      }else if(grid[x][y] == "P"){
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
      }

      SDL_Rect rect = new_rect(x*5, y*5, 5, 5);
      SDL_RenderFillRect(renderer, &rect );
    }
  }

  SDL_RenderDrawPoint(renderer, 1, 1);
  SDL_RenderPresent(renderer);//Update screen
}

bool check_collision(vector<vector<string> > grid, int x, int y){
  if ((x > -1 && x < grid.size()) && (y > -1 && y < grid.size())){
    if (grid[x][y] != "B"){
      return false;//no collision
    }
  }
  return true;//collision
}

int main (int argc, char** argv)
{
  SDL_Window* window = NULL;
  window = SDL_CreateWindow
    (
     "me too thanks", SDL_WINDOWPOS_UNDEFINED,
     SDL_WINDOWPOS_UNDEFINED,
     720,
     720,
     SDL_WINDOW_SHOWN
    );

  // Setup renderer
  SDL_Renderer* renderer = NULL;
  renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);

  // Set render color to white ( background will be rendered in this color )
  SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );

  while (1){//so that it will always restart
    // Clear window
    SDL_RenderClear(renderer);

    //Begin execution
    score = 0;
    grid.resize(height);
    for (int i = 0; i < height; ++i) {
      grid[i].resize(width);
      /*for (int j = 0; j < width; ++j){
        grid[i][j];//.resize(depth);
      }*/
    }

    //Fill with white cells 
    for (int x=0;x<grid.size();x++){
      for (int y=0;y<grid[x].size();y++){
        grid[x][y] = "W";
      }
    }

    grid[width/2][height/2] = "P";

    Player player;
    player.x = width/2;
    player.y = height/2;

    Game::states game_state;
    game_state = Game::idle;

    update_display(grid, renderer);

    SDL_Event event;
    while (1){
      //When they press space move game state to playing
      SDL_PollEvent(&event);

      //collision detection
      if (check_collision(grid, player.x+1, player.y)){
        game_state = Game::over;
        break;
      }

      if (game_state == Game::playing){
        //Update Score if just got free of a pipe
        if (grid[player.x][0] == "W" && grid[player.x-1][0] == "B"){
          score++;
          if (score > high_score){
            high_score = score;
          }
          cout << score << ":" << high_score << endl;
          //cout << "Score: " << score << "\tHigh Score: " << high_score << endl;
        }
        grid = move_background_west(grid);
        //pipe gen
        last_pipe_x--;
        if (grid.size()-last_pipe_x >= pipe_dist){
          grid = gen_pipes(grid, last_pipe_x);
          last_pipe_x = grid.size()-1;
        }

        //Increment values since last jump
        jump_time++;

        //Calculate their position, then subtract it from player.y(inverted graph)
        position = .5*acceleration*(pow(jump_time, 2)) + init_velocity*jump_time + jump_x;

        //Update position
        grid[player.x][player.y] = "W";
        player.y = jump_y- position;
        if (check_collision(grid, player.x, player.y)){
          game_state = Game::over;
          break;
        }
        grid[player.x][player.y] = "P";

        update_display(grid, renderer);
      }
        
      if (event.type == SDL_KEYDOWN){
        if (event.key.keysym.sym == 113){
          SDL_DestroyWindow(window);
          SDL_Quit();

          return EXIT_SUCCESS;
        }else{
          if (event.key.keysym.sym == 32){
            if (game_state != Game::playing){
              game_state = Game::playing;
            }
            //reset all the shit and start the calculations over again
            //player.y-=position;
            jump_y = player.y;
            jump_time = 1;
            //jump_x = 1;
          }
        }
      }
      update_display(grid, renderer);
    }
    cout << "Died" << endl;
  }
}
