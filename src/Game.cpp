#include "Game.hpp"
#include <sstream>
#include <iomanip>
#include <random>
#include <iostream>
#include <cmath>

namespace {
const sf::Color BG(7,13,21), PANEL(12,21,31,242), PANEL2(18,30,43,238), TEXT(224,235,244), MUTED(137,158,176);
const sf::Color CYAN(91,210,255), GREEN(92,220,143), RED(255,92,107), GOLD(255,197,76), PURPLE(203,139,255), ORANGE(255,140,78);
const std::vector<TowerKind> towerOrder={TowerKind::Pulse,TowerKind::Cannon,TowerKind::Frost,TowerKind::Sniper,TowerKind::Tesla,TowerKind::Missile};

sf::FloatRect menuPlay(){return {610,470,380,62};}
sf::FloatRect menuTutorial(){return {610,548,380,62};}
sf::FloatRect menuExit(){return {610,626,380,62};}
sf::FloatRect backRect(){return {48,818,180,50};}
sf::FloatRect startWaveRect(){return {1230,72,340,54};}
sf::FloatRect speedRect(){return {1230,138,160,44};}
sf::FloatRect pauseRect(){return {1410,138,160,44};}
sf::FloatRect targetRect(){return {1230,448,164,42};}
sf::FloatRect sellRect(){return {1406,448,164,42};}
sf::FloatRect upgradeRect(){return {1230,500,340,46};}
sf::FloatRect branchARect(){return {1230,494,164,62};}
sf::FloatRect branchBRect(){return {1406,494,164,62};}

sf::FloatRect towerCardRect(int idx){
    int row=idx/2,col=idx%2; return {1230.f+col*174.f,565.f+row*104.f,164.f,94.f};
}
std::string money(int v){return std::to_string(v)+" C";}
}

Game::Game():window_(sf::VideoMode(WINDOW_W,WINDOW_H),"AEGIS DOMINION — Tactical Tower Defense",sf::Style::Titlebar|sf::Style::Close){
    window_.setFramerateLimit(120); window_.setVerticalSyncEnabled(true);
    if(!assets_.load("assets")) std::cerr<<"Einige Assets konnten nicht geladen werden. Starte das Spiel aus dem Projektordner.\n";
    std::mt19937 r{73}; std::uniform_real_distribution<float> x(0,WINDOW_W), y(0,WINDOW_H), sp(7,24), rr(1,3.5f);
    for(int i=0;i<90;++i) menuParticles_.push_back({{x(r),y(r)},{-sp(r)*.25f,sp(r)},rr(r),sf::Color(85,190,255,60)});
    waves_.reset(); map_.set(0);
}

int Game::run(){
    sf::Clock clock;
    while(window_.isOpen()){
        float dt=std::min(.035f,clock.restart().asSeconds());
        processEvents(); update(dt); render();
    }
    return 0;
}

void Game::processEvents(){
    sf::Event e{};
    while(window_.pollEvent(e)){
        if(e.type==sf::Event::Closed){window_.close();continue;}
        if(e.type==sf::Event::KeyPressed){
            if(state_==GameState::Playing){
                if(e.key.code==sf::Keyboard::Escape){ if(helpOverlay_)helpOverlay_=false; else paused_=!paused_; }
                if(e.key.code==sf::Keyboard::F1) helpOverlay_=!helpOverlay_;
                if(!gameOver_&&!victory_){
                    if(e.key.code==sf::Keyboard::Space) startNextWave();
                    if(e.key.code==sf::Keyboard::P) paused_=!paused_;
                    if(e.key.code==sf::Keyboard::F) speed_=speed_==1?2:1;
                    if(e.key.code==sf::Keyboard::U) upgradeSelected();
                    if(e.key.code==sf::Keyboard::T && selectedTower_>=0) towers_[selectedTower_]->cycleTargetMode();
                    if(e.key.code==sf::Keyboard::S) sellSelected();
                    if(e.key.code>=sf::Keyboard::Num1 && e.key.code<=sf::Keyboard::Num6) selectBuild(towerOrder[static_cast<int>(e.key.code-sf::Keyboard::Num1)]);
                }
            } else if(e.key.code==sf::Keyboard::Escape){ if(state_==GameState::Menu)window_.close(); else state_=GameState::Menu; }
        }
        if(e.type==sf::Event::MouseButtonPressed){
            sf::Vector2f p=window_.mapPixelToCoords({e.mouseButton.x,e.mouseButton.y});
            if(state_==GameState::Menu)processMenuClick(p);
            else if(state_==GameState::MapSelect)processMapSelectClick(p);
            else if(state_==GameState::Tutorial)processTutorialClick(p);
            else processGameplayClick(p,e.mouseButton.button);
        }
    }
}

void Game::update(float dt){
    elapsed_+=dt;
    for(auto& p:menuParticles_){ p.p+=p.v*dt; if(p.p.y>WINDOW_H+10){p.p.y=-10;p.p.x=std::fmod(p.p.x+213.f,WINDOW_W);} }
    if(state_==GameState::Playing) updateGameplay(dt);
}

void Game::render(){
    window_.clear(BG);
    switch(state_){case GameState::Menu:drawMenu();break;case GameState::MapSelect:drawMapSelect();break;case GameState::Tutorial:drawTutorial();break;case GameState::Playing:drawGameplay();break;}
    window_.display();
}

void Game::resetGameplay(){
    enemies_.clear();towers_.clear();projectiles_.clear();fx_.clear();waves_.reset();credits_=520;core_=20;score_=kills_=0;nextEnemyId_=1;speed_=1;paused_=false;helpOverlay_=false;gameOver_=victory_=false;selectedTower_=-1;buildKind_.reset();
}
void Game::startGame(int idx){ resetGameplay();map_.set(idx);state_=GameState::Playing;assets_.play("click",55); }

void Game::updateGameplay(float dt){
    if(paused_||helpOverlay_||gameOver_||victory_) { fx_.update(dt*.3f); return; }
    float step=dt*float(speed_);
    waves_.update(step);
    if(waves_.hasSpawn()){
        EnemyKind k=waves_.consumeSpawn(); auto e=makeEnemy(k,nextEnemyId_++,waves_.enemyScale()); e->setPathState(0,0,map_); enemies_.push_back(std::move(e));
    }
    for(auto& e:enemies_) e->update(step,map_);
    for(auto& t:towers_) t->update(step,enemies_,projectiles_,fx_,assets_);
    for(auto& p:projectiles_) p.update(step,enemies_,fx_);
    projectiles_.erase(std::remove_if(projectiles_.begin(),projectiles_.end(),[](auto& p){return !p.alive;}),projectiles_.end());

    std::vector<std::unique_ptr<Enemy>> splitChildren;
    for(auto& e:enemies_){
        if(e->reachedEnd()&&!e->dead()){
            int leak=e->kind()==EnemyKind::Boss?5:(e->kind()==EnemyKind::Tank?2:1); core_-=leak; fx_.text(e->position()+sf::Vector2f(-20,-30),"-"+std::to_string(leak)+" KERN",RED); e->kill(); assets_.play("explosion",35);
        } else if(e->dead()){
            credits_+=e->reward(); score_+=e->reward()*11; kills_++; fx_.text(e->position()+sf::Vector2f(-12,-28),"+"+std::to_string(e->reward()),GOLD); fx_.burst(e->position(),map_.data().accent,e->kind()==EnemyKind::Boss?38:16,e->kind()==EnemyKind::Boss?220:120);
            if(e->kind()==EnemyKind::Splitter){
                for(int i=0;i<2;++i){ auto child=makeEnemy(EnemyKind::Runner,nextEnemyId_++,std::max(.7f,waves_.enemyScale()*.72f)); child->setPathState(e->segment(),clampf(e->segmentT()+i*.018f,0.f,.98f),map_); splitChildren.push_back(std::move(child)); }
            }
        }
    }
    enemies_.erase(std::remove_if(enemies_.begin(),enemies_.end(),[](const auto& e){return e->dead();}),enemies_.end());
    for(auto& c:splitChildren) enemies_.push_back(std::move(c));

    if(core_<=0){ core_=0;gameOver_=true;assets_.play("gameover",65); }
    if(waves_.active() && waves_.spawningDone() && enemies_.empty()){
        int bonus=waves_.completionBonus(); credits_+=bonus;score_+=bonus*5;waves_.finishWave();fx_.text({520,105},"WELLE GESICHERT  +"+std::to_string(bonus)+" C",GREEN);assets_.play("upgrade",50);
        if(waves_.wave()>=waves_.maxWaves()){victory_=true;score_+=core_*250;}
    }
    fx_.update(step);
}

void Game::startNextWave(){ if(gameOver_||victory_||paused_)return; if(waves_.startNext()){assets_.play("wave",60);fx_.text({505,100},"WELLE "+std::to_string(waves_.wave())+" BEGINNT",CYAN);} }
void Game::selectBuild(TowerKind k){ buildKind_=k;selectedTower_=-1;assets_.play("click",38); }
std::vector<sf::Vector2f> Game::towerPositions()const{ std::vector<sf::Vector2f> p;for(auto& t:towers_)p.push_back(t->position());return p; }
int Game::towerAt(sf::Vector2f p)const{ for(int i=int(towers_.size())-1;i>=0;--i)if(distance(p,towers_[i]->position())<43.f)return i;return -1; }
void Game::sellSelected(){ if(selectedTower_<0||selectedTower_>=int(towers_.size()))return;credits_+=towers_[selectedTower_]->sellValue();fx_.burst(towers_[selectedTower_]->position(),GOLD,12,90);towers_.erase(towers_.begin()+selectedTower_);selectedTower_=-1;assets_.play("build",40); }
void Game::upgradeSelected(UpgradeBranch branch){
    if(selectedTower_<0 || selectedTower_>=int(towers_.size())) return;
    auto& t=*towers_[selectedTower_];
    if(t.maxLevel()) return;
    if(t.needsBranch() && branch==UpgradeBranch::None) return;
    int cost=t.upgradeCost();
    if(credits_<cost) return;
    if(t.upgrade(branch)){
        credits_-=cost;
        fx_.ring(t.position(),branch==UpgradeBranch::B?ORANGE:CYAN,70,.55f);
        fx_.text(t.position()+sf::Vector2f(-20,-50),"UPGRADE",GREEN);
        assets_.play("upgrade",58);
    }
}

void Game::processMenuClick(sf::Vector2f p){
    if(pointIn(menuPlay(),p)){state_=GameState::MapSelect;assets_.play("click",55);} else if(pointIn(menuTutorial(),p)){tutorialPage_=0;state_=GameState::Tutorial;assets_.play("click",55);} else if(pointIn(menuExit(),p))window_.close();
}
void Game::processMapSelectClick(sf::Vector2f p){
    for(int i=0;i<3;++i){sf::FloatRect r(70.f+i*510.f,220,440,500);if(pointIn(r,p)){startGame(i);return;}}
    if(pointIn(backRect(),p)){state_=GameState::Menu;assets_.play("click",50);}
}
void Game::processTutorialClick(sf::Vector2f p){
    sf::FloatRect prev(500,800,180,52),next(920,800,180,52),menu(700,800,200,52);
    if(pointIn(prev,p)&&tutorialPage_>0){tutorialPage_--;assets_.play("click",45);} else if(pointIn(next,p)&&tutorialPage_<5){tutorialPage_++;assets_.play("click",45);} else if(pointIn(menu,p)){state_=GameState::Menu;assets_.play("click",45);}
}

void Game::processGameplayClick(sf::Vector2f p,sf::Mouse::Button button){
    if(button==sf::Mouse::Right){buildKind_.reset();selectedTower_=-1;return;}
    if(button!=sf::Mouse::Left)return;
    if(gameOver_||victory_){
        sf::FloatRect again(555,560,220,54),maps(790,560,220,54),menu(673,630,220,48);
        if(pointIn(again,p))startGame(map_.index()); else if(pointIn(maps,p)){state_=GameState::MapSelect;resetGameplay();} else if(pointIn(menu,p)){state_=GameState::Menu;resetGameplay();} return;
    }
    if(helpOverlay_){ sf::FloatRect close(690,720,220,50); if(pointIn(close,p))helpOverlay_=false; return; }
    if(paused_){ sf::FloatRect resume(690,430,220,54),menu(690,500,220,50); if(pointIn(resume,p))paused_=false; else if(pointIn(menu,p)){state_=GameState::Menu;resetGameplay();} return; }

    if(p.x<WORLD_W){
        if(buildKind_){
            int cost=towerCost(*buildKind_); if(credits_>=cost && map_.canBuild(p,towerPositions())){credits_-=cost;towers_.push_back(makeTower(*buildKind_,p));fx_.ring(p,GREEN,65,.35f);assets_.play("build",52);} else assets_.play("click",25);
        } else { selectedTower_=towerAt(p); }
        return;
    }
    if(pointIn(startWaveRect(),p)){startNextWave();return;}
    if(pointIn(speedRect(),p)){speed_=speed_==1?2:1;assets_.play("click",45);return;}
    if(pointIn(pauseRect(),p)){paused_=true;assets_.play("click",45);return;}
    for(int i=0;i<6;++i)if(pointIn(towerCardRect(i),p)){selectBuild(towerOrder[i]);return;}
    if(selectedTower_>=0){
        auto& t=*towers_[selectedTower_];
        if(pointIn(targetRect(),p)){t.cycleTargetMode();assets_.play("click",40);return;}
        if(pointIn(sellRect(),p)){sellSelected();return;}
        if(t.needsBranch()){
            if(pointIn(branchARect(),p)){upgradeSelected(UpgradeBranch::A);return;}
            if(pointIn(branchBRect(),p)){upgradeSelected(UpgradeBranch::B);return;}
        } else if(pointIn(upgradeRect(),p)){upgradeSelected();return;}
    }
}

void Game::drawPanel(const sf::FloatRect& r,sf::Color fill,sf::Color outline,float thickness){sf::RectangleShape s({r.width,r.height});s.setPosition(r.left,r.top);s.setFillColor(fill);s.setOutlineColor(outline);s.setOutlineThickness(thickness);window_.draw(s);}
void Game::drawText(const std::string& s,unsigned size,sf::Vector2f p,sf::Color color,bool bold,bool centered){if(!assets_.text().loaded())return;sf::Text t=assets_.text().makeText(s,size);t.setFillColor(color);if(bold)t.setStyle(sf::Text::Bold);t.setOutlineColor(sf::Color(0,0,0,100));t.setOutlineThickness(size>=28?1.5f:0);if(centered){auto b=t.getLocalBounds();t.setOrigin(b.left+b.width/2,b.top+b.height/2);}t.setPosition(p);window_.draw(t);}
void Game::drawWrapped(const std::string& s,unsigned size,sf::FloatRect box,sf::Color color,float gap,bool bold){
    if(!assets_.text().loaded()) return;
    std::istringstream iss(s);
    std::string word,line;
    float y=box.top;
    while(iss>>word){
        std::string test=line.empty()?word:line+" "+word;
        sf::Text temp=assets_.text().makeText(test,size);
        if(temp.getLocalBounds().width>box.width && !line.empty()){
            drawText(line,size,{box.left,y},color,bold);
            y+=size+gap;
            line=word;
        } else line=test;
    }
    if(!line.empty()) drawText(line,size,{box.left,y},color,bold);
}
bool Game::drawButton(const sf::FloatRect& r,const std::string& label,sf::Color accent,bool active,unsigned size){sf::Vector2f m=window_.mapPixelToCoords(sf::Mouse::getPosition(window_));bool h=pointIn(r,m);drawPanel(r,h?sf::Color(26,42,57,248):sf::Color(17,29,41,245),active?accent:withAlpha(accent,h?220:120),active?2.5f:1.5f);sf::RectangleShape bar({4,r.height-12});bar.setPosition(r.left+6,r.top+6);bar.setFillColor(withAlpha(accent,h?255:180));window_.draw(bar);drawText(label,size,{r.left+r.width/2+4,r.top+r.height/2-1},h?sf::Color::White:TEXT,true,true);return h;}
void Game::drawStatBadge(const std::string& icon,const std::string& value,sf::Vector2f pos,sf::Color accent){drawPanel({pos.x,pos.y,146,48},sf::Color(7,14,22,205),withAlpha(accent,80),1);sf::Sprite i(assets_.ui(icon));i.setPosition(pos.x+10,pos.y+8);i.setScale(.5f,.5f);window_.draw(i);drawText(value,21,{pos.x+48,pos.y+13},TEXT,true);}
void Game::drawTowerIcon(TowerKind k,sf::Vector2f c,float scale,float rotation,sf::Color tint){sf::Sprite b(assets_.towerBase(k));auto bb=b.getLocalBounds();b.setOrigin(bb.width/2,bb.height/2);b.setPosition(c);b.setScale(scale,scale);b.setColor(tint);window_.draw(b);sf::Sprite t(assets_.towerTurret(k));auto tb=t.getLocalBounds();t.setOrigin(tb.width/2,tb.height/2);t.setPosition(c);t.setScale(scale,scale);t.setRotation(rotation);t.setColor(tint);window_.draw(t);}
void Game::drawEnemyIcon(EnemyKind k,sf::Vector2f c,float scale){sf::Sprite s(assets_.enemy(k));auto b=s.getLocalBounds();s.setOrigin(b.width/2,b.height/2);s.setPosition(c);s.setScale(scale,scale);window_.draw(s);}

void Game::drawMenu(){
    sf::Sprite bg(assets_.ui("menu_background"));window_.draw(bg);
    for(const auto& p:menuParticles_){sf::CircleShape s(p.r);s.setOrigin(p.r,p.r);s.setPosition(p.p);s.setFillColor(p.c);window_.draw(s);}
    sf::Sprite logo(assets_.ui("logo"));auto lb=logo.getLocalBounds();logo.setOrigin(lb.width/2,lb.height/2);logo.setPosition(800,245);logo.setScale(.92f,.92f);window_.draw(logo);
    drawWrapped("Verteidige den Aegis-Kern gegen 20 immer härtere Angriffswellen. Kombiniere Türme, spezialisiere Upgrades und nutze die Karte zu deinem Vorteil.",20,{520,350,560,90},sf::Color(176,199,216),6,false);
    drawButton(menuPlay(),"SPIEL STARTEN",CYAN,false,23);drawButton(menuTutorial(),"ANLEITUNG",GREEN,false,22);drawButton(menuExit(),"BEENDEN",RED,false,21);
    drawText("C++17  •  SFML  •  3 Karten  •  6 Türme  •  7 Gegnertypen",16,{800,755},MUTED,false,true);
    drawText("F1 öffnet im Spiel jederzeit die Kurzanleitung",14,{800,790},sf::Color(110,145,168),false,true);
}

void Game::drawMapSelect(){
    window_.clear(sf::Color(7,13,21));drawText("EINSATZGEBIET WÄHLEN",38,{800,72},TEXT,true,true);drawText("Jede Karte verändert gute Baupositionen und die optimale Turmwahl.",18,{800,118},MUTED,false,true);
    sf::Vector2f m=window_.mapPixelToCoords(sf::Mouse::getPosition(window_));
    for(int i=0;i<3;++i){map_.set(i);sf::FloatRect r(70.f+i*510.f,220,440,500);bool h=pointIn(r,m);drawPanel(r,h?sf::Color(18,31,44):sf::Color(13,23,34),withAlpha(map_.data().accent,h?230:100),h?3:1.5f);
        sf::Sprite pic(assets_.mapTexture(i));pic.setPosition(r.left+20,r.top+20);pic.setScale(400.f/1200.f,280.f/900.f);window_.draw(pic);
        drawText(map_.data().name,28,{r.left+22,r.top+322},TEXT,true);drawText(map_.data().subtitle,16,{r.left+22,r.top+360},map_.data().accent,true);
        drawWrapped(map_.data().description,16,{r.left+22,r.top+394,392,72},MUTED,4,false);
        drawText(i==0?"SCHWIERIGKEIT  ●○○":i==1?"SCHWIERIGKEIT  ●●○":"SCHWIERIGKEIT  ●●●",14,{r.left+22,r.top+468},i==2?ORANGE:map_.data().accent,true);
    }
    map_.set(0);drawButton(backRect(),"← ZURÜCK",CYAN,false,18);
}

void Game::drawTutorial(){
    sf::Sprite bg(assets_.ui("menu_background"));bg.setColor(sf::Color(150,160,170));window_.draw(bg);drawPanel({330,90,940,670},sf::Color(8,16,26,246),sf::Color(75,135,172,130),2);
    static const char* titles[]={"1  DAS ZIEL","2  TÜRME BAUEN","3  WELLEN & GEGNER","4  UPGRADES","5  TAKTIK","6  STEUERUNG"};
    drawText(titles[tutorialPage_],34,{800,145},CYAN,true,true);
    if(tutorialPage_==0){
        drawWrapped("Deine Aufgabe ist einfach: Kein Feind darf den Aegis-Kern am Ende der Straße erreichen. Du startest mit 20 Kernpunkten und Credits. Besiegte Gegner bringen neue Credits. Nach Welle 20 ist der Sektor gesichert.",21,{410,215,780,150},TEXT,8);
        drawEnemyIcon(EnemyKind::Raider,{570,500},1.f);drawText("ANGREIFER",18,{570,570},RED,true,true);drawText("→",60,{800,495},CYAN,true,true);sf::CircleShape core(48);core.setOrigin(48,48);core.setPosition(1030,500);core.setFillColor(sf::Color(35,70,83));core.setOutlineColor(GREEN);core.setOutlineThickness(5);window_.draw(core);drawText("KERN",18,{1030,570},GREEN,true,true);
    }else if(tutorialPage_==1){
        drawWrapped("Wähle rechts einen Turm oder drücke 1–6. Bewege den Mauszeiger über die Karte und klicke auf eine freie Fläche. Grün bedeutet: hier darf gebaut werden. Rot bedeutet: Straße, Rand oder ein anderer Turm blockiert den Platz.",20,{410,205,780,125},TEXT,7);
        for(int i=0;i<6;++i){float x=475+(i%3)*165,y=430+(i/3)*145;drawPanel({x-65,y-65,130,120},sf::Color(18,31,43),withAlpha(CYAN,70),1);drawTowerIcon(towerOrder[i],{x,y-10},.55f);drawText(std::to_string(i+1)+"  "+towerName(towerOrder[i]),15,{x,y+42},TEXT,true,true);}
    }else if(tutorialPage_==2){
        drawWrapped("Mit LEERTASTE oder dem Button 'WELLE STARTEN' schickst du die nächste Angriffswelle los. Schnellere Gegner brauchen frühe Treffer, Panzer widerstehen Schaden, Schildträger besitzen zusätzliche Energie und Regeneratoren heilen sich.",20,{410,205,780,120},TEXT,7);
        std::vector<EnemyKind> es={EnemyKind::Runner,EnemyKind::Tank,EnemyKind::Shield,EnemyKind::Regen,EnemyKind::Splitter,EnemyKind::Boss};for(int i=0;i<6;++i){float x=460+i*135;drawEnemyIcon(es[i],{x,470},.75f);drawText(enemyName(es[i]),13,{x,535},i==5?RED:MUTED,true,true);}drawText("Jede 5. Welle endet mit einem TITAN.",20,{800,625},ORANGE,true,true);
    }else if(tutorialPage_==3){
        drawWrapped("Klicke auf einen gebauten Turm. Du siehst Reichweite, Schaden, Feuerrate und Zielpriorität. Stufe 2 ist ein normales Upgrade. Danach entscheidest du dich dauerhaft für einen von zwei Spezialisierungswegen. Stufe 4 verstärkt diese Wahl noch einmal.",20,{410,205,780,135},TEXT,7);
        drawTowerIcon(TowerKind::Pulse,{575,490},1.f);drawText("STUFE 2",18,{575,575},TEXT,true,true);drawText("→",52,{765,485},MUTED,true,true);drawPanel({850,390,280,190},sf::Color(18,31,43),withAlpha(CYAN,100),2);drawText("OVERCLOCK",20,{990,430},CYAN,true,true);drawText("oder",16,{990,482},MUTED,false,true);drawText("PANZERBRECHER",20,{990,530},ORANGE,true,true);
    }else if(tutorialPage_==4){
        drawWrapped("Kein einzelner Turm löst alles. Kryo hält Gruppen länger in Feuerzonen, Mörser und Raketen bestrafen dichte Wellen, Tesla räumt Gruppen ab und Railguns eliminieren harte Einzelziele. Platziere Türme an Kurven, damit Gegner länger in Reichweite bleiben.",20,{410,205,780,130},TEXT,7);
        drawTowerIcon(TowerKind::Frost,{520,500},.85f);drawText("+",38,{650,500},GREEN,true,true);drawTowerIcon(TowerKind::Cannon,{760,500},.85f);drawText("+",38,{890,500},GREEN,true,true);drawTowerIcon(TowerKind::Sniper,{1000,500},.85f);drawText("KONTROLLE  +  FLÄCHE  +  EINZELSCHADEN",18,{800,625},CYAN,true,true);
    }else{
        std::vector<std::pair<std::string,std::string>> rows={{"1–6","Turm auswählen"},{"Linksklick","Bauen / Turm auswählen"},{"Rechtsklick","Bauauswahl abbrechen"},{"LEERTASTE","Nächste Welle"},{"U","Ausgewählten Turm upgraden"},{"T","Zielpriorität wechseln"},{"S","Turm verkaufen"},{"F","1× / 2× Geschwindigkeit"},{"P / ESC","Pause"},{"F1","Kurzanleitung im Spiel"}};float y=205;for(auto& r:rows){drawText(r.first,18,{500,y},CYAN,true);drawText(r.second,18,{720,y},TEXT,false);y+=47;}
    }
    drawText(std::to_string(tutorialPage_+1)+" / 6",16,{800,755},MUTED,false,true);drawButton({500,800,180,52},"← ZURÜCK",CYAN,tutorialPage_==0,17);drawButton({700,800,200,52},"HAUPTMENÜ",MUTED,false,17);drawButton({920,800,180,52},"WEITER →",GREEN,tutorialPage_==5,17);
}

void Game::drawGameplay(){drawWorld();drawHud();drawCommandPanel();fx_.draw(window_,assets_.text().loaded()?&assets_.text():nullptr);if(helpOverlay_)drawHelpOverlay();else if(paused_)drawPauseOverlay();if(gameOver_||victory_)drawEndOverlay();}
void Game::drawWorld(){
    sf::Sprite mapSprite(assets_.mapTexture(map_.index()));window_.draw(mapSprite);
    // animated spawn / core rings
    const auto& p=map_.path();float pulse=.5f+.5f*std::sin(elapsed_*3.1f);for(int i=0;i<2;++i){float rr=32+i*12+pulse*7;sf::CircleShape r(rr);r.setOrigin(rr,rr);r.setPosition(p.front());r.setFillColor(sf::Color::Transparent);r.setOutlineColor(withAlpha(map_.data().accent,static_cast<sf::Uint8>(110-i*30)));r.setOutlineThickness(2);window_.draw(r);}float cr=35+pulse*5;sf::CircleShape core(cr);core.setOrigin(cr,cr);core.setPosition(p.back());core.setFillColor(withAlpha(map_.data().accent,30));core.setOutlineColor(withAlpha(map_.data().accent,180));core.setOutlineThickness(3);window_.draw(core);
    for(auto& e:enemies_)e->draw(window_,assets_);
    for(std::size_t i=0;i<towers_.size();++i)towers_[i]->draw(window_,assets_,int(i)==selectedTower_);
    for(auto& pr:projectiles_)pr.draw(window_);
    if(buildKind_){sf::Vector2f m=window_.mapPixelToCoords(sf::Mouse::getPosition(window_));if(m.x<WORLD_W){bool valid=credits_>=towerCost(*buildKind_)&&map_.canBuild(m,towerPositions());TowerStats st=makeTower(*buildKind_,m)->stats();sf::CircleShape range(st.range);range.setOrigin(st.range,st.range);range.setPosition(m);range.setFillColor(valid?sf::Color(60,230,140,20):sf::Color(255,70,85,20));range.setOutlineColor(valid?sf::Color(80,240,155,110):sf::Color(255,80,95,130));range.setOutlineThickness(2);window_.draw(range);drawTowerIcon(*buildKind_,m,.72f,0,valid?sf::Color(190,255,220,210):sf::Color(255,145,150,210));}}
}

void Game::drawHud(){
    drawPanel({18,18,640,64},sf::Color(4,10,17,205),sf::Color(90,150,185,70),1);drawStatBadge("credits",std::to_string(credits_),{30,26},GOLD);drawStatBadge("core",std::to_string(core_),{184,26},RED);drawStatBadge("wave",std::to_string(waves_.wave())+"/20",{338,26},CYAN);drawStatBadge("score",std::to_string(score_),{492,26},PURPLE);
    drawPanel({840,20,330,48},sf::Color(4,10,17,180),sf::Color(90,150,185,55),1);drawText(map_.data().name,18,{860,34},TEXT,true);drawText(speed_==2?"2× ZEIT":"1× ZEIT",15,{1148,45},speed_==2?GOLD:MUTED,true,true);
}

void Game::drawCommandPanel(){
    drawPanel({PANEL_X,0,PANEL_W,WORLD_H},PANEL,sf::Color(50,82,104),1);sf::RectangleShape accent({4,900});accent.setPosition(PANEL_X,0);accent.setFillColor(map_.data().accent);window_.draw(accent);
    drawText("AEGIS COMMAND",24,{1230,24},TEXT,true);drawText("SEKTORVERTEIDIGUNG",12,{1230,51},MUTED,true);
    drawButton(startWaveRect(),waves_.active()?"WELLE LÄUFT":"WELLE STARTEN  [SPACE]",waves_.active()?MUTED:GREEN,waves_.active(),18);drawButton(speedRect(),speed_==1?"ZEIT  1×":"ZEIT  2×",GOLD,speed_==2,16);drawButton(pauseRect(),"PAUSE",CYAN,false,16);
    // preview
    drawPanel({1230,198,340,132},PANEL2,sf::Color(48,78,98),1);drawText(waves_.active()?"NÄCHSTE WELLE":"BEDROHUNGSANALYSE",14,{1248,214},MUTED,true);drawText(waves_.wave()>=20?"SEKTOR ENDE":"Welle "+std::to_string(std::min(20,waves_.wave()+1)),22,{1248,238},TEXT,true);drawText("Bedrohung: "+waves_.threatText(),14,{1248,269},waves_.wave()>=15?RED:(waves_.wave()>=8?ORANGE:GREEN),true);
    int shown=0;for(auto k:waves_.preview()){if(shown>=7)break;drawEnemyIcon(k,{1260.f+shown*42.f,304},.32f);shown++;}
    if(selectedTower_>=0&&selectedTower_<int(towers_.size()))drawSelectedTowerPanel();else{drawPanel({1230,346,340,188},sf::Color(14,25,36,220),sf::Color(43,73,94),1);drawText("BAUPLAN",18,{1248,365},TEXT,true);drawWrapped(buildKind_?"Bauplatz wählen. Grün = gültig, Rot = blockiert. Rechtsklick beendet den Baumodus.":"Wähle unten einen Turm. Klicke auf einen gebauten Turm, um Upgrades, Verkauf und Zielpriorität zu öffnen.",15,{1248,398,300,100},MUTED,5);if(buildKind_){drawTowerIcon(*buildKind_,{1518,482},.45f);drawText(towerName(*buildKind_),17,{1248,482},map_.data().accent,true);drawText(money(towerCost(*buildKind_)),17,{1248,507},GOLD,true);}}
    drawBuildCards();drawText("F1  Anleitung     ESC  Pause",12,{1230,877},sf::Color(100,127,146),false);
}

void Game::drawSelectedTowerPanel(){
    auto& t=*towers_[selectedTower_];auto s=t.stats();drawPanel({1230,346,340,206},sf::Color(14,25,36,225),sf::Color(55,93,116),1);drawTowerIcon(t.kind(),{1282,392},.55f,t.rotation());drawText(towerName(t.kind())+"  L"+std::to_string(t.level()),21,{1325,365},TEXT,true);drawText(t.branch()==UpgradeBranch::A?t.branchAName():t.branch()==UpgradeBranch::B?t.branchBName():"Standardkonfiguration",13,{1325,393},t.branch()==UpgradeBranch::B?ORANGE:(t.branch()==UpgradeBranch::A?CYAN:MUTED),true);
    drawText("DMG  "+std::to_string(int(s.damage)),14,{1248,424},GOLD,true);drawText("RNG  "+std::to_string(int(s.range)),14,{1360,424},CYAN,true);std::ostringstream ss;ss<<std::fixed<<std::setprecision(2)<<s.cooldown;drawText("CD  "+ss.str()+"s",14,{1462,424},GREEN,true);
    drawButton(targetRect(),"ZIEL: "+targetModeName(t.targetMode()),CYAN,false,13);drawButton(sellRect(),"VERKAUF  +"+std::to_string(t.sellValue()),GOLD,false,13);
    if(t.maxLevel()){drawPanel(upgradeRect(),sf::Color(21,32,42),sf::Color(80,98,111),1);drawText("MAXIMALE STUFE",15,{1400,523},MUTED,true,true);}else if(t.needsBranch()){
        drawButton(branchARect(),t.branchAName()+"\n"+money(t.upgradeCost()),CYAN,false,12);drawButton(branchBRect(),t.branchBName()+"\n"+money(t.upgradeCost()),ORANGE,false,12);
    }else drawButton(upgradeRect(),"UPGRADE  [U]   "+money(t.upgradeCost()),credits_>=t.upgradeCost()?GREEN:MUTED,false,15);
}

void Game::drawBuildCards(){
    sf::Vector2f m=window_.mapPixelToCoords(sf::Mouse::getPosition(window_));for(int i=0;i<6;++i){auto k=towerOrder[i];auto r=towerCardRect(i);bool h=pointIn(r,m),active=buildKind_&&*buildKind_==k;drawPanel(r,active?sf::Color(23,45,55):h?sf::Color(22,37,50):sf::Color(16,28,40),active?map_.data().accent:withAlpha(map_.data().accent,h?140:55),active?2:1);drawTowerIcon(k,{r.left+39,r.top+43},.38f);drawText(std::to_string(i+1),12,{r.left+9,r.top+7},MUTED,true);drawText(towerName(k),15,{r.left+76,r.top+17},TEXT,true);drawText(std::to_string(towerCost(k))+" C",14,{r.left+76,r.top+42},credits_>=towerCost(k)?GOLD:RED,true);drawText(towerShort(k),10,{r.left+76,r.top+66},MUTED,false);}
}

void Game::drawPauseOverlay(){drawPanel({0,0,1600,900},sf::Color(0,0,0,150));drawPanel({585,300,430,290},sf::Color(9,18,28,250),sf::Color(75,145,180,150),2);drawText("PAUSE",40,{800,355},TEXT,true,true);drawText("Der Angriff ist angehalten.",17,{800,397},MUTED,false,true);drawButton({690,430,220,54},"WEITER",GREEN,false,19);drawButton({690,500,220,50},"HAUPTMENÜ",RED,false,17);drawText("ESC oder P setzt fort",13,{800,570},MUTED,false,true);}
void Game::drawHelpOverlay(){drawPanel({0,0,1600,900},sf::Color(0,0,0,165));drawPanel({390,125,820,650},sf::Color(8,17,27,252),sf::Color(80,173,215,170),2);drawText("KURZANLEITUNG",32,{800,172},CYAN,true,true);drawWrapped("1. Turm mit 1–6 oder rechts auswählen.  2. Auf eine grüne freie Fläche klicken.  3. Mit SPACE eine Welle starten.  4. Gebauten Turm anklicken und mit U verbessern.  5. Auf Stufe 2 einen Spezialisierungsweg wählen.  6. Türme kombinieren: Kryo + Flächenschaden ist besonders stark.",18,{455,225,690,150},TEXT,7);drawText("WICHTIGE TASTEN",18,{455,410},MUTED,true);std::vector<std::pair<std::string,std::string>> rows={{"1–6","Turm wählen"},{"SPACE","Welle starten"},{"U","Upgrade"},{"T","Zielmodus"},{"S","Verkaufen"},{"F","1× / 2×"},{"P / ESC","Pause"}};float y=448;for(auto& r:rows){drawText(r.first,15,{480,y},CYAN,true);drawText(r.second,15,{620,y},TEXT,false);y+=34;}drawButton({690,720,220,50},"VERSTANDEN",GREEN,false,17);}
void Game::drawEndOverlay(){drawPanel({0,0,1600,900},sf::Color(0,0,0,176));drawPanel({470,250,660,440},sf::Color(8,17,27,252),victory_?GREEN:RED,2.5f);drawText(victory_?"SEKTOR GESICHERT":"KERN VERLOREN",38,{800,315},victory_?GREEN:RED,true,true);drawText(victory_?"Alle 20 Angriffswellen wurden abgewehrt.":"Die Angreifer haben den Aegis-Kern durchbrochen.",18,{800,365},TEXT,false,true);drawText("Welle",14,{620,430},MUTED,true,true);drawText(std::to_string(waves_.wave()),31,{620,463},TEXT,true,true);drawText("Abschüsse",14,{800,430},MUTED,true,true);drawText(std::to_string(kills_),31,{800,463},TEXT,true,true);drawText("Score",14,{980,430},MUTED,true,true);drawText(std::to_string(score_),31,{980,463},GOLD,true,true);drawButton({555,560,220,54},"NOCH EINMAL",GREEN,false,17);drawButton({790,560,220,54},"KARTENWAHL",CYAN,false,17);drawButton({673,630,220,48},"HAUPTMENÜ",MUTED,false,16);}
