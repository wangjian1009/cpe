#ifndef CPE_UTILS_TRAFFIC_BPS_H_INCLEDED
#define CPE_UTILS_TRAFFIC_BPS_H_INCLEDED
#include "utils_types.h"

struct cpe_traffic_bps {
    uint64_t m_total_bytes;
    uint64_t m_bytes[9];
	time_t m_begin_s;
	time_t m_last_s;
};

void cpe_traffic_bps_init(cpe_traffic_bps_t traffic);
void cpe_traffic_bps_add_flow(cpe_traffic_bps_t traffic, uint64_t bytes, time_t ct);
uint32_t cpe_traffic_bps_calc_Bps(cpe_traffic_bps_t traffic);

#endif


