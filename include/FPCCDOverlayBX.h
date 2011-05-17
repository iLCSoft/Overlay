#ifndef FPCCDOverlayBX_h
#define FPCCDOverlayBX_h 1

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

/** FPCCDOverlayBX processor for overlaying (pair) background from many bunch crossings.
 *  Only VTXPixelHits which is output of FPCCDDigitizer are overlayed for the number of 
 *  bunchcrossings that will be visible for a given physics events. 
 *  <b>Note: code assumes that background files contain exactly one bunch crossing - this is necesassary as 
 *     guineapig files are ordered.</b>
 * 
 *  @author D. Kamai Tohoku univ. (based on OverlayBX processor by F. Geade)

 * 
 *  @param BackgroundFileNames (StringVec) The names (with absolute or relative pathes) of the files from 
 *  which the background should be read. Events are read in order from the last of strings.
 *  The Duplication will not be occured.
 *
 *  @param NumberOfBunchCrossing (int) The number of bunch crossings will be merged.
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

class FPCCDOverlayBX : public Processor, public EventModifier {
  
 public:
  
  virtual Processor*  newProcessor() { return new FPCCDOverlayBX ; }
  
  
  FPCCDOverlayBX() ;
  
  virtual const std::string & name() const { return Processor::name() ; }

  virtual void modifyEvent( LCEvent * evt ) ; 

  /** Called at the begin of the job before anything is read.
   * Use to initialize the processor, e.g. book histograms.
   */
  virtual void init() ;
  
  /** Called for every run.
   */
  virtual void processRunHeader( LCRunHeader* run ) ;
  
  virtual void check( LCEvent * evt ) ; 
  
  
  /** Called after data processing for clean up.
   */
  virtual void end() ;
  
  
 protected:
  
  /** helper function for (randomly) reading the next event */
  //  LCEvent*  readNextEvent() ;

  /** helper function for reading the next event of BX bxNum */
  //  LCEvent*  readNextEvent(int bxNum) ;

  LCEvent*  readNextEvent(int bxNum) ;
  
  /** helper function */
  void init_geometry() ;

  // ---- variables for processor parameters ----- 
  StringVec   _inputFileNames ;
  StringVec   _tmpInputFileNames ; // not read same file twice or more.
  int         _eventsPerBX;
  int         _numBX ;
 
  
  std::string _vxdCollection ;
  std::string _vtxPixelHitsCollection ;
  StringVec   _mergeCollections ;
  int         _nLayer;
  int         _maxLadder;  
  //---- class member variables ------
  typedef std::map<std::string, std::string> StrMap ;
  StrMap _colMap;
   //  std::map<std::string, std::string> _colMap;

  std::vector< LCReader* > _lcReaders ;
  int _maxBXs ;
  int _nRun ;
  int _nEvt ;

  struct GeoData_t {
    int nladder;
  };
  std::vector<GeoData_t> _geodata;

#ifdef MARLIN_USE_AIDA
   Hist1DVec _hist1DVec ;
#endif

} ;

#endif
