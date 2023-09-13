#pragma once

#include <iostream>
#include <stdint.h>
#include <math.h>
#include <vector>
#include <memory>
#include <map>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

struct SoundStream 
{
	
	std::map<std::string, Mix_Chunk*> sounds;

	/**
	* \param a unique id to identift the sound
	* \param file a file path from where to load music data.
	* \returns true if sound was properly loaded and false otherwise.
	*
	* \sa remove_sound
	*/
	bool load_sound(std::string uuid, std::string path)
	{
		Mix_Chunk* sound = Mix_LoadWAV(path.c_str());

		if (sound)
		{
			sounds[uuid] = sound;
			return true;
		}
		std::cout << "Couldn't Load Sound " + path << std::endl;
		return false;
	}


	bool remove_sound(std::string uuid)
	{
		auto node = sounds.find(uuid);
		if (node->second && node->first == uuid)
		{
			Mix_FreeChunk(node->second);
			sounds.erase(uuid);
			return true;
		}	
		return false;
	}


	int play_sound(std::string uuid, int channel = -1, uint16_t loop = -1)
	{
		auto node = sounds.find(uuid);
		if (node->second && node->first == uuid)
		{
			return Mix_PlayChannel(channel,node->second, loop);
		}
		return -1;
	}



	void pause_sound(int channel = -1)
	{
		Mix_Pause(channel);
	}

	void resume_sound(int channel = -1)
	{
		Mix_Resume(channel);
	}

	bool is_playing(int channel = -1)
	{
		return Mix_Playing(channel);
	}

	bool is_paused(int channel = -1)
	{
		return Mix_Paused(channel);
	}

	void set_volume( std::string uuid,int volumn)
	{
		auto node = sounds.find(uuid);
		if (node->second && node->first == uuid)
		{
			Mix_VolumeChunk(node->second, volumn);
		}
	}


};