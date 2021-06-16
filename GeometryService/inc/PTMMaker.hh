#ifndef GeometryService_PTMMaker_hh
#define GeometryService_PTMMaker_hh

#include <memory>

#include "CLHEP/Vector/Rotation.h"
#include "CLHEP/Vector/ThreeVector.h"

#include "ConfigTools/inc/SimpleConfig.hh"
#include "PTMGeom/inc/PTMPWC.hh"
#include "PTMGeom/inc/PTM.hh"
//
// construct and return a PTMon
//
// original author Helenka Casler
//

namespace mu2e {

  class PTMMaker {

  public:
    static std::unique_ptr<PTM> make(SimpleConfig const& config);
  };

} // namespace mu2e

#endif