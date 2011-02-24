#ifndef OverlayTiming_h
#define OverlayTiming_h 1

#include "marlin/Processor.h"
#include "marlin/EventModifier.h"

#include "lcio.h"

#include <cmath>

using namespace lcio;
using namespace marlin;

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
class OverlayTiming : public Processor, public EventModifier
{
public:
    virtual Processor*  newProcessor();

    OverlayTiming();

    virtual void init();

    virtual const std::string &name() const;

    virtual void processRunHeader(EVENT::LCRunHeader *run) ;

    virtual void modifyEvent(EVENT::LCEvent *evt);

    virtual void check(EVENT::LCEvent *evt);

    virtual void end();

protected:
    float time_of_flight(float x, float y, float z) const;

    void define_time_windows(const std::string &Collection_name);

    void crop_collection(EVENT::LCCollection *collection);

    void merge_collections(EVENT::LCCollection *source_collection, EVENT::LCCollection *dest_collection, float time_offset);

    long long cellID2long(int id0, int id1) const;

    float _T_diff;
    int _nBunchTrain;

    unsigned int _nRun ;
    unsigned int _nEvt ;
    StringVec _inputFileNames ; 

    int _BX_phys;
    float _NOverlay;
    unsigned int overlay_file_list;
    float _BeamCal_int,_ETD_int, _EcalBarrel_int, _EcalBarrelPreShower_int, _EcalEndcap_int, _EcalEndcapPreShower_int, _EcalEndcapRing_int, _EcalEndcapRingPreShower_int;
    float _FTD_int, _HcalBarrelReg_int, _HcalEndCapRings_int, _HcalEndCaps_int, _LHcal_int, _LumiCal_int, _MuonBarrel_int, _MuonEndCap_int, _SET_int, _SIT_int, _VXD_int;
    float _TPC_int, _TPCSpacePoint_int;

    IO::LCReader* overlay_Eventfile_reader;

    float this_start;
    float this_stop;
    int _ranSeed;

    std::string _mcParticleCollectionName;

    bool TPC_hits;

    float _tpcVdrift_mm_ns ;
    bool _randomBX, _Poisson;

    typedef std::map<long long, EVENT::SimCalorimeterHit*> DestMap;
    DestMap destMap;
};

//------------------------------------------------------------------------------------------------------------------------------------------

Processor *OverlayTiming::newProcessor()
{
    return new OverlayTiming;
}

//------------------------------------------------------------------------------------------------------------------------------------------

const std::string &OverlayTiming::name() const
{
    return Processor::name();
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline float OverlayTiming::time_of_flight(float x, float y, float z) const
{
    //returns the time of flight to the radius in ns
    // mm/m/s = 10^{-3}s = 10^6 ns d.h. 299 mm/ns
    return std::sqrt((x * x) + (y * y) + (z * z))/299.792458;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline long long OverlayTiming::cellID2long(int id0, int id1) const
{
    return ((long long) id0 << 32) | id1;
}

#endif
