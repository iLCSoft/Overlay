#ifndef Overlay_h
#define Overlay_h 1

#include "marlin/Processor.h"
#include "marlin/EventModifier.h"
#include "lcio.h"
#include <string>


namespace overlay{

  /** Overlay processor allows to overlay an event with background events from 
   *  additional LCIO files based on different criteria.
   *
   *  A typical use case would be the overlay of gamma gamma -> hadrons background events
   *  with a number drawn from a poissonian distribution with a given mean 'expBG' (NumberOverlayEvents=0).
   *  Best to specfiy only one input file as then direct access in LCIO is used.
   *
   *  See Merger.cc for the collection types that can be merged.
   * 
   * @author N. Chiapolini, DESY
   * @author F. Gaede, DESY
   * @version $Id$
   * 
   * @param InputFileNames (StringVec) The names (with absolute or relative pathes) of the files from which the background should be read.
   *                                   Multiple files can be given by a white spaces separated list or by setting this parameter multiple times. 
   *                                   If the end of the last file is reached, before all events have been processed, a warning will be printed 
   *                                   and reading restarted with the first file.
   *                                   NB: if only one file is given direct access is used - this is much faster (and thus preferred)
   *                              
   * @param CollectionMap (StringVec)  Pairs of collection names. The input collection (given first) will be merged into the output collection.
   *                                   If the output collection does not exist, it will be created. It is recommended to set this parameter once
   *                                   for each collection pair. If this parameter is not set, all collections with the same name and type will be merged. 
   *
   * @param NumberOverlayEvents (int)  Fixed number of background events that should be added to each physics event. (default 0)
   *
   * @param expBG (double)             If this value is set, a random number of background will be added to each physics event. 
   *                                   The Random numbers will be thrown according to a Poisson distribution with this expectation value.
   *                                   If set, NumberOverlayEvents will be added to the random number.
   * @param runOverlay (bool)          If true, NumberOverlayEvents and expBG will be ignored. Instead one run of background events will be added to each physics event.
   *
   * @param NSkipEventsRandom          Maximum number of events to skip between overlayd events (choosen from flat intervall [0,NSkipEventsRandom] )
   *                                   used if more than one inout file given and skipNEvent is used                                       
   */

  class Overlay : public marlin::Processor, public marlin::EventModifier {
  
  public:
  
    virtual marlin::Processor*  newProcessor() { return new Overlay ; }
  
  
    Overlay() ;
  
    virtual const std::string & name() const { return Processor::name() ; }
  
    virtual void modifyEvent( EVENT::LCEvent * evt ) ; 


    /** Called at the begin of the job before anything is read.
     * Use to initialize the processor, e.g. book histograms.
     */
    virtual void init() ;
  
    /** Called for every run.
     */
    virtual void processRunHeader( LCRunHeader* run ) ;
  
    /** Called for every event - the working horse.
     */
    //  virtual void processEvent( EVENT::LCEvent * evt ) ; 
  
  
    virtual void check( EVENT::LCEvent * evt ) ; 
  
  
    /** Called after data processing for clean up.
     */
    virtual void end() ;
  
  
  protected:


    /** Helper method.
     */
    LCEvent* readNextEvent( ) ;

    /** Input file names.
     */
    StringVec _fileNames ;
    int       _numOverlay;
    double    _expBG;
    bool      _runOverlay;
    int       _nSkipEventsRandom ;

    StringVec _colVec;
    std::map<std::string, std::string> _colMap;

    IO::LCReader* _lcReader ;
    EVENT::LCEvent* _overlayEvent ;

    int _activeRunNumber;
    int _nRun ;
    int _nEvt ;
    int _nOverlayEvt ;
    IntVec    _events ;
  } ;

}
#endif



