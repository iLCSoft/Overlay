#ifndef Overlay_h
#define Overlay_h 1

#include "marlin/Processor.h"
#include "marlin/EventModifier.h"
#include "lcio.h"
#include <string>


namespace overlay {
  
  /**
   *  @brief  LCFileHandler class
   */
  class LCFileHandler {
  public:
    // Default constructors
    LCFileHandler() = default;
    LCFileHandler(const LCFileHandler&) = default;
    LCFileHandler& operator =(const LCFileHandler&) = default;

    /**
     *  @brief  Set the LCIO file name
     *  
     *  @param  fname the name of the LCIO file
     */
    void setFileName(const std::string& fname);
    
    /**
     *  @brief  Get the number of events available in the file
     */
    unsigned int getNumberOfEvents() const;
    
    /**
     *  @brief  Get the event number at the specified index (look in the event map)
     *  
     *  @param  index the nth event to get
     */
    unsigned int getEventNumber(unsigned int index) const;
    
    /**
     *  @brief  Get the run number at the specified index (look in the event map)
     *  
     *  @param  index the nth run to get
     */
    unsigned int getRunNumber(unsigned int index) const;
    
    /**
     *  @brief  Read the specified event, by run and event number
     *  
     *  @param  runNumber the run number of the event to read
     *  @param  eventNumber the event number of the event to read
     */
    EVENT::LCEvent* readEvent(int runNumber, int eventNumber);
    
  private:
    /**
     *  @brief  Proxy method to open the LCIO file
     */
    void openFile() const;

  private:
    mutable std::shared_ptr<IO::LCReader>          _lcReader{}; ///< The LCIO file reader
    mutable EVENT::IntVec                          _eventMap{}; ///< The run and event number 
    std::string                                    _fileName{}; ///< The LCIO file name    
  };
  
  typedef std::vector<LCFileHandler> LCFileHandlerList;

  /** Overlay processor allows to overlay an event with background events from 
   *  additional LCIO files based on different criteria.
   *
   *  A typical use case would be the overlay of gamma gamma -> hadrons background events
   *  with a number drawn from a poissonian distribution with a given mean 'expBG' (NumberOverlayEvents=0).
   *
   *  See Merger.cc for the collection types that can be merged.
   * 
   * @author N. Chiapolini, DESY
   * @author F. Gaede, DESY
   * @author R. Ete, DESY
   * @version $Id$
   * 
   * @param InputFileNames (StringVec) The names (with absolute or relative pathes) of the files from which the background should be read.
   *                                   Multiple files can be given by a white spaces separated list or by setting this parameter multiple times. 
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
   * @param ExcludeCollectionMap (StringVec) List of collection to exclude for merging. This is particularly useful when you just want to exclude a few collections.
   *                                   One doesn't have to specify all collections to overlay in the CollectionMap parameter minus the collection to avoid, 
   *                                   but just the ones to exclude. Priority is given to this list over the CollectionMap.                             
   */
  class Overlay final : public marlin::Processor, public marlin::EventModifier {
    // Deleted member functions : no copy
    Overlay( Overlay const&) = delete;
    Overlay& operator=(Overlay const&) = delete;
  public:
    /**
     *  @brief  Constructor
     */
    Overlay() ;
        
    /**
     *  @brief  Factory method to create a new processor
     */
    marlin::Processor* newProcessor() override { return new Overlay() ; }

    /**
     *  @brief  Modify the event
     *  
     *  @param  evt the event to modify
     */
    void modifyEvent( EVENT::LCEvent * evt ) override ; 

    /**
     *  @brief  Get the processor name. Need to override it because of EventModifier inheritance
     */
    const std::string & name() const override ;

    /** 
     *  @brief Called at the begin of the job before anything is read.
     *         Use to initialize the processor, e.g. book histograms.
     */
    void init() override ;
  
    /** 
     *  @brief  Called for every run.
     *
     *  @param  run the run header to process 
     */
    void processRunHeader( LCRunHeader* run ) override ;  
  
    /** 
     *  @brief  Called after data processing for clean up.
     */
    void end() override ;
  
  protected:
    /**
     *  @brief  Get the total number of event available in the overlay input files
     */
    unsigned int getNAvailableEvents() const; 

    /** 
     *  @brief  Helper method to randomly pick an event from available overlay input files 
     */
    LCEvent* readNextEvent() ;

  private:
    // processor parameters
    EVENT::StringVec                      _fileNames {} ;             ///< The overlay input file names
    EVENT::StringVec                      _overlayCollections {} ;    ///< The list of pair of collections to overlay (see class description)
    int                                   _numOverlay {0} ;           ///< The additional number of events to overlay
    double                                _expBG {1} ;                ///< The mean value of the poisson distribution when randomly picking events
    EVENT::StringVec                      _excludeCollections {} ;    ///< The list of collection to exclude for overlay
    
    // internal members
    unsigned int                          _nAvailableEvents {0} ;     ///< The total number of available overlay events from input files
    std::map<std::string, std::string>    _overlayCollectionMap {} ;  ///< The map of collections to overlay, built from _overlayCollections
    int                                   _nRun {0} ;                 ///< The total number of processed runs
    int                                   _nEvt {0} ;                 ///< The total number of processed events
    int                                   _nTotalOverlayEvents {0} ;  ///< The total number of overlaid events when processor ends
    LCFileHandlerList                     _lcFileHandlerList {} ;     ///< The list of file handler to manage overlay input files (see LCFileHandler class)
  } ;

}

#endif
