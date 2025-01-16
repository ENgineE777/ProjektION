#include "Shield.h"
#include "World/WorldManager.h"
#include "Root/Root.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, Shield, "Overkill/Objects", "Shield")

	META_DATA_DESC(Shield)
		BASE_SCENE_ENTITY_PROP(Shield)
		COLOR_PROP(Shield, color, COLOR_YELLOW, "Geometry", "color")
	META_DATA_DESC_END()

    void Shield::Init()
    {
		PhysEntity2D::Init();

        bodyType = BodyType::Kinematic;
		visibleDuringPlay = false;

		physGroup = PhysGroup::MovementBlock;
    }
}