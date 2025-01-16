#pragma once

#include "Turret.h"

namespace Orin::Overkill
{
	class TurretSimple : public Turret
	{		
    public:
		META_DATA_DECL_BASE(TurretStationary)

		void ApplyProperties() override;
    };
}