
#include "View.h"
#include "Log.h"
#include "Tcl.h"

#include "UbuntuMono.ttf.h"

#include <vector>
#include <SDL.h>
#include <stb_truetype.h>

namespace view {

  static const tcl::Variable FONT_SIZE("view_fontSize", 15);

  struct Glyph
  {
    SDL_Surface * surface = nullptr;
    int x = 0;
    int y = 0;
  };

  struct Cell
  {
    int ch = 0;
    uint16_t fg = COLOR_DEFAULT;
    uint16_t bg = COLOR_DEFAULT;
  };

  static SDL_Window * _window = nullptr;
  
  static stbtt_fontinfo _font;
  static int _fontBaseline;
  static int _fontLineHeight;
  static int _fontAdvance;

  static int _width = 0;
  static int _height = 0;
  static uint16_t _clearForeground = COLOR_DEFAULT;
  static uint16_t _clearBackground = COLOR_DEFAULT;

  static std::vector<Glyph> _glyphCache;
  static std::vector<Cell> _cells;

  static bool initializeFont();

  bool init(int preferredWidth, int preferredHeight, const char * title)
  {
    int error = SDL_InitSubSystem(SDL_INIT_VIDEO);
    if (error == -1)
    {
      logError("Could not initialize SDL");
      return false;
    }

    if (!initializeFont())
      return false;

    _window = SDL_CreateWindow(title,
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               _fontAdvance * preferredWidth, _fontLineHeight * preferredHeight,
                               SDL_WINDOW_RESIZABLE);

    if (_window == NULL)
    {
      logError("Could not create SDL window: ", SDL_GetError());
      SDL_Quit();
      return false;
    }

    _width = preferredWidth;
    _height = preferredHeight;

    _cells.resize(_width * _height);

    return true;
  }

  void shutdown()
  {
    SDL_DestroyWindow(_window);
    SDL_Quit();
  }

  void setCursor(int x, int y)
  {

  }

  void hideCursor()
  {

  }

  void setClearAttributes(uint16_t fg, uint16_t bg)
  {
    _clearForeground = fg;
    _clearBackground = bg;
  }

  void clear()
  {
    for (uint32_t i = 0; i < _cells.size(); ++i)
    {
      Cell & cell = _cells[i];
      cell.ch = ' ';
      cell.fg = _clearForeground;
      cell.bg = _clearBackground;
    }
  }

  void changeCell(int x, int y, uint32_t ch, uint16_t fg, uint16_t bg)
  {
    if (x < 0 || x >= _width || y < 0 || y >= _height)
      return;

    int idx = (y * _width) + x;
    Cell & cell = _cells[idx];
    cell.ch = ch;
    cell.fg = fg;
    cell.bg = bg;
  }

  int width()
  {
    return _width;
  }

  int height()
  {
    return _height;
  }

  void present()
  {
    SDL_Surface * screen = SDL_GetWindowSurface(_window);

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

    int xPos = 0;
    int yPos = 0;

    for (int y = 0; y < _height; ++y)
    {
      for (int x = 0; x < _width; ++x)
      {
        Cell & cell = _cells[y * _width + x];
        if (cell.ch < _glyphCache.size())
        {
          SDL_Rect rect;
          Glyph & glyph = _glyphCache[cell.ch];

          if (cell.bg & COLOR_REVERSE)
          {
            rect.x = xPos;
            rect.y = yPos;
            rect.w = _fontAdvance;
            rect.h = _fontLineHeight;
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255, 255, 255));
          }

          rect.x = xPos + glyph.x;
          rect.y = yPos + _fontBaseline + glyph.y;

          if (cell.fg & COLOR_REVERSE)
            SDL_SetSurfaceColorMod(glyph.surface, 0, 0, 0);
          else
            SDL_SetSurfaceColorMod(glyph.surface, 255, 255, 255);

          SDL_BlitSurface(glyph.surface, nullptr, screen, &rect);
        }


        xPos += _fontAdvance;
      }

      xPos = 0;
      yPos += _fontLineHeight;
    }

    SDL_UpdateWindowSurface(_window);
  }

  inline Keys toKeys(SDL_Keysym symb)
  {
    if (symb.mod & KMOD_CTRL)
    {
      switch (symb.sym)
      {
        case SDLK_2: return KEY_CTRL_2;
        case SDLK_a: return KEY_CTRL_A;
        case SDLK_b: return KEY_CTRL_B;
        case SDLK_c: return KEY_CTRL_C;
        case SDLK_d: return KEY_CTRL_D;
        case SDLK_e: return KEY_CTRL_E;
        case SDLK_f: return KEY_CTRL_F;
        case SDLK_g: return KEY_CTRL_G;
        case SDLK_h: return KEY_CTRL_H;
        case SDLK_i: return KEY_CTRL_I;
        case SDLK_j: return KEY_CTRL_J;
        case SDLK_k: return KEY_CTRL_K;
        case SDLK_l: return KEY_CTRL_L;
        case SDLK_m: return KEY_CTRL_M;
        case SDLK_n: return KEY_CTRL_N;
        case SDLK_o: return KEY_CTRL_O;
        case SDLK_p: return KEY_CTRL_P;
        case SDLK_q: return KEY_CTRL_Q;
        case SDLK_r: return KEY_CTRL_R;
        case SDLK_s: return KEY_CTRL_S;
        case SDLK_t: return KEY_CTRL_T;
        case SDLK_u: return KEY_CTRL_U;
        case SDLK_v: return KEY_CTRL_V;
        case SDLK_w: return KEY_CTRL_W;
        case SDLK_x: return KEY_CTRL_X;
        case SDLK_y: return KEY_CTRL_Y;
        case SDLK_z: return KEY_CTRL_Z;
        case SDLK_3: return KEY_CTRL_3;
        case SDLK_4: return KEY_CTRL_4;
        case SDLK_5: return KEY_CTRL_5;
        case SDLK_6: return KEY_CTRL_6;
        case SDLK_7: return KEY_CTRL_7;
        case SDLK_8: return KEY_CTRL_8;
      }
    }
    else
    {
      switch (symb.sym)
      {
        case SDLK_F1: return KEY_F1;
        case SDLK_F2: return KEY_F2;
        case SDLK_F3: return KEY_F3;
        case SDLK_F4: return KEY_F4;
        case SDLK_F5: return KEY_F5;
        case SDLK_F6: return KEY_F6;
        case SDLK_F7: return KEY_F7;
        case SDLK_F8: return KEY_F8;
        case SDLK_F9: return KEY_F9;
        case SDLK_F10: return KEY_F10;
        case SDLK_F11: return KEY_F11;
        case SDLK_F12: return KEY_F12;
        case SDLK_INSERT: return KEY_INSERT;
        case SDLK_DELETE: return KEY_DELETE;
        case SDLK_HOME: return KEY_HOME;
        case SDLK_END: return KEY_END;
        case SDLK_PAGEUP: return KEY_PGUP;
        case SDLK_PAGEDOWN: return KEY_PGDN;
        case SDLK_UP: return KEY_ARROW_UP;
        case SDLK_DOWN: return KEY_ARROW_DOWN;
        case SDLK_LEFT: return KEY_ARROW_LEFT;
        case SDLK_RIGHT: return KEY_ARROW_RIGHT;
        case SDLK_BACKSPACE: return KEY_BACKSPACE;
        case SDLK_TAB: return KEY_TAB;
        case SDLK_RETURN: return KEY_ENTER;
        case SDLK_ESCAPE: return KEY_ESC;
        //case SDLK_SPACE: return KEY_SPACE;
      }
    }

    return KEY_NONE;
  }

  bool peekEvent(Event * event, int timeout)
  {
    SDL_Event sdlEvent;

    event->type = EVENT_NONE;
    event->key = KEY_NONE;
    event->ch = 0;

    if (SDL_WaitEventTimeout(&sdlEvent, timeout))
    {
      switch (sdlEvent.type)
      {
        case SDL_QUIT:
          event->type = EVENT_QUIT;
          break;

        case SDL_WINDOWEVENT:
          {
            if (sdlEvent.window.event == SDL_WINDOWEVENT_RESIZED)
            {
              _width = sdlEvent.window.data1 / _fontAdvance;
              _height = sdlEvent.window.data2 / _fontLineHeight;

              if (_width == 0) _width = 1;
              if (_height == 0) _height = 1;

              _cells.resize(_width * _height);

              event->type = EVENT_RESIZE;
            }
          }
          break;

        case SDL_KEYDOWN:
          {
            event->type = EVENT_KEY;
            event->key = toKeys(sdlEvent.key.keysym);
          }
          break;

        case SDL_TEXTINPUT:
          {
            event->type = EVENT_KEY;
            event->ch = sdlEvent.text.text[0];
          }
          break;
      }     
    }

    return event->type != EVENT_NONE;
  }

  static void initGlyph(int ch, float scale)
  {
    Glyph glyph;
    int width, height;

    unsigned char * pixels = stbtt_GetCodepointBitmap(&_font, scale, scale, ch, &width, &height, &glyph.x, &glyph.y);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    uint32_t rmask = 0xff000000;
    uint32_t gmask = 0x00ff0000;
    uint32_t bmask = 0x0000ff00;
    uint32_t amask = 0x000000ff;
#else
    uint32_t rmask = 0x000000ff;
    uint32_t gmask = 0x0000ff00;
    uint32_t bmask = 0x00ff0000;
    uint32_t amask = 0xff000000;
#endif

    glyph.surface = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);
    SDL_SetSurfaceBlendMode(glyph.surface, SDL_BLENDMODE_BLEND);

    SDL_LockSurface(glyph.surface);

    for (int y = 0; y < height; ++y)
      for (int x = 0; x < width; ++x)
      {
        const int idx = y * width + x;

        int * p = (int *)glyph.surface->pixels;
        p[idx] = SDL_MapRGBA(glyph.surface->format, 255, 255, 255, pixels[idx]);
      }

    SDL_UnlockSurface(glyph.surface);
    stbtt_FreeBitmap(pixels, nullptr);

    if (ch >= _glyphCache.size())
      _glyphCache.resize(ch + 1);

    _glyphCache[ch] = glyph;
  }

  static bool initializeFont()
  {
    if (stbtt_InitFont(&_font, (unsigned char *)UbuntuMono, 0) == 0)
    {
      logError("Could not initialize the font");
      return false;
    }

    // Font size
    int ascent, decent, lineGap, advance;

    float scale = stbtt_ScaleForPixelHeight(&_font, FONT_SIZE.toInt());

    stbtt_GetFontVMetrics(&_font, &ascent, &decent, &lineGap);
    stbtt_GetCodepointHMetrics(&_font, '0', &advance, nullptr);

    _fontBaseline = ascent * scale;
    _fontLineHeight = (ascent - decent + lineGap) * scale;
    _fontAdvance = advance * scale;

    // Initialize some default glyphs
    const char * defaultGlyphs = " 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,.-;:()=+-*/!\"'#$%&{[]}<>|";
    while (*defaultGlyphs)
    {
      initGlyph(*defaultGlyphs, scale);
      ++defaultGlyphs;
    }

    return true;
  }



}
