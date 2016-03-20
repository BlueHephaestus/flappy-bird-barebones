##Description:
This is an extremely barebones [Flappy Bird](http://flappybird.io/) clone, made for AIs using NEAT (Neuroevolution of Augmenting Topologies), or other Genetic Algorithms. A good example of one of these algorithms in action can be found [here](https://www.youtube.com/watch?v=qv6UVOQ0F44). 

I made the physics and settings so that I could barely play it, then I increased the speed to maximum. Hopefully, this will allow a bot to learn how to play it faster, instead of having to spend a large portion of time waiting for the pace of the game.

Feel free to tinker with any of the config settings at the top of the file, good luck, and have fun!

##Usage:
You will need the SDL and SDL2 libraries, and as a result you may have to edit the includes path according to your system. After that, just do
`g++ main.cpp -lSDL2 -std=c++11` 
`./a.out`
To compile and run it.

The controls are as follows:
q -> Close window and quit.
Space -> Jump and restart if you have died.

###In-Game Screenshot:
[!screenshot](https://github.com/DarkElement75/flappy-bird-barebones/screenshot.png)

