#ifndef JoinEvents_h
#define JoinEvents_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>

namespace overlay{

  /** JoinEvents processor allows join events (based on the run number and event number) from a different file.
   *  All collections in the  event are added to the current event, provided their collection name does not yet exist in the
   *  event. If the collection names are the same the user can optionally specify a post fix that is added to all
   *  collection names from the joined event in order to avoid name clashes.
   * 
   * @author F. Gaede, DESY
   * @version $Id$
   * 
   * @param InputFileName: the names (with absolute or relative pathes) of the one input file that 
   * is searched for events with the same run and event number.
   * @param ColNamePostFix: optional post fix that is added to all collection names
   */
  class JoinEvents : public marlin::Processor {
  
  public:
  
    virtual marlin::Processor*  newProcessor() { return new JoinEvents ; }
  
  
    JoinEvents() ;
    JoinEvents( JoinEvents const& ) = delete;
    JoinEvents& operator=(const overlay::JoinEvents&) = delete;
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

    /** Input file names. */
    std::string _fileName = "";

    /** post fix for collection names. */
    std::string _postFix = "";

    LCReader* _lcReader = NULL;

    //  int _activeRunNumber;
    int _nRun = 0;
    int _nEvt = 0;
  } ;

} // namespace 

#endif



