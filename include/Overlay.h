#ifndef Overlay_h
#define Overlay_h 1

#include "marlin/Processor.h"
#include "marlin/EventModifier.h"
#include "lcio.h"
#include <string>

#include "IO/LCReader.h"
#include "IO/LCWriter.h"

using namespace lcio ;
using namespace marlin ;

/** Overlay processor allows to overlay an event with background events from 
 * additional LCIO files based on different criterias.
 * 
 * @author N. Chiapolini, DESY
 * @version $Id: Overlay.h,v 1.6 2007-09-12 08:43:10 chiapoli Exp $
 * 
 * @param InputFileNames list of files that contain the background events
 * @param NumberOverlayEvents Overlay each event with this number of background events. (default 0)
 * @param expBG Add additional background events to NumberOverlayEvents according to a poisson 
 * distribution with this expectation value.
 * @param CollectionMap Pairs of source and destination collections to be merged. (this order, multiple pairs possible)
 */
class Overlay : public Processor, public EventModifier {
  
 public:
  
  virtual Processor*  newProcessor() { return new Overlay ; }
  
  
  Overlay() ;
  
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

  /** Input file names.
   */
  StringVec _fileNames ;
  int       _numOverlay;
  double    _expBG;
  bool      _runOverlay;
  
  StringVec _colVec;
  std::map<std::string, std::string> _colMap;

  LCReader* _lcReader ;

  int _activeRunNumber;
  int _nRun ;
  int _nEvt ;
} ;

#endif



