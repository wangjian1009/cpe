#include "assert.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_math.h"
#include "cpe/utils/traffic_bps.h"

void cpe_traffic_bps_init(cpe_traffic_bps_t traffic) {
    traffic->m_total_bytes = 0;
	traffic->m_begin_s = 0;
    traffic->m_last_s = 0;
}

void cpe_traffic_bps_add_flow(cpe_traffic_bps_t traffic, uint64_t bytes, time_t cur_s) {
	if (traffic->m_last_s == 0/* first insert */
		|| (cur_s + 1) < traffic->m_last_s
		|| (cur_s + 1 - traffic->m_last_s) > CPE_ARRAY_SIZE(traffic->m_bytes)/* all data expired */)
	{
		bzero(traffic->m_bytes, sizeof(traffic->m_bytes));
		traffic->m_begin_s = cur_s;
        traffic->m_last_s = cur_s + 1;
	}

	while(traffic->m_last_s < cur_s + 1) {
        traffic->m_bytes[traffic->m_last_s % CPE_ARRAY_SIZE(traffic->m_bytes)] = 0;
		traffic->m_last_s++;
	}

    time_t window_begin_s = traffic->m_last_s - CPE_ARRAY_SIZE(traffic->m_bytes);
    if (window_begin_s > traffic->m_begin_s) traffic->m_begin_s = window_begin_s;
        
    traffic->m_total_bytes += bytes;
    traffic->m_bytes[cur_s % CPE_ARRAY_SIZE(traffic->m_bytes)] += bytes;
}

uint32_t cpe_traffic_bps_calc_Bps(cpe_traffic_bps_t traffic) {
    time_t cur_s = time(0);
    time_t window_end_s = cur_s + 1;
    time_t window_begin_s = window_end_s - CPE_ARRAY_SIZE(traffic->m_bytes);
    
    if (window_begin_s < traffic->m_begin_s) window_begin_s = traffic->m_begin_s;
    if (window_end_s > traffic->m_last_s) window_end_s = traffic->m_last_s;

    if (window_end_s <= window_begin_s) return 0;
    
    uint8_t window_size = window_end_s - window_begin_s;
    assert(window_size > 0);
    
	uint64_t window_bytes = 0;
    time_t get_s;
    for(get_s = window_begin_s; get_s < window_end_s; ++get_s) {
		window_bytes += traffic->m_bytes[get_s % CPE_ARRAY_SIZE(traffic->m_bytes)];
	}

    return (uint32_t)(window_bytes / window_size);
}
