#ifndef TEveMu2eHit_h
#define TEveMu2eHit_h

#include <TObject.h>
#include <string.h>
#include <string>
#include <TEvePointSet.h>
#include <TEveLine.h>
#include <TMath.h>
#include "RecoDataProducts/inc/ComboHit.hh"
//Mu2e General:
#include "GeometryService/inc/GeomHandle.hh"
#include "GeometryService/inc/DetectorSystem.hh"
#include "TEveEventDisplay/src/dict_classes/GeomUtils.h"
#include "TEveEventDisplay/src/TEveMu2e_base_classes/TEveMu2eCustomHelix.h"
#include "TrackerGeom/inc/Tracker.hh"

namespace mu2e {
  class TEveMu2eHit : public TEvePointSet {
    public:
      #ifndef __CINT__
      explicit TEveMu2eHit();
      TEveMu2eHit(ComboHit chit) : fComboHit_(chit){};
      virtual ~TEveMu2eHit(){};

      ComboHit fComboHit_; 
      Int_t mSize_ = 3; 
      bool AddErrorBar_ = false;

      void DrawHit2D(const std::string &pstr, Int_t b,CLHEP::Hep3Vector HitPos, int energylevel, TEveElementList *list, TEveElementList *listz);
      void DrawHit3D(const std::string &pstr, Int_t b,CLHEP::Hep3Vector HitPos, int energylevel, TEveElementList *list); 
      void DrawHitCollection(const std::string &pstr, size_t i, std::vector<CLHEP::Hep3Vector> HitPos, int energylevel, TEveElementList *list); 
      XYZVec const GetPosition() { return fComboHit_.pos();}
      double GetEnergy(){ return fComboHit_.energyDep();}
      std::string DataTitle(const std::string &pstr, int n);
      #endif
      ClassDef(TEveMu2eHit, 0);
    };
    typedef std::vector<mu2e::TEveMu2eHit> TEveMu2eHitCollection;
}
#endif
