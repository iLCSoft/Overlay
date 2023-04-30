#ifndef OverlayTiming_h
#define OverlayTiming_h 1

#include "marlin/Processor.h"
#include "marlin/EventModifier.h"

#include "lcio.h"

#include <cmath>
#include <limits>

namespace EVENT{
  class SimCalorimeterHit;
  class LCRunHeader;
  class LCEvent;
  class LCCollection;
}

namespace overlay {

  /** OverlayTiming processor for overlaying background to each bunch crossing of a bunch train.
   * 
   *  A physics event is placed at a random or fixed position of the bunch train.
   *  Then background events are overlayed to each bunch crossing of the train and merged into the physics event.
   *
   *  For the merging, integration times can be given for each collection or subdetector, respectively.
   *  Then only hits are added to the physics event's collections, which fall into the integration time window of
   *  the specific subdetector
   * 
   *  @author P. Schade DESY/CERN (based on Overlay processor by F. Gaede)
   * 
   *  @param BackgroundFileNames (StringVec) The names (with absolute or relative pathes) of the files from 
   *  which the background should be read. 
   *  It is the users responsibility to provide sufficient statistics for the signal 
   *  sample under study.
   * 
   *  @param Delta_t [ns] (float) - default 0.5 (0.5 ns) (CliC default) -- Time difference between BXs in the BXtrain 
   *
   *  @param NBunchtrain (int) - default 1 -- Number of bunches in the bunch train
   *  
   *  @param TPCDriftvelocity [mm/s] (float) - default 5.0e+7  (5cm/us)
   * 
   *  @param PhysicsBX - default 1 -- BX of the physics event. Is overwritten if Random number is chosen
   *
   *  @param NumberBackground - default 1 -- Number of background events to overlay to one BX. This is either fixed (if Poisson_random_NOverlay = false) of the mean value if  (if Poisson_random_NOverlay = true)
   *
   *  @param Poisson_random_NOverlay - default false -- Overlay a random number of background events to each BX. If t, the number is pulled from a Poisson distribution.
   *
   *  @param RandomBx - default false -- Put the physics event at a random number of the bunch train
   *
   *  @param RandomSeed (int) random seed - default 42
   * 
   */
  class OverlayTiming : public marlin::Processor, public marlin::EventModifier
  {
  public:
    virtual marlin::Processor*  newProcessor();

    OverlayTiming();
    OverlayTiming(std::string const& name);
    OverlayTiming( OverlayTiming const&) = delete;
    OverlayTiming& operator=(OverlayTiming const&) = delete;

    virtual void init();

    virtual const std::string &name() const;

    virtual void processRunHeader(EVENT::LCRunHeader *run) ;

    virtual void modifyEvent(EVENT::LCEvent *evt);

    virtual void check(EVENT::LCEvent *evt);

    virtual void end();

  protected:
    float time_of_flight(float x, float y, float z) const;

    virtual void define_time_windows(const std::string &Collection_name);

    void crop_collection(EVENT::LCCollection *collection);

    void merge_collections(EVENT::LCCollection *source_collection, EVENT::LCCollection *dest_collection, float time_offset);

    unsigned long long cellID2long(unsigned int id0, unsigned int id1) const;

    float _T_diff = 0.5;
    int _nBunchTrain = 1;

    unsigned int _nRun = 0;
    unsigned int _nEvt = 0;
    StringVec _inputFileNames{};

    int _BX_phys = 1;
    float _NOverlay = 1 ;

    float _BeamCal_int = 10;
    float _ETD_int = 10;
    float _EcalBarrel_int = 10;
    float _EcalBarrelPreShower_int = 10;
    float _EcalEndcap_int = 10;
    float _EcalEndcapPreShower_int = 10;
    float _EcalEndcapRing_int = 10;
    float _EcalEndcapRingPreShower_int = 10;
    float _FTD_int = 10;
    float _HcalBarrelReg_int = 10;
    float _HcalEndCapRings_int = 10;
    float _HcalEndCaps_int = 10;
    float _LHcal_int = 10;
    float _LumiCal_int = 10;
    float _MuonBarrel_int = 10;
    float _MuonEndCap_int = 10;
    float _SET_int = 10;
    float _SIT_int = 10;
    float _VXD_int = 10;
    float _EcalPlug_int = 10;
    float _VXDB_int = 10;
    float _VXDE_int = 10;
    float _ITB_int = 10;
    float _ITE_int = 10;
    float _OTB_int = 10;
    float _OTE_int = 10;
    float _TPC_int = 10;
    float _TPCSpacePoint_int = 10;
    float _DefaultStart_int = -0.25;

    IO::LCReader* overlay_Eventfile_reader = NULL;
    LCEvent* overlay_Evt = nullptr;
    int m_eventCounter = 0;
    int m_currentFileIndex = 0;
    int m_startWithBackgroundFile = -1;
    int m_startWithBackgroundEvent = -1;
    bool m_allowReusingBackgroundFiles = true;

    float this_start = -0.25;
    float this_stop = std::numeric_limits<float>::max();

    std::string _mcParticleCollectionName = "";
    std::string _mcPhysicsParticleCollectionName = "";
    std::string currentDest = "";
    bool TPC_hits = false;

    float _tpcVdrift_mm_ns = 5.0e-2 ;
    bool _randomBX = false, _Poisson = false;

    typedef std::map<unsigned long long, EVENT::SimCalorimeterHit*> DestMap;
    typedef std::map<std::string, DestMap> CollDestMap;
    CollDestMap collDestMap{};
  };

  //------------------------------------------------------------------------------------------------------------------------------------------

  inline marlin::Processor *OverlayTiming::newProcessor()
  {
    return new OverlayTiming;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  inline const std::string &OverlayTiming::name() const
  {
    return marlin::Processor::name();
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  inline float OverlayTiming::time_of_flight(float x, float y, float z) const
  {
    //returns the time of flight to the radius in ns
    // mm/m/s = 10^{-3}s = 10^6 ns d.h. 299 mm/ns
    return std::sqrt((x * x) + (y * y) + (z * z))/299.792458;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  inline unsigned long long OverlayTiming::cellID2long(unsigned int id0, unsigned int id1) const
  {
    const unsigned long long newID = ((unsigned long long)(id0) << sizeof(unsigned int)*8 | (unsigned long long)id1);
    return newID;
  }

} // namespace

#endif

