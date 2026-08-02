#ifndef INC_MENU_H
#define INC_MENU_H

#include <string>
#include <iostream>
#include <sstream>

#include "keybindings.h"

enum MenuNodeType
{
    MNT_NONE,
    MNT_MENU,
    MNT_LEAF
};
enum LeafReturn
{
    LR_NONE,
    LR_QUIT,
    LR_SURRENDER,
    LR_NEWSPEED,
    LR_SETRES,
    LR_NEWBACKGROUND,
    LR_EXITMENU,
    LR_SAVESETTINGS
};

class Menu;
class MenuLeaf;

class Menu
{
    public:
	std::string title;

	/* menus and leaves give the menu or leaf to be found in each
	 * direction; 0->west, 1->north, 2->east, 3->south. For each i, at
	 * most one of menus[i] and leaves[i] should be non-NULL.
	 */
	Menu* menus[4];
	MenuLeaf* leaves[4];

	std::string textOfDir(int dir) const;

	Menu();

	Menu(std::string title,
		Menu* m1=NULL, MenuLeaf* ml1=NULL,
		Menu* m2=NULL, MenuLeaf* ml2=NULL,
		Menu* m3=NULL, MenuLeaf* ml3=NULL,
		Menu* m4=NULL, MenuLeaf* ml4=NULL
		);
};

class MenuLeaf
{
    public:
	virtual std::string name() =0;
	virtual LeafReturn act() =0;

	virtual ~MenuLeaf() {};
};

class MenuLeafToggleBool : public MenuLeaf
{
    private:
	std::string varName;
	bool* var;
    public:
	std::string name();
	LeafReturn act();

	MenuLeafToggleBool(std::string varName, bool* var) :
	    varName(varName), var(var) {}
};

class MenuLeafCycleAA : public MenuLeaf
{
    public:
	std::string name();
	LeafReturn act();

	MenuLeafCycleAA() {}
};

class MenuLeafCycleFreq : public MenuLeaf
{
    public:
	std::string name();
	LeafReturn act();

	MenuLeafCycleFreq() {}
};

template<typename T>
class MenuLeafIncVar : public MenuLeaf
{
    private:
	T* var;
	std::string varName;
	T amount;
	T min;
	T max;
    public:
	std::string name() {
	    return ( (amount >= 0) ? "Inc " : "Dec " ) + varName;
	}
	LeafReturn act() {
	    *var += amount;
	    if (*var < min)
		*var = min;
	    if (max > min && *var > max)
		*var = max;
	    return LR_NONE;
	}

	MenuLeafIncVar(T* var, std::string(varName), T amount, T min = 0, T max = 0) :
	    var(var), varName(varName), amount(amount), min(min), max(max) {}
};

template<typename T>
class MenuLeafShowVar : public MenuLeaf
{
    private:
	T* var;
	std::string varName;
	LeafReturn ret;
    public:
	std::string name() {
	    std::stringstream ss;
	    ss << varName << ": " << *var;
	    return ss.str();
	}
	LeafReturn act() { return ret; }

	MenuLeafShowVar(T* var, std::string(varName), LeafReturn ret=LR_NONE) :
	    var(var), varName(varName), ret(ret) {}
};

class MenuLeafReturn : public MenuLeaf
{
    private:
	std::string text;
	LeafReturn ret;
    public:
	std::string name() { return text; }
	LeafReturn act() { return ret; }

	MenuLeafReturn(std::string text, LeafReturn ret) :
	    text(text), ret(ret) {}
};

class MenuLeafSetRes : public MenuLeaf
{
    public:
	std::string name();
	LeafReturn act() { return LR_SETRES; }
};

class MenuLeafCycleBpp : public MenuLeaf
{
    public:
	std::string name() { return "Depth"; }
	LeafReturn act();
};

class MenuLeafCycleBGType : public MenuLeaf
{
    public:
	std::string name();
	LeafReturn act();
};

class MenuLeafToggleFullscreen : public MenuLeaf
{
    public:
	std::string name() { return "Fullscreen"; }
	LeafReturn act();
};

class MenuLeafCycleRes : public MenuLeaf
{
    public:
	std::string name() { return "Resolution"; }
	LeafReturn act();
};

class MenuLeafSpeed : public MenuLeaf
{
    public:
	std::string name();
	LeafReturn act();

	MenuLeafSpeed() {}
};

class MenuLeafSetKey : public MenuLeaf
{
    public:
	command selectedKey;

	std::string name();
	LeafReturn act();

	MenuLeafSetKey() : selectedKey(C_FIRST) {} 
};

class MenuLeafPrevKey : public MenuLeaf
{
    private:
	command* commandp;
    public:
	std::string name() { return "Previous key"; }
	LeafReturn act();
	MenuLeafPrevKey(command* commandp) : commandp(commandp) {}
};
class MenuLeafNextKey : public MenuLeaf
{
    private:
	command* commandp;
    public:
	std::string name() { return "Next key"; }
	LeafReturn act();
	MenuLeafNextKey(command* commandp) : commandp(commandp) {}
};

class MenuLeafKeyConflicts : public MenuLeaf
{
    private:
	command* commandp;
	command getConflictingCommand();
    public:
	std::string name();
	LeafReturn act();
	MenuLeafKeyConflicts(command* commandp) : commandp(commandp) {}
};

extern Menu topMenu;


struct SDL_Surface;
void drawMenu(SDL_Surface* surface, const Menu& menu);

#endif /* INC_MENU_H */
