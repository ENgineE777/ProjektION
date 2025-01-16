
#include "Rotator.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{	
	ENTITYREG(SceneEntity, Rotator, "Overkill/Effects", "Rotator")

	META_DATA_DESC(Rotator)
		BASE_SCENE_ENTITY_PROP(Rotator)
	META_DATA_DESC_END()

	Rotator::Rotator() : SceneEntity()
	{
	}

	void Rotator::Init()
	{
		transform.objectType = ObjectType::Object2D;
		transform.transformFlag = (TransformFlag)(TransformFlag::SpriteTransformFlags);
		transform.size = 100.0f;

		Tasks(false)->AddTask(0, this, (Object::Delegate)&Rotator::Update);
	}

	void Rotator::Update(float dt)
	{
		if (AnimGraph2D::pause)
		{
			return;
		}

		if (IsVisible())
		{
			if (parent)
			{
				auto rot = parent->GetTransform().rotation;
				rot.z += dt * 360.0f;
				parent->GetTransform().rotation = rot.z;
			}			
		}
	}
}