//
// This module transforms StrawDigi objects into StrawHit objects
//
// $Id: StrawHitsFromStrawDigis_module.cc,v 1.12 2014/03/25 22:14:39 brownd Exp $
// $Author: brownd $ 
// $Date: 2014/03/25 22:14:39 $
//
// Original author David Brown, LBNL
//
// framework
#include "art/Framework/Principal/Event.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Principal/Handle.h"
#include "GeometryService/inc/GeomHandle.hh"
#include "art/Framework/Core/EDProducer.h"
#include "GeometryService/inc/DetectorSystem.hh"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/Optional/TFileService.h"
// conditions
#include "ConditionsService/inc/ConditionsHandle.hh"
#include "ConditionsService/inc/AcceleratorParams.hh"
#include "ConditionsService/inc/TrackerCalibrations.hh"
#include "ConfigTools/inc/ConfigFileLookupPolicy.hh"
#include "TrackerConditions/inc/StrawElectronics.hh"
#include "TrackerConditions/inc/StrawPhysics.hh"
// helpers
#include "TrkHitReco/inc/PeakFit.hh"
#include "TrkHitReco/inc/PeakFitRoot.hh"
#include "TrkHitReco/inc/PeakFitFunction.hh"
#include "TrkHitReco/inc/ComboPeakFitRoot.hh"
//CLHEP
#include "CLHEP/Vector/ThreeVector.h"
// data
#include "RecoDataProducts/inc/StrawDigiCollection.hh"
#include "RecoDataProducts/inc/StrawHitCollection.hh"

using namespace std;
using CLHEP::Hep3Vector;
namespace mu2e {
  using namespace TrkTypes;
  class StrawHitsFromStrawDigis : public art::EDProducer {

  public:
    explicit StrawHitsFromStrawDigis(fhicl::ParameterSet const& pset);
    virtual ~StrawHitsFromStrawDigis(); 
    virtual void beginRun( art::Run& run );
    virtual void produce( art::Event& e);

  private:

    // # of ADC digitizations to sum to define baseline
    unsigned _nbase;
    double _mbbuffer; // buffer on that for ghost hits (wrapping)
    TrkHitReco::FitType _fittype;
    bool _truncateADC; // model ADC truncation
    bool _floatPedestal; // float pedestal in fit
    bool _floatWidth; // _float width in fit
    bool _earlyPeak, _latePeak; // additional peak finding
// Diagnostics level.
    int _printLevel, _diagLevel, _debugLevel;
 // fit option
    string _peakFitOption; // option flag for root fit
    unsigned _maxFitIter; //
    // Name of the StrawDigi collection
    string _strawDigis;
    std::unique_ptr<TrkHitReco::PeakFit> _pfit;
    fhicl::ParameterSet _peakfit;       
    StrawEnd _end[2]; // helper
  };

  StrawHitsFromStrawDigis::StrawHitsFromStrawDigis(fhicl::ParameterSet const& pset) :
    _nbase(pset.get<unsigned>("NumADCBaseline",1)),
    _mbbuffer(pset.get<double>("TimeBuffer",100.0)), // nsec
    _fittype((TrkHitReco::FitType) pset.get<unsigned>("FitType",TrkHitReco::peakminusped)),
    _truncateADC(pset.get<bool>("TruncateADC",true)), 
    _floatPedestal(pset.get<bool>("FloatPedestal",true)), 
    _floatWidth(pset.get<bool>("FloatWidth",true)), 
    _earlyPeak(pset.get<bool>("EarlyPeak",false)),
    _latePeak(pset.get<bool>("LatePeak",false)),
    _printLevel(pset.get<int>("printLevel",0)),
    _diagLevel(pset.get<int>("diagLevel",0)),
    _debugLevel(pset.get<int>("debugLevel",0)),
    _peakFitOption(pset.get<string>("PeakFitOption","QNSEX0B")),
    _maxFitIter(pset.get<unsigned>("MaxFitIterations",1)),
    _strawDigis(pset.get<string>("StrawDigis","makeSD")),
    _peakfit(pset.get<fhicl::ParameterSet>("PeakFitter",fhicl::ParameterSet())),
    _end{TrkTypes::cal,TrkTypes::hv}
  {
    produces<StrawHitCollection>();
    if(_printLevel > 0) cout << "In StrawHitsFromStrawDigis constructor " << endl;
  }

  StrawHitsFromStrawDigis::~StrawHitsFromStrawDigis() { }

  void StrawHitsFromStrawDigis::beginRun( art::Run& run ){
// create and configure the ADC waveform charge extraction fit
    TrkHitReco::FitConfig myconfig;
    myconfig._debug = _debugLevel;
    myconfig._maxnit = _maxFitIter;
    if(_floatWidth)myconfig.setOption(TrkHitReco::FitConfig::floatWidth);
    if(_floatPedestal)myconfig.setOption(TrkHitReco::FitConfig::floatPedestal);
    if(_truncateADC)myconfig.setOption(TrkHitReco::FitConfig::truncateADC);
    if(_earlyPeak)myconfig.setOption(TrkHitReco::FitConfig::earlyPeak);
    if(_latePeak)myconfig.setOption(TrkHitReco::FitConfig::latePeak);
    ConditionsHandle<StrawElectronics> strawele = ConditionsHandle<StrawElectronics>("ignored");

    if ( _fittype == TrkHitReco::FitType::peakminusped)
      _pfit = std::unique_ptr<TrkHitReco::PeakFit>(new TrkHitReco::PeakFit(*strawele,_peakfit) );
    else if (_fittype == TrkHitReco::FitType::combopeakfit)
      _pfit = std::unique_ptr<TrkHitReco::PeakFit>(new TrkHitReco::ComboPeakFitRoot(*strawele,_peakfit) );
    else
      _pfit = std::unique_ptr<TrkHitReco::PeakFit>(new TrkHitReco::PeakFitRoot(*strawele,_peakfit) );


    if(_printLevel > 0) cout << "In StrawHitsFromStrawDigis beginRun " << endl;
  }

  void StrawHitsFromStrawDigis::produce(art::Event& event) {
    if(_printLevel > 0) cout << "In StrawHitsFromStrawDigis produce " << endl;
// update conditions
    
    ConditionsHandle<StrawElectronics> strawele = ConditionsHandle<StrawElectronics>("ignored");
    ConditionsHandle<StrawPhysics> strawphys = ConditionsHandle<StrawPhysics>("ignored");
    unique_ptr<StrawHitCollection>             strawHits(new StrawHitCollection);
    // find the digis
    art::Handle<mu2e::StrawDigiCollection> strawdigisH; 
    const StrawDigiCollection* strawdigis(0);
    if(event.getByLabel(_strawDigis,strawdigisH))
      strawdigis = strawdigisH.product();
    if(strawdigis == 0)
      throw cet::exception("RECO")<<"mu2e::StrawHitsFromStrawDigis: No StrawDigi collection found for label " <<  _strawDigis << endl;

    // loop over digis
    size_t ndigi = strawdigis->size();
    strawHits->reserve(ndigi);

    for(size_t isd=0;isd<ndigi;++isd){
      StrawDigi const& digi = (*strawdigis)[isd];
      // convert the digi to a hit
      TDCTimes times;
      // convert TDC values to times.  Note this is merely a unit change (pedestal and scale), physical effects coming from
      // drift, electronics, particle propagation, etc are NOT corrected here
      strawele->tdcTimes(digi.TDC(),times);
      // convert the digi TOT to physical units.  This needs to be implemented FIXME!!
      TOTTimes tots{0.0,0.0};
      for(size_t iend=0;iend<2;++iend){
	tots[iend] = digi.TOT(_end[iend])*strawele->totLSB();
      }
      // fit the ADC waveform to get the charge
      ADCWaveform const& adc = digi.adcWaveform();
      // note: pedestal is being subtracting inside strawele, in the real experiment we will need
      // per-channel version of this FIXME!!!
      TrkHitReco::PeakFitParams params;
      _pfit->process(adc,params);
      if(_debugLevel > 0){
	cout << "Fit status = " << params._status << " NDF = " << params._ndf << " chisquared " << params._chi2
	  << " Fit charge = " << params._charge << " Fit time = " << params._time << endl;
      }
      // use time division to correct for attenuation FIXME!!
      // the gain should come from a straw-dependent database FIXME!!
      double energy = strawphys->ionizationEnergy(params._charge/strawphys->strawGain());
      // crate the straw hit and append it to the list
      StrawHit newhit(digi.strawIndex(),times,tots,energy);
      strawHits->push_back(newhit);
    }
    // put objects into event
    event.put(move(strawHits));
  }
}
using mu2e::StrawHitsFromStrawDigis;
DEFINE_ART_MODULE(StrawHitsFromStrawDigis);

