
#include "TurretSimple.h"

namespace Orin::Overkill
{
	ENTITYREG(SceneEntity, TurretSimple, "Overkill/Characters", "TurretSimple")

	META_DATA_DESC(TurretSimple)
		BASE_SCENE_ENTITY_PROP(TurretSimple)

		INT_PROP(TurretSimple, killScore, 250, "Turret", "killScore", "Kill Score")
		MARK_DISABLED_FOR_INSTANCE()

        FLOAT_PROP(TurretSimple, leftAngleVaule,    0.f, "Turret", "view Angle", "View Angle")
        BOOL_PROP(TurretSimple, clampEditorVisualization, true, "Turret", "clampEditorVisualization", "Clamp Editor Visualization")

		FLOAT_PROP(TurretSimple, alwaysViewRadius, 50.0f, "Turret", "alwaysViewRadius", "Always View Radius")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(TurretSimple, keepPlayerVisibleMaxTime, 0.25f, "Turret", "keepPlayerVisibleMaxTime", "Keep player is sight during time")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(TurretSimple, rotateSpeed, 600.0f, "Turret", "rotateSpeed", "Rotate Speed")
		MARK_DISABLED_FOR_INSTANCE()

		FLOAT_PROP(TurretSimple, agroTime, 1.0f, "Turret", "agroTime", "Agro Time")
		MARK_DISABLED_FOR_INSTANCE()

	META_DATA_DESC_END()

	void TurretSimple::ApplyProperties()
	{
		rightAngleVaule = leftAngleVaule;
	}
}