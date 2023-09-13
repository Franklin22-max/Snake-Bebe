#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>
#include <stdint.h>
#include <math.h>
#include <vector>
#include <list>
#include <memory>
#include <deque>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>


#include "ShadeShapes.h"
#include "Renderer.h"
#include "SoundStream.h"

#define PI 3.141592653
#define eulers_K 2.7182182

std::string strmap =
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxE"
"x                                                                              xE"
"x            xxxxx                                                             xE"
"x                                                                              xE"
"x                                                                              xE"
"x        x        xxxxxxxxx                  xxxxxxxxxxxxxxxxxxx               xE"
"x        x                                                     x               xE"
"x        x                      x                              x               xE"
"x        x                      x                              x               xE"
"x        x                      x                              x               xE"
"x        x                      x                              x               xE"
"x                               x                              x               xE"
"x                               x       xxxxxxxxxxxxxxx        x      xxx      xE"
"x                               x                              x               xE"
"x        x                      x                              x               xE"
"x        x                      x                              x               xE"
"x        x                                                     x               xE"
"x        x                                                     x               xE"
"x        x                                                     x               xE"
"x               xxxxxxxxxxxxxxxxxxxxxxxxxxx                                    xE"
"x                                                                              xE"
"x                                                                              xE"
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxE";

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Event event;

TTF_Font* font = nullptr;
unsigned int window_w;
unsigned int window_h;

bool isRunning = false;

vec2 mouse_pos;
float delta_time = 0;
float FPS = 60;


/*
	using HRC_time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;
	std::chrono::high_resolution_clock HRC;
	float delta = 0.f;
	uint32_t frame_time = 0;
	uint32_t start_time = 0;
*/


struct SnakeGame
{
	//					TIMER
	int coin_placement_timer = 0;
	int coin_removal_timer = 0;
	int fps_display_timer = 0;

	//					STATE
	int progess = 0;
	enum class STATE { stoped, playing, paused } game_state = STATE::stoped;

	//					WALLS
	/**
		because map could have a lot of walls to be looped, this helps to map certing locations in the world to a range of indexes of the wall contaner
	*/
	struct MapRange
	{
		int start_index = 0;
		int end_index = 0;

		int range_start = 0;
		int range_end = 0;
	};

	std::vector<MapRange> track_book; 
	std::vector<Rect> walls;

	//					SNAKE
	struct Snake {
		//					TURNING POINT
		struct TurningPoint {

			vec2 point;
			vec2 prev_dir;
			vec2 dir;
		};

		struct Head {
			Circle circle;
			vec2 dir;
		} head;

		Snake()
		{
			head.circle.pos = { 100,100 };
			head.circle.r = 8;
			head.dir = { 1,0 };
		}

		int lenght = 0;
		std::vector<Rect> body_segments;
		std::deque<TurningPoint> turning_points;
	} snake;

	float speed = 60;

	//						COIN
	struct Coin
	{
		enum class Type { green, blue, red } type;
		int remaing_time; // in milliseconds
		int gain;
		SDL_Color cl;
		Circle circle;
	};

	struct GreenCoin : Coin
	{
		GreenCoin() {
			type = Coin::Type::green;
			remaing_time = 20000;
			circle.r = 6;
			cl = { 0,255,0,255 };
			gain = 10;
		}
	};
	
	struct RedCoin : Coin
	{
		RedCoin()
		{
			type = Coin::Type::red;
			remaing_time = 50000;
			circle.r = 10;
			cl = { 255,0,0,255 };
			gain = -100;
		}
	};

	struct BlueCoin : Coin
	{
		BlueCoin()
		{
			type = Coin::Type::blue;
			remaing_time = 10000;
			circle.r = 10;
			cl = { 0,0,255,255 };
			gain = 100;
		}
	};

	std::list<Coin> coins;

	int Top_score = 0;
	//					Text
	text_renderer* info;
	text_renderer* best_score;
	text_renderer* my_score;

	//					Sound
	SoundStream sstream;


	//					MAKE MAP
	void make_map(const std::string& strmap)
	{
		int char_width = 0;
		int char_height = 0;


		for (int i = 0; i < strmap.length(); i++)
		{
			if (strmap[i] == 'E')
				char_height++;

			if (char_height == 0)
				char_width++;
		}


		int active_heignt = window_h - 30;

		int rect_w = window_w / char_width;
		int rect_h = active_heignt / char_height;

		int accumulated_width = 0;
		int accumulated_height = 0;


		for (int i = 0; i < strmap.length(); i++)
		{
			if (strmap[i] != 'E')
			{
				if (strmap[i] == 'x')
				{
					SDL_Rect rect = { accumulated_width, 30 + accumulated_height, rect_w, rect_h };
					walls.emplace_back(rect);
				}
				accumulated_width += rect_w;
			}
			else
			{
				accumulated_width = 0;
				accumulated_height += rect_h;
			}
		}
	}

	//			CHANGE STATE
	void change_state(STATE new_state)
	{
		if (game_state == STATE::stoped && new_state == STATE::playing)
		{
			my_score->Load_text("My Score: 0");

			info->Load_text("Space to Pause, Esc to stop", { 10,80,20,255 });
			info->center_horizontally_between(0, window_w);
		}
		else if (new_state == STATE::stoped)
		{
			if (snake.lenght > Top_score)
			{
				Top_score = snake.lenght;
				best_score->Load_text("Best Score: " + to_string(Top_score));
				std::fstream f;
				f.open("snake.skg", std::ios::out | std::ios::trunc);

				if (f.is_open())
					f << snake.lenght;
				else
					std::cout << "couldn't update top score\n";
				f.close();

				info->Load_text("New High Score", { 150,140,0,255 });
				info->center_horizontally_between(0,window_w);
				sstream.play_sound("highscore",2,0);
			}
			else
			{
				info->Load_text("Your Dead", { 120,20,20,255 });
				info->center_horizontally_between(0, window_w);
				sstream.play_sound("die", 2, 0);
			}

			
			coins.erase(coins.begin(), coins.end());
			snake = Snake();
		}
		game_state = new_state;
	}


	//			NEW GAME


	//			MAKE A TURN
	void make_a_turn(vec2 dir)
	{
		if (snake.lenght > 0)
		{
			Snake::TurningPoint tp;
			tp.prev_dir = snake.head.dir;
			tp.dir = snake.head.dir = dir;
			tp.point = snake.head.circle.pos;
			snake.turning_points.push_front(tp);
		}
		else
			snake.head.dir = dir;
	}















	void OnInit()
	{
		make_map(strmap);
		snake.body_segments.reserve(10);

		std::fstream f;
		f.open("snake.skg", std::ios::in);

		if (f.is_open())
			f  >> Top_score;
		else
			std::cout << "couldn't load in top score\n";
		f.close();

		info = new text_renderer(renderer,"./fonts/BOOKOSB.ttf", 20, "Space To Play", {10,80,20,255}, 350, 5);
		best_score = new text_renderer(renderer,"./fonts/BOOKOS.ttf", 20, "Best Score: "+to_string(Top_score), {0,0,20,255}, 30, 5);
		my_score = new text_renderer(renderer,"./fonts/BOOKOS.ttf", 20, "My Score: 0", {0,0,20,255}, 630, 5);
		

		info->center_horizontally_between(0, window_w);
		
		sstream.load_sound("eat", "./Sounds/eat-sound.mp3");
		sstream.load_sound("theme1", "./Sounds/main_theme1.OGG");
		sstream.load_sound("die", "./Sounds/eat-bad.mp3");
		sstream.load_sound("highscore", "./Sounds/success-fanfare.mp3");

		sstream.set_volume("theme1",68);
		sstream.play_sound("theme1", 0);
	}



	void Update()
	{
		if (game_state == STATE::playing)
		{
			// update the position of snake
			snake.head.circle.pos.x += snake.head.dir.x * delta_time * speed;
			snake.head.circle.pos.y += snake.head.dir.y * delta_time * speed;

			if (snake.turning_points.size() == 0)
			{
				snake.body_segments.erase(snake.body_segments.begin(), snake.body_segments.end());
				snake.body_segments.push_back(rect_between_points(snake.head.circle.pos, snake.head.circle.pos - snake.head.dir * snake.lenght, 12));
			}
			else
			{
				snake.body_segments.erase(snake.body_segments.begin(), snake.body_segments.end());
				bool should_pop = false;
				int length = snake.lenght;

				for (int i = 0; i < snake.turning_points.size(); i++)
				{
					vec2 P1 = (i == 0) ? snake.head.circle.pos : snake.turning_points[i - 1].point;

					double l = get_lenght( P1, snake.turning_points[i].point);
					
					if (l <= length)
						snake.body_segments.push_back(rect_between_points(P1, snake.turning_points[i].point, 12));
					else
					{
						snake.body_segments.push_back(rect_between_points(P1, P1 - snake.turning_points[i].dir * length, 12));
						should_pop = true;
					}

					length -= l;

					if( i == snake.turning_points.size() - 1 && length > 0)
						snake.body_segments.push_back(rect_between_points(snake.turning_points[i].point, snake.turning_points[i].point - snake.turning_points[i].prev_dir * length, 12));
				}

				if (should_pop)
					snake.turning_points.pop_back();
			}


			// randomly place coins at 10 seconds intervals
			if (SDL_GetTicks() - coin_placement_timer > 5000)
			{
				int random_num = rand() % 100;

				// 80% chance of getting green coins
				if (random_num <= 80)
				{
					auto coin = GreenCoin();
					coin.circle.pos.x = 30 + rand() % (window_w - 60);
					coin.circle.pos.y = 40 + rand() % (window_h - 60);
					coins.push_back(coin);
				}
				else if (random_num > 80 && random_num <= 90)// 10% chance
				{
					auto coin = BlueCoin();
					coin.circle.pos.x = 30 + rand() % (window_w - 60);
					coin.circle.pos.y = 40 + rand() % (window_h - 60);
					coins.push_back(coin);
				}
				else if (random_num > 90 && random_num <= 100)// 10% chance
				{
					auto coin = RedCoin();
					coin.circle.pos.x = 30 + rand() % (window_w - 60);
					coin.circle.pos.y = 40 + rand() % (window_h - 60);
					coins.push_back(coin);
				}

				// ensure coin is not colliding with any wall
				for (auto& wall : walls)
				{
					auto last_coin = --coins.end();
					is_circle_polygon_colliding(last_coin->circle, wall, true);
				}
				

				coin_placement_timer = SDL_GetTicks();
			}


			// remove coin if its time ellapses every second
			if (SDL_GetTicks() - coin_removal_timer > 1000)
			{
				std::list<Coin>::iterator coin_to_remove = coins.end();
				for (auto coin = coins.begin(); coin != coins.end(); ++coin)
				{
					coin->remaing_time -= SDL_GetTicks() - coin_removal_timer;

					if (coin->remaing_time <= 0)
						coin_to_remove = coin;
				}

				if (coin_to_remove != coins.end())
					coins.erase(coin_to_remove);

				coin_removal_timer = SDL_GetTicks();
			}



			//	handle collision of snake head with wall
			for (auto& wall : walls)
			{
				if (is_circle_polygon_colliding(snake.head.circle,wall))
				{
					// close game failed
					change_state(STATE::stoped);
				}
			}

			std::list<Coin>::iterator coin_to_remove = coins.end();

			//	handle collision of snake head with coin
			for (auto coin = coins.begin(); coin != coins.end(); ++coin)
			{
				if (is_circle_circle_colliding(snake.head.circle, coin->circle))
				{
					progess += coin->gain;
					coin_to_remove = coin;
					if (coin->type == Coin::Type::red)
						change_state(STATE::stoped);
					else
					{
						sstream.play_sound("eat", 1, 0);
						// add new body segments to snakes body
						snake.lenght += (coin->type == Coin::Type::green) ? 20 : 60;
						my_score->Load_text("My Score: " + to_string(snake.lenght));
					}

				}
			}

			if (coin_to_remove != coins.end() && coins.size() > 0)
				coins.erase(coin_to_remove);

			// handle collision of snake head with it's body
			for (auto& seg : snake.body_segments)
			{
				if (is_point_inside_rect(seg, snake.head.circle.pos + snake.head.dir * snake.head.circle.r))
					change_state(STATE::stoped);
					
			}

		}

	}


	void Render()
	{
		// renderer clear
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 170, 170, 170, 255);
		SDL_RenderClear(renderer);

		// draw walls
		for (auto& wall : walls)
		{
			SDL_Rect r = { wall.edges[0].x, wall.edges[0].y, wall.edges[2].x - wall.edges[0].x, wall.edges[2].y - wall.edges[0].y };

			SDL_SetRenderDrawColor(renderer, 10, 0, 30, 255);
			SDL_RenderFillRect(renderer, &r);
		}

		SDL_Rect r;

		// draw coins
		for (auto& coin : coins)
		{
			SDL_Texture* texture = fillCircle(renderer, coin.circle, coin.cl, r);
			SDL_RenderCopy(renderer, texture, NULL, &r);
			SDL_DestroyTexture(texture);
		}

	
		// draw snake body
		for (auto& seg : snake.body_segments)
		{
			SDL_SetRenderDrawColor(renderer, 100, 150, 60, 255);
			SDL_Rect r = { seg.edges[0].x, seg.edges[0].y, seg.edges[2].x - seg.edges[0].x, seg.edges[2].y - seg.edges[0].y };
			SDL_RenderFillRect(renderer, &r);
		}

		// fill the empty space at turning points
		for (auto& p : snake.turning_points)
		{
			Circle c;
			c.r = 6;
			c.pos = p.point;
			SDL_Texture* texture = fillCircle(renderer, c, { 100, 150, 60, 255 }, r);
			SDL_RenderCopy(renderer, texture, NULL, &r);
			SDL_DestroyTexture(texture);
		}

		// draw snake head

		SDL_Texture* texture = fillCircle(renderer, snake.head.circle, { 100, 150, 60, 255 }, r);
		SDL_RenderCopy(renderer, texture, NULL, &r);
		SDL_DestroyTexture(texture);


		vec2 pos = snake.head.circle.pos;
		vec2 dir = snake.head.dir;
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawLine(renderer, pos.x, pos.y, pos.x + dir.x * snake.head.circle.r, pos.y + dir.y * snake.head.circle.r);

		// Draw Text
		my_score->Render();
		best_score->Render();
		info->Render();

		SDL_RenderPresent(renderer);
	}

	void Run()
	{

		while (isRunning)
		{
			int start_time = SDL_GetTicks();
			// event
			ManageEvent();
			// Update
			Update();
			// Render
			Render();
			//	timing
			delta_time = (SDL_GetTicks() - start_time) / 1000.f;

			if (SDL_GetTicks() - fps_display_timer > 1000)
			{
				std::string fps;
				std::stringstream ss;
				ss << 1000.f / (SDL_GetTicks() - start_time);
				ss >> fps;
				fps = "Snake Game " + fps;
				SDL_SetWindowTitle(window, fps.c_str());
				fps_display_timer = SDL_GetTicks();
			}

		}
	}















	//					LOAD FONT
	bool Loadfont(const char* fontpath, uint16_t fontsize = 14)
	{
		font = TTF_OpenFont(fontpath, fontsize);
		if (!font)
		{
			std::cout << "Can't Open font " << SDL_GetError() << std::endl;
			return false;
		}
	}


	//					LOAT TEXT
	SDL_Texture* Loadtext(const char* text, SDL_Color fg = { 200,200,255,255 }, SDL_Color bg = { 0,0,0,0 })
	{
		SDL_Surface* surface = TTF_RenderText(font, text, fg, bg);

		if (!surface)
		{
			std::cout << "Can't Create Surface " << SDL_GetError() << std::endl;
			return nullptr;
		}

		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		if (!texture)
		{
			std::cout << "Can't Create Texture " << SDL_GetError() << std::endl;
			return nullptr;
		}
		SDL_FreeSurface(surface);
		return texture;
	}


	//				LOAD IMAGE
	SDL_Texture* Loadimage(const char* path)
	{
		SDL_Surface* surface = IMG_Load(path);

		if (!surface)
			std::cout << SDL_GetError();

		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		if (!texture)
		{
			std::cout << "Can't Create Texture " << SDL_GetError() << std::endl;
			return nullptr;
		}

		SDL_FreeSurface(surface);
		return texture;
	}


	void ManageEvent()
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				isRunning = false;
				break;
			case SDL_MOUSEMOTION:
				mouse_pos.x = event.motion.x;
				mouse_pos.y = event.motion.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				case SDLK_LEFT:
					std::cout << "Left";
					make_a_turn({ -1 , 0 });
					break;
				case SDLK_RIGHT:
					std::cout << "right";
					make_a_turn({ 1 , 0 });
					break;
				case SDLK_UP:
					std::cout << "up";
					make_a_turn({ 0 , -1 });
					break;
				case SDLK_DOWN:
					std::cout << "down";
					make_a_turn({ 0 , 1 });
					break;
				case SDLK_SPACE:
					if (game_state == STATE::paused || game_state == STATE::stoped)
						change_state(STATE::playing);
					else
						change_state(STATE::paused);
					break;
				case SDLK_ESCAPE:
					change_state(STATE::stoped);
					break;
				default:
					break;
				}
			}
		}
	}

	SnakeGame(const char* app_name, unsigned int window__w, unsigned int window__h, unsigned int WindowFlag = SDL_WINDOW_SHOWN, unsigned int RendererFlag = SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED)
	{
		window_w = window__w;
		window_h = window__h;

		if (SDL_Init(SDL_INIT_EVERYTHING))
			exit(-1);

		if (!IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG))
			exit(-1);

		if (TTF_Init())
			exit(-1);

		if (!Mix_Init(MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_MID | MIX_INIT_OPUS))
		{
			std::cerr << "Can't Initialize Mixer " << SDL_GetError();
			exit(-1);
		}

		if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096) < 0)
		{
			std::cout << "couldnt open audio device\n";
		}


		window = SDL_CreateWindow(app_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_w, window_h, WindowFlag);
		if (!window)
		{
			std::cerr << "Can't Create Window " << SDL_GetError();
			exit(-1);
		}


		renderer = SDL_CreateRenderer(window, 1, RendererFlag);
		if (!renderer)
		{
			std::cerr << "Can't Create Renderer " << SDL_GetError();
			exit(-1);
		}

		isRunning = true;

		OnInit();
	}
};


int main(int argc, char** argv)
{
	SnakeGame test("Snake Game", 800, 500);
	test.Run();

	return 0;
}


