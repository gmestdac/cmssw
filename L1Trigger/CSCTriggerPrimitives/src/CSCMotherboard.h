#ifndef L1Trigger_CSCTriggerPrimitives_CSCMotherboard_h
#define L1Trigger_CSCTriggerPrimitives_CSCMotherboard_h

/** \class CSCMotherboard
 *
 * Correlates anode and cathode LCTs from the same chamber.
 *
 * When the Trigger MotherBoard (TMB) is instantiated it instantiates an ALCT
 * and CLCT board.  The MotherBoard takes up to two LCTs from each anode
 * and cathode LCT card and combines them into a single Correlated LCT.
 * The output is up to two Correlated LCTs.
 *
 * It can be run in either a test mode, where the arguments are a collection
 * of wire times and arrays of halfstrip and distrip times, or
 * for general use, with wire digi and comparator digi collections as
 * arguments.  In the latter mode, the wire & strip info is passed on the
 * LCTProcessors, where it is decoded and converted into a convenient form.
 * After running the anode and cathode LCTProcessors, TMB correlates the
 * anode and cathode LCTs.  At present, it simply matches the best CLCT
 * with the best ALCT; perhaps a better algorithm will be determined in
 * the future.  The MotherBoard then determines a few more numbers (such as
 * quality and pattern) from the ALCT and CLCT information, and constructs
 * two correlated LCT "digis".
 *
 * \author Benn Tannenbaum 28 August 1999 benn@physics.ucla.edu
 *
 * Based on code by Nick Wisniewski (nw@its.caltech.edu) and a framework
 * by Darin Acosta (acosta@phys.ufl.edu).
 *
 * Numerous later improvements by Jason Mumford and Slava Valuev (see cvs
 * in ORCA).
 * Porting from ORCA by S. Valuev (Slava.Valuev@cern.ch), May 2006.
 *
 *
 */

#include "L1Trigger/CSCTriggerPrimitives/src/CSCAnodeLCTProcessor.h"
#include "L1Trigger/CSCTriggerPrimitives/src/CSCCathodeLCTProcessor.h"
#include "DataFormats/CSCDigi/interface/CSCCorrelatedLCTDigi.h"

class CSCGeometry;

class CSCMotherboard
{
 public:
  /** Normal constructor. */
  CSCMotherboard(unsigned endcap, unsigned station, unsigned sector,
		 unsigned subsector, unsigned chamber,
		 const edm::ParameterSet& conf);

  /** Constructor for use during testing. */
  CSCMotherboard();

  /** Default destructor. */
  virtual ~CSCMotherboard() = default;

  /** Test version of run function. */
  void run(const std::vector<int> w_time[CSCConstants::NUM_LAYERS][CSCConstants::MAX_NUM_WIRES],
	   const std::vector<int> hs_times[CSCConstants::NUM_LAYERS][CSCConstants::NUM_HALF_STRIPS_7CFEBS],
	   const std::vector<int> ds_times[CSCConstants::NUM_LAYERS][CSCConstants::NUM_HALF_STRIPS_7CFEBS]);

  /** Run function for normal usage.  Runs cathode and anode LCT processors,
      takes results and correlates into CorrelatedLCT. */
  void run(const CSCWireDigiCollection* wiredc, const CSCComparatorDigiCollection* compdc);

  /** Returns vector of correlated LCTs in the read-out time window, if any. */
  std::vector<CSCCorrelatedLCTDigi> readoutLCTs();

  /** Returns vector of all found correlated LCTs, if any. */
  std::vector<CSCCorrelatedLCTDigi> getLCTs();

  /** Clears correlated LCT and passes clear signal on to cathode and anode
      LCT processors. */
  void clear();

  /** Set configuration parameters obtained via EventSetup mechanism. */
  void setConfigParameters(const CSCDBL1TPParameters* conf);

  void setCSCGeometry(const CSCGeometry *g) { csc_g = g; }

  /** Anode LCT processor. */
  std::unique_ptr<CSCAnodeLCTProcessor> alct;

  /** Cathode LCT processor. */
  std::unique_ptr<CSCCathodeLCTProcessor> clct;

 // VK: change to protected, to allow inheritance
 protected:

  /** Verbosity level: 0: no print (default).
   *                   1: print LCTs found. */
  int infoV;

  /** Chamber id (trigger-type labels). */
  const unsigned theEndcap;
  const unsigned theStation;
  const unsigned theSector;
  const unsigned theSubsector;
  const unsigned theTrigChamber;
  unsigned theRing;

  const CSCGeometry* csc_g;

  /** Flag for MTCC data. */
  bool isMTCC;

  /** Flag for new (2007) version of TMB firmware. */
  bool isTMB07;

  /** Flag for SLHC studies. */
  bool isSLHC;

  /** Configuration parameters. */
  unsigned int mpc_block_me1a;
  unsigned int alct_trig_enable, clct_trig_enable, match_trig_enable;
  unsigned int match_trig_window_size, tmb_l1a_window_size;

  /** SLHC: whether to not reuse ALCTs that were used by previous matching CLCTs */
  bool drop_used_alcts;

  /** SLHC: separate handle for early time bins */
  int early_tbins;

  /** SLHC: whether to readout only the earliest two LCTs in readout window */
  bool readout_earliest_2;

  /** Default values of configuration parameters. */
  static const unsigned int def_mpc_block_me1a;
  static const unsigned int def_alct_trig_enable, def_clct_trig_enable;
  static const unsigned int def_match_trig_enable, def_match_trig_window_size;
  static const unsigned int def_tmb_l1a_window_size;

  /** Container for first correlated LCT. */
  CSCCorrelatedLCTDigi firstLCT[CSCConstants::MAX_LCT_TBINS];

  /** Container for second correlated LCT. */
  CSCCorrelatedLCTDigi secondLCT[CSCConstants::MAX_LCT_TBINS];

  /** Make sure that the parameter values are within the allowed range. */
  void checkConfigParameters();

  void correlateLCTs(CSCALCTDigi bestALCT, CSCALCTDigi secondALCT,
                     CSCCLCTDigi bestCLCT, CSCCLCTDigi secondCLCT);
  CSCCorrelatedLCTDigi constructLCTs(const CSCALCTDigi& aLCT,
                                     const CSCCLCTDigi& cLCT,
                                     int type) const;
  unsigned int encodePattern(const int ptn, const int highPt) const;
  unsigned int findQuality(const CSCALCTDigi& aLCT, const CSCCLCTDigi& cLCT) const;

  enum LCT_Quality{
    INVALID  = 0,
    NO_CLCT = 1,
    NO_ALCT = 2,
    CLCT_LAYER_TRIGGER = 3,
    LOW_QUALITY = 4,
    MARGINAL_ANODE_CATHODE = 5,
    HQ_ANODE_MARGINAL_CATHODE = 6,
    HQ_CATHODE_MARGINAL_ANODE = 7,
    HQ_ACCEL_ALCT = 8,
    HQ_RESERVED_1 = 9,
    HQ_RESERVED_2 = 10,
    HQ_PATTERN_2_3 = 11,
    HQ_PATTERN_4_5 = 12,
    HQ_PATTERN_6_7 = 13,
    HQ_PATTERN_8_9 = 14,
    HQ_PATTERN_10 = 15
  };

  /** Dump TMB/MPC configuration parameters. */
  void dumpConfigParams() const;

  // Method for tests
  void testLCT();
};
#endif
