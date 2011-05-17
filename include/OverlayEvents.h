#ifndef OverlayEvents_h
#define OverlayEvents_h 1

#include "marlin/Processor.h"
#include "marlin/EventModifier.h"
#include "lcio.h"
#include <string>
#include <IMPL/LCEventImpl.h>

#include "IO/LCWriter.h"

using namespace lcio ;
using namespace marlin ;

/** OverlayEvents processor allows to merge a number of events in 
 * a LCIO file into 1 event.
 * See Merger.cc for the collection types that can be merged.
 * 
 * @author D. Kamai, Tohoku univ.
 * 
 * @param mergedCollectionNames (StringVec) The names (with absolute or relative pathes) of collection which will be merged.(not type of colletion)
 * @param OutputFileName (strign) The name of output file.\\
 */
class OverlayEvents : public Processor , public EventModifier {
  
 public:
  
  virtual Processor*  newProcessor() { return new OverlayEvents ; }
  
  
  OverlayEvents() ;
  
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

  std::string _outfileName ;

  LCEventImpl* outEvt ;
  
  double    _expBG;
  StringVec _mergedCollectionNames;
  
  LCWriter* _lcWriter ;
  
  int _activeRunNumber;
  int _nRun ;
  int _nEvt ;
} ;

#endif
