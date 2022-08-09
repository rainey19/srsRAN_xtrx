/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "pdsch_default_time_allocation.h"

using namespace srsgnb;

static pdsch_default_time_allocation_config
pdsch_default_time_allocation_default_A_get_normal(unsigned row_index, dmrs_typeA_position dmrs_pos)
{
  // TS38.214 Table 5.1.2.1.1-2. Default PDSCH time domain resource allocation A for normal CP.
  static const std::array<pdsch_default_time_allocation_config, 16> TABLE = {{{pdsch_mapping_type::typeA, 0, 2, 12},
                                                                              {pdsch_mapping_type::typeA, 0, 2, 10},
                                                                              {pdsch_mapping_type::typeA, 0, 2, 9},
                                                                              {pdsch_mapping_type::typeA, 0, 2, 7},
                                                                              {pdsch_mapping_type::typeA, 0, 2, 5},
                                                                              {pdsch_mapping_type::typeB, 0, 9, 4},
                                                                              {pdsch_mapping_type::typeB, 0, 4, 4},
                                                                              {pdsch_mapping_type::typeB, 0, 5, 7},
                                                                              {pdsch_mapping_type::typeB, 0, 5, 2},
                                                                              {pdsch_mapping_type::typeB, 0, 9, 2},
                                                                              {pdsch_mapping_type::typeB, 0, 12, 2},
                                                                              {pdsch_mapping_type::typeA, 0, 1, 13},
                                                                              {pdsch_mapping_type::typeA, 0, 1, 6},
                                                                              {pdsch_mapping_type::typeA, 0, 2, 4},
                                                                              {pdsch_mapping_type::typeB, 0, 4, 7},
                                                                              {pdsch_mapping_type::typeB, 0, 8, 4}}};

  if (row_index >= TABLE.size()) {
    return PDSCH_DEFAULT_TIME_ALLOCATION_RESERVED;
  }

  pdsch_default_time_allocation_config result = TABLE[row_index];
  if (dmrs_pos == dmrs_typeA_position::pos3) {
    if (row_index < 5) {
      ++result.start_symbol;
      --result.duration;
    } else if (row_index == 5) {
      ++result.start_symbol;
    } else if (row_index == 6) {
      result.start_symbol += 2;
    }
  }

  return result;
}

static pdsch_default_time_allocation_config
pdsch_default_time_allocation_default_A_get_extended(unsigned row_index, dmrs_typeA_position dmrs_pos)
{
  // TS38.214 Table 5.1.2.1.1-2. Default PDSCH time domain resource allocation A for normal CP.
  static const std::array<pdsch_default_time_allocation_config, 16> TABLE = {{{pdsch_mapping_type::typeA, 0, 2, 6},
                                                                              {pdsch_mapping_type::typeA, 0, 2, 10},
                                                                              {pdsch_mapping_type::typeA, 0, 2, 9},
                                                                              {pdsch_mapping_type::typeA, 0, 2, 7},
                                                                              {pdsch_mapping_type::typeA, 0, 2, 5},
                                                                              {pdsch_mapping_type::typeB, 0, 6, 4},
                                                                              {pdsch_mapping_type::typeB, 0, 4, 4},
                                                                              {pdsch_mapping_type::typeB, 0, 5, 6},
                                                                              {pdsch_mapping_type::typeB, 0, 5, 2},
                                                                              {pdsch_mapping_type::typeB, 0, 9, 2},
                                                                              {pdsch_mapping_type::typeB, 0, 10, 2},
                                                                              {pdsch_mapping_type::typeA, 0, 1, 11},
                                                                              {pdsch_mapping_type::typeA, 0, 1, 6},
                                                                              {pdsch_mapping_type::typeA, 0, 2, 4},
                                                                              {pdsch_mapping_type::typeB, 0, 4, 6},
                                                                              {pdsch_mapping_type::typeB, 0, 8, 4}}};

  if (row_index >= TABLE.size()) {
    return PDSCH_DEFAULT_TIME_ALLOCATION_RESERVED;
  }

  pdsch_default_time_allocation_config result = TABLE[row_index];
  if (dmrs_pos == dmrs_typeA_position::pos3) {
    if (row_index < 5) {
      ++result.start_symbol;
      --result.duration;
    } else if (row_index == 5) {
      result.start_symbol += 2;
      result.duration -= 2;
    } else if (row_index == 6) {
      result.start_symbol += 2;
    }
  }

  return result;
}

pdsch_default_time_allocation_config
srsgnb::pdsch_default_time_allocation_default_A_get(cyclic_prefix cp, unsigned row_index, dmrs_typeA_position dmrs_pos)
{
  switch (cp) {
    case cyclic_prefix::NORMAL:
      return pdsch_default_time_allocation_default_A_get_normal(row_index, dmrs_pos);
    case cyclic_prefix::EXTENDED:
    default:
      return pdsch_default_time_allocation_default_A_get_extended(row_index, dmrs_pos);
  }
}

span<const pdsch_time_domain_resource_allocation>
srsgnb::pdsch_default_time_allocations_default_A_table(cyclic_prefix cp, dmrs_typeA_position dmrs_pos)
{
  // Build PDSCH-TimeDomain tables statically.
  auto table_builder = [](cyclic_prefix cp, dmrs_typeA_position dmrs_pos) {
    std::array<pdsch_time_domain_resource_allocation, 16> table;
    for (unsigned i = 0; i < 16; ++i) {
      pdsch_default_time_allocation_config cfg = pdsch_default_time_allocation_default_A_get(cp, i, dmrs_pos);
      table[i].k0                              = cfg.pdcch_to_pdsch_delay;
      table[i].map_type                        = cfg.mapping_type;
      table[i].symbols                         = {cfg.start_symbol, cfg.duration};
    }
    return table;
  };
  static const std::array<std::array<pdsch_time_domain_resource_allocation, 16>, 4> tables = {
      table_builder(cyclic_prefix::NORMAL, dmrs_typeA_position::pos2),
      table_builder(cyclic_prefix::NORMAL, dmrs_typeA_position::pos3),
      table_builder(cyclic_prefix::EXTENDED, dmrs_typeA_position::pos2),
      table_builder(cyclic_prefix::EXTENDED, dmrs_typeA_position::pos3)};

  // Retrieve respective table.
  return tables[static_cast<unsigned>(cp.value) * 2 + (static_cast<unsigned>(dmrs_pos) - 2)];
}
