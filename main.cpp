/*
 * A purposefully barebones flappy bird clone for usage with NEAT (Neuroevolution for Augmenting Topologies),
 * as well as for Genetic Algorithms.
 *
 * The speed is currently really high, however the hope is that the bot will learn quicker this way.
 * You can change the settings if you want it to slow down, however i've not coded in a simple variable for this.
 * Good luck, have fun.
 * -Blake Edwards / Dark Element
*/
/*
 *  NOTES
 *  Ok so we just had a generation 0 where it got 54 fitness for every one EXCEPT for one where it jumped and got 73.
 *  On the second generation, however, it did not jump in either genome 0 or 1. This means that it mutated and decided not to jump in that scenario.
 *  Going to reduce mutation rate to try and get a similar scenario again
 *
 *  OK There actually is a problem with the getting of the next generation. OR perhaps it's just getting different pipe heights and as such generates random input to not jump
 *    The latter is kinda way more likely. testing more.
 *
 *  It may realise jumping is better by getting higher fitness with it ONCE, then it and another parent that didn't realise this start the next generation
 *  Then assuming the good gene gets chosen for a good deal of kids, these kids still have to realise to jump again or else it gets evolved out.
 *    I'm hoping that having a good jump threshold that's not enough for them to jump normally but enough that they will get 2+ genomes that do it will make this work.
 *
 *  GREAT SCOTT I THINK I'VE GOT IT
 *  Ok so every time it runs a simulation it is generating the pipes randomly. 
 *    This is fine for the first sim for each generation, but we need it to use the same environment for each genome. 
 *  going to keep track of these in an array so that we store the next pipe_height in the index that should be determinable from our fitness
 *
 *  So even if we keep the environment the same globally, only inherit from the first parent, and don't mutate, it still makes the same mistakes.
 *    This is because if we do that it will always be doing the same thing over again. We need to let it mutate at the very least.
 *
 *    Alright so it gets more fitness by not jumping on this one generation. woo! It gets passed on as parent 1 for the next generation.
 *    All the kids get made and mutations get made so that 1/4th of the kids might jump where the parent did not jump.
 *    However, all the others should still not jump since they did not mutate then. And then these continue the generation.
 *      Then why is it not learning that?!
 *      If it was doing what we just detailed then it would hopefully work, so let's make sure it's doing what we just detailed.
 *
 *  If we don't mutate and always choose parent 1, it should repeat the exact same behavior every time after gen0
 *    but it doesn't! IVE GOT YOU NOW
 *
 *  I guess it's not checking rules correctly? probably not
 *
 *  I tried setting the parents to be the same since I thought the second genome might be screwing things up
 *    But it's now only doing the same behavior for the first two
 *    try hard coding all of them to be parent 1
 *
 *  am I really fucking unlucky or is the behavior in one genome affecting another, making it so that it's either no jumps or 2+ jumps
 *
 *  TODO
 *  Try it with a different number of parents
 *  Tinker and mess with variables to see what works best
 *  make the pipe 1 start a larger distance away and see if it finds the invincibility area
 */


#include <SDL2/SDL.h>
#include <SDL/SDL.h>
#include <iostream>
#include <string>
#include <vector>
#include <random>

//For vector pretty print
#include <algorithm>
#include <iterator>

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
  bool break_twice;

  int pipe1_start = 0;
  int pipe2_end = width-1;

  int pipe1_end;
  int pipe2_start;
  vector<int> pipe_heights;

//Physics

  //Not even sure if these are possible
  /*
  float init_velocity = 4.5;//px/s
  float velocity;
  float acceleration = -.9;//px/s
  */

  //Slower settings for testing
  float init_velocity = 1.5;//px/s
  float velocity = 0;
  float acceleration = -.1;//px/s

  float old_position;
  float position = 64;
  float jump_time = 1.0;
  int jump_x = 1;
  int jump_y = width/2;

//Bot
  vector<vector<vector<float> > > generations;
  vector<float> genome;

  float rule_position;
  float rule_velocity;
  float rule_next_pipe_dist = 0;
  float rule_next_pipe_height = 0;
  float rule_output;

  float next_pipe_dist;
  float next_pipe_height;
  float fitness;
  float max_fitness;
  float output;//Jump/not jump

  int score;
  int score_interval = 16;
  int score_inc = 0;
  int high_score = 0;

  //Setting this low/high so we know if the fucker is really learning. If we set it to something like .99 it will merely give the illusion of learning
  float jump_threshold = .997;
  float parent1_threshold = 0;//The lower this is the likelier it will pick parent1 for the ruleset
  float mutation_threshold = 1;//The higher the less likely it will mutate
  bool hard_pipe_heights_generations = true;
  int generation_cap = 256;
  int generation_size = 8;
  int generation_num = 0;
  int genome_num = 0;

  vector<float> parent1;
  vector<float> parent2;
  vector<float> parent_mutated;
  int genome_fitness;
  int parent1_fitness = 0;
  int parent2_fitness = 0;
  float parent_genome_choice;
  int parent_choice;

  float mutation_offset_perc;
  float mutation_offset;

std::random_device rseed;
std::mt19937 rgen(rseed()); // mersenne_twister
std::uniform_real_distribution<double> pipe_gen_dist(0+pipe_min_len,width-pipe_min_len-gap_len);//use pipe_gen_dist(rgen) to get rand
std::uniform_real_distribution<float> mutate_gen_dist(0, 1);

class Player{
  public:
    int x;
    int y;
};

class Game{
  public:
    enum states {idle, playing, over};
};

void print_3d_vector(vector<vector<vector<float> > > vec){
  for (int x=0;x<vec.size();x++){
    cout << x << endl;
    for (int y=0;y<vec[x].size();y++){
      cout << endl << "    " << y << endl << "        ";
      for(int z=0;z<vec[x][y].size();z++){
        if (z % 5 == 0){
          cout << endl <<  "        ";
        }else{
          cout << ", ";
        }
        cout << vec[x][y][z];
      }
    }
    cout << endl;
  }
}

void print_2d_vector(vector<vector<float> > vec){
  for (int x=0;x<vec.size();x++){
    cout << x << endl << "   ";
    for (int y=0;y<vec[x].size();y++){
      if (y % 5 == 0){
        cout << endl;
      }else{
        if (y > 0){
          cout << ", ";
        }
      }
      cout << vec[x][y];
    }
    cout << endl;
  }
}

void print_1d_vector(vector<float> vec){
  for (int x=0;x<vec.size();x++){
    if (x % 5 == 0){
      printf("\n%i: ", x/5);
    }
    printf("%f, ", vec[x]);
    //cout << vec[x] << ", ";
  }
  cout << endl;
}

int to_int(float num){
  return floor(num+0.5);
}

float get_output(float position, float velocity, float next_pipe_dist, float next_pipe_height, int genome_num, int generation_num){
  //Ok so we've got all the inputs for it to make it's decision and append to the genome
  /*
   * Position(y)
   * Velocity
   * Next pipe
   * Height of lower pipe section
   */
  /*
   * [
   *  [[1, 2, 3, 4, .6], [fitness]], //Genome 1 example
   *  [[1, 2, 4, 4, .3], [fitness]], //Genome 2 example
   *  [max_fitness]
   * ]
   */
  //if there are no genomes, then use mutate_gen_dist for all of them
  //We have to keep making them as it goes until it dies, then we actually record the fitness and make the next one in the generation
  //we are absolutely calling the last one Lord_Genome.
  //
  //the return of this function determines it's move in the current situation, which means we have to compare against all the other
  //rules in this generation before returning, assuming of course it's not in need of a random output for the current situation
  /*
   * this shouldn't be determined by generation, instead we are going to have it look through all the rules it has already made in it's own
   * genome, and if it finds a match, then do that. If it doesn't, then just randomly generate.
   */

  //Loop through all the rules we have for the current genome
  for (int ruleset_iter=5;ruleset_iter < generations[generation_num][genome_num].size();ruleset_iter+=5){
    rule_position = generations[generation_num][genome_num][ruleset_iter-5];
    rule_velocity = generations[generation_num][genome_num][ruleset_iter-4];
    rule_next_pipe_dist = generations[generation_num][genome_num][ruleset_iter-3];
    rule_next_pipe_height = generations[generation_num][genome_num][ruleset_iter-2];
    rule_output = generations[generation_num][genome_num][ruleset_iter-1];
    if (rule_position == position && rule_velocity == velocity && rule_next_pipe_dist == next_pipe_dist && rule_next_pipe_height == next_pipe_height){
      //Looks like we've got a match! Return the respective output
      //printf("GENERATION %i, GENOME %i\n", generation_num, genome_num);
      /*
      printf("\tRule values\t: %f, %f, %f, %f, %f\n", rule_position, rule_velocity, rule_next_pipe_dist, rule_next_pipe_height, rule_output);
      printf("\tCurrent values\t: %f, %f, %f, %f\n", position, velocity, next_pipe_dist, next_pipe_height);
      printf("MATCHING INPUTS FOUND - ");
      if (rule_output > jump_threshold){
        printf("JUMPING\n");
      }else{
        printf("NOT JUMPING\n");
      }
      */
      //We still push back so we can not have discrepancies in length because of following rules
      generations[generation_num][genome_num].push_back(position);
      generations[generation_num][genome_num].push_back(velocity);
      generations[generation_num][genome_num].push_back(next_pipe_dist);
      generations[generation_num][genome_num].push_back(next_pipe_height);
      generations[generation_num][genome_num].push_back(output);
      return rule_output;
    }
  }
  /*
  printf("Generating random output\n");
  printf("\tRule values\t: %f, %f, %f, %f, %f\n", rule_position, rule_velocity, rule_next_pipe_dist, rule_next_pipe_height, rule_output);
  printf("\tCurrent values\t: %f, %f, %f, %f\n", position, velocity, next_pipe_dist, next_pipe_height);
  */
  //There were no matches if we get to this point in the function
  //Push the current inputs to the genome and generate random output
  output = mutate_gen_dist(rgen);
  generations[generation_num][genome_num].push_back(position);
  generations[generation_num][genome_num].push_back(velocity);
  generations[generation_num][genome_num].push_back(next_pipe_dist);
  generations[generation_num][genome_num].push_back(next_pipe_height);
  generations[generation_num][genome_num].push_back(output);
  return output;

  /*
  if (generation_num > 0){
    output = mutate_gen_dist(rgen);
    //genome.clear();
    //genome = push_to_genome(genome, position, velocity, next_pipe_dist, next_pipe_height);
    return output;
  }else{
    //First generation, this means we randomly generate; Do we consider the rules generated at all?.
    output = mutate_gen_dist(rgen);
    generations[generation_num][genome_num].push_back(position);
    generations[generation_num][genome_num].push_back(velocity);
    generations[generation_num][genome_num].push_back(next_pipe_dist);
    generations[generation_num][genome_num].push_back(next_pipe_height);
    generations[generation_num][genome_num].push_back(output);
    return output;
  }
  */
}

void record_fitness(int generation_num, int genome_num, float fitness){
  generations[generation_num][genome_num].push_back({fitness});
}

float mutate_output(float output){
  //The value obtained is converted to a percentage and used to get a rand offset for addition/subtraction
  mutation_offset_perc = mutate_gen_dist(rgen);
  if (mutate_gen_dist(rgen) > .5){
    //We add, get the offset relative to the interval [output, 1]
    mutation_offset = (1-output)*mutation_offset_perc;
    return output+mutation_offset;
  }else{
    //We subtract, get the offset relative to the interval [0, output]
    mutation_offset = output*mutation_offset_perc;
    return output-mutation_offset;
  }
}

void get_generation(int generation_num){
  /*
   * If the generation given is not the first, then get the highest two fitness genomes from the previous generation 
   * Then make mutated children
   * Everyone loves mutated children
   */
  if (generation_num > 0){
    //Get the best two parents
    for (int genome_iter=0;genome_iter < generation_size;genome_iter++){
      //Check the fitness of each genome we go through
      genome_fitness = generations[generation_num-1][genome_iter][generations[generation_num-1][genome_iter].size()-1];
      //Have to account for the length of the ruleset
      //  We could use the length of the genome but this can end up being skewed I think.
      //  As well as the fact that it includes the fitness of the genome at the end which we don't need.
      //  It also doesn't increase as the size of the genome increases
      //  Do we even need to *5 this?
      //    no we really don't
      //genome_fitness = genome_fitness*5;

      /*
      if (genome_fitness >= parent1_fitness){
        //parent1_fitness is always >= parent2_fitness
        parent2_fitness = parent1_fitness;
        parent1_fitness = genome_fitness;
        parent2 = parent1;
        parent1 = generations[generation_num-1][genome_iter];
      }
      */
      if (genome_fitness >= parent1_fitness){
        parent1_fitness = genome_fitness;
        parent2_fitness = genome_fitness;
        parent1 = generations[generation_num-1][genome_iter];
        parent2 = parent1;
      }
    }

    //Store the best two parents in our new generation
    generations[generation_num][0] = parent1;
    generations[generation_num][1] = parent2;

    //Now we go and take random rules from each parent to initially create unmutated children
    //  We set it to 2 so that it doesn't count the parents as their own children
    for (int child_num = 2; child_num < generation_size; child_num++){
      //Start this with 5 so we can look back to mutate the output for this ruleset
      for (int ruleset_iter = 5; ruleset_iter < parent1_fitness; ruleset_iter+=5){
        //Loop through every 5 elements to get each set, we want to loop through the length of the one with greater fitness since this way we can continue to get values from the longer one even after we go past the length of the shorter parent
        //cout << "Iter: " << ruleset_iter << " Parent 1 fitness: " << parent1_fitness << endl;
        parent_genome_choice = mutate_gen_dist(rgen);
        if (ruleset_iter <= parent2_fitness){
          //If we can still get a value from parent2 to make this child then do so
          //Simple flip of coin rgen, take the last five values in this genome and append them to our new genome
          if (parent_genome_choice > parent1_threshold){
            //parent1
            //cout << "Parent 1 chosen" << endl;
            parent_choice = 1;
          }else{
            //parent2
            //cout << "Parent 2 chosen" << endl;
            parent_choice = 2;
          }
        }else{
          //Otherwise just get the value from parent1
          //cout << "Parent 1 defaulted" << endl;
          parent_choice = 1;
        }
        //cout << "Setting parent_mutated" << endl;
        if (parent_choice == 1){
          parent_mutated.resize(parent1.size());
          parent_mutated = parent1;
        }else{
          parent_mutated.resize(parent2.size());
          parent_mutated = parent2;
        }
        //Now that we've decided what parent we will be taking this ruleset from, we decide to mutate or not
        //  Either way, we put it in parent_mutated for usage in the push_back code.
        //  For now just mutating the output value.
        if (mutate_gen_dist(rgen) > mutation_threshold){
          //Going to make it tilted towards not mutating, because I mad scientist so cool.
          //Call me professor x - or ciruttai apparently lmao
          //Mutate - for now we are going to add/subtract a random offset from the value,
          //  however you could also do this by just replacing the value with an entirely new random one.
          cout << "Mutating" << endl;
          parent_mutated[ruleset_iter-1] = mutate_output(parent_mutated[ruleset_iter-1]);
        }
        //Add the mutated/not mutated ruleset to the new genome  
        //cout << "Pre-push" << endl;
        for (int sub_ruleset_iter = ruleset_iter-5;sub_ruleset_iter<ruleset_iter;sub_ruleset_iter++){
          //if ruleset_iter = 10, we do 5, 6, 7, 8, 9
          //cout << "Pushing" << endl;
          //k so it's only pushing 10 values to this for some reason, it's also not pushing all of them if it's telling the truth in the print
          printf("Pushing: %i:%f\n", sub_ruleset_iter, parent_mutated[sub_ruleset_iter]);
          generations[generation_num][child_num].push_back(parent_mutated[sub_ruleset_iter]);
        }
        //cout << "Post-push" << endl;
      }
      //lets see if we really are using the same one evry tim
      print_1d_vector(parent_mutated);
      print_1d_vector(generations[generation_num][child_num]);
    }
    //Temporary for testing
    /*
    generations[generation_num][2] = parent1;
    generations[generation_num][3] = parent1;
    generations[generation_num][4] = parent1;
    generations[generation_num][5] = parent1;
    generations[generation_num][6] = parent1;
    generations[generation_num][7] = parent1;
    */

    parent1_fitness = 0;
    parent2_fitness = 0;
    //Swap so we clear the memory allocated
    vector<float>().swap(parent1);
    vector<float>().swap(parent2);
    vector<float>().swap(parent_mutated);
  }
}

SDL_Rect new_rect(int x, int y, int w, int h){
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  return rect;
}



vector<float> push_to_genome(vector<float> genome, float position, float velocity, float next_pipe_dist, float output){
  genome.push_back(position);
  genome.push_back(velocity);
  genome.push_back(next_pipe_dist);
  genome.push_back(next_pipe_height);
  genome.push_back(output);
  return genome;
}

template<typename T>
std::ostream & operator<<(std::ostream & os, std::vector<T> vec){
  os<<"{ ";
  std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(os, " "));
  os<<"}";
  return os;
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

vector<vector<string> > gen_pipes(vector<vector<string> > grid, int last_pipe_x, float fitness){
  //We also want to record the new pipe height if this is the first genome, can't have those environmental discrepancies
  //Otherwise we use the pipe height from past genomes (of this generation)
  //pipe2_start is the next_pipe_height


  //Account for the disparency between fitness and pipes, since fitness is gonna be 32 * the pipe number every time this function is called
  //it's actually like 1, 32, 63, ... n+31, so we do /32(2*gap_len)-1 in order to get a nice 0, 1, 2...
  /*
  if (fitness > 1){
    fitness /= (2*gap_len);
  }
  */
  fitness = (fitness-1)/((2*gap_len)-1);
  if (fitness >= pipe_heights.size()){
    //If we havent made it this far in the generation yet, then generate new ones and add them

    //Make new set of pipes at right of screen
    pipe1_end = floor(pipe_gen_dist(rgen));
    pipe_heights.push_back(pipe1_end);
  }else{
    //If we have already been this far before, use the previous pipe heights
    pipe1_end = pipe_heights[to_int(fitness)];
  }
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
     "Jumpy Dot II - The Return of the Dot", SDL_WINDOWPOS_UNDEFINED,
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

  generations.resize(generation_cap);
  grid.resize(height);
  for (int i = 0; i < height; ++i) {
    grid[i].resize(width);
  }

  while (1){
    //Basically the generation loop
    genome_num = 0;

    //Because memory
    if (!(hard_pipe_heights_generations)){
      vector<int>().swap(pipe_heights);
    }

    generations[generation_num].resize(generation_size);
    get_generation(generation_num);
    //print_2d_vector(generations[generation_num]);
    cout << "Starting Generation " << generation_num << endl;
    while (1){//so that it will always restart
      cout << "\tGenome " << genome_num << endl;// ": ";
      //Basically the genome loop
      // Clear window
      SDL_RenderClear(renderer);

      //Begin execution
      fitness = 0;
      score = 0;

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
      //game_state = Game::idle;
      game_state = Game::playing;//having this makes it start automatically

      jump_y = player.y;
      jump_time = 1;
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
          //Update fitness every block
          fitness++;

          if (fitness > max_fitness){
            max_fitness = fitness;
          }

          //Update Score if just got free of a pipe
          if (grid[player.x][0] == "W" && grid[player.x-1][0] == "B"){
            score++;
            if (score > high_score){
              high_score = score;
            }
            //cout << score << ":" << high_score << endl;
            cout << "\t\tScore: " << score << "\tHigh Score: " << high_score << endl;
          }
          grid = move_background_west(grid);

          //pipe gen / handling
          last_pipe_x--;
          if (grid.size()-last_pipe_x >= pipe_dist){
            grid = gen_pipes(grid, last_pipe_x, fitness);
            last_pipe_x = grid.size()-1;
          }

          //Increment values since last jump
          jump_time++;

          //For velocity
          old_position = position;

          //Calculate their position, then subtract it from player.y(inverted graph)
          position = .5*acceleration*(pow(jump_time, 2)) + init_velocity*jump_time + jump_x;

          //Using the change in position we can find the velocity
          velocity = position-old_position;

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
          
        //Update inputs for bot
        for(int x=player.x;x<grid.size();x++){
          if (grid[x][0] == "B"){
            next_pipe_dist = x-player.x;
            for (int y=grid.size()-1;y>-1;y--){
              //Now that we know where the pipe is, search upwards until we find the beginning of the gap, and record the y+1 as the height.
              if (grid[x][y] == "W"){
                next_pipe_height = y+1;
                break;
              }
            }
          }
        }

        if (get_output(position, velocity, next_pipe_dist, next_pipe_height, genome_num, generation_num) > jump_threshold){
          //If the bot returns output greater than .9 then jump.
          printf("Jumping at %f\n", fitness-1);//actually jumps at the fitness before
          if (game_state != Game::playing){
            game_state = Game::playing;
          }
          jump_y = player.y;
          jump_time = 1;
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
              jump_y = player.y;
              jump_time = 1;
            }
          }
        }
        update_display(grid, renderer);
      }
      position = 64;
      velocity = 0;
      next_pipe_dist = 0;
      next_pipe_height = 0;
      last_pipe_x = 0;
      printf("\t\tFitness: %f\n", fitness);
      //cout << fitness << " fitness." << endl;// << generations[generation_num][genome_num].size() << " length." << endl;
      record_fitness(generation_num, genome_num, fitness);
      genome_num++;
      if (genome_num == generation_size){
        //print_2d_vector(generations[0]);
        break;
      }
    }
    generation_num++;
    //print_3d_vector(generation);
  }
}
