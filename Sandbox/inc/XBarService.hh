#ifndef Sandbox_XBarService_hh
#define Sandbox_XBarService_hh

//
//
// A test service that prints tracer ouptut. Used to study order of calls.
// The XbarSerivce depends on the FooService.  The XBarService is just
// a copy of the BarService but it comes after Foo in an alphabetic sort.
//
// $Id: XBarService.hh,v 1.2 2013/03/14 19:54:49 kutschke Exp $
// $Author: kutschke $
// $Date: 2013/03/14 19:54:49 $
//
// Contact person Rob Kutschke
//

// Framework include files
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

namespace mu2e {

  class XBarService {
  public:
    XBarService(const fhicl::ParameterSet&, art::ActivityRegistry&);
    ~XBarService();

    void preBeginRun( art::Run const &run);

    void poke() const;

  private:

  };

}

DECLARE_ART_SERVICE(mu2e::XBarService, LEGACY)
#endif /* Sandbox_XBarService_hh */
