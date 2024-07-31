/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "PokerMainScene.h"
#include "cocostudio/SimpleAudioEngine.h"

Scene* PokerMainScene::createScene()
{
    return PokerMainScene::create();
}


// on "init" you need to initialize your instance
bool PokerMainScene::init()
{
    if ( !Scene::init() )
    {
        return false;
    }
    visibleSize = Director::getInstance()->getVisibleSize();
    origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////

    auto bg = cocos2d::LayerColor::create(Color4B(62, 99, 68, 255));
    this->addChild(bg, -1);

    coinNode = Node::create();
    this->addChild(coinNode,10);
    slotNode = Node::create();
    this->addChild(slotNode,9);

    auto label = Label::createWithTTF("Reset", "fonts/arial.ttf", 8);
    auto reset = MenuItemLabel::create(label,CC_CALLBACK_1(PokerMainScene::didResetClicked, this));
    auto menu = Menu::create(reset, NULL);
    menu->setPosition(Vec2(origin.x + visibleSize.width - reset->getContentSize().width/2 - 2,
                            origin.y + visibleSize.height - reset->getContentSize().height));
    this->addChild(menu);

    // Setup Coin panel
    setupCoinPanel();

    // Setup Table Layout
    setupTable();

    return true;
}

void PokerMainScene::setupTable() {

    Vector<MenuItem*> menuItems;
    for (int i = 0; i < 6; i++) {
        menuItems.pushBack(getSlot());
    }

    auto menu = Menu::createWithArray(menuItems);
    menu->alignItemsHorizontallyWithPadding(2);
    menu->setPosition(Vec2(origin.x + visibleSize.width/2 ,
                           origin.y + visibleSize.height/2));

// *1.5 to prevent overlapping button on menu auto align Items In columns.
    menu->setContentSize(Size(visibleSize.width*1.5, visibleSize.height));
    menu->alignItemsInColumns(2,2,2, NULL);
    slotNode->addChild(menu);

}

void PokerMainScene::setupCoinPanel() {

    auto coin1 = getMenuItem(1);
    auto coin10 = getMenuItem(10);
    auto coin100 = getMenuItem(100);
    auto coin1k = getMenuItem(1000);
    auto coin5k = getMenuItem(5000);

    selectedCoin = coin1;

    auto menu = Menu::create(coin1,coin10,coin100,coin1k,coin5k, NULL);
    menu->alignItemsHorizontallyWithPadding(2);
    menu->setPosition(Vec2(origin.x + visibleSize.width/2,origin.y + coin1->getContentSize().height/2 + 5));
    this->addChild(menu, 1);

    coinPointer = DrawNode::create();
    Vec2 triangle[3];
    triangle[0] = Vec2(0, 0);
    triangle[1] = Vec2(-0.5, -1);
    triangle[2] = Vec2(0.5, -1);
    Color4F white(1, 1, 1, 1);
    coinPointer->drawPolygon(triangle, 3, white, 1, white);
    coinPointer->setPosition(Vec2(selectedCoin->getPositionX(), selectedCoin->getPositionY() - selectedCoin->getContentSize().height / 4 - 3));

    menu->addChild(coinPointer);
    auto action = Sequence::create(
            MoveBy::create(0.4,Vec2(0,-2)),
            MoveBy::create(0.4,Vec2(0,2)), NULL);

    coinPointer->runAction(RepeatForever::create(action));
}

MenuItemImage* PokerMainScene::getSlot() {
    auto slot = MenuItemImage::create(
            "rect.png",
            "rect.png",
            CC_CALLBACK_1(PokerMainScene::didSlotClicked, this));

    auto label = Label::createWithTTF("0", "fonts/arial.ttf", 8);
    label->setTextColor(Color4B(0,0,0,255));
    label->setPosition(Vec2(slot->getContentSize().width/2,-3));
    label->setName("LABEL");
    slot->addChild(label);
    return slot;
}

MenuItemImage* PokerMainScene::getMenuItem(int amount) {

    auto coin = MenuItemImage::create(
            "btn_betgear_1.png",
            "btn_betgear_1_2.png",
            CC_CALLBACK_1(PokerMainScene::didCoinClicked, this));

    auto label = Label::createWithTTF(getDisplayAmount(amount), "fonts/arial.ttf", 16);
    label->setTextColor(Color4B(137, 89, 48, 255));
    label->setPosition(coin->getContentSize().width / 2, coin->getContentSize().height / 2);
    coin->addChild(label);
    coin->setScale(0.5);
    coin->setTag(amount);

    return coin;
}

void PokerMainScene::didResetClicked(Ref* pSender) {
//    Reset coin in table and reset slot status
//    Keep coin selection for next bet
    coinNode->removeAllChildren();
    slotNode->removeAllChildren();
    setupTable();
}

void PokerMainScene::didSlotClicked(Ref* pSender) {
    auto slot = static_cast<MenuItemImage*>(pSender);
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/chips_to_table.wav",false,0,0,0);

    auto orgPos = coinPointer->getParent()->convertToWorldSpace(coinPointer->getPosition());
    // re-position the coin y to selected coin y
    orgPos.y = orgPos.y + selectedCoin->getContentSize().height/2;

    auto coin = Sprite::create("coin.png");
    coin->setPosition(orgPos);
    coinNode->addChild(coin);

    auto coinLabel = Label::createWithTTF(getDisplayAmount(selectedCoin->getTag()), "fonts/arial.ttf", 5);
    coinLabel->setTextColor(Color4B(137, 89, 48, 255));
    coinLabel->setPosition(coin->getContentSize().width / 2, coin->getContentSize().height / 2);
    coin->addChild(coinLabel);

    auto destPos = slot->getParent()->convertToWorldSpace(Vec2(
            slot->getPosition().x + random(-slot->getContentSize().width/2 +2,slot->getContentSize().width/2 -2),
            slot->getPosition().y + random(-slot->getContentSize().height/2 +2,slot->getContentSize().height/2 -2)));

    auto action = Spawn::create(
            RotateBy::create(0.4f,random(540,720)),
            MoveTo::create(0.2f,destPos), NULL);

    coin->runAction(action);

    auto label = static_cast<Label*>(slot->getChildByName("LABEL"));

    int value = std::stoi(label->getString());
    value += selectedCoin->getTag();
    label->setString(std::to_string(value));

}

void PokerMainScene::didCoinClicked(Ref* pSender)
{
    auto coin = static_cast<MenuItemImage*>(pSender);
    selectedCoin = coin;
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/chips_to_table.wav",false,0,0,0);
    coinPointer->setPosition(Vec2(coin->getPositionX(), coinPointer->getPositionY()));
}

std::string PokerMainScene::getDisplayAmount(int amount) {
    switch (amount) {
        case 1:
            return "1";
        case 10:
            return "10";
        case 100:
            return "100";
        case 1000:
            return "1k";
        case 5000:
            return "5k";
        default:
            return "";
    }
}
