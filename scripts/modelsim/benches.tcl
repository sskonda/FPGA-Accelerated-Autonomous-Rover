# Testbench HDL is generated into scratch, keeping the committed additions Tcl.
proc rover::generate_benches {} {
    variable build
    set f [open [file join $build audio_loopback.vhd] w]
    puts $f {library ieee; use ieee.std_logic_1164.all;
library axi_i2s_adi_1_2;
entity audio_loopback is generic(slot_width:integer:=24); end;
architecture test of audio_loopback is
signal clk:std_logic:='0';
signal resetn:std_logic:='0';
signal enable:boolean:=false;
signal bclk,lrclk,channel_sync,frame_sync:std_logic;
signal txdata,rxdata:std_logic_vector(slot_width-1 downto 0):=(others=>'0');
signal serial:std_logic_vector(0 downto 0);
signal txack,rxstb:std_logic;
signal rxack:std_logic:='1';
signal txstb:std_logic:='1';
signal tick:std_logic:='1';
begin
clk<=not clk after 5 ns;
cg:entity axi_i2s_adi_1_2.i2s_clkgen port map(clk,resetn,enable,tick,1,31,bclk,lrclk,channel_sync,frame_sync);
tx:entity axi_i2s_adi_1_2.i2s_tx generic map(slot_width,1)
port map(clk,resetn,enable,bclk,channel_sync,frame_sync,serial,txack,txstb,txdata);
rx:entity axi_i2s_adi_1_2.i2s_rx generic map(slot_width,1)
port map(clk,resetn,enable,bclk,channel_sync,frame_sync,serial,rxstb,rxack,rxdata);
end;}
    close $f
    compile_group rover_benches [list [file join $build audio_loopback.vhd]]
    generate_i2c_bench
    generate_adc_bench
    generate_video_benches
    # VHDL-2008 requires a constrained aggregate for an unconstrained procedure
    # argument. This transient, behavior-preserving TB edit leaves RTL untouched.
    variable root
    set src [file join $root vivado-library-master ip MIPI_CSI_2_RX tb tb_LM.vhd]
    set f [open $src r]; set text [read $f]; close $f
    set text [string map [list {RxDataHS <= (others => '-');} {RxDataHS <= (RxDataHS'range => '-');}] $text]
    set compat [file join $build tb_LM_compat.vhd]
    set f [open $compat w]; puts $f $text; close $f
    puts [tool vcom -2008 -work mipi_csi_2_rx $compat]
}
proc rover::generate_adc_bench {} {
    variable build
    set p [file join $build adc_spi_tb.sv]
    set f [open $p w]
    puts $f {`timescale 1ns/1ps
module adc_spi_tb;
reg clk=0;always #5 clk=~clk;
reg rst=1,sdin0=0,sdin1=0;
wire cs,sclk,drdy;wire[15:0]dout0,dout1;
reg[15:0] word0=0,word1=0;
integer tested=0,bad=0,index=15,finished=0;
integer start_time[0:3],end_time[0:3];
ad1_spi dut(.clk(clk),.rst(rst),.cs(cs),.sdin0(sdin0),.sdin1(sdin1),.sclk(sclk),.drdy(drdy),.dout0(dout0),.dout1(dout1),.led());
always @(negedge cs) if(tested<4) begin
 start_time[tested]=$time; index=15;
 case(tested)
 0:begin word0=0;word1=16'hffff;end
 1:begin word0=16'hffff;word1=0;end
 2:begin word0=16'ha55a;word1=16'h5aa5;end
 3:begin word0=16'h1234;word1=16'habcd;end
 endcase
end
always @(negedge sclk) if(!cs&&index>=0) begin
 sdin0=word0[index];sdin1=word1[index];index=index-1;
end
always @(posedge drdy) if(tested<4) begin
 #1;
 if(dout0!==word0||dout1!==word1) bad=bad+1;
 end_time[tested]=$time;tested=tested+1;
end
initial begin #50;rst=0;wait(tested==4);#100;finished=1;rst=1;end
endmodule}
    close $f
    compile_group rover_adc_benches [list $p]
}
proc rover::generate_i2c_bench {} {
    variable build
    set p [file join $build i2c_master_tb.sv]
    set f [open $p w]
    puts $f {`timescale 1ns/1ps
module i2c_master_tb;
parameter ACK=1, STRETCH=0;
reg clk=0; always #5 clk=~clk;
reg rst=1,stb=0; wire done,error;
wire scl_o,scl_t,sda_o,sda_t;
reg slave_drive=0,stretch=0,active=0;
wire scl=(scl_t ? 1'b1:scl_o)&!stretch;
wire sda=(sda_t ? 1'b1:sda_o)&!slave_drive;
integer bits=0,bytes=0,finished=0,completion_error=0;
reg [7:0] received=0,last_received=0;
wire [7:0] data_out;
TWI_Ctl dut(.MSG_I(1'b0),.STB_I(stb),.STP_I(1'b0),.A_I(8'hA0),.D_I(8'h5A),.D_O(data_out),
.DONE_O(done),.ERR_O(error),.CLK(clk),.SRST(rst),
.s_scl_i(scl),.s_scl_o(scl_o),.s_scl_t(scl_t),.s_sda_i(sda),.s_sda_o(sda_o),.s_sda_t(sda_t));
always @(negedge sda) if(scl===1'b1) begin active=1;bits=0;received=0;end
always @(posedge sda) if(scl===1'b1) active=0;
always @(posedge scl) if(active) begin
 if(bits<8) begin received={received[6:0],sda};bits=bits+1;end
 else begin last_received=received;bytes=bytes+1;bits=0;end
end
always @(negedge scl) slave_drive=active&&bits==8&&ACK;
initial begin #100;rst=0;#100;stb=1;wait(done);completion_error=error;stb=0;#10000;finished=1;end
initial if(STRETCH) begin #40000;stretch=1;#20000;stretch=0;end
endmodule}
    close $f
    compile_group rover_i2c_benches [list $p]
}
proc rover::generate_video_benches {} {
    variable build
    set files {}
    foreach {name module prefix regsetup} {
        gamma hls_gamma_correction s_axi_AXILiteS_ {wr(16,10); wr(24,4); wr(32,8);}
        contrast hls_contrast_stretch s_axi_AXILiteS_ {wr(16,4); wr(24,8); wr(32,0); wr(40,255);}
        saturation hls_saturation_enhance s_axi_AXILiteS_ {wr(16,4); wr(24,8); wr(32,128);}
        scaler video_scaler s_axi_ctrl_ {wr(16,8); wr(24,4); wr(32,8); wr(40,4); wr(0,1);}
    } {
        set template {`timescale 1ns/1ps
module @NAME@_frame_tb;
 parameter STALL=0, GAPS=0, FRAMES=1;
 reg clk=0; always #5 clk=~clk;
 reg resetn=0;
 reg [5:0] awaddr=0,araddr=0;
 reg awvalid=0,wvalid=0,bready=0,arvalid=0,rready=1;
 reg [31:0] wdata=0; reg [3:0] wstrb=15;
 wire awready,wready,bvalid,arready,rvalid;
 wire [1:0] bresp,rresp; wire [31:0] rdata;
 reg [23:0] in_data=0; reg in_user=0,in_last=0,in_valid=0;
 wire in_ready; wire [23:0] out_data; wire out_user,out_last,out_valid;
 wire [2:0] out_keep,out_strb; wire out_id,out_dest;
 reg out_ready=1;
 integer in_count=0,out_count=0,frame_starts=0,line_ends=0,unknown_words=0;
 integer cycle=0,done=0,timed_out=0;
 integer out_time[0:255];
 @MODULE@ dut(
 @INTERRUPT@
 .ap_clk(clk),.ap_rst_n(resetn),
 .@PREFIX@AWADDR(awaddr),.@PREFIX@AWVALID(awvalid),.@PREFIX@AWREADY(awready),
 .@PREFIX@WDATA(wdata),.@PREFIX@WSTRB(wstrb),.@PREFIX@WVALID(wvalid),.@PREFIX@WREADY(wready),
 .@PREFIX@BVALID(bvalid),.@PREFIX@BREADY(bready),.@PREFIX@BRESP(bresp),
 .@PREFIX@ARADDR(araddr),.@PREFIX@ARVALID(arvalid),.@PREFIX@ARREADY(arready),
 .@PREFIX@RDATA(rdata),.@PREFIX@RVALID(rvalid),.@PREFIX@RREADY(rready),.@PREFIX@RRESP(rresp),
 .stream_in_TDATA(in_data),.stream_in_TKEEP(3'b111),.stream_in_TSTRB(3'b111),
 .stream_in_TUSER(in_user),.stream_in_TLAST(in_last),.stream_in_TID(1'b0),.stream_in_TDEST(1'b0),
 .stream_in_TVALID(in_valid),.stream_in_TREADY(in_ready),
 .stream_out_TDATA(out_data),.stream_out_TUSER(out_user),.stream_out_TLAST(out_last),
 .stream_out_TKEEP(out_keep),.stream_out_TSTRB(out_strb),.stream_out_TID(out_id),.stream_out_TDEST(out_dest),
 .stream_out_TVALID(out_valid),.stream_out_TREADY(out_ready)
 );
 task wr(input[5:0] a,input[31:0] d);
 integer wa,wd;
 begin
 @(negedge clk); awaddr=a; wdata=d; awvalid=1; wvalid=1; wa=0;wd=0;
 while(!(wa&&wd)) begin
  @(posedge clk); if(awready&&awvalid) wa=1; if(wready&&wvalid) wd=1;
  @(negedge clk); if(wa) awvalid=0; if(wd) wvalid=0;
 end
 while(!bvalid) @(negedge clk);
 bready=1; @(negedge clk); bready=0;
 end endtask
 function [23:0] color(input integer n);
 case(n%8)
 0:color=24'h000000;1:color=24'hffffff;2:color=24'hff0000;3:color=24'h00ff00;
 4:color=24'h0000ff;5:color=24'h808080;6:color=24'h112233;7:color=24'hada8de;
 endcase endfunction
 always @(negedge clk) begin
  cycle=cycle+1;
  out_ready= !STALL || (cycle%7<4);
 end
 always @(posedge clk) if(resetn) begin
  if(in_valid&&in_ready) in_count=in_count+1;
  if(out_valid&&out_ready) begin
   out_time[out_count]=$time;
   out_count=out_count+1;
   if(out_user) frame_starts=frame_starts+1;
   if(out_last) line_ends=line_ends+1;
   if((^out_data)===1'bx) unknown_words=unknown_words+1;
  end
 end
 integer n;
 initial begin
  repeat(5) @(negedge clk); resetn=1;
  @REGSETUP@
  for(n=0;n<32*FRAMES;n=n+1) begin
   @(negedge clk); in_data=color(n); in_user=(n%32==0); in_last=(n%8==7); in_valid=1;
   @(posedge clk); while(!in_ready) @(posedge clk);
   @(negedge clk); in_valid=0;
   if(GAPS) repeat(2) @(negedge clk);
  end
  wait(out_count>=32); repeat(10) @(negedge clk); done=1;
 end
 initial begin #99000; if(!done) timed_out=1; end
endmodule}
        set interrupt {}
        if {$name eq "scaler"} {set interrupt {.interrupt(),}}
        set text [string map [list @NAME@ $name @MODULE@ $module @PREFIX@ $prefix @REGSETUP@ $regsetup @INTERRUPT@ $interrupt] $template]
        set p [file join $build ${name}_frame_tb.sv]
        set f [open $p w]; puts $f $text; close $f
        lappend files $p
    }
    compile_group rover_video_benches $files
}
