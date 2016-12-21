#include "FightingPokemonLayer.h"
#include "SimpleAudioEngine.h"
#include "head.h"
#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
extern vector<pair<string, string>> initPokemonMap;
USING_NS_CC;
// 构造函数，如有对象成员，需要在此初始化
FightingPokemonLayer::FightingPokemonLayer() {}
// 析构
FightingPokemonLayer::~FightingPokemonLayer(){}

//虚函数，初始化，当执行本类的create()函数时会被自动调用
bool FightingPokemonLayer::init(){
	bool bl = Layer::init();
	this->setAnchorPoint(Vec2(0.5, 0.5));
	isAnimationPlaying = false;
	return bl;
}

void FightingPokemonLayer::update(float) {
	if (isAnimationPlaying) {
		sprite->setOpacity(opacity);
		if (opacity >= 255)
			direction = -1;
		else if (opacity <= 35)
			direction = 1;
		opacity += direction * 5;
	}
	else {
		if (opacity != 255) {
			sprite->setOpacity(255);
			opacity = 255;
		}
	}
}

void FightingPokemonLayer::setInitParameters(std::string name, char inittype, bool upDownFlip) {
	//进场特效？
	this->upDownFlip = upDownFlip;
	this->sprite = Sprite::create(initPokemonMap[inittype - 1].second);
	if (upDownFlip)
		sprite->setFlippedX(true);
	sprite->setScale(100.0 / sprite->getContentSize().width);
	sprite->setPosition(Vec2(0, 0));
	this->addChild(sprite, 1);
	this->name = Label::createWithTTF(name, "fonts/simyou.ttf", 15);
	this->name->enableOutline(Color4B::BLACK, 1);
	this->name->setTextColor(Color4B::WHITE);
	this->name->setPosition(Vec2(0, -80));
	this->addChild(this->name, 2);
	hpbar = ui::LoadingBar::create("HP.png");
	hpbar->setPercent(100);
	hpbar->setDirection(ui::LoadingBar::Direction::LEFT);
	hpbar->setScale(100.0 / hpbar->getContentSize().width, 10.0 / hpbar->getContentSize().height);
	hpbar->setPosition(Vec2(0, 80));
	this->addChild(hpbar, 5);
	this->scheduleUpdate();
}

void FightingPokemonLayer::playMoveOnAnimation(bool play) {
	isAnimationPlaying = play;
}
//重写
Rect FightingPokemonLayer::getBoundingBox() const {
	auto temp = sprite->getBoundingBox();
	temp.origin.x = this->getPosition().x - temp.size.width / 2;
	temp.origin.y = this->getPosition().y - temp.size.height / 2;
	return temp;
}

void FightingPokemonLayer::startFlash() {
	this->schedule(schedule_selector(FightingPokemonLayer::flash), 0.06f, 20, 0);
}

void FightingPokemonLayer::flash(float) {
	if (sprite->getOpacity() <= 255)
		sprite->setOpacity(255);
	else sprite->setOpacity(128);
}

void FightingPokemonLayer::setHPbar(int percent, bool isDifference) {
	targetHP = isDifference ? hpbar->getPercent() + percent : percent;
	this->schedule(schedule_selector(FightingPokemonLayer::changeHP), 0.025f, abs(hpbar->getPercent() - percent), 0);
}

void FightingPokemonLayer::changeHP(float) {
	int hp = hpbar->getPercent();
	if (hp < targetHP)
		hpbar->setPercent(++hp);
	else if (hp > targetHP)
		hpbar->setPercent(--hp);
}

void FightingPokemonLayer::exit() {
	this->unscheduleUpdate();
	this->schedule(schedule_selector(FightingPokemonLayer::changeOpacity), 0.025f, 2000, 0);
}

void FightingPokemonLayer::changeOpacity(float) {
	GLubyte m = sprite->getOpacity();
	if (m > 0) {
		sprite->setOpacity(--m);
	}
	else this->setVisible(false);
}