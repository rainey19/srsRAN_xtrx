
#ifndef SRSGNB_BWP_CONFIGURATION_H
#define SRSGNB_BWP_CONFIGURATION_H

#include "ofdm_symbol_range.h"
#include "resource_block.h"
#include "srsgnb/adt/optional.h"
#include "srsgnb/adt/slot_array.h"
#include "srsgnb/ran/frame_types.h"
#include "srsgnb/ran/prb_grant.h"
#include <bitset>

namespace srsgnb {

/// Max number of frequency domain resources for the CORESET. Each resource corresponds to a group of 6 PRBs.
/// \remark See TS 38.331, "ControlResourceSet"
const size_t CORESET_FREQ_DOMAIN_RESOURCE_SIZE = 45;

/// Max number of OFDM symbols in a control resource set.
/// \remark See TS 38.331, "maxCoReSetDuration".
const size_t MAX_CORESET_DURATION = 3;

/// \brief CORESET identifier. This value is UE-specific, which means that a UE can have up to
/// "maxNrofControlResourceSets" CORESETS configured.
/// \remark See TS 38.331, "maxNrofControlResourceSets".
enum coreset_id : uint8_t { MIN_CORESET_ID = 0, MAX_CORESET_ID = 11, MAX_NOF_CORESETS = 12 };

inline coreset_id to_coreset_id(unsigned cs_id)
{
  return static_cast<coreset_id>(cs_id);
}

/// CORESET configuration.
/// \remark See TS 38.331, "ControlResourceSet".
struct coreset_configuration {
  struct interleaved_mapping_type {
    /// Values: (2, 3, 6).
    unsigned reg_bundle_sz;
    /// Values: (2, 3, 6).
    unsigned interleaver_sz;
    /// Values: (0..MAX_NOF_PRBS-1).
    optional<unsigned> shift_index;
  };
  enum class precoder_granularity_type { same_as_reg_bundle, all_contiguous_rbs };

  coreset_id                                     id;
  std::bitset<CORESET_FREQ_DOMAIN_RESOURCE_SIZE> freq_domain_resources;
  /// Duration in number of symbols. Values: (1..maxCORESETDuration).
  unsigned                           duration;
  optional<interleaved_mapping_type> interleaved;
  precoder_granularity_type          precoder_granurality;
};

/// \brief Search Space identifier. This value is UE-specific, which means that a UE can have up to
/// "maxNrofControlResourceSets" CORESETS configured.
enum search_space_id : uint8_t { MIN_SEARCH_SPACE_ID = 0, MAX_SEARCH_SPACE = 39, MAX_NOF_SEARCH_SPACES = 40 };

inline search_space_id to_search_space_id(unsigned ss_id)
{
  return static_cast<search_space_id>(ss_id);
}

/// SearchSpace configuration.
/// \remark See TS 38.331, "SearchSpace".
struct search_space_configuration {
  /// SearchSpace Type.
  enum search_space_type { common, ue_dedicated };

  search_space_id id;
  coreset_id      cs_id;
  /// Number of consecutive slots that a SearchSpace lasts in every occasion. Values: (1..2559).
  unsigned duration;
  /// The first symbol(s) for PDCCH monitoring in the slots for PDCCH monitoring. The most significant bit represents
  /// the first OFDM in a slot.
  optional<std::bitset<NOF_OFDM_SYM_PER_SLOT_NORMAL_CP> > monitoring_symbols_within_slot;
  /// Number of PDCCH candidates per aggregation level. The aggregation level for the array element with index "x"
  /// is L=1U << x. The possible values for each element are {0, 1, 2, 3, 4, 5, 6, 8}.
  std::array<uint8_t, 5> nof_candidates;
  search_space_type      type;
};

struct pdcch_configuration_common {
  /// May Contain CORESET#0 and common CORESET.
  slot_vector<coreset_configuration>      coresets;
  slot_vector<search_space_configuration> search_spaces;
  search_space_id                         sib1_search_space_id;
  search_space_id                         other_si_search_space_id;
  search_space_id                         paging_search_space_id;
  search_space_id                         ra_search_space_id;
};

/// Generic parameters of a bandwidth part as defined in TS 38.211, clause 4.5 and TS 38.213, clause 12.
/// \remark See TS 38.331, Bandwidth-Part (BWP).
struct bwp_configuration {
  bool               cp_extended;
  subcarrier_spacing scs;
  crb_interval       crbs;
};

/// Used to configure the common, cell-specific parameters of a DL BWP.
/// \remark See TS 38.331, BWP-DownlinkCommon.
struct bwp_downlink_common {
  bwp_configuration          generic_params;
  pdcch_configuration_common pdcch_common;
};

/// \remark See TS 38.331, RACH-ConfigGeneric.
struct rach_config_generic {
  /// Msg2 RAR window length in #slots. Network configures a value < 10msec. Values: (1, 2, 4, 8, 10, 20, 40, 80).
  unsigned ra_resp_window;
};

struct rach_config_common {
  rach_config_generic rach_cfg_generic;
  /// Total number of prambles used for contention based and contention free RA. Values: (1..64).
  unsigned total_nof_ra_preambles;
  /// PRACH root sequence index. Values: (1..839).
  /// \remark See TS 38.211, clause 6.3.3.1.
  bool     prach_root_seq_index_l839_present;
  unsigned prach_root_seq_index;
};

struct pusch_time_domain_resource_allocation {
  enum class mapping_type { typeA, typeB };

  /// Values: (0..32).
  unsigned     k2;
  mapping_type map_type;
  /// OFDM symbol boundaries for PUSCH. Network configures the fields so it does not cross the slot boundary.
  ofdm_symbol_range symbols;
};

/// \remark See TS 38.331, "PUSCH-ConfigCommon".
struct pusch_config_common {
  /// PUSCH time domain resource allocations. Size: (0..maxNrofUL-Allocations=16).
  std::vector<pusch_time_domain_resource_allocation> pusch_td_alloc_list;
};

/// Used to configure the common, cell-specific parameters of an UL BWP.
/// \remark See TS 38.331, BWP-UplinkCommon.
struct bwp_uplink_common {
  bwp_configuration             generic_params;
  optional<rach_config_common>  rach_cfg_common;
  optional<pusch_config_common> pusch_cfg_common;
  // TODO: Other fields.
};

/// \brief It provides parameters determining the location and width of the actual carrier.
/// \remark See TS 38.331, "SCS-SpecificCarrier".
struct scs_specific_carrier {
  /// Offset between Point A (lowest subcarrier of common RB 0) and the lowest usable subcarrier in this carrier in
  /// number of PRBs. Values: (0..2199).
  unsigned           offset_to_carrier;
  subcarrier_spacing scs;
  /// Width of this carrier in number of PRBs. Values: (0..MAX_NOF_PRBS).
  unsigned carrier_bandwidth;
};

/// \brief This class provides basic parameters of a downlink carrier and transmission.
/// \remark See TS 38.331, "FrequencyInfoDL-SIB".
struct frequency_info_dl {
  /// Represents the offset to Point A, as defined in TS 38.211, clause 4.4.4.2. Values: (0..2199).
  unsigned offset_to_point_a;
  /// Set of carriers for different subcarrier spacings. The network configures this for all SCSs that are used in
  /// DL BWPs in this serving cell. Size: (1..maxSCSs=5).
  std::vector<scs_specific_carrier> scs_carrier_list;
};

/// \brief Downlink Configuration, common to the serving cell.
/// \remark See TS 38.331, "DownlinkConfigCommonSIB".
struct dl_config_common {
  frequency_info_dl   freq_info_dl;
  bwp_downlink_common init_dl_bwp;
};

struct frequency_info_ul {
  /// Absolute frequency (in ARFCN) of the CRB0.
  unsigned absolute_freq_point_a;
  /// Set of carriers for different subcarrier spacings. The network configures this for all SCSs that are used in
  /// UL BWPs in this serving cell. Size: (1..maxSCSs=5).
  std::vector<scs_specific_carrier> scs_carrier_list;
  bool                              freq_shift_7p5khz_present;
  // TODO: Add other fields.
};

/// \brief Uplink Configuration, common to the serving cell.
/// \remark See TS 38.331, "UplinkConfigCommonSIB".
struct ul_config_common {
  frequency_info_ul freq_info_ul;
  bwp_uplink_common init_ul_bwp;
};

} // namespace srsgnb

#endif // SRSGNB_BWP_CONFIGURATION_H
