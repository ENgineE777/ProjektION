
#pragma once

#include "Root/Root.h"
#include "Root/Scenes/SceneEntity.h"

namespace Orin::Overkill
{	
	enum MusicTheme
	{
		None,
		ThemeMain,
		ThemeNormal,
		ThemeBattle,		
		ThemeIntro,
		ThemeCaptainRoom
	};

	class DynamicMusicPlayer : public SceneEntity
	{
	public:
		
		META_DATA_DECL(DynamicMusicPlayer)

#ifndef DOXYGEN_SKIP

		virtual ~DynamicMusicPlayer() = default;

		void Init() override;

		void ChangeTheme(MusicTheme theme, float fadeTime);
		void MuteTheme(bool mute);

		void Work(float dt);

		void Release() override;
#endif
	private:

		float fadeSpeed = 2.0f;

		float curStreamVolume = 0.0f;
		class SoundEvent* curStream = nullptr;

		float prevStreamVolume = 0.0f;
		class SoundEvent* prevStream = nullptr;

		bool muted = false;
		float muteAmout = 0.0f;

		MusicTheme theme = MusicTheme::None;

		struct Music
		{
			META_DATA_DECL_BASE(Music)

			MusicTheme theme;
			eastl::string name;
		};

		eastl::vector<Music> musics;

		const char* FindMusicByTheme(MusicTheme theme);
	};
}
