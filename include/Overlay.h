#ifndef Overlay_h
#define Overlay_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>

#include "IO/LCReader.h"
#include "IO/LCWriter.h"


using namespace lcio ;
using namespace marlin ;


/** Overlay processor allows to overlay events from additional LCIO files.
 * Under developmwnt - no overlay yet....
 */

class Overlay : public Processor {
  
 public:
  
  virtual Processor*  newProcessor() { return new Overlay ; }
  
  
  Overlay() ;
  
  /** Called at the begin of the job before anything is read.
   * Use to initialize the processor, e.g. book histograms.
   */
  virtual void init() ;
  
  /** Called for every run.
   */
  virtual void processRunHeader( LCRunHeader* run ) ;
  
  /** Called for every event - the working horse.
   */
  virtual void processEvent( LCEvent * evt ) ; 
  
  
  virtual void check( LCEvent * evt ) ; 
  
  
  /** Called after data processing for clean up.
   */
  virtual void end() ;
  
  
 protected:

  /** Input file names.
   */
  StringVec _fileNames ;

  LCReader* _lcReader ;

  int _nRun ;
  int _nEvt ;
} ;

#endif



