#include <cmath>
#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "GaussianFilter.h"

GaussianFilter::GaussianFilter( sc_module_name n ): sc_module( n )
{
/* #ifndef NATIVE_SYSTEMC
	HLS_MAP_TO_REG_BANK(rgb);
#endif */
	SC_THREAD( do_filter );
	sensitive << i_clk.pos();
	dont_initialize();
	reset_signal_is(i_rst, false);
        
#ifndef NATIVE_SYSTEMC
	for(int i = 0; i < 9; i++){
		i_rgb[i].clk_rst(i_clk, i_rst);
	}
	o_result.clk_rst(i_clk, i_rst);
#endif
}

GaussianFilter::~GaussianFilter() {}

// Gaussian mask
const int mask[MASK_X][MASK_Y] = {{1,2,1},{2,4,2},{1,2,1}};

void GaussianFilter::do_filter() {
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		for(int i = 0; i < 9; i++){
			i_rgb[i].reset();
		}
		o_result.reset();
#endif
		wait();
	}
	while (true) {
		{
			HLS_CONSTRAIN_LATENCY(0, 1, "lat00");
			r_val = 0;
			g_val = 0;
			b_val = 0;
		}
		sc_dt::sc_uint<24> rgb[9];
		for (unsigned int v = 0; v<MASK_Y; ++v) {
			for (unsigned int u = 0; u<MASK_X; ++u) {
				HLS_CONSTRAIN_LATENCY(0, 9, "lat00");
				#ifndef NATIVE_SYSTEMC
					{
						HLS_DEFINE_PROTOCOL("input");
						rgb[(3*v)+u] = i_rgb[(3*v)+u].get();
						wait();
					}
				#else
					rgb[(3*v)+u] = i_rgb[(3*v)+u].read();
				#endif
				r_val += rgb[(3*v)+u].range(7,0)   * mask[u][v];
      			g_val += rgb[(3*v)+u].range(15,8)  * mask[u][v];
      			b_val += rgb[(3*v)+u].range(23,16) * mask[u][v];
			}
		}
		sc_dt::sc_uint<24> o_rgb;
		{
			HLS_CONSTRAIN_LATENCY(0, 2, "lat_01");
			unsigned int r_total = r_val >> 4;
			unsigned int g_total = g_val >> 4;
			unsigned int b_total = b_val >> 4;
			o_rgb.range(7,0)   = r_total;
			o_rgb.range(15,8)  = g_total;
			o_rgb.range(23,16) = b_total;
		}
#ifndef NATIVE_SYSTEMC
		{
			HLS_DEFINE_PROTOCOL("output");
			o_result.put(o_rgb);
			wait();
		}
#else
		o_result.write(o_rgb);
#endif
	}
}
