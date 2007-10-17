#ifndef JoinEvents_h
#define JoinEvents_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>

#include "IO/LCReader.h"
//#include "IO/LCWriter.h"

using namespace lcio ;
using namespace marlin ;

/** JoinEvents processor allows join events (based on the run number and event number) from a different file.
 *  All collections in the  event are added to the current event, provided their collection name does not yet exist in the
 *  event.
 * 
 * @author F. Gaede, DESY
 * @version $Id: JoinEvents.h,v 1.1 2007-10-17 17:19:22 gaede Exp $
 * 
 * @param InputFileName: the names (with absolute or relative pathes) of the one input file that 
 * is searched for events with the same run and event number.
 */
class JoinEvents : public Processor {
  
 public:
  
  virtual Processor*  newProcessor() { return new JoinEvents ; }
  
  
  JoinEvents() ;
  
  virtual const std::string & name() const { return Processor::name() ; }
  

  /** Open the LCIO input file.
   */
  virtual void init() ;
  
  /** Called for every run.
   */
  virtual void processRunHeader( LCRunHeader* run ) ;
  
  /** Search for the corresponding event and add all collections to the current event.
   */
  virtual void processEvent( LCEvent * evt ) ; 
  
  
  virtual void check( LCEvent * evt ) ; 
  
  
  /** Close the input file.
   */
  virtual void end() ;
  
  
 protected:

  /** Input file names.
   */
  std::string _fileName ;

  LCReader* _lcReader ;

  //  int _activeRunNumber;
  int _nRun ;
  int _nEvt ;
} ;

#endif



