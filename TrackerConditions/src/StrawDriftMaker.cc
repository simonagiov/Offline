#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include "TMath.h"

#include "TrackerConditions/inc/StrawDriftMaker.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib_except/exception.h"
#include "GeometryService/inc/GeomHandle.hh"
#include "TrackerGeom/inc/Tracker.hh"
#include "BFieldGeom/inc/BFieldManager.hh"
#include "BTrk/BField/BField.hh"
#include "GeometryService/inc/DetectorSystem.hh"
#include "CLHEP/Matrix/Vector.h"


using namespace std;

namespace mu2e {

  StrawDrift::ptr_t StrawDriftMaker::fromFcl() {

    double wirevoltage = _config.wireVoltage();

    std::vector<double> dataEField = _config.kVcm();
    std::vector<double> dataVInst = _config.cmus();
    if ( dataEField.size()==0 || dataEField.size()!=dataVInst.size() ) {
      throw cet::exception("STRAW_DRIFT_BADMODEL")
	<< "input drift model don't make sense, sizes:"<< dataEField.size() 
	<< " " << dataVInst.size()  << "\n";
    }
    //the set of distances that correspond to a given E-field
    std::vector<double> dataDistances; 
    //convert the E-field from KV/cm to V/mm
    for(auto& x : dataEField) x *= 100.0;
    //convert the speed from cm/us to mm/ns (10/1000 = 0.01)
    for(auto& x : dataVInst) x *= 0.01;

    // Use the E:insta-velc tables to build d:insta-veloc tables 
    // based on the voltage input
    
    //define the wire and straw radius in mm
    GeomHandle<Tracker> tracker;
    //12.5 um in mm
    double wireradius = tracker->wireRadius(); 
    //2.5 mm in mm
    double strawradius = tracker->strawOuterRadius(); 
    
    // calculate the distances that correspond to the efields 
    // listed in the table (fix units!!)
    double logRadius = log(strawradius/wireradius);
    for (size_t i=0; i < dataEField.size(); i++) {
      dataDistances.push_back(wirevoltage/((dataEField[i])*logRadius)); //in mm
    }

    size_t phiBins = _config.phiBins();

    // populate vectors of gamma based on the conditions value of B, 
    // and phiBins values of phi from 0 to pi/2
    //IN THE GETGAMMA FUNCTION, THE PHI INTERPOLATION WILL BE DONE, AS WELL AS THE 0-2PI ->0-PI/2 REDUNDANCY
    GeomHandle<BFieldManager> bfmgr;
    GeomHandle<DetectorSystem> det;
    CLHEP::Hep3Vector vpoint_mu2e = det->toMu2e(CLHEP::Hep3Vector(0.0,0.0,0.0));
    float Bz = bfmgr->getBField(vpoint_mu2e).z();

    float CC = Bz*logRadius/wirevoltage; 

    double deltaD = _config.deltaDistance();
    std::vector<double> distances;
    std::vector<double> instantSpeed;
    std::vector<double> times;

    double deltaPhi = TMath::Pi()/2.0/double(phiBins-1);
    
    size_t index = dataDistances.size()-2;
    double tempD = 0;
    double tempT = 0;
    for (size_t p=0;p<phiBins;p++)
      times.push_back(tempT);
    distances.push_back(tempD);
    instantSpeed.push_back(0);
    tempD += deltaD;
    while (tempD < strawradius){
      while (dataDistances[index] < tempD){
        if (index == 0)
          break;
        index -= 1;
      }
      double dist0 = dataDistances[index+1];
      double dist1 = dataDistances[index];
      double speed0 = dataVInst[index+1];
      double speed1 = dataVInst[index];
      double speed = speed0 + (speed1-speed0) * (tempD - dist0)/(dist1-dist0);
      distances.push_back(tempD);
      instantSpeed.push_back(speed);
      tempT += deltaD/speed;

      double vavg = tempD/tempT;
      
      double C = CC*vavg*1000000.0;//convert mm/ns to m/s

      for (size_t p=0;p<phiBins;p++){
        double tempPhi = deltaPhi * p;
        double zetta = C*tempD*0.001;//convert mm to m
        double tempGamma = (1 + pow(zetta,2)/3.)/(1 + pow(zetta*cos(tempPhi),2)/3.);
        times.push_back(tempT*tempGamma);
      }
      tempD += deltaD;
    }
    instantSpeed[0] = instantSpeed[1];

    auto ptr = std::make_shared<StrawDrift>(phiBins, deltaD, distances, instantSpeed, times);
    
    return ptr;

  } // fromFcl
  

  StrawDrift::ptr_t StrawDriftMaker::fromDb() {
    return fromFcl();
  }

}

