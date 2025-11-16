
#include "RestrictionArea.hpp"
#include "ModDataManager.hpp"

using namespace DLL;
using namespace SM;

void RestrictionArea::setBounds(const SM::Bounds& bounds) {
	gModDataManager->updateRestrictionAreaChunkBounds(this, bounds);
	m_bounds = bounds;
}
