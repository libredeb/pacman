#include "screen.h"

TTF_Font *Screen::smallFont     = NULL;
TTF_Font *Screen::font          = NULL;
TTF_Font *Screen::largeFont     = NULL;
TTF_Font *Screen::veryLargeFont = NULL;
TTF_Font *Screen::hugeFont      = NULL;

Screen *Screen::instance = NULL;

namespace {
	inline int scaleToScreen(int value, float factor) {
		return (int)((float)value * factor);
	}
	inline int scaleToScreen(int value, float factor, int offset) {
		return (int)((float)value * factor + (float)offset);
	}
}

Screen *Screen::getInstance() {
	if (!instance) {
		instance = new Screen();
	}
	return instance;
}

void Screen::cleanUpInstance() {
	if (smallFont) {
		TTF_CloseFont(smallFont);
		smallFont = NULL;
	}
	if (font) {
		TTF_CloseFont(font);
		font = NULL;
	}
	if (largeFont) {
		TTF_CloseFont(largeFont);
		largeFont = NULL;
	}
	if (veryLargeFont) {
		TTF_CloseFont(veryLargeFont);
		veryLargeFont = NULL;
	}
	if (hugeFont) {
		TTF_CloseFont(hugeFont);
		hugeFont = NULL;
	}
	if (instance) {
		delete instance;
		instance = NULL;
	}
}

Screen::Screen():
	sdlInitErrorOccured(false),
	fullscreen(CommandLineOptions::exists("f","fullscreen")),
	rect_num(0),
	scalingFactor(1.0f),
	contentOffsetX(0),
	contentOffsetY(0),
	contentWidth(Constants::WINDOW_WIDTH),
	contentHeight(Constants::WINDOW_HEIGHT),
	visibleX(0),
	visibleY(0),
	visibleW(Constants::WINDOW_WIDTH),
	visibleH(Constants::WINDOW_HEIGHT)
{
	// Prefer nearest-neighbor scaling: cheaper on Pi Zero 2W and keeps pixel-art crisp.
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	// initialize SDL
	if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		std::cout << "SDL video initialization failed: " << SDL_GetError() << std::endl;
        sdlInitErrorOccured = true;
    }
	if(!sdlInitErrorOccured && TTF_Init() == -1) {
		std::cout << "TTF initialization failed: " << TTF_GetError() << std::endl;
        sdlInitErrorOccured = true;
	}
	if (!sdlInitErrorOccured) {
		window = SDL_CreateWindow("Pacman",
								  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                           		  Constants::WINDOW_WIDTH,
                           		  Constants::WINDOW_HEIGHT,
                           		  fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
		screen_surface = SDL_GetWindowSurface(window);
		computeClipRect();
		if(screen_surface == 0) {
			std::cout << "Setting video mode failed: " << SDL_GetError() << std::endl;
			sdlInitErrorOccured = true;
		} else {
			clearOutsideClipRect();
			addTotalUpdateRect();
			Refresh();
		}
	}
	atexit(Screen::cleanUpInstance);
}

Screen::~Screen() {
	TTF_Quit();
	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
}

void Screen::AddUpdateRects(int x, int y, int w, int h) {
	if (rect_num >= Constants::MAX_UPDATE_RECTS)
		return;  // prevent index out of bounds problems
	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}
	if (x + w > clipRect.w)
		w = clipRect.w - x;
	if (y + h > clipRect.h)
		h = clipRect.h - y;
	if (w <= 0 || h <= 0)
		return;
	rects[rect_num].x = (short int)scaleToScreen(x, scalingFactor, clipRect.x);
	rects[rect_num].y = (short int)scaleToScreen(y, scalingFactor, clipRect.y);
	rects[rect_num].w = (short int)scaleToScreen(w, scalingFactor);
	rects[rect_num].h = (short int)scaleToScreen(h, scalingFactor);
	rect_num++;
}

void Screen::addTotalUpdateRect() {
	rects[0].x = 0;
	rects[0].y = 0;
	rects[0].w = screen_surface->w;  // no scalingFactor as screen_surface already is the total screen surface
	rects[0].h = screen_surface->h;
	rect_num = 1;  // all other update rects will be included in this one
}

void Screen::addUpdateClipRect() {
	AddUpdateRects(0, 0, Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT);
}

void Screen::Refresh() {
	SDL_UpdateWindowSurfaceRects(window, rects, rect_num);
	rect_num = 0;
}

void Screen::draw_dynamic_content(SDL_Surface *surface, int x, int y) {
	SDL_Rect dest;
	dest.x = (short int)scaleToScreen(x, scalingFactor, clipRect.x);
	dest.y = (short int)scaleToScreen(y, scalingFactor, clipRect.y);
	dest.w = (short int)scaleToScreen(surface->w, scalingFactor);
	dest.h = (short int)scaleToScreen(surface->h, scalingFactor);
	if (isScaled()) {
		SDL_BlitScaled(surface, NULL, screen_surface, &dest);
	} else {
		SDL_BlitSurface(surface, NULL, screen_surface, &dest);
	}
	AddUpdateRects(x, y, surface->w + 10, surface->h);
}

void Screen::draw(SDL_Surface* graphic, int offset_x, int offset_y) {
    if (0 == offset_x && 0 == offset_y && 0 == clipRect.x && 0 == clipRect.y && !isScaled()) {
        SDL_BlitSurface(graphic, NULL, screen_surface, NULL);
    } else {
        SDL_Rect position;
        position.x = (short int)scaleToScreen(offset_x, scalingFactor, clipRect.x);
        position.y = (short int)scaleToScreen(offset_y, scalingFactor, clipRect.y);
		position.w = (short int)scaleToScreen(graphic->w, scalingFactor);
		position.h = (short int)scaleToScreen(graphic->h, scalingFactor);
		if (isScaled()) {
			SDL_BlitScaled(graphic, NULL, screen_surface, &position);
		} else {
			SDL_BlitSurface(graphic, NULL, screen_surface, &position);
		}
    }
}

void Screen::setFullscreen(bool fs) {
	if (fs == fullscreen) {
		return;  // the desired mode already has been activated, so do nothing
	}
	SDL_SetWindowSize(window, Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT);
	SDL_SetWindowPosition(window, 0, 0);
	if (fs) {
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	} else {
		SDL_SetWindowFullscreen(window, 0);
	}
	SDL_SetWindowSize(window, Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT);
	SDL_SetWindowPosition(window, 0, 0);
	SDL_Surface* newScreen = SDL_GetWindowSurface(window);
	if(newScreen) {
		screen_surface = newScreen;
		computeClipRect();
		clearOutsideClipRect();
		addTotalUpdateRect();
		fullscreen = fs;
	} else {
		if (fs) {
			std::cout << "Switching to fullscreen mode failed: " << SDL_GetError() << std::endl;
		} else {
			std::cout << "Switching from fullscreen mode failed: " << SDL_GetError() << std::endl;
		}
	}
}

SDL_Surface *Screen::loadImage(const char *filename, int transparentColor) {
	char filePath[256];
	getFilePath(filePath, filename);
	SDL_Surface *surface, *temp;
	temp = IMG_Load(filePath);
	if (!temp) {
		std::cout << "Unable to load image: " << IMG_GetError() << std::endl;
		exit(EXIT_FAILURE);
	}
	surface = SDL_ConvertSurface(temp,  Screen::getInstance()->getSurface()->format, 0);
	if (surface == NULL) {
		std::cout << "Unable to convert image to display format: " << SDL_GetError() << std::endl;
		exit(EXIT_FAILURE);
	}
	if (transparentColor != -1) {
		if (SDL_SetColorKey(surface, SDL_TRUE, (Uint32)SDL_MapRGB(surface->format, (uint8_t)transparentColor, (uint8_t)transparentColor, (uint8_t)transparentColor))) {
			std::cout << "Unable to set transparent color: " << SDL_GetError() << std::endl;
		}
	}
	if (SDL_SetSurfaceRLE(surface, SDL_TRUE) < 0) {
		std::cout << "Unable to enable RLE: " << SDL_GetError() << std::endl;
	}
	SDL_FreeSurface(temp);
	return surface;
}

TTF_Font *Screen::loadFont(const char *filename, int ptSize) {
	char filePath[256];
	getFilePath(filePath, filename);
	TTF_Font *font = TTF_OpenFont(filePath, ptSize);
	if (!font) {
		std::cout << "Unable to open TTF font: " << TTF_GetError() << std::endl;
		exit(EXIT_FAILURE);
	}
	return font;
}

SDL_Surface *Screen::getTextSurface(TTF_Font *font, const char *text, SDL_Color color) {
	SDL_Surface *temp = TTF_RenderText_Solid(font, text, color);
	if (!temp) {
		std::cout << "Unable to render text \"" << text << "\": " << TTF_GetError() << std::endl;
		exit(EXIT_FAILURE);
	}
	SDL_Surface *surface = SDL_ConvertSurface(temp,  Screen::getInstance()->getSurface()->format, 0);
	if (surface == NULL) {
		std::cout << "Unable to convert text surface to display format: " << SDL_GetError() << std::endl;
		exit(EXIT_FAILURE);
	}
	SDL_FreeSurface(temp);
	return surface;
}

void Screen::clear() {
	SDL_Rect rect = {0, 0, screen_surface->w, screen_surface->h};
	SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, 0, 0, 0));
}

void Screen::clearOutsideClipRect() {
	SDL_Rect rect;
	if (visibleX > 0) {
		rect.x = 0;
		rect.y = 0;
		rect.w = visibleX;
		rect.h = screen_surface->h;
		SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, 0, 0, 0));
	}
	if (visibleX + visibleW < screen_surface->w) {
		rect.x = visibleX + visibleW;
		rect.y = 0;
		rect.w = screen_surface->w - rect.x;
		rect.h = screen_surface->h;
		SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, 0, 0, 0));
	}
	if (visibleY > 0) {
		rect.x = visibleX;
		rect.y = 0;
		rect.w = visibleW;
		rect.h = visibleY;
		SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, 0, 0, 0));
	}
	if (visibleY + visibleH < screen_surface->h) {
		rect.x = visibleX;
		rect.y = visibleY + visibleH;
		rect.w = visibleW;
		rect.h = screen_surface->h - rect.y;
		SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, 0, 0, 0));
	}
}

void Screen::fillRect(SDL_Rect *rect, Uint8 r, Uint8 g, Uint8 b) {
	if (0 == clipRect.x && 0 == clipRect.y && !isScaled()) {
		SDL_FillRect(screen_surface, rect, SDL_MapRGB(screen_surface->format, r, g, b));
	} else {
		SDL_Rect rect_moved;
		rect_moved.x = scaleToScreen(rect->x, scalingFactor, clipRect.x);
		rect_moved.y = scaleToScreen(rect->y, scalingFactor, clipRect.y);
		rect_moved.w = scaleToScreen(rect->w, scalingFactor);
		rect_moved.h = scaleToScreen(rect->h, scalingFactor);
		SDL_FillRect(screen_surface, &rect_moved, SDL_MapRGB(screen_surface->format, r, g, b));
	}
}

TTF_Font *Screen::getSmallFont() {
	if (!smallFont)
		smallFont = loadFont("fonts/emulogic.ttf", 12);
	return smallFont;
}
TTF_Font *Screen::getFont() {
	if (!font)
		font = loadFont("fonts/emulogic.ttf", 20);
	return font;
}
TTF_Font *Screen::getLargeFont() {
	if (!largeFont)
		largeFont = loadFont("fonts/emulogic.ttf", 24);
	return largeFont;
}
TTF_Font *Screen::getVeryLargeFont() {
	if (!veryLargeFont)
		veryLargeFont = loadFont("fonts/emulogic.ttf", 48);
	return veryLargeFont;
}
TTF_Font *Screen::getHugeFont() {
	if (!hugeFont)
		hugeFont = loadFont("fonts/emulogic.ttf", 96);
	return hugeFont;
}

void Screen::setContentRect(int x, int y, int w, int h) {
	contentOffsetX = x;
	contentOffsetY = y;
	contentWidth = w > 0 ? w : Constants::WINDOW_WIDTH;
	contentHeight = h > 0 ? h : Constants::WINDOW_HEIGHT;
	computeClipRect();
	clearOutsideClipRect();
	addTotalUpdateRect();
	Refresh();
}

void Screen::resetContentRect() {
	setContentRect(0, 0, Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT);
}

void Screen::computeClipRect() {
	bool scaling_allowed = !CommandLineOptions::exists("","noscaling");
	bool centering_allowed = !CommandLineOptions::exists("","nocentering");
	clipRect.w = Constants::WINDOW_WIDTH;
	clipRect.h = Constants::WINDOW_HEIGHT;
	scalingFactor = 1.0f;

	if (scaling_allowed) {
		const float scalingX = (float)screen_surface->w / (float)contentWidth;
		const float scalingY = (float)screen_surface->h / (float)contentHeight;
		scalingFactor = scalingX < scalingY ? scalingX : scalingY;
		if (scalingFactor <= 0.0f) {
			scalingFactor = 1.0f;
		}
	}

	visibleW = scaleToScreen(contentWidth, scalingFactor);
	visibleH = scaleToScreen(contentHeight, scalingFactor);
	if (centering_allowed) {
		visibleX = (screen_surface->w - visibleW) / 2;
		visibleY = (screen_surface->h - visibleH) / 2;
		if (visibleX < 0)
			visibleX = 0;
		if (visibleY < 0)
			visibleY = 0;
	} else {
		visibleX = 0;
		visibleY = 0;
	}
	// Map logical (0,0) so that contentOffset lands at visibleX/visibleY.
	clipRect.x = visibleX - scaleToScreen(contentOffsetX, scalingFactor);
	clipRect.y = visibleY - scaleToScreen(contentOffsetY, scalingFactor);
}

int Screen::xToClipRect(int x) {
	return (int)((float)(x - Screen::getInstance()->getOffsetX()) / Screen::getInstance()->getScalingFactor());
}

int Screen::yToClipRect(int y) {
	return (int)((float)(y - Screen::getInstance()->getOffsetY()) / Screen::getInstance()->getScalingFactor());
}
