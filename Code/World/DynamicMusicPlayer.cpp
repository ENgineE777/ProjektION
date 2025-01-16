#include "DynamicMusicPlayer.h"
#include "Common/Utils.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, DynamicMusicPlayer, "Overkill/World", "DynamicMusicPlayer")

	META_DATA_DESC(DynamicMusicPlayer::Music)
		ENUM_PROP(DynamicMusicPlayer::Music, theme, 0, "Prop", "Vert size", "Type of height of a widget")
			ENUM_ELEM("MainMenu", 1)
			ENUM_ELEM("Normal", 2)
			ENUM_ELEM("Battle", 3)
			ENUM_ELEM("Intro", 4)
			ENUM_ELEM("CaptainRoom", 5)		
		ENUM_END

		STRING_PROP(DynamicMusicPlayer::Music, name, "", "Property", "SoundName")
	META_DATA_DESC_END()

	META_DATA_DESC(DynamicMusicPlayer)
		BASE_SCENE_ENTITY_PROP(DynamicMusicPlayer)
		ARRAY_PROP(DynamicMusicPlayer, musics, Music, "Properties", "Musics")
	META_DATA_DESC_END()

	void DynamicMusicPlayer::Init()
	{
		Tasks(false)->AddTask(100, this, (Object::Delegate)&DynamicMusicPlayer::Work);
	}

	const char* DynamicMusicPlayer::FindMusicByTheme(MusicTheme theme)
	{
		eastl::vector<const char*> themeMusics;

		for (const auto& entry : musics)
		{
			if (entry.theme == theme)
			{
				themeMusics.push_back(entry.name.c_str());
			}
		}

		int count = (int)themeMusics.size();

		if (count > 0)
		{
			int index = (int)Utils::GetRandom(0.1f, (float)count - 0.1f);

			return themeMusics[index];
		}

		return nullptr;
	}

	void DynamicMusicPlayer::ChangeTheme(MusicTheme setTheme, float fadeTime)
	{
		if (theme == MusicTheme::ThemeCaptainRoom && setTheme == MusicTheme::ThemeNormal)
		{
			return;
		}
	
		if (theme == setTheme)
		{
			return;
		}

		fadeSpeed = 1.0f / fadeTime;

		if (prevStream)
		{
			RELEASE(prevStream)
		}

		if (curStream)
		{
			prevStreamVolume = curStreamVolume;
			prevStream = curStream;
			curStream = nullptr;
		}

		theme = setTheme;

		curStreamVolume = 0.0f;

		const char* name = FindMusicByTheme(theme);

		if (name)
		{
			curStream = GetRoot()->GetSounds()->CreateSoundEvent(name);
		}

		if (curStream)
		{
			curStream->Play();
			curStream->SetVolume(curStreamVolume);
		}
	}

	void DynamicMusicPlayer::MuteTheme(bool setMuted)
	{
		muted = setMuted;
	}

	void DynamicMusicPlayer::Work(float dt)
	{		
		muteAmout = Math::Clamp(muteAmout + dt * (muted ? -1.0f : 1.0f) * 2.0f, 0.0f, 1.0f);
		float muteFacotr = muteAmout * 0.75f + 0.25f;

		if (prevStream)
		{
			prevStreamVolume -= dt * fadeSpeed;
			prevStream->SetVolume(prevStreamVolume * muteFacotr);

			if (prevStreamVolume < 0.0f)
			{
				RELEASE(prevStream)
			}
		}

		if (curStream)
		{
			curStreamVolume = fminf(curStreamVolume + dt * fadeSpeed, 1.0f);
			curStream->SetVolume(curStreamVolume * muteFacotr);
		}
	}

	void DynamicMusicPlayer::Release()
	{
		RELEASE(curStream);
		RELEASE(prevStream);

		SceneEntity::Release();
	}
}