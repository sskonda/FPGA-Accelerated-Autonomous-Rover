#include "xclk_wiz.h"

XClk_Wiz_Config XClk_Wiz_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,clk-wiz-6.0", /* compatible */
		0x43c00000, /* reg */
		0x0, /* xlnx,enable-clock-monitor */
		0x0, /* xlnx,enable-user-clock0 */
		0x0, /* xlnx,enable-user-clock1 */
		0x0, /* xlnx,enable-user-clock2 */
		0x0, /* xlnx,enable-user-clock3 */
		0x64, /* xlnx,ref-clk-freq */
		0x64, /* xlnx,user-clk-freq0 */
		0x64, /* xlnx,user-clk-freq1 */
		0x64, /* xlnx,user-clk-freq2 */
		0x64, /* xlnx,user-clk-freq3 */
		0x1, /* xlnx,precision */
		0x0, /* xlnx,enable-pll0 */
		0x0, /* xlnx,enable-pll1 */
		0x64, /* xlnx,prim-in-freq */
		0x1, /* xlnx,num-out-clks */
		0xffff, /* interrupts */
		0xffff /* interrupt-parent */
	},
	 {
		 NULL
	}
};