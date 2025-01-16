
#include "BaseMenu.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	void BaseMenu::PlaySound(SoundType soundType)
	{
		const char* eventNames[] = {"event:/Menu/pick", "event:/Menu/next", "event:/Menu/prev"};

		Utils::PlaySoundEvent(eventNames[(int)soundType]);
	}

	void BaseMenu::OnVisiblityChange(bool set)
	{
		if (curItem != 0)
		{
			items[curItem].label->color = COLOR_WHITE;
			curItem = 0;
			items[curItem].label->color = selectedLabel;
		}
	}

	void BaseMenu::Draw(float dt)
	{
		if (items.empty())
		{
			return;
		}

		if (GetRoot()->GetControls()->GetAliasState(WorldManager::instance->aliasUIUp, AliasAction::JustPressed))
		{
			PlaySound(SoundType::ChangeSelection);

			items[curItem].label->color = COLOR_WHITE;
			curItem--;

			if (curItem < 0)
			{
				curItem = (int)items.size() - 1;
			}

			while (!items[curItem].enabled)
			{
				curItem--;

				if (curItem < 0)
				{
					curItem = (int)items.size() - 1;
				}
			}

			items[curItem].label->color = selectedLabel;
		}

		if (GetRoot()->GetControls()->GetAliasState(WorldManager::instance->aliasUIDown, AliasAction::JustPressed))
		{
			PlaySound(SoundType::ChangeSelection);

			items[curItem].label->color = COLOR_WHITE;
			curItem++;

			if (curItem >= items.size())
			{
				curItem = 0;
			}

			while (!items[curItem].enabled)
			{
				curItem++;

				if (curItem >= items.size())
				{
					curItem = 0;
				}
			}

			items[curItem].label->color = selectedLabel;
		}

		if (GetRoot()->GetControls()->GetAliasState(WorldManager::instance->aliasUIActive, AliasAction::JustPressed))
		{			
			if (items[curItem].callback)
			{
				PlaySound(items[curItem].goBack ? SoundType::GoBack : SoundType::Activate);

				items[curItem].callback();
			}
		}

		if (GetRoot()->GetControls()->GetAliasState(WorldManager::instance->aliasUILeft, AliasAction::JustPressed))
		{			
			if (items[curItem].callbackLeft)
			{
				PlaySound(SoundType::Activate);
				items[curItem].callbackLeft();
			}
		}

		if (GetRoot()->GetControls()->GetAliasState(WorldManager::instance->aliasUIRight, AliasAction::JustPressed))
		{			
			if (items[curItem].callbackRight)
			{
				PlaySound(SoundType::Activate);
				items[curItem].callbackRight();
			}
		}

		ContainerWidget::Draw(dt);
	}
}