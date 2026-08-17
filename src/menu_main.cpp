#include "menu_main.h"
#include "config.h"

MenuMain* MenuMain::instance = NULL;

MenuMain* MenuMain::getInstance() {
	if(!instance)
		instance = new MenuMain();
	return instance;
}

void MenuMain::cleanUpInstance() {
	if(instance) {
		delete instance;
		instance = NULL;
	}
}
MenuMain::MenuMain() {
		SDL_Surface *icon = Screen::loadImage("gfx/pacman_sdl_desktop.png", 0);
		SDL_SetWindowIcon(Screen::getInstance()->getWindow(), icon);
		SDL_FreeSurface(icon);
		// Full title banner (replaces "Pa" + face + "man"). Keep under ~600px wide.
		titleLogo = Screen::loadImage("gfx/full_title_img.png");
		std::string str_version = "version ";
		str_version.append(VERSION);
		version = Screen::getTextSurface(Screen::getSmallFont(), str_version.c_str(), Constants::GRAY_COLOR);
		addMenuItem("Quit");
		addMenuItem("About");
		addMenuItem("Highscore List");
		addMenuItem("Options");
		addMenuItem("Start Game");
		selection = STARTGAME;
		menuoptions = new MenuOptions();
		menuabout = new MenuAbout();
		draw();
}

MenuMain::~MenuMain() {
	SDL_FreeSurface(titleLogo);
	SDL_FreeSurface(version);
	delete menuoptions;
	delete menuabout;
}

void MenuMain::drawTitle() {
	// Native asset is 560x130 — fits the old Pa+face+man footprint with side margins.
	const int logoX = 320 - (titleLogo->w >> 1);
	const int logoY = 24;
	Screen::getInstance()->draw(titleLogo, logoX, logoY);
	Screen::getInstance()->draw(version, 320 - (version->w >> 1), logoY + titleLogo->h + 6);
}

int MenuMain::show() {
	GameController::getInstance()->searchAndOpen();
	draw();
	int event;
	while(!(event = eventloop())) {
		SDL_Delay(Constants::MIN_FRAME_DURATION);
		FunnyAnimation::getInstance()->animate();
	}
	FunnyAnimation::cleanUpInstance();
	return (event == 1 ? 1 : 0);
}

int MenuMain::handleSelection() {
	if(selection == STARTGAME)
		return 1;
	else if(selection == OPTIONS) {
		menuoptions->show();
		this->draw();
	}
	else if(selection == HIGHSCORE) {
		HighscoreList::getInstance()->load();
		HighscoreList::getInstance()->show(false, false);
		this->draw();
	}
	else if(selection == ABOUT) {
		menuabout->show();
		this->draw();
	}
	else if(selection == BACK)
		return 2;
	return 0;
}
