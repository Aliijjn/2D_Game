#ifndef INVENTORY_HPP
#define INVENTORY_HPP

#include "2DGame.hpp"	

//init
void	initCraftingSlots(CraftingRecipe* craft);
void	initSlot(Slot* slots);

//inventory management
bool	toInventory(GameInfo* info, uint16_t itemIndex, uint16_t count);
void	manageDrag(GameInfo* info);
void	manageDrop(GameInfo* info);

#endif // !INVENTORY_HPP
