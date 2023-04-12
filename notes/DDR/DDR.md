## 内存初始化流程

`KeyStone_common_CPU_init, KeyStone_common_device_init,  Keystone_Exception_cfg, KeyStone_main_PLL_init(100, 10, 1)` 

初始化CPU、外设、例外、时钟



`KeyStone_DDR_init(66.66667, 20, 1, &DDR_ECC_cfg)` -> KeyStone_DDR_Init.c: line 1177

设置内存，首先设置PLL，之后分型号具体设置。设置完成后测试内存访问正确性，错误则重新设置，次数小于10次。



`KeyStone_DDR_PLL_init` -> KeyStone_common.c : line 460，`KeyStone_PLL_init` : line 335

`ref_clock_Mhz`为66.6，`multiplier`为20，`divisor`为1，`output divisor`为2。

配置共9步，见PLL文档 3.5节，p13。`MAINPLLCTL0/1`寄存器见C5578文档7.7.1节，p153。

`KeyStone_PLL_init`中第5步认为存在问题，DDR3 PLL和PASS PLL中`PLLRST`位在`CTL1`中位置不同。



`C6678_EVM_DDR_Init` -> KeyStone_DDR_Init.c: line 514

解锁`KICK0/1`寄存器，修改位于Bootcfg中的`MAINPLLCTL0/1`寄存器，调用`CSL_BootCfgUnlockKicker`。注意初始化完成后需要锁定`KICK`寄存器。

调用`KeyStone_DDR_clock_phase_init`: line 434，修改寄存器0和12。设置`INVERT_CLKOUT`为1。

初始化leveling初始参数的寄存器

```c
    //initial value for leveling, 9 byte lanes for each
    /*WRLVL_INIT_RATIO*/
    // write leveling init ratio
    gpBootCfgRegs->DDR3_CONFIG_REG[2] = 0x20;
    gpBootCfgRegs->DDR3_CONFIG_REG[3] = 0x24;
    gpBootCfgRegs->DDR3_CONFIG_REG[4] = 0x3A;
    gpBootCfgRegs->DDR3_CONFIG_REG[5] = 0x38;
    gpBootCfgRegs->DDR3_CONFIG_REG[6] = 0x51;
    gpBootCfgRegs->DDR3_CONFIG_REG[7] = 0x5E;
    gpBootCfgRegs->DDR3_CONFIG_REG[8] = 0x5E;
    gpBootCfgRegs->DDR3_CONFIG_REG[9] = 0x5E;
    gpBootCfgRegs->DDR3_CONFIG_REG[10] = 0x44;

    /*GTLVL_INIT_RATIO*/
    // gate leveling init ratio
    gpBootCfgRegs->DDR3_CONFIG_REG[14] = 0xA1;
    gpBootCfgRegs->DDR3_CONFIG_REG[15] = 0x9E;
    gpBootCfgRegs->DDR3_CONFIG_REG[16] = 0xA7;
    gpBootCfgRegs->DDR3_CONFIG_REG[17] = 0xA9;
    gpBootCfgRegs->DDR3_CONFIG_REG[18] = 0xCA;
    gpBootCfgRegs->DDR3_CONFIG_REG[19] = 0xBE;
    gpBootCfgRegs->DDR3_CONFIG_REG[20] = 0xDD;
    gpBootCfgRegs->DDR3_CONFIG_REG[21] = 0xDD;
    gpBootCfgRegs->DDR3_CONFIG_REG[22] = 0xBA;
```

设置leveling（均衡）时寄存器的初值，`gpBootCfgRegs`定义为：

```C
// KeyStone_common.c line 59
CSL_BootcfgRegs * gpBootCfgRegs = (CSL_BootcfgRegs *)CSL_BOOT_CFG_REGS;


// cslr_device.h line 488
#define CSL_BOOT_CFG_REGS (0x02620000)
```







## 测试记录

#### 测试1：

使用STK中的mem_test_main.c进行测试，cmd使用工程中方案，运行提示DDR3 leveling failed，终端输出见log.txt。内存读取时序可能错误，出现了偏移。个人猜测是因为寄存器参数错误，需要重新配置。

定位至函数`KeyStone_DDR_full_leveling`，line 263，断点显示此处STATUS为0x40000064，RD和WR都有错

论坛相关：

1. [Tms320c6678 DDR3 leveling failed - 处理器论坛 - 处理器 - E2E™ 设计支持 (ti.com)](https://e2echina.ti.com/support/processors/f/processors-forum/182063/tms320c6678-ddr3-leveling-failed)

2. [Keystone Architecture DDR3 Memory Controller (Rev. E) (ti.com)](https://www.ti.com/lit/ug/sprugv8e/sprugv8e.pdf?ts=1679919377450)

3. [C6657 DDR leveling has failed, STATUS = 0x40000064 - 处理器论坛 - 处理器 - E2E™ 设计支持 (ti.com)](https://e2echina.ti.com/support/processors/f/processors-forum/188721/c6657-ddr-leveling-has-failed-status-0x40000064) 对于EVM，可以使用光盘GEL文件中的参数初始化；对于自制板，需要进一步确定参数

4. [TMS320C6678 Multicore Fixed and Floating-Point Digital Signal Processor Silicon Revision 1.0, 2.0 (Rev. H)](https://www.ti.com/lit/er/sprz334h/sprz334h.pdf?ts=1679991678209&ref_url=https%253A%252F%252Fe2echina.ti.com%252F) Advisory 9 p18，自制板考虑使用方案1

5. [KeyStone I DDR3 interface bring-up (ti.com)](https://www.ti.com/lit/ml/spracl8/spracl8.pdf?ts=1680005653002)

6. [KeyStone I DDR3 Initialization (Rev. E)](https://www.ti.com/lit/ml/sprabl2e/sprabl2e.pdf?ts=1679992076739) 写leveling必须在DRAM写之前执行

7. [TMS320C6678: DDR3 初始化中，leveling status无效 - 处理器论坛 - 处理器 - E2E™ 设计支持 (ti.com)](https://e2echina.ti.com/support/processors/f/processors-forum/211663/tms320c6678-ddr3-leveling-status) EVM芯片的版本应该是2.0

#### 测试2：

根据gel文件中的参数修改Leveling Initialization Values寄存器后，依然报上述错误。在KeyStone I DDR3 Initiaization和GEL文件中查阅，发现STK给出的代码中除此寄存器外，其余的寄存器也需要修改。

需修改的寄存器为：

1. 
