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

/** Overlay processor allows to overlay events from additional LCIO files 
 * based on different criterias. Under developement. 
 * 
 * @author N. Chiapolini, DESY
 * @version $Id: Overlay.h,v 1.4 2007-08-21 16:46:13 chiapoli Exp $
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
  double    _bgExpectation;
  
  
  StringVec _colVec;
  std::map<std::string, std::string> _colMap;

  LCReader* _lcReader ;

  int _nRun ;
  int _nEvt ;
} ;

#endif



