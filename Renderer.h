#pragma once



#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include "Shapes.h"

template <class T>
inline std::string to_string(T num)
{
    std::string h;
    std::stringstream ss;
    ss << num;
    ss >> h;
    return h;
}


class Renderder
{
protected:
    SDL_Texture* texture = nullptr;
    SDL_Renderer* renderer = nullptr;

    vec2 pos;
    int h,w;
public:
    void center_vertically_between(int top, int bottom)
    {
        int lent_y = bottom - top;
        pos.y = top + (lent_y - w) / 2;
        pos.y = top + (lent_y - w) / 2;
    }

    void center_horizontally_between(int left, int right)
    {
        int lent_x = right - left;
        pos.x = left + (lent_x - w) / 2;
        pos.x = left + (lent_x - w) / 2;
    }

    virtual double get(std::string key) = 0;
    virtual void set(std::string key, double value) = 0;
};

///                             TEXT RENDERER
class text_renderer : public Renderder
{
public:
    enum TYPE { WRAPPED, LINEAR } type;
private:
    std::string font_name;
    uint16_t wrapper_length;
    bool can_reload = true;
    std::string text;
    SDL_Color color;
    TTF_Font* font = nullptr;
    int font_size = 0;

    void init(std::string font_name = "", int fs = 14)
    {
        Load_font(font_name.c_str(), fs);
        Load_text(text);
    }

public:

    text_renderer(SDL_Renderer* renderer, std::string font_name, int font_size, std::string text, SDL_Color color = { 0,0,255,255 }, int x = 100, int y = 100, TYPE type = LINEAR, uint16_t wrapper_length = 250)
        :text(text), color(color), type(type), wrapper_length(wrapper_length), font_size(font_size), font_name(font_name)
    {
        this->renderer = renderer;
        this->pos = { (double)x,(double)y };
        init(font_name, font_size);

    }

    text_renderer(SDL_Renderer* renderer, std::string text, SDL_Color color = { 0,0,255,255 }, int x = 100, int y = 100, TYPE type = LINEAR, uint16_t wrapper_length = 250)
        :text(text), color(color), type(type), wrapper_length(wrapper_length), font_size(font_size), font_name(font_name)
    {
        this->renderer = renderer;
        this->pos = { (double)x,(double)y };
        init();
    }


    bool Load_font(const char* fontpath, uint16_t fontsize = 14)
    {
        
        auto _font = TTF_OpenFont(fontpath, fontsize);
        if (!_font)
        {
            std::cout << "Can't Open font " << SDL_GetError() << std::endl;
            return false;
        }
        else
            font = _font;
        return true;

    }

    bool is_text_active()
    {
        return (this->font) ? true : false;
    }

    /** \brief loads a text texture
        * \return the size of loaded texture
        *
        */

    vec2 Load_text(std::string _text)
    {
        if (_text != text)
        {
            text = _text;
            if (text.empty() && texture)
                SDL_DestroyTexture(texture);
            can_reload = true;
        }

        if (can_reload && !text.empty() && font)// help prevent unnecessary loading of texture
        {
            if (font)
            {
                
                // load text either linear or wrapped
                SDL_Surface* surface = (type == LINEAR) ? TTF_RenderText_Blended(font, text.c_str(), color) : TTF_RenderText_Blended_Wrapped(font, text.c_str(), color, wrapper_length);
                if (!surface)
                {
                    std::cout << "Error-SURFACE: " + std::string(SDL_GetError()) << std::endl;
                    w = h = 0;
                    return vec2((double)w, (double)h);
                }
                else
                {
                    if (texture != nullptr)// free texture memory b/f assigning a new one
                        SDL_DestroyTexture(texture);

                    texture = SDL_CreateTextureFromSurface(renderer, surface);
                    if (!texture)
                    {
                        std::cout << "Error-TEXTURE: " + std::string(SDL_GetError()) << std::endl;
                        texture = nullptr;          w = h = 0;
                    }
                    else
                    {
                        SDL_QueryTexture(texture, 0, 0, &w, &h);
                        can_reload = false;
                    }
                    SDL_FreeSurface(surface);
                    return vec2((double)w, (double)h);
                }
            }
        }
        return vec2((double)w, (double)h);
    }

    vec2 Load_text(std::string text, SDL_Color cl)
    {
        color = cl;
        return Load_text(text);
    }

    void clear()
    {
        text = "";
        w = h = 0;
        if (texture) SDL_DestroyTexture(texture);
        if (font) TTF_CloseFont(font);

        texture = nullptr;
    }

    void Render()
    {
        if (texture && font)
        {
            SDL_Rect view_port = { pos.x,pos.y,w,h };
            SDL_RenderCopy(renderer, texture,NULL, &view_port);
        }
    }

    double get(std::string key) override
    {
        if (key == "x")
            return pos.x;
        else if (key == "y")
            return pos.y;
        else if (key == "w")
            return w;
        else if (key == "h")
            return h;
        else if ((key == "font size") || (key == "font_size") && (font))
            return font_size;
        else return -1;
    }

    void set(std::string key, double value) override
    {
        if (key == "x")
            pos.x = value;
        else if (key == "y")
            pos.y = value;
        else if (key == "font size" || key == "font_size")
        {
            Load_font(font_name.c_str(), value);
        }
    }

    void set_type(TYPE type, uint16_t length = 0)
    {
        wrapper_length = (length > 0) ? length : wrapper_length;
        this->type = type;
    }
};



///                                     IMAGE RENDERER

class image_renderer : public Renderder
{
    std::string path;
public:

    image_renderer(SDL_Renderer* renderer, std::string path, vec2 _pos, vec2 size)
    {
        this->renderer = renderer;
        Load_image(path, _pos, size);
    }

    vec2 Load_image(std::string path, vec2 _pos, vec2 size)
    {
        this->pos = _pos;
        w = size.w;
        h = size.h;
        this->texture = NULL;
        // load image
        SDL_Surface* surface = IMG_Load(path.c_str());

        if (!surface)
            std::cout << SDL_GetError();

        texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!texture)
        {
            std::cout << "Can't Create Texture " << SDL_GetError() << std::endl;
            return { 0,0 };
        }
        else
        {
            SDL_FreeSurface(surface);
            SDL_QueryTexture(texture, NULL, NULL, &w, &h);
            return { (double)w,(double)h };
        }
    }

    void Render()
    {
        if (texture)
        {
            SDL_Rect display = { pos.x,pos.y,w,h };
            SDL_RenderCopy(renderer, texture, NULL, &display);
        }
    }

    double get(std::string key) override
    {
        if (key == "x")
            return pos.x;
        else if (key == "y")
            return pos.y;
        else if (key == "w")
            return w;
        else if (key == "h")
            return h;
        else return -1;
    }

    void set(std::string key, double value) override
    {
        if (key == "x")
            pos.x = value;
        else if (key == "y")
            pos.y = value;
    }

};



