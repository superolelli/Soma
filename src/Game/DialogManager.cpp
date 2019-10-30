#include "DialogManager.hpp"



void DialogManager::Update()
{
	for (auto it = dialogs.begin(); it != dialogs.end();)
	{
		(*it)->Update();
		if (!(*it)->IsOpen())
		{
			SAFE_DELETE(*it);
			it = dialogs.erase(it);
			continue;

		}
		it++;
	}
}

void DialogManager::RenderDialogs()
{
	for (auto d : dialogs)
		d->Render();
}

void DialogManager::Quit()
{
	for (auto d : dialogs)
		SAFE_DELETE(d);
}

void DialogManager::AddLootableDialog(LootableDialog * _dialog)
{
	dialogs.push_back(_dialog);
}


bool DialogManager::IsDialogOpen()
{
	return !dialogs.empty();
}