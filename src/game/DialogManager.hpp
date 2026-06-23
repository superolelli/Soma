#pragma once

#include "LootableDialog.hpp"

class DialogManager
{
public:

	~DialogManager();

	void Update();
	void RenderDialogs();
	void AddLootableDialog(LootableDialog *_dialog);
	bool IsDialogOpen();

private:
	std::vector<LootableDialog*> dialogs;

};