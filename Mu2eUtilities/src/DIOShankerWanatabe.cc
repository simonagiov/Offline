//
// Generate an energy value for the DIO electrons, using Wanatabe
// data, merged to Shanker's formula near the endpoint. The construction
// of the spectrum is made by ShankerWanatabeSpectrum class
//
// $Id: DIOShankerWanatabe.cc,v 1.3 2011/03/04 23:09:10 onoratog Exp $
// $Author: onoratog $
// $Date: 2011/03/04 23:09:10 $
//
// 

// C++ includes
#include <iostream>

// Framework includes
#include "FWCore/Utilities/interface/Exception.h"

// Mu2e includes
#include "Mu2eUtilities/inc/DIOShankerWanatabe.hh"
#include "Mu2eUtilities/inc/ShankerWanatabeSpectrum.hh"
#include "ConditionsService/inc/ConditionsHandle.hh"
#include "ConditionsService/inc/ParticleDataTable.hh"
#include "Mu2eUtilities/inc/PDGCode.hh"

using namespace std;

namespace mu2e {

  DIOShankerWanatabe::DIOShankerWanatabe(int atomicZ, double emin, double emax, double spectRes,
                                         edm::RandomNumberGeneratorService::base_engine_t& engine):
  //atomic number of the foil material
    _Znum ( atomicZ ),
  //limits on energy generation
    _emin ( emin ),
    _emax ( emax ),
    _res ( spectRes ),
    _randEnergy ( engine, &(ShWaSpectrum()[0]), _nBinsSpectrum )
  {
    if (_Znum!=13) {
      throw cms::Exception("GEOM")
        << "Foil material different from Alluminum";
    }
  }

  DIOShankerWanatabe::~DIOShankerWanatabe()
  {
  }

  double DIOShankerWanatabe::fire() {

    return _emin + (_emax-_emin)*_randEnergy.fire();
 
  }

  vector<double> DIOShankerWanatabe::ShWaSpectrum() {

    vector<double> spectrum;
    ShankerWanatabeSpectrum WSspec(_Znum);

    double step = _emin;

    while (step <= _emax) {
      spectrum.push_back(WSspec[step]);
      step += _res;
    }

    _nBinsSpectrum = spectrum.size();



    return spectrum;
  }
  
}

