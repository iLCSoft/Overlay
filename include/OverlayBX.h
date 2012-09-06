#ifndef OverlayBX_h
#define OverlayBX_h 1

#include "marlin/Processor.h"
#include "marlin/EventModifier.h"
#include "lcio.h"
#include <string>
#include <vector>


#ifdef MARLIN_USE_AIDA
#include <vector>
#include <AIDA/AIDA.h>
typedef std::vector< AIDA::IHistogram1D* > Hist1DVec ;
#endif


#include "CLHEP/Vector/TwoVector.h"

namespace overlay{

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
    VXDLayer(): nBX(0), width(0), ladderArea(0), nLadders(0) {}
  };
  typedef std::vector< VXDLayer >  VXDLayers ;


  // /** Helper struct for TPC parameters */
  // struct TPCParameters{ };


  /** OverlayBX processor for overlaying (pair) background from many bunch crossings.
   *  SimTrackerHits from the TPC and the VXD detectors are overlayed for the number of 
   *  bunchcrossings that will be visible for a given physics event, according to the bunch 
   *  crossing frequency and the corresponding readout characteristics (drift time) of the devices. 
   *  Hits from different bunch crossings are shifted in z for the TPC accordingly.
   *  No shift in  r-phi (along the ladders) is applied for the VXD so far.
   *  For all other detectors high time resolution is assumed and only one bunch crosing will be overalayd.<br>
   *  <b>Note: code assumes that background files contain exactly one bunch crossing - this is necesassary as 
   *     guineapig files are ordered.</b>
   * 
   *  @author F. Gaede DESY (based on Overlay processor by N. Chiapolini)
   *  @version $Id: OverlayBX.h,v 1.4 2009-05-20 09:18:34 gaede Exp $
   * 
   *  @param BackgroundFileNames (StringVec) The names (with absolute or relative pathes) of the files from 
   *  which the background should be read. Events are read in random order from the files in the list with 
   *  possible dublication. It is the users responsibility to provide sufficient statistics for the signal 
   *  sample under study.
   * 
   *  @param BunchCrossingTime [s] (float) - default 3.0e-7 (300 ns) 
   *  
   *  @param TPCDriftvelocity [mm/s] (float) - default 5.0e+7  (5cm/us)
   * 
   *  @param MaxBXsTPC (int) - maximum of BXs to be overlayed for the TPC; -1: 
   *                           compute from length and BXtime; default 10
   * 
   *  @param TPCCollections  pairs of collection names with TPC SimTrackerHits  to be overlaid.
   *  The input collection (given first) will be merged into the output collection. If the output 
   *  collectiondoes not exist, it will be created. 
   * 
   *  @param PhiRotateTPCHits  - if set to true the bg events are rotated in azimuth by a random angle 
   *                             allows to re-use the same bg events more often
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
   *  @param MaxNumberOfEventsPerFile (int) 
   *  Maximum number of background events to be read from one file. Default: -1, i.e. read one file per BX.
   *  This option is essentially for testing. 
   */

  class OverlayBX : public marlin::Processor, public marlin::EventModifier {
    
  public:
  
    virtual marlin::Processor*  newProcessor() { return new OverlayBX ; }
  
  
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
    //  LCEvent*  readNextEvent() ;

    /** helper function for reading the next event of BX bxNum */
    LCEvent*  readNextEvent(int bxNum) ;

    /** helper function */
    void init_geometry() ;
    /** helper function */
    int mergeVXDColsFromBX( LCCollection* vxdCol , LCCollection* vxdBGCol , int bxNum )  ;
    /** helper function */
    int mergeTPCColsFromBX( LCCollection* tpcCol , LCCollection* tpcBGCol , float zPosShift ) ;

    // ---- variables for processor parameters ----- 
    StringVec   _inputFileNames ;
    int         _eventsPerBX;
    float       _bxTime_s ;
    float       _tpcVdrift_mm_s ;
    int         _maxBXsTPC ;

    bool        _phiRotateTPCHits ;

    FloatVec    _vxdLayerReadOutTimes ;

    StringVec   _tpcCollections ;
    //  std::string _tpcCollection ;
  
    std::string _vxdCollection ;
    StringVec   _mergeCollections ;
    int         _ranSeed  ;

    //---- class member variables ------
    typedef std::map<std::string, std::string> StrMap ;
    StrMap _tpcMap;
    StrMap _colMap;
    //  std::map<std::string, std::string> _colMap;

    std::vector< LCReader* > _lcReaders ;
    int _maxBXs ;
    int _nRun ;
    int _nEvt ;
    //  VXDLadders _vxdLadders ;
    VXDLayers  _vxdLayers ;

    int _lastBXNum  ;
    int _lastEvent  ; 
    int _currentRdr ;
  
#ifdef MARLIN_USE_AIDA
    Hist1DVec _hist1DVec ;
#endif

  } ;

} // namespace 


#endif



