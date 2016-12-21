
#ifndef _POKEMONDETAILS_H
#define _POKEMONDETAILS_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "proj.win32\head.h"

class PokemonDetailsLayer : public cocos2d::LayerColor {
public:
	PokemonDetailsLayer();
	~PokemonDetailsLayer();
	cocos2d::Label* nameLabel;

public:
	virtual bool init();
	cocos2d::ui::LoadingBar* expbar;
	cocos2d::ui::Button* okButton;
	cocos2d::Label* expbarLabel;
	cocos2d::Sprite* pokemonSprite;
	bool isChosen = false;
	CREATE_FUNC(PokemonDetailsLayer);
	void setPokemon(Pokemon* p);
	void setPokemon(vector<strengthPokemon>* tempv, vector<int>* tempi, string name, bool isExchangePokemon);
};

#endif 