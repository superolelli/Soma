#pragma once

#include "LootableDialog.hpp"

class DialogManager
{
public:

	void Update();
	void RenderDialogs();
	void Quit();

	void AddLootableDialog(LootableDialog *_dialog);

	bool IsDialogOpen();

private:
	std::vector<LootableDialog*> dialogs;

};