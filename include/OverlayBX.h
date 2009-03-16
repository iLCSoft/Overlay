#ifndef OverlayBX_h
#define OverlayBX_h 1

#include "marlin/Processor.h"
#include "marlin/EventModifier.h"
#include "lcio.h"
#include <string>
#include <vector>

#include "IO/LCReader.h"
#include "IO/LCWriter.h"

#ifdef MARLIN_USE_AIDA
#include <vector>
#include <AIDA/AIDA.h>
typedef std::vector< AIDA::IHistogram1D* > Hist1DVec ;
#endif

using namespace lcio ;
using namespace marlin ;

#include "CLHEP/Vector/TwoVector.h"

/** Helper struct for VXD ladder geometry */
struct VXDLadder{
  double phi ;   // phi off ladder 
  CLHEP::Hep2Vector p0 ;  // 'left' end of ladder in r-phi
  CLHEP::Hep2Vector p1 ;  // 'right' end of ladder in r-phi
  CLHEP::Hep2Vector u  ;  // unit vector along ladder in r-phi
};
typedef std::vector< std::vector< VXDLadder > > VXDLadders ;

/** Helper struct for VXD layer geometry */
struct VXDLayer{
  int nBX ;
  double width ;
  double ladderArea ;
  int nLadders ;
};
typedef std::vector< VXDLayer >  VXDLayers ;



/** OverlayBX processor for overlaying (pair) background from many bunch crossings.
 *  SimTrackerHits from the TPC and the VXD detectors are overlayed for the number of 
 *  bunchcrossings that will be visible for a given physics event, according to the bunch 
 *  crossing frequency and the corresponding readout characteristics (drift time) of the devices. 
 *  Hits from different bunch crossings are shifted in z (TPC) and r-phi (along the ladders) in the VXD.
 *  For all other detectors high time resolution is assumed and only one bunch crosing will be overalayd.
 * 
 *  @author F. Gaede DESY (based on Overlay processor by N. Chiapolini)
 *  @version $Id: OverlayBX.h,v 1.2 2009-03-16 16:15:00 gaede Exp $
 * 
 *  @param InputFileNames (StringVec) The names (with absolute or relative pathes) of the files from 
 *  which the background should be read. Events are read in random order from the files in the list with 
 *  possible dublication.
 *  It is the users responsibility to provide sufficient statistics for the signal sample under study.
 * 
 *  @param BunchCrossingTime [s] (float) - default 3.0e-7 (300 ns) 
 *  
 *  @param TPCDriftvelocity [mm/s] (float) - default 5.0e+7  (5cm/us)
 * 
 *  @param MaxBXsTPC (int) - maximum of BXs to be overlayed for the TPC; -1: 
 *                           compute from length and BXtime; default 10
 * 
 *  @param TPCCollection  collection of TPC SimTrackerHits  
 * 
 *  @param VXDLayerReadOutTimes [us] (FloatVec) - default "50. 50. 200. 200. 200. 200."
 * 
 *  @param VXDCollection collection of VXD SimTrackerHits
 * 
 *  @param RandomSeed (int) random seed - default 42
 * 
 *  @param MergeCollections (StringVec) Pairs of collection names from detectors that will have one 
 *  bunch crossing of beam background overlayd. 
 *  The input collection (given first) will be merged into the output collection. If the output 
 *  collectiondoes not exist, it will be created. 
 * 
 *  @param NumberOfEventsPerBX (int) 
 *  Fixed number of background events that are used for one bunch crossing.
 * 
 */

class OverlayBX : public Processor, public EventModifier {
  
 public:
  
  virtual Processor*  newProcessor() { return new OverlayBX ; }
  
  
  OverlayBX() ;
  
  virtual const std::string & name() const { return Processor::name() ; }
  
  virtual void modifyEvent( LCEvent * evt ) ; 


  /** Called at the begin of the job before anything is read.
   * Use to initialize the processor, e.g. book histograms.
   */
  virtual void init() ;
  
  /** Called for every run.
   */
  virtual void processRunHeader( LCRunHeader* run ) ;
  
  /** Called for every event - the working horse.
   */
  //  virtual void processEvent( LCEvent * evt ) ; 
  
  
  virtual void check( LCEvent * evt ) ; 
  
  
  /** Called after data processing for clean up.
   */
  virtual void end() ;
  
  
 protected:
  
  /** helper function for (randomly) reading the next event */
  LCEvent*  readNextEvent() ;
  /** helper function for reading the next event of BX bxNum */
  LCEvent*  readNextEvent(int bxNum) ;

  /** helper function */
  void init_geometry() ;
  /** helper function */
  int mergeVXDColsFromBX( LCCollection* vxdCol , LCCollection* vxdBGCol , int bxNum )  ;

  // variables for processor parameters 
  StringVec   _inputFileNames ;
  int         _eventsPerBX;
  float       _bxTime_s ;
  float       _tpcVdrift_mm_s ;
  int         _maxBXsTPC ;
  FloatVec    _vxdLayerReadOutTimes ;
  std::string _tpcCollection ;
  std::string _vxdCollection ;
  StringVec   _mergeCollections ;
  int         _ranSeed  ;

  // class member variables
  std::map<std::string, std::string> _colMap;
  std::vector< LCReader* > _lcReaders ;
  int _maxBXs ;
  int _nRun ;
  int _nEvt ;
  VXDLadders _vxdLadders ;
  VXDLayers  _vxdLayers ;

#ifdef MARLIN_USE_AIDA
   Hist1DVec _hist1DVec ;
#endif

} ;

#endif



