# 0 "C:\\Users\\sanat\\FPGA_Car_Project\\Vitis_Vid_Pass\\vitis_vid_pass\\hw\\sdt\\system-top.dts"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "C:\\Users\\sanat\\FPGA_Car_Project\\Vitis_Vid_Pass\\vitis_vid_pass\\hw\\sdt\\system-top.dts"
/dts-v1/;
# 1 "C:\\Users\\sanat\\FPGA_Car_Project\\Vitis_Vid_Pass\\vitis_vid_pass\\hw\\sdt\\zynq-7000.dtsi" 1
# 10 "C:\\Users\\sanat\\FPGA_Car_Project\\Vitis_Vid_Pass\\vitis_vid_pass\\hw\\sdt\\zynq-7000.dtsi"
/ {
 #address-cells = <1>;
 #size-cells = <1>;
 compatible = "xlnx,zynq-7000";
 model = "Xilinx Zynq";

 options {
  u-boot {
   compatible = "u-boot,config";
   bootscr-address = /bits/ 64 <0x3000000>;
  };
 };

 cpus_a9: cpus-a9@0 {
  #address-cells = <1>;
  #size-cells = <0>;

  ps7_cortexa9_0: cpu@0 {
   compatible = "arm,cortex-a9";
   device_type = "cpu";
   reg = <0>;
   clocks = <&clkc 3>;
   clock-latency = <1000>;
   cpu0-supply = <&regulator_vccpint>;
   operating-points = <
    666667 1000000
    333334 1000000
   >;
  };

  ps7_cortexa9_1: cpu@1 {
   compatible = "arm,cortex-a9";
   device_type = "cpu";
   reg = <1>;
   clocks = <&clkc 3>;
  };
 };

 fpga_full: fpga-region {
  compatible = "fpga-region";
  fpga-mgr = <&devcfg>;
  #address-cells = <1>;
  #size-cells = <1>;
  ranges;
 };

 pmu@f8891000 {
  compatible = "arm,cortex-a9-pmu";
  interrupts = <0 5 4>, <0 6 4>;
  interrupt-parent = <&intc>;
  reg = <0xf8891000 0x1000>,
    <0xf8893000 0x1000>;
 };

 regulator_vccpint: fixedregulator {
  compatible = "regulator-fixed";
  regulator-name = "VCCPINT";
  regulator-min-microvolt = <1000000>;
  regulator-max-microvolt = <1000000>;
  regulator-boot-on;
  regulator-always-on;
 };

 replicator {
  compatible = "arm,coresight-static-replicator";
  clocks = <&clkc 27>, <&clkc 46>, <&clkc 47>;
  clock-names = "apb_pclk", "dbg_trc", "dbg_apb";

  out-ports {
   #address-cells = <1>;
   #size-cells = <0>;


   port@0 {
    reg = <0>;
    replicator_out_port0: endpoint {
     remote-endpoint = <&tpiu_in_port>;
    };
   };
   port@1 {
    reg = <1>;
    replicator_out_port1: endpoint {
     remote-endpoint = <&etb_in_port>;
    };
   };
  };
  in-ports {

   port {
    replicator_in_port0: endpoint {
     remote-endpoint = <&funnel_out_port>;
    };
   };
  };
 };

 amba: axi {
  bootph-all;
  compatible = "simple-bus";
  #address-cells = <1>;
  #size-cells = <1>;
  interrupt-parent = <&intc>;
  ranges;

  adc: adc@f8007100 {
   compatible = "xlnx,zynq-xadc-1.00.a";
   reg = <0xf8007100 0x20>;
   interrupts = <0 7 4>;
   interrupt-parent = <&intc>;
   clocks = <&clkc 12>;
  };

  can0: can@e0008000 {
   compatible = "xlnx,zynq-can-1.0";
   status = "disabled";
   clocks = <&clkc 19>, <&clkc 36>;
   clock-names = "can_clk", "pclk";
   reg = <0xe0008000 0x1000>;
   interrupts = <0 28 4>;
   interrupt-parent = <&intc>;
   tx-fifo-depth = <0x40>;
   rx-fifo-depth = <0x40>;
  };

  can1: can@e0009000 {
   compatible = "xlnx,zynq-can-1.0";
   status = "disabled";
   clocks = <&clkc 20>, <&clkc 37>;
   clock-names = "can_clk", "pclk";
   reg = <0xe0009000 0x1000>;
   interrupts = <0 51 4>;
   interrupt-parent = <&intc>;
   tx-fifo-depth = <0x40>;
   rx-fifo-depth = <0x40>;
  };

  gpio0: gpio@e000a000 {
   compatible = "xlnx,zynq-gpio-1.0";
   #gpio-cells = <2>;
   clocks = <&clkc 42>;
   gpio-controller;
   interrupt-controller;
   #interrupt-cells = <2>;
   interrupt-parent = <&intc>;
   interrupts = <0 20 4>;
   reg = <0xe000a000 0x1000>;
  };

  i2c0: i2c@e0004000 {
   compatible = "cdns,i2c-r1p10";
   status = "disabled";
   clocks = <&clkc 38>;
   interrupt-parent = <&intc>;
   interrupts = <0 25 4>;
   clock-frequency = <400000>;
   reg = <0xe0004000 0x1000>;
   #address-cells = <1>;
   #size-cells = <0>;
  };

  i2c1: i2c@e0005000 {
   compatible = "cdns,i2c-r1p10";
   status = "disabled";
   clocks = <&clkc 39>;
   interrupt-parent = <&intc>;
   interrupts = <0 48 4>;
   clock-frequency = <400000>;
   reg = <0xe0005000 0x1000>;
   #address-cells = <1>;
   #size-cells = <0>;
  };

  intc: interrupt-controller@f8f01000 {
   compatible = "arm,cortex-a9-gic";
   #interrupt-cells = <3>;
   interrupt-controller;
   reg = <0xF8F01000 0x1000>,
         <0xF8F00100 0x100>;
  };

  L2: cache-controller@f8f02000 {
   compatible = "arm,pl310-cache";
   reg = <0xF8F02000 0x1000>;
   interrupts = <0 2 4>;
   arm,data-latency = <3 2 2>;
   arm,tag-latency = <2 2 2>;
   cache-unified;
   cache-level = <2>;
  };

  mc: memory-controller@f8006000 {
   compatible = "xlnx,zynq-ddrc-a05";
   reg = <0xf8006000 0x1000>;
  };

  ocm: sram@fffc0000 {
   compatible = "mmio-sram";
   reg = <0xfffc0000 0x10000>;
   #address-cells = <1>;
   #size-cells = <1>;
   ranges = <0 0xfffc0000 0x10000>;
   ocm-sram@0 {
    reg = <0x0 0x10000>;
   };
  };

  uart0: serial@e0000000 {
   compatible = "xlnx,xuartps", "cdns,uart-r1p8";
   status = "disabled";
   clocks = <&clkc 23>, <&clkc 40>;
   clock-names = "uart_clk", "pclk";
   reg = <0xE0000000 0x1000>;
   interrupts = <0 27 4>;
   interrupt-parent = <&intc>;
  };

  uart1: serial@e0001000 {
   compatible = "xlnx,xuartps", "cdns,uart-r1p8";
   status = "disabled";
   clocks = <&clkc 24>, <&clkc 41>;
   clock-names = "uart_clk", "pclk";
   reg = <0xE0001000 0x1000>;
   interrupts = <0 50 4>;
   interrupt-parent = <&intc>;
  };

  spi0: spi@e0006000 {
   compatible = "xlnx,zynq-spi-r1p6";
   reg = <0xe0006000 0x1000>;
   status = "disabled";
   interrupt-parent = <&intc>;
   interrupts = <0 26 4>;
   clocks = <&clkc 25>, <&clkc 34>;
   clock-names = "ref_clk", "pclk";
   #address-cells = <1>;
   #size-cells = <0>;
  };

  spi1: spi@e0007000 {
   compatible = "xlnx,zynq-spi-r1p6";
   reg = <0xe0007000 0x1000>;
   status = "disabled";
   interrupt-parent = <&intc>;
   interrupts = <0 49 4>;
   clocks = <&clkc 26>, <&clkc 35>;
   clock-names = "ref_clk", "pclk";
   #address-cells = <1>;
   #size-cells = <0>;
  };

  qspi: spi@e000d000 {
   compatible = "xlnx,zynq-qspi-1.0";
   reg = <0xe000d000 0x1000>;
   interrupt-parent = <&intc>;
   interrupts = <0 19 4>;
   clocks = <&clkc 10>, <&clkc 43>;
   clock-names = "ref_clk", "pclk";
   status = "disabled";
   #address-cells = <1>;
   #size-cells = <0>;
  };

  gem0: ethernet@e000b000 {
   compatible = "xlnx,zynq-gem", "cdns,gem";
   reg = <0xe000b000 0x1000>;
   status = "disabled";
   interrupts = <0 22 4>;
   interrupt-parent = <&intc>;
   clocks = <&clkc 30>, <&clkc 30>, <&clkc 13>;
   clock-names = "pclk", "hclk", "tx_clk";
   #address-cells = <1>;
   #size-cells = <0>;
  };

  gem1: ethernet@e000c000 {
   compatible = "xlnx,zynq-gem", "cdns,gem";
   reg = <0xe000c000 0x1000>;
   status = "disabled";
   interrupts = <0 45 4>;
   interrupt-parent = <&intc>;
   clocks = <&clkc 31>, <&clkc 31>, <&clkc 14>;
   clock-names = "pclk", "hclk", "tx_clk";
   #address-cells = <1>;
   #size-cells = <0>;
  };

  smcc: memory-controller@e000e000 {
   compatible = "arm,pl353-smc-r2p1", "arm,primecell";
   reg = <0xe000e000 0x0001000>;
   status = "disabled";
   clock-names = "memclk", "apb_pclk";
   clocks = <&clkc 11>, <&clkc 44>;
   ranges = <0x0 0x0 0xe1000000 0x1000000
      0x1 0x0 0xe2000000 0x2000000
      0x2 0x0 0xe4000000 0x2000000>;
   #address-cells = <2>;
   #size-cells = <1>;
   interrupt-parent = <&intc>;
   interrupts = <0 18 4>;
   nfc0: nand-controller@0,0 {
    compatible = "arm,pl353-nand-r2p1";
    reg = <0 0 0x1000000>;
    status = "disabled";
   };
   nor0: flash@1,0 {
    status = "disabled";
    compatible = "cfi-flash";
    reg = <1 0 0x2000000>;
   };
  };

  sdhci0: mmc@e0100000 {
   compatible = "arasan,sdhci-8.9a";
   status = "disabled";
   clock-names = "clk_xin", "clk_ahb";
   clocks = <&clkc 21>, <&clkc 32>;
   interrupt-parent = <&intc>;
   interrupts = <0 24 4>;
   reg = <0xe0100000 0x1000>;
  };

  sdhci1: mmc@e0101000 {
   compatible = "arasan,sdhci-8.9a";
   status = "disabled";
   clock-names = "clk_xin", "clk_ahb";
   clocks = <&clkc 22>, <&clkc 33>;
   interrupt-parent = <&intc>;
   interrupts = <0 47 4>;
   reg = <0xe0101000 0x1000>;
  };

  slcr: slcr@f8000000 {
   bootph-all;
   #address-cells = <1>;
   #size-cells = <1>;
   compatible = "xlnx,zynq-slcr", "syscon", "simple-mfd";
   reg = <0xF8000000 0x1000>;
   ranges;
   clkc: clkc@100 {
    bootph-all;
    #clock-cells = <1>;
    compatible = "xlnx,ps7-clkc";
    fclk-enable = <0xf>;
    clock-output-names = "armpll", "ddrpll", "iopll", "cpu_6or4x",
      "cpu_3or2x", "cpu_2x", "cpu_1x", "ddr2x", "ddr3x",
      "dci", "lqspi", "smc", "pcap", "gem0", "gem1",
      "fclk0", "fclk1", "fclk2", "fclk3", "can0", "can1",
      "sdio0", "sdio1", "uart0", "uart1", "spi0", "spi1",
      "dma", "usb0_aper", "usb1_aper", "gem0_aper",
      "gem1_aper", "sdio0_aper", "sdio1_aper",
      "spi0_aper", "spi1_aper", "can0_aper", "can1_aper",
      "i2c0_aper", "i2c1_aper", "uart0_aper", "uart1_aper",
      "gpio_aper", "lqspi_aper", "smc_aper", "swdt",
      "dbg_trc", "dbg_apb";
    reg = <0x100 0x100>;
   };

   rstc: rstc@200 {
    compatible = "xlnx,zynq-reset";
    reg = <0x200 0x48>;
    #reset-cells = <1>;
    syscon = <&slcr>;
   };

   pinctrl0: pinctrl@700 {
    compatible = "xlnx,pinctrl-zynq";
    reg = <0x700 0x200>;
    syscon = <&slcr>;
   };
  };

  dmac_s: dma-controller@f8003000 {
   compatible = "arm,pl330", "arm,primecell";
   reg = <0xf8003000 0x1000>;
   interrupt-parent = <&intc>;




   interrupts = <0 13 4>,
                <0 14 4>, <0 15 4>,
                <0 16 4>, <0 17 4>,
                <0 40 4>, <0 41 4>,
                <0 42 4>, <0 43 4>;
   #dma-cells = <1>;
   clocks = <&clkc 27>;
   clock-names = "apb_pclk";
  };

  devcfg: devcfg@f8007000 {
   compatible = "xlnx,zynq-devcfg-1.0";
   reg = <0xf8007000 0x100>;
   interrupt-parent = <&intc>;
   interrupts = <0 8 4>;
   clocks = <&clkc 12>, <&clkc 15>, <&clkc 16>, <&clkc 17>, <&clkc 18>;
   clock-names = "ref_clk", "fclk0", "fclk1", "fclk2", "fclk3";
   syscon = <&slcr>;
  };

  efuse: efuse@f800d000 {
   compatible = "xlnx,zynq-efuse";
   reg = <0xf800d000 0x20>;
  };

  global_timer: timer@f8f00200 {
   compatible = "arm,cortex-a9-global-timer";
   reg = <0xf8f00200 0x20>;
   interrupts = <1 11 0x301>;
   interrupt-parent = <&intc>;
   clocks = <&clkc 4>;
  };

  ttc0: timer@f8001000 {
   interrupt-parent = <&intc>;
   interrupts = <0 10 4>, <0 11 4>, <0 12 4>;
   compatible = "cdns,ttc";
   clocks = <&clkc 6>;
   reg = <0xF8001000 0x1000>;
  };

  ttc1: timer@f8002000 {
   interrupt-parent = <&intc>;
   interrupts = <0 37 4>, <0 38 4>, <0 39 4>;
   compatible = "cdns,ttc";
   clocks = <&clkc 6>;
   reg = <0xF8002000 0x1000>;
  };

  scutimer: timer@f8f00600 {
   bootph-all;
   interrupt-parent = <&intc>;
   interrupts = <1 13 0x301>;
   compatible = "arm,cortex-a9-twd-timer";
   reg = <0xf8f00600 0x20>;
   clocks = <&clkc 4>;
  };

  scuwdt: scuwatchdog@f8f00620 {
   interrupt-parent = <&intc>;
   interrupts = <1 14 4>;
   compatible = "xlnx,ps7-scuwdt-1.00.a";
   reg = <0xf8f00620 0xe0>;
  };

  usb0: usb@e0002000 {
   compatible = "xlnx,zynq-usb-2.20a", "chipidea,usb2";
   status = "disabled";
   clocks = <&clkc 28>;
   interrupt-parent = <&intc>;
   interrupts = <0 21 4>;
   reg = <0xe0002000 0x1000>;
   phy_type = "ulpi";
  };

  usb1: usb@e0003000 {
   compatible = "xlnx,zynq-usb-2.20a", "chipidea,usb2";
   status = "disabled";
   clocks = <&clkc 29>;
   interrupt-parent = <&intc>;
   interrupts = <0 44 4>;
   reg = <0xe0003000 0x1000>;
   phy_type = "ulpi";
  };

  watchdog0: watchdog@f8005000 {
   clocks = <&clkc 45>;
   compatible = "cdns,wdt-r1p2";
   interrupt-parent = <&intc>;
   interrupts = <0 9 1>;
   reg = <0xf8005000 0x1000>;
   timeout-sec = <10>;
  };

  coresight: coresight@f8800000 {
   compatible = "xlnx,ps7-coresight-comp-1.00.a";
   status = "disabled";
   reg = <0xf8800000 0x100000>;
  };

  etb@f8801000 {
   compatible = "arm,coresight-etb10", "arm,primecell";
   reg = <0xf8801000 0x1000>;
   clocks = <&clkc 27>, <&clkc 46>, <&clkc 47>;
   clock-names = "apb_pclk", "dbg_trc", "dbg_apb";
   in-ports {
    port {
     etb_in_port: endpoint {
      remote-endpoint = <&replicator_out_port1>;
     };
    };
   };
  };

  tpiu@f8803000 {
   compatible = "arm,coresight-tpiu", "arm,primecell";
   reg = <0xf8803000 0x1000>;
   clocks = <&clkc 27>, <&clkc 46>, <&clkc 47>;
   clock-names = "apb_pclk", "dbg_trc", "dbg_apb";
   in-ports {
    port {
     tpiu_in_port: endpoint {
      remote-endpoint = <&replicator_out_port0>;
     };
    };
   };
  };

  funnel@f8804000 {
   compatible = "arm,coresight-static-funnel", "arm,primecell";
   reg = <0xf8804000 0x1000>;
   clocks = <&clkc 27>, <&clkc 46>, <&clkc 47>;
   clock-names = "apb_pclk", "dbg_trc", "dbg_apb";


   out-ports {
    port {
     funnel_out_port: endpoint {
      remote-endpoint =
       <&replicator_in_port0>;
     };
    };
   };

   in-ports {
    #address-cells = <1>;
    #size-cells = <0>;


    port@0 {
     reg = <0>;
     funnel0_in_port0: endpoint {
      remote-endpoint = <&ptm0_out_port>;
     };
    };

    port@1 {
     reg = <1>;
     funnel0_in_port1: endpoint {
      remote-endpoint = <&ptm1_out_port>;
     };
    };

    port@2 {
     reg = <2>;
    };

   };
  };

  ptm@f889c000 {
   compatible = "arm,coresight-etm3x", "arm,primecell";
   reg = <0xf889c000 0x1000>;
   clocks = <&clkc 27>, <&clkc 46>, <&clkc 47>;
   clock-names = "apb_pclk", "dbg_trc", "dbg_apb";
   cpu = <&ps7_cortexa9_0>;
   out-ports {
    port {
     ptm0_out_port: endpoint {
      remote-endpoint = <&funnel0_in_port0>;
     };
    };
   };
  };

  ptm@f889d000 {
   compatible = "arm,coresight-etm3x", "arm,primecell";
   reg = <0xf889d000 0x1000>;
   clocks = <&clkc 27>, <&clkc 46>, <&clkc 47>;
   clock-names = "apb_pclk", "dbg_trc", "dbg_apb";
   cpu = <&ps7_cortexa9_1>;
   out-ports {
    port {
     ptm1_out_port: endpoint {
      remote-endpoint = <&funnel0_in_port1>;
     };
    };
   };
  };
 };
};
# 3 "C:\\Users\\sanat\\FPGA_Car_Project\\Vitis_Vid_Pass\\vitis_vid_pass\\hw\\sdt\\system-top.dts" 2
# 1 "C:\\Users\\sanat\\FPGA_Car_Project\\Vitis_Vid_Pass\\vitis_vid_pass\\hw\\sdt\\pl.dtsi" 1
/ {
 amba_pl: amba_pl {
  ranges;
  compatible = "simple-bus";
  #address-cells = <1>;
  #size-cells = <1>;
  firmware-name = "video_passthrough.bit.bin";
  clocking0: clocking0 {
   compatible = "xlnx,fclk";
   assigned-clocks = <&clkc 15>;
   assigned-clock-rates = <100000000>;
   #clock-cells = <0>;
   clock-output-names = "fabric_clk";
   clocks = <&clkc 15>;
  };
  afi2: afi2@f800a000 {
   status = "okay";
   compatible = "xlnx,afi-fpga";
   #address-cells = <1>;
   reg = <0xF800A000 0x1000>;
   #size-cells = <0>;
   xlnx,afi-width = <0>;
  };
  afi0: afi0@f8008000 {
   status = "okay";
   compatible = "xlnx,afi-fpga";
   #address-cells = <1>;
   reg = <0xF8008000 0x1000>;
   #size-cells = <0>;
   xlnx,afi-width = <0>;
  };
  misc_clk_0: misc_clk_0 {
   compatible = "fixed-clock";
   clock-frequency = <50000000>;
   #clock-cells = <0>;
  };
  misc_clk_4: misc_clk_4 {
   compatible = "fixed-clock";
   clock-frequency = <74250000>;
   #clock-cells = <0>;
  };
  misc_clk_1: misc_clk_1 {
   compatible = "fixed-clock";
   clock-frequency = <150000000>;
   #clock-cells = <0>;
  };
  misc_clk_3: misc_clk_3 {
   compatible = "fixed-clock";
   clock-frequency = <200000000>;
   #clock-cells = <0>;
  };
  misc_clk_2: misc_clk_2 {
   compatible = "fixed-clock";
   clock-frequency = <84000000>;
   #clock-cells = <0>;
  };
  AXI_GammaCorrection_0: AXI_GammaCorrection@43c40000 {
   compatible = "xlnx,AXI-GammaCorrection-1.0";
   xlnx,kinputcolorwidth = <10>;
   xlnx,rable = <0>;
   xlnx,ip-name = "AXI_GammaCorrection";
   reg = <0x43c40000 0x10000>;
   clocks = <&misc_clk_0>, <&misc_clk_1>;
   xlnx,edk-iptype = "PERIPHERAL";
   xlnx,kaxi-inputdatawidth = <32>;
   xlnx,kaxi-outputdatawidth = <24>;
   status = "okay";
   clock-names = "AxiLiteClk" , "StreamClk";
   xlnx,name = "AXI_GammaCorrection_0";
  };
  MIPI_CSI_2_RX_0: MIPI_CSI_2_RX@43c30000 {
   compatible = "xlnx,MIPI-CSI-2-RX-1.2";
   xlnx,kgenerateaxil;
   xlnx,rable = <0>;
   xlnx,kversionminor = <2>;
   xlnx,ip-name = "MIPI_CSI_2_RX";
   reg = <0x43c30000 0x10000>;
   clocks = <&misc_clk_2>, <&misc_clk_0>, <&misc_clk_1>;
   xlnx,ktargetdt = "RAW10";
   xlnx,kversionmajor = <1>;
   xlnx,edk-iptype = "PERIPHERAL";
   xlnx,klanecount = <2>;
   status = "okay";
   clock-names = "RxByteClkHS" , "s_axi_lite_aclk" , "video_aclk";
   xlnx,m-max-samples-per-clock = <4>;
   xlnx,name = "MIPI_CSI_2_RX_0";
  };
  MIPI_D_PHY_RX_0: MIPI_D_PHY_RX@43c20000 {
   compatible = "xlnx,MIPI-D-PHY-RX-1.3";
   xlnx,knoofdatalanes = <2>;
   xlnx,krefclkfreqhz = <200000000>;
   xlnx,kgenerateaxil;
   xlnx,rable = <0>;
   xlnx,kversionminor = <3>;
   xlnx,kadddelaydata1-ps = <0>;
   xlnx,ip-name = "MIPI_D_PHY_RX";
   reg = <0x43c20000 0x10000>;
   clocks = <&misc_clk_3>, <&misc_clk_0>;
   xlnx,kversionmajor = <1>;
   xlnx,edk-iptype = "PERIPHERAL";
   xlnx,kadddelayclk-ps = <0>;
   xlnx,kadddelaydata0-ps = <0>;
   status = "okay";
   clock-names = "RefClk" , "s_axi_lite_aclk";
   xlnx,ksharedlogic;
   xlnx,name = "MIPI_D_PHY_RX_0";
  };
  axi_vdma_0: dma@43000000 {
   xlnx,enable-debug-info-9 = <0x0>;
   xlnx,enable-s2mm-frm-counter = <1>;
   xlnx,enable-mm2s-delay-counter = <1>;
   xlnx,dlytmr-resolution = <125>;
   xlnx,enable-s2mm-buf-full = <0>;
   xlnx,rable = <0>;
   xlnx,ip-name = "axi_vdma";
   xlnx,s2mm-genlock-num-masters = <1>;
   reg = <0x43000000 0x10000>;
   xlnx,enable-s2mm-fsync-out = <0>;
   xlnx,use-mm2s-fsync = <0>;
   xlnx,s2mm-linebuffer-thresh = <4>;
   xlnx,enable-mm2s-frm-counter = <1>;
   xlnx,m-axis-mm2s-tdata-width = <24>;
   xlnx,enable-all = <0>;
   xlnx,dynamic-resolution = <1>;
   xlnx,enable-s2mm-rst-out = <0>;
   interrupt-names = "mm2s_introut" , "s2mm_introut";
   xlnx,s2mm-sof-enable = <0x1>;
   xlnx,mm2s-genlock-num-masters = <1>;
   xlnx,enable-mm2s-fsync-out = <0>;
   xlnx,mm2s-linebuffer-thresh = <4>;
   compatible = "xlnx,axi-vdma-6.3" , "xlnx,axi-vdma-1.00.a";
   xlnx,enable-tstvec = <0>;
   interrupt-parent = <&intc>;
   xlnx,s2mm-max-burst-length = <8>;
   xlnx,enable-mm2s-rst-out = <0>;
   xlnx,mm2s-sof-enable = <1>;
   xlnx,include-s2mm-dre = <0x0>;
   xlnx,enable-s2mm-frmstr-reg = <0>;
   xlnx,m-axi-s2mm-data-width = <64>;
   xlnx,enable-s2mm-sts-reg = <0>;
   xlnx,mm2s-max-burst-length = <8>;
   status = "okay";
   xlnx,include-mm2s-dre = <0x0>;
   xlnx,name = "axi_vdma_0";
   xlnx,enable-mm2s-frmstr-reg = <0>;
   interrupts = < 0 30 4 0 31 4 >;
   xlnx,m-axi-mm2s-data-width = <64>;
   xlnx,flush-on-fsync = <0x1>;
   xlnx,include-s2mm-sf = <1>;
   xlnx,single-interface = <0>;
   xlnx,s2mm-linebuffer-depth = <0x400>;
   xlnx,include-s2mm = <0x1>;
   xlnx,addr-width = <32>;
   clocks = <&misc_clk_1>, <&misc_clk_1>, <&misc_clk_1>, <&misc_clk_0>, <&misc_clk_1>;
   xlnx,s-axis-s2mm-tdata-width = <24>;
   xlnx,enable-debug-info-10 = <0>;
   xlnx,addrwidth = <0x20>;
   xlnx,enable-debug-info-11 = <0>;
   xlnx,edk-iptype = "PERIPHERAL";
   xlnx,enable-debug-info-12 = <0>;
   xlnx,use-fsync = <1>;
   xlnx,s2mm-genlock-repeat-en = <1>;
   xlnx,enable-debug-info-13 = <0x0>;
   xlnx,enable-vert-flip = <0x0>;
   xlnx,include-mm2s-sf = <0>;
   xlnx,enable-debug-info-14 = <0x1>;
   xlnx,include-internal-genlock = <0x1>;
   clock-names = "m_axi_mm2s_aclk" , "m_axi_s2mm_aclk" , "m_axis_mm2s_aclk" , "s_axi_lite_aclk" , "s_axis_s2mm_aclk";
   xlnx,enable-debug-info-15 = <0x1>;
   xlnx,mm2s-linebuffer-depth = <0x400>;
   xlnx,include-mm2s = <0x1>;
   xlnx,s2mm-genlock-mode = <0x2>;
   xlnx,flush-fsync = <0x1>;
   xlnx,select-xpm = <0>;
   xlnx,s2mm-data-width = <0x40>;
   xlnx,num-fstores = <0x3>;
   xlnx,enable-s2mm-param-updt = <0>;
   xlnx,enable-vidparam-reads = <0x1>;
   xlnx,mm2s-genlock-repeat-en = <0>;
   xlnx,prmry-is-aclk-async = <0>;
   xlnx,s2mm-tdata-width = <0x18>;
   #dma-cells = <1>;
   xlnx,enable-vidprmtr-reads = <1>;
   xlnx,mm2s-genlock-mode = <0x3>;
   xlnx,enable-debug-info-0 = <0>;
   xlnx,enable-debug-all = <0x0>;
   xlnx,enable-debug-info-1 = <0x0>;
   xlnx,mm2s-data-width = <0x40>;
   xlnx,enable-debug-info-2 = <0>;
   xlnx,enable-mm2s-param-updt = <0>;
   xlnx,enable-debug-info-3 = <0>;
   xlnx,enable-s2mm-delay-counter = <1>;
   xlnx,enable-debug-info-4 = <0>;
   xlnx,enable-debug-info-5 = <0x0>;
   xlnx,mm2s-tdata-width = <0x18>;
   xlnx,enable-debug-info-6 = <0x1>;
   xlnx,enable-debug-info-7 = <0x1>;
   xlnx,use-s2mm-fsync = <2>;
   xlnx,enable-debug-info-8 = <0>;
   xlnx,enable-mm2s-buf-empty = <0>;
   dma_channel_43000000: dma-channel@43000000 {
    interrupts = < 0 30 4 >;
    xlnx,datawidth = <0x18>;
    xlnx,device-id = <0x0>;
    compatible = "xlnx,axi-vdma-mm2s-channel";
    xlnx,genlock-mode;
   };
   dma_channel_43000030: dma-channel@43000030 {
    interrupts = < 0 31 4 >;
    xlnx,datawidth = <0x18>;
    xlnx,device-id = <0x0>;
    compatible = "xlnx,axi-vdma-s2mm-channel";
    xlnx,genlock-mode;
   };
  };
  video_dynclk: clk_wiz@43c00000 {
   xlnx,reset-type = "ACTIVE_HIGH";
   xlnx,pll-clkout1-divide = <1>;
   xlnx,clk-out1-port = "pxl_clk_5x";
   xlnx,clkout2-phase = <0>;
   xlnx,clkout3-jitter = <0>;
   xlnx,clkout3-drives = "BUFG";
   reg = <0x43c00000 0x10000>;
   xlnx,use-min-o-jitter = <0>;
   xlnx,clkout4-phase-error = <0>;
   xlnx,clkout5-1 = <0000>;
   xlnx,clkout5-2 = <0000>;
   xlnx,clkout5-out-freq = <100>;
   xlnx,d-max = <80>;
   xlnx,clkout1-phase = <0>;
   xlnx,divide6-auto = <1>;
   xlnx,clkout1-jitter = <0xddc1c68>;
   xlnx,clkfb-in-n-port = "clkfb_in_n";
   xlnx,clkout2-requested-out-freq = <100>;
   xlnx,use-locked = <1>;
   xlnx,pll-clkout5-duty-cycle = <0x7a120>;
   xlnx,mmcm-clkout3-use-fine-ps;
   xlnx,clkout1-drives = "No_buffer";
   xlnx,input-clk-stopped-port = "input_clk_stopped";
   xlnx,clkout4-duty-cycle = <50>;
   xlnx,divide3-auto = <1>;
   xlnx,use-dyn-reconfig = <1>;
   xlnx,clkout3-actual-freq = <100>;
   xlnx,clkout6-requested-duty-cycle = <50>;
   xlnx,diff-clk-in1-board-interface = "Custom";
   xlnx,mmcm-clkout0-duty-cycle = <0x7a120>;
   xlnx,clkin2-jitter-ps = <100>;
   xlnx,clkout2-1 = <0000>;
   xlnx,in-freq-units = "Units_MHz";
   xlnx,clkout2-2 = <0000>;
   xlnx,feedback-source = "FDBK_ONCHIP";
   xlnx,clkout5-requested-out-freq = <100>;
   xlnx,name = "video_dynclk";
   xlnx,prim-in-freq = <100>;
   xlnx,mmcm-clkout5-divide = <1>;
   xlnx,clkout3-phase-error = <0>;
   xlnx,clk-in2-board-interface = "Custom";
   xlnx,clkin1-ui-jitter = <0x2710>;
   xlnx,use-inclk-switchover = <0>;
   clock-names = "clk_in1" , "s_axi_aclk";
   xlnx,clkout5-duty-cycle = <50>;
   xlnx,use-clkout4-bar = <0>;
   xlnx,mmcm-clkout1-duty-cycle = <0x7a120>;
   xlnx,mmcm-clkout2-use-fine-ps;
   xlnx,clkout2-out-freq = <100>;
   xlnx,use-dyn-phase-shift = <0>;
   xlnx,mmcm-clkout3-divide = <1>;
   xlnx,clkout2-actual-freq = <100>;
   xlnx,o-min = <1>;
   xlnx,daddr-port = "daddr";
   xlnx,precision = <1>;
   xlnx,mmcm-ref-jitter1 = <0x2710>;
   clock-output-names = "0x43c00000-pxl_clk_5x";
   xlnx,mmcm-ref-jitter2 = <0x2710>;
   xlnx,pll-notes = "No , notes";
   xlnx,use-phase-alignment = <0>;
   xlnx,ref-clk-freq = <100>;
   xlnx,clkout7-requested-duty-cycle = <50>;
   xlnx,inclk-sum-row0 = "Input , Clock , Freq , (MHz) , Input , Jitter , (UI)";
   xlnx,inclk-sum-row1 = "__primary_________100.000____________0.010";
   xlnx,use-fast-simulation = <0>;
   xlnx,inclk-sum-row2 = "no_secondary_input_clock";
   xlnx,mmcm-clkout1-divide = <1>;
   xlnx,din-port = "din";
   xlnx,reset-board-interface = "Custom";
   xlnx,clkout6-duty-cycle = <50>;
   xlnx,clkout2-phase-error = <0>;
   xlnx,clkout1-requested-duty-cycle = <50>;
   xlnx,vco-max = <1200>;
   xlnx,use-freq-synth = <1>;
   xlnx,primtype-sel = "AUTO";
   xlnx,mmcm-clkout2-duty-cycle = <0x7a120>;
   xlnx,clkout5-used = <0>;
   xlnx,d-min = <1>;
   xlnx,pll-clkfbout-mult = <1>;
   xlnx,mmcm-clkout4-cascade;
   xlnx,use-clkout1-bar = <0>;
   xlnx,clk-out6-port = "clk_out6";
   xlnx,clkout2-sequence-number = <1>;
   xlnx,clk-valid-port = "CLK_VALID";
   xlnx,clkout2-used = <0>;
   xlnx,clkout4-sequence-number = <1>;
   xlnx,psincdec-port = "psincdec";
   xlnx,mmcm-clkout1-use-fine-ps;
   xlnx,clk-out3-port = "clk_out3";
   xlnx,clkout6-sequence-number = <1>;
   xlnx,platform = "UNKNOWN";
   xlnx,mmcm-clkout6-phase = <0>;
   xlnx,clkout1-actual-freq = <100>;
   xlnx,clkout6-out-freq = <100>;
   xlnx,nr-outputs = <1>;
   xlnx,mmcm-clkin2-period = <10>;
   xlnx,prim-in-jitter = <0x2710>;
   xlnx,mmcm-clkfbout-mult-f = <0x2367b88>;
   status = "okay";
   xlnx,clkout6-1 = <0000>;
   xlnx,clkout6-2 = <0000>;
   xlnx,clkout7-duty-cycle = <50>;
   xlnx,outclk-sum-row1 = "pxl_clk_5x__742.50000______0.000______50.0______232.529____322.999";
   xlnx,outclk-sum-row2 = "no_CLK_OUT2_output";
   xlnx,jitter-sel = "No_Jitter";
   xlnx,outclk-sum-row3 = "no_CLK_OUT3_output";
   xlnx,mmcm-clkout5-phase = <0>;
   xlnx,mmcm-clkout3-duty-cycle = <0x7a120>;
   xlnx,outclk-sum-row4 = "no_CLK_OUT4_output";
   xlnx,clkfb-in-p-port = "clkfb_in_p";
   xlnx,outclk-sum-row5 = "no_CLK_OUT5_output";
   xlnx,outclk-sum-row6 = "no_CLK_OUT6_output";
   xlnx,outclk-sum-row7 = "no_CLK_OUT7_output";
   xlnx,clkout1-phase-error = <0x134092d8>;
   xlnx,divide5-auto = <1>;
   xlnx,pll-bandwidth = "OPTIMIZED";
   xlnx,psdone-port = "psdone";
   xlnx,clkout3-1 = <0000>;
   xlnx,edk-iptype = "PERIPHERAL";
   xlnx,clkout3-2 = <0000>;
   xlnx,clkout3-requested-out-freq = <100>;
   xlnx,pll-clkout4-divide = <1>;
   xlnx,pll-clkfbout-phase = <0>;
   xlnx,divide2-auto = <1>;
   xlnx,override-mmcm = <0>;
   xlnx,clkout6-jitter = <0>;
   xlnx,ss-mode = "CENTER_HIGH";
   xlnx,mmcm-notes = "None";
   xlnx,clkout6-drives = "BUFG";
   xlnx,mmcm-clkfbout-use-fine-ps;
   xlnx,mmcm-clkout0-use-fine-ps;
   xlnx,ss-mod-period = <4000>;
   xlnx,mmcm-clkout4-phase = <0>;
   xlnx,clkout2-requested-duty-cycle = <50>;
   xlnx,clkout0-actual-freq = <0x2c41a6a0>;
   xlnx,divclk = <0000>;
   xlnx,primary-port = "clk_in1";
   xlnx,mmcm-compensation = "ZHOLD";
   xlnx,clkout6-requested-out-freq = <100>;
   xlnx,clkout0-1 = <0000>;
   xlnx,clkout0-2 = <0000>;
   xlnx,clkout2-requested-phase = <0>;
   xlnx,relative-inclk = "REL_PRIMARY";
   xlnx,pll-clkout2-divide = <1>;
   xlnx,clkout4-requested-phase = <0>;
   xlnx,enable-user-clock0 = <0>;
   xlnx,filter-1 = <0000>;
   xlnx,mmcm-clkout4-duty-cycle = <0x7a120>;
   xlnx,clkout3-out-freq = <100>;
   xlnx,clkout4-jitter = <0>;
   xlnx,clkout6-requested-phase = <0>;
   xlnx,clkoutphy-requested-freq = <600>;
   xlnx,enable-user-clock1 = <0>;
   xlnx,filter-2 = <0000>;
   xlnx,ss-mod-freq = <250>;
   xlnx,in-jitter-units = "Units_UI";
   xlnx,mmcm-divclk-divide = <5>;
   xlnx,enable-user-clock2 = <0>;
   xlnx,enable-user-clock3 = <0>;
   xlnx,lock-1 = <0000>;
   xlnx,clkout4-drives = "BUFG";
   xlnx,lock-2 = <0000>;
   xlnx,den-port = "den";
   xlnx,lock-3 = <0000>;
   xlnx,use-safe-clock-startup = <0>;
   xlnx,mmcm-clkout3-phase = <0>;
   xlnx,dwe-port = "dwe";
   xlnx,clkfb-out-n-port = "clkfb_out_n";
   xlnx,use-max-i-jitter = <0>;
   xlnx,drdy-port = "drdy";
   xlnx,vco-min = <600>;
   xlnx,pll-clkout0-divide = <1>;
   xlnx,clkin1-jitter-ps = <100>;
   xlnx,clk-in-sel-port = "clk_in_sel";
   xlnx,use-freeze = <0>;
   xlnx,clkout2-jitter = <0>;
   xlnx,enable-pll0 = <0>;
   xlnx,enable-pll1 = <0>;
   xlnx,clkout2-drives = "BUFG";
   xlnx,use-inclk-stopped = <0>;
   xlnx,use-clkout3-bar = <0>;
   xlnx,use-clk-valid = <0>;
   xlnx,mmcm-clkout2-phase = <0>;
   compatible = "xlnx,clk-wiz-6.0" , "xlnx,clocking-wizard";
   xlnx,reset-port = "reset";
   xlnx,mmcm-clkout5-duty-cycle = <0x7a120>;
   xlnx,mmcm-clkout0-divide-f = <1>;
   xlnx,pll-clkout0-duty-cycle = <0x7a120>;
   xlnx,speed-grade = <1>;
   xlnx,diff-clk-in2-board-interface = "Custom";
   xlnx,clkout3-requested-duty-cycle = <50>;
   xlnx,use-status = <0>;
   xlnx,prim-source = "No_buffer";
   xlnx,clkout7-used = <0>;
   xlnx,use-clkfb-stopped = <0>;
   xlnx,mmcm-clkout1-phase = <0>;
   xlnx,mmcm-clkout6-divide = <1>;
   xlnx,clkout7-out-freq = <100>;
   xlnx,prim-in-timeperiod = <10>;
   xlnx,clkout4-used = <0>;
   xlnx,pll-clkin-period = <1>;
   xlnx,clk-out5-port = "clk_out5";
   xlnx,psclk-port = "psclk";
   xlnx,primitive = "MMCM";
   xlnx,clkout1-used;
   xlnx,reset-low = <0>;
   xlnx,clk-in1-board-interface = "Custom";
   xlnx,clk-out2-port = "clk_out2";
   xlnx,power-down-port = "power_down";
   xlnx,clkout7-phase-error = <0>;
   xlnx,mmcm-clkout6-duty-cycle = <0x7a120>;
   xlnx,mmcm-clkout4-divide = <1>;
   xlnx,mmcm-clkfbout-phase = <0>;
   xlnx,mmcm-clkout0-phase = <0>;
   xlnx,pll-clkout1-duty-cycle = <0x7a120>;
   xlnx,user-clk-freq0 = <100>;
   xlnx,user-clk-freq1 = <100>;
   xlnx,pll-clkout5-phase = <0>;
   xlnx,user-clk-freq2 = <100>;
   xlnx,secondary-in-jitter = <0x2710>;
   xlnx,user-clk-freq3 = <100>;
   xlnx,use-clock-sequencing = <0>;
   xlnx,secondary-source = "Single_ended_clock_capable_pin";
   xlnx,cddcdone-port = "cddcdone";
   xlnx,clkout7-phase = <0>;
   xlnx,psen-port = "psen";
   xlnx,mmcm-clkout6-use-fine-ps;
   xlnx,dclk-port = "dclk";
   xlnx,m-max = <64>;
   xlnx,outclk-sum-row0a = "Output , Output , Phase , Duty , Cycle , Pk-to-Pk , Phase";
   xlnx,clkout1-requested-out-freq = <0x2c41a6a0>;
   xlnx,clkout6-actual-freq = <100>;
   xlnx,divide7-auto = <1>;
   xlnx,outclk-sum-row0b = "Clock , Freq , (MHz) , (degrees) , (%) , Jitter , (ps) , Error , (ps)";
   xlnx,clkout4-1 = <0000>;
   xlnx,locked-port = "locked";
   xlnx,clkin2-ui-jitter = <0x2710>;
   xlnx,clkout4-2 = <0000>;
   xlnx,enable-clock-monitor = <0>;
   xlnx,mmcm-clkout2-divide = <1>;
   xlnx,ss-mod-time = <0xfa0>;
   xlnx,auto-primitive = "MMCM";
   xlnx,divide4-auto = <1>;
   xlnx,pll-clkout4-phase = <0>;
   xlnx,use-power-down = <0>;
   xlnx,mmcm-startup-wait;
   xlnx,divide1-auto = <1>;
   xlnx,clkout4-requested-duty-cycle = <50>;
   xlnx,clkout4-requested-out-freq = <100>;
   xlnx,rable = <0>;
   xlnx,optimize-clocking-structure-en = <0>;
   xlnx,clkout4-out-freq = <100>;
   xlnx,clkout6-phase = <0>;
   xlnx,num-out-clks = <1>;
   xlnx,ip-name = "clk_wiz";
   xlnx,pll-clkout2-duty-cycle = <0x7a120>;
   xlnx,clkout1-1 = <0000>;
   xlnx,clkout1-2 = <0000>;
   xlnx,clkout1-duty-cycle = <50>;
   xlnx,pll-divclk-divide = <1>;
   xlnx,clkout6-phase-error = <0>;
   xlnx,dout-port = "dout";
   xlnx,clkout1-sequence-number = <1>;
   xlnx,pll-clkout3-phase = <0>;
   xlnx,cddcreq-port = "cddcreq";
   xlnx,clkout7-requested-out-freq = <100>;
   xlnx,use-spread-spectrum = <0>;
   xlnx,clkout3-sequence-number = <1>;
   xlnx,clkout5-sequence-number = <1>;
   xlnx,summary-strings = "empty";
   xlnx,pll-compensation = "SYSTEM_SYNCHRONOUS";
   xlnx,clkout5-phase = <0>;
   xlnx,clkout7-sequence-number = <1>;
   xlnx,status-port = "STATUS";
   xlnx,clkfb-out-p-port = "clkfb_out_p";
   xlnx,mmcm-clkout5-use-fine-ps;
   xlnx,clkfb-in-port = "clkfb_in";
   xlnx,clkout5-actual-freq = <100>;
   xlnx,input-mode = "frequency";
   xlnx,interface-selection = <1>;
   xlnx,mmcm-bandwidth = "OPTIMIZED";
   xlnx,calc-done = "empty";
   xlnx,pll-clkout2-phase = <0>;
   xlnx,has-cddc = <0>;
   xlnx,power-reg = <0000>;
   xlnx,mmcm-clkin1-period = <10>;
   xlnx,pll-clkout3-duty-cycle = <0x7a120>;
   xlnx,clkout2-duty-cycle = <50>;
   xlnx,clkout4-phase = <0>;
   xlnx,pll-clkout5-divide = <1>;
   xlnx,clkout7-jitter = <0>;
   clocks = <&clkc 15>, <&misc_clk_0>;
   xlnx,clkout1-out-freq = <0x2c41a6a0>;
   xlnx,clkout7-drives = "BUFG";
   xlnx,clkout5-phase-error = <0>;
   xlnx,pll-clkout1-phase = <0>;
   xlnx,clkout5-requested-duty-cycle = <50>;
   xlnx,clkfb-in-signaling = "SINGLE";
   xlnx,enable-clkoutphy = <0>;
   xlnx,use-clkout2-bar = <0>;
   xlnx,secondary-port = "clk_in2";
   xlnx,clkout3-phase = <0>;
   xlnx,pll-clkout3-divide = <1>;
   xlnx,use-reset = <1>;
   xlnx,override-pll = <0>;
   xlnx,clkoutphy-mode = "VCO";
   xlnx,mmcm-clkout4-use-fine-ps;
   xlnx,clkout5-jitter = <0>;
   xlnx,clkout6-used = <0>;
   xlnx,m-min = <2>;
   xlnx,mmcm-clock-hold;
   xlnx,clkfb-stopped-port = "clkfb_stopped";
   xlnx,clk-out7-port = "clk_out7";
   xlnx,clkout1-requested-phase = <0>;
   xlnx,clkout4-actual-freq = <100>;
   xlnx,pll-clk-feedback = "CLKFBOUT";
   xlnx,phaseshift-mode = "WAVEFORM";
   xlnx,clkfb-out-port = "clkfb_out";
   xlnx,clkout5-drives = "BUFG";
   xlnx,o-max = <128>;
   xlnx,clkout3-requested-phase = <0>;
   xlnx,jitter-options = "UI";
   xlnx,secondary-in-timeperiod = <10>;
   xlnx,pll-clkout4-duty-cycle = <0x7a120>;
   xlnx,clkout3-used = <0>;
   xlnx,clkout5-requested-phase = <0>;
   xlnx,pll-clkout0-phase = <0>;
   xlnx,clk-out4-port = "clk_out4";
   xlnx,clkout3-duty-cycle = <50>;
   xlnx,clkout7-requested-phase = <0>;
   xlnx,secondary-in-freq = <100>;
   xlnx,use-min-power = <0>;
   xlnx,clkfbout-1 = <0000>;
   #clock-cells = <1>;
   xlnx,pll-ref-jitter = <0x2710>;
   xlnx,clkfbout-2 = <0000>;
   xlnx,component-name = "system_video_dynclk_0";
  };
  vtg: v_tc_0@43c10000 {
   xlnx,gen-vsync-en = <1>;
   xlnx,gen-hframe-size = <1650>;
   xlnx,num-fsyncs = <1>;
   xlnx,gen-hactive-size = <1280>;
   xlnx,gen-video-format = <2>;
   xlnx,gen-f1-vblank-hstart = <1280>;
   xlnx,active-video-detection;
   xlnx,horizontal-sync-generation;
   xlnx,gen-vblank-en = <1>;
   xlnx,rable = <0>;
   xlnx,gen-avideo-en = <1>;
   xlnx,ip-name = "v_tc";
   xlnx,fsync-hstart10 = <0>;
   xlnx,max-pixels = <4096>;
   xlnx,gen-fieldid-en = <0>;
   reg = <0x43c10000 0x10000>;
   xlnx,det-vblank-en = <1>;
   xlnx,fsync-hstart11 = <0>;
   xlnx,fsync-hstart12 = <0>;
   xlnx,gen-interlaced = <0>;
   xlnx,det-avideo-en = <1>;
   xlnx,fsync-hstart13 = <0>;
   xlnx,fsync-hstart14 = <0>;
   xlnx,gen-f0-vblank-hend = <1280>;
   xlnx,fsync-hstart15 = <0>;
   xlnx,det-hsync-en = <1>;
   interrupt-names = "irq";
   xlnx,enable-generation;
   xlnx,gen-avideo-polarity = <1>;
   xlnx,max-lines-per-frame = <4096>;
   xlnx,has-axi4-lite = <1>;
   compatible = "xlnx,v-tc-6.2" , "xlnx,v-tc-6.1" , "xlnx,bridge-v-tc-6.1";
   xlnx,max-clocks-per-line = <4096>;
   xlnx,gen-f0-vframe-size = <750>;
   xlnx,gen-f0-vblank-hstart = <1280>;
   interrupt-parent = <&intc>;
   xlnx,max-lines = <4096>;
   xlnx,gen-f0-vsync-vstart = <724>;
   xlnx,horizontal-blank-generation;
   xlnx,gen-hsync-end = <1430>;
   xlnx,gen-f0-vsync-hstart = <1280>;
   xlnx,gen-f1-vsync-hend = <1280>;
   xlnx,vertical-blank-detection;
   xlnx,gen-vsync-polarity = <1>;
   xlnx,fsync-vstart0 = <0>;
   xlnx,gen-hsync-polarity = <1>;
   xlnx,fsync-vstart1 = <0>;
   status = "okay";
   xlnx,fsync-vstart2 = <0>;
   xlnx,gen-hsync-start = <1390>;
   xlnx,gen-f0-vsync-vend = <729>;
   xlnx,fsync-vstart3 = <0>;
   xlnx,vertical-sync-detection;
   xlnx,frame-syncs = <1>;
   xlnx,fsync-vstart4 = <0>;
   xlnx,gen-vactive-size = <720>;
   xlnx,name = "vtg";
   xlnx,fsync-vstart5 = <0>;
   interrupts = < 0 29 4 >;
   xlnx,gen-achroma-en = <0>;
   xlnx,fsync-vstart6 = <0>;
   xlnx,det-fieldid-en = <0>;
   xlnx,fsync-vstart7 = <0>;
   xlnx,gen-hsync-en = <1>;
   xlnx,fsync-vstart8 = <0>;
   xlnx,fsync-vstart10 = <0>;
   xlnx,fsync-vstart9 = <0>;
   xlnx,fsync-vstart11 = <0>;
   xlnx,arbitrary-res-en = <0>;
   xlnx,fsync-vstart12 = <0>;
   xlnx,fsync-vstart13 = <0>;
   clocks = <&misc_clk_4>, <&misc_clk_0>;
   xlnx,fsync-vstart14 = <0>;
   xlnx,fsync-vstart15 = <0>;
   xlnx,gen-cparity = <0>;
   xlnx,edk-iptype = "PERIPHERAL";
   xlnx,vertical-sync-generation;
   xlnx,gen-f1-vblank-hend = <1280>;
   xlnx,det-vsync-en = <1>;
   xlnx,gen-vblank-polarity = <1>;
   xlnx,fsync-hstart0 = <0>;
   clock-names = "clk" , "s_axi_aclk";
   xlnx,interlace-en = <0>;
   xlnx,gen-auto-switch = <0>;
   xlnx,fsync-hstart1 = <0>;
   xlnx,fsync-hstart2 = <0>;
   xlnx,active-video-generation;
   xlnx,fsync-hstart3 = <0>;
   xlnx,fsync-hstart4 = <0>;
   xlnx,horizontal-blank-detection;
   xlnx,fsync-hstart5 = <0>;
   xlnx,gen-f1-vframe-size = <750>;
   xlnx,fsync-hstart6 = <0>;
   xlnx,fsync-hstart7 = <0>;
   xlnx,fsync-hstart8 = <0>;
   xlnx,gen-hblank-en = <1>;
   xlnx,fsync-hstart9 = <0>;
   xlnx,horizontal-sync-detection;
   xlnx,gen-f1-vsync-vend = <729>;
   xlnx,detect-en = <0>;
   xlnx,det-hblank-en = <1>;
   xlnx,vid-ppc = <4>;
   xlnx,has-intc-if = <0>;
   xlnx,generate-en = <1>;
   xlnx,gen-achroma-polarity = <1>;
   xlnx,sync-en = <0>;
   xlnx,det-achroma-en = <0>;
   xlnx,generator;
   xlnx,gen-f1-vsync-vstart = <724>;
   xlnx,vertical-blank-generation;
   xlnx,gen-hblank-polarity = <1>;
   xlnx,video-mode = "720p";
   xlnx,gen-fieldid-polarity = <1>;
   xlnx,gen-f1-vsync-hstart = <1280>;
   xlnx,gen-f0-vsync-hend = <1280>;
  };
 };
};
# 4 "C:\\Users\\sanat\\FPGA_Car_Project\\Vitis_Vid_Pass\\vitis_vid_pass\\hw\\sdt\\system-top.dts" 2
# 1 "C:\\Users\\sanat\\FPGA_Car_Project\\Vitis_Vid_Pass\\vitis_vid_pass\\hw\\sdt\\pcw.dtsi" 1
 &ps7_cortexa9_0 {
  xlnx,rable = <0>;
  xlnx,i-cache-size = <0x8000>;
  xlnx,d-cache-line-size = <20>;
  xlnx,i-cache-line-size = <20>;
  xlnx,cpu-1x-clk-freq-hz = <111111115>;
  xlnx,ip-name = "ps7_cortexa9";
  xlnx,d-cache-size = <0x8000>;
  xlnx,num-cores = <2>;
  xlnx,cpu-clk-freq-hz = <666666687>;
  bus-handle = <&amba>;
 };
 &ps7_cortexa9_1 {
  xlnx,rable = <0>;
  xlnx,i-cache-size = <0x8000>;
  xlnx,d-cache-line-size = <20>;
  xlnx,i-cache-line-size = <20>;
  xlnx,cpu-1x-clk-freq-hz = <111111115>;
  xlnx,ip-name = "ps7_cortexa9";
  xlnx,d-cache-size = <0x8000>;
  xlnx,cpu-clk-freq-hz = <666666687>;
  bus-handle = <&amba>;
 };
 &amba {
  processing_system7_0: processing_system7@0 {
   xlnx,pcw-act-enet0-peripheral-freqmhz = <125>;
   xlnx,pcw-nor-grp-sram-cs1-io = "<Select>";
   xlnx,num-f2p-intr-inputs = <3>;
   reg = <0x00000000 0x40000000>;
   xlnx,pcw-mio-18-direction = "out";
   xlnx,pcw-nand-cycles-t-ar = <1>;
   xlnx,pcw-p2f-dmac7-intr = <0>;
   xlnx,pcw-en-clk1-port = <0>;
   xlnx,pcw-crystal-peripheral-freqmhz = <0x1fca055>;
   xlnx,pcw-nor-grp-sram-cs1-enable = <0>;
   xlnx,pcw-uiparam-ddr-dq-1-package-length = <0x416881c>;
   xlnx,pcw-act-wdt-peripheral-freqmhz = <0x69f6bcb>;
   xlnx,pcw-p2f-dmac4-intr = <0>;
   xlnx,pcw-s-axi-hp3-id-width = <6>;
   xlnx,pcw-en-emio-uart0 = <0>;
   xlnx,pcw-en-emio-uart1 = <0>;
   xlnx,pcw-ddr-hpr-to-critical-priority-level = <15>;
   xlnx,pcw-enet-reset-polarity = "Active , Low";
   xlnx,pcw-mio-5-direction = "inout";
   xlnx,pcw-p2f-dmac1-intr = <0>;
   xlnx,pcw-sd0-grp-cd-enable = <1>;
   xlnx,pcw-act-fpga1-peripheral-freqmhz = <10>;
   xlnx,pcw-mio-24-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-ddr-port1-hpr-enable = <0>;
   xlnx,pcw-cpu-peripheral-clksrc = "ARM , PLL";
   xlnx,pcw-mio-53-pullup = "enabled";
   xlnx,pcw-use-axi-nonsecure = <0>;
   xlnx,pcw-spi1-grp-ss0-io = "<Select>";
   xlnx,pcw-uiparam-ddr-dqs-1-propogation-delay = <160>;
   xlnx,pcw-nor-cs1-t-pc = <1>;
   xlnx,pcw-s-axi-gp0-id-width = <6>;
   xlnx,pcw-trace-grp-8bit-io = "<Select>";
   xlnx,pcw-ttc-peripheral-freqmhz = <50>;
   xlnx,pcw-ftm-cti-out0 = "<Select>";
   xlnx,pcw-ttc1-clk0-peripheral-divisor0 = <1>;
   xlnx,pcw-uiparam-ddr-clock-stop-en = <0>;
   xlnx,pcw-ftm-cti-out1 = "<Select>";
   xlnx,pcw-mio-28-direction = "inout";
   xlnx,pcw-en-ttc0 = <0>;
   xlnx,pcw-ftm-cti-out2 = "<Select>";
   xlnx,pcw-mio-11-pullup = "enabled";
   xlnx,pcw-en-ttc1 = <0>;
   xlnx,pcw-ftm-cti-out3 = "<Select>";
   xlnx,pcw-m-axi-gp0-thread-id-width = <12>;
   xlnx,pcw-nor-sram-cs1-t-ceoe = <1>;
   xlnx,pcw-nor-grp-sram-int-enable = <0>;
   xlnx,pcw-can-peripheral-clksrc = "IO , PLL";
   xlnx,pcw-ttc1-clk0-peripheral-clksrc = "CPU_1X";
   xlnx,mio-primitive = <54>;
   xlnx,pcw-gp0-num-write-threads = <4>;
   xlnx,pcw-mio-4-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-mio-41-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-nor-cs1-t-rc = <11>;
   xlnx,pcw-i2c0-grp-int-enable = <1>;
   xlnx,pcw-ddr-port2-hpr-enable = <0>;
   xlnx,pcw-act-tpiu-peripheral-freqmhz = <200>;
   xlnx,pcw-package-name = "clg400";
   xlnx,pcw-s-axi-hp0-id-width = <6>;
   xlnx,pcw-package-ddr-dqs-to-clk-delay-0 = <0xffff9e58>;
   xlnx,pcw-uiparam-ddr-dqs-3-length-mm = <0>;
   xlnx,pcw-package-ddr-dqs-to-clk-delay-1 = <0x36b0>;
   xlnx,pcw-qspi-grp-single-ss-enable = <1>;
   xlnx,pcw-act-usb0-peripheral-freqmhz = <60>;
   xlnx,pcw-mio-27-pullup = "enabled";
   xlnx,pcw-package-ddr-dqs-to-clk-delay-2 = <0xffffdcd8>;
   xlnx,pcw-mio-38-direction = "inout";
   xlnx,pcw-package-ddr-dqs-to-clk-delay-3 = <0xffff7f18>;
   xlnx,pcw-en-spi0 = <0>;
   xlnx,pcw-en-spi1 = <0>;
   xlnx,pcw-include-acp-trans-check = <0>;
   xlnx,pcw-ttc1-clk1-peripheral-divisor0 = <1>;
   xlnx,en-emio-pjtag = <0>;
   xlnx,pcw-nor-sram-cs0-we-time = <0>;
   xlnx,pcw-uiparam-ddr-speed-bin = "DDR3_1066F";
   xlnx,pcw-ddr-port3-hpr-enable = <0>;
   xlnx,pcw-irq-f2p-intr = <1>;
   xlnx,pcw-trace-peripheral-enable = <0>;
   xlnx,pcw-enet0-grp-mdio-io = "MIO , 52 , .. , 53";
   xlnx,pcw-mio-15-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-nor-cs1-t-tr = <1>;
   xlnx,pcw-mio-7-pullup = "disabled";
   xlnx,pcw-mio-44-pullup = "enabled";
   xlnx,pcw-uiparam-act-ddr-freq-mhz = <0x1fca057e>;
   xlnx,pcw-use-ps-slcr-registers = <0>;
   xlnx,pcw-mio-48-direction = "out";
   xlnx,pcw-ttc0-clk0-peripheral-divisor0 = <1>;
   xlnx,pcw-trace-grp-4bit-io = "<Select>";
   xlnx,pcw-nor-cs1-t-wc = <11>;
   xlnx,pcw-gpio-peripheral-enable = <0>;
   xlnx,pcw-nor-cs0-t-ceoe = <1>;
   xlnx,pcw-spi-peripheral-clksrc = "IO , PLL";
   xlnx,pcw-p2f-i2c0-intr = <0>;
   xlnx,pcw-uiparam-ddr-clock-2-package-length = <0x4cb9f7c>;
   status = "okay";
   xlnx,pcw-act-can-peripheral-freqmhz = <10>;
   xlnx,pcw-ttc1-clk2-peripheral-divisor0 = <1>;
   xlnx,pcw-mio-32-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-p2f-gpio-intr = <0>;
   xlnx,pcw-ttc0-clk1-peripheral-freqmhz = <0x7f28155>;
   xlnx,pcw-value-silversion = <3>;
   xlnx,pcw-nor-cs1-t-wp = <1>;
   xlnx,pcw-pcap-peripheral-divisor0 = <5>;
   xlnx,pcw-spi1-spi1-io = "<Select>";
   xlnx,pcw-wdt-peripheral-freqmhz = <0x7f28155>;
   xlnx,pcw-mio-18-pullup = "enabled";
   xlnx,pcw-uart1-baud-rate = <115200>;
   xlnx,pcw-mio-17-direction = "out";
   xlnx,pcw-trace-grp-2bit-io = "<Select>";
   xlnx,pcw-ttc0-clk1-peripheral-divisor0 = <1>;
   xlnx,pcw-spi0-grp-ss2-enable = <0>;
   xlnx,pcw-mio-48-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-gp1-num-read-threads = <4>;
   xlnx,pcw-i2c1-i2c1-io = "<Select>";
   xlnx,pcw-sd0-sd0-io = "MIO , 40 , .. , 45";
   xlnx,pcw-can0-grp-clk-enable = <0>;
   xlnx,pcw-mio-4-direction = "inout";
   xlnx,pcw-use-s-axi-acp = <0>;
   xlnx,pcw-uart-peripheral-freqmhz = <100>;
   xlnx,pcw-mio-35-pullup = "enabled";
   xlnx,pcw-uiparam-ddr-partno = "MT41K256M16 , RE-125";
   xlnx,pcw-qspi-peripheral-freqmhz = <200>;
   xlnx,pcw-mio-27-direction = "in";
   xlnx,pcw-mio-52-slew = "slow";
   xlnx,pcw-can0-peripheral-clksrc = "External";
   xlnx,pcw-uiparam-ddr-dram-width = "16 , Bits";
   xlnx,pcw-nand-cycles-t-rc = <11>;
   xlnx,pcw-nand-nand-io = "<Select>";
   xlnx,pcw-nor-grp-sram-cs0-enable = <0>;
   xlnx,pcw-clk1-freq = <10000000>;
   xlnx,pcw-mio-48-slew = "slow";
   xlnx,pcw-m-axi-gp0-freqmhz = <50>;
   xlnx,pcw-ttc0-clk2-peripheral-divisor0 = <1>;
   xlnx,pcw-mio-45-slew = "slow";
   xlnx,pcw-mio-23-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-mio-42-slew = "slow";
   xlnx,pcw-mio-52-pullup = "enabled";
   xlnx,m-axi-gp1-id-width = <12>;
   xlnx,pcw-fclk-clk1-buf;
   xlnx,pcw-dual-parallel-qspi-data-mode = "<Select>";
   xlnx,pcw-nand-cycles-t-rr = <1>;
   xlnx,pcw-en-rst1-port = <0>;
   xlnx,pcw-mio-38-slew = "slow";
   xlnx,pcw-uiparam-ddr-dqs-2-length-mm = <0>;
   xlnx,pcw-mio-10-pullup = "enabled";
   xlnx,pcw-mio-35-slew = "slow";
   xlnx,pcw-mio-37-direction = "inout";
   xlnx,pcw-nor-sram-cs0-t-ceoe = <1>;
   xlnx,pcw-usb0-usb0-io = "MIO , 28 , .. , 39";
   xlnx,pcw-mio-32-slew = "slow";
   xlnx,pcw-usb1-reset-io = "<Select>";
   xlnx,pcw-enet1-peripheral-freqmhz = "1000 , Mbps";
   xlnx,pcw-uiparam-ddr-clock-2-propogation-delay = <160>;
   xlnx,pcw-qspi-grp-single-ss-io = "MIO , 1 , .. , 6";
   xlnx,pcw-mio-28-slew = "slow";
   xlnx,pcw-mio-3-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-mio-39-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-mio-40-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-nor-cs1-we-time = <0>;
   xlnx,pcw-enet1-reset-enable = <0>;
   xlnx,pcw-mio-25-slew = "slow";
   xlnx,pcw-can0-grp-clk-io = "<Select>";
   xlnx,pcw-uiparam-ddr-dq-3-length-mm = <0>;
   xlnx,pcw-mio-22-slew = "slow";
   xlnx,pcw-mio-26-pullup = "enabled";
   xlnx,pcw-nand-cycles-t-wc = <11>;
   xlnx,pcw-fpga2-peripheral-freqmhz = <200>;
   xlnx,pcw-mio-18-slew = "slow";
   xlnx,pcw-mio-47-direction = "inout";
   xlnx,pcw-nor-sram-cs0-t-pc = <1>;
   xlnx,pcw-can-peripheral-freqmhz = <100>;
   xlnx,pcw-uiparam-ddr-dq-2-package-length = <0x561cad8>;
   xlnx,pcw-mio-15-slew = "slow";
   xlnx,emio-gpio-width = <1>;
   xlnx,fclk-clk2-buf;
   xlnx,s-axi-hp0-data-width = <64>;
   xlnx,pcw-mio-12-slew = "slow";
   xlnx,pcw-en-emio-wdt = <0>;
   xlnx,pcw-nand-cycles-t-wp = <1>;
   xlnx,pcw-ttc1-clk0-peripheral-freqmhz = <0x7f28155>;
   xlnx,pcw-ddr-ddr-pll-freqmhz = <0x3f940bf7>;
   xlnx,pcw-enet0-reset-enable = <0>;
   xlnx,pcw-preset-bank1-voltage = "LVCMOS , 1.8V";
   xlnx,pcw-can1-grp-clk-io = "<Select>";
   xlnx,pcw-nor-sram-cs0-t-rc = <11>;
   xlnx,pcw-mio-14-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-mio-6-pullup = "disabled";
   xlnx,pcw-enet-reset-enable = <0>;
   xlnx,pcw-mio-43-pullup = "enabled";
   xlnx,pcw-spi0-peripheral-enable = <0>;
   xlnx,pcw-act-ttc0-clk0-peripheral-freqmhz = <0x69f6bcb>;
   xlnx,pcw-nor-grp-a25-enable = <0>;
   xlnx,trace-buffer-fifo-size = <128>;
   xlnx,pcw-uiparam-ddr-dq-1-propogation-delay = <160>;
   xlnx,pcw-trace-grp-4bit-enable = <0>;
   xlnx,pcw-qspi-peripheral-enable = <1>;
   xlnx,pcw-dual-stack-qspi-data-mode = "<Select>";
   xlnx,pcw-package-ddr-board-delay0 = <0x15ba8>;
   xlnx,pcw-wdt-peripheral-divisor0 = <1>;
   xlnx,s-axi-hp1-data-width = <64>;
   xlnx,pcw-package-ddr-board-delay1 = <0x124f8>;
   xlnx,pcw-package-ddr-board-delay2 = <0x14c08>;
   xlnx,pcw-uart0-baud-rate = <115200>;
   xlnx,pcw-en-wdt = <0>;
   xlnx,pcw-include-trace-buffer = <0>;
   xlnx,pcw-m-axi-gp0-id-width = <12>;
   xlnx,pcw-mio-16-direction = "out";
   xlnx,pcw-package-ddr-board-delay3 = <0x16760>;
   xlnx,pcw-dci-peripheral-clksrc = "DDR , PLL";
   xlnx,pcw-i2c1-grp-int-enable = <0>;
   xlnx,pcw-p2f-uart1-intr = <0>;
   xlnx,pcw-mio-31-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-usb1-peripheral-enable = <0>;
   xlnx,pcw-act-can0-peripheral-freqmhz = <0x16b4ddc>;
   xlnx,pcw-mio-3-direction = "inout";
   xlnx,pcw-s-axi-hp3-freqmhz = <10>;
   xlnx,pcw-m-axi-gp0-support-narrow-burst = <0>;
   xlnx,pcw-nor-sram-cs0-t-tr = <1>;
   xlnx,pcw-usb1-reset-enable = <0>;
   xlnx,pcw-can1-can1-io = "<Select>";
   xlnx,pcw-mio-17-pullup = "enabled";
   xlnx,pcw-uiparam-ddr-clock-3-length-mm = <0>;
   xlnx,pcw-sd1-peripheral-enable = <0>;
   xlnx,s-axi-hp2-data-width = <64>;
   xlnx,pcw-en-trace = <0>;
   xlnx,pcw-mio-26-direction = "in";
   xlnx,pcw-nor-sram-cs0-t-wc = <11>;
   xlnx,pcw-spi0-grp-ss1-enable = <0>;
   xlnx,pcw-ttc0-clk1-peripheral-clksrc = "CPU_1X";
   xlnx,pcw-mio-47-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-nand-grp-d8-io = "<Select>";
   xlnx,pcw-nor-grp-cs1-io = "<Select>";
   xlnx,pcw-en-clktrig1-port = <0>;
   xlnx,pcw-i2c-reset-select = "<Select>";
   xlnx,pcw-act-uart-peripheral-freqmhz = <100>;
   xlnx,pcw-nor-grp-cs1-enable = <0>;
   xlnx,pcw-usb1-peripheral-freqmhz = <60>;
   xlnx,pcw-usb0-reset-enable = <1>;
   xlnx,m-axi-gp1-thread-id-width = <12>;
   xlnx,pcw-nor-sram-cs0-t-wp = <1>;
   xlnx,pcw-mio-34-pullup = "enabled";
   xlnx,pcw-act-qspi-peripheral-freqmhz = <200>;
   xlnx,pcw-sdio-peripheral-valid = <1>;
   xlnx,pcw-en-emio-trace = <0>;
   xlnx,pcw-uart0-grp-full-enable = <0>;
   xlnx,pcw-uiparam-ddr-dqs-1-length-mm = <0>;
   xlnx,pcw-uiparam-ddr-clock-3-package-length = <0x4cb9f7c>;
   xlnx,s-axi-hp3-data-width = <64>;
   xlnx,pcw-mio-36-direction = "in";
   xlnx,pcw-qspi-qspi-io = "MIO , 1 , .. , 6";
   xlnx,pcw-fclk0-peripheral-clksrc = "IO , PLL";
   xlnx,pcw-nand-cycles-t-clr = <1>;
   xlnx,pcw-pll-bypassmode-enable = <0>;
   xlnx,s-axi-acp-aruser-val = <31>;
   xlnx,pcw-io-io-pll-freqmhz = <1000>;
   xlnx,pcw-mio-22-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-mio-51-pullup = "enabled";
   xlnx,pcw-p2f-sdio1-intr = <0>;
   xlnx,pcw-qspi-grp-fbclk-io = "MIO , 8";
   xlnx,use-s-axi-gp0 = <0>;
   xlnx,use-s-axi-gp1 = <0>;
   xlnx,pcw-i2c0-peripheral-enable = <1>;
   xlnx,pcw-uiparam-ddr-dq-2-length-mm = <0>;
   xlnx,pcw-nor-grp-sram-cs0-io = "<Select>";
   xlnx,pcw-enet1-grp-mdio-io = "<Select>";
   xlnx,pcw-enet0-peripheral-enable = <1>;
   xlnx,pcw-p2f-usb0-intr = <0>;
   xlnx,pcw-enet1-reset-io = "<Select>";
   xlnx,pcw-mio-46-direction = "out";
   xlnx,pcw-spi1-grp-ss2-enable = <0>;
   xlnx,pcw-nor-grp-a25-io = "<Select>";
   xlnx,pcw-en-emio-can0 = <0>;
   xlnx,pcw-core1-fiq-intr = <0>;
   xlnx,s-axi-hp2-id-width = <6>;
   xlnx,pcw-en-emio-can1 = <0>;
   xlnx,pcw-mio-2-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-use-dma0 = <0>;
   xlnx,pcw-can1-grp-clk-enable = <0>;
   xlnx,pcw-mio-38-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-use-dma1 = <0>;
   xlnx,pcw-use-dma2 = <0>;
   xlnx,pcw-use-dma3 = <0>;
   xlnx,pcw-usb-reset-select = "Share , reset , pin";
   xlnx,pcw-mio-7-slew = "slow";
   xlnx,pcw-ttc1-peripheral-enable = <0>;
   xlnx,use-axi-nonsecure = <0>;
   xlnx,pcw-sd0-grp-pow-enable = <0>;
   xlnx,pcw-mio-4-slew = "slow";
   xlnx,pcw-mio-25-pullup = "enabled";
   xlnx,pcw-uiparam-ddr-dqs-0-propogation-delay = <160>;
   xlnx,pcw-mio-1-slew = "slow";
   xlnx,pcw-trace-grp-32bit-io = "<Select>";
   xlnx,pcw-act-ttc1-clk2-peripheral-freqmhz = <0x69f6bcb>;
   xlnx,pcw-spi-peripheral-valid = <0>;
   xlnx,pcw-mio-15-direction = "inout";
   xlnx,pcw-act-enet1-peripheral-freqmhz = <10>;
   xlnx,pcw-en-ptp-enet0 = <0>;
   xlnx,pcw-mio-13-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-en-ptp-enet1 = <0>;
   xlnx,pcw-mio-5-pullup = "disabled";
   xlnx,pcw-en-clk3-port = <0>;
   xlnx,pcw-mio-42-pullup = "enabled";
   xlnx,pcw-sd1-grp-wp-io = "<Select>";
   xlnx,pcw-fpga-fclk3-enable = <0>;
   xlnx,pcw-mio-2-direction = "inout";
   xlnx,pcw-p2f-dmac6-intr = <0>;
   xlnx,pcw-en-clk0-port = <1>;
   xlnx,pcw-s-axi-gp1-id-width = <6>;
   xlnx,pcw-p2f-dmac3-intr = <0>;
   xlnx,s-axi-acp-awuser-val = <31>;
   xlnx,pcw-uiparam-ddr-clock-2-length-mm = <0>;
   xlnx,pcw-p2f-dmac0-intr = <0>;
   xlnx,pcw-uiparam-ddr-bank-addr-count = <3>;
   xlnx,edk-iptype = "PERIPHERAL";
   xlnx,pcw-en-emio-gpio = <1>;
   xlnx,pcw-act-apu-peripheral-freqmhz = <0x27bc86bf>;
   xlnx,pcw-trace-internal-width = <2>;
   xlnx,pcw-mio-25-direction = "in";
   xlnx,pcw-act-fpga2-peripheral-freqmhz = <10>;
   xlnx,pcw-uiparam-ddr-row-addr-count = <15>;
   xlnx,pcw-trace-grp-32bit-enable = <0>;
   xlnx,pcw-nor-cs0-t-pc = <1>;
   xlnx,en-emio-enet0 = <0>;
   xlnx,pcw-mio-29-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-mio-30-iotype = "LVCMOS , 1.8V";
   xlnx,en-emio-enet1 = <0>;
   xlnx,pcw-nor-cs0-we-time = <0>;
   xlnx,use-s-axi-hp0 = <1>;
   xlnx,use-s-axi-hp1 = <0>;
   xlnx,use-s-axi-hp2 = <1>;
   xlnx,use-s-axi-hp3 = <0>;
   xlnx,pcw-enet1-peripheral-clksrc = "IO , PLL";
   xlnx,pcw-s-axi-hp1-id-width = <6>;
   xlnx,pcw-uiparam-ddr-dq-3-package-length = <0x6329028>;
   xlnx,pcw-mio-16-pullup = "enabled";
   xlnx,pcw-trace-trace-io = "<Select>";
   xlnx,pcw-use-trace = <0>;
   xlnx,pcw-cpu-cpu-6x4x-max-range = <667>;
   xlnx,pcw-p2f-enet1-intr = <0>;
   xlnx,pcw-nor-cs0-t-rc = <11>;
   xlnx,pcw-single-qspi-data-mode = "x4";
   xlnx,pcw-uiparam-ddr-dqs-0-length-mm = <0>;
   xlnx,pcw-uiparam-ddr-dqs-0-package-length = <0x6430700>;
   xlnx,pcw-mio-35-direction = "inout";
   xlnx,pcw-gpio-mio-gpio-enable = <1>;
   xlnx,pcw-spi0-grp-ss0-enable = <0>;
   xlnx,fclk-clk0-buf;
   xlnx,pcw-mio-9-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-m-axi-gp0-enable-static-remap = <0>;
   xlnx,pcw-mio-46-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-nand-cycles-t-rea = <1>;
   xlnx,pcw-act-usb1-peripheral-freqmhz = <60>;
   compatible = "xlnx,processing-system7-5.5";
   xlnx,pcw-fclk3-peripheral-clksrc = "IO , PLL";
   xlnx,pcw-preset-bank0-voltage = "LVCMOS , 3.3V";
   xlnx,pcw-nor-grp-cs0-enable = <0>;
   xlnx,pcw-mio-33-pullup = "enabled";
   xlnx,pcw-trace-pipeline-width = <8>;
   xlnx,pcw-ftm-cti-in0 = "<Select>";
   xlnx,pcw-ftm-cti-in1 = "<Select>";
   xlnx,pcw-ftm-cti-in2 = "<Select>";
   xlnx,pcw-ftm-cti-in3 = "<Select>";
   xlnx,pcw-uiparam-ddr-dq-1-length-mm = <0>;
   xlnx,pcw-mio-45-direction = "inout";
   xlnx,pcw-uart-peripheral-divisor0 = <10>;
   xlnx,use-m-axi-gp0 = <1>;
   xlnx,pcw-mio-primitive = <54>;
   xlnx,pcw-ddr-priority-readport-0 = "<Select>";
   xlnx,use-m-axi-gp1 = <0>;
   xlnx,pcw-ddr-priority-readport-1 = "<Select>";
   xlnx,pcw-ddr-priority-readport-2 = "<Select>";
   xlnx,pcw-en-pjtag = <0>;
   xlnx,pcw-fclk0-peripheral-divisor0 = <5>;
   xlnx,pcw-nor-cs0-t-tr = <1>;
   xlnx,pcw-ddr-priority-readport-3 = "<Select>";
   xlnx,pcw-fclk0-peripheral-divisor1 = <2>;
   xlnx,pcw-gpio-emio-gpio-io = <1>;
   xlnx,pcw-smc-peripheral-valid = <0>;
   xlnx,pcw-uart1-peripheral-enable = <1>;
   xlnx,pcw-act-smc-peripheral-freqmhz = <10>;
   xlnx,pcw-m-axi-gp1-enable-static-remap = <0>;
   xlnx,pcw-mio-21-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-mio-49-pullup = "enabled";
   xlnx,pcw-mio-50-pullup = "enabled";
   xlnx,pcw-p2f-cti-intr = <0>;
   xlnx,pcw-mio-tree-signals = "gpio[0]#qspi0_ss_b#qspi0_io[0]#qspi0_io[1]#qspi0_io[2]#qspi0_io[3]/HOLD_B#qspi0_sclk#gpio[7]#qspi_fbclk#gpio[9]#gpio[10]#gpio[11]#gpio[12]#gpio[13]#gpio[14]#gpio[15]#tx_clk#txd[0]#txd[1]#txd[2]#txd[3]#tx_ctl#rx_clk#rxd[0]#rxd[1]#rxd[2]#rxd[3]#rx_ctl#data[4]#dir#stp#nxt#data[0]#data[1]#data[2]#data[3]#clk#data[5]#data[6]#data[7]#clk#cmd#data[0]#data[1]#data[2]#data[3]#reset#gpio[47]#tx#rx#gpio[50]#gpio[51]#mdc#mdio";
   xlnx,pcw-nor-cs0-t-wc = <11>;
   xlnx,pcw-p2f-smc-intr = <0>;
   xlnx,pcw-qspi-grp-ss1-enable = <0>;
   xlnx,pcw-s-axi-hp2-freqmhz = <150>;
   xlnx,pcw-spi0-grp-ss2-io = "<Select>";
   xlnx,pcw-uiparam-ddr-col-addr-count = <10>;
   xlnx,pcw-ddr-lpr-to-critical-priority-level = <2>;
   xlnx,pcw-armpll-ctrl-fbdiv = <40>;
   xlnx,pcw-spi1-grp-ss1-enable = <0>;
   xlnx,pcw-en-emio-pjtag = <0>;
   xlnx,pcw-uiparam-ddr-high-temp = "Normal , (0-85)";
   xlnx,pcw-uiparam-ddr-dqs-3-propogation-delay = <160>;
   xlnx,pcw-nor-cs0-t-wp = <1>;
   xlnx,pcw-trace-grp-2bit-enable = <0>;
   xlnx,pcw-ttc0-clk2-peripheral-freqmhz = <0x7f28155>;
   xlnx,pcw-mio-1-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-mio-37-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-use-cross-trigger = <0>;
   xlnx,pcw-mio-14-direction = "inout";
   xlnx,pcw-s-axi-acp-freqmhz = <10>;
   xlnx,pcw-fclk1-peripheral-divisor0 = <1>;
   xlnx,pcw-fclk1-peripheral-divisor1 = <1>;
   xlnx,pcw-ttc0-ttc0-io = "<Select>";
   xlnx,pcw-spi1-grp-ss2-io = "<Select>";
   xlnx,pcw-ttc1-clk2-peripheral-clksrc = "CPU_1X";
   xlnx,pcw-sd0-grp-wp-io = "<Select>";
   xlnx,pcw-i2c-reset-polarity = "Active , Low";
   xlnx,pcw-mio-24-pullup = "enabled";
   xlnx,pcw-ddr-hprlpr-queue-partition = "HPR(0)/LPR(32)";
   xlnx,pcw-apu-peripheral-freqmhz = <0x27bc86aa>;
   xlnx,pcw-mio-1-direction = "out";
   xlnx,pcw-tpiu-peripheral-clksrc = "External";
   xlnx,pcw-core0-irq-intr = <0>;
   xlnx,pcw-uiparam-ddr-memory-type = "DDR , 3 , (Low , Voltage)";
   xlnx,pcw-uiparam-ddr-clock-1-length-mm = <0>;
   xlnx,s-axi-acp-id-width = <3>;
   xlnx,pcw-mio-24-direction = "in";
   xlnx,pcw-gpio-emio-gpio-width = <1>;
   xlnx,pcw-uiparam-ddr-clock-1-propogation-delay = <160>;
   xlnx,irq-f2p-mode = "DIRECT";
   xlnx,pcw-mio-12-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-mio-4-pullup = "disabled";
   xlnx,pcw-mio-41-pullup = "enabled";
   xlnx,pcw-clk3-freq = <10000000>;
   xlnx,pcw-can1-peripheral-enable = <0>;
   xlnx,pcw-fclk2-peripheral-divisor0 = <1>;
   xlnx,pcw-mio-51-slew = "slow";
   xlnx,pcw-fclk2-peripheral-divisor1 = <1>;
   xlnx,pcw-fpga-fclk2-enable = <0>;
   xlnx,pcw-clk0-freq = <100000000>;
   xlnx,pcw-mio-47-slew = "slow";
   xlnx,pcw-sdio-peripheral-divisor0 = <10>;
   xlnx,pcw-use-expanded-iop = <0>;
   xlnx,pcw-mio-44-slew = "slow";
   xlnx,pcw-qspi-peripheral-divisor0 = <5>;
   xlnx,pcw-gp0-num-read-threads = <4>;
   xlnx,pcw-en-rst3-port = <0>;
   xlnx,pcw-mio-34-direction = "inout";
   xlnx,pcw-mio-41-slew = "slow";
   xlnx,pcw-tpiu-peripheral-divisor0 = <1>;
   xlnx,pcw-en-rst0-port = <1>;
   xlnx,pcw-mio-28-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-mio-37-slew = "slow";
   xlnx,pcw-sd1-grp-pow-enable = <0>;
   xlnx,pcw-mio-34-slew = "slow";
   xlnx,pcw-en-emio-sram-int = <0>;
   xlnx,pcw-uiparam-ddr-dqs-to-clk-delay-0 = <0xfffee2d8>;
   xlnx,pcw-uiparam-ddr-dqs-to-clk-delay-1 = <0xffff7b30>;
   xlnx,pcw-uiparam-ddr-dqs-to-clk-delay-2 = <0xffff8ad0>;
   xlnx,pcw-uiparam-ddr-dqs-to-clk-delay-3 = <0xfffebfb0>;
   xlnx,pcw-mio-31-slew = "slow";
   xlnx,pcw-sd1-grp-wp-enable = <0>;
   xlnx,pcw-mio-15-pullup = "enabled";
   xlnx,pcw-fclk3-peripheral-divisor0 = <1>;
   xlnx,pcw-fclk3-peripheral-divisor1 = <1>;
   xlnx,pcw-uiparam-ddr-dq-0-length-mm = <0>;
   xlnx,pcw-mio-27-slew = "slow";
   xlnx,pcw-smc-peripheral-freqmhz = <100>;
   xlnx,pcw-num-f2p-intr-inputs = <3>;
   xlnx,pcw-wdt-wdt-io = "<Select>";
   xlnx,pcw-uiparam-ddr-dq-0-propogation-delay = <160>;
   xlnx,pcw-mio-24-slew = "slow";
   xlnx,pcw-mio-44-direction = "inout";
   xlnx,pcw-fpga0-peripheral-freqmhz = <100>;
   xlnx,pcw-mio-8-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-iopll-ctrl-fbdiv = <30>;
   xlnx,pcw-mio-45-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-p2f-can1-intr = <0>;
   xlnx,pcw-ddr-priority-writeport-0 = "<Select>";
   xlnx,pcw-i2c0-reset-io = "<Select>";
   xlnx,pcw-ddr-priority-writeport-1 = "<Select>";
   xlnx,pcw-ddr-priority-writeport-2 = "<Select>";
   xlnx,pcw-mio-21-slew = "slow";
   xlnx,pcw-ddr-priority-writeport-3 = "<Select>";
   xlnx,pcw-en-can0 = <0>;
   xlnx,pcw-mio-17-slew = "slow";
   xlnx,pcw-en-can1 = <0>;
   xlnx,pcw-fpga3-peripheral-freqmhz = <50>;
   xlnx,pcw-trace-grp-16bit-io = "<Select>";
   xlnx,pcw-cpu-peripheral-divisor0 = <2>;
   xlnx,pcw-m-axi-gp1-support-narrow-burst = <0>;
   xlnx,pcw-mio-32-pullup = "enabled";
   xlnx,pcw-usb0-peripheral-enable = <1>;
   xlnx,pcw-mio-14-slew = "slow";
   xlnx,pcw-uiparam-ddr-train-read-gate = <1>;
   xlnx,gp1-en-modifiable-txn = <0>;
   xlnx,name = "processing_system7_0";
   xlnx,pcw-act-spi-peripheral-freqmhz = <10>;
   xlnx,pcw-mio-11-slew = "slow";
   xlnx,pcw-ttc1-clk1-peripheral-freqmhz = <0x7f28155>;
   xlnx,pcw-en-emio-modem-uart0 = <0>;
   xlnx,pcw-dq-width = <32>;
   xlnx,pcw-en-emio-modem-uart1 = <0>;
   xlnx,pcw-sd0-peripheral-enable = <1>;
   xlnx,pcw-nor-grp-cs0-io = "<Select>";
   xlnx,pcw-can-peripheral-valid = <0>;
   xlnx,pcw-m-axi-gp1-id-width = <12>;
   xlnx,pcw-ttc0-clk0-peripheral-clksrc = "CPU_1X";
   xlnx,pcw-act-ttc0-clk1-peripheral-freqmhz = <0x69f6bcb>;
   xlnx,pcw-mio-19-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-mio-20-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-wdt-peripheral-enable = <0>;
   xlnx,pcw-mio-13-direction = "inout";
   xlnx,pcw-enet1-grp-mdio-enable = <0>;
   xlnx,pcw-mio-48-pullup = "enabled";
   xlnx,pcw-sdio-peripheral-clksrc = "IO , PLL";
   xlnx,pcw-p2f-dmac-abort-intr = <0>;
   xlnx,pcw-nor-peripheral-enable = <0>;
   xlnx,pcw-en-sdio0 = <1>;
   xlnx,pcw-ddr-peripheral-divisor0 = <2>;
   xlnx,pcw-en-sdio1 = <0>;
   xlnx,pcw-mio-0-direction = "inout";
   xlnx,pcw-uiparam-ddr-dqs-1-package-length = <0x3fcdfc0>;
   xlnx,pcw-uiparam-ddr-enable = <1>;
   xlnx,pcw-spi1-grp-ss0-enable = <0>;
   xlnx,pcw-usb-reset-polarity = "Active , Low";
   xlnx,pcw-fclk-clk2-buf;
   xlnx,pcw-p2f-uart0-intr = <0>;
   xlnx,pcw-uiparam-ddr-t-rcd = <7>;
   xlnx,ps7-si-rev = "PRODUCTION";
   xlnx,pcw-mio-0-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-uiparam-ddr-clock-0-length-mm = <0>;
   xlnx,pcw-mio-36-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-act-can1-peripheral-freqmhz = <0x16b4ddc>;
   xlnx,pcw-cpu-cpu-pll-freqmhz = <0x4f790c08>;
   xlnx,pcw-mio-23-direction = "in";
   xlnx,pcw-uiparam-ddr-train-write-level = <1>;
   xlnx,pcw-ddr-write-to-critical-priority-level = <2>;
   xlnx,pcw-nand-peripheral-enable = <0>;
   xlnx,pcw-en-gpio = <1>;
   xlnx,pcw-spi0-spi0-io = "<Select>";
   xlnx,pcw-use-high-ocm = <0>;
   xlnx,pcw-en-emio-sdio0 = <0>;
   xlnx,pcw-mio-23-pullup = "enabled";
   xlnx,pcw-en-emio-sdio1 = <0>;
   xlnx,pcw-gp1-en-modifiable-txn = <0>;
   xlnx,pcw-mio-9-direction = "inout";
   xlnx,pcw-uart-peripheral-clksrc = "IO , PLL";
   xlnx,pcw-en-clktrig3-port = <0>;
   xlnx,pcw-qspi-peripheral-clksrc = "IO , PLL";
   xlnx,pcw-uiparam-ddr-board-delay0 = <0x2af80>;
   xlnx,pcw-uiparam-ddr-board-delay1 = <0x26d18>;
   xlnx,pcw-uiparam-ddr-board-delay2 = <0x278d0>;
   xlnx,pcw-mio-53-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-uiparam-ddr-board-delay3 = <0x2da78>;
   xlnx,pcw-en-clktrig0-port = <0>;
   xlnx,pcw-dm-width = <4>;
   xlnx,pcw-i2c0-i2c0-io = "EMIO";
   xlnx,pcw-mio-33-direction = "inout";
   xlnx,pcw-mio-11-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-mio-3-pullup = "disabled";
   xlnx,fclk-clk3-buf;
   xlnx,pcw-mio-39-pullup = "enabled";
   xlnx,pcw-mio-40-pullup = "enabled";
   xlnx,pcw-uart0-grp-full-io = "<Select>";
   xlnx,pcw-fpga-fclk1-enable = <0>;
   xlnx,pcw-ttc0-peripheral-enable = <0>;
   xlnx,pcw-uiparam-ddr-t-rc = <0x2e7ddb0>;
   xlnx,pcw-uiparam-ddr-adv-enable = <0>;
   xlnx,pcw-i2c-reset-enable = <0>;
   xlnx,pcw-s-axi-hp1-freqmhz = <10>;
   xlnx,use-default-acp-user-val = <0>;
   xlnx,pcw-act-i2c-peripheral-freqmhz = <50>;
   xlnx,pcw-uiparam-ddr-t-faw = <40>;
   xlnx,preset = "None";
   xlnx,pcw-use-proc-event-bus = <0>;
   xlnx,pcw-uiparam-ddr-t-rp = <7>;
   xlnx,pcw-p2f-sdio0-intr = <0>;
   xlnx,pcw-mio-43-direction = "inout";
   xlnx,s-axi-hp3-id-width = <6>;
   xlnx,pcw-mio-27-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-uiparam-ddr-dq-3-propogation-delay = <160>;
   xlnx,pcw-p2f-spi1-intr = <0>;
   xlnx,pcw-spi-peripheral-freqmhz = <0x9ef21aa>;
   xlnx,m-axi-gp0-thread-id-width = <12>;
   xlnx,s-axi-gp0-id-width = <6>;
   xlnx,pcw-mio-14-pullup = "enabled";
   xlnx,pcw-mio-9-slew = "slow";
   xlnx,m-axi-gp0-enable-static-remap = <0>;
   xlnx,pcw-mio-53-direction = "inout";
   xlnx,pcw-mio-6-slew = "slow";
   xlnx,pcw-mio-7-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-mio-44-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-use-fabric-interrupt = <1>;
   xlnx,pcw-act-ttc1-clk0-peripheral-freqmhz = <0x69f6bcb>;
   xlnx,pcw-mio-3-slew = "slow";
   xlnx,pcw-sd1-grp-cd-io = "<Select>";
   xlnx,pcw-use-s-axi-gp0 = <0>;
   xlnx,pcw-use-s-axi-gp1 = <0>;
   xlnx,pcw-en-usb0 = <1>;
   xlnx,pcw-mio-0-slew = "slow";
   xlnx,pcw-nor-grp-sram-int-io = "<Select>";
   xlnx,pcw-en-usb1 = <0>;
   xlnx,pcw-mio-12-direction = "inout";
   xlnx,pcw-pjtag-peripheral-enable = <0>;
   xlnx,pcw-pcap-peripheral-clksrc = "IO , PLL";
   xlnx,pcw-m-axi-gp1-thread-id-width = <12>;
   xlnx,pcw-mio-31-pullup = "enabled";
   xlnx,pcw-usb-reset-enable = <1>;
   xlnx,pcw-mio-tree-peripherals = "GPIO#Quad , SPI , Flash#Quad , SPI , Flash#Quad , SPI , Flash#Quad , SPI , Flash#Quad , SPI , Flash#Quad , SPI , Flash#GPIO#Quad , SPI , Flash#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#Enet , 0#Enet , 0#Enet , 0#Enet , 0#Enet , 0#Enet , 0#Enet , 0#Enet , 0#Enet , 0#Enet , 0#Enet , 0#Enet , 0#USB , 0#USB , 0#USB , 0#USB , 0#USB , 0#USB , 0#USB , 0#USB , 0#USB , 0#USB , 0#USB , 0#USB , 0#SD , 0#SD , 0#SD , 0#SD , 0#SD , 0#SD , 0#USB , Reset#GPIO#UART , 1#UART , 1#GPIO#GPIO#Enet , 0#Enet , 0";
   xlnx,s-axi-hp0-id-width = <6>;
   xlnx,include-trace-buffer = <0>;
   xlnx,pcw-use-cr-fabric = <1>;
   xlnx,pcw-enet0-peripheral-clksrc = "IO , PLL";
   xlnx,package-name = "clg400";
   xlnx,m-axi-gp1-enable-static-remap = <0>;
   xlnx,pcw-en-clk2-port = <0>;
   xlnx,pcw-p2f-dmac5-intr = <0>;
   xlnx,pcw-mio-18-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-mio-22-direction = "in";
   xlnx,pcw-mio-47-pullup = "enabled";
   xlnx,pcw-apu-clk-ratio-enable = "6:2:1";
   xlnx,pcw-act-fpga0-peripheral-freqmhz = <100>;
   xlnx,pcw-p2f-dmac2-intr = <0>;
   xlnx,pcw-fclk2-peripheral-clksrc = "IO , PLL";
   xlnx,rable = <1>;
   xlnx,pcw-usb1-usb1-io = "<Select>";
   xlnx,ip-name = "processing_system7";
   xlnx,pcw-mio-8-direction = "out";
   xlnx,pcw-s-axi-hp2-id-width = <6>;
   xlnx,pcw-act-fpga3-peripheral-freqmhz = <10>;
   xlnx,pcw-qspi-grp-fbclk-enable = <1>;
   xlnx,pcw-qspi-grp-io1-io = "<Select>";
   xlnx,pcw-trace-buffer-fifo-size = <128>;
   xlnx,pcw-use-trace-data-edge-detector = <0>;
   xlnx,pcw-mio-35-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-sd1-sd1-io = "<Select>";
   xlnx,pcw-en-enet0 = <1>;
   xlnx,pcw-en-enet1 = <0>;
   xlnx,pcw-mio-32-direction = "inout";
   xlnx,pcw-sd0-grp-wp-enable = <0>;
   xlnx,include-acp-trans-check = <0>;
   xlnx,pcw-i2c-peripheral-freqmhz = <0x69f6bcb>;
   xlnx,pcw-uart0-peripheral-enable = <0>;
   xlnx,pcw-p2f-enet0-intr = <0>;
   xlnx,pcw-spi0-grp-ss1-io = "<Select>";
   xlnx,pcw-mio-22-pullup = "enabled";
   xlnx,pcw-can0-can0-io = "<Select>";
   xlnx,pcw-s-axi-gp1-freqmhz = <10>;
   xlnx,pcw-en-ddr = <1>;
   xlnx,pcw-qspi-internal-highaddress = <0xfcffffff>;
   xlnx,pcw-mio-52-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-en-emio-enet0 = <0>;
   xlnx,pcw-uiparam-ddr-clock-0-package-length = <0x4cb9f7c>;
   xlnx,pcw-en-emio-enet1 = <0>;
   xlnx,pcw-uiparam-ddr-dqs-2-propogation-delay = <160>;
   xlnx,pcw-mio-42-direction = "inout";
   xlnx,pcw-use-s-axi-hp0 = <1>;
   xlnx,pcw-use-s-axi-hp1 = <0>;
   xlnx,pcw-enet0-peripheral-divisor0 = <8>;
   xlnx,pcw-mio-10-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-use-s-axi-hp2 = <1>;
   xlnx,pcw-enet0-peripheral-divisor1 = <1>;
   xlnx,pcw-mio-2-pullup = "disabled";
   xlnx,pcw-use-s-axi-hp3 = <0>;
   xlnx,pcw-mio-38-pullup = "enabled";
   xlnx,pcw-spi1-grp-ss1-io = "<Select>";
   xlnx,pcw-fpga-fclk0-enable = <1>;
   xlnx,pcw-uiparam-ddr-dqs-2-package-length = <0x550a62c>;
   xlnx,pcw-p2f-qspi-intr = <0>;
   xlnx,pcw-ttc1-clk1-peripheral-clksrc = "CPU_1X";
   xlnx,pcw-gp1-num-write-threads = <4>;
   xlnx,pcw-fclk-clk0-buf;
   xlnx,pcw-uiparam-ddr-use-internal-vref = <0>;
   xlnx,trace-buffer-clock-delay = <12>;
   xlnx,pcw-en-4k-timer = <0>;
   xlnx,pcw-p2f-i2c1-intr = <0>;
   xlnx,pcw-mio-52-direction = "out";
   xlnx,pcw-override-basic-clock = <0>;
   xlnx,pcw-mio-26-iotype = "LVCMOS , 1.8V";
   xlnx,dq-width = <32>;
   xlnx,pcw-s-axi-hp0-data-width = <64>;
   xlnx,pcw-en-emio-i2c0 = <1>;
   xlnx,pcw-ttc0-clk0-peripheral-freqmhz = <0x7f28155>;
   xlnx,pcw-en-emio-i2c1 = <0>;
   xlnx,use-s-axi-acp = <0>;
   xlnx,pcw-sd0-grp-cd-io = "EMIO";
   xlnx,pcw-uiparam-ddr-clock-0-propogation-delay = <160>;
   xlnx,pcw-pcap-peripheral-freqmhz = <200>;
   xlnx,pcw-act-dci-peripheral-freqmhz = <0x9b028a>;
   xlnx,pcw-mio-11-direction = "inout";
   xlnx,pcw-enet1-peripheral-divisor0 = <1>;
   xlnx,pcw-use-default-acp-user-val = <0>;
   xlnx,pcw-ddrpll-ctrl-fbdiv = <32>;
   xlnx,pcw-enet1-peripheral-divisor1 = <1>;
   xlnx,pcw-mio-13-pullup = "enabled";
   xlnx,pcw-ps7-si-rev = "PRODUCTION";
   xlnx,pcw-can0-peripheral-enable = <0>;
   xlnx,pcw-use-m-axi-gp0 = <1>;
   xlnx,pcw-en-emio-cd-sdio0 = <1>;
   xlnx,pcw-uart0-uart0-io = "<Select>";
   xlnx,pcw-use-m-axi-gp1 = <0>;
   xlnx,pcw-en-emio-cd-sdio1 = <0>;
   xlnx,pcw-core1-irq-intr = <0>;
   xlnx,pcw-mio-6-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-mio-43-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-i2c1-reset-enable = <0>;
   xlnx,pcw-s-axi-hp1-data-width = <64>;
   xlnx,pcw-trace-grp-8bit-enable = <0>;
   xlnx,dqs-width = <4>;
   xlnx,fclk-clk1-buf;
   xlnx,pcw-mio-21-direction = "out";
   xlnx,pcw-mio-29-pullup = "enabled";
   xlnx,pcw-mio-30-pullup = "enabled";
   xlnx,trace-internal-width = <2>;
   xlnx,pcw-s-axi-hp0-freqmhz = <150>;
   xlnx,pcw-mio-7-direction = "out";
   xlnx,pcw-i2c0-reset-enable = <0>;
   xlnx,pcw-mio-53-slew = "slow";
   xlnx,pcw-sdio-peripheral-freqmhz = <100>;
   xlnx,pcw-uart1-uart1-io = "MIO , 48 , .. , 49";
   xlnx,pcw-dqs-width = <4>;
   xlnx,pcw-s-axi-hp2-data-width = <64>;
   xlnx,pcw-clk2-freq = <10000000>;
   xlnx,pcw-mio-49-slew = "slow";
   xlnx,pcw-mio-50-slew = "slow";
   xlnx,pcw-mio-17-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-en-emio-ttc0 = <0>;
   xlnx,pcw-mio-9-pullup = "enabled";
   xlnx,pcw-en-emio-ttc1 = <0>;
   xlnx,pcw-mio-46-pullup = "enabled";
   xlnx,pcw-mio-31-direction = "in";
   xlnx,pcw-mio-46-slew = "slow";
   xlnx,pcw-sd1-grp-cd-enable = <0>;
   xlnx,dm-width = <4>;
   xlnx,pcw-i2c0-grp-int-io = "EMIO";
   xlnx,pcw-mio-43-slew = "slow";
   xlnx,pcw-use-axi-fabric-idle = <0>;
   xlnx,pcw-en-rst2-port = <0>;
   xlnx,pcw-gpio-emio-gpio-enable = <1>;
   xlnx,pcw-mio-39-slew = "slow";
   xlnx,pcw-mio-40-slew = "slow";
   xlnx,gp0-en-modifiable-txn = <0>;
   xlnx,pcw-uart1-grp-full-io = "<Select>";
   xlnx,pcw-uart-peripheral-valid = <1>;
   xlnx,pcw-mio-36-slew = "slow";
   xlnx,pcw-uiparam-ddr-dq-0-package-length = <0x5df0958>;
   xlnx,pcw-s-axi-hp3-data-width = <64>;
   xlnx,pcw-enet0-peripheral-freqmhz = "1000 , Mbps";
   xlnx,trace-pipeline-width = <8>;
   xlnx,m-axi-gp0-id-width = <12>;
   xlnx,pcw-mio-33-slew = "slow";
   xlnx,pcw-mio-34-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-use-coresight = <0>;
   xlnx,pcw-uiparam-ddr-bus-width = "32 , Bit";
   xlnx,pcw-en-emio-spi0 = <0>;
   xlnx,pcw-en-emio-spi1 = <0>;
   xlnx,pcw-i2c1-reset-io = "<Select>";
   xlnx,pcw-mio-29-slew = "slow";
   xlnx,pcw-mio-30-slew = "slow";
   xlnx,pcw-mio-41-direction = "inout";
   xlnx,pcw-use-debug = <0>;
   xlnx,pcw-smc-peripheral-divisor0 = <1>;
   xlnx,pcw-i2c1-grp-int-io = "<Select>";
   xlnx,pcw-mio-26-slew = "slow";
   xlnx,pcw-s-axi-acp-aruser-val = <31>;
   xlnx,pcw-uiparam-ddr-t-ras-min = <35>;
   xlnx,pcw-can1-peripheral-clksrc = "External";
   xlnx,pcw-irq-f2p-mode = "DIRECT";
   xlnx,pcw-mio-21-pullup = "enabled";
   xlnx,pcw-usb0-reset-io = "MIO , 46";
   xlnx,pcw-mio-23-slew = "slow";
   xlnx,pcw-uiparam-ddr-train-data-eye = <1>;
   xlnx,pcw-p2f-can0-intr = <0>;
   xlnx,pcw-fpga1-peripheral-freqmhz = <200>;
   xlnx,pcw-s-axi-acp-id-width = <3>;
   xlnx,pcw-smc-peripheral-clksrc = "IO , PLL";
   xlnx,pcw-mio-19-slew = "slow";
   xlnx,pcw-mio-20-slew = "slow";
   xlnx,pcw-nand-grp-d8-enable = <0>;
   xlnx,pcw-nor-sram-cs1-t-pc = <1>;
   xlnx,pcw-mio-51-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-enet-reset-select = "<Select>";
   xlnx,pcw-mio-16-slew = "slow";
   xlnx,pcw-dci-peripheral-freqmhz = <0x9b0398>;
   xlnx,pcw-mio-13-slew = "slow";
   xlnx,pcw-mio-51-direction = "inout";
   xlnx,pcw-mio-1-pullup = "enabled";
   xlnx,pcw-mio-10-slew = "slow";
   xlnx,pcw-mio-37-pullup = "enabled";
   xlnx,pcw-nor-sram-cs1-t-rc = <11>;
   xlnx,pcw-ttc1-clk2-peripheral-freqmhz = <0x7f28155>;
   xlnx,pcw-mio-10-direction = "inout";
   xlnx,pcw-uiparam-ddr-clock-3-propogation-delay = <160>;
   xlnx,pcw-use-ddr-bypass = <0>;
   xlnx,pcw-en-modem-uart0 = <0>;
   xlnx,pcw-en-modem-uart1 = <0>;
   xlnx,pcw-trace-grp-16bit-enable = <0>;
   xlnx,pcw-uiparam-ddr-freq-mhz = <0x1fca0555>;
   xlnx,pcw-gp0-en-modifiable-txn = <0>;
   xlnx,pcw-nor-sram-cs1-we-time = <0>;
   xlnx,pcw-act-ttc0-clk2-peripheral-freqmhz = <0x69f6bcb>;
   xlnx,pcw-uiparam-ddr-ecc = "Disabled";
   xlnx,pcw-mio-25-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-act-pcap-peripheral-freqmhz = <200>;
   xlnx,pcw-uart1-grp-full-enable = <0>;
   xlnx,pcw-ddr-peripheral-clksrc = "DDR , PLL";
   xlnx,pcw-mio-19-direction = "out";
   xlnx,pcw-mio-20-direction = "out";
   xlnx,pcw-mio-12-pullup = "enabled";
   xlnx,en-emio-trace = <0>;
   xlnx,pcw-nor-sram-cs1-t-tr = <1>;
   xlnx,pcw-nor-nor-io = "<Select>";
   xlnx,pcw-s-axi-gp0-freqmhz = <10>;
   xlnx,pcw-spi-peripheral-divisor0 = <1>;
   xlnx,pcw-trace-buffer-clock-delay = <12>;
   xlnx,pcw-uiparam-ddr-clock-1-package-length = <0x4cb9f7c>;
   xlnx,pcw-mio-5-iotype = "LVCMOS , 3.3V";
   xlnx,pcw-mio-6-direction = "out";
   xlnx,pcw-mio-42-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-s-axi-acp-awuser-val = <31>;
   xlnx,pcw-use-expanded-ps-slcr-registers = <0>;
   xlnx,pcw-uiparam-ddr-cwl = <6>;
   xlnx,pcw-spi1-peripheral-enable = <0>;
   xlnx,pcw-nor-sram-cs1-t-wc = <11>;
   xlnx,pcw-tpiu-peripheral-freqmhz = <200>;
   xlnx,pcw-pjtag-pjtag-io = "<Select>";
   xlnx,pcw-wdt-peripheral-clksrc = "CPU_1X";
   xlnx,pcw-uiparam-ddr-dq-2-propogation-delay = <160>;
   xlnx,pcw-uiparam-ddr-dqs-3-package-length = <0x6c5db30>;
   xlnx,pcw-mio-28-pullup = "enabled";
   xlnx,pcw-mio-29-direction = "in";
   xlnx,pcw-mio-30-direction = "out";
   xlnx,pcw-en-clktrig2-port = <0>;
   xlnx,pcw-enet0-grp-mdio-enable = <1>;
   xlnx,pcw-usb0-peripheral-freqmhz = <60>;
   xlnx,pcw-nor-sram-cs1-t-wp = <1>;
   xlnx,pcw-act-ttc-peripheral-freqmhz = <50>;
   xlnx,pcw-act-sdio-peripheral-freqmhz = <100>;
   xlnx,pcw-import-board-preset = "None";
   xlnx,pcw-mio-16-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-mio-8-pullup = "disabled";
   xlnx,pcw-enet0-enet0-io = "MIO , 16 , .. , 27";
   xlnx,pcw-mio-45-pullup = "enabled";
   xlnx,pcw-mio-39-direction = "inout";
   xlnx,pcw-mio-40-direction = "inout";
   xlnx,pcw-uiparam-ddr-device-capacity = "4096 , MBits";
   xlnx,pcw-uiparam-ddr-al = <0>;
   xlnx,pcw-can-peripheral-divisor0 = <1>;
   xlnx,pcw-can-peripheral-divisor1 = <1>;
   xlnx,use-trace-data-edge-detector = <0>;
   xlnx,pcw-ttc0-clk2-peripheral-clksrc = "CPU_1X";
   xlnx,pcw-nor-cs1-t-ceoe = <1>;
   xlnx,pcw-qspi-grp-io1-enable = <0>;
   xlnx,s-axi-gp1-id-width = <6>;
   xlnx,pcw-uiparam-ddr-bl = <8>;
   xlnx,pcw-peripheral-board-preset = "part0";
   xlnx,pcw-en-emio-wp-sdio0 = <0>;
   xlnx,pcw-p2f-usb1-intr = <0>;
   xlnx,pcw-en-emio-wp-sdio1 = <0>;
   xlnx,pcw-m-axi-gp1-freqmhz = <10>;
   xlnx,pcw-p2f-spi0-intr = <0>;
   xlnx,pcw-sd0-grp-pow-io = "<Select>";
   xlnx,pcw-mio-33-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-uiparam-ddr-cl = <7>;
   xlnx,pcw-ttc1-ttc1-io = "<Select>";
   xlnx,pcw-en-smc = <0>;
   xlnx,pcw-fclk-clk3-buf;
   xlnx,pcw-mio-49-direction = "in";
   xlnx,pcw-mio-50-direction = "inout";
   xlnx,pcw-enet0-reset-io = "<Select>";
   xlnx,pcw-mio-8-slew = "slow";
   xlnx,pcw-mio-19-pullup = "enabled";
   xlnx,pcw-mio-20-pullup = "enabled";
   xlnx,pcw-enet1-enet1-io = "<Select>";
   xlnx,pcw-qspi-grp-ss1-io = "<Select>";
   xlnx,pcw-en-i2c0 = <1>;
   xlnx,pcw-dci-peripheral-divisor0 = <15>;
   xlnx,pcw-en-i2c1 = <0>;
   xlnx,pcw-dci-peripheral-divisor1 = <7>;
   xlnx,pcw-mio-5-slew = "slow";
   xlnx,pcw-core0-fiq-intr = <0>;
   xlnx,s-axi-hp1-id-width = <6>;
   xlnx,pcw-fclk1-peripheral-clksrc = "IO , PLL";
   xlnx,pcw-act-ttc1-clk1-peripheral-freqmhz = <0x69f6bcb>;
   xlnx,pcw-mio-2-slew = "slow";
   xlnx,pcw-en-qspi = <1>;
   xlnx,pcw-mio-49-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-mio-50-iotype = "LVCMOS , 1.8V";
   xlnx,pcw-sd1-grp-pow-io = "<Select>";
   xlnx,pcw-en-uart0 = <0>;
   xlnx,pcw-i2c1-peripheral-enable = <0>;
   xlnx,pcw-en-uart1 = <1>;
   xlnx,pcw-ddr-port0-hpr-enable = <0>;
   xlnx,pcw-mio-0-pullup = "enabled";
   xlnx,pcw-enet1-peripheral-enable = <0>;
   xlnx,pcw-gpio-mio-gpio-io = "MIO";
   xlnx,pcw-mio-36-pullup = "enabled";
   xlnx,pcw-spi0-grp-ss0-io = "<Select>";
  };
  ps7_pmu_0: ps7_pmu@f8891000 {
   xlnx,rable = <0>;
   compatible = "xlnx,ps7-pmu-1.00.a";
   status = "okay";
   xlnx,ip-name = "ps7_pmu";
   reg = <0xf8891000 0x1000 0xf8893000 0x1000>;
   xlnx,name = "ps7_pmu_0";
  };
  ps7_ddr_0: ps7_ddr@100000 {
   xlnx,rable = <0>;
   compatible = "xlnx,ps7-ddr-1.00.a";
   status = "okay";
   xlnx,ip-name = "ps7_ddr";
   reg = <0x00100000 0x3ff00000>;
   xlnx,name = "ps7_ddr_0";
  };
  ps7_ocmc_0: ps7_ocmc@f800c000 {
   xlnx,rable = <0>;
   compatible = "xlnx,ps7-ocmc-1.00.a";
   status = "okay";
   xlnx,ip-name = "ps7_ocmc";
   reg = <0xf800c000 0x1000>;
   xlnx,name = "ps7_ocmc_0";
  };
  ps7_gpv_0: ps7_gpv@f8900000 {
   xlnx,rable = <0>;
   compatible = "xlnx,ps7-gpv-1.00.a";
   status = "okay";
   xlnx,ip-name = "ps7_gpv";
   reg = <0xf8900000 0x100000>;
   xlnx,name = "ps7_gpv_0";
  };
  ps7_scuc_0: ps7_scuc@f8f00000 {
   xlnx,rable = <0>;
   compatible = "xlnx,ps7-scuc-1.00.a";
   status = "okay";
   xlnx,ip-name = "ps7_scuc";
   reg = <0xf8f00000 0xfd>;
   xlnx,name = "ps7_scuc_0";
  };
  ps7_iop_bus_config_0: ps7_iop_bus_config@e0200000 {
   xlnx,rable = <0>;
   compatible = "xlnx,ps7-iop-bus-config-1.00.a";
   status = "okay";
   xlnx,ip-name = "ps7_iop_bus_config";
   reg = <0xe0200000 0x1000>;
   xlnx,name = "ps7_iop_bus_config_0";
  };
  ps7_ram_0: ps7_ram@0 {
   xlnx,rable = <0>;
   compatible = "xlnx,ps7-ram-1.00.a";
   status = "okay";
   xlnx,ip-name = "ps7_ram";
   reg = <0x00000000 0x30000 0x00000000 0x40000>;
   xlnx,name = "ps7_ram_0";
  };
  ps7_ram_1: ps7_ram@ffff0000 {
   xlnx,rable = <0>;
   compatible = "xlnx,ps7-ram-1.00.a";
   status = "okay";
   xlnx,ip-name = "ps7_ram";
   reg = <0xffff0000 0xfe00>;
   xlnx,name = "ps7_ram_1";
  };
  ps7_dma_ns: ps7_dma@f8004000 {
   xlnx,rable = <0>;
   compatible = "xlnx,ps7-dma-1.00.a";
   status = "okay";
   xlnx,ip-name = "ps7_dma";
   xlnx,is-secure;
   reg = <0xf8004000 0x1000>;
   xlnx,name = "ps7_dma_ns";
  };
  ps7_afi_0: ps7_afi@f8008000 {
   xlnx,rable = <0>;
   compatible = "xlnx,ps7-afi-1.00.a";
   status = "okay";
   xlnx,ip-name = "ps7_afi";
   reg = <0xf8008000 0x1000>;
   xlnx,name = "ps7_afi_0";
  };
  ps7_afi_1: ps7_afi@f8009000 {
   xlnx,rable = <0>;
   compatible = "xlnx,ps7-afi-1.00.a";
   status = "okay";
   xlnx,ip-name = "ps7_afi";
   reg = <0xf8009000 0x1000>;
   xlnx,name = "ps7_afi_1";
  };
  ps7_afi_2: ps7_afi@f800a000 {
   xlnx,rable = <0>;
   compatible = "xlnx,ps7-afi-1.00.a";
   status = "okay";
   xlnx,ip-name = "ps7_afi";
   reg = <0xf800a000 0x1000>;
   xlnx,name = "ps7_afi_2";
  };
  ps7_afi_3: ps7_afi@f800b000 {
   xlnx,rable = <0>;
   compatible = "xlnx,ps7-afi-1.00.a";
   status = "okay";
   xlnx,ip-name = "ps7_afi";
   reg = <0xf800b000 0x1000>;
   xlnx,name = "ps7_afi_3";
  };
  ps7_m_axi_gp0: ps7_m_axi_gp@40000000 {
   xlnx,rable = <0>;
   compatible = "xlnx,ps7-m-axi-gp-1.00.a";
   status = "okay";
   xlnx,ip-name = "ps7_m_axi_gp";
   xlnx,is-hierarchy;
   reg = <0x40000000 0x40000000>;
   xlnx,name = "ps7_m_axi_gp0";
  };
  ps7_qspi_linear_0: ps7_qspi_linear@fc000000 {
   reg = <0xfc000000 0x1000000>;
  };
 };
 &uart1 {
  xlnx,rable = <0>;
  status = "okay";
  xlnx,clock-freq = <100000000>;
  xlnx,has-modem = <0>;
  xlnx,ip-name = "ps7_uart";
  cts-override;
  port-number = <0>;
  xlnx,uart-clk-freq-hz = <100000000>;
  xlnx,name = "ps7_uart_1";
 };
 &qspi {
  num-cs = <1>;
  xlnx,qspi-clk-freq-hz = <200000000>;
  xlnx,rable = <0>;
  xlnx,bus-width = <2>;
  xlnx,ip-name = "ps7_qspi";
  spi-rx-bus-width = <4>;
  xlnx,connection-mode = <0>;
  spi-tx-bus-width = <4>;
  status = "okay";
  qspi-fbclk = <0>;
  xlnx,clock-freq = <200000000>;
  xlnx,fb-clk = <1>;
  xlnx,qspi-mode = <0>;
  xlnx,name = "ps7_qspi_0";
  xlnx,qspi-bus-width = <2>;
  is-dual = <0>;
 };
 &gem0 {
  xlnx,has-mdio = <1>;
  phy-mode = "rgmii-id";
  xlnx,enet-slcr-1000mbps-div0 = <8>;
  xlnx,enet-slcr-10mbps-div0 = <8>;
  xlnx,rable = <0>;
  xlnx,enet-slcr-1000mbps-div1 = <1>;
  xlnx,enet-slcr-10mbps-div1 = <50>;
  xlnx,ip-name = "ps7_ethernet";
  xlnx,eth-mode = <1>;
  xlnx,enet-clk-freq-hz = <125000000>;
  xlnx,enet-slcr-100mbps-div0 = <8>;
  xlnx,ptp-enet-clock = <0x69f6bcb>;
  local-mac-address = [ 00 0a 23 00 00 00 ];
  status = "okay";
  xlnx,enet-slcr-100mbps-div1 = <5>;
  xlnx,name = "ps7_ethernet_0";
 };
 &usb0 {
  xlnx,rable = <0>;
  xlnx,usb-reset = "MIO , 46";
  status = "okay";
  xlnx,ip-name = "ps7_usb";
  phy_type = "ulpi";
  usb-reset = <&gpio0 46 0>;
  xlnx,name = "ps7_usb_0";
 };
 &sdhci0 {
  xlnx,rable = <0>;
  xlnx,has-power = <0>;
  status = "okay";
  xlnx,ip-name = "ps7_sdio";
  xlnx,sdio-clk-freq-hz = <100000000>;
  xlnx,has-wp = <0>;
  xlnx,has-cd = <1>;
  xlnx,name = "ps7_sd_0";
 };
 &i2c0 {
  xlnx,rable = <0>;
  xlnx,has-interrupt = <1>;
  status = "okay";
  xlnx,clock-freq = <111111115>;
  xlnx,i2c-clk-freq-hz = <111111115>;
  xlnx,ip-name = "ps7_i2c";
  xlnx,name = "ps7_i2c_0";
 };
 &gpio0 {
  xlnx,rable = <0>;
  xlnx,mio-gpio-mask = <0x5600>;
  status = "okay";
  gpio-mask-high = <0>;
  xlnx,emio-gpio-width = <1>;
  xlnx,ip-name = "ps7_gpio";
  gpio-mask-low = <22016>;
  emio-gpio-width = <0x1>;
  xlnx,name = "ps7_gpio_0";
 };
 &mc {
  xlnx,rable = <0>;
  status = "okay";
  xlnx,ddr-clk-freq-hz = <533333374>;
  xlnx,ip-name = "ps7_ddrc";
  xlnx,has-ecc = <0>;
  xlnx,name = "ps7_ddrc_0";
 };
 &devcfg {
  xlnx,rable = <0>;
  status = "okay";
  xlnx,ip-name = "ps7_dev_cfg";
  xlnx,name = "ps7_dev_cfg_0";
 };
 &adc {
  xlnx,rable = <0>;
  status = "okay";
  xlnx,ip-name = "ps7_xadc";
  xlnx,name = "ps7_xadc_0";
 };
 &coresight {
  xlnx,rable = <0>;
  status = "okay";
  xlnx,ip-name = "ps7_coresight_comp";
  xlnx,name = "ps7_coresight_comp_0";
 };
 &global_timer {
  xlnx,rable = <0>;
  status = "okay";
  xlnx,ip-name = "ps7_globaltimer";
  xlnx,name = "ps7_globaltimer_0";
 };
 &L2 {
  xlnx,rable = <0>;
  status = "okay";
  xlnx,ip-name = "ps7_pl310";
  xlnx,name = "ps7_pl310_0";
 };
 &dmac_s {
  xlnx,rable = <0>;
  status = "okay";
  xlnx,ip-name = "ps7_dma";
  xlnx,is-secure;
  xlnx,name = "ps7_dma_s";
 };
 &intc {
  xlnx,rable = <0>;
  status = "okay";
  xlnx,irq-f2p-mode = "DIRECT";
  xlnx,ip-name = "ps7_intc_dist";
  xlnx,name = "ps7_intc_dist_0";
 };
 &scutimer {
  xlnx,rable = <0>;
  status = "okay";
  xlnx,ip-name = "ps7_scutimer";
  xlnx,name = "ps7_scutimer_0";
 };
 &scuwdt {
  xlnx,rable = <0>;
  status = "okay";
  xlnx,ip-name = "ps7_scuwdt";
  xlnx,name = "ps7_scuwdt_0";
 };
 &slcr {
  xlnx,rable = <0>;
  status = "okay";
  xlnx,ip-name = "ps7_slcr";
  xlnx,name = "ps7_slcr_0";
 };
 &clkc {
  fclk-enable = <0x1>;
  ps-clk-frequency = <33333333>;
 };
# 5 "C:\\Users\\sanat\\FPGA_Car_Project\\Vitis_Vid_Pass\\vitis_vid_pass\\hw\\sdt\\system-top.dts" 2
/ {
 board = "zybo-z7-20";
 compatible = "xlnx,zybo-z7-20";
 device_id = "7z020";
 slrcount = <1>;
 family = "Zynq";
 ps7_qspi_linear_0_memory: memory@fc000000 {
  compatible = "xlnx,ps7-qspi-linear-1.00.a-memory";
  xlnx,ip-name = "ps7_qspi_linear";
  device_type = "memory";
  memory_type = "linear_flash";
  reg = <0xfc000000 0x1000000>;
 };
 ps7_ddr_0_memory: memory@00100000 {
  compatible = "xlnx,ps7-ddr-1.00.a";
  xlnx,ip-name = "ps7_ddr";
  device_type = "memory";
  memory_type = "memory";
  reg = <0x00100000 0x3FF00000>;
 };
 ps7_ram_0_memory: memory@0 {
  compatible = "xlnx,ps7-ram-1.00.a";
  xlnx,ip-name = "ps7_ram";
  device_type = "memory";
  memory_type = "memory";
  reg = <0x0 0x30000>;
 };
 ps7_ram_1_memory: memory@ffff0000 {
  compatible = "xlnx,ps7-ram-1.00.a";
  xlnx,ip-name = "ps7_ram";
  device_type = "memory";
  memory_type = "memory";
  reg = <0xffff0000 0xfe00>;
 };
 chosen {
  stdout-path = "serial0:115200n8";
 };
 aliases {
  serial0 = &uart1;
  spi0 = &qspi;
  serial1 = &coresight;
  i2c0 = &i2c0;
  ethernet0 = &gem0;
 };
 cpus_a9: cpus-a9@0 {
  compatible = "cpus,cluster";
  address-map = <0xf0000000 &amba 0xf0000000 0x10000000>,
         <0x00100000 &ps7_ddr_0_memory 0x00100000 0x3FF00000>,
         <0x0 &ps7_ram_0_memory 0x0 0x30000>,
         <0xffff0000 &ps7_ram_1_memory 0xffff0000 0xfe00>,
         <0x43000000 &axi_vdma_0 0x43000000 0x10000>,
         <0x43c00000 &video_dynclk 0x43c00000 0x10000>,
         <0x43c10000 &vtg 0x43c10000 0x10000>,
         <0x43c20000 &MIPI_D_PHY_RX_0 0x43c20000 0x10000>,
         <0x43c30000 &MIPI_CSI_2_RX_0 0x43c30000 0x10000>,
         <0x43c40000 &AXI_GammaCorrection_0 0x43c40000 0x10000>,
         <0xf8008000 &ps7_afi_0 0xf8008000 0x1000>,
         <0xf8009000 &ps7_afi_1 0xf8009000 0x1000>,
         <0xf800a000 &ps7_afi_2 0xf800a000 0x1000>,
         <0xf800b000 &ps7_afi_3 0xf800b000 0x1000>,
         <0xf8800000 &coresight 0xf8800000 0x100000>,
         <0xf8006000 &mc 0xf8006000 0x1000>,
         <0xf8007000 &devcfg 0xf8007000 0x100>,
         <0xf8004000 &ps7_dma_ns 0xf8004000 0x1000>,
         <0xf8003000 &dmac_s 0xf8003000 0x1000>,
         <0xe000b000 &gem0 0xe000b000 0x1000>,
         <0xf8f00200 &global_timer 0xf8f00200 0x100>,
         <0xe000a000 &gpio0 0xe000a000 0x1000>,
         <0xf8900000 &ps7_gpv_0 0xf8900000 0x100000>,
         <0xe0004000 &i2c0 0xe0004000 0x1000>,
         <0xf8f01000 &intc 0xf8f01000 0x1000>,
         <0xe0200000 &ps7_iop_bus_config_0 0xe0200000 0x1000>,
         <0xf8f02000 &L2 0xf8f02000 0x1000>,
         <0xf800c000 &ps7_ocmc_0 0xf800c000 0x1000>,
         <0xf8891000 &ps7_pmu_0 0xf8891000 0x1000>,
         <0xe000d000 &qspi 0xe000d000 0x1000>,
         <0xfc000000 &ps7_qspi_linear_0_memory 0xfc000000 0x1000000>,
         <0xf8f00000 &ps7_scuc_0 0xf8f00000 0xfd>,
         <0xf8f00600 &scutimer 0xf8f00600 0x20>,
         <0xf8f00620 &scuwdt 0xf8f00620 0xe0>,
         <0xe0100000 &sdhci0 0xe0100000 0x1000>,
         <0xf8000000 &slcr 0xf8000000 0x1000>,
         <0xe0001000 &uart1 0xe0001000 0x1000>,
         <0xe0002000 &usb0 0xe0002000 0x1000>,
         <0xf8007100 &adc 0xf8007100 0x21>;
  #ranges-address-cells = <0x1>;
  #ranges-size-cells = <0x1>;
 };
};
