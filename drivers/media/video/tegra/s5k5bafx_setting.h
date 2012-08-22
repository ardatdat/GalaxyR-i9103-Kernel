/*
 * s5k5bafx.c - s5k5bafx sensor driver
 *
 * Copyright (C) 2010 Google Inc.
 *
 * Contributors:
 *      Rebecca Schultz Zavin <rebecca@android.com>
 *
 * Leverage OV9640.c
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

struct s5k5bafx_reg {
	u16 addr;
	u16 val;
};

static const struct s5k5bafx_reg mode_sensor_init[] = {
//****************************************/
{0xFCFC, 0xD000},
//****************************************/
//=================================================================================================
//	Name	:	5BAFX EVT Initial Setfile
//	PLL mode	:	MCLK=24MHz / SYSCLK=35MHz / PCLK=70MHz
//	FPS 	:	UXGA(1600x1200) 15fps
//	Made by :	SYS.LSI Sang-il Park
//	Date	:	2008.10.21
//	History
//	- 08.10.21 : By AhnJM,	Initial draft
//	- 08.10.27 : By HongHW, Changed CCM, AE_Target, FPS, Gamma, AFIT
//	- 08.10.30 : By ParkSI, Changed GAS LUT
//	- 08.10.31 : Hong TnP	(1031 TnP)
//	- 08.11.03 : Hong Disparity Bypass off / AWB debug. (TnP , Setting )
//	- 08.11.07 : Hong Flicker setting
//	- 08.11.12 : Hong Gamma
//	- 08.11.13 : Hong Low light Noise / NEW analog Setting
//	- 08.11.14 : Hong DNP   (Outdoor Grzone)
//	- 08.11.18 : Hong Tnp (Sketch-effect)
//	- 08.11.19 : Hong ESD Check routine
//	- 08.11.20 : Hong AWB indoor , outdoor (Box, Skin tone) & For Camcoder Previwe config1
//	- 08.11.27 : LSI & HS in LG reduce indoor AWB
//		- 09.01.22 : CSH : MIPI set file + 5BA HS set file merge
//		- 10.01.13 : Trap & Patch Update (896)
//=================================================================================================

//=================================================================================================
// ARM Go
//=================================================================================================
{0x0028, 0xD000},
{0x002A, 0x1030},
{0x0F12, 0x0000},
{0x002A, 0x0014},
{0x0F12, 0x0001},
{S5K5BAFX_TABLE_WAIT_MS, 0x0064},		//Delay

//====, 0x====}=========================================================================================
// Tra, 0xp an}d Patch 	 //  2008-11-18 10:15:41
//====, 0x====}=========================================================================================
{0x0028, 0x7000},
{0x002A, 0x1668},
{0x0F12, 0xB5FE},    //70001668
{0x0F12, 0x0007},    //7000166A
{0x0F12, 0x683C},    //7000166C
{0x0F12, 0x687E},    //7000166E
{0x0F12, 0x1DA5},    //70001670
{0x0F12, 0x88A0},    //70001672
{0x0F12, 0x2800},    //70001674
{0x0F12, 0xD00B},    //70001676
{0x0F12, 0x88A8},    //70001678
{0x0F12, 0x2800},    //7000167A
{0x0F12, 0xD008},    //7000167C
{0x0F12, 0x8820},    //7000167E
{0x0F12, 0x8829},    //70001680
{0x0F12, 0x4288},    //70001682
{0x0F12, 0xD301},    //70001684
{0x0F12, 0x1A40},    //70001686
{0x0F12, 0xE000},    //70001688
{0x0F12, 0x1A08},    //7000168A
{0x0F12, 0x9001},    //7000168C
{0x0F12, 0xE001},    //7000168E
{0x0F12, 0x2019},    //70001690
{0x0F12, 0x9001},    //70001692
{0x0F12, 0x4916},    //70001694
{0x0F12, 0x466B},    //70001696
{0x0F12, 0x8A48},    //70001698
{0x0F12, 0x8118},    //7000169A
{0x0F12, 0x8A88},    //7000169C
{0x0F12, 0x8158},    //7000169E
{0x0F12, 0x4814},    //700016A0
{0x0F12, 0x8940},    //700016A2
{0x0F12, 0x0040},    //700016A4
{0x0F12, 0x2103},    //700016A6
{0x0F12, 0xF000},    //700016A8
{0x0F12, 0xF826},    //700016AA
{0x0F12, 0x88A1},    //700016AC
{0x0F12, 0x4288},    //700016AE
{0x0F12, 0xD908},    //700016B0
{0x0F12, 0x8828},    //700016B2
{0x0F12, 0x8030},    //700016B4
{0x0F12, 0x8868},    //700016B6
{0x0F12, 0x8070},    //700016B8
{0x0F12, 0x88A8},    //700016BA
{0x0F12, 0x6038},    //700016BC
{0x0F12, 0xBCFE},    //700016BE
{0x0F12, 0xBC08},    //700016C0
{0x0F12, 0x4718},    //700016C2
{0x0F12, 0x88A9},    //700016C4
{0x0F12, 0x4288},    //700016C6
{0x0F12, 0xD906},    //700016C8
{0x0F12, 0x8820},    //700016CA
{0x0F12, 0x8030},    //700016CC
{0x0F12, 0x8860},    //700016CE
{0x0F12, 0x8070},    //700016D0
{0x0F12, 0x88A0},    //700016D2
{0x0F12, 0x6038},    //700016D4
{0x0F12, 0xE7F2},    //700016D6
{0x0F12, 0x9801},    //700016D8
{0x0F12, 0xA902},    //700016DA
{0x0F12, 0xF000},    //700016DC
{0x0F12, 0xF812},    //700016DE
{0x0F12, 0x0033},    //700016E0
{0x0F12, 0x0029},    //700016E2
{0x0F12, 0x9A02},    //700016E4
{0x0F12, 0x0020},    //700016E6
{0x0F12, 0xF000},    //700016E8
{0x0F12, 0xF814},    //700016EA
{0x0F12, 0x6038},    //700016EC
{0x0F12, 0xE7E6},    //700016EE
{0x0F12, 0x1A28},    //700016F0
{0x0F12, 0x7000},    //700016F2
{0x0F12, 0x0D64},    //700016F4
{0x0F12, 0x7000},    //700016F6
{0x0F12, 0x4778},    //700016F8
{0x0F12, 0x46C0},    //700016FA
{0x0F12, 0xF004},    //700016FC
{0x0F12, 0xE51F},    //700016FE
{0x0F12, 0xA464},    //70001700
{0x0F12, 0x0000},    //70001702
{0x0F12, 0x4778},    //70001704
{0x0F12, 0x46C0},    //70001706
{0x0F12, 0xC000},    //70001708
{0x0F12, 0xE59F},    //7000170A
{0x0F12, 0xFF1C},    //7000170C
{0x0F12, 0xE12F},    //7000170E
{0x0F12, 0x6009},    //70001710
{0x0F12, 0x0000},    //70001712
{0x0F12, 0x4778},    //70001714
{0x0F12, 0x46C0},    //70001716
{0x0F12, 0xC000},    //70001718
{0x0F12, 0xE59F},    //7000171A
{0x0F12, 0xFF1C},    //7000171C
{0x0F12, 0xE12F},    //7000171E
{0x0F12, 0x622F},    //70001720
{0x0F12, 0x0000},    //70001722
{0x002A, 0x2080},
{0x0F12, 0xB510},    //70002080
{0x0F12, 0xF000},    //70002082
{0x0F12, 0xF8F4},    //70002084
{0x0F12, 0xBC10},    //70002086
{0x0F12, 0xBC08},    //70002088
{0x0F12, 0x4718},    //7000208A
{0x0F12, 0xB5F0},    //7000208C
{0x0F12, 0xB08B},    //7000208E
{0x0F12, 0x0006},    //70002090
{0x0F12, 0x2000},    //70002092
{0x0F12, 0x9004},    //70002094
{0x0F12, 0x6835},    //70002096
{0x0F12, 0x6874},    //70002098
{0x0F12, 0x68B0},    //7000209A
{0x0F12, 0x900A},    //7000209C
{0x0F12, 0x68F0},    //7000209E
{0x0F12, 0x9009},    //700020A0
{0x0F12, 0x4F7D},    //700020A2
{0x0F12, 0x8979},    //700020A4
{0x0F12, 0x084A},    //700020A6
{0x0F12, 0x88A8},    //700020A8
{0x0F12, 0x88A3},    //700020AA
{0x0F12, 0x4298},    //700020AC
{0x0F12, 0xD300},    //700020AE
{0x0F12, 0x0018},    //700020B0
{0x0F12, 0xF000},    //700020B2
{0x0F12, 0xF907},    //700020B4
{0x0F12, 0x9007},    //700020B6
{0x0F12, 0x0021},    //700020B8
{0x0F12, 0x0028},    //700020BA
{0x0F12, 0xAA04},    //700020BC
{0x0F12, 0xF000},    //700020BE
{0x0F12, 0xF909},    //700020C0
{0x0F12, 0x9006},    //700020C2
{0x0F12, 0x88A8},    //700020C4
{0x0F12, 0x2800},    //700020C6
{0x0F12, 0xD102},    //700020C8
{0x0F12, 0x27FF},    //700020CA
{0x0F12, 0x1C7F},    //700020CC
{0x0F12, 0xE047},    //700020CE
{0x0F12, 0x88A0},    //700020D0
{0x0F12, 0x2800},    //700020D2
{0x0F12, 0xD101},    //700020D4
{0x0F12, 0x2700},    //700020D6
{0x0F12, 0xE042},    //700020D8
{0x0F12, 0x8820},    //700020DA
{0x0F12, 0x466B},    //700020DC
{0x0F12, 0x8198},    //700020DE
{0x0F12, 0x8860},    //700020E0
{0x0F12, 0x81D8},    //700020E2
{0x0F12, 0x8828},    //700020E4
{0x0F12, 0x8118},    //700020E6
{0x0F12, 0x8868},    //700020E8
{0x0F12, 0x8158},    //700020EA
{0x0F12, 0xA802},    //700020EC
{0x0F12, 0xC803},    //700020EE
{0x0F12, 0xF000},    //700020F0
{0x0F12, 0xF8F8},    //700020F2
{0x0F12, 0x9008},    //700020F4
{0x0F12, 0x8ABA},    //700020F6
{0x0F12, 0x9808},    //700020F8
{0x0F12, 0x466B},    //700020FA
{0x0F12, 0x4342},    //700020FC
{0x0F12, 0x9202},    //700020FE
{0x0F12, 0x8820},    //70002100
{0x0F12, 0x8198},    //70002102
{0x0F12, 0x8860},    //70002104
{0x0F12, 0x81D8},    //70002106
{0x0F12, 0x980A},    //70002108
{0x0F12, 0x9903},    //7000210A
{0x0F12, 0xF000},    //7000210C
{0x0F12, 0xF8EA},    //7000210E
{0x0F12, 0x9A02},    //70002110
{0x0F12, 0x17D1},    //70002112
{0x0F12, 0x0E09},    //70002114
{0x0F12, 0x1889},    //70002116
{0x0F12, 0x1209},    //70002118
{0x0F12, 0x4288},    //7000211A
{0x0F12, 0xDD1F},    //7000211C
{0x0F12, 0x8820},    //7000211E
{0x0F12, 0x466B},    //70002120
{0x0F12, 0x8198},    //70002122
{0x0F12, 0x8860},    //70002124
{0x0F12, 0x81D8},    //70002126
{0x0F12, 0x980A},    //70002128
{0x0F12, 0x9903},    //7000212A
{0x0F12, 0xF000},    //7000212C
{0x0F12, 0xF8DA},    //7000212E
{0x0F12, 0x9001},    //70002130
{0x0F12, 0x8828},    //70002132
{0x0F12, 0x466B},    //70002134
{0x0F12, 0x8118},    //70002136
{0x0F12, 0x8868},    //70002138
{0x0F12, 0x8158},    //7000213A
{0x0F12, 0x980A},    //7000213C
{0x0F12, 0x9902},    //7000213E
{0x0F12, 0xF000},    //70002140
{0x0F12, 0xF8D0},    //70002142
{0x0F12, 0x8AB9},    //70002144
{0x0F12, 0x9A08},    //70002146
{0x0F12, 0x4351},    //70002148
{0x0F12, 0x17CA},    //7000214A
{0x0F12, 0x0E12},    //7000214C
{0x0F12, 0x1851},    //7000214E
{0x0F12, 0x120A},    //70002150
{0x0F12, 0x9901},    //70002152
{0x0F12, 0xF000},    //70002154
{0x0F12, 0xF8B6},    //70002156
{0x0F12, 0x0407},    //70002158
{0x0F12, 0x0C3F},    //7000215A
{0x0F12, 0xE000},    //7000215C
{0x0F12, 0x2700},    //7000215E
{0x0F12, 0x8820},    //70002160
{0x0F12, 0x466B},    //70002162
{0x0F12, 0xAA05},    //70002164
{0x0F12, 0x8198},    //70002166
{0x0F12, 0x8860},    //70002168
{0x0F12, 0x81D8},    //7000216A
{0x0F12, 0x8828},    //7000216C
{0x0F12, 0x8118},    //7000216E
{0x0F12, 0x8868},    //70002170
{0x0F12, 0x8158},    //70002172
{0x0F12, 0xA802},    //70002174
{0x0F12, 0xC803},    //70002176
{0x0F12, 0x003B},    //70002178
{0x0F12, 0xF000},    //7000217A
{0x0F12, 0xF8BB},    //7000217C
{0x0F12, 0x88A1},    //7000217E
{0x0F12, 0x88A8},    //70002180
{0x0F12, 0x003A},    //70002182
{0x0F12, 0xF000},    //70002184
{0x0F12, 0xF8BE},    //70002186
{0x0F12, 0x0004},    //70002188
{0x0F12, 0xA804},    //7000218A
{0x0F12, 0xC803},    //7000218C
{0x0F12, 0x9A09},    //7000218E
{0x0F12, 0x9B07},    //70002190
{0x0F12, 0xF000},    //70002192
{0x0F12, 0xF8AF},    //70002194
{0x0F12, 0xA806},    //70002196
{0x0F12, 0xC805},    //70002198
{0x0F12, 0x0021},    //7000219A
{0x0F12, 0xF000},    //7000219C
{0x0F12, 0xF8B2},    //7000219E
{0x0F12, 0x6030},    //700021A0
{0x0F12, 0xB00B},    //700021A2
{0x0F12, 0xBCF0},    //700021A4
{0x0F12, 0xBC08},    //700021A6
{0x0F12, 0x4718},    //700021A8
{0x0F12, 0xB5F1},    //700021AA
{0x0F12, 0x9900},    //700021AC
{0x0F12, 0x680C},    //700021AE
{0x0F12, 0x493A},    //700021B0
{0x0F12, 0x694B},    //700021B2
{0x0F12, 0x698A},    //700021B4
{0x0F12, 0x4694},    //700021B6
{0x0F12, 0x69CD},    //700021B8
{0x0F12, 0x6A0E},    //700021BA
{0x0F12, 0x4F38},    //700021BC
{0x0F12, 0x42BC},    //700021BE
{0x0F12, 0xD800},    //700021C0
{0x0F12, 0x0027},    //700021C2
{0x0F12, 0x4937},    //700021C4
{0x0F12, 0x6B89},    //700021C6
{0x0F12, 0x0409},    //700021C8
{0x0F12, 0x0C09},    //700021CA
{0x0F12, 0x4A35},    //700021CC
{0x0F12, 0x1E92},    //700021CE
{0x0F12, 0x6BD2},    //700021D0
{0x0F12, 0x0412},    //700021D2
{0x0F12, 0x0C12},    //700021D4
{0x0F12, 0x429F},    //700021D6
{0x0F12, 0xD801},    //700021D8
{0x0F12, 0x0020},    //700021DA
{0x0F12, 0xE031},    //700021DC
{0x0F12, 0x001F},    //700021DE
{0x0F12, 0x434F},    //700021E0
{0x0F12, 0x0A3F},    //700021E2
{0x0F12, 0x42A7},    //700021E4
{0x0F12, 0xD301},    //700021E6
{0x0F12, 0x0018},    //700021E8
{0x0F12, 0xE02A},    //700021EA
{0x0F12, 0x002B},    //700021EC
{0x0F12, 0x434B},    //700021EE
{0x0F12, 0x0A1B},    //700021F0
{0x0F12, 0x42A3},    //700021F2
{0x0F12, 0xD303},    //700021F4
{0x0F12, 0x0220},    //700021F6
{0x0F12, 0xF000},    //700021F8
{0x0F12, 0xF88C},    //700021FA
{0x0F12, 0xE021},    //700021FC
{0x0F12, 0x0029},    //700021FE
{0x0F12, 0x4351},    //70002200
{0x0F12, 0x0A09},    //70002202
{0x0F12, 0x42A1},    //70002204
{0x0F12, 0xD301},    //70002206
{0x0F12, 0x0028},    //70002208
{0x0F12, 0xE01A},    //7000220A
{0x0F12, 0x0031},    //7000220C
{0x0F12, 0x4351},    //7000220E
{0x0F12, 0x0A09},    //70002210
{0x0F12, 0x42A1},    //70002212
{0x0F12, 0xD304},    //70002214
{0x0F12, 0x0220},    //70002216
{0x0F12, 0x0011},    //70002218
{0x0F12, 0xF000},    //7000221A
{0x0F12, 0xF87B},    //7000221C
{0x0F12, 0xE010},    //7000221E
{0x0F12, 0x491E},    //70002220
{0x0F12, 0x8C89},    //70002222
{0x0F12, 0x000A},    //70002224
{0x0F12, 0x4372},    //70002226
{0x0F12, 0x0A12},    //70002228
{0x0F12, 0x42A2},    //7000222A
{0x0F12, 0xD301},    //7000222C
{0x0F12, 0x0030},    //7000222E
{0x0F12, 0xE007},    //70002230
{0x0F12, 0x4662},    //70002232
{0x0F12, 0x434A},    //70002234
{0x0F12, 0x0A12},    //70002236
{0x0F12, 0x42A2},    //70002238
{0x0F12, 0xD302},    //7000223A
{0x0F12, 0x0220},    //7000223C
{0x0F12, 0xF000},    //7000223E
{0x0F12, 0xF869},    //70002240
{0x0F12, 0x4B16},    //70002242
{0x0F12, 0x4D18},    //70002244
{0x0F12, 0x8D99},    //70002246
{0x0F12, 0x1FCA},    //70002248
{0x0F12, 0x3AF9},    //7000224A
{0x0F12, 0xD00A},    //7000224C
{0x0F12, 0x2001},    //7000224E
{0x0F12, 0x0240},    //70002250
{0x0F12, 0x8468},    //70002252
{0x0F12, 0x0220},    //70002254
{0x0F12, 0xF000},    //70002256
{0x0F12, 0xF85D},    //70002258
{0x0F12, 0x9900},    //7000225A
{0x0F12, 0x6008},    //7000225C
{0x0F12, 0xBCF8},    //7000225E
{0x0F12, 0xBC08},    //70002260
{0x0F12, 0x4718},    //70002262
{0x0F12, 0x8D19},    //70002264
{0x0F12, 0x8469},    //70002266
{0x0F12, 0x9900},    //70002268
{0x0F12, 0x6008},    //7000226A
{0x0F12, 0xE7F7},    //7000226C
{0x0F12, 0xB570},    //7000226E
{0x0F12, 0x2200},    //70002270
{0x0F12, 0x490E},    //70002272
{0x0F12, 0x480E},    //70002274
{0x0F12, 0x2401},    //70002276
{0x0F12, 0xF000},    //70002278
{0x0F12, 0xF852},    //7000227A
{0x0F12, 0x0022},    //7000227C
{0x0F12, 0x490D},    //7000227E
{0x0F12, 0x480D},    //70002280
{0x0F12, 0x2502},    //70002282
{0x0F12, 0xF000},    //70002284
{0x0F12, 0xF84C},    //70002286
{0x0F12, 0x490C},    //70002288
{0x0F12, 0x480D},    //7000228A
{0x0F12, 0x002A},    //7000228C
{0x0F12, 0xF000},    //7000228E
{0x0F12, 0xF847},    //70002290
{0x0F12, 0xBC70},    //70002292
{0x0F12, 0xBC08},    //70002294
{0x0F12, 0x4718},    //70002296
{0x0F12, 0x0D64},    //70002298
{0x0F12, 0x7000},    //7000229A
{0x0F12, 0x0470},    //7000229C
{0x0F12, 0x7000},    //7000229E
{0x0F12, 0xA120},    //700022A0
{0x0F12, 0x0007},    //700022A2
{0x0F12, 0x0402},    //700022A4
{0x0F12, 0x7000},    //700022A6
{0x0F12, 0x14A0},    //700022A8
{0x0F12, 0x7000},    //700022AA
{0x0F12, 0x208D},    //700022AC
{0x0F12, 0x7000},    //700022AE
{0x0F12, 0x622F},    //700022B0
{0x0F12, 0x0000},    //700022B2
{0x0F12, 0x1669},    //700022B4
{0x0F12, 0x7000},    //700022B6
{0x0F12, 0x6445},    //700022B8
{0x0F12, 0x0000},    //700022BA
{0x0F12, 0x21AB},    //700022BC
{0x0F12, 0x7000},    //700022BE
{0x0F12, 0x2AA9},    //700022C0
{0x0F12, 0x0000},    //700022C2
{0x0F12, 0x4778},    //700022C4
{0x0F12, 0x46C0},    //700022C6
{0x0F12, 0xC000},    //700022C8
{0x0F12, 0xE59F},    //700022CA
{0x0F12, 0xFF1C},    //700022CC
{0x0F12, 0xE12F},    //700022CE
{0x0F12, 0x5F49},    //700022D0
{0x0F12, 0x0000},    //700022D2
{0x0F12, 0x4778},    //700022D4
{0x0F12, 0x46C0},    //700022D6
{0x0F12, 0xC000},    //700022D8
{0x0F12, 0xE59F},    //700022DA
{0x0F12, 0xFF1C},    //700022DC
{0x0F12, 0xE12F},    //700022DE
{0x0F12, 0x5FC7},    //700022E0
{0x0F12, 0x0000},    //700022E2
{0x0F12, 0x4778},    //700022E4
{0x0F12, 0x46C0},    //700022E6
{0x0F12, 0xC000},    //700022E8
{0x0F12, 0xE59F},    //700022EA
{0x0F12, 0xFF1C},    //700022EC
{0x0F12, 0xE12F},    //700022EE
{0x0F12, 0x5457},    //700022F0
{0x0F12, 0x0000},    //700022F2
{0x0F12, 0x4778},    //700022F4
{0x0F12, 0x46C0},    //700022F6
{0x0F12, 0xC000},    //700022F8
{0x0F12, 0xE59F},    //700022FA
{0x0F12, 0xFF1C},    //700022FC
{0x0F12, 0xE12F},    //700022FE
{0x0F12, 0x5FA3},    //70002300
{0x0F12, 0x0000},    //70002302
{0x0F12, 0x4778},    //70002304
{0x0F12, 0x46C0},    //70002306
{0x0F12, 0xC000},    //70002308
{0x0F12, 0xE59F},    //7000230A
{0x0F12, 0xFF1C},    //7000230C
{0x0F12, 0xE12F},    //7000230E
{0x0F12, 0x51F9},    //70002310
{0x0F12, 0x0000},    //70002312
{0x0F12, 0x4778},    //70002314
{0x0F12, 0x46C0},    //70002316
{0x0F12, 0xF004},    //70002318
{0x0F12, 0xE51F},    //7000231A
{0x0F12, 0xA464},    //7000231C
{0x0F12, 0x0000},    //7000231E
{0x0F12, 0x4778},    //70002320
{0x0F12, 0x46C0},    //70002322
{0x0F12, 0xC000},    //70002324
{0x0F12, 0xE59F},    //70002326
{0x0F12, 0xFF1C},    //70002328
{0x0F12, 0xE12F},    //7000232A
{0x0F12, 0xA007},    //7000232C
{0x0F12, 0x0000},    //7000232E
{0x0F12, 0x6546},    //70002330
{0x0F12, 0x2062},    //70002332
{0x0F12, 0x3120},    //70002334
{0x0F12, 0x3220},    //70002336
{0x0F12, 0x3130},    //70002338
{0x0F12, 0x0030},    //7000233A
{0x0F12, 0xE010},    //7000233C
{0x0F12, 0x0208},    //7000233E
{0x0F12, 0x0058},    //70002340
{0x0F12, 0x0000},    //70002342
// End, 0x of }Trap and Patch (Last : 70002342h)
// Tot, 0xal S}ize 896 (0x0380)

{0x0028, 0xD000},
{0x002A, 0x1000},
{0x0F12, 0x0001},


{0x0028, 0x7000},
{0x002A, 0x1662},
{0x0F12, 0x03B0},
{0x0F12, 0x03B0},


{0x0028, 0x7000},
{0x002A, 0x1658},
{0x0F12, 0x9C40},
{0x0F12, 0x0000},
{0x0F12, 0x9C40},
{0x0F12, 0x0000},


{0x0028, 0x7000},
{0x002A, 0x0ADC},
{0x0F12, 0x0AF0},	//setot_uOnlineClocksDiv40
{0x002A, 0x0AE2},
{0x0F12, 0x222E},	//setot_usSetRomWaitStateThreshold4KHz

{0x002A, 0x0B94},
{0x0F12, 0x0580},	//awbb_GainsInit_0_:R
{0x0F12, 0x0400},	//awbb_GainsInit_1_:G
{0x0F12, 0x05F0},	//awbb_GainsInit_2_:B
{0x002A, 0x04A0},
{0x0F12, 0x8000},	//lt_uLeiInit:AE start
{0x002A, 0x049A},
{0x0F12, 0x00FA},	//lt_uMinExp   0.5ms

//====, 0x====}=========================================================================================
// Set, 0x CIS}/APS/Analog
//====, 0x====}=========================================================================================
// Thi, 0xs re}gisters are for FACTORY ONLY. If you change it without prior notification,
// YOU, 0x are} RESPONSIBLE for the FAILURE that will happen in the future.
//====, 0x====}=========================================================================================
{0x0028, 0xD000},
{0x002A, 0xF106},
{0x0F12, 0x0001},	//L-OB non sub-sampling: revised by Ana 080128
{0x002A, 0xF206},
{0x0F12, 0x0001},	//F-OB non sub-sampling: revised by Ana 080128

//WRIT, 0xE	D}000F260	0000	   tgr_auto_exp (shutter on off:0b shutter off): revised by Ana 080112
{0x002A, 0xC202},
{0x0F12, 0x0700},	//tgr_coarse_integration_time[15:0]: revised by Ana 080115

{0x002A, 0xF260},
{0x0F12, 0x0001}, 	//TGR_AUTO EXP OFF

{0x002A, 0xF414},
{0x0F12, 0x0030}, 	//aig_adc_sat[7:0] (aig_adc_sat[7:4]+1)*33mV + 690mV

{0x002A, 0xC204},
{0x0F12, 0x0100},	//tgr_analogue_code_global[12:0] Analog gain X8
{0x002A, 0xF402},
{0x0F12, 0x0092},	//aig_offset[7:0] : revised by Ana 080425
{0x0F12, 0x007F},	//aig_offset2[7:0]: revised by Ana 080425

{0x002A, 0xF700},
{0x0F12, 0x0040},	//bpradlc_control[7:0]: revised by Ana 080529
// bpr, 0xadlc}_nactive_pedestal[7:5],bpradlc_passthrough[1],bpradlc_bypass[0]
{0x002A, 0xF708},
{0x0F12, 0x0000},	//bpradlc_f_adlc_tune_r[7:0]: revised by TG 080529
{0x0F12, 0x0000},	//bpradlc_f_adlc_tune_gr[7:0]: revised by Tg 080529
{0x0F12, 0x0000},	//bpradlc_f_adlc_tune_gb[7:0]: revised by TG 080529
{0x0F12, 0x0000},	//bpradlc_f_adlc_tune_b[7:0]: revised by TG 080529
{0x0F12, 0x0000},	//bpradlc_f_adlc_tune_total[7:0]: revised by TG 080529
{0x0F12, 0x0040},	//bpradlc_line_adlc_tune_r[7:0]: revised by Ana 080425
{0x0F12, 0x0040},	//bpradlc_line_adlc_tune_gr[7:0]: revised by Ana 080425
{0x0F12, 0x0040},	//bpradlc_line_adlc_tune_gb[7:0]: revised by Ana 080425
{0x0F12, 0x0040},	//bpradlc_line_adlc_tune_b[7:0]: revised by Ana 080425
{0x0F12, 0x0040},	//bpradlc_line_adlc_tune_total[7:0]: revised by Ana 080425
{0x0F12, 0x0001},	//bpradlc_adlc_en[7:0]: revised by TG 080529
// bpr, 0xadlc}_f_l_adlc_en[2],bpradlc_f_adlc_en[1],bpradlc_h_adlc_en[0]
{0x0F12, 0x0015},	// bpradlc_adlc_option[7:0]: revised by TG 080425
// bpr, 0xadlc}_h_adlc_ch_sel[4],
// bpr, 0xadlc}_max_data_clip_sel[3:2],bpradlc_f_adlc_max_data_clip_sel[1:0]
{0x0F12, 0x0001},	// bpradlc_adlc_bpr_en[7:0] adlc_bpr_enable: revised by Ana 080112
{0x0F12, 0x0040},	// bpradlc_adlc_bpr_thresh[7:0]: 080425

{0x002A, 0xF48A},
{0x0F12, 0x0048},	// aig_ld_ptr7[9:0] line OB start addr(04h~48h): revised by Ana 080911
{0x002A, 0xF10A},
{0x0F12, 0x008B},	// tgr_h_desc0_addr_end[10:0] line OB end addr(47h~8Bh): revised by Ana 080911
// lin, 0xe OB} start - line OB end = 67d

{0x002A, 0xF900},
{0x0F12, 0x0067},	// cfpn_config: revised by Ana 080425
// cfp, 0xn_re}f_gain[9:7],cpfn_ref_lines[6:5],cfpn_hbinning[4],cfpn_output_direct[3],
// cfp, 0xn_ov}erflow_protect[2],cpfn_passthrough[1],cfpn_bypass[0]
{0x002A, 0xF406},
{0x0F12, 0x0092},	// aig_cfpn_ref_offset[7:0] : revised by Ana 080425
{0x0F12, 0x007F},	// aig_cfpn_ref_offset2[7:0]: revised by Ana 080425
{0x0F12, 0x0003},	// aig_cfpn_ref_gain[7:0]: revised by Ana 080425

{0x0F12, 0x0003},	// aig_ld_ctrl[1:0] aig_ldb_en[1], aig_ld_en[0]
{0x0F12, 0x0003}, // aig_row_id_ctrl[1:0]
{0x002A, 0xF442},
{0x0F12, 0x0000},	// aig_vavg[0]
{0x0F12, 0x0000},	// aig_havg[0]=1b @H-avg mode: revised by Ana 080116
{0x002A, 0xF448},
{0x0F12, 0x0000},	// aig_sl_off[0]
{0x002A, 0xF456},
{0x0F12, 0x0001},	// aig_blst_en[0]
{0x0F12, 0x0010},	// aig_blst_en_cintr[15:0]
{0x0F12, 0x0000},	// aig_dshut_en[0]

{0x002A, 0xF41A},
{0x0F12, 0x00FF},	// aig_comp_bias[7:0] aig_comp2_bias[7:4], aig_comp1_bias[3:0]: revised by Ana 081013
{0x0F12, 0x0003},	// aig_pix_bias[3:0]

{0x002A, 0xF420},
{0x0F12, 0x0030},	// aig_clp_lvl[7:0]: revised by Ana 080910(refer to 6AA)
{0x002A, 0xF410},
{0x0F12, 0x0001},	// aig_clp_sl_ctrl[0]

{0x0F12, 0x0000},	// aig_cds_test[7:0], aig_cds_test[1]=1b @H-avg mode current save: revised by Ana 080116
{0x002A, 0xF416},
{0x0F12, 0x0001},	// aig_rmp_option[7:0]
{0x002A, 0xF424},
{0x0F12, 0x0000},	// aig_ref_option[7:0], aig_ref_option[0]=1b @H-avg mode odd cds off: revised by Ana 080116
{0x002A, 0xF422},
{0x0F12, 0x0000},	// aig_monit[7:0]

{0x002A, 0xF41E},
{0x0F12, 0x0000},	// aig_pd_pix[0]
{0x002A, 0xF428},
{0x0F12, 0x0000},	// aig_pd_cp[1:0] aig_pd_ncp[1], aig_pd_cp[0]
{0x0F12, 0x0000},	// aig_pd_reg_pix[0]
{0x0F12, 0x0000},	// aig_pd_reg_rg[0]
{0x002A, 0xF430},
{0x0F12, 0x0000},	// aig_pd_reg_tgsl[0]
{0x0F12, 0x0000},	// aig_pd_reg_ntg[0]

{0x0F12, 0x0008},	// aig_rosc_tune_cp[3:0]: revised by Ana 080418
{0x0F12, 0x0005},	// aig_rosc_tune_ncp[3:0]
{0x0F12, 0x000F},	// aig_cp_capa[3:0]
{0x0F12, 0x0001},	// aig_reg_tune_pix[7:0]
{0x0F12, 0x0040},	// aig_reg_tune_rg[7:0]
{0x0F12, 0x0040},	// aig_reg_tune_tgsl[7:0]
{0x0F12, 0x0010}, // aig_reg_tune_ntg[7:0]

{0x002A, 0xF4D6},
{0x0F12, 0x0090}, // aig_v15_tune[7:0]: revised by Ana 081010
// aig, 0x_v15}_tune[7:4]=7h @sensor only Mode
// aig, 0x_v15}_tune[7:4]=9h @ISP 7.5fps Mode
// aig, 0x_v15}_tune[7:4]=Dh @ISP 15fps Mode

{0x0F12, 0x0000},	//aig_vreg_sel[7:0] [9]h_test, [1]reg_sw_stby, [0]aig_reg_sel

{0x002A, 0xF47C},
{0x0F12, 0x000C},	//aig_ld_ptr0[4:0]
{0x0F12, 0x0000},	//aig_ld_ptr1[8:0]: revised by Ana 081010
{0x002A, 0xF49A},
{0x0F12, 0x0008},	//aig_sla_ptr0[3:0]: revised by Ana 080911
{0x0F12, 0x0000},	//aig_sla_ptr1[8:0]: revised by Ana 081010
{0x002A, 0xF4A2},
{0x0F12, 0x0008},	//aig_slb_ptr0[7:0]: revised by Ana 080911
{0x0F12, 0x0000},	//aig_slb_ptr1[8:0]: revised by Ana 081010
{0x002A, 0xF4B2},
{0x0F12, 0x0013},	//aig_rxa_ptr0[6:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x0000},	//aig_rxa_ptr1[9:0]: revised by Ana 081010
{0x0F12, 0x0013},	//aig_rxb_ptr0[6:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x0000},	//aig_rxb_ptr1[9:0]: revised by Ana 081010
{0x002A, 0xF4AA},
{0x0F12, 0x009B},	//aig_txa_ptr0[8:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x00FB},	//aig_txa_ptr1[8:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x009B},	//aig_txb_ptr0[9:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x00FB},	//aig_txb_ptr1[9:0]: revised by Ana 081016 for CFPN
{0x002A, 0xF474},
{0x0F12, 0x0017},	//aig_s3_ptr0[6:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x005F},	//aig_s3_ptr1[8:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x0017},	//aig_s4_ptr0[6:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x008F},	//aig_s4_ptr1[8:0]: revised by Ana 081016 for CFPN

{0x002A, 0xF48C},
{0x0F12, 0x0017},	//aig_clp_sl_ptr0[6:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x009B},	//aig_clp_sl_ptr1[8:0]: revised by Ana 081016 for CFPN
{0x002A, 0xF4C8},
{0x0F12, 0x0163},	//aig_off_en_ptr0[9:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x0193},	//aig_rmp_en_ptr0[9:0]: revised by Ana 081016 for CFPN
{0x002A, 0xF490},
{0x0F12, 0x0191},	//aig_comp_en_ptr0[9:0]: revised by Ana 081016 for CFPN

{0x002A, 0xF418},
{0x0F12, 0x0083},	//aig_dbs_option[7:0]: revised by Ana 081010

{0x002A, 0xF454},
{0x0F12, 0x0001}, 	// aig_power_save[0]: revised by Ana 081229

{0x002A, 0xF702},
{0x0F12, 0x0081},
{0x002A, 0xF4D2},
{0x0F12, 0x0000},

//For , 0xESD }Check
{0x0028, 0x7000},
//Set , 0xFPN }Gain Input
{0x002A, 0x1176},
{0x0F12, 0x0020},	// fpn_GainInput[0]
{0x0F12, 0x0040},	// fpn_GainInput[1]
{0x0F12, 0x0080},	// fpn_GainInput[2]
{0x0F12, 0x0100},	// fpn_GainInput[3]
{0x0F12, 0x0014},	// fpn_GainOutput[0]
{0x0F12, 0x000A},	// fpn_GainOutput[1]
{0x0F12, 0x0008},	// fpn_GainOutput[2]
{0x0F12, 0x0004},	// fpn_GainOutput[3]

// CFP, 0xN Ca}nceller
{0x002A, 0x116C},
{0x0F12, 0x0000}, 	// REF Gain
{0x0F12, 0x0000},	// OvflProtect
{0x0F12, 0x0000},	// bypassThrough
{0x0F12, 0x0000},	// bypass
{0x0F12, 0x0002}, 	// fpn.CollectMethod           0 : Center	1 : Right	2: LEFT
{0x002A, 0x0AE8},
{0x0F12, 0x0000},	// setot_bSendFpnToISP = ??

// sen, 0xsor }aig table setting   sunkyu start.
{0x002A, 0x10EE},
{0x0F12, 0x0000}, 	// senHal_SenRegsModes3_usRegCount   0 value --> not use aig table 3
// , 0xbelo}w register is needed for caculating forbidden and non-linear area.
// , 0xbeca}use senHal_SenRegsModes3_usRegCount is 0, below value does not write to HW.
{0x002A, 0x10F2},
{0x0F12, 0x0000}, 	//senHal_SenRegsModes3_pSenModesRegsArray3[0]	real register 0xD000F45A
{0x002A, 0x1152},
{0x0F12, 0x0030}, 	//senHal_SenRegsModes3_pSenModesRegsArray3[48] real register 0xD000F4BA
{0x0F12, 0x0028}, 	//senHal_SenRegsModes3_pSenModesRegsArray3[49] real register 0xD000F4BC
{0x0F12, 0x0030}, 	//senHal_SenRegsModes3_pSenModesRegsArray3[50] real register 0xD000F4BE
{0x002A, 0x1148},
{0x0F12, 0x00FB}, 	//senHal_SenRegsModes3_pSenModesRegsArray3[43] real register 0xD000F4B0
{0x002A, 0x1144},
{0x0F12, 0x00FB}, 	//senHal_SenRegsModes3_pSenModesRegsArray3[41] real register 0xD000F4AC
// , 0xthis} register is needed to cac forbidden area, this register have to be large for forbidden.
{0x002A, 0x1150},
{0x0F12, 0x01F4}, 	//senHal_SenRegsModes3_pSenModesRegsArray3[47] real register 0xD000F4B8
//sensor aig table setting  sunkyu end

{0x002A, 0x1084},
{0x0F12, 0x0000},	//senHal_bUseAnalogBinning
{0x0F12, 0x0000},	//senHal_bUseAnalogVerAvg

// Set, 0x AE }Target
{0x002A, 0x0F4C},
{0x0F12, 0x003A}, //38, //TVAR_ae_BrAve

{0x002A, 0x0478},
{0x0F12, 0x0114},
{0x0F12, 0x00EB}, //ae boundary


//====, 0x====}========================================================================================
// Set, 0x Fra}me Rate
//====, 0x====}=========================================================================================
{0x002A, 0x0484},
{0x0F12, 0x410A},	//8214	//uMaxExp1
{0x0F12, 0x0000},
{0x002A, 0x048C},
{0x0F12, 0x8214},	//045A	//uMaxExp2
{0x0F12, 0x0000},	//0001
{0x0F12, 0xA122},	//545A	//uMaxExp3
{0x0F12, 0x0000},	//0001
{0x002A, 0x0488},
{0x0F12, 0xf424},	//C4B4	//E45A	//uMaxExp4
{0x0F12, 0x0000},	//0004	//0001
{0x002A, 0x043A},
{0x0F12, 0x01D0}, //1B0	//lt_uMaxAnGain0
{0x0F12, 0x01E0}, //1C0	//lt_uMaxAnGain0_1
{0x002A, 0x0494},
{0x0F12, 0x0300}, //2B0	//lt_uMaxAnGain1
{0x0F12, 0x0600}, //580	//lt_uMaxAnGain2
{0x0f12, 0x0100},
{0x002A, 0x0F52},
{0x0F12, 0x000F}, 	//ae_StatMode

{0x002A, 0x0E98}, //    0180 -> 0270    //bp_uMaxBrightnessFactor
{0x0F12, 0x02A8}, //B0,
{0x002A, 0x0E9E}, //    0180 -> 0270    //bp_uMinBrightnessFactor
{0x0F12, 0x0298}, //290,

//1. A, 0xuto }Flicker 60Hz Start
{0x002A, 0x0B2E},
{0x0F12, 0x0001}, // AFC_Default60Hz	 Auto Flicker 60Hz start 0: Auto Flicker 50Hz start
{0x002A, 0x03F8},
{0x0F12, 0x007F}, // REG_TC_DBG_AutoAlgEnBits	                  default : 007F

////2., 0x Aut}o Flicker 50Hz Start
//WRIT, 0xE	7}0000B2E	0000	//AFC_Default60Hz		1: Auto Flicker 60Hz start 0: Auto Flicker 50Hz start
//WRIT, 0xE 70}0003F8	007F	//REG_TC_DBG_AutoAlgEnBits	 //default : 007F

////3., 0x Man}ual Flicker 60Hz
//WRIT, 0xE 70}0003F8	005F  //REG_TC_DBG_AutoAlgEnBits	 //default : 007F
//WRIT, 0xE 70}0003D4	0002  //REG_SF_USER_FlickerQuant	 //Flicker 50Hz:0001/60Hz:0002/off:0000
//WRIT, 0xE 70}0003D6	0001  //REG_SF_USER_FlickerQuantChanged //if change 0001 write

////4., 0x Man}ual Flicker 50Hz
//WRIT, 0xE 70}0003F8	005F  //REG_TC_DBG_AutoAlgEnBits	 //default : 007F
//WRIT, 0xE 70}0003D4	0001  //REG_SF_USER_FlickerQuant	 //Flicker 50Hz:0001/60Hz:0002/off:0000
//WRIT, 0xE 70}0003D6	0001  //REG_SF_USER_FlickerQuantChanged //if change 0001 write

////5., 0x Fli}cker Off
//WRIT, 0xE 70}0003F8	005F  //REG_TC_DBG_AutoAlgEnBits	 //default : 007F
//WRIT, 0xE 70}0003D4	0000  //REG_SF_USER_FlickerQuant	 //Flicker 50Hz:0001/60Hz:0002/off:0000
//WRIT, 0xE 70}0003D6	0001  //REG_SF_USER_FlickerQuantChanged //if change 0001 write


//WRIT, 0xE 70}000B36	0001  //AFC_ManualQuant


// add, 0xed f}or test
//WRIT, 0xE #s}enHal_uMinColsAddAnalogBin       	 	0048		//Subsampling 1-H min   1768 ϱ  Register
//WRIT, 0xE #s}etot_uOnlineClocksDiv40		 	  			0A28 		//2800
//WRIT, 0xE #s}etot_usSetRomWaitStateThreshold4KHz	222E 		//ROM Wait Threshold to 35MHz


//====, 0x====}=============================================================================================================
{S5K5BAFX_TABLE_WAIT_MS, 0x000a},	// Wait10mSec

//====, 0x====}=========================================================================================
//	Set, 0x PLL}
//====, 0x====}=========================================================================================
// External CLOCK (MCLK : 24Mhz )
{0x002A, 0x01B8},
{0x0F12, 0x5DC0},    	//REG_TC_IPRM_InClockLSBs                   	2   700001B8
{0x0F12, 0x0000},    	//REG_TC_IPRM_InClockMSBs                   	2   700001BA

// Par, 0xalle}l or MIP Selection
{0x002A, 0x01C6},
{0x0F12, 0x0001},    	//REG_TC_IPRM_UseNPviClocks                 	2   700001C6
{0x0F12, 0x0001},    	//REG_TC_IPRM_UseNMipiClocks                	2   700001C8
{0x0F12, 0x0000},    	//REG_TC_IPRM_bBlockInternalPllCalc         	2   700001CA	//0:Auto

// Sys, 0xtem }Clock 0 (System : 24Mhz, PCLK : 48Mhz)
{0x002A, 0x01CC},
{0x0F12, 0x1770},    	//REG_TC_IPRM_OpClk4KHz_0                   	2   700001CC	//1770:24Mhz
{0x0F12, 0x2EE0},    	//REG_TC_IPRM_MinOutRate4KHz_0              	2   700001CE	//2EE0:48Mhz
{0x0F12, 0x2EE0},    	//REG_TC_IPRM_MaxOutRate4KHz_0              	2   700001D0    //2EE0:48Mhz

// Sys, 0xtem }Clock 1 (System : 28Mhz, PCLK : 48Mhz)
{0x002A, 0x01D2},
{0x0F12, 0x1B58}, //222E,    	//REG_TC_IPRM_OpClk4KHz_1                   	2   700001D2	//222E:35Mhz
{0x0F12, 0x2EE0}, //445C,    	//REG_TC_IPRM_MinOutRate4KHz_1              	2   700001D4	//445C:70Mhz
{0x0F12, 0x2EE0}, //445C,    	//REG_TC_IPRM_MaxOutRate4KHz_1              	2   700001D6	//445C:70Mhz

// Sys, 0xtem }Clock 2 (System : 35Mhz, PCLK : 68 ~ 72Mhz)
//s002, 0xA01D}8
//s0F1, 0x2271}0    	//REG_TC_IPRM_OpClk4KHz_2                   	2   700001D8    //2710:40Mhz
//s0F1, 0x2445}C    	//REG_TC_IPRM_MinOutRate4KHz_2              	2   700001DA	//445C:70Mhz
//s0F1, 0x2445}C    	//REG_TC_IPRM_MaxOutRate4KHz_2              	2   700001DC	//445C:70Mhz

{0x002A, 0x01DE},
{0x0F12, 0x0001},    	//REG_TC_IPRM_UseRegsAPI                    	2   700001DE
{0x0F12, 0x0001},    	//REG_TC_IPRM_InitParamsUpdated             	2   700001E0
{S5K5BAFX_TABLE_WAIT_MS, 0x0064}, //100ms Delay

//s002, 0xA01C}0
//s0F1, 0x2000}0 //EG_TC_IPRM_ValidVActiveLow
//    , 0x    }
//s002, 0xA108}4
//s0F1, 0x2000}1 //senHal_bUseAnalogBinning		Avg ub-Sampling
//s002, 0xA10B}6
//s0F1, 0x2004}8 //senHal_uMinColsAddAnalogBin
//    , 0x    }
//s002, 0xA0AD}C
//s0F1, 0x20A2}8 //default : 0AF0, setot_uOnlineClockDiv40


//====, 0x====}=========================================================================================
// Cro, 0xp   }
//====, 0x====}=========================================================================================
{0x002A, 0x01FA},
{0x0F12, 0x0640},    	//REG_TC_GP_PrevReqInputWidth               	2   700001FA
{0x0F12, 0x04B0},    	//REG_TC_GP_PrevReqInputHeight              	2   700001FC
{0x0F12, 0x0000},    	//REG_TC_GP_PrevInputWidthOfs               	2   700001FE
{0x0F12, 0x0000},    	//REG_TC_GP_PrevInputHeightOfs              	2   70000200

//====, 0x====}=========================================================================================
// Set, 0x Pre}view Config
//====, 0x====}=========================================================================================
// Preview Config 0 (640x480, Variable Frame,  15~8fps)
{0x002A, 0x0242},
{0x0F12, 0x0280},    	//REG_0TC_PCFG_usWidth                      	2   70000242	//0640:1600
{0x0F12, 0x01E0},    	//REG_0TC_PCFG_usHeight                     	2   70000244    //04B0:1200
{0x0F12, 0x0005},    	//REG_0TC_PCFG_Format                       	2   70000246
{0x0F12, 0x2EE0}, //445C,    	//REG_0TC_PCFG_usMaxOut4KHzRate             	2   70000248
{0x0F12, 0x2EE0}, //445C,    	//REG_0TC_PCFG_usMinOut4KHzRate             	2   7000024A
{0x0F12, 0x0052},    	//REG_0TC_PCFG_PVIMask                      	2   7000024C
{0x0F12, 0x0001},    	//REG_0TC_PCFG_uClockInd                    	2   7000024E
{0x0F12, 0x0000},    	//REG_0TC_PCFG_usFrTimeType                 	2   70000250
{0x0F12, 0x0000}, 	//REG_0TC_PCFG_FrRateQualityType				2	70000252	//1:Bining, 2:No Bining
{0x0F12, 0x0535}, 	//REG_0TC_PCFG_usMaxFrTimeMsecMult10			2	70000254	//30fps:014D, 15fps:029A, 7.5fps:0535, 3.75fps:A6A
{0x0F12, 0x029A},    	//REG_0TC_PCFG_usMinFrTimeMsecMult10        	2   70000256	//30fps:014D, 15fps:029A, 7.5fps:0535, 3.75fps:A6A
{0x0F12, 0x0000},    	//REG_0TC_PCFG_sSaturation                  	2   70000258
{0x0F12, 0x0000},    	//REG_0TC_PCFG_sSharpBlur                   	2   7000025A
{0x0F12, 0x0000},    	//REG_0TC_PCFG_sGlamour                     	2   7000025C
{0x0F12, 0x0000},    	//REG_0TC_PCFG_sColorTemp                   	2   7000025E
{0x0F12, 0x0000},    	//REG_0TC_PCFG_uDeviceGammaIndex            	2   70000260
{0x0F12, 0x0000},    	//REG_0TC_PCFG_uPrevMirror                  	2   70000262
{0x0F12, 0x0000},    	//REG_0TC_PCFG_uCaptureMirror               	2   70000264
{0x0F12, 0x0000},    	//REG_0TC_PCFG_uRotation                    	2   70000266

// Preview Config 1 (640x480, Fixed Frame,  30~15fps)
{0x002A, 0x0268},
{0x0F12, 0x0280},    	//REG_1TC_PCFG_usWidth                      	2   70000268	//0280:640
{0x0F12, 0x01E0},    	//REG_1TC_PCFG_usHeight                     	2   7000026A    //01E0:480
{0x0F12, 0x0005},    	//REG_1TC_PCFG_Format                       	2   7000026C
{0x0F12, 0x2EE0}, //445C,    	//REG_1TC_PCFG_usMaxOut4KHzRate             	2   7000026E
{0x0F12, 0x2EE0}, //445C,    	//REG_1TC_PCFG_usMinOut4KHzRate             	2   70000270
{0x0F12, 0x0052},    	//REG_1TC_PCFG_PVIMask                      	2   70000272
{0x0F12, 0x0001},    	//REG_1TC_PCFG_uClockInd                    	2   70000274
{0x0F12, 0x0000},    	//REG_1TC_PCFG_usFrTimeType                 	2   70000276
{0x0F12, 0x0000},    	//REG_1TC_PCFG_FrRateQualityType            	2   70000278
{0x0F12, 0x029A},    	//REG_1TC_PCFG_usMaxFrTimeMsecMult10        	2   7000027A
{0x0F12, 0x014D},    	//REG_1TC_PCFG_usMinFrTimeMsecMult10        	2   7000027C
{0x0F12, 0x0000},    	//REG_1TC_PCFG_sSaturation                  	2   7000027E
{0x0F12, 0x0000},    	//REG_1TC_PCFG_sSharpBlur                   	2   70000280
{0x0F12, 0x0000},    	//REG_1TC_PCFG_sGlamour                     	2   70000282
{0x0F12, 0x0000},    	//REG_1TC_PCFG_sColorTemp                   	2   70000284
{0x0F12, 0x0000},    	//REG_1TC_PCFG_uDeviceGammaIndex            	2   70000286
{0x0F12, 0x0001},    	//REG_1TC_PCFG_uPrevMirror                  	2   70000288
{0x0F12, 0x0001},    	//REG_1TC_PCFG_uCaptureMirror               	2   7000028A
{0x0F12, 0x0000},    	//REG_1TC_PCFG_uRotation                    	2   7000028C

//// P, 0xrevi}ew Config 2 (800x600, Night min 5fps)
//s002, 0xA028}E
//s0F1, 0x2032}0    	//REG_2TC_PCFG_usWidth                      	2   7000028E
//s0F1, 0x2025}8    	//REG_2TC_PCFG_usHeight                     	2   70000290
//s0F1, 0x2000}5    	//REG_2TC_PCFG_Format                       	2   70000292
//s0F1, 0x2251}C    	//REG_2TC_PCFG_usMaxOut4KHzRate             	2   70000294
//s0F1, 0x2232}8    	//REG_2TC_PCFG_usMinOut4KHzRate             	2   70000296
//s0F1, 0x2005}2    	//REG_2TC_PCFG_PVIMask                      	2   70000298
//s0F1, 0x2000}0    	//REG_2TC_PCFG_uClockInd                    	2   7000029A
//s0F1, 0x2000}0    	//REG_2TC_PCFG_usFrTimeType                 	2   7000029C
//s0F1, 0x2000}0    	//REG_2TC_PCFG_FrRateQualityType            	2   7000029E
//s0F1, 0x207D}0    	//REG_2TC_PCFG_usMaxFrTimeMsecMult10        	2   700002A0
//s0F1, 0x2029}A    	//REG_2TC_PCFG_usMinFrTimeMsecMult10        	2   700002A2
//s0F1, 0x2000}0    	//REG_2TC_PCFG_sSaturation                  	2   700002A4
//s0F1, 0x2000}0    	//REG_2TC_PCFG_sSharpBlur                   	2   700002A6
//s0F1, 0x2000}0    	//REG_2TC_PCFG_sGlamour                     	2   700002A8
//s0F1, 0x2000}0    	//REG_2TC_PCFG_sColorTemp                   	2   700002AA
//s0F1, 0x2000}0    	//REG_2TC_PCFG_uDeviceGammaIndex            	2   700002AC
//s0F1, 0x2000}0    	//REG_2TC_PCFG_uPrevMirror                  	2   700002AE
//s0F1, 0x2000}0    	//REG_2TC_PCFG_uCaptureMirror               	2   700002B0
//s0F1, 0x2000}0    	//REG_2TC_PCFG_uRotation                    	2   700002B2

//// P, 0xrevi}ew Config 3 (Fire min 1.5fps)
//s002, 0xA02B}4
//s0F1, 0x2032}0    	//REG_3TC_PCFG_usWidth                      	2   700002B4
//s0F1, 0x2025}8    	//REG_3TC_PCFG_usHeight                     	2   700002B6
//s0F1, 0x2000}5    	//REG_3TC_PCFG_Format                       	2   700002B8
//s0F1, 0x2251}C    	//REG_3TC_PCFG_usMaxOut4KHzRate             	2   700002BA
//s0F1, 0x2232}8    	//REG_3TC_PCFG_usMinOut4KHzRate             	2   700002BC
//s0F1, 0x2005}2    	//REG_3TC_PCFG_PVIMask                      	2   700002BE
//s0F1, 0x2000}0    	//REG_3TC_PCFG_uClockInd                    	2   700002C0
//s0F1, 0x2000}0    	//REG_3TC_PCFG_usFrTimeType                 	2   700002C2
//s0F1, 0x2000}0    	//REG_3TC_PCFG_FrRateQualityType            	2   700002C4
//s0F1, 0x2186}A    	//REG_3TC_PCFG_usMaxFrTimeMsecMult10        	2   700002C6
//s0F1, 0x2029}A    	//REG_3TC_PCFG_usMinFrTimeMsecMult10        	2   700002C8
//s0F1, 0x2000}0    	//REG_3TC_PCFG_sSaturation                  	2   700002CA
//s0F1, 0x2000}0    	//REG_3TC_PCFG_sSharpBlur                   	2   700002CC
//s0F1, 0x2000}0    	//REG_3TC_PCFG_sGlamour                     	2   700002CE
//s0F1, 0x2000}0    	//REG_3TC_PCFG_sColorTemp                   	2   700002D0
//s0F1, 0x2000}0    	//REG_3TC_PCFG_uDeviceGammaIndex            	2   700002D2
//s0F1, 0x2000}0    	//REG_3TC_PCFG_uPrevMirror                  	2   700002D4
//s0F1, 0x2000}0    	//REG_3TC_PCFG_uCaptureMirror               	2   700002D6
//s0F1, 0x2000}0    	//REG_3TC_PCFG_uRotation                    	2   700002D8

//// P, 0xrevi}ew Config 4
//s002, 0xA02D}A
//s0F1, 0x2014}0    	//REG_4TC_PCFG_usWidth                      	2   700002DA
//s0F1, 0x200F}0    	//REG_4TC_PCFG_usHeight                     	2   700002DC
//s0F1, 0x2000}5    	//REG_4TC_PCFG_Format                       	2   700002DE
//s0F1, 0x2177}0    	//REG_4TC_PCFG_usMaxOut4KHzRate             	2   700002E0
//s0F1, 0x205D}C    	//REG_4TC_PCFG_usMinOut4KHzRate             	2   700002E2
//s0F1, 0x2005}2    	//REG_4TC_PCFG_PVIMask                      	2   700002E4
//s0F1, 0x2000}0    	//REG_4TC_PCFG_uClockInd                    	2   700002E6
//s0F1, 0x2000}0    	//REG_4TC_PCFG_usFrTimeType                 	2   700002E8
//s0F1, 0x2000}0    	//REG_4TC_PCFG_FrRateQualityType            	2   700002EA
//s0F1, 0x2196}4    	//REG_4TC_PCFG_usMaxFrTimeMsecMult10        	2   700002EC
//s0F1, 0x2000}0    	//REG_4TC_PCFG_usMinFrTimeMsecMult10        	2   700002EE
//s0F1, 0x2000}0    	//REG_4TC_PCFG_sSaturation                  	2   700002F0
//s0F1, 0x2000}0    	//REG_4TC_PCFG_sSharpBlur                   	2   700002F2
//s0F1, 0x2000}0    	//REG_4TC_PCFG_sGlamour                     	2   700002F4
//s0F1, 0x2000}0    	//REG_4TC_PCFG_sColorTemp                   	2   700002F6
//s0F1, 0x2000}0    	//REG_4TC_PCFG_uDeviceGammaIndex            	2   700002F8
//s0F1, 0x2000}0    	//REG_4TC_PCFG_uPrevMirror                  	2   700002FA
//s0F1, 0x2000}0    	//REG_4TC_PCFG_uCaptureMirror               	2   700002FC
//s0F1, 0x2000}0    	//REG_4TC_PCFG_uRotation                    	2   700002FE


//====, 0x====}=========================================================================================
//	Set, 0x MIP}I
//====, 0x====}=========================================================================================
{0x002A, 0x03AC},
{0x0F12, 0x0000},    	//REG_TC_FLS_Mode                           	2   700003AC
{0x002A, 0x03F2},
{0x0F12, 0x0001},    	//REG_TC_OIF_EnMipiLanes                    	2   700003F2
{0x0F12, 0x00C3},    	//REG_TC_OIF_EnPackets                      	2   700003F4
{0x0F12, 0x0001},    	//REG_TC_OIF_CfgChanged                     	2   700003F6

// App, 0xly p}review config
{0x002A, 0x021C},
{0x0F12, 0x0000},	//REG_TC_GP_ActivePrevConfig
{0x002A, 0x0220},
{0x0F12, 0x0001},	//REG_TC_GP_PrevOpenAfterChange
{0x002A, 0x01F8},
{0x0F12, 0x0001},	//REG_TC_GP_NewConfigSync
{0x002A, 0x021E},
{0x0F12, 0x0001},	//REG_TC_GP_PrevConfigChanged
{0x002A, 0x01F0},
{0x0F12, 0x0001},	//REG_TC_GP_EnablePreview
{0x0F12, 0x0001},	//REG_TC_GP_EnablePreviewChanged


//====, 0x====}=========================================================================================
//	Set, 0x Cap}ture Config
//====, 0x====}=========================================================================================
// For, 0x 160}0x1200 Capture
//s002, 0xA030}2
//s0F1, 0x2000}0	//REG_0TC_CCFG_uCaptureMode	//AE/AWB Off
//s0F1, 0x2064}0	//REG_0TC_CCFG_usWidth
//s0F1, 0x204B}0	//REG_0TC_CCFG_usHeight						// Vsize = 1200
//s0F1, 0x2000}5	//REG_0TC_CCFG_Format     //5:YUV,9:JPEG
//s0F1, 0x2252}C	//REG_0TC_CCFG_usMaxOut4KHzRate
//s0F1, 0x2250}C	//REG_0TC_CCFG_usMinOut4KHzRate
//s0F1, 0x2005}2	//REG_0TC_CCFG_PVIMask	(x2: PCLK rising	x0: PCLK Falling)
//s0F1, 0x2000}0	//REG_0TC_CCFG_uClockInd
//s0F1, 0x2000}0	//REG_0TC_CCFG_usFrTimeType
//s0F1, 0x2000}2	//REG_0TC_CCFG_FrRateQualityType
//s0F1, 0x209C}4	//067C  //REG_0TC_CCFG_usMaxFrTimeMsecMult10 // 4fps
//s0F1, 0x2053}5	//REG_0TC_CCFG_usMinFrTimeMsecMult10 // 7.5fps

// Cap, 0xture} Config 0 (1600x1200 fixed 8fps)
{0x002A, 0x0302},
{0x0F12, 0x0000},    	//REG_0TC_CCFG_uCaptureMode                 	2   70000302
{0x0F12, 0x0640},    	//REG_0TC_CCFG_usWidth                      	2   70000304	//0640:1600
{0x0F12, 0x04B0},    	//REG_0TC_CCFG_usHeight                     	2   70000306	//04B0:
{0x0F12, 0x0005},    	//REG_0TC_CCFG_Format                       	2   70000308
{0x0F12, 0x2EE0}, //445C,    	//REG_0TC_CCFG_usMaxOut4KHzRate             	2   7000030A
{0x0F12, 0x2EE0}, //445C,    	//REG_0TC_CCFG_usMinOut4KHzRate             	2   7000030C
{0x0F12, 0x0052},    	//REG_0TC_CCFG_PVIMask                      	2   7000030E
{0x0F12, 0x0001},    	//REG_0TC_CCFG_uClockInd                    	2   70000310
{0x0F12, 0x0002},    	//REG_0TC_CCFG_usFrTimeType                 	2   70000312
{0x0F12, 0x0002},    	//REG_0TC_CCFG_FrRateQualityType            	2   70000314
{0x0F12, 0x0535}, //4E2,    	//REG_0TC_CCFG_usMaxFrTimeMsecMult10        	2   70000316
{0x0F12, 0x0535}, //4E2,    	//REG_0TC_CCFG_usMinFrTimeMsecMult10        	2   70000318
{0x0F12, 0x0000},    	//REG_0TC_CCFG_sSaturation                  	2   7000031A
{0x0F12, 0x0000},    	//REG_0TC_CCFG_sSharpBlur                   	2   7000031C
{0x0F12, 0x0000},    	//REG_0TC_CCFG_sGlamour                     	2   7000031E
{0x0F12, 0x0000},    	//REG_0TC_CCFG_sColorTemp                   	2   70000320
{0x0F12, 0x0000},    	//REG_0TC_CCFG_uDeviceGammaIndex            	2   70000322

//// C, 0xaptu}re Config 1 (Low Capture)
//s002, 0xA032}4
//s0F1, 0x2000}1    	REG_1TC_CCFG_uCaptureMode                 	2   70000324
//s0F1, 0x2064}0    	REG_1TC_CCFG_usWidth                      	2   70000326
//s0F1, 0x204B}0    	REG_1TC_CCFG_usHeight                     	2   70000328
//s0F1, 0x2000}5    	REG_1TC_CCFG_Format                       	2   7000032A
//s0F1, 0x218a}6    	REG_1TC_CCFG_usMaxOut4KHzRate             	2   7000032C
//s0F1, 0x2167}6    	REG_1TC_CCFG_usMinOut4KHzRate             	2   7000032E
//s0F1, 0x2005}2    	REG_1TC_CCFG_PVIMask                      	2   70000330
//s0F1, 0x2000}1    	REG_1TC_CCFG_uClockInd                    	2   70000332
//s0F1, 0x2000}0    	REG_1TC_CCFG_usFrTimeType                 	2   70000334
//s0F1, 0x2000}2    	REG_1TC_CCFG_FrRateQualityType            	2   70000336
//s0F1, 0x2138}8    	REG_1TC_CCFG_usMaxFrTimeMsecMult10        	2   70000338
//s0F1, 0x2138}8    	REG_1TC_CCFG_usMinFrTimeMsecMult10        	2   7000033A
//s0F1, 0x2000}0    	REG_1TC_CCFG_sSaturation                  	2   7000033C
//s0F1, 0x2000}0    	REG_1TC_CCFG_sSharpBlur                   	2   7000033E
//s0F1, 0x2000}0    	REG_1TC_CCFG_sGlamour                     	2   70000340
//s0F1, 0x2000}0    	REG_1TC_CCFG_sColorTemp                   	2   70000342
//s0F1, 0x2000}0    	REG_1TC_CCFG_uDeviceGammaIndex            	2   70000344

//// C, 0xaptu}re Config 2 (Night low lux Capture)
//s002, 0xA034}6
//s0F1, 0x2000}1    	REG_2TC_CCFG_uCaptureMode                 	2   70000346
//s0F1, 0x2064}0    	REG_2TC_CCFG_usWidth                      	2   70000348
//s0F1, 0x204B}0    	REG_2TC_CCFG_usHeight                     	2   7000034A
//s0F1, 0x2000}5    	REG_2TC_CCFG_Format                       	2   7000034C
//s0F1, 0x218a}6    	REG_2TC_CCFG_usMaxOut4KHzRate             	2   7000034E
//s0F1, 0x2167}6    	REG_2TC_CCFG_usMinOut4KHzRate             	2   70000350
//s0F1, 0x2005}2    	REG_2TC_CCFG_PVIMask                      	2   70000352
//s0F1, 0x2000}1    	REG_2TC_CCFG_uClockInd                    	2   70000354
//s0F1, 0x2000}0    	REG_2TC_CCFG_usFrTimeType                 	2   70000356
//s0F1, 0x2000}2    	REG_2TC_CCFG_FrRateQualityType            	2   70000358
//s0F1, 0x2271}0    	REG_2TC_CCFG_usMaxFrTimeMsecMult10        	2   7000035A
//s0F1, 0x2271}0    	REG_2TC_CCFG_usMinFrTimeMsecMult10        	2   7000035C
//s0F1, 0x2000}0    	REG_2TC_CCFG_sSaturation                  	2   7000035E
//s0F1, 0x2000}0    	REG_2TC_CCFG_sSharpBlur                   	2   70000360
//s0F1, 0x2000}0    	REG_2TC_CCFG_sGlamour                     	2   70000362
//s0F1, 0x2000}0    	REG_2TC_CCFG_sColorTemp                   	2   70000364
//s0F1, 0x2000}0    	REG_2TC_CCFG_uDeviceGammaIndex            	2   70000366

//// C, 0xaptu}re Config 3
//s002, 0xA036}8
//s0F1, 0x2000}0    	REG_3TC_CCFG_uCaptureMode                 	2   70000368
//s0F1, 0x2028}0    	REG_3TC_CCFG_usWidth                      	2   7000036A
//s0F1, 0x201E}0    	REG_3TC_CCFG_usHeight                     	2   7000036C
//s0F1, 0x2000}9    	REG_3TC_CCFG_Format                       	2   7000036E
//s0F1, 0x2177}0    	REG_3TC_CCFG_usMaxOut4KHzRate             	2   70000370
//s0F1, 0x205D}C    	REG_3TC_CCFG_usMinOut4KHzRate             	2   70000372
//s0F1, 0x2004}2    	REG_3TC_CCFG_PVIMask                      	2   70000374
//s0F1, 0x2000}0    	REG_3TC_CCFG_uClockInd                    	2   70000376
//s0F1, 0x2000}0    	REG_3TC_CCFG_usFrTimeType                 	2   70000378
//s0F1, 0x2000}2    	REG_3TC_CCFG_FrRateQualityType            	2   7000037A
//s0F1, 0x2196}4    	REG_3TC_CCFG_usMaxFrTimeMsecMult10        	2   7000037C
//s0F1, 0x2000}0    	REG_3TC_CCFG_usMinFrTimeMsecMult10        	2   7000037E
//s0F1, 0x2000}0    	REG_3TC_CCFG_sSaturation                  	2   70000380
//s0F1, 0x2000}0    	REG_3TC_CCFG_sSharpBlur                   	2   70000382
//s0F1, 0x2000}0    	REG_3TC_CCFG_sGlamour                     	2   70000384
//s0F1, 0x2000}0    	REG_3TC_CCFG_sColorTemp                   	2   70000386
//s0F1, 0x2000}0    	REG_3TC_CCFG_uDeviceGammaIndex            	2   70000388

//// C, 0xaptu}re Config 4
//s002, 0xA038}A
//s0F1, 0x2000}0    	REG_4TC_CCFG_uCaptureMode                 	2   7000038A
//s0F1, 0x2014}0    	REG_4TC_CCFG_usWidth                      	2   7000038C
//s0F1, 0x200F}0    	REG_4TC_CCFG_usHeight                     	2   7000038E
//s0F1, 0x2000}9    	REG_4TC_CCFG_Format                       	2   70000390
//s0F1, 0x2177}0    	REG_4TC_CCFG_usMaxOut4KHzRate             	2   70000392
//s0F1, 0x205D}C    	REG_4TC_CCFG_usMinOut4KHzRate             	2   70000394
//s0F1, 0x2004}2    	REG_4TC_CCFG_PVIMask                      	2   70000396
//s0F1, 0x2000}0    	REG_4TC_CCFG_uClockInd                    	2   70000398
//s0F1, 0x2000}0    	REG_4TC_CCFG_usFrTimeType                 	2   7000039A
//s0F1, 0x2000}2    	REG_4TC_CCFG_FrRateQualityType            	2   7000039C
//s0F1, 0x2196}4    	REG_4TC_CCFG_usMaxFrTimeMsecMult10        	2   7000039E
//s0F1, 0x2000}0    	REG_4TC_CCFG_usMinFrTimeMsecMult10        	2   700003A0
//s0F1, 0x2000}0    	REG_4TC_CCFG_sSaturation                  	2   700003A2
//s0F1, 0x2000}0    	REG_4TC_CCFG_sSharpBlur                   	2   700003A4
//s0F1, 0x2000}0    	REG_4TC_CCFG_sGlamour                     	2   700003A6
//s0F1, 0x2000}0    	REG_4TC_CCFG_sColorTemp                   	2   700003A8
//s0F1, 0x2000}0    	REG_4TC_CCFG_uDeviceGammaIndex            	2   700003AA




// Per, 0xiodi}c mismatch
{0x002A, 0x0780},
{0x0F12, 0x0000}, // msm_uOffsetNoBin[0][0]
{0x0F12, 0x0000}, // msm_uOffsetNoBin[0][1]
{0x0F12, 0x0000}, // msm_uOffsetNoBin[1][0]
{0x0F12, 0x0000}, // msm_uOffsetNoBin[1][1]
{0x0F12, 0x0000}, // msm_uOffsetNoBin[2][0]
{0x0F12, 0x0000}, // msm_uOffsetNoBin[2][1]
{0x0F12, 0x0000}, // msm_uOffsetNoBin[3][0]
{0x0F12, 0x0000}, // msm_uOffsetNoBin[3][1]

{0x002A, 0x0798},
{0x0F12, 0x0000}, //msm_uOffsetBin[0][0]
{0x0F12, 0x0000}, //msm_uOffsetBin[0][1]
{0x0F12, 0x0000}, //msm_uOffsetBin[1][0]
{0x0F12, 0x0000}, //msm_uOffsetBin[1][1]

{0x002A, 0x07C0},
{0x0F12, 0x0004}, //msm_NonLinearOfsOutput[2]
{0x0F12, 0x0004}, //msm_NonLinearOfsOutput[3]

{0x002A, 0x0B94},
{0x0F12, 0x0580}, //awbb_GainsInit_0_:R
{0x0F12, 0x0400}, //awbb_GainsInit_1_:G
{0x0F12, 0x05F0}, //awbb_GainsInit_2_:B
{0x002A, 0x04A0},
{0x0F12, 0x8000}, //lt_uLeiInit:AE start

//====, 0x====}=========================================================================================
//	Set, 0x AE }Weights
//====, 0x====}=========================================================================================
{0x002A, 0x0F5A},
{0x0F12, 0x0000},  //0101	//ae_WeightTbl_16_0_
{0x0F12, 0x0000},  //0101	//ae_WeightTbl_16_1_
{0x0F12, 0x0000},  //0101	//ae_WeightTbl_16_2_
{0x0F12, 0x0000},  //0101	//ae_WeightTbl_16_3_
{0x0F12, 0x0101},  //0101	//ae_WeightTbl_16_4_
{0x0F12, 0x0101},  //0101	//ae_WeightTbl_16_5_
{0x0F12, 0x0101},  //0101	//ae_WeightTbl_16_6_
{0x0F12, 0x0101},  //0101	//ae_WeightTbl_16_7_
{0x0F12, 0x0101},  //0101	//ae_WeightTbl_16_8_
{0x0F12, 0x0302},  //0202 // ae_WeightTbl_16_9_
{0x0F12, 0x0203},  //0202 // ae_WeightTbl_16_10
{0x0F12, 0x0101},  //0101	//ae_WeightTbl_16_11
{0x0F12, 0x0101},  //0101	//ae_WeightTbl_16_12
{0x0F12, 0x0403},  //0202 // ae_WeightTbl_16_13
{0x0F12, 0x0304},  //0202 // ae_WeightTbl_16_14
{0x0F12, 0x0101},  //0101	//ae_WeightTbl_16_15
{0x0F12, 0x0101},  //0201 // ae_WeightTbl_16_16
{0x0F12, 0x0403},  //0303 // ae_WeightTbl_16_17
{0x0F12, 0x0304},  //0303 // ae_WeightTbl_16_18
{0x0F12, 0x0101},  //0102 // ae_WeightTbl_16_19
{0x0F12, 0x0101},  //0201 // ae_WeightTbl_16_20
{0x0F12, 0x0302},  //0303 // ae_WeightTbl_16_21
{0x0F12, 0x0203},  //0303 // ae_WeightTbl_16_22
{0x0F12, 0x0101},  //0102 // ae_WeightTbl_16_23
{0x0F12, 0x0101},  //0101	//ae_WeightTbl_16_24
{0x0F12, 0x0101},  //0202 // ae_WeightTbl_16_25
{0x0F12, 0x0101},  //0202 // ae_WeightTbl_16_26
{0x0F12, 0x0101},  //0101	//ae_WeightTbl_16_27
{0x0F12, 0x0000},  //0101	//ae_WeightTbl_16_28
{0x0F12, 0x0000},  //0101	//ae_WeightTbl_16_29
{0x0F12, 0x0000},  //0101	//ae_WeightTbl_16_30
{0x0F12, 0x0000},  //0101	//ae_WeightTbl_16_31

//====, 0x====}=========================================================================================
//	Set, 0x GAS} & CCM White Point
//====, 0x====}=========================================================================================
// par, 0xam_s}tart	TVAR_ash_AwbAshCord
{0x002A, 0x0704},
{0x0F12, 0x00B3}, //TVAR_ash_AwbAshCord_0_
{0x0F12, 0x00E5}, //TVAR_ash_AwbAshCord_1_
{0x0F12, 0x0120}, //TVAR_ash_AwbAshCord_2_
{0x0F12, 0x0136}, //TVAR_ash_AwbAshCord_3_
{0x0F12, 0x0180}, //TVAR_ash_AwbAshCord_4_
{0x0F12, 0x01B0}, //TVAR_ash_AwbAshCord_5_
{0x0F12, 0x0200}, //TVAR_ash_AwbAshCord_6_

// par, 0xam_s}tart	wbt_AwbCcmCord
{0x002A, 0x06F2},
{0x0F12, 0x00B3}, //SARR_AwbCcmCord_0_	Hor
{0x0F12, 0x00E5}, //SARR_AwbCcmCord_1_	IncaA
{0x0F12, 0x0120}, //SARR_AwbCcmCord_2_	WW
{0x0F12, 0x0136}, //SARR_AwbCcmCord_3_	CW
{0x0F12, 0x0180}, //SARR_AwbCcmCord_4_	D50
{0x0F12, 0x0190}, //SARR_AwbCcmCord_5_	D65

// Target Brightness Control
{0x002A, 0x103E},
{0x0F12, 0x0000},	//SARR_IllumType_0_
{0x0F12, 0x0009},	//SARR_IllumType_1_
{0x0F12, 0x0018},	//SARR_IllumType_2_
{0x0F12, 0x0032},	//SARR_IllumType_3_
{0x0F12, 0x004A},	//SARR_IllumType_4_
{0x0F12, 0x0051},	//SARR_IllumType_5_
{0x0F12, 0x0056},	//SARR_IllumType_6_
{0x0F12, 0x010C},	//SARe_2_R_IllumTypeF_0_
{0x0F12, 0x010C},	//SARe_3_R_IllumTypeF_1_
{0x0F12, 0x0109},	//SARe_4_R_IllumTypeF_2_
{0x0F12, 0x0105},	//SARe_5_R_IllumTypeF_3_
{0x0F12, 0x0102},	//SARe_6_R_IllumTypeF_4_
{0x0F12, 0x00FB},	//SARR_IllumTypeF_5_
{0x0F12, 0x00F8},	//SARR_IllumTypeF_6_

// TVAR_ash_GASAlpha(Indoor)
{0x002A, 0x0712},
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[0]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[1]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[2]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[3]

{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[4]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[5]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[6]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[7]

{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[8]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[9]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[10]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[11]

{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[12]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[13]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[14]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[15]

{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[16]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[17]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[18]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[19]

{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[20]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[21]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[22]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[23]

{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[24]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[25]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[26]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[27]

//  TVAR_ash_GASAlpha(Outdoor)
{0x0F12, 0x0108}, //100	//TVAR_ash_GASOutdoorAlpha_0_
{0x0F12, 0x0100},	//TVAR_ash_GASOutdoorAlpha_1_
{0x0F12, 0x0100},	//TVAR_ash_GASOutdoorAlpha_2_
{0x0F12, 0x0100},	//TVAR_ash_GASOutdoorAlpha_3_

// GAS, 0x LUT} Start Address
{0x002A, 0x0754},
{0x0F12, 0x2388},	//TVAR_ash_pGAS
{0x0F12, 0x7000},	//TVAR_ash_pGAS

// param_start	TVAR_ash_pGAS
{0x002A, 0x2388},
{0x0F12, 0x0160},  //0103,  //01CC //TVAR_ash_pGAS[0]
{0x0F12, 0x0134},  //00D9,  //0178 //TVAR_ash_pGAS[1]
{0x0F12, 0x00FF},  //00B2,  //013B //TVAR_ash_pGAS[2]
{0x0F12, 0x00D1},  //0096,  //0108 //TVAR_ash_pGAS[3]
{0x0F12, 0x00B1},  //0083,  //00E4 //TVAR_ash_pGAS[4]
{0x0F12, 0x009D},  //0076,  //00CC //TVAR_ash_pGAS[5]
{0x0F12, 0x0096},  //0076,  //00C5 //TVAR_ash_pGAS[6]
{0x0F12, 0x009E},  //007D,  //00CF //TVAR_ash_pGAS[7]
{0x0F12, 0x00B3},  //008B,  //00E8 //TVAR_ash_pGAS[8]
{0x0F12, 0x00D3},  //00A2,  //0111 //TVAR_ash_pGAS[9]
{0x0F12, 0x00FF},  //00C6,  //0142 //TVAR_ash_pGAS[10]
{0x0F12, 0x0131},  //00EE,  //0183 //TVAR_ash_pGAS[11]
{0x0F12, 0x0159},  //0116,  //01D9 //TVAR_ash_pGAS[12]
{0x0F12, 0x013C},  //00CD,  //0184 //TVAR_ash_pGAS[13]
{0x0F12, 0x0107},  //00AB,  //0142 //TVAR_ash_pGAS[14]
{0x0F12, 0x00CD},  //0086,  //0101 //TVAR_ash_pGAS[15]
{0x0F12, 0x00A1},  //006D,  //00CF //TVAR_ash_pGAS[16]
{0x0F12, 0x0080},  //005A,  //00A7 //TVAR_ash_pGAS[17]
{0x0F12, 0x006B},  //004F,  //0090 //TVAR_ash_pGAS[18]
{0x0F12, 0x0064},  //004D,  //0088 //TVAR_ash_pGAS[19]
{0x0F12, 0x006C},  //0054,  //0092 //TVAR_ash_pGAS[20]
{0x0F12, 0x0080},  //0064,  //00AC //TVAR_ash_pGAS[21]
{0x0F12, 0x00A1},  //007A,  //00D5 //TVAR_ash_pGAS[22]
{0x0F12, 0x00CD},  //009A,  //010D //TVAR_ash_pGAS[23]
{0x0F12, 0x0106},  //00C2,  //014E //TVAR_ash_pGAS[24]
{0x0F12, 0x0139},  //00EC,  //0190 //TVAR_ash_pGAS[25]
{0x0F12, 0x0116},  //00A8,  //014E //TVAR_ash_pGAS[26]
{0x0F12, 0x00DC},  //0089,  //010D //TVAR_ash_pGAS[27]
{0x0F12, 0x00A2},  //0064,  //00CA //TVAR_ash_pGAS[28]
{0x0F12, 0x0073},  //004A,  //0094 //TVAR_ash_pGAS[29]
{0x0F12, 0x0051},  //0037,  //006D //TVAR_ash_pGAS[30]
{0x0F12, 0x003B},  //002A,  //0054 //TVAR_ash_pGAS[31]
{0x0F12, 0x0033},  //0028,  //004E //TVAR_ash_pGAS[32]
{0x0F12, 0x003B},  //002F,  //0058 //TVAR_ash_pGAS[33]
{0x0F12, 0x0050},  //003F,  //0073 //TVAR_ash_pGAS[34]
{0x0F12, 0x0073},  //0059,  //009D //TVAR_ash_pGAS[35]
{0x0F12, 0x00A2},  //0077,  //00D5 //TVAR_ash_pGAS[36]
{0x0F12, 0x00DD},  //00A1,  //011D //TVAR_ash_pGAS[37]
{0x0F12, 0x0115},  //00CB,  //015A //TVAR_ash_pGAS[38]
{0x0F12, 0x00FA},  //008D,  //0129 //TVAR_ash_pGAS[39]
{0x0F12, 0x00BF},  //006F,  //00E5 //TVAR_ash_pGAS[40]
{0x0F12, 0x0085},  //004E,  //00A1 //TVAR_ash_pGAS[41]
{0x0F12, 0x0055},  //0032,  //006B //TVAR_ash_pGAS[42]
{0x0F12, 0x0031},  //001E,  //0042 //TVAR_ash_pGAS[43]
{0x0F12, 0x001B},  //0014,  //002A //TVAR_ash_pGAS[44]
{0x0F12, 0x0014},  //0011,  //0022 //TVAR_ash_pGAS[45]
{0x0F12, 0x001A},  //0017,  //002D //TVAR_ash_pGAS[46]
{0x0F12, 0x0031},  //0027,  //0049 //TVAR_ash_pGAS[47]
{0x0F12, 0x0055},  //003E,  //0075 //TVAR_ash_pGAS[48]
{0x0F12, 0x0085},  //0062,  //00AF //TVAR_ash_pGAS[49]
{0x0F12, 0x00C0},  //0089,  //00F8 //TVAR_ash_pGAS[50]
{0x0F12, 0x00FB},  //00B4,  //013A //TVAR_ash_pGAS[51]
{0x0F12, 0x00EA},  //007A,  //0113 //TVAR_ash_pGAS[52]
{0x0F12, 0x00AF},  //0061,  //00CE //TVAR_ash_pGAS[53]
{0x0F12, 0x0074},  //0040,  //0088 //TVAR_ash_pGAS[54]
{0x0F12, 0x0045},  //0025,  //0052 //TVAR_ash_pGAS[55]
{0x0F12, 0x0020},  //0011,  //002A //TVAR_ash_pGAS[56]
{0x0F12, 0x000B},  //0005,  //0010 //TVAR_ash_pGAS[57]
{0x0F12, 0x0003},  //0003,  //0009 //TVAR_ash_pGAS[58]
{0x0F12, 0x000A},  //000A,  //0015 //TVAR_ash_pGAS[59]
{0x0F12, 0x0020},  //001A,  //0032 //TVAR_ash_pGAS[60]
{0x0F12, 0x0046},  //0033,  //005E //TVAR_ash_pGAS[61]
{0x0F12, 0x0076},  //0054,  //0098 //TVAR_ash_pGAS[62]
{0x0F12, 0x00B1},  //007E,  //00E3 //TVAR_ash_pGAS[63]
{0x0F12, 0x00ED},  //00A6,  //0128 //TVAR_ash_pGAS[64]
{0x0F12, 0x00E6},  //0075,  //010A //TVAR_ash_pGAS[65]
{0x0F12, 0x00AA},  //005F,  //00C4 //TVAR_ash_pGAS[66]
{0x0F12, 0x0071},  //003D,  //0080 //TVAR_ash_pGAS[67]
{0x0F12, 0x0041},  //0020,  //0049 //TVAR_ash_pGAS[68]
{0x0F12, 0x001D},  //000E,  //0020 //TVAR_ash_pGAS[69]
{0x0F12, 0x0008},  //0001,  //0008 //TVAR_ash_pGAS[70]
{0x0F12, 0x0000},  //0000,  //0000 //TVAR_ash_pGAS[71]
{0x0F12, 0x0007},  //0006,  //000D //TVAR_ash_pGAS[72]
{0x0F12, 0x001E},  //0019,  //002A //TVAR_ash_pGAS[73]
{0x0F12, 0x0044},  //0030,  //0058 //TVAR_ash_pGAS[74]
{0x0F12, 0x0074},  //0052,  //0093 //TVAR_ash_pGAS[75]
{0x0F12, 0x00B0},  //007B,  //00DD //TVAR_ash_pGAS[76]
{0x0F12, 0x00EC},  //00A3,  //0123 //TVAR_ash_pGAS[77]
{0x0F12, 0x00EF},  //007A,  //010D //TVAR_ash_pGAS[78]
{0x0F12, 0x00B3},  //0060,  //00CA //TVAR_ash_pGAS[79]
{0x0F12, 0x007A},  //003F,  //0085 //TVAR_ash_pGAS[80]
{0x0F12, 0x004A},  //0024,  //004E //TVAR_ash_pGAS[81]
{0x0F12, 0x0026},  //0011,  //0026 //TVAR_ash_pGAS[82]
{0x0F12, 0x0011},  //0005,  //000E //TVAR_ash_pGAS[83]
{0x0F12, 0x000A},  //0003,  //0007 //TVAR_ash_pGAS[84]
{0x0F12, 0x0011},  //000B,  //0014 //TVAR_ash_pGAS[85]
{0x0F12, 0x0029},  //001C,  //0032 //TVAR_ash_pGAS[86]
{0x0F12, 0x004F},  //0036,  //0061 //TVAR_ash_pGAS[87]
{0x0F12, 0x0080},  //0059,  //009C //TVAR_ash_pGAS[88]
{0x0F12, 0x00BC},  //0082,  //00E8 //TVAR_ash_pGAS[89]
{0x0F12, 0x00F8},  //00AB,  //012F //TVAR_ash_pGAS[90]
{0x0F12, 0x0105},  //0088,  //0121 //TVAR_ash_pGAS[91]
{0x0F12, 0x00C9},  //006F,  //00DE //TVAR_ash_pGAS[92]
{0x0F12, 0x008F},  //004C,  //009A //TVAR_ash_pGAS[93]
{0x0F12, 0x0060},  //002F,  //0063 //TVAR_ash_pGAS[94]
{0x0F12, 0x003C},  //001C,  //003B //TVAR_ash_pGAS[95]
{0x0F12, 0x0026},  //0010,  //0024 //TVAR_ash_pGAS[96]
{0x0F12, 0x001F},  //0010,  //001D //TVAR_ash_pGAS[97]
{0x0F12, 0x0028},  //0019,  //002B //TVAR_ash_pGAS[98]
{0x0F12, 0x0040},  //002A,  //0049 //TVAR_ash_pGAS[99]
{0x0F12, 0x0066},  //0044,  //0079 //TVAR_ash_pGAS[100]
{0x0F12, 0x0097},  //0067,  //00B5 //TVAR_ash_pGAS[101]
{0x0F12, 0x00D4},  //0092,  //0100 //TVAR_ash_pGAS[102]
{0x0F12, 0x0110},  //00BB,  //0145 //TVAR_ash_pGAS[103]
{0x0F12, 0x0124},  //009F,  //013F //TVAR_ash_pGAS[104]
{0x0F12, 0x00EB},  //0084,  //0101 //TVAR_ash_pGAS[105]
{0x0F12, 0x00B1},  //0060,  //00BE //TVAR_ash_pGAS[106]
{0x0F12, 0x0082},  //0044,  //0087 //TVAR_ash_pGAS[107]
{0x0F12, 0x005F},  //0031,  //005F //TVAR_ash_pGAS[108]
{0x0F12, 0x004A},  //0025,  //0048 //TVAR_ash_pGAS[109]
{0x0F12, 0x0043},  //0025,  //0043 //TVAR_ash_pGAS[110]
{0x0F12, 0x004C},  //002E,  //0051 //TVAR_ash_pGAS[111]
{0x0F12, 0x0064},  //0041,  //0070 //TVAR_ash_pGAS[112]
{0x0F12, 0x0089},  //005B,  //00A0 //TVAR_ash_pGAS[113]
{0x0F12, 0x00BA},  //007F,  //00DF //TVAR_ash_pGAS[114]
{0x0F12, 0x00F8},  //00AC,  //0126 //TVAR_ash_pGAS[115]
{0x0F12, 0x012F},  //00D5,  //0168 //TVAR_ash_pGAS[116]
{0x0F12, 0x0147},  //00C5,  //016D //TVAR_ash_pGAS[117]
{0x0F12, 0x0116},  //00A5,  //012F //TVAR_ash_pGAS[118]
{0x0F12, 0x00DE},  //007D,  //00EF //TVAR_ash_pGAS[119]
{0x0F12, 0x00AF},  //0061,  //00BA //TVAR_ash_pGAS[120]
{0x0F12, 0x008E},  //004F,  //0093 //TVAR_ash_pGAS[121]
{0x0F12, 0x007A},  //0043,  //007E //TVAR_ash_pGAS[122]
{0x0F12, 0x0072},  //0044,  //0079 //TVAR_ash_pGAS[123]
{0x0F12, 0x007A},  //004E,  //0087 //TVAR_ash_pGAS[124]
{0x0F12, 0x0091},  //0060,  //00A6 //TVAR_ash_pGAS[125]
{0x0F12, 0x00B6},  //007B,  //00D7 //TVAR_ash_pGAS[126]
{0x0F12, 0x00E8},  //00A0,  //0114 //TVAR_ash_pGAS[127]
{0x0F12, 0x0121},  //00D2,  //0158 //TVAR_ash_pGAS[128]
{0x0F12, 0x0150},  //00F7,  //0199 //TVAR_ash_pGAS[129]
{0x0F12, 0x0170},  //00F0,  //01A6 //TVAR_ash_pGAS[130]
{0x0F12, 0x013F},  //00D2,  //015E //TVAR_ash_pGAS[131]
{0x0F12, 0x0110},  //00A1,  //0122 //TVAR_ash_pGAS[132]
{0x0F12, 0x00E2},  //0085,  //00F1 //TVAR_ash_pGAS[133]
{0x0F12, 0x00C0},  //0070,  //00CB //TVAR_ash_pGAS[134]
{0x0F12, 0x00AB},  //0065,  //00B6 //TVAR_ash_pGAS[135]
{0x0F12, 0x00A4},  //0064,  //00B2 //TVAR_ash_pGAS[136]
{0x0F12, 0x00AC},  //006E,  //00C0 //TVAR_ash_pGAS[137]
{0x0F12, 0x00C3},  //0082,  //00DF //TVAR_ash_pGAS[138]
{0x0F12, 0x00E6},  //009D,  //010D //TVAR_ash_pGAS[139]
{0x0F12, 0x0117},  //00C6,  //0145 //TVAR_ash_pGAS[140]
{0x0F12, 0x0145},  //00F7,  //0188 //TVAR_ash_pGAS[141]
{0x0F12, 0x0172},  //0121,  //01DF //TVAR_ash_pGAS[142]
{0x0F12, 0x0127},  //00D5,  //016C //TVAR_ash_pGAS[143]
{0x0F12, 0x0100},  //00B3,  //0127 //TVAR_ash_pGAS[144]
{0x0F12, 0x00CF},  //008D,  //00F2 //TVAR_ash_pGAS[145]
{0x0F12, 0x00A7},  //0073,  //00CA //TVAR_ash_pGAS[146]
{0x0F12, 0x008D},  //0065,  //00AC //TVAR_ash_pGAS[147]
{0x0F12, 0x007D},  //005D,  //009B //TVAR_ash_pGAS[148]
{0x0F12, 0x0077},  //0059,  //0096 //TVAR_ash_pGAS[149]
{0x0F12, 0x007A},  //005A,  //009C //TVAR_ash_pGAS[150]
{0x0F12, 0x0087},  //0061,  //00AE //TVAR_ash_pGAS[151]
{0x0F12, 0x009E},  //006E,  //00CC //TVAR_ash_pGAS[152]
{0x0F12, 0x00C0},  //0081,  //00F4 //TVAR_ash_pGAS[153]
{0x0F12, 0x00EC},  //009C,  //012D //TVAR_ash_pGAS[154]
{0x0F12, 0x010F},  //00B9,  //0179 //TVAR_ash_pGAS[155]
{0x0F12, 0x0108},  //00AB,  //0130 //TVAR_ash_pGAS[156]
{0x0F12, 0x00D8},  //008F,  //00F6 //TVAR_ash_pGAS[157]
{0x0F12, 0x00A5},  //006C,  //00C4 //TVAR_ash_pGAS[158]
{0x0F12, 0x0080},  //0055,  //0099 //TVAR_ash_pGAS[159]
{0x0F12, 0x0066},  //0047,  //007C //TVAR_ash_pGAS[160]
{0x0F12, 0x0056},  //003E,  //006C //TVAR_ash_pGAS[161]
{0x0F12, 0x004F},  //003B,  //0067 //TVAR_ash_pGAS[162]
{0x0F12, 0x0053},  //003D,  //006E //TVAR_ash_pGAS[163]
{0x0F12, 0x0061},  //0044,  //007F //TVAR_ash_pGAS[164]
{0x0F12, 0x0077},  //0052,  //009E //TVAR_ash_pGAS[165]
{0x0F12, 0x0098},  //0062,  //00C8 //TVAR_ash_pGAS[166]
{0x0F12, 0x00C6},  //007F,  //0100 //TVAR_ash_pGAS[167]
{0x0F12, 0x00F3},  //0099,  //0138 //TVAR_ash_pGAS[168]
{0x0F12, 0x00E7},  //008D,  //0107 //TVAR_ash_pGAS[169]
{0x0F12, 0x00B4},  //0072,  //00CF //TVAR_ash_pGAS[170]
{0x0F12, 0x0081},  //0050,  //0097 //TVAR_ash_pGAS[171]
{0x0F12, 0x005C},  //003C,  //006D //TVAR_ash_pGAS[172]
{0x0F12, 0x0041},  //002C,  //0050 //TVAR_ash_pGAS[173]
{0x0F12, 0x0030},  //0023,  //0040 //TVAR_ash_pGAS[174]
{0x0F12, 0x0029},  //0020,  //003B //TVAR_ash_pGAS[175]
{0x0F12, 0x002E},  //0024,  //0042 //TVAR_ash_pGAS[176]
{0x0F12, 0x003D},  //002C,  //0055 //TVAR_ash_pGAS[177]
{0x0F12, 0x0055},  //003B,  //0074 //TVAR_ash_pGAS[178]
{0x0F12, 0x0076},  //004A,  //009F //TVAR_ash_pGAS[179]
{0x0F12, 0x00A5},  //0067,  //00D9 //TVAR_ash_pGAS[180]
{0x0F12, 0x00D4},  //007F,  //0110 //TVAR_ash_pGAS[181]
{0x0F12, 0x00CF},  //0076,  //00E9 //TVAR_ash_pGAS[182]
{0x0F12, 0x009B},  //0061,  //00AE //TVAR_ash_pGAS[183]
{0x0F12, 0x006A},  //003F,  //0077 //TVAR_ash_pGAS[184]
{0x0F12, 0x0043},  //0029,  //004D //TVAR_ash_pGAS[185]
{0x0F12, 0x0027},  //0019,  //002F //TVAR_ash_pGAS[186]
{0x0F12, 0x0016},  //0010,  //001F //TVAR_ash_pGAS[187]
{0x0F12, 0x000F},  //000D,  //001A //TVAR_ash_pGAS[188]
{0x0F12, 0x0015},  //0011,  //0022 //TVAR_ash_pGAS[189]
{0x0F12, 0x0025},  //001B,  //0036 //TVAR_ash_pGAS[190]
{0x0F12, 0x003E},  //002A,  //0055 //TVAR_ash_pGAS[191]
{0x0F12, 0x0061},  //003C,  //0081 //TVAR_ash_pGAS[192]
{0x0F12, 0x008E},  //0058,  //00BC //TVAR_ash_pGAS[193]
{0x0F12, 0x00BF},  //0070,  //00F5 //TVAR_ash_pGAS[194]
{0x0F12, 0x00C2},  //0067,  //00D8 //TVAR_ash_pGAS[195]
{0x0F12, 0x008E},  //0056,  //009C //TVAR_ash_pGAS[196]
{0x0F12, 0x005D},  //0034,  //0064 //TVAR_ash_pGAS[197]
{0x0F12, 0x0037},  //001E,  //003A //TVAR_ash_pGAS[198]
{0x0F12, 0x001A},  //000E,  //001C //TVAR_ash_pGAS[199]
{0x0F12, 0x0009},  //0006,  //000B //TVAR_ash_pGAS[200]
{0x0F12, 0x0002},  //0004,  //0006 //TVAR_ash_pGAS[201]
{0x0F12, 0x0007},  //0008,  //000F //TVAR_ash_pGAS[202]
{0x0F12, 0x0018},  //0011,  //0024 //TVAR_ash_pGAS[203]
{0x0F12, 0x0033},  //0022,  //0044 //TVAR_ash_pGAS[204]
{0x0F12, 0x0057},  //0034,  //0070 //TVAR_ash_pGAS[205]
{0x0F12, 0x0083},  //0052,  //00AD //TVAR_ash_pGAS[206]
{0x0F12, 0x00B3},  //006A,  //00E6 //TVAR_ash_pGAS[207]
{0x0F12, 0x00BE},  //0062,  //00D0 //TVAR_ash_pGAS[208]
{0x0F12, 0x008A},  //0053,  //0095 //TVAR_ash_pGAS[209]
{0x0F12, 0x005A},  //0032,  //005D //TVAR_ash_pGAS[210]
{0x0F12, 0x0034},  //001B,  //0033 //TVAR_ash_pGAS[211]
{0x0F12, 0x0017},  //000C,  //0015 //TVAR_ash_pGAS[212]
{0x0F12, 0x0006},  //0004,  //0005 //TVAR_ash_pGAS[213]
{0x0F12, 0x0000},  //0002,  //0000 //TVAR_ash_pGAS[214]
{0x0F12, 0x0006},  //0006,  //0009 //TVAR_ash_pGAS[215]
{0x0F12, 0x0017},  //0010,  //001E //TVAR_ash_pGAS[216]
{0x0F12, 0x0033},  //0021,  //0041 //TVAR_ash_pGAS[217]
{0x0F12, 0x0057},  //0035,  //006D //TVAR_ash_pGAS[218]
{0x0F12, 0x0083},  //0053,  //00AA //TVAR_ash_pGAS[219]
{0x0F12, 0x00B3},  //0069,  //00E4 //TVAR_ash_pGAS[220]
{0x0F12, 0x00C5},  //0066,  //00D6 //TVAR_ash_pGAS[221]
{0x0F12, 0x0091},  //0055,  //009A //TVAR_ash_pGAS[222]
{0x0F12, 0x0061},  //0035,  //0062 //TVAR_ash_pGAS[223]
{0x0F12, 0x003B},  //001F,  //0038 //TVAR_ash_pGAS[224]
{0x0F12, 0x0020},  //0010,  //001B //TVAR_ash_pGAS[225]
{0x0F12, 0x000F},  //0009,  //000A //TVAR_ash_pGAS[226]
{0x0F12, 0x0009},  //0006,  //0006 //TVAR_ash_pGAS[227]
{0x0F12, 0x0010},  //000B,  //0010 //TVAR_ash_pGAS[228]
{0x0F12, 0x0021},  //0016,  //0026 //TVAR_ash_pGAS[229]
{0x0F12, 0x003D},  //0027,  //0049 //TVAR_ash_pGAS[230]
{0x0F12, 0x0060},  //003B,  //0076 //TVAR_ash_pGAS[231]
{0x0F12, 0x008D},  //005A,  //00B4 //TVAR_ash_pGAS[232]
{0x0F12, 0x00BE},  //006F,  //00ED //TVAR_ash_pGAS[233]
{0x0F12, 0x00D7},  //006E,  //00E4 //TVAR_ash_pGAS[234]
{0x0F12, 0x00A2},  //0061,  //00AB //TVAR_ash_pGAS[235]
{0x0F12, 0x0072},  //003F,  //0072 //TVAR_ash_pGAS[236]
{0x0F12, 0x004D},  //002A,  //0049 //TVAR_ash_pGAS[237]
{0x0F12, 0x0032},  //001C,  //002C //TVAR_ash_pGAS[238]
{0x0F12, 0x0022},  //0013,  //001C //TVAR_ash_pGAS[239]
{0x0F12, 0x001D},  //0012,  //0019 //TVAR_ash_pGAS[240]
{0x0F12, 0x0024},  //0017,  //0023 //TVAR_ash_pGAS[241]
{0x0F12, 0x0035},  //0023,  //003A //TVAR_ash_pGAS[242]
{0x0F12, 0x0050},  //0033,  //005D //TVAR_ash_pGAS[243]
{0x0F12, 0x0073},  //0048,  //008B //TVAR_ash_pGAS[244]
{0x0F12, 0x00A0},  //0067,  //00C8 //TVAR_ash_pGAS[245]
{0x0F12, 0x00D2},  //007B,  //00FF //TVAR_ash_pGAS[246]
{0x0F12, 0x00F0},  //0083,  //00FF //TVAR_ash_pGAS[247]
{0x0F12, 0x00BE},  //0073,  //00C8 //TVAR_ash_pGAS[248]
{0x0F12, 0x008C},  //0050,  //0090 //TVAR_ash_pGAS[249]
{0x0F12, 0x0068},  //003A,  //0066 //TVAR_ash_pGAS[250]
{0x0F12, 0x004F},  //002D,  //004A //TVAR_ash_pGAS[251]
{0x0F12, 0x0040},  //0026,  //003A //TVAR_ash_pGAS[252]
{0x0F12, 0x003B},  //0024,  //0038 //TVAR_ash_pGAS[253]
{0x0F12, 0x0041},  //002A,  //0042 //TVAR_ash_pGAS[254]
{0x0F12, 0x0052},  //0035,  //0059 //TVAR_ash_pGAS[255]
{0x0F12, 0x006C},  //0046,  //007C //TVAR_ash_pGAS[256]
{0x0F12, 0x008E},  //005A,  //00AB //TVAR_ash_pGAS[257]
{0x0F12, 0x00BE},  //007A,  //00E7 //TVAR_ash_pGAS[258]
{0x0F12, 0x00ED},  //008E,  //011D //TVAR_ash_pGAS[259]
{0x0F12, 0x010C},  //009E,  //0123 //TVAR_ash_pGAS[260]
{0x0F12, 0x00E1},  //008D,  //00EC //TVAR_ash_pGAS[261]
{0x0F12, 0x00AF},  //0066,  //00B7 //TVAR_ash_pGAS[262]
{0x0F12, 0x008A},  //0050,  //008E //TVAR_ash_pGAS[263]
{0x0F12, 0x0072},  //0043,  //0073 //TVAR_ash_pGAS[264]
{0x0F12, 0x0064},  //003B,  //0064 //TVAR_ash_pGAS[265]
{0x0F12, 0x005F},  //003B,  //0062 //TVAR_ash_pGAS[266]
{0x0F12, 0x0065},  //0041,  //006D //TVAR_ash_pGAS[267]
{0x0F12, 0x0074},  //004D,  //0083 //TVAR_ash_pGAS[268]
{0x0F12, 0x008D},  //005D,  //00A7 //TVAR_ash_pGAS[269]
{0x0F12, 0x00B2},  //0072,  //00D4 //TVAR_ash_pGAS[270]
{0x0F12, 0x00E0},  //0096,  //010B //TVAR_ash_pGAS[271]
{0x0F12, 0x010A},  //00AB,  //0144 //TVAR_ash_pGAS[272]
{0x0F12, 0x012F},  //00CB,  //0156 //TVAR_ash_pGAS[273]
{0x0F12, 0x0104},  //00B7,  //0114 //TVAR_ash_pGAS[274]
{0x0F12, 0x00D9},  //0089,  //00E2 //TVAR_ash_pGAS[275]
{0x0F12, 0x00B3},  //006F,  //00BB //TVAR_ash_pGAS[276]
{0x0F12, 0x0099},  //0062,  //009F //TVAR_ash_pGAS[277]
{0x0F12, 0x008B},  //005B,  //0090 //TVAR_ash_pGAS[278]
{0x0F12, 0x0086},  //0059,  //008E //TVAR_ash_pGAS[279]
{0x0F12, 0x008B},  //005E,  //0099 //TVAR_ash_pGAS[280]
{0x0F12, 0x009B},  //006A,  //00B0 //TVAR_ash_pGAS[281]
{0x0F12, 0x00B5},  //007C,  //00D2 //TVAR_ash_pGAS[282]
{0x0F12, 0x00DA},  //0095,  //00FE //TVAR_ash_pGAS[283]
{0x0F12, 0x0101},  //00BE,  //0133 //TVAR_ash_pGAS[284]
{0x0F12, 0x0128},  //00D4,  //017D //TVAR_ash_pGAS[285]
{0x0F12, 0x012F},  //00DB,  //0174 //TVAR_ash_pGAS[286]
{0x0F12, 0x0106},  //00B4,  //012A //TVAR_ash_pGAS[287]
{0x0F12, 0x00D4},  //008C,  //00F6 //TVAR_ash_pGAS[288]
{0x0F12, 0x00AA},  //0071,  //00CC //TVAR_ash_pGAS[289]
{0x0F12, 0x008E},  //0060,  //00AD //TVAR_ash_pGAS[290]
{0x0F12, 0x007D},  //0059,  //009C //TVAR_ash_pGAS[291]
{0x0F12, 0x0079},  //0056,  //0099 //TVAR_ash_pGAS[292]
{0x0F12, 0x0080},  //005D,  //00A4 //TVAR_ash_pGAS[293]
{0x0F12, 0x0093},  //0069,  //00BC //TVAR_ash_pGAS[294]
{0x0F12, 0x00B1},  //007B,  //00E0 //TVAR_ash_pGAS[295]
{0x0F12, 0x00DC},  //0095,  //010E //TVAR_ash_pGAS[296]
{0x0F12, 0x010C},  //00B6,  //0147 //TVAR_ash_pGAS[297]
{0x0F12, 0x0130},  //00D3,  //0193 //TVAR_ash_pGAS[298]
{0x0F12, 0x0112},  //00B2,  //013A //TVAR_ash_pGAS[299]
{0x0F12, 0x00E0},  //0092,  //00FE //TVAR_ash_pGAS[300]
{0x0F12, 0x00AB},  //006D,  //00C9 //TVAR_ash_pGAS[301]
{0x0F12, 0x0083},  //0056,  //009E //TVAR_ash_pGAS[302]
{0x0F12, 0x0067},  //0045,  //007E //TVAR_ash_pGAS[303]
{0x0F12, 0x0057},  //003C,  //006E //TVAR_ash_pGAS[304]
{0x0F12, 0x0051},  //003A,  //006B //TVAR_ash_pGAS[305]
{0x0F12, 0x0059},  //0040,  //0075 //TVAR_ash_pGAS[306]
{0x0F12, 0x006B},  //004D,  //008D //TVAR_ash_pGAS[307]
{0x0F12, 0x0089},  //005F,  //00B2 //TVAR_ash_pGAS[308]
{0x0F12, 0x00B2},  //0076,  //00E0 //TVAR_ash_pGAS[309]
{0x0F12, 0x00E5},  //0096,  //011B //TVAR_ash_pGAS[310]
{0x0F12, 0x0114},  //00B2,  //0152 //TVAR_ash_pGAS[311]
{0x0F12, 0x00F2},  //0091,  //0112 //TVAR_ash_pGAS[312]
{0x0F12, 0x00BD},  //0078,  //00D8 //TVAR_ash_pGAS[313]
{0x0F12, 0x0088},  //0055,  //009F //TVAR_ash_pGAS[314]
{0x0F12, 0x0061},  //003D,  //0073 //TVAR_ash_pGAS[315]
{0x0F12, 0x0044},  //002B,  //0054 //TVAR_ash_pGAS[316]
{0x0F12, 0x0031},  //0022,  //0042 //TVAR_ash_pGAS[317]
{0x0F12, 0x002C},  //001F,  //003F //TVAR_ash_pGAS[318]
{0x0F12, 0x0033},  //0025,  //0049 //TVAR_ash_pGAS[319]
{0x0F12, 0x0047},  //0033,  //0061 //TVAR_ash_pGAS[320]
{0x0F12, 0x0065},  //0046,  //0085 //TVAR_ash_pGAS[321]
{0x0F12, 0x008C},  //005C,  //00B5 //TVAR_ash_pGAS[322]
{0x0F12, 0x00C0},  //007B,  //00F2 //TVAR_ash_pGAS[323]
{0x0F12, 0x00F3},  //0097,  //0128 //TVAR_ash_pGAS[324]
{0x0F12, 0x00DB},  //007B,  //00F5 //TVAR_ash_pGAS[325]
{0x0F12, 0x00A5},  //0065,  //00BA //TVAR_ash_pGAS[326]
{0x0F12, 0x0071},  //0044,  //0080 //TVAR_ash_pGAS[327]
{0x0F12, 0x0049},  //002B,  //0054 //TVAR_ash_pGAS[328]
{0x0F12, 0x002A},  //0019,  //0034 //TVAR_ash_pGAS[329]
{0x0F12, 0x0018},  //000F,  //0022 //TVAR_ash_pGAS[330]
{0x0F12, 0x0011},  //000C,  //001D //TVAR_ash_pGAS[331]
{0x0F12, 0x0018},  //0012,  //0027 //TVAR_ash_pGAS[332]
{0x0F12, 0x002C},  //001F,  //003F //TVAR_ash_pGAS[333]
{0x0F12, 0x004B},  //0034,  //0064 //TVAR_ash_pGAS[334]
{0x0F12, 0x0072},  //004A,  //0092 //TVAR_ash_pGAS[335]
{0x0F12, 0x00A3},  //0067,  //00CF //TVAR_ash_pGAS[336]
{0x0F12, 0x00D7},  //0081,  //0109 //TVAR_ash_pGAS[337]
{0x0F12, 0x00CD},  //006C,  //00E4 //TVAR_ash_pGAS[338]
{0x0F12, 0x0097},  //005C,  //00A8 //TVAR_ash_pGAS[339]
{0x0F12, 0x0065},  //003A,  //006E //TVAR_ash_pGAS[340]
{0x0F12, 0x003C},  //0022,  //0041 //TVAR_ash_pGAS[341]
{0x0F12, 0x001D},  //000F,  //0021 //TVAR_ash_pGAS[342]
{0x0F12, 0x000A},  //0005,  //000E //TVAR_ash_pGAS[343]
{0x0F12, 0x0003},  //0002,  //0008 //TVAR_ash_pGAS[344]
{0x0F12, 0x0009},  //0007,  //0012 //TVAR_ash_pGAS[345]
{0x0F12, 0x001D},  //0014,  //0029 //TVAR_ash_pGAS[346]
{0x0F12, 0x003B},  //0027,  //004D //TVAR_ash_pGAS[347]
{0x0F12, 0x0063},  //003D,  //007C //TVAR_ash_pGAS[348]
{0x0F12, 0x0092},  //005B,  //00B8 //TVAR_ash_pGAS[349]
{0x0F12, 0x00C4},  //0073,  //00F3 //TVAR_ash_pGAS[350]
{0x0F12, 0x00CA},  //0067,  //00DF //TVAR_ash_pGAS[351]
{0x0F12, 0x0094},  //0058,  //00A2 //TVAR_ash_pGAS[352]
{0x0F12, 0x0062},  //0038,  //0068 //TVAR_ash_pGAS[353]
{0x0F12, 0x003A},  //001E,  //003B //TVAR_ash_pGAS[354]
{0x0F12, 0x001A},  //000C,  //001A //TVAR_ash_pGAS[355]
{0x0F12, 0x0007},  //0002,  //0006 //TVAR_ash_pGAS[356]
{0x0F12, 0x0000},  //0000,  //0000 //TVAR_ash_pGAS[357]
{0x0F12, 0x0006},  //0003,  //0009 //TVAR_ash_pGAS[358]
{0x0F12, 0x0018},  //000F,  //001F //TVAR_ash_pGAS[359]
{0x0F12, 0x0036},  //0021,  //0042 //TVAR_ash_pGAS[360]
{0x0F12, 0x005C},  //0037,  //0071 //TVAR_ash_pGAS[361]
{0x0F12, 0x008A},  //0054,  //00AE //TVAR_ash_pGAS[362]
{0x0F12, 0x00BC},  //006C,  //00E9 //TVAR_ash_pGAS[363]
{0x0F12, 0x00D1},  //006A,  //00E4 //TVAR_ash_pGAS[364]
{0x0F12, 0x009B},  //005B,  //00A7 //TVAR_ash_pGAS[365]
{0x0F12, 0x0069},  //0038,  //006C //TVAR_ash_pGAS[366]
{0x0F12, 0x0042},  //0021,  //003F //TVAR_ash_pGAS[367]
{0x0F12, 0x0022},  //000F,  //001E //TVAR_ash_pGAS[368]
{0x0F12, 0x000F},  //0005,  //000B //TVAR_ash_pGAS[369]
{0x0F12, 0x0008},  //0001,  //0004 //TVAR_ash_pGAS[370]
{0x0F12, 0x000D},  //0005,  //000D //TVAR_ash_pGAS[371]
{0x0F12, 0x001F},  //0010,  //0022 //TVAR_ash_pGAS[372]
{0x0F12, 0x003B},  //0021,  //0044 //TVAR_ash_pGAS[373]
{0x0F12, 0x0060},  //0036,  //0072 //TVAR_ash_pGAS[374]
{0x0F12, 0x008D},  //0053,  //00AE //TVAR_ash_pGAS[375]
{0x0F12, 0x00BF},  //006C,  //00EA //TVAR_ash_pGAS[376]
{0x0F12, 0x00E3},  //0072,  //00F5 //TVAR_ash_pGAS[377]
{0x0F12, 0x00AC},  //0066,  //00B9 //TVAR_ash_pGAS[378]
{0x0F12, 0x007A},  //0042,  //007D //TVAR_ash_pGAS[379]
{0x0F12, 0x0053},  //002A,  //0051 //TVAR_ash_pGAS[380]
{0x0F12, 0x0035},  //0018,  //002F //TVAR_ash_pGAS[381]
{0x0F12, 0x0022},  //000F,  //001C //TVAR_ash_pGAS[382]
{0x0F12, 0x001B},  //000B,  //0015 //TVAR_ash_pGAS[383]
{0x0F12, 0x001F},  //000F,  //001D //TVAR_ash_pGAS[384]
{0x0F12, 0x0030},  //0019,  //0031 //TVAR_ash_pGAS[385]
{0x0F12, 0x004B},  //0029,  //0053 //TVAR_ash_pGAS[386]
{0x0F12, 0x006D},  //003D,  //0080 //TVAR_ash_pGAS[387]
{0x0F12, 0x009C},  //005B,  //00BC //TVAR_ash_pGAS[388]
{0x0F12, 0x00CE},  //0072,  //00F7 //TVAR_ash_pGAS[389]
{0x0F12, 0x00FE},  //0087,  //0111 //TVAR_ash_pGAS[390]
{0x0F12, 0x00C9},  //0076,  //00D6 //TVAR_ash_pGAS[391]
{0x0F12, 0x0095},  //0053,  //009C //TVAR_ash_pGAS[392]
{0x0F12, 0x006F},  //003B,  //006F //TVAR_ash_pGAS[393]
{0x0F12, 0x0052},  //002A,  //004E //TVAR_ash_pGAS[394]
{0x0F12, 0x0040},  //0020,  //003A //TVAR_ash_pGAS[395]
{0x0F12, 0x0039},  //001C,  //0033 //TVAR_ash_pGAS[396]
{0x0F12, 0x003D},  //001F,  //003A //TVAR_ash_pGAS[397]
{0x0F12, 0x004B},  //0029,  //004E //TVAR_ash_pGAS[398]
{0x0F12, 0x0063},  //0037,  //006E //TVAR_ash_pGAS[399]
{0x0F12, 0x0086},  //004A,  //009B //TVAR_ash_pGAS[400]
{0x0F12, 0x00B5},  //006A,  //00D5 //TVAR_ash_pGAS[401]
{0x0F12, 0x00E6},  //0081,  //010F //TVAR_ash_pGAS[402]
{0x0F12, 0x011B},  //00A2,  //0139 //TVAR_ash_pGAS[403]
{0x0F12, 0x00ED},  //0093,  //00FD //TVAR_ash_pGAS[404]
{0x0F12, 0x00BA},  //006A,  //00C6 //TVAR_ash_pGAS[405]
{0x0F12, 0x0092},  //004F,  //0098 //TVAR_ash_pGAS[406]
{0x0F12, 0x0076},  //003F,  //0077 //TVAR_ash_pGAS[407]
{0x0F12, 0x0065},  //0035,  //0064 //TVAR_ash_pGAS[408]
{0x0F12, 0x005D},  //0032,  //005D //TVAR_ash_pGAS[409]
{0x0F12, 0x0060},  //0035,  //0064 //TVAR_ash_pGAS[410]
{0x0F12, 0x006D},  //003E,  //0076 //TVAR_ash_pGAS[411]
{0x0F12, 0x0084},  //004B,  //0095 //TVAR_ash_pGAS[412]
{0x0F12, 0x00A8},  //0061,  //00C2 //TVAR_ash_pGAS[413]
{0x0F12, 0x00D6},  //0082,  //00F8 //TVAR_ash_pGAS[414]
{0x0F12, 0x0101},  //0099,  //0135 //TVAR_ash_pGAS[415]
{0x0F12, 0x0140},  //00CC,  //016C //TVAR_ash_pGAS[416]
{0x0F12, 0x0112},  //00BC,  //0128 //TVAR_ash_pGAS[417]
{0x0F12, 0x00E5},  //008C,  //00F2 //TVAR_ash_pGAS[418]
{0x0F12, 0x00BD},  //006F,  //00C7 //TVAR_ash_pGAS[419]
{0x0F12, 0x009E},  //005E,  //00A4 //TVAR_ash_pGAS[420]
{0x0F12, 0x008C},  //0053,  //0092 //TVAR_ash_pGAS[421]
{0x0F12, 0x0085},  //0050,  //008A //TVAR_ash_pGAS[422]
{0x0F12, 0x0087},  //0052,  //008F //TVAR_ash_pGAS[423]
{0x0F12, 0x0094},  //005B,  //00A3 //TVAR_ash_pGAS[424]
{0x0F12, 0x00AC},  //006B,  //00C0 //TVAR_ash_pGAS[425]
{0x0F12, 0x00D0},  //0084,  //00EA //TVAR_ash_pGAS[426]
{0x0F12, 0x00F8},  //00A7,  //0121 //TVAR_ash_pGAS[427]
{0x0F12, 0x0123},  //00BE,  //016F //TVAR_ash_pGAS[428]
{0x0F12, 0x00F2},  //0086,  //0123 //TVAR_ash_pGAS[429]
{0x0F12, 0x00D1},  //006E,  //00E7 //TVAR_ash_pGAS[430]
{0x0F12, 0x00A7},  //0055,  //00BD //TVAR_ash_pGAS[431]
{0x0F12, 0x0087},  //0045,  //009C //TVAR_ash_pGAS[432]
{0x0F12, 0x0073},  //003E,  //0087 //TVAR_ash_pGAS[433]
{0x0F12, 0x0067},  //003B,  //007C //TVAR_ash_pGAS[434]
{0x0F12, 0x0064},  //003C,  //007B //TVAR_ash_pGAS[435]
{0x0F12, 0x006B},  //003E,  //0086 //TVAR_ash_pGAS[436]
{0x0F12, 0x007C},  //0045,  //0099 //TVAR_ash_pGAS[437]
{0x0F12, 0x0094},  //0052,  //00B7 //TVAR_ash_pGAS[438]
{0x0F12, 0x00B7},  //0063,  //00DC //TVAR_ash_pGAS[439]
{0x0F12, 0x00E1},  //0079,  //010E //TVAR_ash_pGAS[440]
{0x0F12, 0x00FF},  //008B,  //014A //TVAR_ash_pGAS[441]
{0x0F12, 0x00D6},  //0063,  //00F1 //TVAR_ash_pGAS[442]
{0x0F12, 0x00AE},  //0052,  //00C1 //TVAR_ash_pGAS[443]
{0x0F12, 0x0085},  //003D,  //0096 //TVAR_ash_pGAS[444]
{0x0F12, 0x0068},  //0032,  //0077 //TVAR_ash_pGAS[445]
{0x0F12, 0x0054},  //002B,  //0062 //TVAR_ash_pGAS[446]
{0x0F12, 0x0048},  //0029,  //0058 //TVAR_ash_pGAS[447]
{0x0F12, 0x0045},  //0029,  //0057 //TVAR_ash_pGAS[448]
{0x0F12, 0x004B},  //002C,  //0061 //TVAR_ash_pGAS[449]
{0x0F12, 0x005B},  //0034,  //0074 //TVAR_ash_pGAS[450]
{0x0F12, 0x0073},  //003C,  //0090 //TVAR_ash_pGAS[451]
{0x0F12, 0x0093},  //004B,  //00B7 //TVAR_ash_pGAS[452]
{0x0F12, 0x00BF},  //0060,  //00E7 //TVAR_ash_pGAS[453]
{0x0F12, 0x00E9},  //0070,  //0113 //TVAR_ash_pGAS[454]
{0x0F12, 0x00B8},  //0048,  //00CB //TVAR_ash_pGAS[455]
{0x0F12, 0x008E},  //003B,  //009D //TVAR_ash_pGAS[456]
{0x0F12, 0x0066},  //002B,  //0071 //TVAR_ash_pGAS[457]
{0x0F12, 0x0049},  //0023,  //0052 //TVAR_ash_pGAS[458]
{0x0F12, 0x0035},  //001B,  //0040 //TVAR_ash_pGAS[459]
{0x0F12, 0x0028},  //0017,  //0035 //TVAR_ash_pGAS[460]
{0x0F12, 0x0025},  //0017,  //0034 //TVAR_ash_pGAS[461]
{0x0F12, 0x002B},  //001B,  //003D //TVAR_ash_pGAS[462]
{0x0F12, 0x003B},  //0022,  //004F //TVAR_ash_pGAS[463]
{0x0F12, 0x0053},  //002C,  //006B //TVAR_ash_pGAS[464]
{0x0F12, 0x0072},  //0037,  //0090 //TVAR_ash_pGAS[465]
{0x0F12, 0x009D},  //0048,  //00C2 //TVAR_ash_pGAS[466]
{0x0F12, 0x00C8},  //0058,  //00EC //TVAR_ash_pGAS[467]
{0x0F12, 0x00A2},  //0033,  //00B0 //TVAR_ash_pGAS[468]
{0x0F12, 0x0078},  //002D,  //0082 //TVAR_ash_pGAS[469]
{0x0F12, 0x0051},  //001F,  //0057 //TVAR_ash_pGAS[470]
{0x0F12, 0x0034},  //0016,  //003A //TVAR_ash_pGAS[471]
{0x0F12, 0x001F},  //000E,  //0026 //TVAR_ash_pGAS[472]
{0x0F12, 0x0012},  //000C,  //001B //TVAR_ash_pGAS[473]
{0x0F12, 0x000E},  //000B,  //0019 //TVAR_ash_pGAS[474]
{0x0F12, 0x0014},  //000E,  //0021 //TVAR_ash_pGAS[475]
{0x0F12, 0x0024},  //0014,  //0033 //TVAR_ash_pGAS[476]
{0x0F12, 0x003B},  //001F,  //004F //TVAR_ash_pGAS[477]
{0x0F12, 0x005B},  //0028,  //0072 //TVAR_ash_pGAS[478]
{0x0F12, 0x0083},  //0038,  //00A2 //TVAR_ash_pGAS[479]
{0x0F12, 0x00AD},  //0045,  //00CF //TVAR_ash_pGAS[480]
{0x0F12, 0x0095},  //0028,  //009F //TVAR_ash_pGAS[481]
{0x0F12, 0x006C},  //0023,  //0072 //TVAR_ash_pGAS[482]
{0x0F12, 0x0046},  //0016,  //0047 //TVAR_ash_pGAS[483]
{0x0F12, 0x002A},  //000E,  //002A //TVAR_ash_pGAS[484]
{0x0F12, 0x0014},  //0008,  //0016 //TVAR_ash_pGAS[485]
{0x0F12, 0x0007},  //0004,  //000A //TVAR_ash_pGAS[486]
{0x0F12, 0x0002},  //0004,  //0008 //TVAR_ash_pGAS[487]
{0x0F12, 0x0008},  //0005,  //000F //TVAR_ash_pGAS[488]
{0x0F12, 0x0016},  //000A,  //0021 //TVAR_ash_pGAS[489]
{0x0F12, 0x002D},  //0013,  //003A //TVAR_ash_pGAS[490]
{0x0F12, 0x004C},  //001D,  //005C //TVAR_ash_pGAS[491]
{0x0F12, 0x0072},  //002D,  //008C //TVAR_ash_pGAS[492]
{0x0F12, 0x009B},  //0036,  //00BB //TVAR_ash_pGAS[493]
{0x0F12, 0x0093},  //0022,  //009A //TVAR_ash_pGAS[494]
{0x0F12, 0x006A},  //0020,  //006C //TVAR_ash_pGAS[495]
{0x0F12, 0x0045},  //0013,  //0042 //TVAR_ash_pGAS[496]
{0x0F12, 0x0028},  //000B,  //0024 //TVAR_ash_pGAS[497]
{0x0F12, 0x0013},  //0006,  //0010 //TVAR_ash_pGAS[498]
{0x0F12, 0x0005},  //0001,  //0004 //TVAR_ash_pGAS[499]
{0x0F12, 0x0000},  //0000,  //0000 //TVAR_ash_pGAS[500]
{0x0F12, 0x0004},  //0001,  //0007 //TVAR_ash_pGAS[501]
{0x0F12, 0x0012},  //0005,  //0018 //TVAR_ash_pGAS[502]
{0x0F12, 0x0028},  //000C,  //0030 //TVAR_ash_pGAS[503]
{0x0F12, 0x0045},  //0015,  //0050 //TVAR_ash_pGAS[504]
{0x0F12, 0x006A},  //0024,  //0080 //TVAR_ash_pGAS[505]
{0x0F12, 0x0093},  //002E,  //00AF //TVAR_ash_pGAS[506]
{0x0F12, 0x009B},  //0022,  //009F //TVAR_ash_pGAS[507]
{0x0F12, 0x0071},  //001F,  //0071 //TVAR_ash_pGAS[508]
{0x0F12, 0x004C},  //0014,  //0046 //TVAR_ash_pGAS[509]
{0x0F12, 0x0030},  //000B,  //0028 //TVAR_ash_pGAS[510]
{0x0F12, 0x001A},  //0006,  //0014 //TVAR_ash_pGAS[511]
{0x0F12, 0x000C},  //0002,  //0006 //TVAR_ash_pGAS[512]
{0x0F12, 0x0007},  //0000,  //0003 //TVAR_ash_pGAS[513]
{0x0F12, 0x000B},  //0000,  //0009 //TVAR_ash_pGAS[514]
{0x0F12, 0x0018},  //0004,  //0019 //TVAR_ash_pGAS[515]
{0x0F12, 0x002C},  //0009,  //0030 //TVAR_ash_pGAS[516]
{0x0F12, 0x0048},  //0012,  //0051 //TVAR_ash_pGAS[517]
{0x0F12, 0x006D},  //0021,  //0080 //TVAR_ash_pGAS[518]
{0x0F12, 0x0097},  //002B,  //00B0 //TVAR_ash_pGAS[519]
{0x0F12, 0x00AE},  //0029,  //00AD //TVAR_ash_pGAS[520]
{0x0F12, 0x0083},  //0026,  //0080 //TVAR_ash_pGAS[521]
{0x0F12, 0x005C},  //0019,  //0055 //TVAR_ash_pGAS[522]
{0x0F12, 0x0040},  //0010,  //0036 //TVAR_ash_pGAS[523]
{0x0F12, 0x002B},  //000B,  //0021 //TVAR_ash_pGAS[524]
{0x0F12, 0x001E},  //0008,  //0015 //TVAR_ash_pGAS[525]
{0x0F12, 0x0018},  //0005,  //0010 //TVAR_ash_pGAS[526]
{0x0F12, 0x001C},  //0005,  //0016 //TVAR_ash_pGAS[527]
{0x0F12, 0x0027},  //0008,  //0024 //TVAR_ash_pGAS[528]
{0x0F12, 0x003A},  //000D,  //003A //TVAR_ash_pGAS[529]
{0x0F12, 0x0055},  //0014,  //005B //TVAR_ash_pGAS[530]
{0x0F12, 0x007B},  //0025,  //008B //TVAR_ash_pGAS[531]
{0x0F12, 0x00A6},  //002E,  //00BA //TVAR_ash_pGAS[532]
{0x0F12, 0x00CA},  //0035,  //00C8 //TVAR_ash_pGAS[533]
{0x0F12, 0x009E},  //0031,  //0099 //TVAR_ash_pGAS[534]
{0x0F12, 0x0076},  //0021,  //006E //TVAR_ash_pGAS[535]
{0x0F12, 0x0059},  //001A,  //004E //TVAR_ash_pGAS[536]
{0x0F12, 0x0046},  //0015,  //003A //TVAR_ash_pGAS[537]
{0x0F12, 0x0039},  //0012,  //002D //TVAR_ash_pGAS[538]
{0x0F12, 0x0033},  //0010,  //002A //TVAR_ash_pGAS[539]
{0x0F12, 0x0036},  //0010,  //002E //TVAR_ash_pGAS[540]
{0x0F12, 0x0040},  //0012,  //003B //TVAR_ash_pGAS[541]
{0x0F12, 0x0052},  //0015,  //0051 //TVAR_ash_pGAS[542]
{0x0F12, 0x006C},  //001E,  //0072 //TVAR_ash_pGAS[543]
{0x0F12, 0x0094},  //002E,  //00A1 //TVAR_ash_pGAS[544]
{0x0F12, 0x00BF},  //0039,  //00D2 //TVAR_ash_pGAS[545]
{0x0F12, 0x00EB},  //004A,  //00EC //TVAR_ash_pGAS[546]
{0x0F12, 0x00C3},  //0043,  //00BE //TVAR_ash_pGAS[547]
{0x0F12, 0x0099},  //0031,  //0092 //TVAR_ash_pGAS[548]
{0x0F12, 0x007A},  //0027,  //0072 //TVAR_ash_pGAS[549]
{0x0F12, 0x0066},  //0023,  //005C //TVAR_ash_pGAS[550]
{0x0F12, 0x005A},  //0020,  //0050 //TVAR_ash_pGAS[551]
{0x0F12, 0x0054},  //001F,  //004D //TVAR_ash_pGAS[552]
{0x0F12, 0x0056},  //001F,  //0050 //TVAR_ash_pGAS[553]
{0x0F12, 0x005F},  //0020,  //005D //TVAR_ash_pGAS[554]
{0x0F12, 0x0071},  //0024,  //0073 //TVAR_ash_pGAS[555]
{0x0F12, 0x008D},  //002E,  //0094 //TVAR_ash_pGAS[556]
{0x0F12, 0x00B6},  //0042,  //00C2 //TVAR_ash_pGAS[557]
{0x0F12, 0x00DE},  //004B,  //00F4 //TVAR_ash_pGAS[558]
{0x0F12, 0x010D},  //0066,  //011A //TVAR_ash_pGAS[559]
{0x0F12, 0x00E7},  //0061,  //00E3 //TVAR_ash_pGAS[560]
{0x0F12, 0x00C1},  //0046,  //00B7 //TVAR_ash_pGAS[561]
{0x0F12, 0x00A0},  //003A,  //0097 //TVAR_ash_pGAS[562]
{0x0F12, 0x008A},  //0034,  //0081 //TVAR_ash_pGAS[563]
{0x0F12, 0x007C},  //002F,  //0075 //TVAR_ash_pGAS[564]
{0x0F12, 0x0076},  //002F,  //006F //TVAR_ash_pGAS[565]
{0x0F12, 0x0078},  //002F,  //0074 //TVAR_ash_pGAS[566]
{0x0F12, 0x0081},  //0031,  //0081 //TVAR_ash_pGAS[567]
{0x0F12, 0x0093},  //0037,  //0097 //TVAR_ash_pGAS[568]
{0x0F12, 0x00B1},  //0043,  //00B8 //TVAR_ash_pGAS[569]
{0x0F12, 0x00D5},  //0059,  //00E7 //TVAR_ash_pGAS[570]
{0x0F12, 0x00FD},  //0063,  //0127 //TVAR_ash_pGAS[571]

// Gam, 0xma  }
{0x002A, 0x04CC},
{0x0F12, 0x0000}, //0000 //SARR_usGammaLutRGBIndoor[0][0]
{0x0F12, 0x0002}, //0002 //SARR_usGammaLutRGBIndoor[0][1]
{0x0F12, 0x0008}, //0008 //SARR_usGammaLutRGBIndoor[0][2]
{0x0F12, 0x0016}, //0018 //SARR_usGammaLutRGBIndoor[0][3]
{0x0F12, 0x0055}, //005A //SARR_usGammaLutRGBIndoor[0][4]
{0x0F12, 0x00E6}, //00DF //SARR_usGammaLutRGBIndoor[0][5]
{0x0F12, 0x0141}, //013F //SARR_usGammaLutRGBIndoor[0][6]
{0x0F12, 0x0188}, //0186 //SARR_usGammaLutRGBIndoor[0][7]
{0x0F12, 0x01E6}, //01E6 //SARR_usGammaLutRGBIndoor[0][8]
{0x0F12, 0x0236}, //0236 //SARR_usGammaLutRGBIndoor[0][9]
{0x0F12, 0x02BA}, //02BA //SARR_usGammaLutRGBIndoor[0][10]
{0x0F12, 0x032A}, //032A //SARR_usGammaLutRGBIndoor[0][11]
{0x0F12, 0x0385}, //0385 //SARR_usGammaLutRGBIndoor[0][12]
{0x0F12, 0x03C2}, //03C2 //SARR_usGammaLutRGBIndoor[0][13]
{0x0F12, 0x03EA}, //03EA //SARR_usGammaLutRGBIndoor[0][14]
{0x0F12, 0x03FF}, //03FF //SARR_usGammaLutRGBIndoor[0][15]

{0x0F12, 0x0000}, //0000 //SARR_usGammaLutRGBIndoor[1][0]
{0x0F12, 0x0002}, //0002 //SARR_usGammaLutRGBIndoor[1][1]
{0x0F12, 0x0008}, //0008 //SARR_usGammaLutRGBIndoor[1][2]
{0x0F12, 0x0016}, //0018 //SARR_usGammaLutRGBIndoor[1][3]
{0x0F12, 0x0055}, //005A //SARR_usGammaLutRGBIndoor[1][4]
{0x0F12, 0x00E6}, //00DF //SARR_usGammaLutRGBIndoor[1][5]
{0x0F12, 0x0141}, //013F //SARR_usGammaLutRGBIndoor[1][6]
{0x0F12, 0x0188}, //0186 //SARR_usGammaLutRGBIndoor[1][7]
{0x0F12, 0x01E6}, //01E6 //SARR_usGammaLutRGBIndoor[1][8]
{0x0F12, 0x0236}, //0236 //SARR_usGammaLutRGBIndoor[1][9]
{0x0F12, 0x02BA}, //02BA //SARR_usGammaLutRGBIndoor[1][10]
{0x0F12, 0x032A}, //032A //SARR_usGammaLutRGBIndoor[1][11]
{0x0F12, 0x0385}, //0385 //SARR_usGammaLutRGBIndoor[1][12]
{0x0F12, 0x03C2}, //03C2 //SARR_usGammaLutRGBIndoor[1][13]
{0x0F12, 0x03EA}, //03EA //SARR_usGammaLutRGBIndoor[1][14]
{0x0F12, 0x03FF}, //03FF //SARR_usGammaLutRGBIndoor[1][15]

{0x0F12, 0x0000}, //0000 //SARR_usGammaLutRGBIndoor[2][0]
{0x0F12, 0x0002}, //0002 //SARR_usGammaLutRGBIndoor[2][1]
{0x0F12, 0x0008}, //0008 //SARR_usGammaLutRGBIndoor[2][2]
{0x0F12, 0x0016}, //0018 //SARR_usGammaLutRGBIndoor[2][3]
{0x0F12, 0x0055}, //005A //SARR_usGammaLutRGBIndoor[2][4]
{0x0F12, 0x00E6}, //00DF //SARR_usGammaLutRGBIndoor[2][5]
{0x0F12, 0x0141}, //013F //SARR_usGammaLutRGBIndoor[2][6]
{0x0F12, 0x0188}, //0186 //SARR_usGammaLutRGBIndoor[2][7]
{0x0F12, 0x01E6}, //01E6 //SARR_usGammaLutRGBIndoor[2][8]
{0x0F12, 0x0236}, //0236 //SARR_usGammaLutRGBIndoor[2][9]
{0x0F12, 0x02BA}, //02BA //SARR_usGammaLutRGBIndoor[2][10]
{0x0F12, 0x032A}, //032A //SARR_usGammaLutRGBIndoor[2][11]
{0x0F12, 0x0385}, //0385 //SARR_usGammaLutRGBIndoor[2][12]
{0x0F12, 0x03C2}, //03C2 //SARR_usGammaLutRGBIndoor[2][13]
{0x0F12, 0x03EA}, //03EA //SARR_usGammaLutRGBIndoor[2][14]
{0x0F12, 0x03FF}, //03FF //SARR_usGammaLutRGBIndoor[2][15]


//====, 0x====}=========================================================================================
//	Set, 0x AWB}
//====, 0x====}=========================================================================================
{0x002A, 0x0DA6},
{0x0F12, 0x0000}, // awbb_LowBr_NBzone
{0x0F12, 0x0000}, // awbb_LowBr0_NBzone
{0x002A, 0x0E8C},
{0x0F12, 0x0000},	//awbb_LowBr0_PatchNumZone
{0x002A, 0x0D6C},
{0x0F12, 0x0040},	//awbb_YMedMoveToYAv

// Ind, 0xoor }Gray Zone
{0x002A, 0x0B9C},
{0x0F12, 0x038F}, //awbb_IndoorGrZones_m_BGrid_0__m_left
{0x0F12, 0x039B}, //awbb_IndoorGrZones_m_BGrid_0__m_right
{0x0F12, 0x0373}, //awbb_IndoorGrZones_m_BGrid_1__m_left
{0x0F12, 0x03B0}, //awbb_IndoorGrZones_m_BGrid_1__m_right
{0x0F12, 0x0352}, //awbb_IndoorGrZones_m_BGrid_2__m_left
{0x0F12, 0x03B7}, //awbb_IndoorGrZones_m_BGrid_2__m_right
{0x0F12, 0x0334}, //awbb_IndoorGrZones_m_BGrid_3__m_left
{0x0F12, 0x03B5}, //awbb_IndoorGrZones_m_BGrid_3__m_right
{0x0F12, 0x0318}, //awbb_IndoorGrZones_m_BGrid_4__m_left
{0x0F12, 0x03B0}, //awbb_IndoorGrZones_m_BGrid_4__m_right
{0x0F12, 0x02FF}, //awbb_IndoorGrZones_m_BGrid_5__m_left
{0x0F12, 0x038D}, //awbb_IndoorGrZones_m_BGrid_5__m_right
{0x0F12, 0x02E7}, //awbb_IndoorGrZones_m_BGrid_6__m_left
{0x0F12, 0x0372}, //awbb_IndoorGrZones_m_BGrid_6__m_right
{0x0F12, 0x02D0}, //awbb_IndoorGrZones_m_BGrid_7__m_left
{0x0F12, 0x035D}, //awbb_IndoorGrZones_m_BGrid_7__m_right
{0x0F12, 0x02B5}, //awbb_IndoorGrZones_m_BGrid_8__m_left
{0x0F12, 0x0345}, //awbb_IndoorGrZones_m_BGrid_8__m_right
{0x0F12, 0x02A1}, //awbb_IndoorGrZones_m_BGrid_9__m_left
{0x0F12, 0x0331}, //awbb_IndoorGrZones_m_BGrid_9__m_right
{0x0F12, 0x028B}, //awbb_IndoorGrZones_m_BGrid_10__m_left
{0x0F12, 0x031E}, //awbb_IndoorGrZones_m_BGrid_10__m_right
{0x0F12, 0x0273}, //awbb_IndoorGrZones_m_BGrid_11__m_left
{0x0F12, 0x0309}, //awbb_IndoorGrZones_m_BGrid_11__m_right
{0x0F12, 0x025F}, //awbb_IndoorGrZones_m_BGrid_12__m_left
{0x0F12, 0x02F5}, //awbb_IndoorGrZones_m_BGrid_12__m_right
{0x0F12, 0x0250}, //awbb_IndoorGrZones_m_BGrid_13__m_left
{0x0F12, 0x02DB}, //awbb_IndoorGrZones_m_BGrid_13__m_right
{0x0F12, 0x0241}, //awbb_IndoorGrZones_m_BGrid_14__m_left
{0x0F12, 0x02C7}, //awbb_IndoorGrZones_m_BGrid_14__m_right
{0x0F12, 0x0233}, //awbb_IndoorGrZones_m_BGrid_15__m_left
{0x0F12, 0x02B9}, //awbb_IndoorGrZones_m_BGrid_15__m_right
{0x0F12, 0x0223}, //awbb_IndoorGrZones_m_BGrid_16__m_left
{0x0F12, 0x02AB}, //awbb_IndoorGrZones_m_BGrid_16__m_right
{0x0F12, 0x0217}, //awbb_IndoorGrZones_m_BGrid_17__m_left
{0x0F12, 0x02A2}, //awbb_IndoorGrZones_m_BGrid_17__m_right
{0x0F12, 0x0207}, //awbb_IndoorGrZones_m_BGrid_18__m_left
{0x0F12, 0x0294}, //awbb_IndoorGrZones_m_BGrid_18__m_right
{0x0F12, 0x01FA}, //awbb_IndoorGrZones_m_BGrid_19__m_left
{0x0F12, 0x0289}, //awbb_IndoorGrZones_m_BGrid_19__m_right
{0x0F12, 0x01EA}, //awbb_IndoorGrZones_m_BGrid_20__m_left
{0x0F12, 0x0281}, //awbb_IndoorGrZones_m_BGrid_20__m_right
{0x0F12, 0x01DD}, //awbb_IndoorGrZones_m_BGrid_21__m_left
{0x0F12, 0x027B}, //awbb_IndoorGrZones_m_BGrid_21__m_right
{0x0F12, 0x01D0}, //awbb_IndoorGrZones_m_BGrid_22__m_left
{0x0F12, 0x0273}, //awbb_IndoorGrZones_m_BGrid_22__m_right
{0x0F12, 0x01C3}, //awbb_IndoorGrZones_m_BGrid_23__m_left
{0x0F12, 0x026A}, //awbb_IndoorGrZones_m_BGrid_23__m_right
{0x0F12, 0x01B6}, //awbb_IndoorGrZones_m_BGrid_24__m_left
{0x0F12, 0x0265}, //awbb_IndoorGrZones_m_BGrid_24__m_right
{0x0F12, 0x01AB}, //awbb_IndoorGrZones_m_BGrid_25__m_left
{0x0F12, 0x025B}, //awbb_IndoorGrZones_m_BGrid_25__m_right
{0x0F12, 0x01A1}, //awbb_IndoorGrZones_m_BGrid_26__m_left
{0x0F12, 0x0254}, //awbb_IndoorGrZones_m_BGrid_26__m_right
{0x0F12, 0x0198}, //awbb_IndoorGrZones_m_BGrid_27__m_left
{0x0F12, 0x024B}, //awbb_IndoorGrZones_m_BGrid_27__m_right
{0x0F12, 0x0192}, //awbb_IndoorGrZones_m_BGrid_28__m_left
{0x0F12, 0x0242}, //awbb_IndoorGrZones_m_BGrid_28__m_right
{0x0F12, 0x0191}, //awbb_IndoorGrZones_m_BGrid_29__m_left
{0x0F12, 0x023A}, //awbb_IndoorGrZones_m_BGrid_29__m_right
{0x0F12, 0x0192}, //awbb_IndoorGrZones_m_BGrid_30__m_left
{0x0F12, 0x0222}, //awbb_IndoorGrZones_m_BGrid_30__m_right
{0x0F12, 0x01C5}, //awbb_IndoorGrZones_m_BGrid_31__m_left
{0x0F12, 0x01DF}, //awbb_IndoorGrZones_m_BGrid_31__m_right
{0x0F12, 0x0000}, //awbb_IndoorGrZones_m_BGrid_32__m_left
{0x0F12, 0x0000}, //awbb_IndoorGrZones_m_BGrid_32__m_right
{0x0F12, 0x0000}, //awbb_IndoorGrZones_m_BGrid_33__m_left
{0x0F12, 0x0000}, //awbb_IndoorGrZones_m_BGrid_33__m_right


        //  param_end	awbb_IndoorGrZones_m_BGrid
{0x002A, 0x0C3C},
{0x0F12, 0x0004},	//awbb_IndoorGrZones_m_GridStep
{0x0F12, 0x0000},
{0x0F12, 0x0022}, //awbb_IndoorGrZones_m_GridSz
{0x0F12, 0x0000},
{0x0F12, 0x010F}, //awbb_IndoorGrZones_m_Boffs   																												  //
{0x0F12, 0x0000},
{0x0F12, 0x0020}, //awbb_IndoorGrZones_y_low     																													//
{0x0F12, 0x0000},
{0x002A, 0x0C50},
{0x0F12, 0x00E0}, //awbb_IndoorGrZones_y_high  																												    //
{0x0F12, 0x0000},

// Out, 0xdoor} Gray Zone
{0x0F12, 0x025E},	//0264 //awbb_OutdoorGrZones_m_BGrid_0__m_left
{0x0F12, 0x0282},	//0279 //awbb_OutdoorGrZones_m_BGrid_0__m_right
{0x0F12, 0x0240},	//0250 //awbb_OutdoorGrZones_m_BGrid_1__m_left
{0x0F12, 0x0298},	//0287 //awbb_OutdoorGrZones_m_BGrid_1__m_right
{0x0F12, 0x022A},	//0244 //awbb_OutdoorGrZones_m_BGrid_2__m_left
{0x0F12, 0x029A},	//0287 //awbb_OutdoorGrZones_m_BGrid_2__m_right
{0x0F12, 0x021A},	//0235 //awbb_OutdoorGrZones_m_BGrid_3__m_left
{0x0F12, 0x029A},	//0289 //awbb_OutdoorGrZones_m_BGrid_3__m_right
{0x0F12, 0x0206},	//0225 //awbb_OutdoorGrZones_m_BGrid_4__m_left
{0x0F12, 0x0298},	//0287 //awbb_OutdoorGrZones_m_BGrid_4__m_right
{0x0F12, 0x01FE},	//0213 //awbb_OutdoorGrZones_m_BGrid_5__m_left
{0x0F12, 0x028C},	//0286 //awbb_OutdoorGrZones_m_BGrid_5__m_right
{0x0F12, 0x01FA},	//0202 //awbb_OutdoorGrZones_m_BGrid_6__m_left
{0x0F12, 0x0278},	//027A //awbb_OutdoorGrZones_m_BGrid_6__m_right
{0x0F12, 0x01F8},	//01F3 //awbb_OutdoorGrZones_m_BGrid_7__m_left
{0x0F12, 0x0266},	//0272 //awbb_OutdoorGrZones_m_BGrid_7__m_right
{0x0F12, 0x0214},	//01E9 //awbb_OutdoorGrZones_m_BGrid_8__m_left
{0x0F12, 0x0238},	//0269 //awbb_OutdoorGrZones_m_BGrid_8__m_right
{0x0F12, 0x0000},	//01E2 //awbb_OutdoorGrZones_m_BGrid_9__m_left
{0x0F12, 0x0000},	//0263 //awbb_OutdoorGrZones_m_BGrid_9__m_right
{0x0F12, 0x0000},	//01E0 //awbb_OutdoorGrZones_m_BGrid_10__m_left
{0x0F12, 0x0000},	//025A //awbb_OutdoorGrZones_m_BGrid_10__m_right
{0x0F12, 0x0000},	//01E1 //awbb_OutdoorGrZones_m_BGrid_11__m_left
{0x0F12, 0x0000},	//0256 //awbb_OutdoorGrZones_m_BGrid_11__m_right
{0x0F12, 0x0000},	//01EE //awbb_OutdoorGrZones_m_BGrid_12__m_left
{0x0F12, 0x0000},	//0251 //awbb_OutdoorGrZones_m_BGrid_12__m_right
{0x0F12, 0x0000},	//01F8 //awbb_OutdoorGrZones_m_BGrid(26)
{0x0F12, 0x0000},	//024A //awbb_OutdoorGrZones_m_BGrid(27)
{0x0F12, 0x0000},	//020D //awbb_OutdoorGrZones_m_BGrid(28)
{0x0F12, 0x0000},	//0231 //awbb_OutdoorGrZones_m_BGrid(29)
{0x0F12, 0x0000},	//0000 //awbb_OutdoorGrZones_m_BGrid(30)
{0x0F12, 0x0000},	//0000 //awbb_OutdoorGrZones_m_BGrid(31)
{0x0F12, 0x0000},	//0000 //awbb_OutdoorGrZones_m_BGrid(32)
{0x0F12, 0x0000},	//0000 //awbb_OutdoorGrZones_m_BGrid(33)


//  pa, 0xram_}WRITE 70000CC6  B2end	awbb_OutdoorGrZones_m_BGrid
{0x002A, 0x0CB8},
{0x0F12, 0x0004}, // awbb_OutdoorGrZones_m_GridStep
{0x0F12, 0x0000},
{0x0F12, 0x0009}, // awbb_OutdoorGrZones_m_GridSz
{0x0F12, 0x0000},
{0x0F12, 0x0210}, // awbb_OutdoorGrZones_m_Boffs
{0x0F12, 0x0000},
{0x0F12, 0x0020}, // awbb_OutdoorGrZones_y_low
{0x0F12, 0x0000},
{0x002A, 0x0CCC},
{0x0F12, 0x00C0}, // awbb_OutdoorGrZones_y_high
{0x0F12, 0x0000},

//  7-, 0x3. L}ow Br grey zone
// par, 0xam_ } C4start	awbb_LowBrGrZones_m_BGrid

{0x0F12, 0x031F}, // awbb_LowBrGrZones_m_BGrid_0__m_left
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_0__m_right
{0x0F12, 0x02FC}, // awbb_LowBrGrZones_m_BGrid_1__m_left
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_1__m_right
{0x0F12, 0x02D9}, // awbb_LowBrGrZones_m_BGrid_2__m_left
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_2__m_right
{0x0F12, 0x02B6}, // awbb_LowBrGrZones_m_BGrid_3__m_left
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_3__m_right
{0x0F12, 0x0293}, // awbb_LowBrGrZones_m_BGrid_4__m_left
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_4__m_right
{0x0F12, 0x0270}, // awbb_LowBrGrZones_m_BGrid_5__m_left
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_5__m_right
{0x0F12, 0x024E}, // awbb_LowBrGrZones_m_BGrid_6__m_left
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_6__m_right
{0x0F12, 0x022B}, // awbb_LowBrGrZones_m_BGrid_7__m_left
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_7__m_right
{0x0F12, 0x0208}, // awbb_LowBrGrZones_m_BGrid_8__m_left
{0x0F12, 0x048A}, // awbb_LowBrGrZones_m_BGrid_8__m_right
{0x0F12, 0x01E5}, // awbb_LowBrGrZones_m_BGrid_9__m_left
{0x0F12, 0x0455}, // awbb_LowBrGrZones_m_BGrid_9__m_right
{0x0F12, 0x01C2}, // awbb_LowBrGrZones_m_BGrid_10__m_left
{0x0F12, 0x041F}, // awbb_LowBrGrZones_m_BGrid_10__m_right
{0x0F12, 0x019F}, // awbb_LowBrGrZones_m_BGrid_11__m_left
{0x0F12, 0x03EA}, // awbb_LowBrGrZones_m_BGrid_11__m_right
{0x0F12, 0x017D}, // awbb_LowBrGrZones_m_BGrid_12__m_left
{0x0F12, 0x03B4}, // awbb_LowBrGrZones_m_BGrid_12__m_right
{0x0F12, 0x015A}, // awbb_LowBrGrZones_m_BGrid_13__m_left
{0x0F12, 0x037F}, // awbb_LowBrGrZones_m_BGrid_13__m_right
{0x0F12, 0x0137}, // awbb_LowBrGrZones_m_BGrid_14__m_left
{0x0F12, 0x0349}, // awbb_LowBrGrZones_m_BGrid_14__m_right
{0x0F12, 0x0130}, // awbb_LowBrGrZones_m_BGrid_15__m_left
{0x0F12, 0x0314}, // awbb_LowBrGrZones_m_BGrid_15__m_right
{0x0F12, 0x012F}, // awbb_LowBrGrZones_m_BGrid_16__m_left
{0x0F12, 0x02DE}, // awbb_LowBrGrZones_m_BGrid_16__m_right
{0x0F12, 0x012F}, // awbb_LowBrGrZones_m_BGrid_17__m_left
{0x0F12, 0x02B1}, // awbb_LowBrGrZones_m_BGrid_17__m_right
{0x0F12, 0x012E}, // awbb_LowBrGrZones_m_BGrid_18__m_left
{0x0F12, 0x028B}, // awbb_LowBrGrZones_m_BGrid_18__m_right
{0x0F12, 0x012D}, // awbb_LowBrGrZones_m_BGrid_19__m_left
{0x0F12, 0x0265}, // awbb_LowBrGrZones_m_BGrid_19__m_right
{0x0F12, 0x012C}, // awbb_LowBrGrZones_m_BGrid_20__m_left
{0x0F12, 0x023F}, // awbb_LowBrGrZones_m_BGrid_20__m_right
{0x0F12, 0x012C}, // awbb_LowBrGrZones_m_BGrid_21__m_left
{0x0F12, 0x0219}, // awbb_LowBrGrZones_m_BGrid_21__m_right
{0x0F12, 0x012B}, // awbb_LowBrGrZones_m_BGrid_22__m_left
{0x0F12, 0x01F3}, // awbb_LowBrGrZones_m_BGrid_22__m_right
{0x0F12, 0x012A}, // awbb_LowBrGrZones_m_BGrid_23__m_left
{0x0F12, 0x01CD}, // awbb_LowBrGrZones_m_BGrid_23__m_right
{0x0F12, 0x0000}, // awbb_LowBrGrZones_m_BGrid_24__m_left
{0x0F12, 0x0000}, // awbb_LowBrGrZones_m_BGrid_24__m_right


//  42, 0xpara}m_end	awbb_LowBrGrZones_m_BGrid 																												  //
{0x0F12, 0x0005},
{0x0F12, 0x0000}, //awbb_LowBrGrZones_m_GridStep																																			  //
{0x0F12, 0x0018},
{0x0F12, 0x0000}, //awbb_LowBrGrZones_m_GridSz  																																				//
{0x0F12, 0x00AF},
{0x0F12, 0x0000}, //awbb_LowBrGrZones_m_Boffs  																																			  //
{0x0F12, 0x0002},
{0x0F12, 0x0000}, //A awbb_LowBrGrZones_y_low  																																			  //
{0x002A, 0x0D48},
{0x0F12, 0x00E0},
{0x0F12, 0x0000}, //awbb_LowBrGrZones_y_high    																																				//

// Lo, 0xwtem}p circle                            																											  //
{0x0F12, 0x032F},
{0x0F12, 0x0000}, //awbb_CrclLowT_R_c          																																				//
{0x0F12, 0x017A},
{0x0F12, 0x0000}, //awbb_CrclLowT_B_c         																																				  //
{0x0F12, 0x7300},
{0x0F12, 0x0000}, //awbb_CrclLowT_Rad_c       																																				  //
{0x0F12, 0x000A},
{0x0F12, 0x0000}, //awbb_CrclLowT_y_low   																																				      //
{0x002A, 0x0D60},
{0x0F12, 0x00E0},
{0x0F12, 0x0000}, //awbb_CrclLowT_y_high 																																				      //
{0x002A, 0x0D82},
{0x0F12, 0x0001},
        //awbb_ByPass_LowTempMode  																																				  //

// Duk, 0xs ad}d 																																													//
{0x002A, 0x0D8E},
{0x0F12, 0x0002}, // awbb_GridEnable 																																									//

// Grid coefficients and Contrants 																																	//
{0x002A, 0x0DCE},
{0x0F12, 0xFFE0}, //0000, //awbb_GridCorr_R_0__0_ 																																							//
{0x0F12, 0xFFE0}, //FFE0, //FFD0, //0000, //awbb_GridCorr_R_0__1_ 																																							//
{0x0F12, 0x0000}, //40, //awbb_GridCorr_R_0__2_ 																																							//
{0x0F12, 0x0000}, //awbb_GridCorr_R_0__3_ 																																							//
{0x0F12, 0x0020}, //awbb_GridCorr_R_0__4_ 																																							//
{0x0F12, 0x0060}, //40, //60, //awbb_GridCorr_R_0__5_ 																																							//

{0x0F12, 0xFFE0}, //0000, //awbb_GridCorr_R_1__0_ 																																							//
{0x0F12, 0xFFE0}, //FFE0, //FFD0, //0000, //awbb_GridCorr_R_1__1_ 																																							//
{0x0F12, 0x0000}, //40, //awbb_GridCorr_R_1__2_ 																																							//
{0x0F12, 0x0000}, //awbb_GridCorr_R_1__3_ 																																							//
{0x0F12, 0x0020}, //awbb_GridCorr_R_1__4_ 																																							//
{0x0F12, 0x0060}, //40, //60, //awbb_GridCorr_R_1__5_ 																																							//

{0x0F12, 0xFFE0}, //0000, //awbb_GridCorr_R_2__0_ 																																							//
{0x0F12, 0xFFD8}, //FFE0, //FFD0, //0000, //awbb_GridCorr_R_2__1_ 																																							//
{0x0F12, 0x0000}, //40, //awbb_GridCorr_R_2__2_ 																																							//
{0x0F12, 0x0000}, //awbb_GridCorr_R_2__3_ 																																							//
{0x0F12, 0x0000}, //awbb_GridCorr_R_2__4_ 																																							//
{0x0F12, 0x0030}, //40, //60, //awbb_GridCorr_R_2__5_ 																																							//

{0x0F12, 0x0004}, //0000, //awbb_GridCorr_B_0__0_ 																																							//
{0x0F12, 0x0000}, //FFE0, //awbb_GridCorr_B_0__1_ 																																							//
{0x0F12, 0x0000}, //0040, //awbb_GridCorr_B_0__2_ 																																							//
{0x0F12, 0xFFC0}, //awbb_GridCorr_B_0__3_ 																																							//
{0x0F12, 0xFFB0}, //awbb_GridCorr_B_0__4_ 																																							//
{0x0F12, 0xFF30}, //FF00, //awbb_GridCorr_B_0__5_ 																																							//

{0x0F12, 0x0004}, //0000, //awbb_GridCorr_B_1__0_ 																																							//
{0x0F12, 0x0000}, //FFE0, //awbb_GridCorr_B_1__1_ 																																							//
{0x0F12, 0x0000}, //0040, //awbb_GridCorr_B_1__2_ 																																							//
{0x0F12, 0xFFC0}, //awbb_GridCorr_B_1__3_ 																																							//
{0x0F12, 0xFFB0}, //awbb_GridCorr_B_1__4_ 																																							//
{0x0F12, 0xFF30}, //FF00, //awbb_GridCorr_B_1__5_ 																																							//

{0x0F12, 0x0004}, //0000, //awbb_GridCorr_B_2__0_ 																																							//
{0x0F12, 0x0000}, //FFE0, //awbb_GridCorr_B_2__1_ 																																							//
{0x0F12, 0x0000}, //0040, //awbb_GridCorr_B_2__2_ 																																							//
{0x0F12, 0xFFC0}, //awbb_GridCorr_B_2__3_ 																																							//
{0x0F12, 0xFFB0}, //awbb_GridCorr_B_2__4_ 																																							//
{0x0F12, 0xFF30}, //FF00, //awbb_GridCorr_B_2__5_ 																																							//

{0x0F12, 0x02C6}, //awbb_GridConst_1_0_
{0x0F12, 0x0335}, //awbb_GridConst_1_1_
{0x0F12, 0x03B3}, //awbb_GridConst_1_2_
{0x0F12, 0x1021}, //awbb_GridConst_2_0
{0x0F12, 0x107E}, //awbb_GridConst_2_1
{0x0F12, 0x113E}, //awbb_GridConst_2_2
{0x0F12, 0x117C}, //awbb_GridConst_2_3
{0x0F12, 0x11C2}, //awbb_GridConst_2_4
{0x0F12, 0x120B}, //awbb_GridConst_2_5   																																								//

{0x0F12, 0x00B3}, //awbb_GridCoeff_R_1 																																								//
{0x0F12, 0x00B7}, //awbb_GridCoeff_B_1 																																								//
{0x0F12, 0x00D3}, //awbb_GridCoeff_R_2 																																								//
{0x0F12, 0x0091}, //awbb_GridCoeff_B_2 																																								//

// Whi, 0xte L}ocus 																																											//
{0x002A, 0x0D66},
{0x0F12, 0x0133}, // awbb_IntcR																																												//
{0x0F12, 0x010F}, // awbb_IntcB																																												//
{0x002A, 0x0D74},
{0x0F12, 0x052A}, // awbb_MvEq_RBthresh 																																								//

// Gam, 0xut T}hresholds
{0x002A, 0x0DAE},
{0x0F12, 0x0036}, //awbb_GamutWidthThr2 																																	  					  //
{0x0F12, 0x001C}, //awbb_GamutHeightThr2																																	  					  //
{0x002A, 0x0DAA},
{0x0F12, 0x071A}, //awbb_GamutWidthThr1  																																							//
{0x0F12, 0x03A4}, //awbb_GamutHeightThr1 																																							//

// Sce, 0xneDe}tection Thresholds
{0x002A, 0x0D92},
{0x0F12, 0x0BB8}, //awbb_SunnyBr		              																																			//
{0x0F12, 0x0096}, //awbb_Sunny_NBzone	          																																			//
{0x002A, 0x0E86},
{0x0F12, 0x0216}, //awbb_OutdoorWP_r             																																			//
{0x0F12, 0x029F}, //awbb_OutdoorWP_b             																																			//
{0x002A, 0x0D96},
{0x0F12, 0x0BB7}, //awbb_CloudyBr		          																																			  //
{0x0F12, 0x0096}, //awbb_Cloudy_NBzone	        																																			  //
{0x002A, 0x0DB2},
{0x0F12, 0x00DA}, //awbb_CloudyRB              																																			  //
{0x002A, 0x0D9A},
{0x0F12, 0x000A}, //awbb_Cloudy_BdivRzone        																																			//
{0x002A, 0x0DB4},
{0x0F12, 0x0459}, //awbb_LowTempRB               																																			//
{0x002A, 0x0DA4},
{0x0F12, 0x000E}, //awbb_LowTemp_RBzone       																																				  //
{0x002A, 0x0D64},
{0x0F12, 0x0032}, //awbb_DarkBr		             																																				//
{0x002A, 0x0DA6},
{0x0F12, 0x001E}, //awbb_LowBr_NBzone	         																																				//
{0x002A, 0x0D9C},
{0x0F12, 0x001B}, //awbb_MacbethGamut_WidthZone  																																			//
{0x0F12, 0x000E}, //awbb_MacbethGamut_HeightZone 																																			//
{0x0F12, 0x0008}, //awbb_MacbethGamut_WidthZone2 																																			//
{0x0F12, 0x0004}, //awbb_MacbethGamut_HeightZone2																																			//

// AWB, 0x Deb}ug.(Outdoor Pink)																																				  //
{0x002A, 0x0E30},
{0x0F12, 0x0000}, //awbb_OutdoorFltrSz (outdoor WB moving average filtering)
{0x002A, 0x0E84},
{0x0F12, 0x0000}, //awbb_UseFixedOutdoor

//  Us, 0xeInv}alidOutdoor option																																					//
{0x002A, 0x0D88},
{0x0F12, 0x0001}, //awbb_Use_InvalidOutDoor																																						//

// AWB, 0x inp}ut Y-Filter setting    																																		//
{0x002A, 0x0C48},
{0x0F12, 0x0020}, //awbb_IndoorGrZones_y_low  																																					//
{0x002A, 0x0C50},
{0x0F12, 0x00E0}, //awbb_IndoorGrZones_y_high 																																					//
{0x002A, 0x0CC4},
{0x0F12, 0x0020}, //awbb_OutdoorGrZones_y_low 																																					//
{0x002A, 0x0CCC},
{0x0F12, 0x00C0}, //awbb_OutdoorGrZones_y_high																																					//

// awb, 0xb_Ch}romaClassifyEn, default : enable 																																						//
{0x002A, 0x0DC2},
{0x0F12, 0x0030}, //awbb_GnCurPntImmunity 																																							//
{0x0F12, 0x00C8}, //awbb_GnFarPntImmunity 																																							//
{0x0F12, 0x012C}, //awbb_GnCurPntLongJump 																																							//
{0x0F12, 0x0258}, //awbb_GainsMaxMove     																																							//
{0x0F12, 0x0003}, //awbb_GnMinMatchToJump 																																							//


//====, 0x====}=================
//	Set, 0x CCM}
//====, 0x====}=================
// CCM, 0x Sta}rt Address
{0x002A, 0x06D0},
{0x0F12, 0x2800}, //TVAR_wbt_pBaseCcmsAddr[0] 																																					//
{0x0F12, 0x7000},
{0x0F12, 0x2824}, //TVAR_wbt_pBaseCcmsAddr[1]																																					//
{0x0F12, 0x7000},
{0x0F12, 0x2848}, //TVAR_wbt_pBaseCcmsAddr[2]																																					//
{0x0F12, 0x7000},
{0x0F12, 0x286C}, //TVAR_wbt_pBaseCcmsAddr[3] 																																					//
{0x0F12, 0x7000},
{0x0F12, 0x2890}, //TVAR_wbt_pBaseCcmsAddr[4]																																					//
{0x0F12, 0x7000},
{0x0F12, 0x28B4}, //TVAR_wbt_pBaseCcmsAddr[5] 																																					//
{0x0F12, 0x7000},
{0x002A, 0x06EC},
{0x0F12, 0x28D8}, //TVAR_wbt_pOutdoorCcm      																																					//
{0x0F12, 0x7000},

//  pa, 0xram_}start	TVAR_wbt_pBaseCcms  																																//
{0x002A, 0x2800},
{0x0F12, 0x01E1},	//01FB
{0x0F12, 0xFFC4},	//FF9C
{0x0F12, 0xFFF8},	//FFFF
{0x0F12, 0x0101},	//0137
{0x0F12, 0x014C},	//0113
{0x0F12, 0xFF55},	//FF6F
{0x0F12, 0xFF5B},	//FF21
{0x0F12, 0x0205},	//0194
{0x0F12, 0xFF17},	//FF69
{0x0F12, 0xFEFE},	//FF14
{0x0F12, 0x01B6},	//0158
{0x0F12, 0x0107},	//015D
{0x0F12, 0xFFDB},	//FFF2
{0x0F12, 0xFFDB},	//FFF1
{0x0F12, 0x01D1},	//0179
{0x0F12, 0x0163},	//017C
{0x0F12, 0xFF9E},	//FFC3
{0x0F12, 0x01B3},	//0197

{0x0F12, 0x01E1},	//01FB
{0x0F12, 0xFFC4},	//FF9C
{0x0F12, 0xFFF8},	//FFFF
{0x0F12, 0x0101},	//0137
{0x0F12, 0x014C},	//0113
{0x0F12, 0xFF55},	//FF6F
{0x0F12, 0xFF5B},	//FF21
{0x0F12, 0x0205},	//0194
{0x0F12, 0xFF17},	//FF69
{0x0F12, 0xFEFE},	//FF14
{0x0F12, 0x01B6},	//0158
{0x0F12, 0x0107},	//015D
{0x0F12, 0xFFDB},	//FFF2
{0x0F12, 0xFFDB},	//FFF1
{0x0F12, 0x01D1},	//0179
{0x0F12, 0x0163},	//017C
{0x0F12, 0xFF9E},	//FFC3
{0x0F12, 0x01B3},	//0197

{0x0F12, 0x01E1},	//01FB
{0x0F12, 0xFFC4},	//FF9C
{0x0F12, 0xFFF8},	//FFFF
{0x0F12, 0x0101},	//0137
{0x0F12, 0x014C},	//0113
{0x0F12, 0xFF55},	//FF6F
{0x0F12, 0xFF5B},	//FF21
{0x0F12, 0x0205},	//0194
{0x0F12, 0xFF17},	//FF69
{0x0F12, 0xFEFE},	//FF14
{0x0F12, 0x01B6},	//0158
{0x0F12, 0x0107},	//015D
{0x0F12, 0xFFDB},	//FFF2
{0x0F12, 0xFFDB},	//FFF1
{0x0F12, 0x01D1},	//0179
{0x0F12, 0x0163},	//017C
{0x0F12, 0xFF9E},	//FFC3
{0x0F12, 0x01B3},	//0197

{0x0F12, 0x01FB}, //0207, //01FB,	//01FB
{0x0F12, 0xFFA9}, //FFA1, //FFA9,	//FF9C
{0x0F12, 0xFFEA}, //FFE4, //FFEA,	//FFFF
{0x0F12, 0x013C}, //0134,	//0137
{0x0F12, 0x0140}, //0133,	//0113
{0x0F12, 0xFF53}, //FF5D,	//FF6F
{0x0F12, 0xFE7A}, //FE7A,	//FF21
{0x0F12, 0x017D}, //017D,	//0194
{0x0F12, 0xFEED}, //FEED,	//FF69
{0x0F12, 0xFF39}, //FF39,	//FF14
{0x0F12, 0x01D6}, //01D6,	//0158
{0x0F12, 0x00C4}, //00C4,	//015D
{0x0F12, 0xFFC0}, //FFCE,	//FFF2
{0x0F12, 0xFFBF}, //FFCD,	//FFF1
{0x0F12, 0x01CD}, //01B7,	//0179
{0x0F12, 0x0182}, //0176,	//017C
{0x0F12, 0xFF91}, //FFBD,	//FFC3
{0x0F12, 0x01AA}, //0191,	//0197

{0x0F12, 0x01C5},	//0207	//01FB
{0x0F12, 0xFF9F},	//FF7E	//FF9C
{0x0F12, 0xFFE5},	//FFAD	//FFFF
{0x0F12, 0x00E2},	//00CB	//0137
{0x0F12, 0x010E},	//010B	//0113
{0x0F12, 0xFF62},	//FF28	//FF6F
{0x0F12, 0xFF03},	//FF17	//FF21
{0x0F12, 0x01D0},	//01B0	//0194
{0x0F12, 0xFF3E},	//FEB2	//FF69
{0x0F12, 0xFF00},	//FF50	//FF14
{0x0F12, 0x01A6},	//0184	//0158
{0x0F12, 0x00BB},	//0184	//015D
{0x0F12, 0xFFBF},	//FFE1	//FFF2
{0x0F12, 0xFFDD},	//FFE1	//FFF1
{0x0F12, 0x01F6},	//01E1	//0179
{0x0F12, 0x00CB},	//0127	//017C
{0x0F12, 0xFF94},	//FF50	//FFC3
{0x0F12, 0x019E},	//0127	//0197

{0x0F12, 0x01D2},	//01D2	//01D2	//01D0
{0x0F12, 0xFFC2},	//FFC2	//FFC2	//FFB4
{0x0F12, 0xFFFC},	//FFFC	//FFFC	//000C
{0x0F12, 0x00E8},	//00E8	//011E	//0122
{0x0F12, 0x0126},	//0126	//011D	//0103
{0x0F12, 0xFF83},	//FF83	//FF86	//FF9B
{0x0F12, 0xFE7A}, //FF3E,	//FEE7	//FE78	//FF33
{0x0F12, 0x017D}, //01B1,	//01EA	//017B	//01C5
{0x0F12, 0xFEED}, //FF3E,	//FF5A	//FEEB	//FF33
{0x0F12, 0xFF8A}, //FF38	//FF16   C
{0x0F12, 0x01F9}, //01D5	//015A
{0x0F12, 0x005B}, //00C3	//015F
{0x0F12, 0xFFCA}, //FFCF	//FFE0   B
{0x0F12, 0xFFA3}, //FFCE	//FFDF
{0x0F12, 0x01DA}, //01B8	//0197
{0x0F12, 0x0108}, //0178	//0178   M
{0x0F12, 0xFFB3}, //FFBF	//FFBF
{0x0F12, 0x01DD}, //0193	//0193

{0x0F12, 0x01D2}, //01F1,	//01FA	//01D2	//01E0 // outdoor CCM
{0x0F12, 0xFFC2}, //FFB0,	//FF9F	//FFC2	//FFBF
{0x0F12, 0xFFFC}, //FFEF,	//FFF7	//FFFC	//FFFD
{0x0F12, 0x00F4},	//00F4	//00E8	//00F5
{0x0F12, 0x0139},	//0139	//0126	//0139
{0x0F12, 0xFF64},	//FF64	//FF83	//FF74
{0x0F12, 0xFEEC}, //FEDD,	//FECE	//FEE7	//FEEC
{0x0F12, 0x01FD}, //0216,	//0214	//01EA	//01FD
{0x0F12, 0xFF8E}, //FF3A,	//FF4B	//FF5A	//FF8E
{0x0F12, 0xFEF4}, //FEFE,
{0x0F12, 0x01BD}, //01B6,
{0x0F12, 0x010A}, //0107,
{0x0F12, 0xFFA2}, //FFDB,
{0x0F12, 0xFFDE}, //FFDB,
{0x0F12, 0x0208}, //01D1,
{0x0F12, 0x0163}, //0163,
{0x0F12, 0xFF9E}, //FF9E,
{0x0F12, 0x01B3}, //01B3,

//=================================================================================================
//	Set NB
//=================================================================================================
{0x002A, 0x07EA},
{0x0F12, 0x0000},	//afit_bUseNoiseInd 0 : NB 1: Noise Index

//  pa, 0xram_}start	SARR_uNormBrInDoor 																																	//
{0x0F12, 0x000A}, //SARR_uNormBrInDoor[0] 																																							//
{0x0F12, 0x0019}, //SARR_uNormBrInDoor[1] 																																							//
{0x0F12, 0x007D}, //SARR_uNormBrInDoor[2] 																																							//
{0x0F12, 0x02BC}, //SARR_uNormBrInDoor[3] 																																							//
{0x0F12, 0x07D0}, //SARR_uNormBrInDoor[4] 																																							//

// par, 0xam_s}tart	SARR_uNormBrOutDoor 																																	//
{0x0F12, 0x000A}, //SARR_uNormBrOutDoor[0] 																																						//
{0x0F12, 0x0019}, //SARR_uNormBrOutDoor[1] 																																						//
{0x0F12, 0x007D}, //SARR_uNormBrOutDoor[2] 																																						//
{0x0F12, 0x02BC}, //SARR_uNormBrOutDoor[3] 																																						//
{0x0F12, 0x07D0}, //SARR_uNormBrOutDoor[4] 																																						//

//====, 0x====}=========================================================================================
//	Set, 0x AFI}T
//====, 0x====}=========================================================================================
// AFI, 0xT St}art Address
{0x002A, 0x0814},
{0x0F12, 0x082C}, // TVAR_afit_pBaseVals  																																							//
{0x0F12, 0x7000}, // TVAR_afit_pBaseVals  																																							//

//  pa, 0xram_}start	TVAR_afit_pBaseVals
{0x002A, 0x082C},
{0x0F12, 0x0000}, // BRIGHTNESS
{0x0F12, 0x0000}, // CONTRAST
{0x0F12, 0xFFFE}, // SATURATION
{0x0F12, 0xFFE2}, // SHARP_BLUR
{0x0F12, 0x0000}, // GLAMOUR
{0x0F12, 0x03FF}, // Disparity_iSatSat
{0x0F12, 0x03FF}, // Denoise1_iYDenThreshLow
{0x0F12, 0x03FF}, //028, // Denoise1_iYDenThreshLow_Bin
{0x0F12, 0x03FF}, // Denoise1_iYDenThreshHigh
{0x0F12, 0x03FF}, //0FF, // Denoise1_iYDenThreshHigh_Bin
{0x0F12, 0x0002}, // Denoise1_iLowWWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWWideThresh
{0x0F12, 0x000A}, // Denoise1_iLowWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWideThresh
{0x0F12, 0x03FF}, // Denoise1_iSatSat
{0x0F12, 0x03FF}, // Demosaic4_iHystGrayLow
{0x0F12, 0x0000}, // Demosaic4_iHystGrayHigh
{0x0F12, 0x0344}, // UVDenoise_iYLowThresh
{0x0F12, 0x033A}, // UVDenoise_iYHighThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVLowThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVHighThresh
{0x0F12, 0x0028}, // DSMix1_iLowLimit_Wide
{0x0F12, 0x0032}, // DSMix1_iLowLimit_Wide_Bin
{0x0F12, 0x0014}, // DSMix1_iHighLimit_Wide
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Wide_Bin
{0x0F12, 0x0050}, // DSMix1_iLowLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iLowLimit_Fine_Bin
{0x0F12, 0x0046}, // DSMix1_iHighLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Fine_Bin
{0x0F12, 0x0106}, // DSMix1_iRGBOffset
{0x0F12, 0x006F}, // DSMix1_iDemClamp
{0x0F12, 0x0C0F}, // "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"     bin: desparity low
{0x0F12, 0x0C0F}, // "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"  bin: desparity high
{0x0F12, 0x0303}, // "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"
{0x0F12, 0x0303}, // Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin
{0x0F12, 0x140A}, // "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"
{0x0F12, 0x140A}, // "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"
{0x0F12, 0x2828}, // "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"
{0x0F12, 0x0606}, // "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"
{0x0F12, 0x023F}, // "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"
{0x0F12, 0x0480}, // "Denoise1_iRadialLimitDenoise1_iLWBNoise"
{0x0F12, 0x000F}, // "Denoise1_iWideDenoise1_iWideWide"
{0x0F12, 0x030A}, // "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"
{0x0F12, 0x0003}, // "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"
{0x0F12, 0x0011}, // "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"
{0x0F12, 0x0A0F}, // "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"
{0x0F12, 0x050A}, // "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"  (negati fine   )
{0x0F12, 0x0900}, // "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"     (low fine      )
{0x0F12, 0x0000}, // "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"   (high fine    )
{0x0F12, 0x980A}, // "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"      (high low thres)
{0x0F12, 0x0005}, // "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"
{0x0F12, 0x0000}, // "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"
{0x0F12, 0x0000}, // "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"
{0x0F12, 0x0000}, // "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"
{0x0F12, 0x0000}, // "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"
{0x0F12, 0x0A00}, // "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"
{0x0F12, 0x000A}, // "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"
{0x0F12, 0x0180}, // "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"
{0x0F12, 0x0180}, // "RGBGamma2_iLinearityRGBGamma2_bLinearity"
{0x0F12, 0x0100}, // "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"
{0x0F12, 0x6E14}, // "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"
{0x0F12, 0x0180}, // "RGB2YUV_iSaturationRGB2YUV_bGainOffset"
{0x0F12, 0x0018}, //16,  RGB2YUV_iYOffset

{0x0F12, 0x0000}, // BRIGHTNESS
{0x0F12, 0x0000}, // CONTRAST
{0x0F12, 0x0000}, // SATURATION
{0x0F12, 0x0000}, // SHARP_BLUR
{0x0F12, 0x0000}, // GLAMOUR
{0x0F12, 0x03FF}, // Disparity_iSatSat
{0x0F12, 0x0016}, // Denoise1_iYDenThreshLow
{0x0F12, 0x000E}, // Denoise1_iYDenThreshLow_Bin
{0x0F12, 0x0072}, // Denoise1_iYDenThreshHigh
{0x0F12, 0x00FF}, // Denoise1_iYDenThreshHigh_Bin
{0x0F12, 0x0002}, // Denoise1_iLowWWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWWideThresh
{0x0F12, 0x000A}, // Denoise1_iLowWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWideThresh
{0x0F12, 0x03FF}, // Denoise1_iSatSat
{0x0F12, 0x03FF}, // Demosaic4_iHystGrayLow
{0x0F12, 0x0000}, // Demosaic4_iHystGrayHigh
{0x0F12, 0x0114}, // UVDenoise_iYLowThresh
{0x0F12, 0x020A}, // UVDenoise_iYHighThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVLowThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVHighThresh
{0x0F12, 0x0028}, // DSMix1_iLowLimit_Wide
{0x0F12, 0x0000}, // DSMix1_iLowLimit_Wide_Bin
{0x0F12, 0x0014}, // DSMix1_iHighLimit_Wide
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Wide_Bin
{0x0F12, 0x0050}, // DSMix1_iLowLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iLowLimit_Fine_Bin
{0x0F12, 0x0046}, // DSMix1_iHighLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Fine_Bin
{0x0F12, 0x0106}, // DSMix1_iRGBOffset
{0x0F12, 0x006F}, // DSMix1_iDemClamp
{0x0F12, 0x050F}, // "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"    bin: desparity low
{0x0F12, 0x0A0F}, // "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"
{0x0F12, 0x0203}, // "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"
{0x0F12, 0x0203}, // Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin
{0x0F12, 0x140A}, // "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"
{0x0F12, 0x140A}, // "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"
{0x0F12, 0x2828}, // "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"
{0x0F12, 0x0606}, // "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"
{0x0F12, 0x020A}, // "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"
{0x0F12, 0x0480}, // "Denoise1_iRadialLimitDenoise1_iLWBNoise"
{0x0F12, 0x000F}, // "Denoise1_iWideDenoise1_iWideWide"
{0x0F12, 0x0305}, // "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"
{0x0F12, 0x2803}, // "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"
{0x0F12, 0x2811}, // "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"
{0x0F12, 0x0A0F}, // "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"
{0x0F12, 0x050A}, // "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"
{0x0F12, 0x101E}, // "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"
{0x0F12, 0x101E}, // "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"
{0x0F12, 0x200A}, // "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"
{0x0F12, 0x0005}, // "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"
{0x0F12, 0x0400}, // "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"
{0x0F12, 0x0400}, // "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"
{0x0F12, 0x0000}, // "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"
{0x0F12, 0x0000}, // "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"
{0x0F12, 0x0A00}, // "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"
{0x0F12, 0x100A}, // "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"
{0x0F12, 0x0180}, // "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"
{0x0F12, 0x0180}, // "RGBGamma2_iLinearityRGBGamma2_bLinearity"
{0x0F12, 0x0100}, // "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"
{0x0F12, 0x8030}, // "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"
{0x0F12, 0x0180}, // "RGB2YUV_iSaturationRGB2YUV_bGainOffset"
{0x0F12, 0x0009}, //08,  RGB2YUV_iYOffset

{0x0F12, 0x0000},	//0000 BRIGHTNESS
{0x0F12, 0x0000},	//0000 CONTRAST
{0x0F12, 0x0000},	//0000 SATURATION
{0x0F12, 0x000E},	//0000 SHARP_BLUR
{0x0F12, 0x0000},	//0000 GLAMOUR
{0x0F12, 0x03FF},	//03FF Disparity_iSatSat
{0x0F12, 0x0012},	//000C Denoise1_iYDenThreshLow
{0x0F12, 0x0006},	//0006 Denoise1_iYDenThreshLow_Bin
{0x0F12, 0x006E},	//0060 Denoise1_iYDenThreshHigh
{0x0F12, 0x0050},	//0050 Denoise1_iYDenThreshHigh_Bin
{0x0F12, 0x0002},	//0002 Denoise1_iLowWWideThresh
{0x0F12, 0x000A},	//000A Denoise1_iHighWWideThresh
{0x0F12, 0x000A},	//000A Denoise1_iLowWideThresh
{0x0F12, 0x000A},	//000A Denoise1_iHighWideThresh
{0x0F12, 0x03FF},	//03FF Denoise1_iSatSat
{0x0F12, 0x03FF},	//03FF Demosaic4_iHystGrayLow
{0x0F12, 0x0000},	//0000 Demosaic4_iHystGrayHigh
{0x0F12, 0x0014},	//0014 UVDenoise_iYLowThresh
{0x0F12, 0x000A},	//000A UVDenoise_iYHighThresh
{0x0F12, 0x03FF},	//03FF UVDenoise_iUVLowThresh
{0x0F12, 0x03FF},	//03FF UVDenoise_iUVHighThresh
{0x0F12, 0x0028},	//0028 DSMix1_iLowLimit_Wide
{0x0F12, 0x0032},	//0032 DSMix1_iLowLimit_Wide_Bin
{0x0F12, 0x0014},	//0014 DSMix1_iHighLimit_Wide
{0x0F12, 0x0032},	//0032 DSMix1_iHighLimit_Wide_Bin
{0x0F12, 0x0050},	//0050 DSMix1_iLowLimit_Fine
{0x0F12, 0x0032},	//0032 DSMix1_iLowLimit_Fine_Bin
{0x0F12, 0x0010},	//0010 DSMix1_iHighLimit_Fine
{0x0F12, 0x0032},	//0032 DSMix1_iHighLimit_Fine_Bin
{0x0F12, 0x0106},	//0106 DSMix1_iRGBOffset
{0x0F12, 0x006F},	//006F DSMix1_iDemClamp
{0x0F12, 0x0202},	//0202 "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"
{0x0F12, 0x0502},	//0502 "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"
{0x0F12, 0x0102},	//0102 "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"
{0x0F12, 0x0102},	//0102 Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin
{0x0F12, 0x140A},	//140A "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"
{0x0F12, 0x140A},	//140A "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"
{0x0F12, 0x2828},	//2828 "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"
{0x0F12, 0x0606},	//0606 "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"
{0x0F12, 0x0205},	//0205 "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"
{0x0F12, 0x0480},	//0480 "Denoise1_iRadialLimitDenoise1_iLWBNoise"
{0x0F12, 0x000F},	//000F "Denoise1_iWideDenoise1_iWideWide"
{0x0F12, 0x0005},	//0005 "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"
{0x0F12, 0x2803},	//2803 "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"
{0x0F12, 0x2811},	//2811 "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"
{0x0F12, 0x0A0F},	//0A0F "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"
{0x0F12, 0x050A},	//050A "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"
{0x0F12, 0x2028},	//2020 "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"
{0x0F12, 0x2028},	//2020 "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"
{0x0F12, 0x2000},	//980A "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"
{0x0F12, 0x0007},	//0007 "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"
{0x0F12, 0x0403},	//0403 "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"
{0x0F12, 0x0402},	//0402 "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"
{0x0F12, 0x0000},	//0000 "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"
{0x0F12, 0x0203},	//0203 "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"
{0x0F12, 0x0000},	//0000 "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"
{0x0F12, 0x1006},	//1006 "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"
{0x0F12, 0x0180},	//0180 "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"
{0x0F12, 0x0180},	//0180 "RGBGamma2_iLinearityRGBGamma2_bLinearity"
{0x0F12, 0x0100},	//0100 "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"
{0x0F12, 0x803C},	//803C "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"
{0x0F12, 0x0180},	//0180 "RGB2YUV_iSaturationRGB2YUV_bGainOffset"
{0x0F12, 0x0000},	//0000  RGB2YUV_iYOffset

{0x0F12, 0x0000},	//0000 BRIGHTNESS
{0x0F12, 0x0000},	//0000 CONTRAST
{0x0F12, 0x0000},	//0000 SATURATION
{0x0F12, 0x000E},	//0000 SHARP_BLUR
{0x0F12, 0x0000},	//0000 GLAMOUR
{0x0F12, 0x03FF},	//03FF Disparity_iSatSat
{0x0F12, 0x000F},	//000A Denoise1_iYDenThreshLow
{0x0F12, 0x0006},	//0006 Denoise1_iYDenThreshLow_Bin
{0x0F12, 0x006C},	//005A Denoise1_iYDenThreshHigh
{0x0F12, 0x0050},	//0050 Denoise1_iYDenThreshHigh_Bin
{0x0F12, 0x0002},	//0002 Denoise1_iLowWWideThresh
{0x0F12, 0x000A},	//000A Denoise1_iHighWWideThresh
{0x0F12, 0x000A},	//000A Denoise1_iLowWideThresh
{0x0F12, 0x000A},	//000A Denoise1_iHighWideThresh
{0x0F12, 0x03FF},	//03FF Denoise1_iSatSat
{0x0F12, 0x03FF},	//03FF Demosaic4_iHystGrayLow
{0x0F12, 0x0000},	//0000 Demosaic4_iHystGrayHigh
{0x0F12, 0x0014},	//0014 UVDenoise_iYLowThresh
{0x0F12, 0x000A},	//000A UVDenoise_iYHighThresh
{0x0F12, 0x03FF},	//03FF UVDenoise_iUVLowThresh
{0x0F12, 0x03FF},	//03FF UVDenoise_iUVHighThresh
{0x0F12, 0x0028},	//0028 DSMix1_iLowLimit_Wide
{0x0F12, 0x0032},	//0032 DSMix1_iLowLimit_Wide_Bin
{0x0F12, 0x0014},	//0014 DSMix1_iHighLimit_Wide
{0x0F12, 0x0032},	//0032 DSMix1_iHighLimit_Wide_Bin
{0x0F12, 0x0050},	//0050 DSMix1_iLowLimit_Fine
{0x0F12, 0x0032},	//0032 DSMix1_iLowLimit_Fine_Bin
{0x0F12, 0x0010},	//0010 DSMix1_iHighLimit_Fine
{0x0F12, 0x0032},	//0032 DSMix1_iHighLimit_Fine_Bin
{0x0F12, 0x0106},	//0106 DSMix1_iRGBOffset
{0x0F12, 0x006F},	//006F DSMix1_iDemClamp
{0x0F12, 0x0202},	//0202 "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"
{0x0F12, 0x0502},	//0502 "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"
{0x0F12, 0x0102},	//0102 "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"
{0x0F12, 0x0102},	//0102 Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin
{0x0F12, 0x140A},	//140A "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"
{0x0F12, 0x140A},	//140A "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"
{0x0F12, 0x2828},	//2828 "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"
{0x0F12, 0x0606},	//0606 "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"
{0x0F12, 0x0205},	//0205 "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"
{0x0F12, 0x0480},	//0480 "Denoise1_iRadialLimitDenoise1_iLWBNoise"
{0x0F12, 0x000F},	//000F "Denoise1_iWideDenoise1_iWideWide"
{0x0F12, 0x0005},	//0005 "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"
{0x0F12, 0x2803},	//2803 "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"
{0x0F12, 0x2811},	//2811 "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"
{0x0F12, 0x0A0F},	//0A0F "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"
{0x0F12, 0x050A},	//050A "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"
{0x0F12, 0x2028},	//2020 "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"
{0x0F12, 0x2028},	//2020 "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"
{0x0F12, 0x2000},	//980A "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"
{0x0F12, 0x0007},	//0007 "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"
{0x0F12, 0x0403},	//0403 "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"
{0x0F12, 0x0402},	//0402 "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"
{0x0F12, 0x0000},	//0000 "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"
{0x0F12, 0x0203},	//0203 "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"
{0x0F12, 0x0000},	//0000 "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"
{0x0F12, 0x1006},	//1006 "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"
{0x0F12, 0x0180},	//0180 "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"
{0x0F12, 0x0180},	//0180 "RGBGamma2_iLinearityRGBGamma2_bLinearity"
{0x0F12, 0x0100},	//0100 "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"
{0x0F12, 0x803C},	//803C "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"
{0x0F12, 0x0180},	//0180 "RGB2YUV_iSaturationRGB2YUV_bGainOffset"
{0x0F12, 0x0000},	//0000  RGB2YUV_iYOffset

{0x0F12, 0x0000}, // BRIGHTNESS
{0x0F12, 0x000A}, // CONTRAST
{0x0F12, 0x0000}, // SATURATION
{0x0F12, 0x0014}, // SHARP_BLUR
{0x0F12, 0x0000}, // GLAMOUR
{0x0F12, 0x03FF}, // Disparity_iSatSat
{0x0F12, 0x000F}, // Denoise1_iYDenThreshLow
{0x0F12, 0x0006}, // Denoise1_iYDenThreshLow_Bin
{0x0F12, 0x0068}, // Denoise1_iYDenThreshHigh
{0x0F12, 0x0050}, // Denoise1_iYDenThreshHigh_Bin
{0x0F12, 0x0002}, // Denoise1_iLowWWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWWideThresh
{0x0F12, 0x000A}, // Denoise1_iLowWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWideThresh
{0x0F12, 0x03FF}, // Denoise1_iSatSat
{0x0F12, 0x03FF}, // Demosaic4_iHystGrayLow
{0x0F12, 0x0000}, // Demosaic4_iHystGrayHigh
{0x0F12, 0x0000}, // UVDenoise_iYLowThresh
{0x0F12, 0x0000}, // UVDenoise_iYHighThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVLowThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVHighThresh
{0x0F12, 0x0028}, // DSMix1_iLowLimit_Wide
{0x0F12, 0x0032}, // DSMix1_iLowLimit_Wide_Bin
{0x0F12, 0x0000}, // DSMix1_iHighLimit_Wide
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Wide_Bin
{0x0F12, 0x0030}, // DSMix1_iLowLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iLowLimit_Fine_Bin
{0x0F12, 0x0000}, // DSMix1_iHighLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Fine_Bin
{0x0F12, 0x0106}, // DSMix1_iRGBOffset
{0x0F12, 0x006F}, // DSMix1_iDemClamp
{0x0F12, 0x0202}, // "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"
{0x0F12, 0x0502}, // "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"
{0x0F12, 0x0102}, // "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"
{0x0F12, 0x0102}, // Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin
{0x0F12, 0x140A}, // "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"
{0x0F12, 0x140A}, // "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"
{0x0F12, 0x2828}, // "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"
{0x0F12, 0x0606}, // "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"
{0x0F12, 0x0205}, // "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"
{0x0F12, 0x0880}, // "Denoise1_iRadialLimitDenoise1_iLWBNoise"
{0x0F12, 0x000F}, // "Denoise1_iWideDenoise1_iWideWide"
{0x0F12, 0x0005}, // "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"
{0x0F12, 0x2803}, // "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"
{0x0F12, 0x2811}, // "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"
{0x0F12, 0x0A0F}, // "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"
{0x0F12, 0x050A}, // "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"
{0x0F12, 0x2020}, // "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"
{0x0F12, 0x2020}, // "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"
{0x0F12, 0x6400}, // "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"
{0x0F12, 0x0007}, // "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"
{0x0F12, 0x0408}, // "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"
{0x0F12, 0x0406}, // "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"
{0x0F12, 0x0000}, // "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"
{0x0F12, 0x0608}, // "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"
{0x0F12, 0x0000}, // "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"
{0x0F12, 0x1006}, // "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"
{0x0F12, 0x0180}, // "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"
{0x0F12, 0x0180}, // "RGBGamma2_iLinearityRGBGamma2_bLinearity"
{0x0F12, 0x0100}, // "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"
{0x0F12, 0x8050}, // "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"
{0x0F12, 0x0140}, // "RGB2YUV_iSaturationRGB2YUV_bGainOffset"
{0x0F12, 0x0000}, //  RGB2YUV_iYOffset

{0x0F12, 0x00FF}, // Denoise1_iUVDenThreshLow
{0x0F12, 0x00FF}, // Denoise1_iUVDenThreshHigh
{0x0F12, 0x0800}, // Denoise1_sensor_width
{0x0F12, 0x0600}, // Denoise1_sensor_height
{0x0F12, 0x0000}, // Denoise1_start_x
{0x0F12, 0x0000}, // Denoise1_start_y
{0x0F12, 0x0000}, // "Denoise1_iYDenSmoothDenoise1_iWSharp  "
{0x0F12, 0x0300}, // "Denoise1_iWWSharp Denoise1_iRadialTune  "
{0x0F12, 0x0002}, // "Denoise1_iOutputBrightnessDenoise1_binning_x  "
{0x0F12, 0x0400}, // "Denoise1_binning_yDemosaic4_iFDeriv  "
{0x0F12, 0x0106}, // "Demosaic4_iFDerivNeiDemosaic4_iSDeriv  "
{0x0F12, 0x0005}, // "Demosaic4_iSDerivNeiDemosaic4_iEnhancerG  "
{0x0F12, 0x0000}, // "Demosaic4_iEnhancerRBDemosaic4_iEnhancerV  "
{0x0F12, 0x0703}, // "Demosaic4_iDecisionThreshDemosaic4_iDesatThresh"
{0x0F12, 0x0000}, //  Demosaic4_iBypassSelect
{0x0F12, 0xFFD6},
{0x0F12, 0x53C1},
{0x0F12, 0xE1FE},
{0x0F12, 0x0001},

// Upd, 0xate }Changed Registers
{0x002A, 0x03FC},
{0x0F12, 0x0001},	//REG_TC_DBG_ReInitCmd

{0x0028, 0xD000},
{0x002A, 0xB0CC},
{0x0F12, 0x000B},	//Non contious mode
//////	END of Initial	////////////////////////////////

// MIPI
{0x0028, 0xD000},
{0x002A, 0xB0CC},
{0x0F12, 0x000B},

{0xFFFF, 0xFFFF},

};

static const struct s5k5bafx_reg mode_sensor_vt_init[] = {
// VT-Call

{0xFCFC, 0xD000},

//=================================================================================================
// ARM Go
//=================================================================================================
{0x0028, 0xD000},
{0x002A, 0x1030},
{0x0F12, 0x0000},
{0x002A, 0x0014},
{0x0F12, 0x0001},
{S5K5BAFX_TABLE_WAIT_MS, 0x0064},		//Delay

//=================================================================================================
// Trap and Patch 	 //  2008-11-18 10:15:41
//=================================================================================================
{0x0028, 0x7000},
{0x002A, 0x1668},
{0x0F12, 0xB5FE},    //70001668
{0x0F12, 0x0007},    //7000166A
{0x0F12, 0x683C},    //7000166C
{0x0F12, 0x687E},    //7000166E
{0x0F12, 0x1DA5},    //70001670
{0x0F12, 0x88A0},    //70001672
{0x0F12, 0x2800},    //70001674
{0x0F12, 0xD00B},    //70001676
{0x0F12, 0x88A8},    //70001678
{0x0F12, 0x2800},    //7000167A
{0x0F12, 0xD008},    //7000167C
{0x0F12, 0x8820},    //7000167E
{0x0F12, 0x8829},    //70001680
{0x0F12, 0x4288},    //70001682
{0x0F12, 0xD301},    //70001684
{0x0F12, 0x1A40},    //70001686
{0x0F12, 0xE000},    //70001688
{0x0F12, 0x1A08},    //7000168A
{0x0F12, 0x9001},    //7000168C
{0x0F12, 0xE001},    //7000168E
{0x0F12, 0x2019},    //70001690
{0x0F12, 0x9001},    //70001692
{0x0F12, 0x4916},    //70001694
{0x0F12, 0x466B},    //70001696
{0x0F12, 0x8A48},    //70001698
{0x0F12, 0x8118},    //7000169A
{0x0F12, 0x8A88},    //7000169C
{0x0F12, 0x8158},    //7000169E
{0x0F12, 0x4814},    //700016A0
{0x0F12, 0x8940},    //700016A2
{0x0F12, 0x0040},    //700016A4
{0x0F12, 0x2103},    //700016A6
{0x0F12, 0xF000},    //700016A8
{0x0F12, 0xF826},    //700016AA
{0x0F12, 0x88A1},    //700016AC
{0x0F12, 0x4288},    //700016AE
{0x0F12, 0xD908},    //700016B0
{0x0F12, 0x8828},    //700016B2
{0x0F12, 0x8030},    //700016B4
{0x0F12, 0x8868},    //700016B6
{0x0F12, 0x8070},    //700016B8
{0x0F12, 0x88A8},    //700016BA
{0x0F12, 0x6038},    //700016BC
{0x0F12, 0xBCFE},    //700016BE
{0x0F12, 0xBC08},    //700016C0
{0x0F12, 0x4718},    //700016C2
{0x0F12, 0x88A9},    //700016C4
{0x0F12, 0x4288},    //700016C6
{0x0F12, 0xD906},    //700016C8
{0x0F12, 0x8820},    //700016CA
{0x0F12, 0x8030},    //700016CC
{0x0F12, 0x8860},    //700016CE
{0x0F12, 0x8070},    //700016D0
{0x0F12, 0x88A0},    //700016D2
{0x0F12, 0x6038},    //700016D4
{0x0F12, 0xE7F2},    //700016D6
{0x0F12, 0x9801},    //700016D8
{0x0F12, 0xA902},    //700016DA
{0x0F12, 0xF000},    //700016DC
{0x0F12, 0xF812},    //700016DE
{0x0F12, 0x0033},    //700016E0
{0x0F12, 0x0029},    //700016E2
{0x0F12, 0x9A02},    //700016E4
{0x0F12, 0x0020},    //700016E6
{0x0F12, 0xF000},    //700016E8
{0x0F12, 0xF814},    //700016EA
{0x0F12, 0x6038},    //700016EC
{0x0F12, 0xE7E6},    //700016EE
{0x0F12, 0x1A28},    //700016F0
{0x0F12, 0x7000},    //700016F2
{0x0F12, 0x0D64},    //700016F4
{0x0F12, 0x7000},    //700016F6
{0x0F12, 0x4778},    //700016F8
{0x0F12, 0x46C0},    //700016FA
{0x0F12, 0xF004},    //700016FC
{0x0F12, 0xE51F},    //700016FE
{0x0F12, 0xA464},    //70001700
{0x0F12, 0x0000},    //70001702
{0x0F12, 0x4778},    //70001704
{0x0F12, 0x46C0},    //70001706
{0x0F12, 0xC000},    //70001708
{0x0F12, 0xE59F},    //7000170A
{0x0F12, 0xFF1C},    //7000170C
{0x0F12, 0xE12F},    //7000170E
{0x0F12, 0x6009},    //70001710
{0x0F12, 0x0000},    //70001712
{0x0F12, 0x4778},    //70001714
{0x0F12, 0x46C0},    //70001716
{0x0F12, 0xC000},    //70001718
{0x0F12, 0xE59F},    //7000171A
{0x0F12, 0xFF1C},    //7000171C
{0x0F12, 0xE12F},    //7000171E
{0x0F12, 0x622F},    //70001720
{0x0F12, 0x0000},    //70001722
{0x002A, 0x2080},
{0x0F12, 0xB510},    //70002080
{0x0F12, 0xF000},    //70002082
{0x0F12, 0xF8F4},    //70002084
{0x0F12, 0xBC10},    //70002086
{0x0F12, 0xBC08},    //70002088
{0x0F12, 0x4718},    //7000208A
{0x0F12, 0xB5F0},    //7000208C
{0x0F12, 0xB08B},    //7000208E
{0x0F12, 0x0006},    //70002090
{0x0F12, 0x2000},    //70002092
{0x0F12, 0x9004},    //70002094
{0x0F12, 0x6835},    //70002096
{0x0F12, 0x6874},    //70002098
{0x0F12, 0x68B0},    //7000209A
{0x0F12, 0x900A},    //7000209C
{0x0F12, 0x68F0},    //7000209E
{0x0F12, 0x9009},    //700020A0
{0x0F12, 0x4F7D},    //700020A2
{0x0F12, 0x8979},    //700020A4
{0x0F12, 0x084A},    //700020A6
{0x0F12, 0x88A8},    //700020A8
{0x0F12, 0x88A3},    //700020AA
{0x0F12, 0x4298},    //700020AC
{0x0F12, 0xD300},    //700020AE
{0x0F12, 0x0018},    //700020B0
{0x0F12, 0xF000},    //700020B2
{0x0F12, 0xF907},    //700020B4
{0x0F12, 0x9007},    //700020B6
{0x0F12, 0x0021},    //700020B8
{0x0F12, 0x0028},    //700020BA
{0x0F12, 0xAA04},    //700020BC
{0x0F12, 0xF000},    //700020BE
{0x0F12, 0xF909},    //700020C0
{0x0F12, 0x9006},    //700020C2
{0x0F12, 0x88A8},    //700020C4
{0x0F12, 0x2800},    //700020C6
{0x0F12, 0xD102},    //700020C8
{0x0F12, 0x27FF},    //700020CA
{0x0F12, 0x1C7F},    //700020CC
{0x0F12, 0xE047},    //700020CE
{0x0F12, 0x88A0},    //700020D0
{0x0F12, 0x2800},    //700020D2
{0x0F12, 0xD101},    //700020D4
{0x0F12, 0x2700},    //700020D6
{0x0F12, 0xE042},    //700020D8
{0x0F12, 0x8820},    //700020DA
{0x0F12, 0x466B},    //700020DC
{0x0F12, 0x8198},    //700020DE
{0x0F12, 0x8860},    //700020E0
{0x0F12, 0x81D8},    //700020E2
{0x0F12, 0x8828},    //700020E4
{0x0F12, 0x8118},    //700020E6
{0x0F12, 0x8868},    //700020E8
{0x0F12, 0x8158},    //700020EA
{0x0F12, 0xA802},    //700020EC
{0x0F12, 0xC803},    //700020EE
{0x0F12, 0xF000},    //700020F0
{0x0F12, 0xF8F8},    //700020F2
{0x0F12, 0x9008},    //700020F4
{0x0F12, 0x8ABA},    //700020F6
{0x0F12, 0x9808},    //700020F8
{0x0F12, 0x466B},    //700020FA
{0x0F12, 0x4342},    //700020FC
{0x0F12, 0x9202},    //700020FE
{0x0F12, 0x8820},    //70002100
{0x0F12, 0x8198},    //70002102
{0x0F12, 0x8860},    //70002104
{0x0F12, 0x81D8},    //70002106
{0x0F12, 0x980A},    //70002108
{0x0F12, 0x9903},    //7000210A
{0x0F12, 0xF000},    //7000210C
{0x0F12, 0xF8EA},    //7000210E
{0x0F12, 0x9A02},    //70002110
{0x0F12, 0x17D1},    //70002112
{0x0F12, 0x0E09},    //70002114
{0x0F12, 0x1889},    //70002116
{0x0F12, 0x1209},    //70002118
{0x0F12, 0x4288},    //7000211A
{0x0F12, 0xDD1F},    //7000211C
{0x0F12, 0x8820},    //7000211E
{0x0F12, 0x466B},    //70002120
{0x0F12, 0x8198},    //70002122
{0x0F12, 0x8860},    //70002124
{0x0F12, 0x81D8},    //70002126
{0x0F12, 0x980A},    //70002128
{0x0F12, 0x9903},    //7000212A
{0x0F12, 0xF000},    //7000212C
{0x0F12, 0xF8DA},    //7000212E
{0x0F12, 0x9001},    //70002130
{0x0F12, 0x8828},    //70002132
{0x0F12, 0x466B},    //70002134
{0x0F12, 0x8118},    //70002136
{0x0F12, 0x8868},    //70002138
{0x0F12, 0x8158},    //7000213A
{0x0F12, 0x980A},    //7000213C
{0x0F12, 0x9902},    //7000213E
{0x0F12, 0xF000},    //70002140
{0x0F12, 0xF8D0},    //70002142
{0x0F12, 0x8AB9},    //70002144
{0x0F12, 0x9A08},    //70002146
{0x0F12, 0x4351},    //70002148
{0x0F12, 0x17CA},    //7000214A
{0x0F12, 0x0E12},    //7000214C
{0x0F12, 0x1851},    //7000214E
{0x0F12, 0x120A},    //70002150
{0x0F12, 0x9901},    //70002152
{0x0F12, 0xF000},    //70002154
{0x0F12, 0xF8B6},    //70002156
{0x0F12, 0x0407},    //70002158
{0x0F12, 0x0C3F},    //7000215A
{0x0F12, 0xE000},    //7000215C
{0x0F12, 0x2700},    //7000215E
{0x0F12, 0x8820},    //70002160
{0x0F12, 0x466B},    //70002162
{0x0F12, 0xAA05},    //70002164
{0x0F12, 0x8198},    //70002166
{0x0F12, 0x8860},    //70002168
{0x0F12, 0x81D8},    //7000216A
{0x0F12, 0x8828},    //7000216C
{0x0F12, 0x8118},    //7000216E
{0x0F12, 0x8868},    //70002170
{0x0F12, 0x8158},    //70002172
{0x0F12, 0xA802},    //70002174
{0x0F12, 0xC803},    //70002176
{0x0F12, 0x003B},    //70002178
{0x0F12, 0xF000},    //7000217A
{0x0F12, 0xF8BB},    //7000217C
{0x0F12, 0x88A1},    //7000217E
{0x0F12, 0x88A8},    //70002180
{0x0F12, 0x003A},    //70002182
{0x0F12, 0xF000},    //70002184
{0x0F12, 0xF8BE},    //70002186
{0x0F12, 0x0004},    //70002188
{0x0F12, 0xA804},    //7000218A
{0x0F12, 0xC803},    //7000218C
{0x0F12, 0x9A09},    //7000218E
{0x0F12, 0x9B07},    //70002190
{0x0F12, 0xF000},    //70002192
{0x0F12, 0xF8AF},    //70002194
{0x0F12, 0xA806},    //70002196
{0x0F12, 0xC805},    //70002198
{0x0F12, 0x0021},    //7000219A
{0x0F12, 0xF000},    //7000219C
{0x0F12, 0xF8B2},    //7000219E
{0x0F12, 0x6030},    //700021A0
{0x0F12, 0xB00B},    //700021A2
{0x0F12, 0xBCF0},    //700021A4
{0x0F12, 0xBC08},    //700021A6
{0x0F12, 0x4718},    //700021A8
{0x0F12, 0xB5F1},    //700021AA
{0x0F12, 0x9900},    //700021AC
{0x0F12, 0x680C},    //700021AE
{0x0F12, 0x493A},    //700021B0
{0x0F12, 0x694B},    //700021B2
{0x0F12, 0x698A},    //700021B4
{0x0F12, 0x4694},    //700021B6
{0x0F12, 0x69CD},    //700021B8
{0x0F12, 0x6A0E},    //700021BA
{0x0F12, 0x4F38},    //700021BC
{0x0F12, 0x42BC},    //700021BE
{0x0F12, 0xD800},    //700021C0
{0x0F12, 0x0027},    //700021C2
{0x0F12, 0x4937},    //700021C4
{0x0F12, 0x6B89},    //700021C6
{0x0F12, 0x0409},    //700021C8
{0x0F12, 0x0C09},    //700021CA
{0x0F12, 0x4A35},    //700021CC
{0x0F12, 0x1E92},    //700021CE
{0x0F12, 0x6BD2},    //700021D0
{0x0F12, 0x0412},    //700021D2
{0x0F12, 0x0C12},    //700021D4
{0x0F12, 0x429F},    //700021D6
{0x0F12, 0xD801},    //700021D8
{0x0F12, 0x0020},    //700021DA
{0x0F12, 0xE031},    //700021DC
{0x0F12, 0x001F},    //700021DE
{0x0F12, 0x434F},    //700021E0
{0x0F12, 0x0A3F},    //700021E2
{0x0F12, 0x42A7},    //700021E4
{0x0F12, 0xD301},    //700021E6
{0x0F12, 0x0018},    //700021E8
{0x0F12, 0xE02A},    //700021EA
{0x0F12, 0x002B},    //700021EC
{0x0F12, 0x434B},    //700021EE
{0x0F12, 0x0A1B},    //700021F0
{0x0F12, 0x42A3},    //700021F2
{0x0F12, 0xD303},    //700021F4
{0x0F12, 0x0220},    //700021F6
{0x0F12, 0xF000},    //700021F8
{0x0F12, 0xF88C},    //700021FA
{0x0F12, 0xE021},    //700021FC
{0x0F12, 0x0029},    //700021FE
{0x0F12, 0x4351},    //70002200
{0x0F12, 0x0A09},    //70002202
{0x0F12, 0x42A1},    //70002204
{0x0F12, 0xD301},    //70002206
{0x0F12, 0x0028},    //70002208
{0x0F12, 0xE01A},    //7000220A
{0x0F12, 0x0031},    //7000220C
{0x0F12, 0x4351},    //7000220E
{0x0F12, 0x0A09},    //70002210
{0x0F12, 0x42A1},    //70002212
{0x0F12, 0xD304},    //70002214
{0x0F12, 0x0220},    //70002216
{0x0F12, 0x0011},    //70002218
{0x0F12, 0xF000},    //7000221A
{0x0F12, 0xF87B},    //7000221C
{0x0F12, 0xE010},    //7000221E
{0x0F12, 0x491E},    //70002220
{0x0F12, 0x8C89},    //70002222
{0x0F12, 0x000A},    //70002224
{0x0F12, 0x4372},    //70002226
{0x0F12, 0x0A12},    //70002228
{0x0F12, 0x42A2},    //7000222A
{0x0F12, 0xD301},    //7000222C
{0x0F12, 0x0030},    //7000222E
{0x0F12, 0xE007},    //70002230
{0x0F12, 0x4662},    //70002232
{0x0F12, 0x434A},    //70002234
{0x0F12, 0x0A12},    //70002236
{0x0F12, 0x42A2},    //70002238
{0x0F12, 0xD302},    //7000223A
{0x0F12, 0x0220},    //7000223C
{0x0F12, 0xF000},    //7000223E
{0x0F12, 0xF869},    //70002240
{0x0F12, 0x4B16},    //70002242
{0x0F12, 0x4D18},    //70002244
{0x0F12, 0x8D99},    //70002246
{0x0F12, 0x1FCA},    //70002248
{0x0F12, 0x3AF9},    //7000224A
{0x0F12, 0xD00A},    //7000224C
{0x0F12, 0x2001},    //7000224E
{0x0F12, 0x0240},    //70002250
{0x0F12, 0x8468},    //70002252
{0x0F12, 0x0220},    //70002254
{0x0F12, 0xF000},    //70002256
{0x0F12, 0xF85D},    //70002258
{0x0F12, 0x9900},    //7000225A
{0x0F12, 0x6008},    //7000225C
{0x0F12, 0xBCF8},    //7000225E
{0x0F12, 0xBC08},    //70002260
{0x0F12, 0x4718},    //70002262
{0x0F12, 0x8D19},    //70002264
{0x0F12, 0x8469},    //70002266
{0x0F12, 0x9900},    //70002268
{0x0F12, 0x6008},    //7000226A
{0x0F12, 0xE7F7},    //7000226C
{0x0F12, 0xB570},    //7000226E
{0x0F12, 0x2200},    //70002270
{0x0F12, 0x490E},    //70002272
{0x0F12, 0x480E},    //70002274
{0x0F12, 0x2401},    //70002276
{0x0F12, 0xF000},    //70002278
{0x0F12, 0xF852},    //7000227A
{0x0F12, 0x0022},    //7000227C
{0x0F12, 0x490D},    //7000227E
{0x0F12, 0x480D},    //70002280
{0x0F12, 0x2502},    //70002282
{0x0F12, 0xF000},    //70002284
{0x0F12, 0xF84C},    //70002286
{0x0F12, 0x490C},    //70002288
{0x0F12, 0x480D},    //7000228A
{0x0F12, 0x002A},    //7000228C
{0x0F12, 0xF000},    //7000228E
{0x0F12, 0xF847},    //70002290
{0x0F12, 0xBC70},    //70002292
{0x0F12, 0xBC08},    //70002294
{0x0F12, 0x4718},    //70002296
{0x0F12, 0x0D64},    //70002298
{0x0F12, 0x7000},    //7000229A
{0x0F12, 0x0470},    //7000229C
{0x0F12, 0x7000},    //7000229E
{0x0F12, 0xA120},    //700022A0
{0x0F12, 0x0007},    //700022A2
{0x0F12, 0x0402},    //700022A4
{0x0F12, 0x7000},    //700022A6
{0x0F12, 0x14A0},    //700022A8
{0x0F12, 0x7000},    //700022AA
{0x0F12, 0x208D},    //700022AC
{0x0F12, 0x7000},    //700022AE
{0x0F12, 0x622F},    //700022B0
{0x0F12, 0x0000},    //700022B2
{0x0F12, 0x1669},    //700022B4
{0x0F12, 0x7000},    //700022B6
{0x0F12, 0x6445},    //700022B8
{0x0F12, 0x0000},    //700022BA
{0x0F12, 0x21AB},    //700022BC
{0x0F12, 0x7000},    //700022BE
{0x0F12, 0x2AA9},    //700022C0
{0x0F12, 0x0000},    //700022C2
{0x0F12, 0x4778},    //700022C4
{0x0F12, 0x46C0},    //700022C6
{0x0F12, 0xC000},    //700022C8
{0x0F12, 0xE59F},    //700022CA
{0x0F12, 0xFF1C},    //700022CC
{0x0F12, 0xE12F},    //700022CE
{0x0F12, 0x5F49},    //700022D0
{0x0F12, 0x0000},    //700022D2
{0x0F12, 0x4778},    //700022D4
{0x0F12, 0x46C0},    //700022D6
{0x0F12, 0xC000},    //700022D8
{0x0F12, 0xE59F},    //700022DA
{0x0F12, 0xFF1C},    //700022DC
{0x0F12, 0xE12F},    //700022DE
{0x0F12, 0x5FC7},    //700022E0
{0x0F12, 0x0000},    //700022E2
{0x0F12, 0x4778},    //700022E4
{0x0F12, 0x46C0},    //700022E6
{0x0F12, 0xC000},    //700022E8
{0x0F12, 0xE59F},    //700022EA
{0x0F12, 0xFF1C},    //700022EC
{0x0F12, 0xE12F},    //700022EE
{0x0F12, 0x5457},    //700022F0
{0x0F12, 0x0000},    //700022F2
{0x0F12, 0x4778},    //700022F4
{0x0F12, 0x46C0},    //700022F6
{0x0F12, 0xC000},    //700022F8
{0x0F12, 0xE59F},    //700022FA
{0x0F12, 0xFF1C},    //700022FC
{0x0F12, 0xE12F},    //700022FE
{0x0F12, 0x5FA3},    //70002300
{0x0F12, 0x0000},    //70002302
{0x0F12, 0x4778},    //70002304
{0x0F12, 0x46C0},    //70002306
{0x0F12, 0xC000},    //70002308
{0x0F12, 0xE59F},    //7000230A
{0x0F12, 0xFF1C},    //7000230C
{0x0F12, 0xE12F},    //7000230E
{0x0F12, 0x51F9},    //70002310
{0x0F12, 0x0000},    //70002312
{0x0F12, 0x4778},    //70002314
{0x0F12, 0x46C0},    //70002316
{0x0F12, 0xF004},    //70002318
{0x0F12, 0xE51F},    //7000231A
{0x0F12, 0xA464},    //7000231C
{0x0F12, 0x0000},    //7000231E
{0x0F12, 0x4778},    //70002320
{0x0F12, 0x46C0},    //70002322
{0x0F12, 0xC000},    //70002324
{0x0F12, 0xE59F},    //70002326
{0x0F12, 0xFF1C},    //70002328
{0x0F12, 0xE12F},    //7000232A
{0x0F12, 0xA007},    //7000232C
{0x0F12, 0x0000},    //7000232E
{0x0F12, 0x6546},    //70002330
{0x0F12, 0x2062},    //70002332
{0x0F12, 0x3120},    //70002334
{0x0F12, 0x3220},    //70002336
{0x0F12, 0x3130},    //70002338
{0x0F12, 0x0030},    //7000233A
{0x0F12, 0xE010},    //7000233C
{0x0F12, 0x0208},    //7000233E
{0x0F12, 0x0058},    //70002340
{0x0F12, 0x0000},    //70002342
// End of Trap and Patch (Last : 70002342h)
// Total Size 896 (0x0380)

{0x0028, 0xD000},
{0x002A, 0x1000},
{0x0F12, 0x0001},


{0x0028, 0x7000},
{0x002A, 0x1662},
{0x0F12, 0x03B0},
{0x0F12, 0x03B0},


{0x0028, 0x7000},
{0x002A, 0x1658},
{0x0F12, 0x9C40},
{0x0F12, 0x0000},
{0x0F12, 0x9C40},
{0x0F12, 0x0000},


{0x0028, 0x7000},
{0x002A, 0x0ADC},
{0x0F12, 0x0AF0},	//setot_uOnlineClocksDiv40
{0x002A, 0x0AE2},
{0x0F12, 0x222E},	//setot_usSetRomWaitStateThreshold4KHz

{0x002A, 0x0B94},
{0x0F12, 0x0580},	//awbb_GainsInit_0_:R
{0x0F12, 0x0400},	//awbb_GainsInit_1_:G
{0x0F12, 0x05F0},	//awbb_GainsInit_2_:B
{0x002A, 0x04A0},
{0x0F12, 0x8000},	//lt_uLeiInit:AE start
{0x002A, 0x049A},
{0x0F12, 0x00FA},	//lt_uMinExp   0.5ms·̠º¯°䞠

//=================================================================================================
// Set CIS/APS/Analog
//=================================================================================================
// This registers are for FACTORY ONLY. If you change it without prior notification,
// YOU are RESPONSIBLE for the FAILURE that will happen in the future.
//=================================================================================================
{0x0028, 0xD000},
{0x002A, 0xF106},
{0x0F12, 0x0001},	//L-OB non sub-sampling: revised by Ana 080128
{0x002A, 0xF206},
{0x0F12, 0x0001},	//F-OB non sub-sampling: revised by Ana 080128

//WRITE	D000F260	0000	   tgr_auto_exp (shutter on off:0b shutter off): revised by Ana 080112
{0x002A, 0xC202},
{0x0F12, 0x0700},	//tgr_coarse_integration_time[15:0]: revised by Ana 080115

{0x002A, 0xF260},
{0x0F12, 0x0001}, 	//TGR_AUTO EXP OFF

{0x002A, 0xF414},
{0x0F12, 0x0030}, 	//aig_adc_sat[7:0] (aig_adc_sat[7:4]+1)*33mV + 690mV

{0x002A, 0xC204},
{0x0F12, 0x0100},	//tgr_analogue_code_global[12:0] Analog gain X8
{0x002A, 0xF402},
{0x0F12, 0x0092},	//aig_offset[7:0] : revised by Ana 080425
{0x0F12, 0x007F},	//aig_offset2[7:0]: revised by Ana 080425

{0x002A, 0xF700},
{0x0F12, 0x0040},	//bpradlc_control[7:0]: revised by Ana 080529
// bpradlc_nactive_pedestal[7:5],bpradlc_passthrough[1],bpradlc_bypass[0]
{0x002A, 0xF708},
{0x0F12, 0x0000},	//bpradlc_f_adlc_tune_r[7:0]: revised by TG 080529
{0x0F12, 0x0000},	//bpradlc_f_adlc_tune_gr[7:0]: revised by Tg 080529
{0x0F12, 0x0000},	//bpradlc_f_adlc_tune_gb[7:0]: revised by TG 080529
{0x0F12, 0x0000},	//bpradlc_f_adlc_tune_b[7:0]: revised by TG 080529
{0x0F12, 0x0000},	//bpradlc_f_adlc_tune_total[7:0]: revised by TG 080529
{0x0F12, 0x0040},	//bpradlc_line_adlc_tune_r[7:0]: revised by Ana 080425
{0x0F12, 0x0040},	//bpradlc_line_adlc_tune_gr[7:0]: revised by Ana 080425
{0x0F12, 0x0040},	//bpradlc_line_adlc_tune_gb[7:0]: revised by Ana 080425
{0x0F12, 0x0040},	//bpradlc_line_adlc_tune_b[7:0]: revised by Ana 080425
{0x0F12, 0x0040},	//bpradlc_line_adlc_tune_total[7:0]: revised by Ana 080425
{0x0F12, 0x0001},	//bpradlc_adlc_en[7:0]: revised by TG 080529
// bpradlc_f_l_adlc_en[2],bpradlc_f_adlc_en[1],bpradlc_h_adlc_en[0]
{0x0F12, 0x0015},	// bpradlc_adlc_option[7:0]: revised by TG 080425
// bpradlc_h_adlc_ch_sel[4],
// bpradlc_max_data_clip_sel[3:2],bpradlc_f_adlc_max_data_clip_sel[1:0]
{0x0F12, 0x0001},	// bpradlc_adlc_bpr_en[7:0] adlc_bpr_enable: revised by Ana 080112
{0x0F12, 0x0040},	// bpradlc_adlc_bpr_thresh[7:0]: 080425

{0x002A, 0xF48A},
{0x0F12, 0x0048},	// aig_ld_ptr7[9:0] line OB start addr(04h~48h): revised by Ana 080911
{0x002A, 0xF10A},
{0x0F12, 0x008B},	// tgr_h_desc0_addr_end[10:0] line OB end addr(47h~8Bh): revised by Ana 080911
// line OB start - line OB end = 67d

{0x002A, 0xF900},
{0x0F12, 0x0067},	// cfpn_config: revised by Ana 080425
// cfpn_ref_gain[9:7],cpfn_ref_lines[6:5],cfpn_hbinning[4],cfpn_output_direct[3],
// cfpn_overflow_protect[2],cpfn_passthrough[1],cfpn_bypass[0]
{0x002A, 0xF406},
{0x0F12, 0x0092},	// aig_cfpn_ref_offset[7:0] : revised by Ana 080425
{0x0F12, 0x007F},	// aig_cfpn_ref_offset2[7:0]: revised by Ana 080425
{0x0F12, 0x0003},	// aig_cfpn_ref_gain[7:0]: revised by Ana 080425

{0x0F12, 0x0003},	// aig_ld_ctrl[1:0] aig_ldb_en[1], aig_ld_en[0]
{0x0F12, 0x0003}, // aig_row_id_ctrl[1:0]
{0x002A, 0xF442},
{0x0F12, 0x0000},	// aig_vavg[0]
{0x0F12, 0x0000},	// aig_havg[0]=1b @H-avg mode: revised by Ana 080116
{0x002A, 0xF448},
{0x0F12, 0x0000},	// aig_sl_off[0]
{0x002A, 0xF456},
{0x0F12, 0x0001},	// aig_blst_en[0]
{0x0F12, 0x0010},	// aig_blst_en_cintr[15:0]
{0x0F12, 0x0000},	// aig_dshut_en[0]

{0x002A, 0xF41A},
{0x0F12, 0x00FF},	// aig_comp_bias[7:0] aig_comp2_bias[7:4], aig_comp1_bias[3:0]: revised by Ana 081013
{0x0F12, 0x0003},	// aig_pix_bias[3:0]

{0x002A, 0xF420},
{0x0F12, 0x0030},	// aig_clp_lvl[7:0]: revised by Ana 080910(refer to 6AA)
{0x002A, 0xF410},
{0x0F12, 0x0001},	// aig_clp_sl_ctrl[0]

{0x0F12, 0x0000},	// aig_cds_test[7:0], aig_cds_test[1]=1b @H-avg mode current save: revised by Ana 080116
{0x002A, 0xF416},
{0x0F12, 0x0001},	// aig_rmp_option[7:0]
{0x002A, 0xF424},
{0x0F12, 0x0000},	// aig_ref_option[7:0], aig_ref_option[0]=1b @H-avg mode odd cds off: revised by Ana 080116
{0x002A, 0xF422},
{0x0F12, 0x0000},	// aig_monit[7:0]

{0x002A, 0xF41E},
{0x0F12, 0x0000},	// aig_pd_pix[0]
{0x002A, 0xF428},
{0x0F12, 0x0000},	// aig_pd_cp[1:0] aig_pd_ncp[1], aig_pd_cp[0]
{0x0F12, 0x0000},	// aig_pd_reg_pix[0]
{0x0F12, 0x0000},	// aig_pd_reg_rg[0]
{0x002A, 0xF430},
{0x0F12, 0x0000},	// aig_pd_reg_tgsl[0]
{0x0F12, 0x0000},	// aig_pd_reg_ntg[0]

{0x0F12, 0x0008},	// aig_rosc_tune_cp[3:0]: revised by Ana 080418
{0x0F12, 0x0005},	// aig_rosc_tune_ncp[3:0]
{0x0F12, 0x000F},	// aig_cp_capa[3:0]
{0x0F12, 0x0001},	// aig_reg_tune_pix[7:0]
{0x0F12, 0x0040},	// aig_reg_tune_rg[7:0]
{0x0F12, 0x0040},	// aig_reg_tune_tgsl[7:0]
{0x0F12, 0x0010}, // aig_reg_tune_ntg[7:0]

{0x002A, 0xF4D6},
{0x0F12, 0x0090}, // aig_v15_tune[7:0]: revised by Ana 081010
// aig_v15_tune[7:4]=7h @sensor only Mode
// aig_v15_tune[7:4]=9h @ISP 7.5fps Mode
// aig_v15_tune[7:4]=Dh @ISP 15fps Mode

{0x0F12, 0x0000},	//aig_vreg_sel[7:0] [9]h_test, [1]reg_sw_stby, [0]aig_reg_sel

{0x002A, 0xF47C},
{0x0F12, 0x000C},	//aig_ld_ptr0[4:0]
{0x0F12, 0x0000},	//aig_ld_ptr1[8:0]: revised by Ana 081010
{0x002A, 0xF49A},
{0x0F12, 0x0008},	//aig_sla_ptr0[3:0]: revised by Ana 080911
{0x0F12, 0x0000},	//aig_sla_ptr1[8:0]: revised by Ana 081010
{0x002A, 0xF4A2},
{0x0F12, 0x0008},	//aig_slb_ptr0[7:0]: revised by Ana 080911
{0x0F12, 0x0000},	//aig_slb_ptr1[8:0]: revised by Ana 081010
{0x002A, 0xF4B2},
{0x0F12, 0x0013},	//aig_rxa_ptr0[6:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x0000},	//aig_rxa_ptr1[9:0]: revised by Ana 081010
{0x0F12, 0x0013},	//aig_rxb_ptr0[6:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x0000},	//aig_rxb_ptr1[9:0]: revised by Ana 081010
{0x002A, 0xF4AA},
{0x0F12, 0x009B},	//aig_txa_ptr0[8:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x00FB},	//aig_txa_ptr1[8:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x009B},	//aig_txb_ptr0[9:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x00FB},	//aig_txb_ptr1[9:0]: revised by Ana 081016 for CFPN
{0x002A, 0xF474},
{0x0F12, 0x0017},	//aig_s3_ptr0[6:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x005F},	//aig_s3_ptr1[8:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x0017},	//aig_s4_ptr0[6:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x008F},	//aig_s4_ptr1[8:0]: revised by Ana 081016 for CFPN

{0x002A, 0xF48C},
{0x0F12, 0x0017},	//aig_clp_sl_ptr0[6:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x009B},	//aig_clp_sl_ptr1[8:0]: revised by Ana 081016 for CFPN
{0x002A, 0xF4C8},
{0x0F12, 0x0163},	//aig_off_en_ptr0[9:0]: revised by Ana 081016 for CFPN
{0x0F12, 0x0193},	//aig_rmp_en_ptr0[9:0]: revised by Ana 081016 for CFPN
{0x002A, 0xF490},
{0x0F12, 0x0191},	//aig_comp_en_ptr0[9:0]: revised by Ana 081016 for CFPN

{0x002A, 0xF418},
{0x0F12, 0x0083},	//aig_dbs_option[7:0]: revised by Ana 081010

{0x002A, 0xF454},
{0x0F12, 0x0001}, 	// aig_power_save[0]: revised by Ana 081229

{0x002A, 0xF702},
{0x0F12, 0x0081},
{0x002A, 0xF4D2},
{0x0F12, 0x0000},

//For ESD Check
{0x0028, 0x7000},
//Set FPN Gain Input
{0x002A, 0x1176},
{0x0F12, 0x0020},	// fpn_GainInput[0]
{0x0F12, 0x0040},	// fpn_GainInput[1]
{0x0F12, 0x0080},	// fpn_GainInput[2]
{0x0F12, 0x0100},	// fpn_GainInput[3]
{0x0F12, 0x0014},	// fpn_GainOutput[0]
{0x0F12, 0x000A},	// fpn_GainOutput[1]
{0x0F12, 0x0008},	// fpn_GainOutput[2]
{0x0F12, 0x0004},	// fpn_GainOutput[3]

// CFPN Canceller
{0x002A, 0x116C},
{0x0F12, 0x0000}, 	// REF Gain
{0x0F12, 0x0000},	// OvflProtect
{0x0F12, 0x0000},	// bypassThrough
{0x0F12, 0x0000},	// bypass
{0x0F12, 0x0002}, 	// fpn.CollectMethod           0 : Center	1 : Right	2: LEFT
{0x002A, 0x0AE8},
{0x0F12, 0x0000},	// setot_bSendFpnToISP = ??

// sensor aig table setting   sunkyu start.
{0x002A, 0x10EE},
{0x0F12, 0x0000}, 	// senHal_SenRegsModes3_usRegCount   0 value --> not use aig table 3
// below register is needed for caculating forbidden and non-linear area.
// because senHal_SenRegsModes3_usRegCount is 0, below value does not write to HW.
{0x002A, 0x10F2},
{0x0F12, 0x0000}, 	//senHal_SenRegsModes3_pSenModesRegsArray3[0]	real register 0xD000F45A}
{0x002A, 0x1152},
{0x0F12, 0x0030}, 	//senHal_SenRegsModes3_pSenModesRegsArray3[48] real register 0xD000F4BA}
{0x0F12, 0x0028}, 	//senHal_SenRegsModes3_pSenModesRegsArray3[49] real register 0xD000F4BC}
{0x0F12, 0x0030}, 	//senHal_SenRegsModes3_pSenModesRegsArray3[50] real register 0xD000F4BE}
{0x002A, 0x1148},
{0x0F12, 0x00FB}, 	//senHal_SenRegsModes3_pSenModesRegsArray3[43] real register 0xD000F4B0}
{0x002A, 0x1144},
{0x0F12, 0x00FB}, 	//senHal_SenRegsModes3_pSenModesRegsArray3[41] real register 0xD000F4AC}
// this register is needed to cac forbidden area, this register have to be large for forbidden.
{0x002A, 0x1150},
{0x0F12, 0x01F4}, 	//senHal_SenRegsModes3_pSenModesRegsArray3[47] real register 0xD000F4B8}
                //sensor aig table setting  sunkyu end

{0x002A, 0x1084},
{0x0F12, 0x0000},	//senHal_bUseAnalogBinning
{0x0F12, 0x0000},	//senHal_bUseAnalogVerAvg

// Set AE Target
{0x002A, 0x0F4C},
{0x0F12, 0x003C}, //3A TVAR_ae_BrAve

{0x002A, 0x0478},
{0x0F12, 0x0114},
{0x0F12, 0x00EB}, //ae boundary


//================================================================================================
// Set Frame Rate
//=================================================================================================
{0x002A, 0x0484},
{0x0F12, 0x410A},	//8214	//uMaxExp1
{0x0F12, 0x0000},
{0x002A, 0x048C},
{0x0F12, 0x8214},	//045A	//uMaxExp2
{0x0F12, 0x0000},	//0001
{0x0F12, 0xA122},	//545A	//uMaxExp3
{0x0F12, 0x0000},	//0001
{0x002A, 0x0488},
{0x0F12, 0xf424},	//C4B4	//E45A	//uMaxExp4
{0x0F12, 0x0000},	//0004	//0001
{0x002A, 0x043A},
{0x0F12, 0x01D0}, //1B0	//lt_uMaxAnGain0
{0x0F12, 0x01E0}, //1C0	//lt_uMaxAnGain0_1
{0x002A, 0x0494},
{0x0F12, 0x0300}, //2B0	//lt_uMaxAnGain1
{0x0F12, 0x0E00}, //650, //580, //650, //B00, //650 //650 //580	//lt_uMaxAnGain2
{0x0f12, 0x0100}, //100
{0x002A, 0x0F52},
{0x0F12, 0x000F}, 	//ae_StatMode

{0x002A, 0x0E98}, //    0180 -> 0270    //bp_uMaxBrightnessFactor
{0x0F12, 0x02B0},
{0x002A, 0x0E9E}, //    0180 -> 0270    //bp_uMinBrightnessFactor
{0x0F12, 0x0290},

//1. Auto Flicker 60Hz Start
{0x002A, 0x0B2E},
{0x0F12, 0x0001}, // AFC_Default60Hz	 Auto Flicker 60Hz start 0: Auto Flicker 50Hz start
{0x002A, 0x03F8},
{0x0F12, 0x007F}, // REG_TC_DBG_AutoAlgEnBits	                  default : 007F




//=====================================================================================================================
{S5K5BAFX_TABLE_WAIT_MS, 0x000a},	// Wait10mSec

//=================================================================================================
//	Set PLL
//=================================================================================================
// External CLOCK (MCLK)
{0x002A, 0x01B8},
{0x0F12, 0x5DC0},    	//REG_TC_IPRM_InClockLSBs                   	2   700001B8
{0x0F12, 0x0000},    	//REG_TC_IPRM_InClockMSBs                   	2   700001BA

// Parallel or MIP Selection
{0x002A, 0x01C6},
{0x0F12, 0x0001},    	//REG_TC_IPRM_UseNPviClocks                 	2   700001C6
{0x0F12, 0x0001},    	//REG_TC_IPRM_UseNMipiClocks                	2   700001C8
{0x0F12, 0x0000},    	//REG_TC_IPRM_bBlockInternalPllCalc         	2   700001CA	//0:Auto

// System Clock 0 (System : 24Mhz, PCLK : 48Mhz)
{0x002A, 0x01CC},
{0x0F12, 0x1770},    	//REG_TC_IPRM_OpClk4KHz_0                   	2   700001CC	//1770:24Mhz
{0x0F12, 0x2EE0},    	//REG_TC_IPRM_MinOutRate4KHz_0              	2   700001CE	//2EE0:48Mhz
{0x0F12, 0x2EE0},    	//REG_TC_IPRM_MaxOutRate4KHz_0              	2   700001D0    //2EE0:48Mhz

// System Clock 1 (System : 35Mhz, PCLK : 70Mhz)
{0x002A, 0x01D2},
{0x0F12, 0x1B58}, //222E,    	//REG_TC_IPRM_OpClk4KHz_1                   	2   700001D2	//222E:35Mhz
{0x0F12, 0x2EE0}, //445C,    	//REG_TC_IPRM_MinOutRate4KHz_1              	2   700001D4	//445C:70Mhz
{0x0F12, 0x2EE0}, //445C,    	//REG_TC_IPRM_MaxOutRate4KHz_1              	2   700001D6	//445C:70Mhz



{0x002A, 0x01DE},
{0x0F12, 0x0001},    	//REG_TC_IPRM_UseRegsAPI                    	2   700001DE
{0x0F12, 0x0001},    	//REG_TC_IPRM_InitParamsUpdated             	2   700001E0
{S5K5BAFX_TABLE_WAIT_MS, 0x0064}, //Delay 100ms




//=================================================================================================
// Crop
//=================================================================================================
{0x002A, 0x01FA},
{0x0F12, 0x0640},    	//REG_TC_GP_PrevReqInputWidth               	2   700001FA
{0x0F12, 0x04B0},    	//REG_TC_GP_PrevReqInputHeight              	2   700001FC
{0x0F12, 0x0000},    	//REG_TC_GP_PrevInputWidthOfs               	2   700001FE
{0x0F12, 0x0000},    	//REG_TC_GP_PrevInputHeightOfs              	2   70000200

//=================================================================================================
// Set Preview Config
//=================================================================================================
// Preview Config 0 (1600x1200 fixed 15fps)
{0x002A, 0x0242},
{0x0F12, 0x0280},    	//REG_0TC_PCFG_usWidth                      	2   70000242	//0640:1600
{0x0F12, 0x01E0},    	//REG_0TC_PCFG_usHeight                     	2   70000244    //04B0:1200
{0x0F12, 0x0005},    	//REG_0TC_PCFG_Format                       	2   70000246
{0x0F12, 0x2EE0}, //445C,    	//REG_0TC_PCFG_usMaxOut4KHzRate             	2   70000248
{0x0F12, 0x2EE0}, //445C,    	//REG_0TC_PCFG_usMinOut4KHzRate             	2   7000024A
{0x0F12, 0x0052},    	//REG_0TC_PCFG_PVIMask                      	2   7000024C
{0x0F12, 0x0001},    	//REG_0TC_PCFG_uClockInd                    	2   7000024E
{0x0F12, 0x0002},    	//REG_0TC_PCFG_usFrTimeType                 	2   70000250
{0x0F12, 0x0002},    	//REG_0TC_PCFG_FrRateQualityType            	2   70000252	//1:Bining, 2:No Bining
{0x0F12, 0x04E2},    	//REG_0TC_PCFG_usMaxFrTimeMsecMult10        	2   70000254	//30fps:014D, 15fps:029A, 7.5fps:0535, 3.75fps:A6A
{0x0F12, 0x04E2},    	//REG_0TC_PCFG_usMinFrTimeMsecMult10        	2   70000256	//30fps:014D, 15fps:029A, 7.5fps:0535, 3.75fps:A6A
{0x0F12, 0x0000},    	//REG_0TC_PCFG_sSaturation                  	2   70000258
{0x0F12, 0x0000},    	//REG_0TC_PCFG_sSharpBlur                   	2   7000025A
{0x0F12, 0x0000},    	//REG_0TC_PCFG_sGlamour                     	2   7000025C
{0x0F12, 0x0000},    	//REG_0TC_PCFG_sColorTemp                   	2   7000025E
{0x0F12, 0x0000},    	//REG_0TC_PCFG_uDeviceGammaIndex            	2   70000260
{0x0F12, 0x0000},    	//REG_0TC_PCFG_uPrevMirror                  	2   70000262
{0x0F12, 0x0000},    	//REG_0TC_PCFG_uCaptureMirror               	2   70000264
{0x0F12, 0x0000},    	//REG_0TC_PCFG_uRotation                    	2   70000266

// Preview Config 1 (640x480, Not Fixed 15 ~ 30fps)
{0x002A, 0x0268},
{0x0F12, 0x0280},    	//REG_1TC_PCFG_usWidth                      	2   70000268	//0280:640
{0x0F12, 0x01E0},    	//REG_1TC_PCFG_usHeight                     	2   7000026A    //01E0:480
{0x0F12, 0x0005},    	//REG_1TC_PCFG_Format                       	2   7000026C
{0x0F12, 0x2EE0}, //445C,    	//REG_1TC_PCFG_usMaxOut4KHzRate             	2   7000026E
{0x0F12, 0x2EE0}, //445C,    	//REG_1TC_PCFG_usMinOut4KHzRate             	2   70000270
{0x0F12, 0x0052},    	//REG_1TC_PCFG_PVIMask                      	2   70000272
{0x0F12, 0x0001},    	//REG_1TC_PCFG_uClockInd                    	2   70000274
{0x0F12, 0x0000},    	//REG_1TC_PCFG_usFrTimeType                 	2   70000276
{0x0F12, 0x0000},    	//REG_1TC_PCFG_FrRateQualityType            	2   70000278
{0x0F12, 0x029A},    	//REG_1TC_PCFG_usMaxFrTimeMsecMult10        	2   7000027A
{0x0F12, 0x014D},    	//REG_1TC_PCFG_usMinFrTimeMsecMult10        	2   7000027C
{0x0F12, 0x0000},    	//REG_1TC_PCFG_sSaturation                  	2   7000027E
{0x0F12, 0x0000},    	//REG_1TC_PCFG_sSharpBlur                   	2   70000280
{0x0F12, 0x0000},    	//REG_1TC_PCFG_sGlamour                     	2   70000282
{0x0F12, 0x0000},    	//REG_1TC_PCFG_sColorTemp                   	2   70000284
{0x0F12, 0x0000},    	//REG_1TC_PCFG_uDeviceGammaIndex            	2   70000286
{0x0F12, 0x0001},    	//REG_1TC_PCFG_uPrevMirror                  	2   70000288
{0x0F12, 0x0001},    	//REG_1TC_PCFG_uCaptureMirror               	2   7000028A
{0x0F12, 0x0000},    	//REG_1TC_PCFG_uRotation                    	2   7000028C




//=================================================================================================
//	Set MIPI
//=================================================================================================
{0x002A, 0x03AC},
{0x0F12, 0x0000},    	//REG_TC_FLS_Mode                           	2   700003AC
{0x002A, 0x03F2},
{0x0F12, 0x0001},    	//REG_TC_OIF_EnMipiLanes                    	2   700003F2
{0x0F12, 0x00C3},    	//REG_TC_OIF_EnPackets                      	2   700003F4
{0x0F12, 0x0001},    	//REG_TC_OIF_CfgChanged                     	2   700003F6

// Apply preview config
{0x002A, 0x021C},
{0x0F12, 0x0000},	//REG_TC_GP_ActivePrevConfig
{0x002A, 0x0220},
{0x0F12, 0x0001},	//REG_TC_GP_PrevOpenAfterChange
{0x002A, 0x01F8},
{0x0F12, 0x0001},	//REG_TC_GP_NewConfigSync
{0x002A, 0x021E},
{0x0F12, 0x0001},	//REG_TC_GP_PrevConfigChanged
{0x002A, 0x01F0},
{0x0F12, 0x0001},	//REG_TC_GP_EnablePreview
{0x0F12, 0x0001},	//REG_TC_GP_EnablePreviewChanged




// Capture Config 0 (1600x1200 fixed 8fps)
{0x002A, 0x0302},
{0x0F12, 0x0000},    	//REG_0TC_CCFG_uCaptureMode                 	2   70000302
{0x0F12, 0x0640},    	//REG_0TC_CCFG_usWidth                      	2   70000304	//0640:1600
{0x0F12, 0x04B0},    	//REG_0TC_CCFG_usHeight                     	2   70000306	//04B0:
{0x0F12, 0x0005},    	//REG_0TC_CCFG_Format                       	2   70000308
{0x0F12, 0x2EE0}, //445C,    	//REG_0TC_CCFG_usMaxOut4KHzRate             	2   7000030A
{0x0F12, 0x2EE0}, //445C,    	//REG_0TC_CCFG_usMinOut4KHzRate             	2   7000030C
{0x0F12, 0x0052},    	//REG_0TC_CCFG_PVIMask                      	2   7000030E
{0x0F12, 0x0001},    	//REG_0TC_CCFG_uClockInd                    	2   70000310
{0x0F12, 0x0002},    	//REG_0TC_CCFG_usFrTimeType                 	2   70000312
{0x0F12, 0x0002},    	//REG_0TC_CCFG_FrRateQualityType            	2   70000314
{0x0F12, 0x04E2},    	//REG_0TC_CCFG_usMaxFrTimeMsecMult10        	2   70000316
{0x0F12, 0x04E2},    	//REG_0TC_CCFG_usMinFrTimeMsecMult10        	2   70000318
{0x0F12, 0x0000},    	//REG_0TC_CCFG_sSaturation                  	2   7000031A
{0x0F12, 0x0000},    	//REG_0TC_CCFG_sSharpBlur                   	2   7000031C
{0x0F12, 0x0000},    	//REG_0TC_CCFG_sGlamour                     	2   7000031E
{0x0F12, 0x0000},    	//REG_0TC_CCFG_sColorTemp                   	2   70000320
{0x0F12, 0x0000},    	//REG_0TC_CCFG_uDeviceGammaIndex            	2   70000322






// Periodic mismatch
{0x002A, 0x0780},
{0x0F12, 0x0000}, // msm_uOffsetNoBin[0][0]
{0x0F12, 0x0000}, // msm_uOffsetNoBin[0][1]
{0x0F12, 0x0000}, // msm_uOffsetNoBin[1][0]
{0x0F12, 0x0000}, // msm_uOffsetNoBin[1][1]
{0x0F12, 0x0000}, // msm_uOffsetNoBin[2][0]
{0x0F12, 0x0000}, // msm_uOffsetNoBin[2][1]
{0x0F12, 0x0000}, // msm_uOffsetNoBin[3][0]
{0x0F12, 0x0000}, // msm_uOffsetNoBin[3][1]

{0x002A, 0x0798},
{0x0F12, 0x0000}, //msm_uOffsetBin[0][0]
{0x0F12, 0x0000}, //msm_uOffsetBin[0][1]
{0x0F12, 0x0000}, //msm_uOffsetBin[1][0]
{0x0F12, 0x0000}, //msm_uOffsetBin[1][1]

{0x002A, 0x07C0},
{0x0F12, 0x0004}, //msm_NonLinearOfsOutput[2]
{0x0F12, 0x0004}, //msm_NonLinearOfsOutput[3]

{0x002A, 0x0B94},
{0x0F12, 0x0580}, //awbb_GainsInit_0_:R
{0x0F12, 0x0400}, //awbb_GainsInit_1_:G
{0x0F12, 0x05F0}, //awbb_GainsInit_2_:B
{0x002A, 0x04A0},
{0x0F12, 0x8000}, //lt_uLeiInit:AE start

//=================================================================================================
//	Set AE Weights
//=================================================================================================
{0x002A, 0x0F5A},
{0x0F12, 0x0000}, //0000,  //0101	//ae_WeightTbl_16_0_
{0x0F12, 0x0000}, //0000,  //0101	//ae_WeightTbl_16_1_
{0x0F12, 0x0000}, //0000,  //0101	//ae_WeightTbl_16_2_
{0x0F12, 0x0000}, //0000,  //0101	//ae_WeightTbl_16_3_
{0x0F12, 0x0100}, //0101,  //0101	//ae_WeightTbl_16_4_
{0x0F12, 0x0101}, //0101,  //0101	//ae_WeightTbl_16_5_
{0x0F12, 0x0101}, //0101,  //0101	//ae_WeightTbl_16_6_
{0x0F12, 0x0001}, //0101,  //0101	//ae_WeightTbl_16_7_
{0x0F12, 0x0100}, //0101,  //0101	//ae_WeightTbl_16_8_
{0x0F12, 0x0302}, //0302,  //0202 // ae_WeightTbl_16_9_
{0x0F12, 0x0203}, //0203,  //0202 // ae_WeightTbl_16_10
{0x0F12, 0x0001}, //0101,  //0101	//ae_WeightTbl_16_11
{0x0F12, 0x0100}, //0101,  //0101	//ae_WeightTbl_16_12
{0x0F12, 0x0403}, //0403,  //0202 // ae_WeightTbl_16_13
{0x0F12, 0x0304}, //0304,  //0202 // ae_WeightTbl_16_14
{0x0F12, 0x0001}, //0101,  //0101	//ae_WeightTbl_16_15
{0x0F12, 0x0100}, //0101,  //0201 // ae_WeightTbl_16_16
{0x0F12, 0x0403}, //0403,  //0303 // ae_WeightTbl_16_17
{0x0F12, 0x0304}, //0304,  //0303 // ae_WeightTbl_16_18
{0x0F12, 0x0001}, //0101,  //0102 // ae_WeightTbl_16_19
{0x0F12, 0x0100}, //0101,  //0201 // ae_WeightTbl_16_20
{0x0F12, 0x0302}, //0302,  //0303 // ae_WeightTbl_16_21
{0x0F12, 0x0203}, //0203,  //0303 // ae_WeightTbl_16_22
{0x0F12, 0x0001}, //0101,  //0102 // ae_WeightTbl_16_23
{0x0F12, 0x0100}, //0101,  //0101	//ae_WeightTbl_16_24
{0x0F12, 0x0101}, //0101,  //0202 // ae_WeightTbl_16_25
{0x0F12, 0x0101}, //0101,  //0202 // ae_WeightTbl_16_26
{0x0F12, 0x0001}, //0101,  //0101	//ae_WeightTbl_16_27
{0x0F12, 0x0000}, //0000,  //0101	//ae_WeightTbl_16_28
{0x0F12, 0x0000}, //0000,  //0101	//ae_WeightTbl_16_29
{0x0F12, 0x0000}, //0000,  //0101	//ae_WeightTbl_16_30
{0x0F12, 0x0000}, //0000,  //0101	//ae_WeightTbl_16_31

//=================================================================================================
//	Set GAS & CCM White Point
//=================================================================================================
// param_start	TVAR_ash_AwbAshCord
{0x002A, 0x0704},
{0x0F12, 0x00B3}, //TVAR_ash_AwbAshCord_0_
{0x0F12, 0x00E5}, //TVAR_ash_AwbAshCord_1_
{0x0F12, 0x0120}, //TVAR_ash_AwbAshCord_2_
{0x0F12, 0x0136}, //TVAR_ash_AwbAshCord_3_
{0x0F12, 0x0180}, //TVAR_ash_AwbAshCord_4_
{0x0F12, 0x01B0}, //TVAR_ash_AwbAshCord_5_
{0x0F12, 0x0200}, //TVAR_ash_AwbAshCord_6_

// param_start	wbt_AwbCcmCord
{0x002A, 0x06F2},
{0x0F12, 0x00B3}, //SARR_AwbCcmCord_0_	Hor
{0x0F12, 0x00E5}, //SARR_AwbCcmCord_1_	IncaA
{0x0F12, 0x0120}, //SARR_AwbCcmCord_2_	WW
{0x0F12, 0x0136}, //SARR_AwbCcmCord_3_	CW
{0x0F12, 0x0180}, //SARR_AwbCcmCord_4_	D50
{0x0F12, 0x0190}, //SARR_AwbCcmCord_5_	D65

                  // Target Brightness Control
{0x002A, 0x103E},
{0x0F12, 0x0000},	//SARR_IllumType_0_
{0x0F12, 0x0009},	//SARR_IllumType_1_
{0x0F12, 0x0018},	//SARR_IllumType_2_
{0x0F12, 0x0032},	//SARR_IllumType_3_
{0x0F12, 0x004A},	//SARR_IllumType_4_
{0x0F12, 0x0051},	//SARR_IllumType_5_
{0x0F12, 0x0056},	//SARR_IllumType_6_
{0x0F12, 0x010C},	//SARe_2_R_IllumTypeF_0_
{0x0F12, 0x010C},	//SARe_3_R_IllumTypeF_1_
{0x0F12, 0x0109},	//SARe_4_R_IllumTypeF_2_
{0x0F12, 0x0105},	//SARe_5_R_IllumTypeF_3_
{0x0F12, 0x0102},	//SARe_6_R_IllumTypeF_4_
{0x0F12, 0x00FB},	//SARR_IllumTypeF_5_
{0x0F12, 0x00F8},	//SARR_IllumTypeF_6_

                  // TVAR_ash_GASAlpha(Indoor)
{0x002A, 0x0712},
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[0]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[1]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[2]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[3]

{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[4]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[5]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[6]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[7]

{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[8]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[9]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[10]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[11]

{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[12]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[13]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[14]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[15]

{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[16]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[17]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[18]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[19]

{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[20]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[21]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[22]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[23]

{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[24]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[25]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[26]
{0x0F12, 0x0100},	//TVAR_ash_GASAlpha[27]

                  //  TVAR_ash_GASAlpha(Outdoor)
{0x0F12, 0x0108},	//TVAR_ash_GASOutdoorAlpha_0_
{0x0F12, 0x0100},	//TVAR_ash_GASOutdoorAlpha_1_
{0x0F12, 0x0100},	//TVAR_ash_GASOutdoorAlpha_2_
{0x0F12, 0x0100},	//TVAR_ash_GASOutdoorAlpha_3_

// GAS LUT Start Address
{0x002A, 0x0754},
{0x0F12, 0x2388},	//TVAR_ash_pGAS
{0x0F12, 0x7000},	//TVAR_ash_pGAS

                  // param_start	TVAR_ash_pGAS
{0x002A, 0x2388},
{0x0F12, 0x0160},  //0103,  //01CC //TVAR_ash_pGAS[0]
{0x0F12, 0x0134},  //00D9,  //0178 //TVAR_ash_pGAS[1]
{0x0F12, 0x00FF},  //00B2,  //013B //TVAR_ash_pGAS[2]
{0x0F12, 0x00D1},  //0096,  //0108 //TVAR_ash_pGAS[3]
{0x0F12, 0x00B1},  //0083,  //00E4 //TVAR_ash_pGAS[4]
{0x0F12, 0x009D},  //0076,  //00CC //TVAR_ash_pGAS[5]
{0x0F12, 0x0096},  //0076,  //00C5 //TVAR_ash_pGAS[6]
{0x0F12, 0x009E},  //007D,  //00CF //TVAR_ash_pGAS[7]
{0x0F12, 0x00B3},  //008B,  //00E8 //TVAR_ash_pGAS[8]
{0x0F12, 0x00D3},  //00A2,  //0111 //TVAR_ash_pGAS[9]
{0x0F12, 0x00FF},  //00C6,  //0142 //TVAR_ash_pGAS[10]
{0x0F12, 0x0131},  //00EE,  //0183 //TVAR_ash_pGAS[11]
{0x0F12, 0x0159},  //0116,  //01D9 //TVAR_ash_pGAS[12]
{0x0F12, 0x013C},  //00CD,  //0184 //TVAR_ash_pGAS[13]
{0x0F12, 0x0107},  //00AB,  //0142 //TVAR_ash_pGAS[14]
{0x0F12, 0x00CD},  //0086,  //0101 //TVAR_ash_pGAS[15]
{0x0F12, 0x00A1},  //006D,  //00CF //TVAR_ash_pGAS[16]
{0x0F12, 0x0080},  //005A,  //00A7 //TVAR_ash_pGAS[17]
{0x0F12, 0x006B},  //004F,  //0090 //TVAR_ash_pGAS[18]
{0x0F12, 0x0064},  //004D,  //0088 //TVAR_ash_pGAS[19]
{0x0F12, 0x006C},  //0054,  //0092 //TVAR_ash_pGAS[20]
{0x0F12, 0x0080},  //0064,  //00AC //TVAR_ash_pGAS[21]
{0x0F12, 0x00A1},  //007A,  //00D5 //TVAR_ash_pGAS[22]
{0x0F12, 0x00CD},  //009A,  //010D //TVAR_ash_pGAS[23]
{0x0F12, 0x0106},  //00C2,  //014E //TVAR_ash_pGAS[24]
{0x0F12, 0x0139},  //00EC,  //0190 //TVAR_ash_pGAS[25]
{0x0F12, 0x0116},  //00A8,  //014E //TVAR_ash_pGAS[26]
{0x0F12, 0x00DC},  //0089,  //010D //TVAR_ash_pGAS[27]
{0x0F12, 0x00A2},  //0064,  //00CA //TVAR_ash_pGAS[28]
{0x0F12, 0x0073},  //004A,  //0094 //TVAR_ash_pGAS[29]
{0x0F12, 0x0051},  //0037,  //006D //TVAR_ash_pGAS[30]
{0x0F12, 0x003B},  //002A,  //0054 //TVAR_ash_pGAS[31]
{0x0F12, 0x0033},  //0028,  //004E //TVAR_ash_pGAS[32]
{0x0F12, 0x003B},  //002F,  //0058 //TVAR_ash_pGAS[33]
{0x0F12, 0x0050},  //003F,  //0073 //TVAR_ash_pGAS[34]
{0x0F12, 0x0073},  //0059,  //009D //TVAR_ash_pGAS[35]
{0x0F12, 0x00A2},  //0077,  //00D5 //TVAR_ash_pGAS[36]
{0x0F12, 0x00DD},  //00A1,  //011D //TVAR_ash_pGAS[37]
{0x0F12, 0x0115},  //00CB,  //015A //TVAR_ash_pGAS[38]
{0x0F12, 0x00FA},  //008D,  //0129 //TVAR_ash_pGAS[39]
{0x0F12, 0x00BF},  //006F,  //00E5 //TVAR_ash_pGAS[40]
{0x0F12, 0x0085},  //004E,  //00A1 //TVAR_ash_pGAS[41]
{0x0F12, 0x0055},  //0032,  //006B //TVAR_ash_pGAS[42]
{0x0F12, 0x0031},  //001E,  //0042 //TVAR_ash_pGAS[43]
{0x0F12, 0x001B},  //0014,  //002A //TVAR_ash_pGAS[44]
{0x0F12, 0x0014},  //0011,  //0022 //TVAR_ash_pGAS[45]
{0x0F12, 0x001A},  //0017,  //002D //TVAR_ash_pGAS[46]
{0x0F12, 0x0031},  //0027,  //0049 //TVAR_ash_pGAS[47]
{0x0F12, 0x0055},  //003E,  //0075 //TVAR_ash_pGAS[48]
{0x0F12, 0x0085},  //0062,  //00AF //TVAR_ash_pGAS[49]
{0x0F12, 0x00C0},  //0089,  //00F8 //TVAR_ash_pGAS[50]
{0x0F12, 0x00FB},  //00B4,  //013A //TVAR_ash_pGAS[51]
{0x0F12, 0x00EA},  //007A,  //0113 //TVAR_ash_pGAS[52]
{0x0F12, 0x00AF},  //0061,  //00CE //TVAR_ash_pGAS[53]
{0x0F12, 0x0074},  //0040,  //0088 //TVAR_ash_pGAS[54]
{0x0F12, 0x0045},  //0025,  //0052 //TVAR_ash_pGAS[55]
{0x0F12, 0x0020},  //0011,  //002A //TVAR_ash_pGAS[56]
{0x0F12, 0x000B},  //0005,  //0010 //TVAR_ash_pGAS[57]
{0x0F12, 0x0003},  //0003,  //0009 //TVAR_ash_pGAS[58]
{0x0F12, 0x000A},  //000A,  //0015 //TVAR_ash_pGAS[59]
{0x0F12, 0x0020},  //001A,  //0032 //TVAR_ash_pGAS[60]
{0x0F12, 0x0046},  //0033,  //005E //TVAR_ash_pGAS[61]
{0x0F12, 0x0076},  //0054,  //0098 //TVAR_ash_pGAS[62]
{0x0F12, 0x00B1},  //007E,  //00E3 //TVAR_ash_pGAS[63]
{0x0F12, 0x00ED},  //00A6,  //0128 //TVAR_ash_pGAS[64]
{0x0F12, 0x00E6},  //0075,  //010A //TVAR_ash_pGAS[65]
{0x0F12, 0x00AA},  //005F,  //00C4 //TVAR_ash_pGAS[66]
{0x0F12, 0x0071},  //003D,  //0080 //TVAR_ash_pGAS[67]
{0x0F12, 0x0041},  //0020,  //0049 //TVAR_ash_pGAS[68]
{0x0F12, 0x001D},  //000E,  //0020 //TVAR_ash_pGAS[69]
{0x0F12, 0x0008},  //0001,  //0008 //TVAR_ash_pGAS[70]
{0x0F12, 0x0000},  //0000,  //0000 //TVAR_ash_pGAS[71]
{0x0F12, 0x0007},  //0006,  //000D //TVAR_ash_pGAS[72]
{0x0F12, 0x001E},  //0019,  //002A //TVAR_ash_pGAS[73]
{0x0F12, 0x0044},  //0030,  //0058 //TVAR_ash_pGAS[74]
{0x0F12, 0x0074},  //0052,  //0093 //TVAR_ash_pGAS[75]
{0x0F12, 0x00B0},  //007B,  //00DD //TVAR_ash_pGAS[76]
{0x0F12, 0x00EC},  //00A3,  //0123 //TVAR_ash_pGAS[77]
{0x0F12, 0x00EF},  //007A,  //010D //TVAR_ash_pGAS[78]
{0x0F12, 0x00B3},  //0060,  //00CA //TVAR_ash_pGAS[79]
{0x0F12, 0x007A},  //003F,  //0085 //TVAR_ash_pGAS[80]
{0x0F12, 0x004A},  //0024,  //004E //TVAR_ash_pGAS[81]
{0x0F12, 0x0026},  //0011,  //0026 //TVAR_ash_pGAS[82]
{0x0F12, 0x0011},  //0005,  //000E //TVAR_ash_pGAS[83]
{0x0F12, 0x000A},  //0003,  //0007 //TVAR_ash_pGAS[84]
{0x0F12, 0x0011},  //000B,  //0014 //TVAR_ash_pGAS[85]
{0x0F12, 0x0029},  //001C,  //0032 //TVAR_ash_pGAS[86]
{0x0F12, 0x004F},  //0036,  //0061 //TVAR_ash_pGAS[87]
{0x0F12, 0x0080},  //0059,  //009C //TVAR_ash_pGAS[88]
{0x0F12, 0x00BC},  //0082,  //00E8 //TVAR_ash_pGAS[89]
{0x0F12, 0x00F8},  //00AB,  //012F //TVAR_ash_pGAS[90]
{0x0F12, 0x0105},  //0088,  //0121 //TVAR_ash_pGAS[91]
{0x0F12, 0x00C9},  //006F,  //00DE //TVAR_ash_pGAS[92]
{0x0F12, 0x008F},  //004C,  //009A //TVAR_ash_pGAS[93]
{0x0F12, 0x0060},  //002F,  //0063 //TVAR_ash_pGAS[94]
{0x0F12, 0x003C},  //001C,  //003B //TVAR_ash_pGAS[95]
{0x0F12, 0x0026},  //0010,  //0024 //TVAR_ash_pGAS[96]
{0x0F12, 0x001F},  //0010,  //001D //TVAR_ash_pGAS[97]
{0x0F12, 0x0028},  //0019,  //002B //TVAR_ash_pGAS[98]
{0x0F12, 0x0040},  //002A,  //0049 //TVAR_ash_pGAS[99]
{0x0F12, 0x0066},  //0044,  //0079 //TVAR_ash_pGAS[100]
{0x0F12, 0x0097},  //0067,  //00B5 //TVAR_ash_pGAS[101]
{0x0F12, 0x00D4},  //0092,  //0100 //TVAR_ash_pGAS[102]
{0x0F12, 0x0110},  //00BB,  //0145 //TVAR_ash_pGAS[103]
{0x0F12, 0x0124},  //009F,  //013F //TVAR_ash_pGAS[104]
{0x0F12, 0x00EB},  //0084,  //0101 //TVAR_ash_pGAS[105]
{0x0F12, 0x00B1},  //0060,  //00BE //TVAR_ash_pGAS[106]
{0x0F12, 0x0082},  //0044,  //0087 //TVAR_ash_pGAS[107]
{0x0F12, 0x005F},  //0031,  //005F //TVAR_ash_pGAS[108]
{0x0F12, 0x004A},  //0025,  //0048 //TVAR_ash_pGAS[109]
{0x0F12, 0x0043},  //0025,  //0043 //TVAR_ash_pGAS[110]
{0x0F12, 0x004C},  //002E,  //0051 //TVAR_ash_pGAS[111]
{0x0F12, 0x0064},  //0041,  //0070 //TVAR_ash_pGAS[112]
{0x0F12, 0x0089},  //005B,  //00A0 //TVAR_ash_pGAS[113]
{0x0F12, 0x00BA},  //007F,  //00DF //TVAR_ash_pGAS[114]
{0x0F12, 0x00F8},  //00AC,  //0126 //TVAR_ash_pGAS[115]
{0x0F12, 0x012F},  //00D5,  //0168 //TVAR_ash_pGAS[116]
{0x0F12, 0x0147},  //00C5,  //016D //TVAR_ash_pGAS[117]
{0x0F12, 0x0116},  //00A5,  //012F //TVAR_ash_pGAS[118]
{0x0F12, 0x00DE},  //007D,  //00EF //TVAR_ash_pGAS[119]
{0x0F12, 0x00AF},  //0061,  //00BA //TVAR_ash_pGAS[120]
{0x0F12, 0x008E},  //004F,  //0093 //TVAR_ash_pGAS[121]
{0x0F12, 0x007A},  //0043,  //007E //TVAR_ash_pGAS[122]
{0x0F12, 0x0072},  //0044,  //0079 //TVAR_ash_pGAS[123]
{0x0F12, 0x007A},  //004E,  //0087 //TVAR_ash_pGAS[124]
{0x0F12, 0x0091},  //0060,  //00A6 //TVAR_ash_pGAS[125]
{0x0F12, 0x00B6},  //007B,  //00D7 //TVAR_ash_pGAS[126]
{0x0F12, 0x00E8},  //00A0,  //0114 //TVAR_ash_pGAS[127]
{0x0F12, 0x0121},  //00D2,  //0158 //TVAR_ash_pGAS[128]
{0x0F12, 0x0150},  //00F7,  //0199 //TVAR_ash_pGAS[129]
{0x0F12, 0x0170},  //00F0,  //01A6 //TVAR_ash_pGAS[130]
{0x0F12, 0x013F},  //00D2,  //015E //TVAR_ash_pGAS[131]
{0x0F12, 0x0110},  //00A1,  //0122 //TVAR_ash_pGAS[132]
{0x0F12, 0x00E2},  //0085,  //00F1 //TVAR_ash_pGAS[133]
{0x0F12, 0x00C0},  //0070,  //00CB //TVAR_ash_pGAS[134]
{0x0F12, 0x00AB},  //0065,  //00B6 //TVAR_ash_pGAS[135]
{0x0F12, 0x00A4},  //0064,  //00B2 //TVAR_ash_pGAS[136]
{0x0F12, 0x00AC},  //006E,  //00C0 //TVAR_ash_pGAS[137]
{0x0F12, 0x00C3},  //0082,  //00DF //TVAR_ash_pGAS[138]
{0x0F12, 0x00E6},  //009D,  //010D //TVAR_ash_pGAS[139]
{0x0F12, 0x0117},  //00C6,  //0145 //TVAR_ash_pGAS[140]
{0x0F12, 0x0145},  //00F7,  //0188 //TVAR_ash_pGAS[141]
{0x0F12, 0x0172},  //0121,  //01DF //TVAR_ash_pGAS[142]
{0x0F12, 0x0127},  //00D5,  //016C //TVAR_ash_pGAS[143]
{0x0F12, 0x0100},  //00B3,  //0127 //TVAR_ash_pGAS[144]
{0x0F12, 0x00CF},  //008D,  //00F2 //TVAR_ash_pGAS[145]
{0x0F12, 0x00A7},  //0073,  //00CA //TVAR_ash_pGAS[146]
{0x0F12, 0x008D},  //0065,  //00AC //TVAR_ash_pGAS[147]
{0x0F12, 0x007D},  //005D,  //009B //TVAR_ash_pGAS[148]
{0x0F12, 0x0077},  //0059,  //0096 //TVAR_ash_pGAS[149]
{0x0F12, 0x007A},  //005A,  //009C //TVAR_ash_pGAS[150]
{0x0F12, 0x0087},  //0061,  //00AE //TVAR_ash_pGAS[151]
{0x0F12, 0x009E},  //006E,  //00CC //TVAR_ash_pGAS[152]
{0x0F12, 0x00C0},  //0081,  //00F4 //TVAR_ash_pGAS[153]
{0x0F12, 0x00EC},  //009C,  //012D //TVAR_ash_pGAS[154]
{0x0F12, 0x010F},  //00B9,  //0179 //TVAR_ash_pGAS[155]
{0x0F12, 0x0108},  //00AB,  //0130 //TVAR_ash_pGAS[156]
{0x0F12, 0x00D8},  //008F,  //00F6 //TVAR_ash_pGAS[157]
{0x0F12, 0x00A5},  //006C,  //00C4 //TVAR_ash_pGAS[158]
{0x0F12, 0x0080},  //0055,  //0099 //TVAR_ash_pGAS[159]
{0x0F12, 0x0066},  //0047,  //007C //TVAR_ash_pGAS[160]
{0x0F12, 0x0056},  //003E,  //006C //TVAR_ash_pGAS[161]
{0x0F12, 0x004F},  //003B,  //0067 //TVAR_ash_pGAS[162]
{0x0F12, 0x0053},  //003D,  //006E //TVAR_ash_pGAS[163]
{0x0F12, 0x0061},  //0044,  //007F //TVAR_ash_pGAS[164]
{0x0F12, 0x0077},  //0052,  //009E //TVAR_ash_pGAS[165]
{0x0F12, 0x0098},  //0062,  //00C8 //TVAR_ash_pGAS[166]
{0x0F12, 0x00C6},  //007F,  //0100 //TVAR_ash_pGAS[167]
{0x0F12, 0x00F3},  //0099,  //0138 //TVAR_ash_pGAS[168]
{0x0F12, 0x00E7},  //008D,  //0107 //TVAR_ash_pGAS[169]
{0x0F12, 0x00B4},  //0072,  //00CF //TVAR_ash_pGAS[170]
{0x0F12, 0x0081},  //0050,  //0097 //TVAR_ash_pGAS[171]
{0x0F12, 0x005C},  //003C,  //006D //TVAR_ash_pGAS[172]
{0x0F12, 0x0041},  //002C,  //0050 //TVAR_ash_pGAS[173]
{0x0F12, 0x0030},  //0023,  //0040 //TVAR_ash_pGAS[174]
{0x0F12, 0x0029},  //0020,  //003B //TVAR_ash_pGAS[175]
{0x0F12, 0x002E},  //0024,  //0042 //TVAR_ash_pGAS[176]
{0x0F12, 0x003D},  //002C,  //0055 //TVAR_ash_pGAS[177]
{0x0F12, 0x0055},  //003B,  //0074 //TVAR_ash_pGAS[178]
{0x0F12, 0x0076},  //004A,  //009F //TVAR_ash_pGAS[179]
{0x0F12, 0x00A5},  //0067,  //00D9 //TVAR_ash_pGAS[180]
{0x0F12, 0x00D4},  //007F,  //0110 //TVAR_ash_pGAS[181]
{0x0F12, 0x00CF},  //0076,  //00E9 //TVAR_ash_pGAS[182]
{0x0F12, 0x009B},  //0061,  //00AE //TVAR_ash_pGAS[183]
{0x0F12, 0x006A},  //003F,  //0077 //TVAR_ash_pGAS[184]
{0x0F12, 0x0043},  //0029,  //004D //TVAR_ash_pGAS[185]
{0x0F12, 0x0027},  //0019,  //002F //TVAR_ash_pGAS[186]
{0x0F12, 0x0016},  //0010,  //001F //TVAR_ash_pGAS[187]
{0x0F12, 0x000F},  //000D,  //001A //TVAR_ash_pGAS[188]
{0x0F12, 0x0015},  //0011,  //0022 //TVAR_ash_pGAS[189]
{0x0F12, 0x0025},  //001B,  //0036 //TVAR_ash_pGAS[190]
{0x0F12, 0x003E},  //002A,  //0055 //TVAR_ash_pGAS[191]
{0x0F12, 0x0061},  //003C,  //0081 //TVAR_ash_pGAS[192]
{0x0F12, 0x008E},  //0058,  //00BC //TVAR_ash_pGAS[193]
{0x0F12, 0x00BF},  //0070,  //00F5 //TVAR_ash_pGAS[194]
{0x0F12, 0x00C2},  //0067,  //00D8 //TVAR_ash_pGAS[195]
{0x0F12, 0x008E},  //0056,  //009C //TVAR_ash_pGAS[196]
{0x0F12, 0x005D},  //0034,  //0064 //TVAR_ash_pGAS[197]
{0x0F12, 0x0037},  //001E,  //003A //TVAR_ash_pGAS[198]
{0x0F12, 0x001A},  //000E,  //001C //TVAR_ash_pGAS[199]
{0x0F12, 0x0009},  //0006,  //000B //TVAR_ash_pGAS[200]
{0x0F12, 0x0002},  //0004,  //0006 //TVAR_ash_pGAS[201]
{0x0F12, 0x0007},  //0008,  //000F //TVAR_ash_pGAS[202]
{0x0F12, 0x0018},  //0011,  //0024 //TVAR_ash_pGAS[203]
{0x0F12, 0x0033},  //0022,  //0044 //TVAR_ash_pGAS[204]
{0x0F12, 0x0057},  //0034,  //0070 //TVAR_ash_pGAS[205]
{0x0F12, 0x0083},  //0052,  //00AD //TVAR_ash_pGAS[206]
{0x0F12, 0x00B3},  //006A,  //00E6 //TVAR_ash_pGAS[207]
{0x0F12, 0x00BE},  //0062,  //00D0 //TVAR_ash_pGAS[208]
{0x0F12, 0x008A},  //0053,  //0095 //TVAR_ash_pGAS[209]
{0x0F12, 0x005A},  //0032,  //005D //TVAR_ash_pGAS[210]
{0x0F12, 0x0034},  //001B,  //0033 //TVAR_ash_pGAS[211]
{0x0F12, 0x0017},  //000C,  //0015 //TVAR_ash_pGAS[212]
{0x0F12, 0x0006},  //0004,  //0005 //TVAR_ash_pGAS[213]
{0x0F12, 0x0000},  //0002,  //0000 //TVAR_ash_pGAS[214]
{0x0F12, 0x0006},  //0006,  //0009 //TVAR_ash_pGAS[215]
{0x0F12, 0x0017},  //0010,  //001E //TVAR_ash_pGAS[216]
{0x0F12, 0x0033},  //0021,  //0041 //TVAR_ash_pGAS[217]
{0x0F12, 0x0057},  //0035,  //006D //TVAR_ash_pGAS[218]
{0x0F12, 0x0083},  //0053,  //00AA //TVAR_ash_pGAS[219]
{0x0F12, 0x00B3},  //0069,  //00E4 //TVAR_ash_pGAS[220]
{0x0F12, 0x00C5},  //0066,  //00D6 //TVAR_ash_pGAS[221]
{0x0F12, 0x0091},  //0055,  //009A //TVAR_ash_pGAS[222]
{0x0F12, 0x0061},  //0035,  //0062 //TVAR_ash_pGAS[223]
{0x0F12, 0x003B},  //001F,  //0038 //TVAR_ash_pGAS[224]
{0x0F12, 0x0020},  //0010,  //001B //TVAR_ash_pGAS[225]
{0x0F12, 0x000F},  //0009,  //000A //TVAR_ash_pGAS[226]
{0x0F12, 0x0009},  //0006,  //0006 //TVAR_ash_pGAS[227]
{0x0F12, 0x0010},  //000B,  //0010 //TVAR_ash_pGAS[228]
{0x0F12, 0x0021},  //0016,  //0026 //TVAR_ash_pGAS[229]
{0x0F12, 0x003D},  //0027,  //0049 //TVAR_ash_pGAS[230]
{0x0F12, 0x0060},  //003B,  //0076 //TVAR_ash_pGAS[231]
{0x0F12, 0x008D},  //005A,  //00B4 //TVAR_ash_pGAS[232]
{0x0F12, 0x00BE},  //006F,  //00ED //TVAR_ash_pGAS[233]
{0x0F12, 0x00D7},  //006E,  //00E4 //TVAR_ash_pGAS[234]
{0x0F12, 0x00A2},  //0061,  //00AB //TVAR_ash_pGAS[235]
{0x0F12, 0x0072},  //003F,  //0072 //TVAR_ash_pGAS[236]
{0x0F12, 0x004D},  //002A,  //0049 //TVAR_ash_pGAS[237]
{0x0F12, 0x0032},  //001C,  //002C //TVAR_ash_pGAS[238]
{0x0F12, 0x0022},  //0013,  //001C //TVAR_ash_pGAS[239]
{0x0F12, 0x001D},  //0012,  //0019 //TVAR_ash_pGAS[240]
{0x0F12, 0x0024},  //0017,  //0023 //TVAR_ash_pGAS[241]
{0x0F12, 0x0035},  //0023,  //003A //TVAR_ash_pGAS[242]
{0x0F12, 0x0050},  //0033,  //005D //TVAR_ash_pGAS[243]
{0x0F12, 0x0073},  //0048,  //008B //TVAR_ash_pGAS[244]
{0x0F12, 0x00A0},  //0067,  //00C8 //TVAR_ash_pGAS[245]
{0x0F12, 0x00D2},  //007B,  //00FF //TVAR_ash_pGAS[246]
{0x0F12, 0x00F0},  //0083,  //00FF //TVAR_ash_pGAS[247]
{0x0F12, 0x00BE},  //0073,  //00C8 //TVAR_ash_pGAS[248]
{0x0F12, 0x008C},  //0050,  //0090 //TVAR_ash_pGAS[249]
{0x0F12, 0x0068},  //003A,  //0066 //TVAR_ash_pGAS[250]
{0x0F12, 0x004F},  //002D,  //004A //TVAR_ash_pGAS[251]
{0x0F12, 0x0040},  //0026,  //003A //TVAR_ash_pGAS[252]
{0x0F12, 0x003B},  //0024,  //0038 //TVAR_ash_pGAS[253]
{0x0F12, 0x0041},  //002A,  //0042 //TVAR_ash_pGAS[254]
{0x0F12, 0x0052},  //0035,  //0059 //TVAR_ash_pGAS[255]
{0x0F12, 0x006C},  //0046,  //007C //TVAR_ash_pGAS[256]
{0x0F12, 0x008E},  //005A,  //00AB //TVAR_ash_pGAS[257]
{0x0F12, 0x00BE},  //007A,  //00E7 //TVAR_ash_pGAS[258]
{0x0F12, 0x00ED},  //008E,  //011D //TVAR_ash_pGAS[259]
{0x0F12, 0x010C},  //009E,  //0123 //TVAR_ash_pGAS[260]
{0x0F12, 0x00E1},  //008D,  //00EC //TVAR_ash_pGAS[261]
{0x0F12, 0x00AF},  //0066,  //00B7 //TVAR_ash_pGAS[262]
{0x0F12, 0x008A},  //0050,  //008E //TVAR_ash_pGAS[263]
{0x0F12, 0x0072},  //0043,  //0073 //TVAR_ash_pGAS[264]
{0x0F12, 0x0064},  //003B,  //0064 //TVAR_ash_pGAS[265]
{0x0F12, 0x005F},  //003B,  //0062 //TVAR_ash_pGAS[266]
{0x0F12, 0x0065},  //0041,  //006D //TVAR_ash_pGAS[267]
{0x0F12, 0x0074},  //004D,  //0083 //TVAR_ash_pGAS[268]
{0x0F12, 0x008D},  //005D,  //00A7 //TVAR_ash_pGAS[269]
{0x0F12, 0x00B2},  //0072,  //00D4 //TVAR_ash_pGAS[270]
{0x0F12, 0x00E0},  //0096,  //010B //TVAR_ash_pGAS[271]
{0x0F12, 0x010A},  //00AB,  //0144 //TVAR_ash_pGAS[272]
{0x0F12, 0x012F},  //00CB,  //0156 //TVAR_ash_pGAS[273]
{0x0F12, 0x0104},  //00B7,  //0114 //TVAR_ash_pGAS[274]
{0x0F12, 0x00D9},  //0089,  //00E2 //TVAR_ash_pGAS[275]
{0x0F12, 0x00B3},  //006F,  //00BB //TVAR_ash_pGAS[276]
{0x0F12, 0x0099},  //0062,  //009F //TVAR_ash_pGAS[277]
{0x0F12, 0x008B},  //005B,  //0090 //TVAR_ash_pGAS[278]
{0x0F12, 0x0086},  //0059,  //008E //TVAR_ash_pGAS[279]
{0x0F12, 0x008B},  //005E,  //0099 //TVAR_ash_pGAS[280]
{0x0F12, 0x009B},  //006A,  //00B0 //TVAR_ash_pGAS[281]
{0x0F12, 0x00B5},  //007C,  //00D2 //TVAR_ash_pGAS[282]
{0x0F12, 0x00DA},  //0095,  //00FE //TVAR_ash_pGAS[283]
{0x0F12, 0x0101},  //00BE,  //0133 //TVAR_ash_pGAS[284]
{0x0F12, 0x0128},  //00D4,  //017D //TVAR_ash_pGAS[285]
{0x0F12, 0x012F},  //00DB,  //0174 //TVAR_ash_pGAS[286]
{0x0F12, 0x0106},  //00B4,  //012A //TVAR_ash_pGAS[287]
{0x0F12, 0x00D4},  //008C,  //00F6 //TVAR_ash_pGAS[288]
{0x0F12, 0x00AA},  //0071,  //00CC //TVAR_ash_pGAS[289]
{0x0F12, 0x008E},  //0060,  //00AD //TVAR_ash_pGAS[290]
{0x0F12, 0x007D},  //0059,  //009C //TVAR_ash_pGAS[291]
{0x0F12, 0x0079},  //0056,  //0099 //TVAR_ash_pGAS[292]
{0x0F12, 0x0080},  //005D,  //00A4 //TVAR_ash_pGAS[293]
{0x0F12, 0x0093},  //0069,  //00BC //TVAR_ash_pGAS[294]
{0x0F12, 0x00B1},  //007B,  //00E0 //TVAR_ash_pGAS[295]
{0x0F12, 0x00DC},  //0095,  //010E //TVAR_ash_pGAS[296]
{0x0F12, 0x010C},  //00B6,  //0147 //TVAR_ash_pGAS[297]
{0x0F12, 0x0130},  //00D3,  //0193 //TVAR_ash_pGAS[298]
{0x0F12, 0x0112},  //00B2,  //013A //TVAR_ash_pGAS[299]
{0x0F12, 0x00E0},  //0092,  //00FE //TVAR_ash_pGAS[300]
{0x0F12, 0x00AB},  //006D,  //00C9 //TVAR_ash_pGAS[301]
{0x0F12, 0x0083},  //0056,  //009E //TVAR_ash_pGAS[302]
{0x0F12, 0x0067},  //0045,  //007E //TVAR_ash_pGAS[303]
{0x0F12, 0x0057},  //003C,  //006E //TVAR_ash_pGAS[304]
{0x0F12, 0x0051},  //003A,  //006B //TVAR_ash_pGAS[305]
{0x0F12, 0x0059},  //0040,  //0075 //TVAR_ash_pGAS[306]
{0x0F12, 0x006B},  //004D,  //008D //TVAR_ash_pGAS[307]
{0x0F12, 0x0089},  //005F,  //00B2 //TVAR_ash_pGAS[308]
{0x0F12, 0x00B2},  //0076,  //00E0 //TVAR_ash_pGAS[309]
{0x0F12, 0x00E5},  //0096,  //011B //TVAR_ash_pGAS[310]
{0x0F12, 0x0114},  //00B2,  //0152 //TVAR_ash_pGAS[311]
{0x0F12, 0x00F2},  //0091,  //0112 //TVAR_ash_pGAS[312]
{0x0F12, 0x00BD},  //0078,  //00D8 //TVAR_ash_pGAS[313]
{0x0F12, 0x0088},  //0055,  //009F //TVAR_ash_pGAS[314]
{0x0F12, 0x0061},  //003D,  //0073 //TVAR_ash_pGAS[315]
{0x0F12, 0x0044},  //002B,  //0054 //TVAR_ash_pGAS[316]
{0x0F12, 0x0031},  //0022,  //0042 //TVAR_ash_pGAS[317]
{0x0F12, 0x002C},  //001F,  //003F //TVAR_ash_pGAS[318]
{0x0F12, 0x0033},  //0025,  //0049 //TVAR_ash_pGAS[319]
{0x0F12, 0x0047},  //0033,  //0061 //TVAR_ash_pGAS[320]
{0x0F12, 0x0065},  //0046,  //0085 //TVAR_ash_pGAS[321]
{0x0F12, 0x008C},  //005C,  //00B5 //TVAR_ash_pGAS[322]
{0x0F12, 0x00C0},  //007B,  //00F2 //TVAR_ash_pGAS[323]
{0x0F12, 0x00F3},  //0097,  //0128 //TVAR_ash_pGAS[324]
{0x0F12, 0x00DB},  //007B,  //00F5 //TVAR_ash_pGAS[325]
{0x0F12, 0x00A5},  //0065,  //00BA //TVAR_ash_pGAS[326]
{0x0F12, 0x0071},  //0044,  //0080 //TVAR_ash_pGAS[327]
{0x0F12, 0x0049},  //002B,  //0054 //TVAR_ash_pGAS[328]
{0x0F12, 0x002A},  //0019,  //0034 //TVAR_ash_pGAS[329]
{0x0F12, 0x0018},  //000F,  //0022 //TVAR_ash_pGAS[330]
{0x0F12, 0x0011},  //000C,  //001D //TVAR_ash_pGAS[331]
{0x0F12, 0x0018},  //0012,  //0027 //TVAR_ash_pGAS[332]
{0x0F12, 0x002C},  //001F,  //003F //TVAR_ash_pGAS[333]
{0x0F12, 0x004B},  //0034,  //0064 //TVAR_ash_pGAS[334]
{0x0F12, 0x0072},  //004A,  //0092 //TVAR_ash_pGAS[335]
{0x0F12, 0x00A3},  //0067,  //00CF //TVAR_ash_pGAS[336]
{0x0F12, 0x00D7},  //0081,  //0109 //TVAR_ash_pGAS[337]
{0x0F12, 0x00CD},  //006C,  //00E4 //TVAR_ash_pGAS[338]
{0x0F12, 0x0097},  //005C,  //00A8 //TVAR_ash_pGAS[339]
{0x0F12, 0x0065},  //003A,  //006E //TVAR_ash_pGAS[340]
{0x0F12, 0x003C},  //0022,  //0041 //TVAR_ash_pGAS[341]
{0x0F12, 0x001D},  //000F,  //0021 //TVAR_ash_pGAS[342]
{0x0F12, 0x000A},  //0005,  //000E //TVAR_ash_pGAS[343]
{0x0F12, 0x0003},  //0002,  //0008 //TVAR_ash_pGAS[344]
{0x0F12, 0x0009},  //0007,  //0012 //TVAR_ash_pGAS[345]
{0x0F12, 0x001D},  //0014,  //0029 //TVAR_ash_pGAS[346]
{0x0F12, 0x003B},  //0027,  //004D //TVAR_ash_pGAS[347]
{0x0F12, 0x0063},  //003D,  //007C //TVAR_ash_pGAS[348]
{0x0F12, 0x0092},  //005B,  //00B8 //TVAR_ash_pGAS[349]
{0x0F12, 0x00C4},  //0073,  //00F3 //TVAR_ash_pGAS[350]
{0x0F12, 0x00CA},  //0067,  //00DF //TVAR_ash_pGAS[351]
{0x0F12, 0x0094},  //0058,  //00A2 //TVAR_ash_pGAS[352]
{0x0F12, 0x0062},  //0038,  //0068 //TVAR_ash_pGAS[353]
{0x0F12, 0x003A},  //001E,  //003B //TVAR_ash_pGAS[354]
{0x0F12, 0x001A},  //000C,  //001A //TVAR_ash_pGAS[355]
{0x0F12, 0x0007},  //0002,  //0006 //TVAR_ash_pGAS[356]
{0x0F12, 0x0000},  //0000,  //0000 //TVAR_ash_pGAS[357]
{0x0F12, 0x0006},  //0003,  //0009 //TVAR_ash_pGAS[358]
{0x0F12, 0x0018},  //000F,  //001F //TVAR_ash_pGAS[359]
{0x0F12, 0x0036},  //0021,  //0042 //TVAR_ash_pGAS[360]
{0x0F12, 0x005C},  //0037,  //0071 //TVAR_ash_pGAS[361]
{0x0F12, 0x008A},  //0054,  //00AE //TVAR_ash_pGAS[362]
{0x0F12, 0x00BC},  //006C,  //00E9 //TVAR_ash_pGAS[363]
{0x0F12, 0x00D1},  //006A,  //00E4 //TVAR_ash_pGAS[364]
{0x0F12, 0x009B},  //005B,  //00A7 //TVAR_ash_pGAS[365]
{0x0F12, 0x0069},  //0038,  //006C //TVAR_ash_pGAS[366]
{0x0F12, 0x0042},  //0021,  //003F //TVAR_ash_pGAS[367]
{0x0F12, 0x0022},  //000F,  //001E //TVAR_ash_pGAS[368]
{0x0F12, 0x000F},  //0005,  //000B //TVAR_ash_pGAS[369]
{0x0F12, 0x0008},  //0001,  //0004 //TVAR_ash_pGAS[370]
{0x0F12, 0x000D},  //0005,  //000D //TVAR_ash_pGAS[371]
{0x0F12, 0x001F},  //0010,  //0022 //TVAR_ash_pGAS[372]
{0x0F12, 0x003B},  //0021,  //0044 //TVAR_ash_pGAS[373]
{0x0F12, 0x0060},  //0036,  //0072 //TVAR_ash_pGAS[374]
{0x0F12, 0x008D},  //0053,  //00AE //TVAR_ash_pGAS[375]
{0x0F12, 0x00BF},  //006C,  //00EA //TVAR_ash_pGAS[376]
{0x0F12, 0x00E3},  //0072,  //00F5 //TVAR_ash_pGAS[377]
{0x0F12, 0x00AC},  //0066,  //00B9 //TVAR_ash_pGAS[378]
{0x0F12, 0x007A},  //0042,  //007D //TVAR_ash_pGAS[379]
{0x0F12, 0x0053},  //002A,  //0051 //TVAR_ash_pGAS[380]
{0x0F12, 0x0035},  //0018,  //002F //TVAR_ash_pGAS[381]
{0x0F12, 0x0022},  //000F,  //001C //TVAR_ash_pGAS[382]
{0x0F12, 0x001B},  //000B,  //0015 //TVAR_ash_pGAS[383]
{0x0F12, 0x001F},  //000F,  //001D //TVAR_ash_pGAS[384]
{0x0F12, 0x0030},  //0019,  //0031 //TVAR_ash_pGAS[385]
{0x0F12, 0x004B},  //0029,  //0053 //TVAR_ash_pGAS[386]
{0x0F12, 0x006D},  //003D,  //0080 //TVAR_ash_pGAS[387]
{0x0F12, 0x009C},  //005B,  //00BC //TVAR_ash_pGAS[388]
{0x0F12, 0x00CE},  //0072,  //00F7 //TVAR_ash_pGAS[389]
{0x0F12, 0x00FE},  //0087,  //0111 //TVAR_ash_pGAS[390]
{0x0F12, 0x00C9},  //0076,  //00D6 //TVAR_ash_pGAS[391]
{0x0F12, 0x0095},  //0053,  //009C //TVAR_ash_pGAS[392]
{0x0F12, 0x006F},  //003B,  //006F //TVAR_ash_pGAS[393]
{0x0F12, 0x0052},  //002A,  //004E //TVAR_ash_pGAS[394]
{0x0F12, 0x0040},  //0020,  //003A //TVAR_ash_pGAS[395]
{0x0F12, 0x0039},  //001C,  //0033 //TVAR_ash_pGAS[396]
{0x0F12, 0x003D},  //001F,  //003A //TVAR_ash_pGAS[397]
{0x0F12, 0x004B},  //0029,  //004E //TVAR_ash_pGAS[398]
{0x0F12, 0x0063},  //0037,  //006E //TVAR_ash_pGAS[399]
{0x0F12, 0x0086},  //004A,  //009B //TVAR_ash_pGAS[400]
{0x0F12, 0x00B5},  //006A,  //00D5 //TVAR_ash_pGAS[401]
{0x0F12, 0x00E6},  //0081,  //010F //TVAR_ash_pGAS[402]
{0x0F12, 0x011B},  //00A2,  //0139 //TVAR_ash_pGAS[403]
{0x0F12, 0x00ED},  //0093,  //00FD //TVAR_ash_pGAS[404]
{0x0F12, 0x00BA},  //006A,  //00C6 //TVAR_ash_pGAS[405]
{0x0F12, 0x0092},  //004F,  //0098 //TVAR_ash_pGAS[406]
{0x0F12, 0x0076},  //003F,  //0077 //TVAR_ash_pGAS[407]
{0x0F12, 0x0065},  //0035,  //0064 //TVAR_ash_pGAS[408]
{0x0F12, 0x005D},  //0032,  //005D //TVAR_ash_pGAS[409]
{0x0F12, 0x0060},  //0035,  //0064 //TVAR_ash_pGAS[410]
{0x0F12, 0x006D},  //003E,  //0076 //TVAR_ash_pGAS[411]
{0x0F12, 0x0084},  //004B,  //0095 //TVAR_ash_pGAS[412]
{0x0F12, 0x00A8},  //0061,  //00C2 //TVAR_ash_pGAS[413]
{0x0F12, 0x00D6},  //0082,  //00F8 //TVAR_ash_pGAS[414]
{0x0F12, 0x0101},  //0099,  //0135 //TVAR_ash_pGAS[415]
{0x0F12, 0x0140},  //00CC,  //016C //TVAR_ash_pGAS[416]
{0x0F12, 0x0112},  //00BC,  //0128 //TVAR_ash_pGAS[417]
{0x0F12, 0x00E5},  //008C,  //00F2 //TVAR_ash_pGAS[418]
{0x0F12, 0x00BD},  //006F,  //00C7 //TVAR_ash_pGAS[419]
{0x0F12, 0x009E},  //005E,  //00A4 //TVAR_ash_pGAS[420]
{0x0F12, 0x008C},  //0053,  //0092 //TVAR_ash_pGAS[421]
{0x0F12, 0x0085},  //0050,  //008A //TVAR_ash_pGAS[422]
{0x0F12, 0x0087},  //0052,  //008F //TVAR_ash_pGAS[423]
{0x0F12, 0x0094},  //005B,  //00A3 //TVAR_ash_pGAS[424]
{0x0F12, 0x00AC},  //006B,  //00C0 //TVAR_ash_pGAS[425]
{0x0F12, 0x00D0},  //0084,  //00EA //TVAR_ash_pGAS[426]
{0x0F12, 0x00F8},  //00A7,  //0121 //TVAR_ash_pGAS[427]
{0x0F12, 0x0123},  //00BE,  //016F //TVAR_ash_pGAS[428]
{0x0F12, 0x00F2},  //0086,  //0123 //TVAR_ash_pGAS[429]
{0x0F12, 0x00D1},  //006E,  //00E7 //TVAR_ash_pGAS[430]
{0x0F12, 0x00A7},  //0055,  //00BD //TVAR_ash_pGAS[431]
{0x0F12, 0x0087},  //0045,  //009C //TVAR_ash_pGAS[432]
{0x0F12, 0x0073},  //003E,  //0087 //TVAR_ash_pGAS[433]
{0x0F12, 0x0067},  //003B,  //007C //TVAR_ash_pGAS[434]
{0x0F12, 0x0064},  //003C,  //007B //TVAR_ash_pGAS[435]
{0x0F12, 0x006B},  //003E,  //0086 //TVAR_ash_pGAS[436]
{0x0F12, 0x007C},  //0045,  //0099 //TVAR_ash_pGAS[437]
{0x0F12, 0x0094},  //0052,  //00B7 //TVAR_ash_pGAS[438]
{0x0F12, 0x00B7},  //0063,  //00DC //TVAR_ash_pGAS[439]
{0x0F12, 0x00E1},  //0079,  //010E //TVAR_ash_pGAS[440]
{0x0F12, 0x00FF},  //008B,  //014A //TVAR_ash_pGAS[441]
{0x0F12, 0x00D6},  //0063,  //00F1 //TVAR_ash_pGAS[442]
{0x0F12, 0x00AE},  //0052,  //00C1 //TVAR_ash_pGAS[443]
{0x0F12, 0x0085},  //003D,  //0096 //TVAR_ash_pGAS[444]
{0x0F12, 0x0068},  //0032,  //0077 //TVAR_ash_pGAS[445]
{0x0F12, 0x0054},  //002B,  //0062 //TVAR_ash_pGAS[446]
{0x0F12, 0x0048},  //0029,  //0058 //TVAR_ash_pGAS[447]
{0x0F12, 0x0045},  //0029,  //0057 //TVAR_ash_pGAS[448]
{0x0F12, 0x004B},  //002C,  //0061 //TVAR_ash_pGAS[449]
{0x0F12, 0x005B},  //0034,  //0074 //TVAR_ash_pGAS[450]
{0x0F12, 0x0073},  //003C,  //0090 //TVAR_ash_pGAS[451]
{0x0F12, 0x0093},  //004B,  //00B7 //TVAR_ash_pGAS[452]
{0x0F12, 0x00BF},  //0060,  //00E7 //TVAR_ash_pGAS[453]
{0x0F12, 0x00E9},  //0070,  //0113 //TVAR_ash_pGAS[454]
{0x0F12, 0x00B8},  //0048,  //00CB //TVAR_ash_pGAS[455]
{0x0F12, 0x008E},  //003B,  //009D //TVAR_ash_pGAS[456]
{0x0F12, 0x0066},  //002B,  //0071 //TVAR_ash_pGAS[457]
{0x0F12, 0x0049},  //0023,  //0052 //TVAR_ash_pGAS[458]
{0x0F12, 0x0035},  //001B,  //0040 //TVAR_ash_pGAS[459]
{0x0F12, 0x0028},  //0017,  //0035 //TVAR_ash_pGAS[460]
{0x0F12, 0x0025},  //0017,  //0034 //TVAR_ash_pGAS[461]
{0x0F12, 0x002B},  //001B,  //003D //TVAR_ash_pGAS[462]
{0x0F12, 0x003B},  //0022,  //004F //TVAR_ash_pGAS[463]
{0x0F12, 0x0053},  //002C,  //006B //TVAR_ash_pGAS[464]
{0x0F12, 0x0072},  //0037,  //0090 //TVAR_ash_pGAS[465]
{0x0F12, 0x009D},  //0048,  //00C2 //TVAR_ash_pGAS[466]
{0x0F12, 0x00C8},  //0058,  //00EC //TVAR_ash_pGAS[467]
{0x0F12, 0x00A2},  //0033,  //00B0 //TVAR_ash_pGAS[468]
{0x0F12, 0x0078},  //002D,  //0082 //TVAR_ash_pGAS[469]
{0x0F12, 0x0051},  //001F,  //0057 //TVAR_ash_pGAS[470]
{0x0F12, 0x0034},  //0016,  //003A //TVAR_ash_pGAS[471]
{0x0F12, 0x001F},  //000E,  //0026 //TVAR_ash_pGAS[472]
{0x0F12, 0x0012},  //000C,  //001B //TVAR_ash_pGAS[473]
{0x0F12, 0x000E},  //000B,  //0019 //TVAR_ash_pGAS[474]
{0x0F12, 0x0014},  //000E,  //0021 //TVAR_ash_pGAS[475]
{0x0F12, 0x0024},  //0014,  //0033 //TVAR_ash_pGAS[476]
{0x0F12, 0x003B},  //001F,  //004F //TVAR_ash_pGAS[477]
{0x0F12, 0x005B},  //0028,  //0072 //TVAR_ash_pGAS[478]
{0x0F12, 0x0083},  //0038,  //00A2 //TVAR_ash_pGAS[479]
{0x0F12, 0x00AD},  //0045,  //00CF //TVAR_ash_pGAS[480]
{0x0F12, 0x0095},  //0028,  //009F //TVAR_ash_pGAS[481]
{0x0F12, 0x006C},  //0023,  //0072 //TVAR_ash_pGAS[482]
{0x0F12, 0x0046},  //0016,  //0047 //TVAR_ash_pGAS[483]
{0x0F12, 0x002A},  //000E,  //002A //TVAR_ash_pGAS[484]
{0x0F12, 0x0014},  //0008,  //0016 //TVAR_ash_pGAS[485]
{0x0F12, 0x0007},  //0004,  //000A //TVAR_ash_pGAS[486]
{0x0F12, 0x0002},  //0004,  //0008 //TVAR_ash_pGAS[487]
{0x0F12, 0x0008},  //0005,  //000F //TVAR_ash_pGAS[488]
{0x0F12, 0x0016},  //000A,  //0021 //TVAR_ash_pGAS[489]
{0x0F12, 0x002D},  //0013,  //003A //TVAR_ash_pGAS[490]
{0x0F12, 0x004C},  //001D,  //005C //TVAR_ash_pGAS[491]
{0x0F12, 0x0072},  //002D,  //008C //TVAR_ash_pGAS[492]
{0x0F12, 0x009B},  //0036,  //00BB //TVAR_ash_pGAS[493]
{0x0F12, 0x0093},  //0022,  //009A //TVAR_ash_pGAS[494]
{0x0F12, 0x006A},  //0020,  //006C //TVAR_ash_pGAS[495]
{0x0F12, 0x0045},  //0013,  //0042 //TVAR_ash_pGAS[496]
{0x0F12, 0x0028},  //000B,  //0024 //TVAR_ash_pGAS[497]
{0x0F12, 0x0013},  //0006,  //0010 //TVAR_ash_pGAS[498]
{0x0F12, 0x0005},  //0001,  //0004 //TVAR_ash_pGAS[499]
{0x0F12, 0x0000},  //0000,  //0000 //TVAR_ash_pGAS[500]
{0x0F12, 0x0004},  //0001,  //0007 //TVAR_ash_pGAS[501]
{0x0F12, 0x0012},  //0005,  //0018 //TVAR_ash_pGAS[502]
{0x0F12, 0x0028},  //000C,  //0030 //TVAR_ash_pGAS[503]
{0x0F12, 0x0045},  //0015,  //0050 //TVAR_ash_pGAS[504]
{0x0F12, 0x006A},  //0024,  //0080 //TVAR_ash_pGAS[505]
{0x0F12, 0x0093},  //002E,  //00AF //TVAR_ash_pGAS[506]
{0x0F12, 0x009B},  //0022,  //009F //TVAR_ash_pGAS[507]
{0x0F12, 0x0071},  //001F,  //0071 //TVAR_ash_pGAS[508]
{0x0F12, 0x004C},  //0014,  //0046 //TVAR_ash_pGAS[509]
{0x0F12, 0x0030},  //000B,  //0028 //TVAR_ash_pGAS[510]
{0x0F12, 0x001A},  //0006,  //0014 //TVAR_ash_pGAS[511]
{0x0F12, 0x000C},  //0002,  //0006 //TVAR_ash_pGAS[512]
{0x0F12, 0x0007},  //0000,  //0003 //TVAR_ash_pGAS[513]
{0x0F12, 0x000B},  //0000,  //0009 //TVAR_ash_pGAS[514]
{0x0F12, 0x0018},  //0004,  //0019 //TVAR_ash_pGAS[515]
{0x0F12, 0x002C},  //0009,  //0030 //TVAR_ash_pGAS[516]
{0x0F12, 0x0048},  //0012,  //0051 //TVAR_ash_pGAS[517]
{0x0F12, 0x006D},  //0021,  //0080 //TVAR_ash_pGAS[518]
{0x0F12, 0x0097},  //002B,  //00B0 //TVAR_ash_pGAS[519]
{0x0F12, 0x00AE},  //0029,  //00AD //TVAR_ash_pGAS[520]
{0x0F12, 0x0083},  //0026,  //0080 //TVAR_ash_pGAS[521]
{0x0F12, 0x005C},  //0019,  //0055 //TVAR_ash_pGAS[522]
{0x0F12, 0x0040},  //0010,  //0036 //TVAR_ash_pGAS[523]
{0x0F12, 0x002B},  //000B,  //0021 //TVAR_ash_pGAS[524]
{0x0F12, 0x001E},  //0008,  //0015 //TVAR_ash_pGAS[525]
{0x0F12, 0x0018},  //0005,  //0010 //TVAR_ash_pGAS[526]
{0x0F12, 0x001C},  //0005,  //0016 //TVAR_ash_pGAS[527]
{0x0F12, 0x0027},  //0008,  //0024 //TVAR_ash_pGAS[528]
{0x0F12, 0x003A},  //000D,  //003A //TVAR_ash_pGAS[529]
{0x0F12, 0x0055},  //0014,  //005B //TVAR_ash_pGAS[530]
{0x0F12, 0x007B},  //0025,  //008B //TVAR_ash_pGAS[531]
{0x0F12, 0x00A6},  //002E,  //00BA //TVAR_ash_pGAS[532]
{0x0F12, 0x00CA},  //0035,  //00C8 //TVAR_ash_pGAS[533]
{0x0F12, 0x009E},  //0031,  //0099 //TVAR_ash_pGAS[534]
{0x0F12, 0x0076},  //0021,  //006E //TVAR_ash_pGAS[535]
{0x0F12, 0x0059},  //001A,  //004E //TVAR_ash_pGAS[536]
{0x0F12, 0x0046},  //0015,  //003A //TVAR_ash_pGAS[537]
{0x0F12, 0x0039},  //0012,  //002D //TVAR_ash_pGAS[538]
{0x0F12, 0x0033},  //0010,  //002A //TVAR_ash_pGAS[539]
{0x0F12, 0x0036},  //0010,  //002E //TVAR_ash_pGAS[540]
{0x0F12, 0x0040},  //0012,  //003B //TVAR_ash_pGAS[541]
{0x0F12, 0x0052},  //0015,  //0051 //TVAR_ash_pGAS[542]
{0x0F12, 0x006C},  //001E,  //0072 //TVAR_ash_pGAS[543]
{0x0F12, 0x0094},  //002E,  //00A1 //TVAR_ash_pGAS[544]
{0x0F12, 0x00BF},  //0039,  //00D2 //TVAR_ash_pGAS[545]
{0x0F12, 0x00EB},  //004A,  //00EC //TVAR_ash_pGAS[546]
{0x0F12, 0x00C3},  //0043,  //00BE //TVAR_ash_pGAS[547]
{0x0F12, 0x0099},  //0031,  //0092 //TVAR_ash_pGAS[548]
{0x0F12, 0x007A},  //0027,  //0072 //TVAR_ash_pGAS[549]
{0x0F12, 0x0066},  //0023,  //005C //TVAR_ash_pGAS[550]
{0x0F12, 0x005A},  //0020,  //0050 //TVAR_ash_pGAS[551]
{0x0F12, 0x0054},  //001F,  //004D //TVAR_ash_pGAS[552]
{0x0F12, 0x0056},  //001F,  //0050 //TVAR_ash_pGAS[553]
{0x0F12, 0x005F},  //0020,  //005D //TVAR_ash_pGAS[554]
{0x0F12, 0x0071},  //0024,  //0073 //TVAR_ash_pGAS[555]
{0x0F12, 0x008D},  //002E,  //0094 //TVAR_ash_pGAS[556]
{0x0F12, 0x00B6},  //0042,  //00C2 //TVAR_ash_pGAS[557]
{0x0F12, 0x00DE},  //004B,  //00F4 //TVAR_ash_pGAS[558]
{0x0F12, 0x010D},  //0066,  //011A //TVAR_ash_pGAS[559]
{0x0F12, 0x00E7},  //0061,  //00E3 //TVAR_ash_pGAS[560]
{0x0F12, 0x00C1},  //0046,  //00B7 //TVAR_ash_pGAS[561]
{0x0F12, 0x00A0},  //003A,  //0097 //TVAR_ash_pGAS[562]
{0x0F12, 0x008A},  //0034,  //0081 //TVAR_ash_pGAS[563]
{0x0F12, 0x007C},  //002F,  //0075 //TVAR_ash_pGAS[564]
{0x0F12, 0x0076},  //002F,  //006F //TVAR_ash_pGAS[565]
{0x0F12, 0x0078},  //002F,  //0074 //TVAR_ash_pGAS[566]
{0x0F12, 0x0081},  //0031,  //0081 //TVAR_ash_pGAS[567]
{0x0F12, 0x0093},  //0037,  //0097 //TVAR_ash_pGAS[568]
{0x0F12, 0x00B1},  //0043,  //00B8 //TVAR_ash_pGAS[569]
{0x0F12, 0x00D5},  //0059,  //00E7 //TVAR_ash_pGAS[570]
{0x0F12, 0x00FD},  //0063,  //0127 //TVAR_ash_pGAS[571]

// Gamma
{0x002A, 0x04CC},
{0x0F12, 0x0000}, //0000 //SARR_usGammaLutRGBIndoor[0][0]
{0x0F12, 0x0002}, //0002 //SARR_usGammaLutRGBIndoor[0][1]
{0x0F12, 0x0008}, //0008 //SARR_usGammaLutRGBIndoor[0][2]
{0x0F12, 0x0016}, //0018 //SARR_usGammaLutRGBIndoor[0][3]
{0x0F12, 0x0055}, //005A //SARR_usGammaLutRGBIndoor[0][4]
{0x0F12, 0x00E6}, //00DF //SARR_usGammaLutRGBIndoor[0][5]
{0x0F12, 0x0141}, //013F //SARR_usGammaLutRGBIndoor[0][6]
{0x0F12, 0x0188}, //0186 //SARR_usGammaLutRGBIndoor[0][7]
{0x0F12, 0x01E6}, //01E6 //SARR_usGammaLutRGBIndoor[0][8]
{0x0F12, 0x0236}, //0236 //SARR_usGammaLutRGBIndoor[0][9]
{0x0F12, 0x02BA}, //02BA //SARR_usGammaLutRGBIndoor[0][10]
{0x0F12, 0x032A}, //032A //SARR_usGammaLutRGBIndoor[0][11]
{0x0F12, 0x0385}, //0385 //SARR_usGammaLutRGBIndoor[0][12]
{0x0F12, 0x03C2}, //03C2 //SARR_usGammaLutRGBIndoor[0][13]
{0x0F12, 0x03EA}, //03EA //SARR_usGammaLutRGBIndoor[0][14]
{0x0F12, 0x03FF}, //03FF //SARR_usGammaLutRGBIndoor[0][15]

{0x0F12, 0x0000}, //0000 //SARR_usGammaLutRGBIndoor[1][0]
{0x0F12, 0x0002}, //0002 //SARR_usGammaLutRGBIndoor[1][1]
{0x0F12, 0x0008}, //0008 //SARR_usGammaLutRGBIndoor[1][2]
{0x0F12, 0x0016}, //0018 //SARR_usGammaLutRGBIndoor[1][3]
{0x0F12, 0x0055}, //005A //SARR_usGammaLutRGBIndoor[1][4]
{0x0F12, 0x00E6}, //00DF //SARR_usGammaLutRGBIndoor[1][5]
{0x0F12, 0x0141}, //013F //SARR_usGammaLutRGBIndoor[1][6]
{0x0F12, 0x0188}, //0186 //SARR_usGammaLutRGBIndoor[1][7]
{0x0F12, 0x01E6}, //01E6 //SARR_usGammaLutRGBIndoor[1][8]
{0x0F12, 0x0236}, //0236 //SARR_usGammaLutRGBIndoor[1][9]
{0x0F12, 0x02BA}, //02BA //SARR_usGammaLutRGBIndoor[1][10]
{0x0F12, 0x032A}, //032A //SARR_usGammaLutRGBIndoor[1][11]
{0x0F12, 0x0385}, //0385 //SARR_usGammaLutRGBIndoor[1][12]
{0x0F12, 0x03C2}, //03C2 //SARR_usGammaLutRGBIndoor[1][13]
{0x0F12, 0x03EA}, //03EA //SARR_usGammaLutRGBIndoor[1][14]
{0x0F12, 0x03FF}, //03FF //SARR_usGammaLutRGBIndoor[1][15]

{0x0F12, 0x0000}, //0000 //SARR_usGammaLutRGBIndoor[2][0]
{0x0F12, 0x0002}, //0002 //SARR_usGammaLutRGBIndoor[2][1]
{0x0F12, 0x0008}, //0008 //SARR_usGammaLutRGBIndoor[2][2]
{0x0F12, 0x0016}, //0018 //SARR_usGammaLutRGBIndoor[2][3]
{0x0F12, 0x0055}, //005A //SARR_usGammaLutRGBIndoor[2][4]
{0x0F12, 0x00E6}, //00DF //SARR_usGammaLutRGBIndoor[2][5]
{0x0F12, 0x0141}, //013F //SARR_usGammaLutRGBIndoor[2][6]
{0x0F12, 0x0188}, //0186 //SARR_usGammaLutRGBIndoor[2][7]
{0x0F12, 0x01E6}, //01E6 //SARR_usGammaLutRGBIndoor[2][8]
{0x0F12, 0x0236}, //0236 //SARR_usGammaLutRGBIndoor[2][9]
{0x0F12, 0x02BA}, //02BA //SARR_usGammaLutRGBIndoor[2][10]
{0x0F12, 0x032A}, //032A //SARR_usGammaLutRGBIndoor[2][11]
{0x0F12, 0x0385}, //0385 //SARR_usGammaLutRGBIndoor[2][12]
{0x0F12, 0x03C2}, //03C2 //SARR_usGammaLutRGBIndoor[2][13]
{0x0F12, 0x03EA}, //03EA //SARR_usGammaLutRGBIndoor[2][14]
{0x0F12, 0x03FF}, //03FF //SARR_usGammaLutRGBIndoor[2][15]


//=================================================================================================
//	Set AWB
//=================================================================================================
{0x002A, 0x0DA6},
{0x0F12, 0x0000}, // awbb_LowBr_NBzone
{0x0F12, 0x0000}, // awbb_LowBr0_NBzone
{0x002A, 0x0E8C},
{0x0F12, 0x0000},	//awbb_LowBr0_PatchNumZone
{0x002A, 0x0D6C},
{0x0F12, 0x0040},	//awbb_YMedMoveToYAv

// Indoor Gray Zone
{0x002A, 0x0B9C},
{0x0F12, 0x038F}, //awbb_IndoorGrZones_m_BGrid_0__m_left
{0x0F12, 0x039B}, //awbb_IndoorGrZones_m_BGrid_0__m_right
{0x0F12, 0x0373}, //awbb_IndoorGrZones_m_BGrid_1__m_left
{0x0F12, 0x03B0}, //awbb_IndoorGrZones_m_BGrid_1__m_right
{0x0F12, 0x0352}, //awbb_IndoorGrZones_m_BGrid_2__m_left
{0x0F12, 0x03B7}, //awbb_IndoorGrZones_m_BGrid_2__m_right
{0x0F12, 0x0334}, //awbb_IndoorGrZones_m_BGrid_3__m_left
{0x0F12, 0x03B5}, //awbb_IndoorGrZones_m_BGrid_3__m_right
{0x0F12, 0x0318}, //awbb_IndoorGrZones_m_BGrid_4__m_left
{0x0F12, 0x03B0}, //awbb_IndoorGrZones_m_BGrid_4__m_right
{0x0F12, 0x02FF}, //awbb_IndoorGrZones_m_BGrid_5__m_left
{0x0F12, 0x038D}, //awbb_IndoorGrZones_m_BGrid_5__m_right
{0x0F12, 0x02E7}, //awbb_IndoorGrZones_m_BGrid_6__m_left
{0x0F12, 0x0372}, //awbb_IndoorGrZones_m_BGrid_6__m_right
{0x0F12, 0x02D0}, //awbb_IndoorGrZones_m_BGrid_7__m_left
{0x0F12, 0x035D}, //awbb_IndoorGrZones_m_BGrid_7__m_right
{0x0F12, 0x02B5}, //awbb_IndoorGrZones_m_BGrid_8__m_left
{0x0F12, 0x0345}, //awbb_IndoorGrZones_m_BGrid_8__m_right
{0x0F12, 0x02A1}, //awbb_IndoorGrZones_m_BGrid_9__m_left
{0x0F12, 0x0331}, //awbb_IndoorGrZones_m_BGrid_9__m_right
{0x0F12, 0x028B}, //awbb_IndoorGrZones_m_BGrid_10__m_left
{0x0F12, 0x031E}, //awbb_IndoorGrZones_m_BGrid_10__m_right
{0x0F12, 0x0273}, //awbb_IndoorGrZones_m_BGrid_11__m_left
{0x0F12, 0x0309}, //awbb_IndoorGrZones_m_BGrid_11__m_right
{0x0F12, 0x025F}, //awbb_IndoorGrZones_m_BGrid_12__m_left
{0x0F12, 0x02F5}, //awbb_IndoorGrZones_m_BGrid_12__m_right
{0x0F12, 0x0250}, //awbb_IndoorGrZones_m_BGrid_13__m_left
{0x0F12, 0x02DB}, //awbb_IndoorGrZones_m_BGrid_13__m_right
{0x0F12, 0x0241}, //awbb_IndoorGrZones_m_BGrid_14__m_left
{0x0F12, 0x02C7}, //awbb_IndoorGrZones_m_BGrid_14__m_right
{0x0F12, 0x0233}, //awbb_IndoorGrZones_m_BGrid_15__m_left
{0x0F12, 0x02B9}, //awbb_IndoorGrZones_m_BGrid_15__m_right
{0x0F12, 0x0223}, //awbb_IndoorGrZones_m_BGrid_16__m_left
{0x0F12, 0x02AB}, //awbb_IndoorGrZones_m_BGrid_16__m_right
{0x0F12, 0x0217}, //awbb_IndoorGrZones_m_BGrid_17__m_left
{0x0F12, 0x02A2}, //awbb_IndoorGrZones_m_BGrid_17__m_right
{0x0F12, 0x0207}, //awbb_IndoorGrZones_m_BGrid_18__m_left
{0x0F12, 0x0294}, //awbb_IndoorGrZones_m_BGrid_18__m_right
{0x0F12, 0x01FA}, //awbb_IndoorGrZones_m_BGrid_19__m_left
{0x0F12, 0x0289}, //awbb_IndoorGrZones_m_BGrid_19__m_right
{0x0F12, 0x01EA}, //awbb_IndoorGrZones_m_BGrid_20__m_left
{0x0F12, 0x0281}, //awbb_IndoorGrZones_m_BGrid_20__m_right
{0x0F12, 0x01DD}, //awbb_IndoorGrZones_m_BGrid_21__m_left
{0x0F12, 0x027B}, //awbb_IndoorGrZones_m_BGrid_21__m_right
{0x0F12, 0x01D0}, //awbb_IndoorGrZones_m_BGrid_22__m_left
{0x0F12, 0x0273}, //awbb_IndoorGrZones_m_BGrid_22__m_right
{0x0F12, 0x01C3}, //awbb_IndoorGrZones_m_BGrid_23__m_left
{0x0F12, 0x026A}, //awbb_IndoorGrZones_m_BGrid_23__m_right
{0x0F12, 0x01B6}, //awbb_IndoorGrZones_m_BGrid_24__m_left
{0x0F12, 0x0265}, //awbb_IndoorGrZones_m_BGrid_24__m_right
{0x0F12, 0x01AB}, //awbb_IndoorGrZones_m_BGrid_25__m_left
{0x0F12, 0x025B}, //awbb_IndoorGrZones_m_BGrid_25__m_right
{0x0F12, 0x01A1}, //awbb_IndoorGrZones_m_BGrid_26__m_left
{0x0F12, 0x0254}, //awbb_IndoorGrZones_m_BGrid_26__m_right
{0x0F12, 0x0198}, //awbb_IndoorGrZones_m_BGrid_27__m_left
{0x0F12, 0x024B}, //awbb_IndoorGrZones_m_BGrid_27__m_right
{0x0F12, 0x0192}, //awbb_IndoorGrZones_m_BGrid_28__m_left
{0x0F12, 0x0242}, //awbb_IndoorGrZones_m_BGrid_28__m_right
{0x0F12, 0x0191}, //awbb_IndoorGrZones_m_BGrid_29__m_left
{0x0F12, 0x023A}, //awbb_IndoorGrZones_m_BGrid_29__m_right
{0x0F12, 0x0192}, //awbb_IndoorGrZones_m_BGrid_30__m_left
{0x0F12, 0x0222}, //awbb_IndoorGrZones_m_BGrid_30__m_right
{0x0F12, 0x01C5}, //awbb_IndoorGrZones_m_BGrid_31__m_left
{0x0F12, 0x01DF}, //awbb_IndoorGrZones_m_BGrid_31__m_right
{0x0F12, 0x0000}, //awbb_IndoorGrZones_m_BGrid_32__m_left
{0x0F12, 0x0000}, //awbb_IndoorGrZones_m_BGrid_32__m_right
{0x0F12, 0x0000}, //awbb_IndoorGrZones_m_BGrid_33__m_left
{0x0F12, 0x0000}, //awbb_IndoorGrZones_m_BGrid_33__m_right


                  //  param_end	awbb_IndoorGrZones_m_BGrid
{0x002A, 0x0C3C},
{0x0F12, 0x0004},	//awbb_IndoorGrZones_m_GridStep
{0x0F12, 0x0000},
{0x0F12, 0x0022}, //awbb_IndoorGrZones_m_GridSz
{0x0F12, 0x0000},
{0x0F12, 0x010F}, //awbb_IndoorGrZones_m_Boffs   																												  //
{0x0F12, 0x0000},
{0x0F12, 0x0020}, //awbb_IndoorGrZones_y_low     																													//
{0x0F12, 0x0000},
{0x002A, 0x0C50},
{0x0F12, 0x00E0}, //awbb_IndoorGrZones_y_high  																												    //
{0x0F12, 0x0000},

// Outdoor Gray Zone
{0x0F12, 0x0264}, //awbb_OutdoorGrZones_m_BGrid_0__m_left
{0x0F12, 0x0279}, //awbb_OutdoorGrZones_m_BGrid_0__m_right
{0x0F12, 0x0250}, //awbb_OutdoorGrZones_m_BGrid_1__m_left
{0x0F12, 0x0287}, //awbb_OutdoorGrZones_m_BGrid_1__m_right
{0x0F12, 0x0244}, //awbb_OutdoorGrZones_m_BGrid_2__m_left
{0x0F12, 0x0287}, //awbb_OutdoorGrZones_m_BGrid_2__m_right
{0x0F12, 0x0235}, //awbb_OutdoorGrZones_m_BGrid_3__m_left
{0x0F12, 0x0289}, //awbb_OutdoorGrZones_m_BGrid_3__m_right
{0x0F12, 0x0225}, //awbb_OutdoorGrZones_m_BGrid_4__m_left
{0x0F12, 0x0287}, //awbb_OutdoorGrZones_m_BGrid_4__m_right
{0x0F12, 0x0213}, //awbb_OutdoorGrZones_m_BGrid_5__m_left
{0x0F12, 0x0286}, //awbb_OutdoorGrZones_m_BGrid_5__m_right
{0x0F12, 0x0202}, //awbb_OutdoorGrZones_m_BGrid_6__m_left
{0x0F12, 0x027A}, //awbb_OutdoorGrZones_m_BGrid_6__m_right
{0x0F12, 0x01F3}, //awbb_OutdoorGrZones_m_BGrid_7__m_left
{0x0F12, 0x0272}, //awbb_OutdoorGrZones_m_BGrid_7__m_right
{0x0F12, 0x01E9}, //awbb_OutdoorGrZones_m_BGrid_8__m_left
{0x0F12, 0x0269}, //awbb_OutdoorGrZones_m_BGrid_8__m_right
{0x0F12, 0x01E2}, //awbb_OutdoorGrZones_m_BGrid_9__m_left
{0x0F12, 0x0263}, //awbb_OutdoorGrZones_m_BGrid_9__m_right
{0x0F12, 0x01E0}, //awbb_OutdoorGrZones_m_BGrid_10__m_left
{0x0F12, 0x025A}, //awbb_OutdoorGrZones_m_BGrid_10__m_right
{0x0F12, 0x01E1}, //awbb_OutdoorGrZones_m_BGrid_11__m_left
{0x0F12, 0x0256}, //awbb_OutdoorGrZones_m_BGrid_11__m_right
{0x0F12, 0x01EE}, //awbb_OutdoorGrZones_m_BGrid_12__m_left
{0x0F12, 0x0251}, //awbb_OutdoorGrZones_m_BGrid_12__m_right
{0x0F12, 0x01F8}, //awbb_OutdoorGrZones_m_BGrid[26]
{0x0F12, 0x024A}, //awbb_OutdoorGrZones_m_BGrid[27]
{0x0F12, 0x020D}, //awbb_OutdoorGrZones_m_BGrid[28]
{0x0F12, 0x0231}, //awbb_OutdoorGrZones_m_BGrid[29]
{0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[30]
{0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[31]
{0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[32]
{0x0F12, 0x0000}, //awbb_OutdoorGrZones_m_BGrid[33]


//  param_WRITE 70000CC6  B2end	awbb_OutdoorGrZones_m_BGrid 																							//
{0x002A, 0x0CB8},
{0x0F12, 0x0004}, // awbb_OutdoorGrZones_m_GridStep         																							  //
{0x0F12, 0x0000},
{0x0F12, 0x0011}, // awbb_OutdoorGrZones_m_GridSz            																							//
{0x0F12, 0x0000},
{0x0F12, 0x01FF}, // awbb_OutdoorGrZones_m_Boffs             																							//
{0x0F12, 0x0000},
{0x0F12, 0x0020}, // awbb_OutdoorGrZones_y_low               																													//
{0x0F12, 0x0000},
{0x002A, 0x0CCC},
{0x0F12, 0x00C0}, // awbb_OutdoorGrZones_y_high          																													    //
{0x0F12, 0x0000},

// Low Brightness Gray Zone
{0x0F12, 0x031F}, // awbb_LowBrGrZones_m_BGrid_0__m_left  																															//
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_0__m_right 																															//
{0x0F12, 0x02FC}, // awbb_LowBrGrZones_m_BGrid_1__m_left  																															//
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_1__m_right 																															//
{0x0F12, 0x02D9}, // awbb_LowBrGrZones_m_BGrid_2__m_left  																															//
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_2__m_right 																															//
{0x0F12, 0x02B6}, // awbb_LowBrGrZones_m_BGrid_3__m_left  																															//
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_3__m_right 																															//
{0x0F12, 0x0293}, // awbb_LowBrGrZones_m_BGrid_4__m_left  																															//
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_4__m_right 																															//
{0x0F12, 0x0270}, // awbb_LowBrGrZones_m_BGrid_5__m_left  																															//
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_5__m_right 																															//
{0x0F12, 0x024E}, // awbb_LowBrGrZones_m_BGrid_6__m_left  																															//
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_6__m_right 																															//
{0x0F12, 0x022B}, // awbb_LowBrGrZones_m_BGrid_7__m_left  																															//
{0x0F12, 0x0495}, // awbb_LowBrGrZones_m_BGrid_7__m_right 																															//
{0x0F12, 0x0208}, // awbb_LowBrGrZones_m_BGrid_8__m_left  																															//
{0x0F12, 0x048A}, // awbb_LowBrGrZones_m_BGrid_8__m_right 																															//
{0x0F12, 0x01E5}, // awbb_LowBrGrZones_m_BGrid_9__m_left  																															//
{0x0F12, 0x0455}, // awbb_LowBrGrZones_m_BGrid_9__m_right 																															//
{0x0F12, 0x01C2}, // awbb_LowBrGrZones_m_BGrid_10__m_left 																															//
{0x0F12, 0x041F}, // awbb_LowBrGrZones_m_BGrid_10__m_right																															//
{0x0F12, 0x019F}, // awbb_LowBrGrZones_m_BGrid_11__m_left 																															//
{0x0F12, 0x03EA}, // awbb_LowBrGrZones_m_BGrid_11__m_right																															//
{0x0F12, 0x017D}, // awbb_LowBrGrZones_m_BGrid_12__m_left 																															//
{0x0F12, 0x03B4}, // awbb_LowBrGrZones_m_BGrid_12__m_right																															//
{0x0F12, 0x015A}, // awbb_LowBrGrZones_m_BGrid_13__m_left 																															//
{0x0F12, 0x037F}, // awbb_LowBrGrZones_m_BGrid_13__m_right																															//
{0x0F12, 0x0137}, // awbb_LowBrGrZones_m_BGrid_14__m_left 																															//
{0x0F12, 0x0349}, // awbb_LowBrGrZones_m_BGrid_14__m_right																															//
{0x0F12, 0x0130}, // awbb_LowBrGrZones_m_BGrid_15__m_left 																															//
{0x0F12, 0x0314}, // awbb_LowBrGrZones_m_BGrid_15__m_right																															//
{0x0F12, 0x012F}, // awbb_LowBrGrZones_m_BGrid_16__m_left 																															//
{0x0F12, 0x02DE}, // awbb_LowBrGrZones_m_BGrid_16__m_right																															//
{0x0F12, 0x012F}, // awbb_LowBrGrZones_m_BGrid_17__m_left 																															//
{0x0F12, 0x02B1}, // awbb_LowBrGrZones_m_BGrid_17__m_right																															//
{0x0F12, 0x012E}, // awbb_LowBrGrZones_m_BGrid_18__m_left 																															//
{0x0F12, 0x028B}, // awbb_LowBrGrZones_m_BGrid_18__m_right																															//
{0x0F12, 0x012D}, // awbb_LowBrGrZones_m_BGrid_19__m_left 																															//
{0x0F12, 0x0265}, // awbb_LowBrGrZones_m_BGrid_19__m_right																															//
{0x0F12, 0x012C}, // awbb_LowBrGrZones_m_BGrid_20__m_left 																															//
{0x0F12, 0x023F}, // awbb_LowBrGrZones_m_BGrid_20__m_right																															//
{0x0F12, 0x012C}, // awbb_LowBrGrZones_m_BGrid_21__m_left 																															//
{0x0F12, 0x0219}, // awbb_LowBrGrZones_m_BGrid_21__m_right																															//
{0x0F12, 0x012B}, // awbb_LowBrGrZones_m_BGrid_22__m_left 																															//
{0x0F12, 0x01F3}, // awbb_LowBrGrZones_m_BGrid_22__m_right																															//
{0x0F12, 0x012A}, // awbb_LowBrGrZones_m_BGrid_23__m_left 																															//
{0x0F12, 0x01CD}, // awbb_LowBrGrZones_m_BGrid_23__m_right																															//
{0x0F12, 0x0000}, // awbb_LowBrGrZones_m_BGrid_24__m_left 																															//
{0x0F12, 0x0000}, // awbb_LowBrGrZones_m_BGrid_24__m_right																															//


//  42param_end	awbb_LowBrGrZones_m_BGrid 																												  //
{0x0F12, 0x0005},
{0x0F12, 0x0000}, //awbb_LowBrGrZones_m_GridStep																																			  //
{0x0F12, 0x0018},
{0x0F12, 0x0000}, //awbb_LowBrGrZones_m_GridSz  																																				//
{0x0F12, 0x00AF},
{0x0F12, 0x0000}, //awbb_LowBrGrZones_m_Boffs  																																			  //
{0x0F12, 0x0002},
{0x0F12, 0x0000}, //A awbb_LowBrGrZones_y_low  																																			  //
{0x002A, 0x0D48},
{0x0F12, 0x00E0},
{0x0F12, 0x0000}, //awbb_LowBrGrZones_y_high    																																				//

 // Lowtemp circle                            																											  //
{0x0F12, 0x032F},
{0x0F12, 0x0000}, //awbb_CrclLowT_R_c          																																				//
{0x0F12, 0x017A},
{0x0F12, 0x0000}, //awbb_CrclLowT_B_c         																																				  //
{0x0F12, 0x7300},
{0x0F12, 0x0000}, //awbb_CrclLowT_Rad_c       																																				  //
{0x0F12, 0x000A},
{0x0F12, 0x0000}, //awbb_CrclLowT_y_low   																																				      //
{0x002A, 0x0D60},
{0x0F12, 0x00E0},
{0x0F12, 0x0000}, //awbb_CrclLowT_y_high 																																				      //
{0x002A, 0x0D82},
{0x0F12, 0x0001},
                  //awbb_ByPass_LowTempMode  																																				  //

// Duks add 																																													//
{0x002A, 0x0D8E},
{0x0F12, 0x0002}, // awbb_GridEnable 																																									//

 // Grid coefficients and Contrants 																																	//
{0x002A, 0x0DCE},
{0x0F12, 0x0000}, //awbb_GridCorr_R_0__0_ 																																							//
{0x0F12, 0x0000}, //awbb_GridCorr_R_0__1_ 																																							//
{0x0F12, 0x0040}, //awbb_GridCorr_R_0__2_ 																																							//
{0x0F12, 0xFFF8}, //awbb_GridCorr_R_0__3_ 																																							//
{0x0F12, 0xFFE0}, //awbb_GridCorr_R_0__4_ 																																							//
{0x0F12, 0xFFD0}, //awbb_GridCorr_R_0__5_ 																																							//

{0x0F12, 0x0000}, //awbb_GridCorr_R_1__0_ 																																							//
{0x0F12, 0x0000}, //awbb_GridCorr_R_1__1_ 																																							//
{0x0F12, 0x0040}, //awbb_GridCorr_R_1__2_ 																																							//
{0x0F12, 0xFFF8}, //awbb_GridCorr_R_1__3_ 																																							//
{0x0F12, 0xFFE0}, //awbb_GridCorr_R_1__4_ 																																							//
{0x0F12, 0xFFD0}, //awbb_GridCorr_R_1__5_ 																																							//

{0x0F12, 0x0000}, //awbb_GridCorr_R_2__0_ 																																							//
{0x0F12, 0x0000}, //awbb_GridCorr_R_2__1_ 																																							//
{0x0F12, 0x0040}, //awbb_GridCorr_R_2__2_ 																																							//
{0x0F12, 0xFFF8}, //awbb_GridCorr_R_2__3_ 																																							//
{0x0F12, 0xFFE0}, //awbb_GridCorr_R_2__4_ 																																							//
{0x0F12, 0xFFD0}, //awbb_GridCorr_R_2__5_ 																																							//

{0x0F12, 0x0000}, //awbb_GridCorr_B_0__0_ 																																							//
{0x0F12, 0x0000}, //awbb_GridCorr_B_0__1_ 																																							//
{0x0F12, 0x0040}, //awbb_GridCorr_B_0__2_ 																																							//
{0x0F12, 0x0000}, //awbb_GridCorr_B_0__3_ 																																							//
{0x0F12, 0x0028}, //awbb_GridCorr_B_0__4_ 																																							//
{0x0F12, 0x0050}, //awbb_GridCorr_B_0__5_ 																																							//

{0x0F12, 0x0000}, //awbb_GridCorr_B_1__0_ 																																							//
{0x0F12, 0x0000}, //awbb_GridCorr_B_1__1_ 																																							//
{0x0F12, 0x0040}, //awbb_GridCorr_B_1__2_ 																																							//
{0x0F12, 0x0000}, //awbb_GridCorr_B_1__3_ 																																							//
{0x0F12, 0x0028}, //awbb_GridCorr_B_1__4_ 																																							//
{0x0F12, 0x0050}, //awbb_GridCorr_B_1__5_ 																																							//

{0x0F12, 0x0000}, //awbb_GridCorr_B_2__0_ 																																							//
{0x0F12, 0x0000}, //awbb_GridCorr_B_2__1_ 																																							//
{0x0F12, 0x0040}, //awbb_GridCorr_B_2__2_ 																																							//
{0x0F12, 0x0000}, //awbb_GridCorr_B_2__3_ 																																							//
{0x0F12, 0x0028}, //awbb_GridCorr_B_2__4_ 																																							//
{0x0F12, 0x0050}, //awbb_GridCorr_B_2__5_ 																																							//

{0x0F12, 0x02C6}, //awbb_GridConst_1_0_ 																																								//
{0x0F12, 0x0335}, //awbb_GridConst_1_1_ 																																								//
{0x0F12, 0x03B3}, //awbb_GridConst_1_2_ 																																								//
{0x0F12, 0x0FD7}, //awbb_GridConst_2_0  																																								//
{0x0F12, 0x10C5}, //awbb_GridConst_2_1  																																								//
{0x0F12, 0x116A}, //awbb_GridConst_2_2  																																								//
{0x0F12, 0x117C}, //awbb_GridConst_2_3  																																								//
{0x0F12, 0x11C2}, //awbb_GridConst_2_4  																																								//
{0x0F12, 0x120B}, //awbb_GridConst_2_5  																																								//

{0x0F12, 0x00B3}, //awbb_GridCoeff_R_1 																																								//
{0x0F12, 0x00B7}, //awbb_GridCoeff_B_1 																																								//
{0x0F12, 0x00D3}, //awbb_GridCoeff_R_2 																																								//
{0x0F12, 0x0091}, //awbb_GridCoeff_B_2 																																								//

// White Locus 																																											//
{0x002A, 0x0D66},
{0x0F12, 0x0133}, // awbb_IntcR																																												//
{0x0F12, 0x010F}, // awbb_IntcB																																												//
{0x002A, 0x0D74},
{0x0F12, 0x052A}, // awbb_MvEq_RBthresh 																																								//

// Gamut Thresholds
{0x002A, 0x0DAE},
{0x0F12, 0x0036}, //awbb_GamutWidthThr2 																																	  					  //
{0x0F12, 0x001C}, //awbb_GamutHeightThr2																																	  					  //
{0x002A, 0x0DAA},
{0x0F12, 0x071A}, //awbb_GamutWidthThr1  																																							//
{0x0F12, 0x03A4}, //awbb_GamutHeightThr1 																																							//

// SceneDetection Thresholds
{0x002A, 0x0D92},
{0x0F12, 0x0BB8}, //awbb_SunnyBr		              																																			//
{0x0F12, 0x0096}, //awbb_Sunny_NBzone	          																																			//
{0x002A, 0x0E86},
{0x0F12, 0x0216}, //awbb_OutdoorWP_r             																																			//
{0x0F12, 0x029F}, //awbb_OutdoorWP_b             																																			//
{0x002A, 0x0D96},
{0x0F12, 0x0BB7}, //awbb_CloudyBr		          																																			  //
{0x0F12, 0x0096}, //awbb_Cloudy_NBzone	        																																			  //
{0x002A, 0x0DB2},
{0x0F12, 0x00DA}, //awbb_CloudyRB              																																			  //
{0x002A, 0x0D9A},
{0x0F12, 0x000A}, //awbb_Cloudy_BdivRzone        																																			//
{0x002A, 0x0DB4},
{0x0F12, 0x0459}, //awbb_LowTempRB               																																			//
{0x002A, 0x0DA4},
{0x0F12, 0x000E}, //awbb_LowTemp_RBzone       																																				  //
{0x002A, 0x0D64},
{0x0F12, 0x0032}, //awbb_DarkBr		             																																				//
{0x002A, 0x0DA6},
{0x0F12, 0x001E}, //awbb_LowBr_NBzone	         																																				//
{0x002A, 0x0D9C},
{0x0F12, 0x001B}, //awbb_MacbethGamut_WidthZone  																																			//
{0x0F12, 0x000E}, //awbb_MacbethGamut_HeightZone 																																			//
{0x0F12, 0x0008}, //awbb_MacbethGamut_WidthZone2 																																			//
{0x0F12, 0x0004}, //awbb_MacbethGamut_HeightZone2																																			//

// AWB Debug.(Outdoor Pink)																																				  //
{0x002A, 0x0E30},
{0x0F12, 0x0000}, //awbb_OutdoorFltrSz (outdoor WB moving average filtering)																						//
{0x002A, 0x0E84},
{0x0F12, 0x0000},

//  UseInvalidOutdoor option																																					//
{0x002A, 0x0D88},
{0x0F12, 0x0001}, //awbb_Use_InvalidOutDoor																																						//

// AWB input Y-Filter setting    																																		//
{0x002A, 0x0C48},
{0x0F12, 0x0020}, //awbb_IndoorGrZones_y_low  																																					//
{0x002A, 0x0C50},
{0x0F12, 0x00E0}, //awbb_IndoorGrZones_y_high 																																					//
{0x002A, 0x0CC4},
{0x0F12, 0x0020}, //awbb_OutdoorGrZones_y_low 																																					//
{0x002A, 0x0CCC},
{0x0F12, 0x00C0}, //awbb_OutdoorGrZones_y_high																																					//

// awbb_ChromaClassifyEn, default : enable 																																						//
{0x002A, 0x0DC2},
{0x0F12, 0x0030}, //awbb_GnCurPntImmunity 																																							//
{0x0F12, 0x00C8}, //awbb_GnFarPntImmunity 																																							//
{0x0F12, 0x012C}, //awbb_GnCurPntLongJump 																																							//
{0x0F12, 0x0258}, //awbb_GainsMaxMove     																																							//
{0x0F12, 0x0003}, //awbb_GnMinMatchToJump 																																							//


//=========================
//	Set CCM
//=========================
// CCM Start Address
{0x002A, 0x06D0},
{0x0F12, 0x2800}, //TVAR_wbt_pBaseCcmsAddr[0] 																																					//
{0x0F12, 0x7000},
{0x0F12, 0x2824}, //TVAR_wbt_pBaseCcmsAddr[1]																																					//
{0x0F12, 0x7000},
{0x0F12, 0x2848}, //TVAR_wbt_pBaseCcmsAddr[2]																																					//
{0x0F12, 0x7000},
{0x0F12, 0x286C}, //TVAR_wbt_pBaseCcmsAddr[3] 																																					//
{0x0F12, 0x7000},
{0x0F12, 0x2890}, //TVAR_wbt_pBaseCcmsAddr[4]																																					//
{0x0F12, 0x7000},
{0x0F12, 0x28B4}, //TVAR_wbt_pBaseCcmsAddr[5] 																																					//
{0x0F12, 0x7000},
{0x002A, 0x06EC},
{0x0F12, 0x28D8}, //TVAR_wbt_pOutdoorCcm      																																					//
{0x0F12, 0x7000},

//  param_start	TVAR_wbt_pBaseCcms  																																//
{0x002A, 0x2800},
{0x0F12, 0x01E5}, //01D6 //01E1	//01FB
{0x0F12, 0xFFBF}, //FFC9 //FFC4	//FF9C
{0x0F12, 0xFFF4}, //FFFB //FFF8	//FFFF
{0x0F12, 0x00EE}, //010A //0101	//0137
{0x0F12, 0x019B}, //0177 //014C	//0113
{0x0F12, 0xFF1E}, //FF24 //FF55	//FF6F
{0x0F12, 0xFF38}, //FF9C //FF5B	//FF21
{0x0F12, 0x026D}, //0230 //0205	//0194
{0x0F12, 0xFEDD}, //FEB1 //FF17	//FF69
{0x0F12, 0xFF00}, //FEFF //FEFE	//FF14
{0x0F12, 0x01B8}, //01B7 //01B6	//0158
{0x0F12, 0x0109}, //0108 //0107	//015D
{0x0F12, 0xFFCC}, //FFEB //FFDB	//FFF2
{0x0F12, 0xFFA4}, //FFB5 //FFDB	//FFF1
{0x0F12, 0x0212}, //01E4 //01D1	//0179
{0x0F12, 0x0136}, //011E //0163	//017C
{0x0F12, 0xFFA5}, //FFAD //FF9E	//FFC3
{0x0F12, 0x01CF}, //01E3 //01B3	//0197

{0x0F12, 0x01E5}, //01D6 //01E1	//01FB
{0x0F12, 0xFFBF}, //FFC9 //FFC4	//FF9C
{0x0F12, 0xFFF4}, //FFFB //FFF8	//FFFF
{0x0F12, 0x00EE}, //010A //0101	//0137
{0x0F12, 0x019B}, //0177 //014C	//0113
{0x0F12, 0xFF1E}, //FF24 //FF55	//FF6F
{0x0F12, 0xFF38}, //FF9C //FF5B	//FF21
{0x0F12, 0x026D}, //0230 //0205	//0194
{0x0F12, 0xFEDD}, //FEB1 //FF17	//FF69
{0x0F12, 0xFF00}, //FEFF //FEFE	//FF14
{0x0F12, 0x01B8}, //01B7 //01B6	//0158
{0x0F12, 0x0109}, //0108 //0107	//015D
{0x0F12, 0xFFCC}, //FFEB //FFDB	//FFF2
{0x0F12, 0xFFA4}, //FFB5 //FFDB	//FFF1
{0x0F12, 0x0212}, //01E4 //01D1	//0179
{0x0F12, 0x0136}, //011E //0163	//017C
{0x0F12, 0xFFA5}, //FFAD //FF9E	//FFC3
{0x0F12, 0x01CF}, //01E3 //01B3	//0197

{0x0F12, 0x01E1},	//01FB
{0x0F12, 0xFFC4},	//FF9C
{0x0F12, 0xFFF8},	//FFFF
{0x0F12, 0x0101},	//0137
{0x0F12, 0x014C},	//0113
{0x0F12, 0xFF55},	//FF6F
{0x0F12, 0xFF5B},	//FF21
{0x0F12, 0x0205},	//0194
{0x0F12, 0xFF17},	//FF69
{0x0F12, 0xFEFE},	//FF14
{0x0F12, 0x01B6},	//0158
{0x0F12, 0x0107},	//015D
{0x0F12, 0xFFDB},	//FFF2
{0x0F12, 0xFFDB},	//FFF1
{0x0F12, 0x01D1},	//0179
{0x0F12, 0x0163},	//017C
{0x0F12, 0xFF9E},	//FFC3
{0x0F12, 0x01B3},	//0197

{0x0F12, 0x01FB}, //01FB,	//01FB
{0x0F12, 0xFFA9}, //FFA9,	//FF9C
{0x0F12, 0xFFEA}, //FFEA,	//FFFF
{0x0F12, 0x013C}, //0134,	//0137
{0x0F12, 0x0140}, //0133,	//0113
{0x0F12, 0xFF53}, //FF5D,	//FF6F
{0x0F12, 0xFE7A}, //FE7A,	//FF21
{0x0F12, 0x017D}, //017D,	//0194
{0x0F12, 0xFEED}, //FEED,	//FF69
{0x0F12, 0xFF39}, //FF39,	//FF14
{0x0F12, 0x01D6}, //01D6,	//0158
{0x0F12, 0x00C4}, //00C4,	//015D
{0x0F12, 0xFFC0}, //FFCE,	//FFF2
{0x0F12, 0xFFBF}, //FFCD,	//FFF1
{0x0F12, 0x01CD}, //01B7,	//0179
{0x0F12, 0x0182}, //0176,	//017C
{0x0F12, 0xFF91}, //FFBD,	//FFC3
{0x0F12, 0x01AA}, //0191,	//0197

{0x0F12, 0x01D2}, //01F9, //020A //01FB	//01FB
{0x0F12, 0xFFC2}, //FFBC, //FFB2 //FFA9	//FF9C
{0x0F12, 0xFFFC}, //FFF2, //FFEB //FFEA	//FFFF
{0x0F12, 0x00E8}, //00FA, //0134	//0137
{0x0F12, 0x0126}, //0157, //0133	//0113
{0x0F12, 0xFF83}, //FF81, //FF5D	//FF6F
{0x0F12, 0xFF7A}, //FE7A, //FEFD //FE8D //FE7A	//FF21
{0x0F12, 0x017D}, //017D, //01BF //028C //017D	//0194
{0x0F12, 0xFEED}, //FEED, //FF2A //FECE //FEED	//FF69
{0x0F12, 0xFF1C}, //FF8A, //FF39	//FF14
{0x0F12, 0x0194}, //01F9, //01D6	//0158
{0x0F12, 0x011F}, //005B, //00C4	//015D
{0x0F12, 0xFFEA}, //FFCA, //FFCE	//FFF2
{0x0F12, 0xFFDE}, //FFA3, //FFCD	//FFF1
{0x0F12, 0x01E9}, //01DA, //01B7	//0179
{0x0F12, 0x0178}, //0108, //0176	//017C
{0x0F12, 0xFFBF}, //FFB3, //FFBD	//FFC3
{0x0F12, 0x0193}, //01DD, //0191	//0197

{0x0F12, 0x01D2}, //01F9, //020A //01D2	//01D0   R
{0x0F12, 0xFFC2}, //FFBC, //FFB2 //FFC2	//FFB4
{0x0F12, 0xFFFC}, //FFF2, //FFEB //FFFC	//000C
{0x0F12, 0x00E8}, //00FA, //011E	//0122   Y
{0x0F12, 0x0126}, //0157, //011D	//0103
{0x0F12, 0xFF83}, //FF81, //FF86	//FF9B
{0x0F12, 0xFE7A}, //FE7A, //FEFD //FE8D //FE78	//FF33   G
{0x0F12, 0x017D}, //017D, //01BF //028C //017B	//01C5
{0x0F12, 0xFEED}, //FEED, //FF2A //FECE //FEEB	//FF33
{0x0F12, 0xFF1C}, //FF8A, //FF38	//FF16   C
{0x0F12, 0x0194}, //01F9, //01D5	//015A
{0x0F12, 0x011F}, //005B, //00C3	//015F
{0x0F12, 0xFFEA}, //FFCA, //FFCF	//FFE0   B
{0x0F12, 0xFFDE}, //FFA3, //FFCE	//FFDF
{0x0F12, 0x01E9}, //01DA, //01B8	//0197
{0x0F12, 0x0178}, //0108, //0178	//0178   M
{0x0F12, 0xFFBF}, //FFB3, //FFBF	//FFBF
{0x0F12, 0x0193}, //01DD, //0193	//0193

{0x0F12, 0x01F1}, //01E0, // outdoor CCM 																																											//
{0x0F12, 0xFFB0}, //FFBF,
{0x0F12, 0xFFEF}, //FFFD,
{0x0F12, 0x00F4}, //00F5,
{0x0F12, 0x0139}, //0139,
{0x0F12, 0xFF64}, //FF74,
{0x0F12, 0xFEEC}, //FEEC,
{0x0F12, 0x01FD}, //01FD,
{0x0F12, 0xFF8E}, //FF8E,
{0x0F12, 0xFF4E}, //FEFE,
{0x0F12, 0x0164}, //01B6,
{0x0F12, 0x011D}, //0107,
{0x0F12, 0xFFEA}, //FFDB,
{0x0F12, 0xFFDE}, //FFDB,
{0x0F12, 0x01E9}, //01D1,
{0x0F12, 0x0178}, //0163,
{0x0F12, 0xFFBF}, //FF9E,
{0x0F12, 0x0193}, //01B3,
//=================================================================================================
//	Set NB
//=================================================================================================
{0x002A, 0x07EA},
{0x0F12, 0x0000},	//afit_bUseNoiseInd 0 : NB 1: Noise Index

//  param_start	SARR_uNormBrInDoor 																																	//
{0x0F12, 0x000A}, //SARR_uNormBrInDoor[0] 																																							//
{0x0F12, 0x0019}, //SARR_uNormBrInDoor[1] 																																							//
{0x0F12, 0x007D}, //SARR_uNormBrInDoor[2] 																																							//
{0x0F12, 0x02BC}, //SARR_uNormBrInDoor[3] 																																							//
{0x0F12, 0x07D0}, //SARR_uNormBrInDoor[4] 																																							//

// param_start	SARR_uNormBrOutDoor 																																	//
{0x0F12, 0x000A}, //SARR_uNormBrOutDoor[0] 																																						//
{0x0F12, 0x0019}, //SARR_uNormBrOutDoor[1] 																																						//
{0x0F12, 0x007D}, //SARR_uNormBrOutDoor[2] 																																						//
{0x0F12, 0x02BC}, //SARR_uNormBrOutDoor[3] 																																						//
{0x0F12, 0x07D0}, //SARR_uNormBrOutDoor[4] 																																						//

//=================================================================================================
//	Set AFIT
//=================================================================================================
// AFIT Start Address
{0x002A, 0x0814},
{0x0F12, 0x082C}, // TVAR_afit_pBaseVals  																																							//
{0x0F12, 0x7000}, // TVAR_afit_pBaseVals  																																							//

//  param_start	TVAR_afit_pBaseVals																																	//
{0x002A, 0x082C},
{0x0F12, 0x0003}, //1D, //00, // BRIGHTNESS
{0x0F12, 0x0000}, // CONTRAST
{0x0F12, 0xFFFE}, // SATURATION
{0x0F12, 0xFFE2}, // SHARP_BLUR
{0x0F12, 0x0000}, // GLAMOUR
{0x0F12, 0x03FF}, // Disparity_iSatSat
{0x0F12, 0x03FF}, // Denoise1_iYDenThreshLow
{0x0F12, 0x0028}, // Denoise1_iYDenThreshLow_Bin
{0x0F12, 0x03FF}, // Denoise1_iYDenThreshHigh
{0x0F12, 0x00FF}, // Denoise1_iYDenThreshHigh_Bin
{0x0F12, 0x0002}, // Denoise1_iLowWWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWWideThresh
{0x0F12, 0x000A}, // Denoise1_iLowWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWideThresh
{0x0F12, 0x03FF}, // Denoise1_iSatSat
{0x0F12, 0x03FF}, // Demosaic4_iHystGrayLow
{0x0F12, 0x0000}, // Demosaic4_iHystGrayHigh
{0x0F12, 0x0344}, // UVDenoise_iYLowThresh
{0x0F12, 0x033A}, // UVDenoise_iYHighThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVLowThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVHighThresh
{0x0F12, 0x0028}, // DSMix1_iLowLimit_Wide
{0x0F12, 0x0032}, // DSMix1_iLowLimit_Wide_Bin
{0x0F12, 0x0014}, // DSMix1_iHighLimit_Wide
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Wide_Bin
{0x0F12, 0x0050}, // DSMix1_iLowLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iLowLimit_Fine_Bin
{0x0F12, 0x0046}, // DSMix1_iHighLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Fine_Bin
{0x0F12, 0x0106}, // DSMix1_iRGBOffset
{0x0F12, 0x006F}, // DSMix1_iDemClamp
{0x0F12, 0x0C0F}, // "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"     bin: desparity low
{0x0F12, 0x0C0F}, // "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"  bin: desparity high
{0x0F12, 0x0303}, // "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"
{0x0F12, 0x0303}, // Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin
{0x0F12, 0x140A}, // "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"
{0x0F12, 0x140A}, // "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"
{0x0F12, 0x2828}, // "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"
{0x0F12, 0x0606}, // "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"
{0x0F12, 0x023F}, // "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"
{0x0F12, 0x0480}, // "Denoise1_iRadialLimitDenoise1_iLWBNoise"
{0x0F12, 0x000F}, // "Denoise1_iWideDenoise1_iWideWide"
{0x0F12, 0x030A}, // "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"
{0x0F12, 0x0003}, // "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"
{0x0F12, 0x0011}, // "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"
{0x0F12, 0x0A0F}, // "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"
{0x0F12, 0x050A}, // "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"  ]negati fine
{0x0F12, 0x0900}, // "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"     ]low fine
{0x0F12, 0x0000}, // "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"   ]high fine
{0x0F12, 0x980A}, // "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"      ]high low thres
{0x0F12, 0x0005}, // "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"
{0x0F12, 0x0000}, // "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"
{0x0F12, 0x0000}, // "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"
{0x0F12, 0x0000}, // "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"
{0x0F12, 0x0000}, // "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"
{0x0F12, 0x0A00}, // "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"
{0x0F12, 0x000A}, // "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"
{0x0F12, 0x0180}, // "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"
{0x0F12, 0x0180}, // "RGBGamma2_iLinearityRGBGamma2_bLinearity"
{0x0F12, 0x0100}, // "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"
{0x0F12, 0x6E14}, // "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"
{0x0F12, 0x0180}, // "RGB2YUV_iSaturationRGB2YUV_bGainOffset"
{0x0F12, 0x0035}, //35, //16  RGB2YUV_iYOffset

{0x0F12, 0x0000}, // BRIGHTNESS
{0x0F12, 0x0000}, // CONTRAST
{0x0F12, 0x0000}, // SATURATION
{0x0F12, 0x0000}, // SHARP_BLUR
{0x0F12, 0x0000}, // GLAMOUR
{0x0F12, 0x03FF}, // Disparity_iSatSat
{0x0F12, 0x0014}, // Denoise1_iYDenThreshLow
{0x0F12, 0x000E}, // Denoise1_iYDenThreshLow_Bin
{0x0F12, 0x0064}, // Denoise1_iYDenThreshHigh
{0x0F12, 0x00FF}, // Denoise1_iYDenThreshHigh_Bin
{0x0F12, 0x0002}, // Denoise1_iLowWWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWWideThresh
{0x0F12, 0x000A}, // Denoise1_iLowWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWideThresh
{0x0F12, 0x03FF}, // Denoise1_iSatSat
{0x0F12, 0x03FF}, // Demosaic4_iHystGrayLow
{0x0F12, 0x0000}, // Demosaic4_iHystGrayHigh
{0x0F12, 0x0114}, // UVDenoise_iYLowThresh
{0x0F12, 0x020A}, // UVDenoise_iYHighThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVLowThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVHighThresh
{0x0F12, 0x0028}, // DSMix1_iLowLimit_Wide
{0x0F12, 0x0000}, // DSMix1_iLowLimit_Wide_Bin
{0x0F12, 0x0014}, // DSMix1_iHighLimit_Wide
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Wide_Bin
{0x0F12, 0x0050}, // DSMix1_iLowLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iLowLimit_Fine_Bin
{0x0F12, 0x0046}, // DSMix1_iHighLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Fine_Bin
{0x0F12, 0x0106}, // DSMix1_iRGBOffset
{0x0F12, 0x006F}, // DSMix1_iDemClamp
{0x0F12, 0x050F}, // "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"    bin: desparity low
{0x0F12, 0x0A0F}, // "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"
{0x0F12, 0x0203}, // "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"
{0x0F12, 0x0203}, // Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin
{0x0F12, 0x140A}, // "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"
{0x0F12, 0x140A}, // "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"
{0x0F12, 0x2828}, // "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"
{0x0F12, 0x0606}, // "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"
{0x0F12, 0x020A}, // "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"
{0x0F12, 0x0480}, // "Denoise1_iRadialLimitDenoise1_iLWBNoise"
{0x0F12, 0x000F}, // "Denoise1_iWideDenoise1_iWideWide"
{0x0F12, 0x0305}, // "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"
{0x0F12, 0x2803}, // "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"
{0x0F12, 0x2811}, // "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"
{0x0F12, 0x0A0F}, // "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"
{0x0F12, 0x050A}, // "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"
{0x0F12, 0x101E}, // "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"
{0x0F12, 0x101E}, // "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"
{0x0F12, 0x980A}, // "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"
{0x0F12, 0x0005}, // "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"
{0x0F12, 0x0400}, // "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"
{0x0F12, 0x0400}, // "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"
{0x0F12, 0x0000}, // "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"
{0x0F12, 0x0000}, // "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"
{0x0F12, 0x0A00}, // "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"
{0x0F12, 0x100A}, // "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"
{0x0F12, 0x0180}, // "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"
{0x0F12, 0x0180}, // "RGBGamma2_iLinearityRGBGamma2_bLinearity"
{0x0F12, 0x0100}, // "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"
{0x0F12, 0x8030}, // "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"
{0x0F12, 0x0180}, // "RGB2YUV_iSaturationRGB2YUV_bGainOffset"
{0x0F12, 0x0008}, //08  RGB2YUV_iYOffset

{0x0F12, 0x0000}, // BRIGHTNESS
{0x0F12, 0x0000}, // CONTRAST
{0x0F12, 0x0000}, // SATURATION
{0x0F12, 0x0000}, // SHARP_BLUR
{0x0F12, 0x0000}, // GLAMOUR
{0x0F12, 0x03FF}, // Disparity_iSatSat
{0x0F12, 0x000C}, // Denoise1_iYDenThreshLow
{0x0F12, 0x0006}, // Denoise1_iYDenThreshLow_Bin
{0x0F12, 0x0060}, // Denoise1_iYDenThreshHigh
{0x0F12, 0x0050}, // Denoise1_iYDenThreshHigh_Bin
{0x0F12, 0x0002}, // Denoise1_iLowWWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWWideThresh
{0x0F12, 0x000A}, // Denoise1_iLowWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWideThresh
{0x0F12, 0x03FF}, // Denoise1_iSatSat
{0x0F12, 0x03FF}, // Demosaic4_iHystGrayLow
{0x0F12, 0x0000}, // Demosaic4_iHystGrayHigh
{0x0F12, 0x0014}, // UVDenoise_iYLowThresh
{0x0F12, 0x000A}, // UVDenoise_iYHighThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVLowThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVHighThresh
{0x0F12, 0x0028}, // DSMix1_iLowLimit_Wide
{0x0F12, 0x0032}, // DSMix1_iLowLimit_Wide_Bin
{0x0F12, 0x0014}, // DSMix1_iHighLimit_Wide
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Wide_Bin
{0x0F12, 0x0050}, // DSMix1_iLowLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iLowLimit_Fine_Bin
{0x0F12, 0x0010}, // DSMix1_iHighLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Fine_Bin
{0x0F12, 0x0106}, // DSMix1_iRGBOffset
{0x0F12, 0x006F}, // DSMix1_iDemClamp
{0x0F12, 0x0202}, // "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"
{0x0F12, 0x0502}, // "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"
{0x0F12, 0x0102}, // "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"
{0x0F12, 0x0102}, // Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin
{0x0F12, 0x140A}, // "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"
{0x0F12, 0x140A}, // "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"
{0x0F12, 0x2828}, // "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"
{0x0F12, 0x0606}, // "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"
{0x0F12, 0x0205}, // "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"
{0x0F12, 0x0480}, // "Denoise1_iRadialLimitDenoise1_iLWBNoise"
{0x0F12, 0x000F}, // "Denoise1_iWideDenoise1_iWideWide"
{0x0F12, 0x0005}, // "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"
{0x0F12, 0x2803}, // "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"
{0x0F12, 0x2811}, // "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"
{0x0F12, 0x0A0F}, // "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"
{0x0F12, 0x050A}, // "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"
{0x0F12, 0x2020}, // "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"
{0x0F12, 0x2020}, // "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"
{0x0F12, 0x980A}, // "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"
{0x0F12, 0x0007}, // "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"
{0x0F12, 0x0403}, // "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"
{0x0F12, 0x0402}, // "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"
{0x0F12, 0x0000}, // "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"
{0x0F12, 0x0203}, // "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"
{0x0F12, 0x0000}, // "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"
{0x0F12, 0x1006}, // "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"
{0x0F12, 0x0180}, // "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"
{0x0F12, 0x0180}, // "RGBGamma2_iLinearityRGBGamma2_bLinearity"
{0x0F12, 0x0100}, // "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"
{0x0F12, 0x803C}, // "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"
{0x0F12, 0x0180}, // "RGB2YUV_iSaturationRGB2YUV_bGainOffset"
{0x0F12, 0x0000}, //  RGB2YUV_iYOffset

{0x0F12, 0x0000}, // BRIGHTNESS
{0x0F12, 0x0000}, // CONTRAST
{0x0F12, 0x0000}, // SATURATION
{0x0F12, 0x0000}, // SHARP_BLUR
{0x0F12, 0x0000}, // GLAMOUR
{0x0F12, 0x03FF}, // Disparity_iSatSat
{0x0F12, 0x0006}, // Denoise1_iYDenThreshLow
{0x0F12, 0x0006}, // Denoise1_iYDenThreshLow_Bin
{0x0F12, 0x005A}, // Denoise1_iYDenThreshHigh
{0x0F12, 0x0050}, // Denoise1_iYDenThreshHigh_Bin
{0x0F12, 0x0002}, // Denoise1_iLowWWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWWideThresh
{0x0F12, 0x000A}, // Denoise1_iLowWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWideThresh
{0x0F12, 0x03FF}, // Denoise1_iSatSat
{0x0F12, 0x03FF}, // Demosaic4_iHystGrayLow
{0x0F12, 0x0000}, // Demosaic4_iHystGrayHigh
{0x0F12, 0x0014}, // UVDenoise_iYLowThresh
{0x0F12, 0x000A}, // UVDenoise_iYHighThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVLowThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVHighThresh
{0x0F12, 0x0028}, // DSMix1_iLowLimit_Wide
{0x0F12, 0x0032}, // DSMix1_iLowLimit_Wide_Bin
{0x0F12, 0x0014}, // DSMix1_iHighLimit_Wide
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Wide_Bin
{0x0F12, 0x0050}, // DSMix1_iLowLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iLowLimit_Fine_Bin
{0x0F12, 0x0010}, // DSMix1_iHighLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Fine_Bin
{0x0F12, 0x0106}, // DSMix1_iRGBOffset
{0x0F12, 0x006F}, // DSMix1_iDemClamp
{0x0F12, 0x0202}, // "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"
{0x0F12, 0x0502}, // "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"
{0x0F12, 0x0102}, // "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"
{0x0F12, 0x0102}, // Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin
{0x0F12, 0x140A}, // "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"
{0x0F12, 0x140A}, // "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"
{0x0F12, 0x2828}, // "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"
{0x0F12, 0x0606}, // "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"
{0x0F12, 0x0205}, // "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"
{0x0F12, 0x0480}, // "Denoise1_iRadialLimitDenoise1_iLWBNoise"
{0x0F12, 0x000F}, // "Denoise1_iWideDenoise1_iWideWide"
{0x0F12, 0x0005}, // "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"
{0x0F12, 0x2803}, // "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"
{0x0F12, 0x2811}, // "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"
{0x0F12, 0x0A0F}, // "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"
{0x0F12, 0x050A}, // "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"
{0x0F12, 0x2020}, // "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"
{0x0F12, 0x2020}, // "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"
{0x0F12, 0x980A}, // "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"
{0x0F12, 0x0007}, // "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"
{0x0F12, 0x0403}, // "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"
{0x0F12, 0x0402}, // "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"
{0x0F12, 0x0000}, // "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"
{0x0F12, 0x0203}, // "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"
{0x0F12, 0x0000}, // "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"
{0x0F12, 0x1006}, // "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"
{0x0F12, 0x0180}, // "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"
{0x0F12, 0x0180}, // "RGBGamma2_iLinearityRGBGamma2_bLinearity"
{0x0F12, 0x0100}, // "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"
{0x0F12, 0x803C}, // "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"
{0x0F12, 0x0180}, // "RGB2YUV_iSaturationRGB2YUV_bGainOffset"
{0x0F12, 0x0000}, //  RGB2YUV_iYOffset

{0x0F12, 0x0000}, // BRIGHTNESS
{0x0F12, 0x000A}, // CONTRAST
{0x0F12, 0x0000}, // SATURATION
{0x0F12, 0x0000}, // SHARP_BLUR
{0x0F12, 0x0000}, // GLAMOUR
{0x0F12, 0x03FF}, // Disparity_iSatSat
{0x0F12, 0x0006}, // Denoise1_iYDenThreshLow
{0x0F12, 0x0006}, // Denoise1_iYDenThreshLow_Bin
{0x0F12, 0x0050}, // Denoise1_iYDenThreshHigh
{0x0F12, 0x0050}, // Denoise1_iYDenThreshHigh_Bin
{0x0F12, 0x0002}, // Denoise1_iLowWWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWWideThresh
{0x0F12, 0x000A}, // Denoise1_iLowWideThresh
{0x0F12, 0x000A}, // Denoise1_iHighWideThresh
{0x0F12, 0x03FF}, // Denoise1_iSatSat
{0x0F12, 0x03FF}, // Demosaic4_iHystGrayLow
{0x0F12, 0x0000}, // Demosaic4_iHystGrayHigh
{0x0F12, 0x0000}, // UVDenoise_iYLowThresh
{0x0F12, 0x0000}, // UVDenoise_iYHighThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVLowThresh
{0x0F12, 0x03FF}, // UVDenoise_iUVHighThresh
{0x0F12, 0x0028}, // DSMix1_iLowLimit_Wide
{0x0F12, 0x0032}, // DSMix1_iLowLimit_Wide_Bin
{0x0F12, 0x0000}, // DSMix1_iHighLimit_Wide
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Wide_Bin
{0x0F12, 0x0030}, // DSMix1_iLowLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iLowLimit_Fine_Bin
{0x0F12, 0x0000}, // DSMix1_iHighLimit_Fine
{0x0F12, 0x0032}, // DSMix1_iHighLimit_Fine_Bin
{0x0F12, 0x0106}, // DSMix1_iRGBOffset
{0x0F12, 0x006F}, // DSMix1_iDemClamp
{0x0F12, 0x0202}, // "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"
{0x0F12, 0x0502}, // "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"
{0x0F12, 0x0102}, // "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin"
{0x0F12, 0x0102}, // Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin
{0x0F12, 0x140A}, // "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"
{0x0F12, 0x140A}, // "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"
{0x0F12, 0x2828}, // "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"
{0x0F12, 0x0606}, // "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"
{0x0F12, 0x0205}, // "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"
{0x0F12, 0x0880}, // "Denoise1_iRadialLimitDenoise1_iLWBNoise"
{0x0F12, 0x000F}, // "Denoise1_iWideDenoise1_iWideWide"
{0x0F12, 0x0005}, // "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"
{0x0F12, 0x2803}, // "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"
{0x0F12, 0x2811}, // "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"
{0x0F12, 0x0A0F}, // "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"
{0x0F12, 0x050A}, // "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"
{0x0F12, 0x2020}, // "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"
{0x0F12, 0x2020}, // "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"
{0x0F12, 0x980A}, // "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"
{0x0F12, 0x0007}, // "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"
{0x0F12, 0x0408}, // "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"
{0x0F12, 0x0406}, // "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"
{0x0F12, 0x0000}, // "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"
{0x0F12, 0x0608}, // "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"
{0x0F12, 0x0000}, // "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"
{0x0F12, 0x1006}, // "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"
{0x0F12, 0x0180}, // "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"
{0x0F12, 0x0180}, // "RGBGamma2_iLinearityRGBGamma2_bLinearity"
{0x0F12, 0x0100}, // "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"
{0x0F12, 0x8050}, // "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"
{0x0F12, 0x0140}, // "RGB2YUV_iSaturationRGB2YUV_bGainOffset"
{0x0F12, 0x0000}, //  RGB2YUV_iYOffset

{0x0F12, 0x00FF}, // Denoise1_iUVDenThreshLow
{0x0F12, 0x00FF}, // Denoise1_iUVDenThreshHigh
{0x0F12, 0x0800}, // Denoise1_sensor_width
{0x0F12, 0x0600}, // Denoise1_sensor_height
{0x0F12, 0x0000}, // Denoise1_start_x
{0x0F12, 0x0000}, // Denoise1_start_y
{0x0F12, 0x0000}, // "Denoise1_iYDenSmoothDenoise1_iWSharp  "
{0x0F12, 0x0300}, // "Denoise1_iWWSharp Denoise1_iRadialTune  "
{0x0F12, 0x0002}, // "Denoise1_iOutputBrightnessDenoise1_binning_x  "
{0x0F12, 0x0400}, // "Denoise1_binning_yDemosaic4_iFDeriv  "
{0x0F12, 0x0106}, // "Demosaic4_iFDerivNeiDemosaic4_iSDeriv  "
{0x0F12, 0x0005}, // "Demosaic4_iSDerivNeiDemosaic4_iEnhancerG  "
{0x0F12, 0x0000}, // "Demosaic4_iEnhancerRBDemosaic4_iEnhancerV  "
{0x0F12, 0x0703}, // "Demosaic4_iDecisionThreshDemosaic4_iDesatThresh"
{0x0F12, 0x0000}, //  Demosaic4_iBypassSelect
{0x0F12, 0xFFD6},
{0x0F12, 0x53C1},
{0x0F12, 0xE1FE},
{0x0F12, 0x0001},

// Update Changed Registers
{0x002A, 0x03FC},
{0x0F12, 0x0001},	//REG_TC_DBG_ReInitCmd

{0x0028, 0xD000},
{0x002A, 0xB0CC},
{0x0F12, 0x000B},	//Non contious mode
//////	END of Initial	////////////////////////////////

// MIPI
{0x0028, 0xD000},
{0x002A, 0xB0CC},
{0x0F12, 0x000B},

};

/*===========================================
*	CAMERA_RECORDING WITH 25fps  *
============================================*/
static const struct s5k5bafx_reg mode_sensor_recording_50Hz_init[] = {

/* recording 25fps Anti-Flicker 50Hz*/

{0xFCFC, 0xD000},

/* ARM Go */
{0x0028, 0xD000},
{0x002A, 0x1030},
{0x0F12, 0x0000},
{0x002A, 0x0014},
{0x0F12, 0x0001},
{S5K5BAFX_TABLE_WAIT_MS, 0x0064}, /* p100	Delay */


/* Trap and Patch  2008-11-18 10:15:41 */
{0x0028, 0x7000},
{0x002A, 0x1668},
{0x0F12, 0xB5FE},
{0x0F12, 0x0007},
{0x0F12, 0x683C},
{0x0F12, 0x687E},
{0x0F12, 0x1DA5},
{0x0F12, 0x88A0},
{0x0F12, 0x2800},
{0x0F12, 0xD00B},
{0x0F12, 0x88A8},
{0x0F12, 0x2800},
{0x0F12, 0xD008},
{0x0F12, 0x8820},
{0x0F12, 0x8829},
{0x0F12, 0x4288},
{0x0F12, 0xD301},
{0x0F12, 0x1A40},
{0x0F12, 0xE000},
{0x0F12, 0x1A08},
{0x0F12, 0x9001},
{0x0F12, 0xE001},
{0x0F12, 0x2019},
{0x0F12, 0x9001},
{0x0F12, 0x4916},
{0x0F12, 0x466B},
{0x0F12, 0x8A48},
{0x0F12, 0x8118},
{0x0F12, 0x8A88},
{0x0F12, 0x8158},
{0x0F12, 0x4814},
{0x0F12, 0x8940},
{0x0F12, 0x0040},
{0x0F12, 0x2103},
{0x0F12, 0xF000},
{0x0F12, 0xF826},
{0x0F12, 0x88A1},
{0x0F12, 0x4288},
{0x0F12, 0xD908},
{0x0F12, 0x8828},
{0x0F12, 0x8030},
{0x0F12, 0x8868},
{0x0F12, 0x8070},
{0x0F12, 0x88A8},
{0x0F12, 0x6038},
{0x0F12, 0xBCFE},
{0x0F12, 0xBC08},
{0x0F12, 0x4718},
{0x0F12, 0x88A9},
{0x0F12, 0x4288},
{0x0F12, 0xD906},
{0x0F12, 0x8820},
{0x0F12, 0x8030},
{0x0F12, 0x8860},
{0x0F12, 0x8070},
{0x0F12, 0x88A0},
{0x0F12, 0x6038},
{0x0F12, 0xE7F2},
{0x0F12, 0x9801},
{0x0F12, 0xA902},
{0x0F12, 0xF000},
{0x0F12, 0xF812},
{0x0F12, 0x0033},
{0x0F12, 0x0029},
{0x0F12, 0x9A02},
{0x0F12, 0x0020},
{0x0F12, 0xF000},
{0x0F12, 0xF814},
{0x0F12, 0x6038},
{0x0F12, 0xE7E6},
{0x0F12, 0x1A28},
{0x0F12, 0x7000},
{0x0F12, 0x0D64},
{0x0F12, 0x7000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xF004},
{0x0F12, 0xE51F},
{0x0F12, 0xA464},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x6009},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x622F},
{0x0F12, 0x0000},
{0x002A, 0x2080},
{0x0F12, 0xB510},
{0x0F12, 0xF000},
{0x0F12, 0xF8F4},
{0x0F12, 0xBC10},
{0x0F12, 0xBC08},
{0x0F12, 0x4718},
{0x0F12, 0xB5F0},
{0x0F12, 0xB08B},
{0x0F12, 0x0006},
{0x0F12, 0x2000},
{0x0F12, 0x9004},
{0x0F12, 0x6835},
{0x0F12, 0x6874},
{0x0F12, 0x68B0},
{0x0F12, 0x900A},
{0x0F12, 0x68F0},
{0x0F12, 0x9009},
{0x0F12, 0x4F7D},
{0x0F12, 0x8979},
{0x0F12, 0x084A},
{0x0F12, 0x88A8},
{0x0F12, 0x88A3},
{0x0F12, 0x4298},
{0x0F12, 0xD300},
{0x0F12, 0x0018},
{0x0F12, 0xF000},
{0x0F12, 0xF907},
{0x0F12, 0x9007},
{0x0F12, 0x0021},
{0x0F12, 0x0028},
{0x0F12, 0xAA04},
{0x0F12, 0xF000},
{0x0F12, 0xF909},
{0x0F12, 0x9006},
{0x0F12, 0x88A8},
{0x0F12, 0x2800},
{0x0F12, 0xD102},
{0x0F12, 0x27FF},
{0x0F12, 0x1C7F},
{0x0F12, 0xE047},
{0x0F12, 0x88A0},
{0x0F12, 0x2800},
{0x0F12, 0xD101},
{0x0F12, 0x2700},
{0x0F12, 0xE042},
{0x0F12, 0x8820},
{0x0F12, 0x466B},
{0x0F12, 0x8198},
{0x0F12, 0x8860},
{0x0F12, 0x81D8},
{0x0F12, 0x8828},
{0x0F12, 0x8118},
{0x0F12, 0x8868},
{0x0F12, 0x8158},
{0x0F12, 0xA802},
{0x0F12, 0xC803},
{0x0F12, 0xF000},
{0x0F12, 0xF8F8},
{0x0F12, 0x9008},
{0x0F12, 0x8ABA},
{0x0F12, 0x9808},
{0x0F12, 0x466B},
{0x0F12, 0x4342},
{0x0F12, 0x9202},
{0x0F12, 0x8820},
{0x0F12, 0x8198},
{0x0F12, 0x8860},
{0x0F12, 0x81D8},
{0x0F12, 0x980A},
{0x0F12, 0x9903},
{0x0F12, 0xF000},
{0x0F12, 0xF8EA},
{0x0F12, 0x9A02},
{0x0F12, 0x17D1},
{0x0F12, 0x0E09},
{0x0F12, 0x1889},
{0x0F12, 0x1209},
{0x0F12, 0x4288},
{0x0F12, 0xDD1F},
{0x0F12, 0x8820},
{0x0F12, 0x466B},
{0x0F12, 0x8198},
{0x0F12, 0x8860},
{0x0F12, 0x81D8},
{0x0F12, 0x980A},
{0x0F12, 0x9903},
{0x0F12, 0xF000},
{0x0F12, 0xF8DA},
{0x0F12, 0x9001},
{0x0F12, 0x8828},
{0x0F12, 0x466B},
{0x0F12, 0x8118},
{0x0F12, 0x8868},
{0x0F12, 0x8158},
{0x0F12, 0x980A},
{0x0F12, 0x9902},
{0x0F12, 0xF000},
{0x0F12, 0xF8D0},
{0x0F12, 0x8AB9},
{0x0F12, 0x9A08},
{0x0F12, 0x4351},
{0x0F12, 0x17CA},
{0x0F12, 0x0E12},
{0x0F12, 0x1851},
{0x0F12, 0x120A},
{0x0F12, 0x9901},
{0x0F12, 0xF000},
{0x0F12, 0xF8B6},
{0x0F12, 0x0407},
{0x0F12, 0x0C3F},
{0x0F12, 0xE000},
{0x0F12, 0x2700},
{0x0F12, 0x8820},
{0x0F12, 0x466B},
{0x0F12, 0xAA05},
{0x0F12, 0x8198},
{0x0F12, 0x8860},
{0x0F12, 0x81D8},
{0x0F12, 0x8828},
{0x0F12, 0x8118},
{0x0F12, 0x8868},
{0x0F12, 0x8158},
{0x0F12, 0xA802},
{0x0F12, 0xC803},
{0x0F12, 0x003B},
{0x0F12, 0xF000},
{0x0F12, 0xF8BB},
{0x0F12, 0x88A1},
{0x0F12, 0x88A8},
{0x0F12, 0x003A},
{0x0F12, 0xF000},
{0x0F12, 0xF8BE},
{0x0F12, 0x0004},
{0x0F12, 0xA804},
{0x0F12, 0xC803},
{0x0F12, 0x9A09},
{0x0F12, 0x9B07},
{0x0F12, 0xF000},
{0x0F12, 0xF8AF},
{0x0F12, 0xA806},
{0x0F12, 0xC805},
{0x0F12, 0x0021},
{0x0F12, 0xF000},
{0x0F12, 0xF8B2},
{0x0F12, 0x6030},
{0x0F12, 0xB00B},
{0x0F12, 0xBCF0},
{0x0F12, 0xBC08},
{0x0F12, 0x4718},
{0x0F12, 0xB5F1},
{0x0F12, 0x9900},
{0x0F12, 0x680C},
{0x0F12, 0x493A},
{0x0F12, 0x694B},
{0x0F12, 0x698A},
{0x0F12, 0x4694},
{0x0F12, 0x69CD},
{0x0F12, 0x6A0E},
{0x0F12, 0x4F38},
{0x0F12, 0x42BC},
{0x0F12, 0xD800},
{0x0F12, 0x0027},
{0x0F12, 0x4937},
{0x0F12, 0x6B89},
{0x0F12, 0x0409},
{0x0F12, 0x0C09},
{0x0F12, 0x4A35},
{0x0F12, 0x1E92},
{0x0F12, 0x6BD2},
{0x0F12, 0x0412},
{0x0F12, 0x0C12},
{0x0F12, 0x429F},
{0x0F12, 0xD801},
{0x0F12, 0x0020},
{0x0F12, 0xE031},
{0x0F12, 0x001F},
{0x0F12, 0x434F},
{0x0F12, 0x0A3F},
{0x0F12, 0x42A7},
{0x0F12, 0xD301},
{0x0F12, 0x0018},
{0x0F12, 0xE02A},
{0x0F12, 0x002B},
{0x0F12, 0x434B},
{0x0F12, 0x0A1B},
{0x0F12, 0x42A3},
{0x0F12, 0xD303},
{0x0F12, 0x0220},
{0x0F12, 0xF000},
{0x0F12, 0xF88C},
{0x0F12, 0xE021},
{0x0F12, 0x0029},
{0x0F12, 0x4351},
{0x0F12, 0x0A09},
{0x0F12, 0x42A1},
{0x0F12, 0xD301},
{0x0F12, 0x0028},
{0x0F12, 0xE01A},
{0x0F12, 0x0031},
{0x0F12, 0x4351},
{0x0F12, 0x0A09},
{0x0F12, 0x42A1},
{0x0F12, 0xD304},
{0x0F12, 0x0220},
{0x0F12, 0x0011},
{0x0F12, 0xF000},
{0x0F12, 0xF87B},
{0x0F12, 0xE010},
{0x0F12, 0x491E},
{0x0F12, 0x8C89},
{0x0F12, 0x000A},
{0x0F12, 0x4372},
{0x0F12, 0x0A12},
{0x0F12, 0x42A2},
{0x0F12, 0xD301},
{0x0F12, 0x0030},
{0x0F12, 0xE007},
{0x0F12, 0x4662},
{0x0F12, 0x434A},
{0x0F12, 0x0A12},
{0x0F12, 0x42A2},
{0x0F12, 0xD302},
{0x0F12, 0x0220},
{0x0F12, 0xF000},
{0x0F12, 0xF869},
{0x0F12, 0x4B16},
{0x0F12, 0x4D18},
{0x0F12, 0x8D99},
{0x0F12, 0x1FCA},
{0x0F12, 0x3AF9},
{0x0F12, 0xD00A},
{0x0F12, 0x2001},
{0x0F12, 0x0240},
{0x0F12, 0x8468},
{0x0F12, 0x0220},
{0x0F12, 0xF000},
{0x0F12, 0xF85D},
{0x0F12, 0x9900},
{0x0F12, 0x6008},
{0x0F12, 0xBCF8},
{0x0F12, 0xBC08},
{0x0F12, 0x4718},
{0x0F12, 0x8D19},
{0x0F12, 0x8469},
{0x0F12, 0x9900},
{0x0F12, 0x6008},
{0x0F12, 0xE7F7},
{0x0F12, 0xB570},
{0x0F12, 0x2200},
{0x0F12, 0x490E},
{0x0F12, 0x480E},
{0x0F12, 0x2401},
{0x0F12, 0xF000},
{0x0F12, 0xF852},
{0x0F12, 0x0022},
{0x0F12, 0x490D},
{0x0F12, 0x480D},
{0x0F12, 0x2502},
{0x0F12, 0xF000},
{0x0F12, 0xF84C},
{0x0F12, 0x490C},
{0x0F12, 0x480D},
{0x0F12, 0x002A},
{0x0F12, 0xF000},
{0x0F12, 0xF847},
{0x0F12, 0xBC70},
{0x0F12, 0xBC08},
{0x0F12, 0x4718},
{0x0F12, 0x0D64},
{0x0F12, 0x7000},
{0x0F12, 0x0470},
{0x0F12, 0x7000},
{0x0F12, 0xA120},
{0x0F12, 0x0007},
{0x0F12, 0x0402},
{0x0F12, 0x7000},
{0x0F12, 0x14A0},
{0x0F12, 0x7000},
{0x0F12, 0x208D},
{0x0F12, 0x7000},
{0x0F12, 0x622F},
{0x0F12, 0x0000},
{0x0F12, 0x1669},
{0x0F12, 0x7000},
{0x0F12, 0x6445},
{0x0F12, 0x0000},
{0x0F12, 0x21AB},
{0x0F12, 0x7000},
{0x0F12, 0x2AA9},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x5F49},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x5FC7},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x5457},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x5FA3},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x51F9},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xF004},
{0x0F12, 0xE51F},
{0x0F12, 0xA464},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0xA007},
{0x0F12, 0x0000},
{0x0F12, 0x6546},
{0x0F12, 0x2062},
{0x0F12, 0x3120},
{0x0F12, 0x3220},
{0x0F12, 0x3130},
{0x0F12, 0x0030},
{0x0F12, 0xE010},
{0x0F12, 0x0208},
{0x0F12, 0x0058},
{0x0F12, 0x0000},
/* End of Trap and Patch (Last : 70002342h) */
/* Total Size 896 (0x0380)                  */


{0x0028, 0xD000},
{0x002A, 0x1000},
{0x0F12, 0x0001},


{0x0028, 0x7000},
{0x002A, 0x1662},
{0x0F12, 0x03B0},
{0x0F12, 0x03B0},


{0x0028, 0x7000},
{0x002A, 0x1658},
{0x0F12, 0x9C40},
{0x0F12, 0x0000},
{0x0F12, 0x9C40},
{0x0F12, 0x0000},


{0x0028, 0x7000},
{0x002A, 0x0ADC},
{0x0F12, 0x0AF0}, /* setot_uOnlineClocksDiv40             */
{0x002A, 0x0AE2},
{0x0F12, 0x222E}, /* setot_usSetRomWaitStateThreshold4KHz */

{0x002A, 0x0B94},
{0x0F12, 0x0580}, /* awbb_GainsInit_0_:R       */
{0x0F12, 0x0400}, /* awbb_GainsInit_1_:G       */
{0x0F12, 0x05F0}, /* awbb_GainsInit_2_:B       */
{0x002A, 0x04A0},
{0x0F12, 0x8000}, /* lt_uLeiInit:AE start      */
{0x002A, 0x049A},
{0x0F12, 0x00FA}, /* lt_uMinExp   0.5ms·̠º¯°䞪/


/* Set CIS/APS/Analog */
{0x0028, 0xD000},
{0x002A, 0xF106},
{0x0F12, 0x0001},
{0x002A, 0xF206},
{0x0F12, 0x0001},


{0x002A, 0xC202},
{0x0F12, 0x0700},

{0x002A, 0xF260},
{0x0F12, 0x0001},

{0x002A, 0xF414},
{0x0F12, 0x0030},

{0x002A, 0xC204},
{0x0F12, 0x0100},
{0x002A, 0xF402},
{0x0F12, 0x0092},
{0x0F12, 0x007F},

{0x002A, 0xF700},
{0x0F12, 0x0040},
{0x002A, 0xF708},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0040},
{0x0F12, 0x0040},
{0x0F12, 0x0040},
{0x0F12, 0x0040},
{0x0F12, 0x0040},
{0x0F12, 0x0001},
{0x0F12, 0x0015},
{0x0F12, 0x0001},
{0x0F12, 0x0040},

{0x002A, 0xF48A},
{0x0F12, 0x0048},
{0x002A, 0xF10A},
{0x0F12, 0x008B},


{0x002A, 0xF900},
{0x0F12, 0x0067},


{0x002A, 0xF406},
{0x0F12, 0x0092},
{0x0F12, 0x007F},
{0x0F12, 0x0003},

{0x0F12, 0x0003},
{0x0F12, 0x0003},
{0x002A, 0xF442},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x002A, 0xF448},
{0x0F12, 0x0000},
{0x002A, 0xF456},
{0x0F12, 0x0001},
{0x0F12, 0x0010},
{0x0F12, 0x0000},

{0x002A, 0xF41A},
{0x0F12, 0x00FF},
{0x0F12, 0x0003},

{0x002A, 0xF420},
{0x0F12, 0x0030},
{0x002A, 0xF410},
{0x0F12, 0x0001},

{0x0F12, 0x0000},
{0x002A, 0xF416},
{0x0F12, 0x0001},
{0x002A, 0xF424},
{0x0F12, 0x0000},
{0x002A, 0xF422},
{0x0F12, 0x0000},

{0x002A, 0xF41E},
{0x0F12, 0x0000},
{0x002A, 0xF428},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x002A, 0xF430},
{0x0F12, 0x0000},
{0x0F12, 0x0000},

{0x0F12, 0x0008},
{0x0F12, 0x0005},
{0x0F12, 0x000F},
{0x0F12, 0x0001},
{0x0F12, 0x0040},
{0x0F12, 0x0040},
{0x0F12, 0x0010},

{0x002A, 0xF4D6},
{0x0F12, 0x0090},


{0x0F12, 0x0000},

{0x002A, 0xF47C},
{0x0F12, 0x000C},
{0x0F12, 0x0000},
{0x002A, 0xF49A},
{0x0F12, 0x0008},
{0x0F12, 0x0000},
{0x002A, 0xF4A2},
{0x0F12, 0x0008},
{0x0F12, 0x0000},
{0x002A, 0xF4B2},
{0x0F12, 0x0013},
{0x0F12, 0x0000},
{0x0F12, 0x0013},
{0x0F12, 0x0000},
{0x002A, 0xF4AA},
{0x0F12, 0x009B},
{0x0F12, 0x00FB},
{0x0F12, 0x009B},
{0x0F12, 0x00FB},
{0x002A, 0xF474},
{0x0F12, 0x0017},
{0x0F12, 0x005F},
{0x0F12, 0x0017},
{0x0F12, 0x008F},

{0x002A, 0xF48C},
{0x0F12, 0x0017},
{0x0F12, 0x009B},
{0x002A, 0xF4C8},
{0x0F12, 0x0163},
{0x0F12, 0x0193},
{0x002A, 0xF490},
{0x0F12, 0x0191},

{0x002A, 0xF418},
{0x0F12, 0x0083},

{0x002A, 0xF454},
{0x0F12, 0x0001},

{0x002A, 0xF702},
{0x0F12, 0x0081},
{0x002A, 0xF4D2},
{0x0F12, 0x0000},

/* For ESD Check */
{0x0028, 0x7000},
/* Set FPN Gain Input */
{0x002A, 0x1176},
{0x0F12, 0x0020},
{0x0F12, 0x0040},
{0x0F12, 0x0080},
{0x0F12, 0x0100},
{0x0F12, 0x0014},
{0x0F12, 0x000A},
{0x0F12, 0x0008},
{0x0F12, 0x0004},

/* CFPN Canceller */
{0x002A, 0x116C},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0002},
{0x002A, 0x0AE8},
{0x0F12, 0x0000},

/* sensor aig table setting */
{0x002A, 0x10EE},
{0x0F12, 0x0000},
{0x002A, 0x10F2},
{0x0F12, 0x0000},
{0x002A, 0x1152},
{0x0F12, 0x0030},
{0x0F12, 0x0028},
{0x0F12, 0x0030},
{0x002A, 0x1148},
{0x0F12, 0x00FB},
{0x002A, 0x1144},
{0x0F12, 0x00FB},
{0x002A, 0x1150},
{0x0F12, 0x01F4},


{0x002A, 0x1084},
{0x0F12, 0x0000},
{0x0F12, 0x0000},

/* Set AE Target */
{0x002A, 0x0F4C},
{0x0F12, 0x003A}, /* TVAR_ae_BrAve */

{0x002A, 0x0478},
{0x0F12, 0x0114},
{0x0F12, 0x00EB}, /* ae boundary */



/* Set Frame Rate */
{0x002A, 0x0484},
{0x0F12, 0x410A}, /* uMaxExp1 */
{0x0F12, 0x0000},
{0x002A, 0x048C},
{0x0F12, 0x8214}, /* uMaxExp2 */
{0x0F12, 0x0000},
{0x0F12, 0xA122}, /* uMaxExp3 */
{0x0F12, 0x0000},
{0x002A, 0x0488},
{0x0F12, 0xf424}, /* uMaxExp4 */
{0x0F12, 0x0000},
{0x002A, 0x043A},
{0x0F12, 0x01D0}, /* lt_uMaxAnGain0   */
{0x0F12, 0x01E0}, /* lt_uMaxAnGain0_1 */
{0x002A, 0x0494},
{0x0F12, 0x0300}, /* lt_uMaxAnGain1 */
{0x0F12, 0x0650}, /* lt_uMaxAnGain2 */
{0x0f12, 0x0100},
{0x002A, 0x0F52},
{0x0F12, 0x000F}, /* ae_StatMode */

{0x002A, 0x0E98}, /* bp_uMaxBrightnessFactor */
{0x0F12, 0x02A8},
{0x002A, 0x0E9E}, /* bp_uMinBrightnessFactor */
{0x0F12, 0x0298},

/* 1. Auto Flicker 60Hz Start */
{0x002A, 0x0B2E},
{0x0F12, 0x0001}, /* AFC_Default60Hz	 Auto Flicker 60Hz start 0: Auto Flicker 50Hz start */
{0x002A, 0x03F8},
{0x0F12, 0x005F}, /* REG_TC_DBG_AutoAlgEnBits   default : 007F */


{S5K5BAFX_TABLE_WAIT_MS, 0x000a}, /* p10	Wait10mSec */

/* Set PLL */
/* External CLOCK (MCLK) */
{0x002A, 0x01B8},
{0x0F12, 0x5DC0}, /* REG_TC_IPRM_InClockLSBs */
{0x0F12, 0x0000}, /* REG_TC_IPRM_InClockMSBs */

/* Parallel or MIPI Selection */
{0x002A, 0x01C6},
{0x0F12, 0x0001}, /* REG_TC_IPRM_UseNPviClocks         */
{0x0F12, 0x0001}, /* REG_TC_IPRM_UseNMipiClocks        */
{0x0F12, 0x0000}, /* REG_TC_IPRM_bBlockInternalPllCalc */

/* System Clock 0 (System : 24Mhz, PCLK : 48Mhz) */
{0x002A, 0x01CC},
{0x0F12, 0x1770}, /* REG_TC_IPRM_OpClk4KHz_0      */
{0x0F12, 0x2EE0}, /* REG_TC_IPRM_MinOutRate4KHz_0 */
{0x0F12, 0x2EE0}, /* REG_TC_IPRM_MaxOutRate4KHz_0 */

/* System Clock 1 (System : 48Mhz, PCLK : 48Mhz) */
{0x002A, 0x01D2},
{0x0F12, 0x2EE0}, /* REG_TC_IPRM_OpClk4KHz_1      */
{0x0F12, 0x2EE0}, /* REG_TC_IPRM_MinOutRate4KHz_1 */
{0x0F12, 0x2EE0}, /* REG_TC_IPRM_MaxOutRate4KHz_1 */



{0x002A, 0x01DE},
{0x0F12, 0x0001}, /* REG_TC_IPRM_UseRegsAPI        */
{0x0F12, 0x0001}, /* REG_TC_IPRM_InitParamsUpdated */
{S5K5BAFX_TABLE_WAIT_MS, 0x0064}, /* p100                          */



/* Crop */
{0x002A, 0x01FA},
{0x0F12, 0x0640}, /* REG_TC_GP_PrevReqInputWidth  */
{0x0F12, 0x04B0}, /* REG_TC_GP_PrevReqInputHeight */
{0x0F12, 0x0000}, /* REG_TC_GP_PrevInputWidthOfs  */
{0x0F12, 0x0000}, /* REG_TC_GP_PrevInputHeightOfs */


/* Set Preview Config */
/* Preview Config 0 (VGA fixed 30fps) */
{0x002A, 0x0242},
{0x0F12, 0x0280}, /* REG_0TC_PCFG_usWidth                */
{0x0F12, 0x01E0}, /* REG_0TC_PCFG_usHeight               */
{0x0F12, 0x0005}, /* REG_0TC_PCFG_Format                 */
{0x0F12, 0x2EE0}, /* REG_0TC_PCFG_usMaxOut4KHzRate       */
{0x0F12, 0x2EE0}, /* REG_0TC_PCFG_usMinOut4KHzRate       */
{0x0F12, 0x0052}, /* REG_0TC_PCFG_PVIMask                */
{0x0F12, 0x0001}, /* REG_0TC_PCFG_uClockInd              */
{0x0F12, 0x0002}, /* REG_0TC_PCFG_usFrTimeType           */
{0x0F12, 0x0001}, /* REG_0TC_PCFG_FrRateQualityType      */
{0x0F12, 0x018c}, /* REG_0TC_PCFG_usMaxFrTimeMsecMult10  */
{0x0F12, 0x018c}, /* REG_0TC_PCFG_usMinFrTimeMsecMult10  */
{0x0F12, 0x0000}, /* REG_0TC_PCFG_sSaturation            */
{0x0F12, 0x0000}, /* REG_0TC_PCFG_sSharpBlur             */
{0x0F12, 0x0000}, /* REG_0TC_PCFG_sGlamour               */
{0x0F12, 0x0000}, /* REG_0TC_PCFG_sColorTemp             */
{0x0F12, 0x0000}, /* REG_0TC_PCFG_uDeviceGammaIndex      */
{0x0F12, 0x0000}, /* REG_0TC_PCFG_uPrevMirror            */
{0x0F12, 0x0000}, /* REG_0TC_PCFG_uCaptureMirror         */
{0x0F12, 0x0000}, /* REG_0TC_PCFG_uRotation              */

/* Preview Config 1 (640x480, Not Fixed 15 ~ 30fps) */
{0x002A, 0x0268},
{0x0F12, 0x0280}, /* REG_1TC_PCFG_usWidth                */
{0x0F12, 0x01E0}, /* REG_1TC_PCFG_usHeight               */
{0x0F12, 0x0005}, /* REG_1TC_PCFG_Format                 */
{0x0F12, 0x2EE0}, /* REG_1TC_PCFG_usMaxOut4KHzRate       */
{0x0F12, 0x2EE0}, /* REG_1TC_PCFG_usMinOut4KHzRate       */
{0x0F12, 0x0052}, /* REG_1TC_PCFG_PVIMask                */
{0x0F12, 0x0001}, /* REG_1TC_PCFG_uClockInd              */
{0x0F12, 0x0000}, /* REG_1TC_PCFG_usFrTimeType           */
{0x0F12, 0x0000}, /* REG_1TC_PCFG_FrRateQualityType      */
{0x0F12, 0x029A}, /* REG_1TC_PCFG_usMaxFrTimeMsecMult10  */
{0x0F12, 0x014D}, /* REG_1TC_PCFG_usMinFrTimeMsecMult10  */
{0x0F12, 0x0000}, /* REG_1TC_PCFG_sSaturation            */
{0x0F12, 0x0000}, /* REG_1TC_PCFG_sSharpBlur             */
{0x0F12, 0x0000}, /* REG_1TC_PCFG_sGlamour               */
{0x0F12, 0x0000}, /* REG_1TC_PCFG_sColorTemp             */
{0x0F12, 0x0000}, /* REG_1TC_PCFG_uDeviceGammaIndex      */
{0x0F12, 0x0001}, /* REG_1TC_PCFG_uPrevMirror            */
{0x0F12, 0x0001}, /* REG_1TC_PCFG_uCaptureMirror         */
{0x0F12, 0x0000}, /* REG_1TC_PCFG_uRotation              */


/* Set MIPI */
{0x002A, 0x03AC},
{0x0F12, 0x0000}, /* REG_TC_FLS_Mode        */
{0x002A, 0x03F2},
{0x0F12, 0x0001}, /* REG_TC_OIF_EnMipiLanes */
{0x0F12, 0x00C3}, /* REG_TC_OIF_EnPackets   */
{0x0F12, 0x0001}, /* REG_TC_OIF_CfgChanged  */

/* Apply preview config */
{0x002A, 0x021C},
{0x0F12, 0x0000}, /* REG_TC_GP_ActivePrevConfig */
{0x002A, 0x0220},
{0x0F12, 0x0001}, /* REG_TC_GP_PrevOpenAfterChange */
{0x002A, 0x01F8},
{0x0F12, 0x0001}, /* REG_TC_GP_NewConfigSync */
{0x002A, 0x021E},
{0x0F12, 0x0001}, /* REG_TC_GP_PrevConfigChanged */
{0x002A, 0x01F0},
{0x0F12, 0x0001}, /* REG_TC_GP_EnablePreview */
{0x0F12, 0x0001}, /* REG_TC_GP_EnablePreviewChanged */



/* Set Capture Config */
/* Capture Config 0 (1600x1200 fixed 8fps) */
{0x002A, 0x0302},
{0x0F12, 0x0000}, /* REG_0TC_CCFG_uCaptureMode          */
{0x0F12, 0x0640}, /* REG_0TC_CCFG_usWidth               */
{0x0F12, 0x04B0}, /* REG_0TC_CCFG_usHeight              */
{0x0F12, 0x0005}, /* REG_0TC_CCFG_Format                */
{0x0F12, 0x2EE0}, /* REG_0TC_CCFG_usMaxOut4KHzRate      */
{0x0F12, 0x2EE0}, /* REG_0TC_CCFG_usMinOut4KHzRate      */
{0x0F12, 0x0052}, /* REG_0TC_CCFG_PVIMask               */
{0x0F12, 0x0001}, /* REG_0TC_CCFG_uClockInd             */
{0x0F12, 0x0002}, /* REG_0TC_CCFG_usFrTimeType          */
{0x0F12, 0x0002}, /* REG_0TC_CCFG_FrRateQualityType     */
{0x0F12, 0x04E2}, /* REG_0TC_CCFG_usMaxFrTimeMsecMult10 */
{0x0F12, 0x04E2}, /* REG_0TC_CCFG_usMinFrTimeMsecMult10 */
{0x0F12, 0x0000}, /* REG_0TC_CCFG_sSaturation           */
{0x0F12, 0x0000}, /* REG_0TC_CCFG_sSharpBlur            */
{0x0F12, 0x0000}, /* REG_0TC_CCFG_sGlamour              */
{0x0F12, 0x0000}, /* REG_0TC_CCFG_sColorTemp            */
{0x0F12, 0x0000}, /* REG_0TC_CCFG_uDeviceGammaIndex     */



/* Periodic mismatch */
{0x002A, 0x0780},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},

{0x002A, 0x0798},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},

{0x002A, 0x07C0},
{0x0F12, 0x0004},
{0x0F12, 0x0004},

{0x002A, 0x0B94},
{0x0F12, 0x0580},
{0x0F12, 0x0400},
{0x0F12, 0x05F0},
{0x002A, 0x04A0},
{0x0F12, 0x8000},


/* Set AE Weights */
{0x002A, 0x0F5A},
{0x0F12, 0x0000}, /* ae_WeightTbl_16_0_ */
{0x0F12, 0x0000}, /* ae_WeightTbl_16_1_ */
{0x0F12, 0x0000}, /* ae_WeightTbl_16_2_ */
{0x0F12, 0x0000}, /* ae_WeightTbl_16_3_ */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_4_ */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_5_ */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_6_ */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_7_ */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_8_ */
{0x0F12, 0x0302}, /* ae_WeightTbl_16_9_ */
{0x0F12, 0x0203}, /* ae_WeightTbl_16_10 */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_11 */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_12 */
{0x0F12, 0x0403}, /* ae_WeightTbl_16_13 */
{0x0F12, 0x0304}, /* ae_WeightTbl_16_14 */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_15 */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_16 */
{0x0F12, 0x0403}, /* ae_WeightTbl_16_17 */
{0x0F12, 0x0304}, /* ae_WeightTbl_16_18 */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_19 */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_20 */
{0x0F12, 0x0302}, /* ae_WeightTbl_16_21 */
{0x0F12, 0x0203}, /* ae_WeightTbl_16_22 */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_23 */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_24 */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_25 */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_26 */
{0x0F12, 0x0101}, /* ae_WeightTbl_16_27 */
{0x0F12, 0x0000}, /* ae_WeightTbl_16_28 */
{0x0F12, 0x0000}, /* ae_WeightTbl_16_29 */
{0x0F12, 0x0000}, /* ae_WeightTbl_16_30 */
{0x0F12, 0x0000}, /* ae_WeightTbl_16_31 */


/* Set GAS & CCM White Point */
/* param_start	TVAR_ash_AwbAshCord */
{0x002A, 0x0704},
{0x0F12, 0x00B3},
{0x0F12, 0x00E5},
{0x0F12, 0x0120},
{0x0F12, 0x0136},
{0x0F12, 0x0180},
{0x0F12, 0x01B0},
{0x0F12, 0x0200},

/* param_start	wbt_AwbCcmCord */
{0x002A, 0x06F2},
{0x0F12, 0x00B3},
{0x0F12, 0x00E5},
{0x0F12, 0x0120},
{0x0F12, 0x0136},
{0x0F12, 0x0180},
{0x0F12, 0x0190},

/* Target Brightness Control */
{0x002A, 0x103E},
{0x0F12, 0x0000},
{0x0F12, 0x0009},
{0x0F12, 0x0018},
{0x0F12, 0x0032},
{0x0F12, 0x004A},
{0x0F12, 0x0051},
{0x0F12, 0x0056},
{0x0F12, 0x010C},
{0x0F12, 0x010C},
{0x0F12, 0x0109},
{0x0F12, 0x0105},
{0x0F12, 0x0102},
{0x0F12, 0x00FB},
{0x0F12, 0x00F8},

/* TVAR_ash_GASAlpha(Indoor) */
{0x002A, 0x0712},
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[0]  */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[1]  */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[2]  */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[3]  */

{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[4]  */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[5]  */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[6]  */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[7]  */

{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[8]  */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[9]  */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[10] */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[11] */

{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[12] */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[13] */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[14] */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[15] */

{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[16] */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[17] */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[18] */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[19] */

{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[20] */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[21] */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[22] */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[23] */

{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[24] */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[25] */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[26] */
{0x0F12, 0x0100}, /* TVAR_ash_GASAlpha[27] */

/* TVAR_ash_GASAlpha(Outdoor) */
{0x0F12, 0x0108}, /* 100 TVAR_ash_GASOutdoorAlpha_0_*/
{0x0F12, 0x0100}, /* TVAR_ash_GASOutdoorAlpha_1_*/
{0x0F12, 0x0100}, /* TVAR_ash_GASOutdoorAlpha_2_*/
{0x0F12, 0x0100}, /* TVAR_ash_GASOutdoorAlpha_3_*/

/* GAS LUT Start Address */
{0x002A, 0x0754},
{0x0F12, 0x2388},
{0x0F12, 0x7000},

/* param_start	TVAR_ash_pGAS */
{0x002A, 0x2388},
{0x0F12, 0x0160}, /* TVAR_ash_pGAS[0]   */
{0x0F12, 0x0134}, /* TVAR_ash_pGAS[1]   */
{0x0F12, 0x00FF}, /* TVAR_ash_pGAS[2]   */
{0x0F12, 0x00D1}, /* TVAR_ash_pGAS[3]   */
{0x0F12, 0x00B1}, /* TVAR_ash_pGAS[4]   */
{0x0F12, 0x009D}, /* TVAR_ash_pGAS[5]   */
{0x0F12, 0x0096}, /* TVAR_ash_pGAS[6]   */
{0x0F12, 0x009E}, /* TVAR_ash_pGAS[7]   */
{0x0F12, 0x00B3}, /* TVAR_ash_pGAS[8]   */
{0x0F12, 0x00D3}, /* TVAR_ash_pGAS[9]   */
{0x0F12, 0x00FF}, /* TVAR_ash_pGAS[10]  */
{0x0F12, 0x0131}, /* TVAR_ash_pGAS[11]  */
{0x0F12, 0x0159}, /* TVAR_ash_pGAS[12]  */
{0x0F12, 0x013C}, /* TVAR_ash_pGAS[13]  */
{0x0F12, 0x0107}, /* TVAR_ash_pGAS[14]  */
{0x0F12, 0x00CD}, /* TVAR_ash_pGAS[15]  */
{0x0F12, 0x00A1}, /* TVAR_ash_pGAS[16]  */
{0x0F12, 0x0080}, /* TVAR_ash_pGAS[17]  */
{0x0F12, 0x006B}, /* TVAR_ash_pGAS[18]  */
{0x0F12, 0x0064}, /* TVAR_ash_pGAS[19]  */
{0x0F12, 0x006C}, /* TVAR_ash_pGAS[20]  */
{0x0F12, 0x0080}, /* TVAR_ash_pGAS[21]  */
{0x0F12, 0x00A1}, /* TVAR_ash_pGAS[22]  */
{0x0F12, 0x00CD}, /* TVAR_ash_pGAS[23]  */
{0x0F12, 0x0106}, /* TVAR_ash_pGAS[24]  */
{0x0F12, 0x0139}, /* TVAR_ash_pGAS[25]  */
{0x0F12, 0x0116}, /* TVAR_ash_pGAS[26]  */
{0x0F12, 0x00DC}, /* TVAR_ash_pGAS[27]  */
{0x0F12, 0x00A2}, /* TVAR_ash_pGAS[28]  */
{0x0F12, 0x0073}, /* TVAR_ash_pGAS[29]  */
{0x0F12, 0x0051}, /* TVAR_ash_pGAS[30]  */
{0x0F12, 0x003B}, /* TVAR_ash_pGAS[31]  */
{0x0F12, 0x0033}, /* TVAR_ash_pGAS[32]  */
{0x0F12, 0x003B}, /* TVAR_ash_pGAS[33]  */
{0x0F12, 0x0050}, /* TVAR_ash_pGAS[34]  */
{0x0F12, 0x0073}, /* TVAR_ash_pGAS[35]  */
{0x0F12, 0x00A2}, /* TVAR_ash_pGAS[36]  */
{0x0F12, 0x00DD}, /* TVAR_ash_pGAS[37]  */
{0x0F12, 0x0115}, /* TVAR_ash_pGAS[38]  */
{0x0F12, 0x00FA}, /* TVAR_ash_pGAS[39]  */
{0x0F12, 0x00BF}, /* TVAR_ash_pGAS[40]  */
{0x0F12, 0x0085}, /* TVAR_ash_pGAS[41]  */
{0x0F12, 0x0055}, /* TVAR_ash_pGAS[42]  */
{0x0F12, 0x0031}, /* TVAR_ash_pGAS[43]  */
{0x0F12, 0x001B}, /* TVAR_ash_pGAS[44]  */
{0x0F12, 0x0014}, /* TVAR_ash_pGAS[45]  */
{0x0F12, 0x001A}, /* TVAR_ash_pGAS[46]  */
{0x0F12, 0x0031}, /* TVAR_ash_pGAS[47]  */
{0x0F12, 0x0055}, /* TVAR_ash_pGAS[48]  */
{0x0F12, 0x0085}, /* TVAR_ash_pGAS[49]  */
{0x0F12, 0x00C0}, /* TVAR_ash_pGAS[50]  */
{0x0F12, 0x00FB}, /* TVAR_ash_pGAS[51]  */
{0x0F12, 0x00EA}, /* TVAR_ash_pGAS[52]  */
{0x0F12, 0x00AF}, /* TVAR_ash_pGAS[53]  */
{0x0F12, 0x0074}, /* TVAR_ash_pGAS[54]  */
{0x0F12, 0x0045}, /* TVAR_ash_pGAS[55]  */
{0x0F12, 0x0020}, /* TVAR_ash_pGAS[56]  */
{0x0F12, 0x000B}, /* TVAR_ash_pGAS[57]  */
{0x0F12, 0x0003}, /* TVAR_ash_pGAS[58]  */
{0x0F12, 0x000A}, /* TVAR_ash_pGAS[59]  */
{0x0F12, 0x0020}, /* TVAR_ash_pGAS[60]  */
{0x0F12, 0x0046}, /* TVAR_ash_pGAS[61]  */
{0x0F12, 0x0076}, /* TVAR_ash_pGAS[62]  */
{0x0F12, 0x00B1}, /* TVAR_ash_pGAS[63]  */
{0x0F12, 0x00ED}, /* TVAR_ash_pGAS[64]  */
{0x0F12, 0x00E6}, /* TVAR_ash_pGAS[65]  */
{0x0F12, 0x00AA}, /* TVAR_ash_pGAS[66]  */
{0x0F12, 0x0071}, /* TVAR_ash_pGAS[67]  */
{0x0F12, 0x0041}, /* TVAR_ash_pGAS[68]  */
{0x0F12, 0x001D}, /* TVAR_ash_pGAS[69]  */
{0x0F12, 0x0008}, /* TVAR_ash_pGAS[70]  */
{0x0F12, 0x0000}, /* TVAR_ash_pGAS[71]  */
{0x0F12, 0x0007}, /* TVAR_ash_pGAS[72]  */
{0x0F12, 0x001E}, /* TVAR_ash_pGAS[73]  */
{0x0F12, 0x0044}, /* TVAR_ash_pGAS[74]  */
{0x0F12, 0x0074}, /* TVAR_ash_pGAS[75]  */
{0x0F12, 0x00B0}, /* TVAR_ash_pGAS[76]  */
{0x0F12, 0x00EC}, /* TVAR_ash_pGAS[77]  */
{0x0F12, 0x00EF}, /* TVAR_ash_pGAS[78]  */
{0x0F12, 0x00B3}, /* TVAR_ash_pGAS[79]  */
{0x0F12, 0x007A}, /* TVAR_ash_pGAS[80]  */
{0x0F12, 0x004A}, /* TVAR_ash_pGAS[81]  */
{0x0F12, 0x0026}, /* TVAR_ash_pGAS[82]  */
{0x0F12, 0x0011}, /* TVAR_ash_pGAS[83]  */
{0x0F12, 0x000A}, /* TVAR_ash_pGAS[84]  */
{0x0F12, 0x0011}, /* TVAR_ash_pGAS[85]  */
{0x0F12, 0x0029}, /* TVAR_ash_pGAS[86]  */
{0x0F12, 0x004F}, /* TVAR_ash_pGAS[87]  */
{0x0F12, 0x0080}, /* TVAR_ash_pGAS[88]  */
{0x0F12, 0x00BC}, /* TVAR_ash_pGAS[89]  */
{0x0F12, 0x00F8}, /* TVAR_ash_pGAS[90]  */
{0x0F12, 0x0105}, /* TVAR_ash_pGAS[91]  */
{0x0F12, 0x00C9}, /* TVAR_ash_pGAS[92]  */
{0x0F12, 0x008F}, /* TVAR_ash_pGAS[93]  */
{0x0F12, 0x0060}, /* TVAR_ash_pGAS[94]  */
{0x0F12, 0x003C}, /* TVAR_ash_pGAS[95]  */
{0x0F12, 0x0026}, /* TVAR_ash_pGAS[96]  */
{0x0F12, 0x001F}, /* TVAR_ash_pGAS[97]  */
{0x0F12, 0x0028}, /* TVAR_ash_pGAS[98]  */
{0x0F12, 0x0040}, /* TVAR_ash_pGAS[99]  */
{0x0F12, 0x0066}, /* TVAR_ash_pGAS[100] */
{0x0F12, 0x0097}, /* TVAR_ash_pGAS[101] */
{0x0F12, 0x00D4}, /* TVAR_ash_pGAS[102] */
{0x0F12, 0x0110}, /* TVAR_ash_pGAS[103] */
{0x0F12, 0x0124}, /* TVAR_ash_pGAS[104] */
{0x0F12, 0x00EB}, /* TVAR_ash_pGAS[105] */
{0x0F12, 0x00B1}, /* TVAR_ash_pGAS[106] */
{0x0F12, 0x0082}, /* TVAR_ash_pGAS[107] */
{0x0F12, 0x005F}, /* TVAR_ash_pGAS[108] */
{0x0F12, 0x004A}, /* TVAR_ash_pGAS[109] */
{0x0F12, 0x0043}, /* TVAR_ash_pGAS[110] */
{0x0F12, 0x004C}, /* TVAR_ash_pGAS[111] */
{0x0F12, 0x0064}, /* TVAR_ash_pGAS[112] */
{0x0F12, 0x0089}, /* TVAR_ash_pGAS[113] */
{0x0F12, 0x00BA}, /* TVAR_ash_pGAS[114] */
{0x0F12, 0x00F8}, /* TVAR_ash_pGAS[115] */
{0x0F12, 0x012F}, /* TVAR_ash_pGAS[116] */
{0x0F12, 0x0147}, /* TVAR_ash_pGAS[117] */
{0x0F12, 0x0116}, /* TVAR_ash_pGAS[118] */
{0x0F12, 0x00DE}, /* TVAR_ash_pGAS[119] */
{0x0F12, 0x00AF}, /* TVAR_ash_pGAS[120] */
{0x0F12, 0x008E}, /* TVAR_ash_pGAS[121] */
{0x0F12, 0x007A}, /* TVAR_ash_pGAS[122] */
{0x0F12, 0x0072}, /* TVAR_ash_pGAS[123] */
{0x0F12, 0x007A}, /* TVAR_ash_pGAS[124] */
{0x0F12, 0x0091}, /* TVAR_ash_pGAS[125] */
{0x0F12, 0x00B6}, /* TVAR_ash_pGAS[126] */
{0x0F12, 0x00E8}, /* TVAR_ash_pGAS[127] */
{0x0F12, 0x0121}, /* TVAR_ash_pGAS[128] */
{0x0F12, 0x0150}, /* TVAR_ash_pGAS[129] */
{0x0F12, 0x0170}, /* TVAR_ash_pGAS[130] */
{0x0F12, 0x013F}, /* TVAR_ash_pGAS[131] */
{0x0F12, 0x0110}, /* TVAR_ash_pGAS[132] */
{0x0F12, 0x00E2}, /* TVAR_ash_pGAS[133] */
{0x0F12, 0x00C0}, /* TVAR_ash_pGAS[134] */
{0x0F12, 0x00AB}, /* TVAR_ash_pGAS[135] */
{0x0F12, 0x00A4}, /* TVAR_ash_pGAS[136] */
{0x0F12, 0x00AC}, /* TVAR_ash_pGAS[137] */
{0x0F12, 0x00C3}, /* TVAR_ash_pGAS[138] */
{0x0F12, 0x00E6}, /* TVAR_ash_pGAS[139] */
{0x0F12, 0x0117}, /* TVAR_ash_pGAS[140] */
{0x0F12, 0x0145}, /* TVAR_ash_pGAS[141] */
{0x0F12, 0x0172}, /* TVAR_ash_pGAS[142] */
{0x0F12, 0x0127}, /* TVAR_ash_pGAS[143] */
{0x0F12, 0x0100}, /* TVAR_ash_pGAS[144] */
{0x0F12, 0x00CF}, /* TVAR_ash_pGAS[145] */
{0x0F12, 0x00A7}, /* TVAR_ash_pGAS[146] */
{0x0F12, 0x008D}, /* TVAR_ash_pGAS[147] */
{0x0F12, 0x007D}, /* TVAR_ash_pGAS[148] */
{0x0F12, 0x0077}, /* TVAR_ash_pGAS[149] */
{0x0F12, 0x007A}, /* TVAR_ash_pGAS[150] */
{0x0F12, 0x0087}, /* TVAR_ash_pGAS[151] */
{0x0F12, 0x009E}, /* TVAR_ash_pGAS[152] */
{0x0F12, 0x00C0}, /* TVAR_ash_pGAS[153] */
{0x0F12, 0x00EC}, /* TVAR_ash_pGAS[154] */
{0x0F12, 0x010F}, /* TVAR_ash_pGAS[155] */
{0x0F12, 0x0108}, /* TVAR_ash_pGAS[156] */
{0x0F12, 0x00D8}, /* TVAR_ash_pGAS[157] */
{0x0F12, 0x00A5}, /* TVAR_ash_pGAS[158] */
{0x0F12, 0x0080}, /* TVAR_ash_pGAS[159] */
{0x0F12, 0x0066}, /* TVAR_ash_pGAS[160] */
{0x0F12, 0x0056}, /* TVAR_ash_pGAS[161] */
{0x0F12, 0x004F}, /* TVAR_ash_pGAS[162] */
{0x0F12, 0x0053}, /* TVAR_ash_pGAS[163] */
{0x0F12, 0x0061}, /* TVAR_ash_pGAS[164] */
{0x0F12, 0x0077}, /* TVAR_ash_pGAS[165] */
{0x0F12, 0x0098}, /* TVAR_ash_pGAS[166] */
{0x0F12, 0x00C6}, /* TVAR_ash_pGAS[167] */
{0x0F12, 0x00F3}, /* TVAR_ash_pGAS[168] */
{0x0F12, 0x00E7}, /* TVAR_ash_pGAS[169] */
{0x0F12, 0x00B4}, /* TVAR_ash_pGAS[170] */
{0x0F12, 0x0081}, /* TVAR_ash_pGAS[171] */
{0x0F12, 0x005C}, /* TVAR_ash_pGAS[172] */
{0x0F12, 0x0041}, /* TVAR_ash_pGAS[173] */
{0x0F12, 0x0030}, /* TVAR_ash_pGAS[174] */
{0x0F12, 0x0029}, /* TVAR_ash_pGAS[175] */
{0x0F12, 0x002E}, /* TVAR_ash_pGAS[176] */
{0x0F12, 0x003D}, /* TVAR_ash_pGAS[177] */
{0x0F12, 0x0055}, /* TVAR_ash_pGAS[178] */
{0x0F12, 0x0076}, /* TVAR_ash_pGAS[179] */
{0x0F12, 0x00A5}, /* TVAR_ash_pGAS[180] */
{0x0F12, 0x00D4}, /* TVAR_ash_pGAS[181] */
{0x0F12, 0x00CF}, /* TVAR_ash_pGAS[182] */
{0x0F12, 0x009B}, /* TVAR_ash_pGAS[183] */
{0x0F12, 0x006A}, /* TVAR_ash_pGAS[184] */
{0x0F12, 0x0043}, /* TVAR_ash_pGAS[185] */
{0x0F12, 0x0027}, /* TVAR_ash_pGAS[186] */
{0x0F12, 0x0016}, /* TVAR_ash_pGAS[187] */
{0x0F12, 0x000F}, /* TVAR_ash_pGAS[188] */
{0x0F12, 0x0015}, /* TVAR_ash_pGAS[189] */
{0x0F12, 0x0025}, /* TVAR_ash_pGAS[190] */
{0x0F12, 0x003E}, /* TVAR_ash_pGAS[191] */
{0x0F12, 0x0061}, /* TVAR_ash_pGAS[192] */
{0x0F12, 0x008E}, /* TVAR_ash_pGAS[193] */
{0x0F12, 0x00BF}, /* TVAR_ash_pGAS[194] */
{0x0F12, 0x00C2}, /* TVAR_ash_pGAS[195] */
{0x0F12, 0x008E}, /* TVAR_ash_pGAS[196] */
{0x0F12, 0x005D}, /* TVAR_ash_pGAS[197] */
{0x0F12, 0x0037}, /* TVAR_ash_pGAS[198] */
{0x0F12, 0x001A}, /* TVAR_ash_pGAS[199] */
{0x0F12, 0x0009}, /* TVAR_ash_pGAS[200] */
{0x0F12, 0x0002}, /* TVAR_ash_pGAS[201] */
{0x0F12, 0x0007}, /* TVAR_ash_pGAS[202] */
{0x0F12, 0x0018}, /* TVAR_ash_pGAS[203] */
{0x0F12, 0x0033}, /* TVAR_ash_pGAS[204] */
{0x0F12, 0x0057}, /* TVAR_ash_pGAS[205] */
{0x0F12, 0x0083}, /* TVAR_ash_pGAS[206] */
{0x0F12, 0x00B3}, /* TVAR_ash_pGAS[207] */
{0x0F12, 0x00BE}, /* TVAR_ash_pGAS[208] */
{0x0F12, 0x008A}, /* TVAR_ash_pGAS[209] */
{0x0F12, 0x005A}, /* TVAR_ash_pGAS[210] */
{0x0F12, 0x0034}, /* TVAR_ash_pGAS[211] */
{0x0F12, 0x0017}, /* TVAR_ash_pGAS[212] */
{0x0F12, 0x0006}, /* TVAR_ash_pGAS[213] */
{0x0F12, 0x0000}, /* TVAR_ash_pGAS[214] */
{0x0F12, 0x0006}, /* TVAR_ash_pGAS[215] */
{0x0F12, 0x0017}, /* TVAR_ash_pGAS[216] */
{0x0F12, 0x0033}, /* TVAR_ash_pGAS[217] */
{0x0F12, 0x0057}, /* TVAR_ash_pGAS[218] */
{0x0F12, 0x0083}, /* TVAR_ash_pGAS[219] */
{0x0F12, 0x00B3}, /* TVAR_ash_pGAS[220] */
{0x0F12, 0x00C5}, /* TVAR_ash_pGAS[221] */
{0x0F12, 0x0091}, /* TVAR_ash_pGAS[222] */
{0x0F12, 0x0061}, /* TVAR_ash_pGAS[223] */
{0x0F12, 0x003B}, /* TVAR_ash_pGAS[224] */
{0x0F12, 0x0020}, /* TVAR_ash_pGAS[225] */
{0x0F12, 0x000F}, /* TVAR_ash_pGAS[226] */
{0x0F12, 0x0009}, /* TVAR_ash_pGAS[227] */
{0x0F12, 0x0010}, /* TVAR_ash_pGAS[228] */
{0x0F12, 0x0021}, /* TVAR_ash_pGAS[229] */
{0x0F12, 0x003D}, /* TVAR_ash_pGAS[230] */
{0x0F12, 0x0060}, /* TVAR_ash_pGAS[231] */
{0x0F12, 0x008D}, /* TVAR_ash_pGAS[232] */
{0x0F12, 0x00BE}, /* TVAR_ash_pGAS[233] */
{0x0F12, 0x00D7}, /* TVAR_ash_pGAS[234] */
{0x0F12, 0x00A2}, /* TVAR_ash_pGAS[235] */
{0x0F12, 0x0072}, /* TVAR_ash_pGAS[236] */
{0x0F12, 0x004D}, /* TVAR_ash_pGAS[237] */
{0x0F12, 0x0032}, /* TVAR_ash_pGAS[238] */
{0x0F12, 0x0022}, /* TVAR_ash_pGAS[239] */
{0x0F12, 0x001D}, /* TVAR_ash_pGAS[240] */
{0x0F12, 0x0024}, /* TVAR_ash_pGAS[241] */
{0x0F12, 0x0035}, /* TVAR_ash_pGAS[242] */
{0x0F12, 0x0050}, /* TVAR_ash_pGAS[243] */
{0x0F12, 0x0073}, /* TVAR_ash_pGAS[244] */
{0x0F12, 0x00A0}, /* TVAR_ash_pGAS[245] */
{0x0F12, 0x00D2}, /* TVAR_ash_pGAS[246] */
{0x0F12, 0x00F0}, /* TVAR_ash_pGAS[247] */
{0x0F12, 0x00BE}, /* TVAR_ash_pGAS[248] */
{0x0F12, 0x008C}, /* TVAR_ash_pGAS[249] */
{0x0F12, 0x0068}, /* TVAR_ash_pGAS[250] */
{0x0F12, 0x004F}, /* TVAR_ash_pGAS[251] */
{0x0F12, 0x0040}, /* TVAR_ash_pGAS[252] */
{0x0F12, 0x003B}, /* TVAR_ash_pGAS[253] */
{0x0F12, 0x0041}, /* TVAR_ash_pGAS[254] */
{0x0F12, 0x0052}, /* TVAR_ash_pGAS[255] */
{0x0F12, 0x006C}, /* TVAR_ash_pGAS[256] */
{0x0F12, 0x008E}, /* TVAR_ash_pGAS[257] */
{0x0F12, 0x00BE}, /* TVAR_ash_pGAS[258] */
{0x0F12, 0x00ED}, /* TVAR_ash_pGAS[259] */
{0x0F12, 0x010C}, /* TVAR_ash_pGAS[260] */
{0x0F12, 0x00E1}, /* TVAR_ash_pGAS[261] */
{0x0F12, 0x00AF}, /* TVAR_ash_pGAS[262] */
{0x0F12, 0x008A}, /* TVAR_ash_pGAS[263] */
{0x0F12, 0x0072}, /* TVAR_ash_pGAS[264] */
{0x0F12, 0x0064}, /* TVAR_ash_pGAS[265] */
{0x0F12, 0x005F}, /* TVAR_ash_pGAS[266] */
{0x0F12, 0x0065}, /* TVAR_ash_pGAS[267] */
{0x0F12, 0x0074}, /* TVAR_ash_pGAS[268] */
{0x0F12, 0x008D}, /* TVAR_ash_pGAS[269] */
{0x0F12, 0x00B2}, /* TVAR_ash_pGAS[270] */
{0x0F12, 0x00E0}, /* TVAR_ash_pGAS[271] */
{0x0F12, 0x010A}, /* TVAR_ash_pGAS[272] */
{0x0F12, 0x012F}, /* TVAR_ash_pGAS[273] */
{0x0F12, 0x0104}, /* TVAR_ash_pGAS[274] */
{0x0F12, 0x00D9}, /* TVAR_ash_pGAS[275] */
{0x0F12, 0x00B3}, /* TVAR_ash_pGAS[276] */
{0x0F12, 0x0099}, /* TVAR_ash_pGAS[277] */
{0x0F12, 0x008B}, /* TVAR_ash_pGAS[278] */
{0x0F12, 0x0086}, /* TVAR_ash_pGAS[279] */
{0x0F12, 0x008B}, /* TVAR_ash_pGAS[280] */
{0x0F12, 0x009B}, /* TVAR_ash_pGAS[281] */
{0x0F12, 0x00B5}, /* TVAR_ash_pGAS[282] */
{0x0F12, 0x00DA}, /* TVAR_ash_pGAS[283] */
{0x0F12, 0x0101}, /* TVAR_ash_pGAS[284] */
{0x0F12, 0x0128}, /* TVAR_ash_pGAS[285] */
{0x0F12, 0x012F}, /* TVAR_ash_pGAS[286] */
{0x0F12, 0x0106}, /* TVAR_ash_pGAS[287] */
{0x0F12, 0x00D4}, /* TVAR_ash_pGAS[288] */
{0x0F12, 0x00AA}, /* TVAR_ash_pGAS[289] */
{0x0F12, 0x008E}, /* TVAR_ash_pGAS[290] */
{0x0F12, 0x007D}, /* TVAR_ash_pGAS[291] */
{0x0F12, 0x0079}, /* TVAR_ash_pGAS[292] */
{0x0F12, 0x0080}, /* TVAR_ash_pGAS[293] */
{0x0F12, 0x0093}, /* TVAR_ash_pGAS[294] */
{0x0F12, 0x00B1}, /* TVAR_ash_pGAS[295] */
{0x0F12, 0x00DC}, /* TVAR_ash_pGAS[296] */
{0x0F12, 0x010C}, /* TVAR_ash_pGAS[297] */
{0x0F12, 0x0130}, /* TVAR_ash_pGAS[298] */
{0x0F12, 0x0112}, /* TVAR_ash_pGAS[299] */
{0x0F12, 0x00E0}, /* TVAR_ash_pGAS[300] */
{0x0F12, 0x00AB}, /* TVAR_ash_pGAS[301] */
{0x0F12, 0x0083}, /* TVAR_ash_pGAS[302] */
{0x0F12, 0x0067}, /* TVAR_ash_pGAS[303] */
{0x0F12, 0x0057}, /* TVAR_ash_pGAS[304] */
{0x0F12, 0x0051}, /* TVAR_ash_pGAS[305] */
{0x0F12, 0x0059}, /* TVAR_ash_pGAS[306] */
{0x0F12, 0x006B}, /* TVAR_ash_pGAS[307] */
{0x0F12, 0x0089}, /* TVAR_ash_pGAS[308] */
{0x0F12, 0x00B2}, /* TVAR_ash_pGAS[309] */
{0x0F12, 0x00E5}, /* TVAR_ash_pGAS[310] */
{0x0F12, 0x0114}, /* TVAR_ash_pGAS[311] */
{0x0F12, 0x00F2}, /* TVAR_ash_pGAS[312] */
{0x0F12, 0x00BD}, /* TVAR_ash_pGAS[313] */
{0x0F12, 0x0088}, /* TVAR_ash_pGAS[314] */
{0x0F12, 0x0061}, /* TVAR_ash_pGAS[315] */
{0x0F12, 0x0044}, /* TVAR_ash_pGAS[316] */
{0x0F12, 0x0031}, /* TVAR_ash_pGAS[317] */
{0x0F12, 0x002C}, /* TVAR_ash_pGAS[318] */
{0x0F12, 0x0033}, /* TVAR_ash_pGAS[319] */
{0x0F12, 0x0047}, /* TVAR_ash_pGAS[320] */
{0x0F12, 0x0065}, /* TVAR_ash_pGAS[321] */
{0x0F12, 0x008C}, /* TVAR_ash_pGAS[322] */
{0x0F12, 0x00C0}, /* TVAR_ash_pGAS[323] */
{0x0F12, 0x00F3}, /* TVAR_ash_pGAS[324] */
{0x0F12, 0x00DB}, /* TVAR_ash_pGAS[325] */
{0x0F12, 0x00A5}, /* TVAR_ash_pGAS[326] */
{0x0F12, 0x0071}, /* TVAR_ash_pGAS[327] */
{0x0F12, 0x0049}, /* TVAR_ash_pGAS[328] */
{0x0F12, 0x002A}, /* TVAR_ash_pGAS[329] */
{0x0F12, 0x0018}, /* TVAR_ash_pGAS[330] */
{0x0F12, 0x0011}, /* TVAR_ash_pGAS[331] */
{0x0F12, 0x0018}, /* TVAR_ash_pGAS[332] */
{0x0F12, 0x002C}, /* TVAR_ash_pGAS[333] */
{0x0F12, 0x004B}, /* TVAR_ash_pGAS[334] */
{0x0F12, 0x0072}, /* TVAR_ash_pGAS[335] */
{0x0F12, 0x00A3}, /* TVAR_ash_pGAS[336] */
{0x0F12, 0x00D7}, /* TVAR_ash_pGAS[337] */
{0x0F12, 0x00CD}, /* TVAR_ash_pGAS[338] */
{0x0F12, 0x0097}, /* TVAR_ash_pGAS[339] */
{0x0F12, 0x0065}, /* TVAR_ash_pGAS[340] */
{0x0F12, 0x003C}, /* TVAR_ash_pGAS[341] */
{0x0F12, 0x001D}, /* TVAR_ash_pGAS[342] */
{0x0F12, 0x000A}, /* TVAR_ash_pGAS[343] */
{0x0F12, 0x0003}, /* TVAR_ash_pGAS[344] */
{0x0F12, 0x0009}, /* TVAR_ash_pGAS[345] */
{0x0F12, 0x001D}, /* TVAR_ash_pGAS[346] */
{0x0F12, 0x003B}, /* TVAR_ash_pGAS[347] */
{0x0F12, 0x0063}, /* TVAR_ash_pGAS[348] */
{0x0F12, 0x0092}, /* TVAR_ash_pGAS[349] */
{0x0F12, 0x00C4}, /* TVAR_ash_pGAS[350] */
{0x0F12, 0x00CA}, /* TVAR_ash_pGAS[351] */
{0x0F12, 0x0094}, /* TVAR_ash_pGAS[352] */
{0x0F12, 0x0062}, /* TVAR_ash_pGAS[353] */
{0x0F12, 0x003A}, /* TVAR_ash_pGAS[354] */
{0x0F12, 0x001A}, /* TVAR_ash_pGAS[355] */
{0x0F12, 0x0007}, /* TVAR_ash_pGAS[356] */
{0x0F12, 0x0000}, /* TVAR_ash_pGAS[357] */
{0x0F12, 0x0006}, /* TVAR_ash_pGAS[358] */
{0x0F12, 0x0018}, /* TVAR_ash_pGAS[359] */
{0x0F12, 0x0036}, /* TVAR_ash_pGAS[360] */
{0x0F12, 0x005C}, /* TVAR_ash_pGAS[361] */
{0x0F12, 0x008A}, /* TVAR_ash_pGAS[362] */
{0x0F12, 0x00BC}, /* TVAR_ash_pGAS[363] */
{0x0F12, 0x00D1}, /* TVAR_ash_pGAS[364] */
{0x0F12, 0x009B}, /* TVAR_ash_pGAS[365] */
{0x0F12, 0x0069}, /* TVAR_ash_pGAS[366] */
{0x0F12, 0x0042}, /* TVAR_ash_pGAS[367] */
{0x0F12, 0x0022}, /* TVAR_ash_pGAS[368] */
{0x0F12, 0x000F}, /* TVAR_ash_pGAS[369] */
{0x0F12, 0x0008}, /* TVAR_ash_pGAS[370] */
{0x0F12, 0x000D}, /* TVAR_ash_pGAS[371] */
{0x0F12, 0x001F}, /* TVAR_ash_pGAS[372] */
{0x0F12, 0x003B}, /* TVAR_ash_pGAS[373] */
{0x0F12, 0x0060}, /* TVAR_ash_pGAS[374] */
{0x0F12, 0x008D}, /* TVAR_ash_pGAS[375] */
{0x0F12, 0x00BF}, /* TVAR_ash_pGAS[376] */
{0x0F12, 0x00E3}, /* TVAR_ash_pGAS[377] */
{0x0F12, 0x00AC}, /* TVAR_ash_pGAS[378] */
{0x0F12, 0x007A}, /* TVAR_ash_pGAS[379] */
{0x0F12, 0x0053}, /* TVAR_ash_pGAS[380] */
{0x0F12, 0x0035}, /* TVAR_ash_pGAS[381] */
{0x0F12, 0x0022}, /* TVAR_ash_pGAS[382] */
{0x0F12, 0x001B}, /* TVAR_ash_pGAS[383] */
{0x0F12, 0x001F}, /* TVAR_ash_pGAS[384] */
{0x0F12, 0x0030}, /* TVAR_ash_pGAS[385] */
{0x0F12, 0x004B}, /* TVAR_ash_pGAS[386] */
{0x0F12, 0x006D}, /* TVAR_ash_pGAS[387] */
{0x0F12, 0x009C}, /* TVAR_ash_pGAS[388] */
{0x0F12, 0x00CE}, /* TVAR_ash_pGAS[389] */
{0x0F12, 0x00FE}, /* TVAR_ash_pGAS[390] */
{0x0F12, 0x00C9}, /* TVAR_ash_pGAS[391] */
{0x0F12, 0x0095}, /* TVAR_ash_pGAS[392] */
{0x0F12, 0x006F}, /* TVAR_ash_pGAS[393] */
{0x0F12, 0x0052}, /* TVAR_ash_pGAS[394] */
{0x0F12, 0x0040}, /* TVAR_ash_pGAS[395] */
{0x0F12, 0x0039}, /* TVAR_ash_pGAS[396] */
{0x0F12, 0x003D}, /* TVAR_ash_pGAS[397] */
{0x0F12, 0x004B}, /* TVAR_ash_pGAS[398] */
{0x0F12, 0x0063}, /* TVAR_ash_pGAS[399] */
{0x0F12, 0x0086}, /* TVAR_ash_pGAS[400] */
{0x0F12, 0x00B5}, /* TVAR_ash_pGAS[401] */
{0x0F12, 0x00E6}, /* TVAR_ash_pGAS[402] */
{0x0F12, 0x011B}, /* TVAR_ash_pGAS[403] */
{0x0F12, 0x00ED}, /* TVAR_ash_pGAS[404] */
{0x0F12, 0x00BA}, /* TVAR_ash_pGAS[405] */
{0x0F12, 0x0092}, /* TVAR_ash_pGAS[406] */
{0x0F12, 0x0076}, /* TVAR_ash_pGAS[407] */
{0x0F12, 0x0065}, /* TVAR_ash_pGAS[408] */
{0x0F12, 0x005D}, /* TVAR_ash_pGAS[409] */
{0x0F12, 0x0060}, /* TVAR_ash_pGAS[410] */
{0x0F12, 0x006D}, /* TVAR_ash_pGAS[411] */
{0x0F12, 0x0084}, /* TVAR_ash_pGAS[412] */
{0x0F12, 0x00A8}, /* TVAR_ash_pGAS[413] */
{0x0F12, 0x00D6}, /* TVAR_ash_pGAS[414] */
{0x0F12, 0x0101}, /* TVAR_ash_pGAS[415] */
{0x0F12, 0x0140}, /* TVAR_ash_pGAS[416] */
{0x0F12, 0x0112}, /* TVAR_ash_pGAS[417] */
{0x0F12, 0x00E5}, /* TVAR_ash_pGAS[418] */
{0x0F12, 0x00BD}, /* TVAR_ash_pGAS[419] */
{0x0F12, 0x009E}, /* TVAR_ash_pGAS[420] */
{0x0F12, 0x008C}, /* TVAR_ash_pGAS[421] */
{0x0F12, 0x0085}, /* TVAR_ash_pGAS[422] */
{0x0F12, 0x0087}, /* TVAR_ash_pGAS[423] */
{0x0F12, 0x0094}, /* TVAR_ash_pGAS[424] */
{0x0F12, 0x00AC}, /* TVAR_ash_pGAS[425] */
{0x0F12, 0x00D0}, /* TVAR_ash_pGAS[426] */
{0x0F12, 0x00F8}, /* TVAR_ash_pGAS[427] */
{0x0F12, 0x0123}, /* TVAR_ash_pGAS[428] */
{0x0F12, 0x00F2}, /* TVAR_ash_pGAS[429] */
{0x0F12, 0x00D1}, /* TVAR_ash_pGAS[430] */
{0x0F12, 0x00A7}, /* TVAR_ash_pGAS[431] */
{0x0F12, 0x0087}, /* TVAR_ash_pGAS[432] */
{0x0F12, 0x0073}, /* TVAR_ash_pGAS[433] */
{0x0F12, 0x0067}, /* TVAR_ash_pGAS[434] */
{0x0F12, 0x0064}, /* TVAR_ash_pGAS[435] */
{0x0F12, 0x006B}, /* TVAR_ash_pGAS[436] */
{0x0F12, 0x007C}, /* TVAR_ash_pGAS[437] */
{0x0F12, 0x0094}, /* TVAR_ash_pGAS[438] */
{0x0F12, 0x00B7}, /* TVAR_ash_pGAS[439] */
{0x0F12, 0x00E1}, /* TVAR_ash_pGAS[440] */
{0x0F12, 0x00FF}, /* TVAR_ash_pGAS[441] */
{0x0F12, 0x00D6}, /* TVAR_ash_pGAS[442] */
{0x0F12, 0x00AE}, /* TVAR_ash_pGAS[443] */
{0x0F12, 0x0085}, /* TVAR_ash_pGAS[444] */
{0x0F12, 0x0068}, /* TVAR_ash_pGAS[445] */
{0x0F12, 0x0054}, /* TVAR_ash_pGAS[446] */
{0x0F12, 0x0048}, /* TVAR_ash_pGAS[447] */
{0x0F12, 0x0045}, /* TVAR_ash_pGAS[448] */
{0x0F12, 0x004B}, /* TVAR_ash_pGAS[449] */
{0x0F12, 0x005B}, /* TVAR_ash_pGAS[450] */
{0x0F12, 0x0073}, /* TVAR_ash_pGAS[451] */
{0x0F12, 0x0093}, /* TVAR_ash_pGAS[452] */
{0x0F12, 0x00BF}, /* TVAR_ash_pGAS[453] */
{0x0F12, 0x00E9}, /* TVAR_ash_pGAS[454] */
{0x0F12, 0x00B8}, /* TVAR_ash_pGAS[455] */
{0x0F12, 0x008E}, /* TVAR_ash_pGAS[456] */
{0x0F12, 0x0066}, /* TVAR_ash_pGAS[457] */
{0x0F12, 0x0049}, /* TVAR_ash_pGAS[458] */
{0x0F12, 0x0035}, /* TVAR_ash_pGAS[459] */
{0x0F12, 0x0028}, /* TVAR_ash_pGAS[460] */
{0x0F12, 0x0025}, /* TVAR_ash_pGAS[461] */
{0x0F12, 0x002B}, /* TVAR_ash_pGAS[462] */
{0x0F12, 0x003B}, /* TVAR_ash_pGAS[463] */
{0x0F12, 0x0053}, /* TVAR_ash_pGAS[464] */
{0x0F12, 0x0072}, /* TVAR_ash_pGAS[465] */
{0x0F12, 0x009D}, /* TVAR_ash_pGAS[466] */
{0x0F12, 0x00C8}, /* TVAR_ash_pGAS[467] */
{0x0F12, 0x00A2}, /* TVAR_ash_pGAS[468] */
{0x0F12, 0x0078}, /* TVAR_ash_pGAS[469] */
{0x0F12, 0x0051}, /* TVAR_ash_pGAS[470] */
{0x0F12, 0x0034}, /* TVAR_ash_pGAS[471] */
{0x0F12, 0x001F}, /* TVAR_ash_pGAS[472] */
{0x0F12, 0x0012}, /* TVAR_ash_pGAS[473] */
{0x0F12, 0x000E}, /* TVAR_ash_pGAS[474] */
{0x0F12, 0x0014}, /* TVAR_ash_pGAS[475] */
{0x0F12, 0x0024}, /* TVAR_ash_pGAS[476] */
{0x0F12, 0x003B}, /* TVAR_ash_pGAS[477] */
{0x0F12, 0x005B}, /* TVAR_ash_pGAS[478] */
{0x0F12, 0x0083}, /* TVAR_ash_pGAS[479] */
{0x0F12, 0x00AD}, /* TVAR_ash_pGAS[480] */
{0x0F12, 0x0095}, /* TVAR_ash_pGAS[481] */
{0x0F12, 0x006C}, /* TVAR_ash_pGAS[482] */
{0x0F12, 0x0046}, /* TVAR_ash_pGAS[483] */
{0x0F12, 0x002A}, /* TVAR_ash_pGAS[484] */
{0x0F12, 0x0014}, /* TVAR_ash_pGAS[485] */
{0x0F12, 0x0007}, /* TVAR_ash_pGAS[486] */
{0x0F12, 0x0002}, /* TVAR_ash_pGAS[487] */
{0x0F12, 0x0008}, /* TVAR_ash_pGAS[488] */
{0x0F12, 0x0016}, /* TVAR_ash_pGAS[489] */
{0x0F12, 0x002D}, /* TVAR_ash_pGAS[490] */
{0x0F12, 0x004C}, /* TVAR_ash_pGAS[491] */
{0x0F12, 0x0072}, /* TVAR_ash_pGAS[492] */
{0x0F12, 0x009B}, /* TVAR_ash_pGAS[493] */
{0x0F12, 0x0093}, /* TVAR_ash_pGAS[494] */
{0x0F12, 0x006A}, /* TVAR_ash_pGAS[495] */
{0x0F12, 0x0045}, /* TVAR_ash_pGAS[496] */
{0x0F12, 0x0028}, /* TVAR_ash_pGAS[497] */
{0x0F12, 0x0013}, /* TVAR_ash_pGAS[498] */
{0x0F12, 0x0005}, /* TVAR_ash_pGAS[499] */
{0x0F12, 0x0000}, /* TVAR_ash_pGAS[500] */
{0x0F12, 0x0004}, /* TVAR_ash_pGAS[501] */
{0x0F12, 0x0012}, /* TVAR_ash_pGAS[502] */
{0x0F12, 0x0028}, /* TVAR_ash_pGAS[503] */
{0x0F12, 0x0045}, /* TVAR_ash_pGAS[504] */
{0x0F12, 0x006A}, /* TVAR_ash_pGAS[505] */
{0x0F12, 0x0093}, /* TVAR_ash_pGAS[506] */
{0x0F12, 0x009B}, /* TVAR_ash_pGAS[507] */
{0x0F12, 0x0071}, /* TVAR_ash_pGAS[508] */
{0x0F12, 0x004C}, /* TVAR_ash_pGAS[509] */
{0x0F12, 0x0030}, /* TVAR_ash_pGAS[510] */
{0x0F12, 0x001A}, /* TVAR_ash_pGAS[511] */
{0x0F12, 0x000C}, /* TVAR_ash_pGAS[512] */
{0x0F12, 0x0007}, /* TVAR_ash_pGAS[513] */
{0x0F12, 0x000B}, /* TVAR_ash_pGAS[514] */
{0x0F12, 0x0018}, /* TVAR_ash_pGAS[515] */
{0x0F12, 0x002C}, /* TVAR_ash_pGAS[516] */
{0x0F12, 0x0048}, /* TVAR_ash_pGAS[517] */
{0x0F12, 0x006D}, /* TVAR_ash_pGAS[518] */
{0x0F12, 0x0097}, /* TVAR_ash_pGAS[519] */
{0x0F12, 0x00AE}, /* TVAR_ash_pGAS[520] */
{0x0F12, 0x0083}, /* TVAR_ash_pGAS[521] */
{0x0F12, 0x005C}, /* TVAR_ash_pGAS[522] */
{0x0F12, 0x0040}, /* TVAR_ash_pGAS[523] */
{0x0F12, 0x002B}, /* TVAR_ash_pGAS[524] */
{0x0F12, 0x001E}, /* TVAR_ash_pGAS[525] */
{0x0F12, 0x0018}, /* TVAR_ash_pGAS[526] */
{0x0F12, 0x001C}, /* TVAR_ash_pGAS[527] */
{0x0F12, 0x0027}, /* TVAR_ash_pGAS[528] */
{0x0F12, 0x003A}, /* TVAR_ash_pGAS[529] */
{0x0F12, 0x0055}, /* TVAR_ash_pGAS[530] */
{0x0F12, 0x007B}, /* TVAR_ash_pGAS[531] */
{0x0F12, 0x00A6}, /* TVAR_ash_pGAS[532] */
{0x0F12, 0x00CA}, /* TVAR_ash_pGAS[533] */
{0x0F12, 0x009E}, /* TVAR_ash_pGAS[534] */
{0x0F12, 0x0076}, /* TVAR_ash_pGAS[535] */
{0x0F12, 0x0059}, /* TVAR_ash_pGAS[536] */
{0x0F12, 0x0046}, /* TVAR_ash_pGAS[537] */
{0x0F12, 0x0039}, /* TVAR_ash_pGAS[538] */
{0x0F12, 0x0033}, /* TVAR_ash_pGAS[539] */
{0x0F12, 0x0036}, /* TVAR_ash_pGAS[540] */
{0x0F12, 0x0040}, /* TVAR_ash_pGAS[541] */
{0x0F12, 0x0052}, /* TVAR_ash_pGAS[542] */
{0x0F12, 0x006C}, /* TVAR_ash_pGAS[543] */
{0x0F12, 0x0094}, /* TVAR_ash_pGAS[544] */
{0x0F12, 0x00BF}, /* TVAR_ash_pGAS[545] */
{0x0F12, 0x00EB}, /* TVAR_ash_pGAS[546] */
{0x0F12, 0x00C3}, /* TVAR_ash_pGAS[547] */
{0x0F12, 0x0099}, /* TVAR_ash_pGAS[548] */
{0x0F12, 0x007A}, /* TVAR_ash_pGAS[549] */
{0x0F12, 0x0066}, /* TVAR_ash_pGAS[550] */
{0x0F12, 0x005A}, /* TVAR_ash_pGAS[551] */
{0x0F12, 0x0054}, /* TVAR_ash_pGAS[552] */
{0x0F12, 0x0056}, /* TVAR_ash_pGAS[553] */
{0x0F12, 0x005F}, /* TVAR_ash_pGAS[554] */
{0x0F12, 0x0071}, /* TVAR_ash_pGAS[555] */
{0x0F12, 0x008D}, /* TVAR_ash_pGAS[556] */
{0x0F12, 0x00B6}, /* TVAR_ash_pGAS[557] */
{0x0F12, 0x00DE}, /* TVAR_ash_pGAS[558] */
{0x0F12, 0x010D}, /* TVAR_ash_pGAS[559] */
{0x0F12, 0x00E7}, /* TVAR_ash_pGAS[560] */
{0x0F12, 0x00C1}, /* TVAR_ash_pGAS[561] */
{0x0F12, 0x00A0}, /* TVAR_ash_pGAS[562] */
{0x0F12, 0x008A}, /* TVAR_ash_pGAS[563] */
{0x0F12, 0x007C}, /* TVAR_ash_pGAS[564] */
{0x0F12, 0x0076}, /* TVAR_ash_pGAS[565] */
{0x0F12, 0x0078}, /* TVAR_ash_pGAS[566] */
{0x0F12, 0x0081}, /* TVAR_ash_pGAS[567] */
{0x0F12, 0x0093}, /* TVAR_ash_pGAS[568] */
{0x0F12, 0x00B1}, /* TVAR_ash_pGAS[569] */
{0x0F12, 0x00D5}, /* TVAR_ash_pGAS[570] */
{0x0F12, 0x00FD}, /* TVAR_ash_pGAS[571] */

/* Gamma */
{0x002A, 0x04CC},
{0x0F12, 0x0000}, /* 0000 SARR_usGammaLutRGBIndoor[0][0]  */
{0x0F12, 0x0002}, /* 0002 SARR_usGammaLutRGBIndoor[0][1]  */
{0x0F12, 0x0008}, /* 0008 SARR_usGammaLutRGBIndoor[0][2]  */
{0x0F12, 0x0016}, /* 0018 SARR_usGammaLutRGBIndoor[0][3]  */
{0x0F12, 0x0055}, /* 005A SARR_usGammaLutRGBIndoor[0][4]  */
{0x0F12, 0x00E6}, /* 00DF SARR_usGammaLutRGBIndoor[0][5]  */
{0x0F12, 0x0141}, /* 013F SARR_usGammaLutRGBIndoor[0][6]  */
{0x0F12, 0x0188}, /* 0186 SARR_usGammaLutRGBIndoor[0][7]  */
{0x0F12, 0x01E6}, /* 01E6 SARR_usGammaLutRGBIndoor[0][8]  */
{0x0F12, 0x0236}, /* 0236 SARR_usGammaLutRGBIndoor[0][9]  */
{0x0F12, 0x02BA}, /* 02BA SARR_usGammaLutRGBIndoor[0][10] */
{0x0F12, 0x032A}, /* 032A SARR_usGammaLutRGBIndoor[0][11] */
{0x0F12, 0x0385}, /* 0385 SARR_usGammaLutRGBIndoor[0][12] */
{0x0F12, 0x03C2}, /* 03C2 SARR_usGammaLutRGBIndoor[0][13] */
{0x0F12, 0x03EA}, /* 03EA SARR_usGammaLutRGBIndoor[0][14] */
{0x0F12, 0x03FF}, /* 03FF SARR_usGammaLutRGBIndoor[0][15] */

{0x0F12, 0x0000}, /* 0000 SARR_usGammaLutRGBIndoor[1][0]  */
{0x0F12, 0x0002}, /* 0002 SARR_usGammaLutRGBIndoor[1][1]  */
{0x0F12, 0x0008}, /* 0008 SARR_usGammaLutRGBIndoor[1][2]  */
{0x0F12, 0x0016}, /* 0018 SARR_usGammaLutRGBIndoor[1][3]  */
{0x0F12, 0x0055}, /* 005A SARR_usGammaLutRGBIndoor[1][4]  */
{0x0F12, 0x00E6}, /* 00DF SARR_usGammaLutRGBIndoor[1][5]  */
{0x0F12, 0x0141}, /* 013F SARR_usGammaLutRGBIndoor[1][6]  */
{0x0F12, 0x0188}, /* 0186 SARR_usGammaLutRGBIndoor[1][7]  */
{0x0F12, 0x01E6}, /* 01E6 SARR_usGammaLutRGBIndoor[1][8]  */
{0x0F12, 0x0236}, /* 0236 SARR_usGammaLutRGBIndoor[1][9]  */
{0x0F12, 0x02BA}, /* 02BA SARR_usGammaLutRGBIndoor[1][10] */
{0x0F12, 0x032A}, /* 032A SARR_usGammaLutRGBIndoor[1][11] */
{0x0F12, 0x0385}, /* 0385 SARR_usGammaLutRGBIndoor[1][12] */
{0x0F12, 0x03C2}, /* 03C2 SARR_usGammaLutRGBIndoor[1][13] */
{0x0F12, 0x03EA}, /* 03EA SARR_usGammaLutRGBIndoor[1][14] */
{0x0F12, 0x03FF}, /* 03FF SARR_usGammaLutRGBIndoor[1][15] */

{0x0F12, 0x0000}, /* 0000 SARR_usGammaLutRGBIndoor[2][0]  */
{0x0F12, 0x0002}, /* 0002 SARR_usGammaLutRGBIndoor[2][1]  */
{0x0F12, 0x0008}, /* 0008 SARR_usGammaLutRGBIndoor[2][2]  */
{0x0F12, 0x0016}, /* 0018 SARR_usGammaLutRGBIndoor[2][3]  */
{0x0F12, 0x0055}, /* 005A SARR_usGammaLutRGBIndoor[2][4]  */
{0x0F12, 0x00E6}, /* 00DF SARR_usGammaLutRGBIndoor[2][5]  */
{0x0F12, 0x0141}, /* 013F SARR_usGammaLutRGBIndoor[2][6]  */
{0x0F12, 0x0188}, /* 0186 SARR_usGammaLutRGBIndoor[2][7]  */
{0x0F12, 0x01E6}, /* 01E6 SARR_usGammaLutRGBIndoor[2][8]  */
{0x0F12, 0x0236}, /* 0236 SARR_usGammaLutRGBIndoor[2][9]  */
{0x0F12, 0x02BA}, /* 02BA SARR_usGammaLutRGBIndoor[2][10] */
{0x0F12, 0x032A}, /* 032A SARR_usGammaLutRGBIndoor[2][11] */
{0x0F12, 0x0385}, /* 0385 SARR_usGammaLutRGBIndoor[2][12] */
{0x0F12, 0x03C2}, /* 03C2 SARR_usGammaLutRGBIndoor[2][13] */
{0x0F12, 0x03EA}, /* 03EA SARR_usGammaLutRGBIndoor[2][14] */
{0x0F12, 0x03FF}, /* 03FF SARR_usGammaLutRGBIndoor[2][15] */


/* Set AWB */
{0x002A, 0x0DA6},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x002A, 0x0E8C},
{0x0F12, 0x0000},
{0x002A, 0x0D6C},
{0x0F12, 0x0040},

/* Indoor Gray Zone */
{0x002A, 0x0B9C},
{0x0F12, 0x038F}, /* awbb_IndoorGrZones_m_BGrid_0__m_left   */
{0x0F12, 0x039B}, /* awbb_IndoorGrZones_m_BGrid_0__m_right  */
{0x0F12, 0x0373}, /* awbb_IndoorGrZones_m_BGrid_1__m_left   */
{0x0F12, 0x03B0}, /* awbb_IndoorGrZones_m_BGrid_1__m_right  */
{0x0F12, 0x0352}, /* awbb_IndoorGrZones_m_BGrid_2__m_left   */
{0x0F12, 0x03B7}, /* awbb_IndoorGrZones_m_BGrid_2__m_right  */
{0x0F12, 0x0334}, /* awbb_IndoorGrZones_m_BGrid_3__m_left   */
{0x0F12, 0x03B5}, /* awbb_IndoorGrZones_m_BGrid_3__m_right  */
{0x0F12, 0x0318}, /* awbb_IndoorGrZones_m_BGrid_4__m_left   */
{0x0F12, 0x03B0}, /* awbb_IndoorGrZones_m_BGrid_4__m_right  */
{0x0F12, 0x02FF}, /* awbb_IndoorGrZones_m_BGrid_5__m_left   */
{0x0F12, 0x038D}, /* awbb_IndoorGrZones_m_BGrid_5__m_right  */
{0x0F12, 0x02E7}, /* awbb_IndoorGrZones_m_BGrid_6__m_left   */
{0x0F12, 0x0372}, /* awbb_IndoorGrZones_m_BGrid_6__m_right  */
{0x0F12, 0x02D0}, /* awbb_IndoorGrZones_m_BGrid_7__m_left   */
{0x0F12, 0x035D}, /* awbb_IndoorGrZones_m_BGrid_7__m_right  */
{0x0F12, 0x02B5}, /* awbb_IndoorGrZones_m_BGrid_8__m_left   */
{0x0F12, 0x0345}, /* awbb_IndoorGrZones_m_BGrid_8__m_right  */
{0x0F12, 0x02A1}, /* awbb_IndoorGrZones_m_BGrid_9__m_left   */
{0x0F12, 0x0331}, /* awbb_IndoorGrZones_m_BGrid_9__m_right  */
{0x0F12, 0x028B}, /* awbb_IndoorGrZones_m_BGrid_10__m_left  */
{0x0F12, 0x031E}, /* awbb_IndoorGrZones_m_BGrid_10__m_right */
{0x0F12, 0x0273}, /* awbb_IndoorGrZones_m_BGrid_11__m_left  */
{0x0F12, 0x0309}, /* awbb_IndoorGrZones_m_BGrid_11__m_right */
{0x0F12, 0x025F}, /* awbb_IndoorGrZones_m_BGrid_12__m_left  */
{0x0F12, 0x02F5}, /* awbb_IndoorGrZones_m_BGrid_12__m_right */
{0x0F12, 0x0250}, /* awbb_IndoorGrZones_m_BGrid_13__m_left  */
{0x0F12, 0x02DB}, /* awbb_IndoorGrZones_m_BGrid_13__m_right */
{0x0F12, 0x0241}, /* awbb_IndoorGrZones_m_BGrid_14__m_left  */
{0x0F12, 0x02C7}, /* awbb_IndoorGrZones_m_BGrid_14__m_right */
{0x0F12, 0x0233}, /* awbb_IndoorGrZones_m_BGrid_15__m_left  */
{0x0F12, 0x02B9}, /* awbb_IndoorGrZones_m_BGrid_15__m_right */
{0x0F12, 0x0223}, /* awbb_IndoorGrZones_m_BGrid_16__m_left  */
{0x0F12, 0x02AB}, /* awbb_IndoorGrZones_m_BGrid_16__m_right */
{0x0F12, 0x0217}, /* awbb_IndoorGrZones_m_BGrid_17__m_left  */
{0x0F12, 0x02A2}, /* awbb_IndoorGrZones_m_BGrid_17__m_right */
{0x0F12, 0x0207}, /* awbb_IndoorGrZones_m_BGrid_18__m_left  */
{0x0F12, 0x0294}, /* awbb_IndoorGrZones_m_BGrid_18__m_right */
{0x0F12, 0x01FA}, /* awbb_IndoorGrZones_m_BGrid_19__m_left  */
{0x0F12, 0x0289}, /* awbb_IndoorGrZones_m_BGrid_19__m_right */
{0x0F12, 0x01EA}, /* awbb_IndoorGrZones_m_BGrid_20__m_left  */
{0x0F12, 0x0281}, /* awbb_IndoorGrZones_m_BGrid_20__m_right */
{0x0F12, 0x01DD}, /* awbb_IndoorGrZones_m_BGrid_21__m_left  */
{0x0F12, 0x027B}, /* awbb_IndoorGrZones_m_BGrid_21__m_right */
{0x0F12, 0x01D0}, /* awbb_IndoorGrZones_m_BGrid_22__m_left  */
{0x0F12, 0x0273}, /* awbb_IndoorGrZones_m_BGrid_22__m_right */
{0x0F12, 0x01C3}, /* awbb_IndoorGrZones_m_BGrid_23__m_left  */
{0x0F12, 0x026A}, /* awbb_IndoorGrZones_m_BGrid_23__m_right */
{0x0F12, 0x01B6}, /* awbb_IndoorGrZones_m_BGrid_24__m_left  */
{0x0F12, 0x0265}, /* awbb_IndoorGrZones_m_BGrid_24__m_right */
{0x0F12, 0x01AB}, /* awbb_IndoorGrZones_m_BGrid_25__m_left  */
{0x0F12, 0x025B}, /* awbb_IndoorGrZones_m_BGrid_25__m_right */
{0x0F12, 0x01A1}, /* awbb_IndoorGrZones_m_BGrid_26__m_left  */
{0x0F12, 0x0254}, /* awbb_IndoorGrZones_m_BGrid_26__m_right */
{0x0F12, 0x0198}, /* awbb_IndoorGrZones_m_BGrid_27__m_left  */
{0x0F12, 0x024B}, /* awbb_IndoorGrZones_m_BGrid_27__m_right */
{0x0F12, 0x0192}, /* awbb_IndoorGrZones_m_BGrid_28__m_left  */
{0x0F12, 0x0242}, /* awbb_IndoorGrZones_m_BGrid_28__m_right */
{0x0F12, 0x0191}, /* awbb_IndoorGrZones_m_BGrid_29__m_left  */
{0x0F12, 0x023A}, /* awbb_IndoorGrZones_m_BGrid_29__m_right */
{0x0F12, 0x0192}, /* awbb_IndoorGrZones_m_BGrid_30__m_left  */
{0x0F12, 0x0222}, /* awbb_IndoorGrZones_m_BGrid_30__m_right */
{0x0F12, 0x01C5}, /* awbb_IndoorGrZones_m_BGrid_31__m_left  */
{0x0F12, 0x01DF}, /* awbb_IndoorGrZones_m_BGrid_31__m_right */
{0x0F12, 0x0000}, /* awbb_IndoorGrZones_m_BGrid_32__m_left  */
{0x0F12, 0x0000}, /* awbb_IndoorGrZones_m_BGrid_32__m_right */
{0x0F12, 0x0000}, /* awbb_IndoorGrZones_m_BGrid_33__m_left  */
{0x0F12, 0x0000}, /* awbb_IndoorGrZones_m_BGrid_33__m_right */


/* param_end	awbb_IndoorGrZones_m_BGrid */
{0x002A, 0x0C3C},
{0x0F12, 0x0004},
{0x0F12, 0x0000},
{0x0F12, 0x0022},
{0x0F12, 0x0000},
{0x0F12, 0x010F},
{0x0F12, 0x0000},
{0x0F12, 0x0020},
{0x0F12, 0x0000},
{0x002A, 0x0C50},
{0x0F12, 0x00E0},
{0x0F12, 0x0000},

/* Outdoor Gray Zone */
{0x0F12, 0x025E}, /* 0264 awbb_OutdoorGrZones_m_BGrid_0__m_left    */
{0x0F12, 0x0282}, /* 0279 awbb_OutdoorGrZones_m_BGrid_0__m_right   */
{0x0F12, 0x0240}, /* 0250 awbb_OutdoorGrZones_m_BGrid_1__m_left    */
{0x0F12, 0x0298}, /* 0287 awbb_OutdoorGrZones_m_BGrid_1__m_right   */
{0x0F12, 0x022A}, /* 0244 awbb_OutdoorGrZones_m_BGrid_2__m_left    */
{0x0F12, 0x029A}, /* 0287 awbb_OutdoorGrZones_m_BGrid_2__m_right   */
{0x0F12, 0x021A}, /* 0235 awbb_OutdoorGrZones_m_BGrid_3__m_left    */
{0x0F12, 0x029A}, /* 0289 awbb_OutdoorGrZones_m_BGrid_3__m_right   */
{0x0F12, 0x0206}, /* 0225 awbb_OutdoorGrZones_m_BGrid_4__m_left    */
{0x0F12, 0x0298}, /* 0287 awbb_OutdoorGrZones_m_BGrid_4__m_right   */
{0x0F12, 0x01FE}, /* 0213 awbb_OutdoorGrZones_m_BGrid_5__m_left    */
{0x0F12, 0x028C}, /* 0286 awbb_OutdoorGrZones_m_BGrid_5__m_right   */
{0x0F12, 0x01FA}, /* 0202 awbb_OutdoorGrZones_m_BGrid_6__m_left    */
{0x0F12, 0x0278}, /* 027A awbb_OutdoorGrZones_m_BGrid_6__m_right   */
{0x0F12, 0x01F8}, /* 01F3 awbb_OutdoorGrZones_m_BGrid_7__m_left    */
{0x0F12, 0x0266}, /* 0272 awbb_OutdoorGrZones_m_BGrid_7__m_right   */
{0x0F12, 0x0214}, /* 01E9 awbb_OutdoorGrZones_m_BGrid_8__m_left    */
{0x0F12, 0x0238}, /* 0269 awbb_OutdoorGrZones_m_BGrid_8__m_right   */
{0x0F12, 0x0000}, /* 01E2 awbb_OutdoorGrZones_m_BGrid_9__m_left    */
{0x0F12, 0x0000}, /* 0263 awbb_OutdoorGrZones_m_BGrid_9__m_right   */
{0x0F12, 0x0000}, /* 01E0 awbb_OutdoorGrZones_m_BGrid_10__m_left   */
{0x0F12, 0x0000}, /* 025A awbb_OutdoorGrZones_m_BGrid_10__m_right  */
{0x0F12, 0x0000}, /* 01E1 awbb_OutdoorGrZones_m_BGrid_11__m_left   */
{0x0F12, 0x0000}, /* 0256 awbb_OutdoorGrZones_m_BGrid_11__m_right  */
{0x0F12, 0x0000}, /* 01EE awbb_OutdoorGrZones_m_BGrid_12__m_left   */
{0x0F12, 0x0000}, /* 0251 awbb_OutdoorGrZones_m_BGrid_12__m_right  */
{0x0F12, 0x0000}, /* 01F8 awbb_OutdoorGrZones_m_BGrid(26)          */
{0x0F12, 0x0000}, /* 024A awbb_OutdoorGrZones_m_BGrid(27)          */
{0x0F12, 0x0000}, /* 020D awbb_OutdoorGrZones_m_BGrid(28)          */
{0x0F12, 0x0000}, /* 0231 awbb_OutdoorGrZones_m_BGrid(29)          */
{0x0F12, 0x0000}, /* 0000 awbb_OutdoorGrZones_m_BGrid(30)          */
{0x0F12, 0x0000}, /* 0000 awbb_OutdoorGrZones_m_BGrid(31)          */
{0x0F12, 0x0000}, /* 0000 awbb_OutdoorGrZones_m_BGrid(32)          */
{0x0F12, 0x0000}, /* 0000 awbb_OutdoorGrZones_m_BGrid(33)          */


/* param_WRITE 70000CC6  B2end	awbb_OutdoorGrZones_m_BGrid */
{0x002A, 0x0CB8},
{0x0F12, 0x0004},
{0x0F12, 0x0000},
{0x0F12, 0x0009},
{0x0F12, 0x0000},
{0x0F12, 0x0210},
{0x0F12, 0x0000},
{0x0F12, 0x0020},
{0x0F12, 0x0000},
{0x002A, 0x0CCC},
{0x0F12, 0x00C0},
{0x0F12, 0x0000},

/*  7-3. Low Br grey zone                     */
/* param_  C4start	awbb_LowBrGrZones_m_BGrid */

{0x0F12, 0x031F}, /* awbb_LowBrGrZones_m_BGrid_0__m_left   */
{0x0F12, 0x0495}, /* awbb_LowBrGrZones_m_BGrid_0__m_right  */
{0x0F12, 0x02FC}, /* awbb_LowBrGrZones_m_BGrid_1__m_left   */
{0x0F12, 0x0495}, /* awbb_LowBrGrZones_m_BGrid_1__m_right  */
{0x0F12, 0x02D9}, /* awbb_LowBrGrZones_m_BGrid_2__m_left   */
{0x0F12, 0x0495}, /* awbb_LowBrGrZones_m_BGrid_2__m_right  */
{0x0F12, 0x02B6}, /* awbb_LowBrGrZones_m_BGrid_3__m_left   */
{0x0F12, 0x0495}, /* awbb_LowBrGrZones_m_BGrid_3__m_right  */
{0x0F12, 0x0293}, /* awbb_LowBrGrZones_m_BGrid_4__m_left   */
{0x0F12, 0x0495}, /* awbb_LowBrGrZones_m_BGrid_4__m_right  */
{0x0F12, 0x0270}, /* awbb_LowBrGrZones_m_BGrid_5__m_left   */
{0x0F12, 0x0495}, /* awbb_LowBrGrZones_m_BGrid_5__m_right  */
{0x0F12, 0x024E}, /* awbb_LowBrGrZones_m_BGrid_6__m_left   */
{0x0F12, 0x0495}, /* awbb_LowBrGrZones_m_BGrid_6__m_right  */
{0x0F12, 0x022B}, /* awbb_LowBrGrZones_m_BGrid_7__m_left   */
{0x0F12, 0x0495}, /* awbb_LowBrGrZones_m_BGrid_7__m_right  */
{0x0F12, 0x0208}, /* awbb_LowBrGrZones_m_BGrid_8__m_left   */
{0x0F12, 0x048A}, /* awbb_LowBrGrZones_m_BGrid_8__m_right  */
{0x0F12, 0x01E5}, /* awbb_LowBrGrZones_m_BGrid_9__m_left   */
{0x0F12, 0x0455}, /* awbb_LowBrGrZones_m_BGrid_9__m_right  */
{0x0F12, 0x01C2}, /* awbb_LowBrGrZones_m_BGrid_10__m_left  */
{0x0F12, 0x041F}, /* awbb_LowBrGrZones_m_BGrid_10__m_right */
{0x0F12, 0x019F}, /* awbb_LowBrGrZones_m_BGrid_11__m_left  */
{0x0F12, 0x03EA}, /* awbb_LowBrGrZones_m_BGrid_11__m_right */
{0x0F12, 0x017D}, /* awbb_LowBrGrZones_m_BGrid_12__m_left  */
{0x0F12, 0x03B4}, /* awbb_LowBrGrZones_m_BGrid_12__m_right */
{0x0F12, 0x015A}, /* awbb_LowBrGrZones_m_BGrid_13__m_left  */
{0x0F12, 0x037F}, /* awbb_LowBrGrZones_m_BGrid_13__m_right */
{0x0F12, 0x0137}, /* awbb_LowBrGrZones_m_BGrid_14__m_left  */
{0x0F12, 0x0349}, /* awbb_LowBrGrZones_m_BGrid_14__m_right */
{0x0F12, 0x0130}, /* awbb_LowBrGrZones_m_BGrid_15__m_left  */
{0x0F12, 0x0314}, /* awbb_LowBrGrZones_m_BGrid_15__m_right */
{0x0F12, 0x012F}, /* awbb_LowBrGrZones_m_BGrid_16__m_left  */
{0x0F12, 0x02DE}, /* awbb_LowBrGrZones_m_BGrid_16__m_right */
{0x0F12, 0x012F}, /* awbb_LowBrGrZones_m_BGrid_17__m_left  */
{0x0F12, 0x02B1}, /* awbb_LowBrGrZones_m_BGrid_17__m_right */
{0x0F12, 0x012E}, /* awbb_LowBrGrZones_m_BGrid_18__m_left  */
{0x0F12, 0x028B}, /* awbb_LowBrGrZones_m_BGrid_18__m_right */
{0x0F12, 0x012D}, /* awbb_LowBrGrZones_m_BGrid_19__m_left  */
{0x0F12, 0x0265}, /* awbb_LowBrGrZones_m_BGrid_19__m_right */
{0x0F12, 0x012C}, /* awbb_LowBrGrZones_m_BGrid_20__m_left  */
{0x0F12, 0x023F}, /* awbb_LowBrGrZones_m_BGrid_20__m_right */
{0x0F12, 0x012C}, /* awbb_LowBrGrZones_m_BGrid_21__m_left  */
{0x0F12, 0x0219}, /* awbb_LowBrGrZones_m_BGrid_21__m_right */
{0x0F12, 0x012B}, /* awbb_LowBrGrZones_m_BGrid_22__m_left  */
{0x0F12, 0x01F3}, /* awbb_LowBrGrZones_m_BGrid_22__m_right */
{0x0F12, 0x012A}, /* awbb_LowBrGrZones_m_BGrid_23__m_left  */
{0x0F12, 0x01CD}, /* awbb_LowBrGrZones_m_BGrid_23__m_right */
{0x0F12, 0x0000}, /* awbb_LowBrGrZones_m_BGrid_24__m_left  */
{0x0F12, 0x0000}, /* awbb_LowBrGrZones_m_BGrid_24__m_right */


/* 42param_end awbb_LowBrGrZones_m_BGrid */
{0x0F12, 0x0005},
{0x0F12, 0x0000},
{0x0F12, 0x0018},
{0x0F12, 0x0000},
{0x0F12, 0x00AF},
{0x0F12, 0x0000},
{0x0F12, 0x0002},
{0x0F12, 0x0000},
{0x002A, 0x0D48},
{0x0F12, 0x00E0},
{0x0F12, 0x0000},

/* Lowtemp circle */
{0x0F12, 0x032F},
{0x0F12, 0x0000},
{0x0F12, 0x017A},
{0x0F12, 0x0000},
{0x0F12, 0x7300},
{0x0F12, 0x0000},
{0x0F12, 0x000A},
{0x0F12, 0x0000},
{0x002A, 0x0D60},
{0x0F12, 0x00E0},
{0x0F12, 0x0000},
{0x002A, 0x0D82},
{0x0F12, 0x0001},



{0x002A, 0x0D8E},
{0x0F12, 0x0002}, /* awbb_GridEnable */

/* Grid coefficients and Contrants */
{0x002A, 0x0DCE},
{0x0F12, 0xFFE0}, /* awbb_GridCorr_R_0__0_ */
{0x0F12, 0xFFE0}, /* D8 awbb_GridCorr_R_0__1_ */
{0x0F12, 0x0000}, /* awbb_GridCorr_R_0__2_ */
{0x0F12, 0x0000}, /* awbb_GridCorr_R_0__3_ */
{0x0F12, 0x0020}, /* awbb_GridCorr_R_0__4_ */
{0x0F12, 0x0060}, /* awbb_GridCorr_R_0__5_ */

{0x0F12, 0xFFE0}, /* awbb_GridCorr_R_1__0_ */
{0x0F12, 0xFFE0}, /* D8 awbb_GridCorr_R_1__1_ */
{0x0F12, 0x0000}, /* awbb_GridCorr_R_1__2_ */
{0x0F12, 0x0000}, /* awbb_GridCorr_R_1__3_ */
{0x0F12, 0x0020}, /* awbb_GridCorr_R_1__4_ */
{0x0F12, 0x0060}, /* awbb_GridCorr_R_1__5_ */

{0x0F12, 0xFFE0}, /* awbb_GridCorr_R_2__0_ */
{0x0F12, 0xFFE0}, /* D8 awbb_GridCorr_R_2__1_ */
{0x0F12, 0x0000}, /* awbb_GridCorr_R_2__2_ */
{0x0F12, 0x0000}, /* awbb_GridCorr_R_2__3_ */
{0x0F12, 0x0020}, /* awbb_GridCorr_R_2__4_ */
{0x0F12, 0x0060}, /* awbb_GridCorr_R_2__5_ */

{0x0F12, 0x0004}, /* 08 awbb_GridCorr_B_0__0_ */
{0x0F12, 0x0000}, /* awbb_GridCorr_B_0__1_ */
{0x0F12, 0x0000}, /* awbb_GridCorr_B_0__2_ */
{0x0F12, 0xFFC0}, /* awbb_GridCorr_B_0__3_ */
{0x0F12, 0xFFB0}, /* awbb_GridCorr_B_0__4_ */
{0x0F12, 0xFF30}, /* awbb_GridCorr_B_0__5_ */

{0x0F12, 0x0004}, /* 08 awbb_GridCorr_B_1__0_ */
{0x0F12, 0x0000}, /* awbb_GridCorr_B_1__1_ */
{0x0F12, 0x0000}, /* awbb_GridCorr_B_1__2_ */
{0x0F12, 0xFFC0}, /* awbb_GridCorr_B_1__3_ */
{0x0F12, 0xFFB0}, /* awbb_GridCorr_B_1__4_ */
{0x0F12, 0xFF30}, /* awbb_GridCorr_B_1__5_ */

{0x0F12, 0x0004}, /* 08 awbb_GridCorr_B_2__0_ */
{0x0F12, 0x0000}, /* awbb_GridCorr_B_2__1_ */
{0x0F12, 0x0000}, /* awbb_GridCorr_B_2__2_ */
{0x0F12, 0xFFC0}, /* awbb_GridCorr_B_2__3_ */
{0x0F12, 0xFFB0}, /* awbb_GridCorr_B_2__4_ */
{0x0F12, 0xFF30}, /* awbb_GridCorr_B_2__5_ */

{0x0F12, 0x02C6},
{0x0F12, 0x0335},
{0x0F12, 0x03B3},
{0x0F12, 0x1021},
{0x0F12, 0x107E},
{0x0F12, 0x113E},
{0x0F12, 0x117C},
{0x0F12, 0x11C2},
{0x0F12, 0x120B},

{0x0F12, 0x00B3},
{0x0F12, 0x00B7},
{0x0F12, 0x00D3},
{0x0F12, 0x0091},

/* White Locus */
{0x002A, 0x0D66},
{0x0F12, 0x0133},
{0x0F12, 0x010F},
{0x002A, 0x0D74},
{0x0F12, 0x052A},

/* Gamut Thresholds */
{0x002A, 0x0DAE},
{0x0F12, 0x0036},
{0x0F12, 0x001C},
{0x002A, 0x0DAA},
{0x0F12, 0x071A},
{0x0F12, 0x03A4},

/* SceneDetection Thresholds */
{0x002A, 0x0D92},
{0x0F12, 0x0BB8},
{0x0F12, 0x0096},
{0x002A, 0x0E86},
{0x0F12, 0x0216},
{0x0F12, 0x029F},
{0x002A, 0x0D96},
{0x0F12, 0x0BB7},
{0x0F12, 0x0096},
{0x002A, 0x0DB2},
{0x0F12, 0x00DA},
{0x002A, 0x0D9A},
{0x0F12, 0x000A},
{0x002A, 0x0DB4},
{0x0F12, 0x0459},
{0x002A, 0x0DA4},
{0x0F12, 0x000E},
{0x002A, 0x0D64},
{0x0F12, 0x0032},
{0x002A, 0x0DA6},
{0x0F12, 0x001E},
{0x002A, 0x0D9C},
{0x0F12, 0x001B},
{0x0F12, 0x000E},
{0x0F12, 0x0008},
{0x0F12, 0x0004},

/* AWB Debug.(Outdoor Pink) */
{0x002A, 0x0E30},
{0x0F12, 0x0000},
{0x002A, 0x0E84},
{0x0F12, 0x0000},

/* UseInvalidOutdoor option */
{0x002A, 0x0D88},
{0x0F12, 0x0001},

/* AWB input Y-Filter setting */
{0x002A, 0x0C48},
{0x0F12, 0x0020},
{0x002A, 0x0C50},
{0x0F12, 0x00E0},
{0x002A, 0x0CC4},
{0x0F12, 0x0020},
{0x002A, 0x0CCC},
{0x0F12, 0x00C0},


{0x002A, 0x0DC2},
{0x0F12, 0x0030},
{0x0F12, 0x00C8},
{0x0F12, 0x012C},
{0x0F12, 0x0258},
{0x0F12, 0x0003},


/* Set CCM */
/* CCM Start Address */
{0x002A, 0x06D0},
{0x0F12, 0x2800},
{0x0F12, 0x7000},
{0x0F12, 0x2824},
{0x0F12, 0x7000},
{0x0F12, 0x2848},
{0x0F12, 0x7000},
{0x0F12, 0x286C},
{0x0F12, 0x7000},
{0x0F12, 0x2890},
{0x0F12, 0x7000},
{0x0F12, 0x28B4},
{0x0F12, 0x7000},
{0x002A, 0x06EC},
{0x0F12, 0x28D8},
{0x0F12, 0x7000},

/* CCM */
{0x002A, 0x2800},
{0x0F12, 0x01E1},
{0x0F12, 0xFFC4},
{0x0F12, 0xFFF8},
{0x0F12, 0x0101},
{0x0F12, 0x014C},
{0x0F12, 0xFF55},
{0x0F12, 0xFF5B},
{0x0F12, 0x0205},
{0x0F12, 0xFF17},
{0x0F12, 0xFEFE},
{0x0F12, 0x01B6},
{0x0F12, 0x0107},
{0x0F12, 0xFFDB},
{0x0F12, 0xFFDB},
{0x0F12, 0x01D1},
{0x0F12, 0x0163},
{0x0F12, 0xFF9E},
{0x0F12, 0x01B3},

{0x0F12, 0x01E1},
{0x0F12, 0xFFC4},
{0x0F12, 0xFFF8},
{0x0F12, 0x0101},
{0x0F12, 0x014C},
{0x0F12, 0xFF55},
{0x0F12, 0xFF5B},
{0x0F12, 0x0205},
{0x0F12, 0xFF17},
{0x0F12, 0xFEFE},
{0x0F12, 0x01B6},
{0x0F12, 0x0107},
{0x0F12, 0xFFDB},
{0x0F12, 0xFFDB},
{0x0F12, 0x01D1},
{0x0F12, 0x0163},
{0x0F12, 0xFF9E},
{0x0F12, 0x01B3},

{0x0F12, 0x01E1},
{0x0F12, 0xFFC4},
{0x0F12, 0xFFF8},
{0x0F12, 0x0101},
{0x0F12, 0x014C},
{0x0F12, 0xFF55},
{0x0F12, 0xFF5B},
{0x0F12, 0x0205},
{0x0F12, 0xFF17},
{0x0F12, 0xFEFE},
{0x0F12, 0x01B6},
{0x0F12, 0x0107},
{0x0F12, 0xFFDB},
{0x0F12, 0xFFDB},
{0x0F12, 0x01D1},
{0x0F12, 0x0163},
{0x0F12, 0xFF9E},
{0x0F12, 0x01B3},

{0x0F12, 0x01FB},
{0x0F12, 0xFFA9},
{0x0F12, 0xFFEA},
{0x0F12, 0x013C},
{0x0F12, 0x0140},
{0x0F12, 0xFF53},
{0x0F12, 0xFE7A},
{0x0F12, 0x017D},
{0x0F12, 0xFEED},
{0x0F12, 0xFF39},
{0x0F12, 0x01D6},
{0x0F12, 0x00C4},
{0x0F12, 0xFFC0},
{0x0F12, 0xFFBF},
{0x0F12, 0x01CD},
{0x0F12, 0x0182},
{0x0F12, 0xFF91},
{0x0F12, 0x01AA},

{0x0F12, 0x01C5},
{0x0F12, 0xFF9F},
{0x0F12, 0xFFE5},
{0x0F12, 0x00E2},
{0x0F12, 0x010E},
{0x0F12, 0xFF62},
{0x0F12, 0xFF03},
{0x0F12, 0x01D0},
{0x0F12, 0xFF3E},
{0x0F12, 0xFF00},
{0x0F12, 0x01A6},
{0x0F12, 0x00BB},
{0x0F12, 0xFFBF},
{0x0F12, 0xFFDD},
{0x0F12, 0x01F6},
{0x0F12, 0x00CB},
{0x0F12, 0xFF94},
{0x0F12, 0x019E},

{0x0F12, 0x01D2},
{0x0F12, 0xFFC2},
{0x0F12, 0xFFFC},
{0x0F12, 0x00E8},
{0x0F12, 0x0126},
{0x0F12, 0xFF83},
{0x0F12, 0xFE7A},
{0x0F12, 0x017D},
{0x0F12, 0xFEED},
{0x0F12, 0xFF8A},
{0x0F12, 0x01F9},
{0x0F12, 0x005B},
{0x0F12, 0xFFCA},
{0x0F12, 0xFFA3},
{0x0F12, 0x01DA},
{0x0F12, 0x0108},
{0x0F12, 0xFFB3},
{0x0F12, 0x01DD},

{0x0F12, 0x01D2},
{0x0F12, 0xFFC2},
{0x0F12, 0xFFFC},
{0x0F12, 0x00F4},
{0x0F12, 0x0139},
{0x0F12, 0xFF64},
{0x0F12, 0xFEEC},
{0x0F12, 0x01FD},
{0x0F12, 0xFF8E},
{0x0F12, 0xFEF4},
{0x0F12, 0x01BD},
{0x0F12, 0x010A},
{0x0F12, 0xFFA2},
{0x0F12, 0xFFDE},
{0x0F12, 0x0208},
{0x0F12, 0x0163},
{0x0F12, 0xFF9E},
{0x0F12, 0x01B3},

/* Set NB */
{0x002A, 0x07EA},
{0x0F12, 0x0000}, /*afit_bUseNoiseInd 0 : NB 1: Noise Index */

/* param_start	SARR_uNormBrInDoor */
{0x0F12, 0x000A}, /* SARR_uNormBrInDoor[0] */
{0x0F12, 0x0019}, /* SARR_uNormBrInDoor[1] */
{0x0F12, 0x007D}, /* SARR_uNormBrInDoor[2] */
{0x0F12, 0x02BC}, /* SARR_uNormBrInDoor[3] */
{0x0F12, 0x07D0}, /* SARR_uNormBrInDoor[4] */

/* param_start	SARR_uNormBrOutDoor */
{0x0F12, 0x000A}, /* SARR_uNormBrOutDoor[0] */
{0x0F12, 0x0019}, /* SARR_uNormBrOutDoor[1] */
{0x0F12, 0x007D}, /* SARR_uNormBrOutDoor[2] */
{0x0F12, 0x02BC}, /* SARR_uNormBrOutDoor[3] */
{0x0F12, 0x07D0}, /* SARR_uNormBrOutDoor[4] */

/* Set AFIT */
{0x002A, 0x0814},
{0x0F12, 0x082C},
{0x0F12, 0x7000},

{0x002A, 0x082C},
{0x0F12, 0x0000}, /* BRIGHTNESS                                                         */
{0x0F12, 0x0000}, /* CONTRAST                                                           */
{0x0F12, 0x0010}, /* SATURATION                                                         */
{0x0F12, 0xFFE2}, /* SHARP_BLUR                                                         */
{0x0F12, 0x0000}, /* GLAMOUR                                                            */
{0x0F12, 0x03FF}, /* Disparity_iSatSat                                                  */
{0x0F12, 0x03FF}, /* Denoise1_iYDenThreshLow                                            */
{0x0F12, 0x0028}, /* Denoise1_iYDenThreshLow_Bin                                        */
{0x0F12, 0x03FF}, /* Denoise1_iYDenThreshHigh                                           */
{0x0F12, 0x00FF}, /* Denoise1_iYDenThreshHigh_Bin                                       */
{0x0F12, 0x0002}, /* Denoise1_iLowWWideThresh                                           */
{0x0F12, 0x000A}, /* Denoise1_iHighWWideThresh                                          */
{0x0F12, 0x000A}, /* Denoise1_iLowWideThresh                                            */
{0x0F12, 0x000A}, /* Denoise1_iHighWideThresh                                           */
{0x0F12, 0x03FF}, /* Denoise1_iSatSat                                                   */
{0x0F12, 0x03FF}, /* Demosaic4_iHystGrayLow                                             */
{0x0F12, 0x0000}, /* Demosaic4_iHystGrayHigh                                            */
{0x0F12, 0x0344}, /* UVDenoise_iYLowThresh                                              */
{0x0F12, 0x033A}, /* UVDenoise_iYHighThresh                                             */
{0x0F12, 0x03FF}, /* UVDenoise_iUVLowThresh                                             */
{0x0F12, 0x03FF}, /* UVDenoise_iUVHighThresh                                            */
{0x0F12, 0x0028}, /* DSMix1_iLowLimit_Wide                                              */
{0x0F12, 0x0032}, /* DSMix1_iLowLimit_Wide_Bin                                          */
{0x0F12, 0x0014}, /* DSMix1_iHighLimit_Wide                                             */
{0x0F12, 0x0032}, /* DSMix1_iHighLimit_Wide_Bin                                         */
{0x0F12, 0x0050}, /* DSMix1_iLowLimit_Fine                                              */
{0x0F12, 0x0032}, /* DSMix1_iLowLimit_Fine_Bin                                          */
{0x0F12, 0x0046}, /* DSMix1_iHighLimit_Fine                                             */
{0x0F12, 0x0032}, /* DSMix1_iHighLimit_Fine_Bin                                         */
{0x0F12, 0x0106}, /* DSMix1_iRGBOffset                                                  */
{0x0F12, 0x006F}, /* DSMix1_iDemClamp                                                   */
{0x0F12, 0x0C0F}, /* "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"                   */
{0x0F12, 0x0C0F}, /* "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"                */
{0x0F12, 0x0303}, /* "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin" */
{0x0F12, 0x0303}, /* Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin     */
{0x0F12, 0x140A}, /* "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"                */
{0x0F12, 0x140A}, /* "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"                  */
{0x0F12, 0x2828}, /* "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"        */
{0x0F12, 0x0606}, /* "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"                */
{0x0F12, 0x023F}, /* "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"                 */
{0x0F12, 0x0480}, /* "Denoise1_iRadialLimitDenoise1_iLWBNoise"                          */
{0x0F12, 0x000F}, /* "Denoise1_iWideDenoise1_iWideWide"                                 */
{0x0F12, 0x030A}, /* "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"                      */
{0x0F12, 0x0003}, /* "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"                        */
{0x0F12, 0x0011}, /* "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"                  */
{0x0F12, 0x0A0F}, /* "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"                 */
{0x0F12, 0x050A}, /* "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"                */
{0x0F12, 0x0900}, /* "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"                   */
{0x0F12, 0x0000}, /* "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"                 */
{0x0F12, 0x980A}, /* "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"                    */
{0x0F12, 0x0005}, /* "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"                  */
{0x0F12, 0x0000}, /* "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"                 */
{0x0F12, 0x0000}, /* "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"         */
{0x0F12, 0x0000}, /* "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"               */
{0x0F12, 0x0000}, /* "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"                 */
{0x0F12, 0x0A00}, /* "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"                 */
{0x0F12, 0x000A}, /* "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"                 */
{0x0F12, 0x0180}, /* "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"                       */
{0x0F12, 0x0180}, /* "RGBGamma2_iLinearityRGBGamma2_bLinearity"                         */
{0x0F12, 0x0100}, /* "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"                       */
{0x0F12, 0x6E14}, /* "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"                           */
{0x0F12, 0x0180}, /* "RGB2YUV_iSaturationRGB2YUV_bGainOffset"                           */
{0x0F12, 0x0008}, /*  RGB2YUV_iYOffset                                                  */

{0x0F12, 0x0000}, /* BRIGHTNESS                                                         */
{0x0F12, 0x0000}, /* CONTRAST                                                           */
{0x0F12, 0x0000}, /* SATURATION                                                         */
{0x0F12, 0x0000}, /* SHARP_BLUR                                                         */
{0x0F12, 0x0000}, /* GLAMOUR                                                            */
{0x0F12, 0x03FF}, /* Disparity_iSatSat                                                  */
{0x0F12, 0x0014}, /* Denoise1_iYDenThreshLow                                            */
{0x0F12, 0x000E}, /* Denoise1_iYDenThreshLow_Bin                                        */
{0x0F12, 0x0064}, /* Denoise1_iYDenThreshHigh                                           */
{0x0F12, 0x00FF}, /* Denoise1_iYDenThreshHigh_Bin                                       */
{0x0F12, 0x0002}, /* Denoise1_iLowWWideThresh                                           */
{0x0F12, 0x000A}, /* Denoise1_iHighWWideThresh                                          */
{0x0F12, 0x000A}, /* Denoise1_iLowWideThresh                                            */
{0x0F12, 0x000A}, /* Denoise1_iHighWideThresh                                           */
{0x0F12, 0x03FF}, /* Denoise1_iSatSat                                                   */
{0x0F12, 0x03FF}, /* Demosaic4_iHystGrayLow                                             */
{0x0F12, 0x0000}, /* Demosaic4_iHystGrayHigh                                            */
{0x0F12, 0x0114}, /* UVDenoise_iYLowThresh                                              */
{0x0F12, 0x020A}, /* UVDenoise_iYHighThresh                                             */
{0x0F12, 0x03FF}, /* UVDenoise_iUVLowThresh                                             */
{0x0F12, 0x03FF}, /* UVDenoise_iUVHighThresh                                            */
{0x0F12, 0x0028}, /* DSMix1_iLowLimit_Wide                                              */
{0x0F12, 0x0000}, /* DSMix1_iLowLimit_Wide_Bin                                          */
{0x0F12, 0x0014}, /* DSMix1_iHighLimit_Wide                                             */
{0x0F12, 0x0032}, /* DSMix1_iHighLimit_Wide_Bin                                         */
{0x0F12, 0x0050}, /* DSMix1_iLowLimit_Fine                                              */
{0x0F12, 0x0032}, /* DSMix1_iLowLimit_Fine_Bin                                          */
{0x0F12, 0x0046}, /* DSMix1_iHighLimit_Fine                                             */
{0x0F12, 0x0032}, /* DSMix1_iHighLimit_Fine_Bin                                         */
{0x0F12, 0x0106}, /* DSMix1_iRGBOffset                                                  */
{0x0F12, 0x006F}, /* DSMix1_iDemClamp                                                   */
{0x0F12, 0x050F}, /* "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"                   */
{0x0F12, 0x0A0F}, /* "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"                */
{0x0F12, 0x0203}, /* "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin" */
{0x0F12, 0x0203}, /* Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin     */
{0x0F12, 0x140A}, /* "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"                */
{0x0F12, 0x140A}, /* "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"                  */
{0x0F12, 0x2828}, /* "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"        */
{0x0F12, 0x0606}, /* "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"                */
{0x0F12, 0x020A}, /* "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"                 */
{0x0F12, 0x0480}, /* "Denoise1_iRadialLimitDenoise1_iLWBNoise"                          */
{0x0F12, 0x000F}, /* "Denoise1_iWideDenoise1_iWideWide"                                 */
{0x0F12, 0x0305}, /* "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"                      */
{0x0F12, 0x2803}, /* "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"                        */
{0x0F12, 0x2811}, /* "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"                  */
{0x0F12, 0x0A0F}, /* "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"                 */
{0x0F12, 0x050A}, /* "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"                */
{0x0F12, 0x101E}, /* "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"                   */
{0x0F12, 0x101E}, /* "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"                 */
{0x0F12, 0x980A}, /* "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"                    */
{0x0F12, 0x0005}, /* "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"                  */
{0x0F12, 0x0400}, /* "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"                 */
{0x0F12, 0x0400}, /* "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"         */
{0x0F12, 0x0000}, /* "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"               */
{0x0F12, 0x0000}, /* "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"                 */
{0x0F12, 0x0A00}, /* "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"                 */
{0x0F12, 0x100A}, /* "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"                 */
{0x0F12, 0x0180}, /* "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"                       */
{0x0F12, 0x0180}, /* "RGBGamma2_iLinearityRGBGamma2_bLinearity"                         */
{0x0F12, 0x0100}, /* "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"                       */
{0x0F12, 0x8030}, /* "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"                           */
{0x0F12, 0x0180}, /* "RGB2YUV_iSaturationRGB2YUV_bGainOffset"                           */
{0x0F12, 0x0000}, /*  RGB2YUV_iYOffset                                                  */

{0x0F12, 0x0000}, /* BRIGHTNESS                                                         */
{0x0F12, 0x0000}, /* CONTRAST                                                           */
{0x0F12, 0x0000}, /* SATURATION                                                         */
{0x0F12, 0x0000}, /* SHARP_BLUR                                                         */
{0x0F12, 0x0000}, /* GLAMOUR                                                            */
{0x0F12, 0x03FF}, /* Disparity_iSatSat                                                  */
{0x0F12, 0x000C}, /* Denoise1_iYDenThreshLow                                            */
{0x0F12, 0x0006}, /* Denoise1_iYDenThreshLow_Bin                                        */
{0x0F12, 0x0060}, /* Denoise1_iYDenThreshHigh                                           */
{0x0F12, 0x0050}, /* Denoise1_iYDenThreshHigh_Bin                                       */
{0x0F12, 0x0002}, /* Denoise1_iLowWWideThresh                                           */
{0x0F12, 0x000A}, /* Denoise1_iHighWWideThresh                                          */
{0x0F12, 0x000A}, /* Denoise1_iLowWideThresh                                            */
{0x0F12, 0x000A}, /* Denoise1_iHighWideThresh                                           */
{0x0F12, 0x03FF}, /* Denoise1_iSatSat                                                   */
{0x0F12, 0x03FF}, /* Demosaic4_iHystGrayLow                                             */
{0x0F12, 0x0000}, /* Demosaic4_iHystGrayHigh                                            */
{0x0F12, 0x0014}, /* UVDenoise_iYLowThresh                                              */
{0x0F12, 0x000A}, /* UVDenoise_iYHighThresh                                             */
{0x0F12, 0x03FF}, /* UVDenoise_iUVLowThresh                                             */
{0x0F12, 0x03FF}, /* UVDenoise_iUVHighThresh                                            */
{0x0F12, 0x0028}, /* DSMix1_iLowLimit_Wide                                              */
{0x0F12, 0x0032}, /* DSMix1_iLowLimit_Wide_Bin                                          */
{0x0F12, 0x0014}, /* DSMix1_iHighLimit_Wide                                             */
{0x0F12, 0x0032}, /* DSMix1_iHighLimit_Wide_Bin                                         */
{0x0F12, 0x0050}, /* DSMix1_iLowLimit_Fine                                              */
{0x0F12, 0x0032}, /* DSMix1_iLowLimit_Fine_Bin                                          */
{0x0F12, 0x0010}, /* DSMix1_iHighLimit_Fine                                             */
{0x0F12, 0x0032}, /* DSMix1_iHighLimit_Fine_Bin                                         */
{0x0F12, 0x0106}, /* DSMix1_iRGBOffset                                                  */
{0x0F12, 0x006F}, /* DSMix1_iDemClamp                                                   */
{0x0F12, 0x0202}, /* "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"                   */
{0x0F12, 0x0502}, /* "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"                */
{0x0F12, 0x0102}, /* "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin" */
{0x0F12, 0x0102}, /* Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin     */
{0x0F12, 0x140A}, /* "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"                */
{0x0F12, 0x140A}, /* "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"                  */
{0x0F12, 0x2828}, /* "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"        */
{0x0F12, 0x0606}, /* "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"                */
{0x0F12, 0x0205}, /* "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"                 */
{0x0F12, 0x0480}, /* "Denoise1_iRadialLimitDenoise1_iLWBNoise"                          */
{0x0F12, 0x000F}, /* "Denoise1_iWideDenoise1_iWideWide"                                 */
{0x0F12, 0x0005}, /* "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"                      */
{0x0F12, 0x2803}, /* "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"                        */
{0x0F12, 0x2811}, /* "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"                  */
{0x0F12, 0x0A0F}, /* "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"                 */
{0x0F12, 0x050A}, /* "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"                */
{0x0F12, 0x2020}, /* "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"                   */
{0x0F12, 0x2020}, /* "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"                 */
{0x0F12, 0x980A}, /* "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"                    */
{0x0F12, 0x0007}, /* "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"                  */
{0x0F12, 0x0403}, /* "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"                 */
{0x0F12, 0x0402}, /* "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"         */
{0x0F12, 0x0000}, /* "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"               */
{0x0F12, 0x0203}, /* "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"                 */
{0x0F12, 0x0000}, /* "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"                 */
{0x0F12, 0x1006}, /* "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"                 */
{0x0F12, 0x0180}, /* "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"                       */
{0x0F12, 0x0180}, /* "RGBGamma2_iLinearityRGBGamma2_bLinearity"                         */
{0x0F12, 0x0100}, /* "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"                       */
{0x0F12, 0x803C}, /* "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"                           */
{0x0F12, 0x0180}, /* "RGB2YUV_iSaturationRGB2YUV_bGainOffset"                           */
{0x0F12, 0x0000}, /*  RGB2YUV_iYOffset                                                  */

{0x0F12, 0x0000}, /* BRIGHTNESS                                                         */
{0x0F12, 0x0000}, /* CONTRAST                                                           */
{0x0F12, 0x0000}, /* SATURATION                                                         */
{0x0F12, 0x0000}, /* SHARP_BLUR                                                         */
{0x0F12, 0x0000}, /* GLAMOUR                                                            */
{0x0F12, 0x03FF}, /* Disparity_iSatSat                                                  */
{0x0F12, 0x0006}, /* Denoise1_iYDenThreshLow                                            */
{0x0F12, 0x0006}, /* Denoise1_iYDenThreshLow_Bin                                        */
{0x0F12, 0x005A}, /* Denoise1_iYDenThreshHigh                                           */
{0x0F12, 0x0050}, /* Denoise1_iYDenThreshHigh_Bin                                       */
{0x0F12, 0x0002}, /* Denoise1_iLowWWideThresh                                           */
{0x0F12, 0x000A}, /* Denoise1_iHighWWideThresh                                          */
{0x0F12, 0x000A}, /* Denoise1_iLowWideThresh                                            */
{0x0F12, 0x000A}, /* Denoise1_iHighWideThresh                                           */
{0x0F12, 0x03FF}, /* Denoise1_iSatSat                                                   */
{0x0F12, 0x03FF}, /* Demosaic4_iHystGrayLow                                             */
{0x0F12, 0x0000}, /* Demosaic4_iHystGrayHigh                                            */
{0x0F12, 0x0014}, /* UVDenoise_iYLowThresh                                              */
{0x0F12, 0x000A}, /* UVDenoise_iYHighThresh                                             */
{0x0F12, 0x03FF}, /* UVDenoise_iUVLowThresh                                             */
{0x0F12, 0x03FF}, /* UVDenoise_iUVHighThresh                                            */
{0x0F12, 0x0028}, /* DSMix1_iLowLimit_Wide                                              */
{0x0F12, 0x0032}, /* DSMix1_iLowLimit_Wide_Bin                                          */
{0x0F12, 0x0014}, /* DSMix1_iHighLimit_Wide                                             */
{0x0F12, 0x0032}, /* DSMix1_iHighLimit_Wide_Bin                                         */
{0x0F12, 0x0050}, /* DSMix1_iLowLimit_Fine                                              */
{0x0F12, 0x0032}, /* DSMix1_iLowLimit_Fine_Bin                                          */
{0x0F12, 0x0010}, /* DSMix1_iHighLimit_Fine                                             */
{0x0F12, 0x0032}, /* DSMix1_iHighLimit_Fine_Bin                                         */
{0x0F12, 0x0106}, /* DSMix1_iRGBOffset                                                  */
{0x0F12, 0x006F}, /* DSMix1_iDemClamp                                                   */
{0x0F12, 0x0202}, /* "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"                   */
{0x0F12, 0x0502}, /* "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"                */
{0x0F12, 0x0102}, /* "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin" */
{0x0F12, 0x0102}, /* Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin     */
{0x0F12, 0x140A}, /* "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"                */
{0x0F12, 0x140A}, /* "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"                  */
{0x0F12, 0x2828}, /* "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"        */
{0x0F12, 0x0606}, /* "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"                */
{0x0F12, 0x0205}, /* "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"                 */
{0x0F12, 0x0480}, /* "Denoise1_iRadialLimitDenoise1_iLWBNoise"                          */
{0x0F12, 0x000F}, /* "Denoise1_iWideDenoise1_iWideWide"                                 */
{0x0F12, 0x0005}, /* "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"                      */
{0x0F12, 0x2803}, /* "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"                        */
{0x0F12, 0x2811}, /* "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"                  */
{0x0F12, 0x0A0F}, /* "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"                 */
{0x0F12, 0x050A}, /* "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"                */
{0x0F12, 0x2020}, /* "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"                   */
{0x0F12, 0x2020}, /* "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"                 */
{0x0F12, 0x980A}, /* "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"                    */
{0x0F12, 0x0007}, /* "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"                  */
{0x0F12, 0x0403}, /* "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"                 */
{0x0F12, 0x0402}, /* "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"         */
{0x0F12, 0x0000}, /* "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"               */
{0x0F12, 0x0203}, /* "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"                 */
{0x0F12, 0x0000}, /* "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"                 */
{0x0F12, 0x1006}, /* "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"                 */
{0x0F12, 0x0180}, /* "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"                       */
{0x0F12, 0x0180}, /* "RGBGamma2_iLinearityRGBGamma2_bLinearity"                         */
{0x0F12, 0x0100}, /* "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"                       */
{0x0F12, 0x803C}, /* "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"                           */
{0x0F12, 0x0180}, /* "RGB2YUV_iSaturationRGB2YUV_bGainOffset"                           */
{0x0F12, 0x0000}, /*  RGB2YUV_iYOffset                                                  */

{0x0F12, 0x0000}, /* BRIGHTNESS                                                         */
{0x0F12, 0x000A}, /* CONTRAST                                                           */
{0x0F12, 0x0000}, /* SATURATION                                                         */
{0x0F12, 0x0000}, /* SHARP_BLUR                                                         */
{0x0F12, 0x0000}, /* GLAMOUR                                                            */
{0x0F12, 0x03FF}, /* Disparity_iSatSat                                                  */
{0x0F12, 0x0006}, /* Denoise1_iYDenThreshLow                                            */
{0x0F12, 0x0006}, /* Denoise1_iYDenThreshLow_Bin                                        */
{0x0F12, 0x0050}, /* Denoise1_iYDenThreshHigh                                           */
{0x0F12, 0x0050}, /* Denoise1_iYDenThreshHigh_Bin                                       */
{0x0F12, 0x0002}, /* Denoise1_iLowWWideThresh                                           */
{0x0F12, 0x000A}, /* Denoise1_iHighWWideThresh                                          */
{0x0F12, 0x000A}, /* Denoise1_iLowWideThresh                                            */
{0x0F12, 0x000A}, /* Denoise1_iHighWideThresh                                           */
{0x0F12, 0x03FF}, /* Denoise1_iSatSat                                                   */
{0x0F12, 0x03FF}, /* Demosaic4_iHystGrayLow                                             */
{0x0F12, 0x0000}, /* Demosaic4_iHystGrayHigh                                            */
{0x0F12, 0x0000}, /* UVDenoise_iYLowThresh                                              */
{0x0F12, 0x0000}, /* UVDenoise_iYHighThresh                                             */
{0x0F12, 0x03FF}, /* UVDenoise_iUVLowThresh                                             */
{0x0F12, 0x03FF}, /* UVDenoise_iUVHighThresh                                            */
{0x0F12, 0x0028}, /* DSMix1_iLowLimit_Wide                                              */
{0x0F12, 0x0032}, /* DSMix1_iLowLimit_Wide_Bin                                          */
{0x0F12, 0x0000}, /* DSMix1_iHighLimit_Wide                                             */
{0x0F12, 0x0032}, /* DSMix1_iHighLimit_Wide_Bin                                         */
{0x0F12, 0x0030}, /* DSMix1_iLowLimit_Fine                                              */
{0x0F12, 0x0032}, /* DSMix1_iLowLimit_Fine_Bin                                          */
{0x0F12, 0x0000}, /* DSMix1_iHighLimit_Fine                                             */
{0x0F12, 0x0032}, /* DSMix1_iHighLimit_Fine_Bin                                         */
{0x0F12, 0x0106}, /* DSMix1_iRGBOffset                                                  */
{0x0F12, 0x006F}, /* DSMix1_iDemClamp                                                   */
{0x0F12, 0x0202}, /* "Disparity_iDispTH_LowDisparity_iDispTH_Low_Bin"                   */
{0x0F12, 0x0502}, /* "Disparity_iDispTH_High Disparity_iDispTH_High_Bin"                */
{0x0F12, 0x0102}, /* "Despeckle_iCorrectionLevelColdDespeckle_iCorrectionLevelCold_Bin" */
{0x0F12, 0x0102}, /* Despeckle_iCorrectionLevelHotDespeckle_iCorrectionLevelHot_Bin     */
{0x0F12, 0x140A}, /* "Despeckle_iColdThreshLowDespeckle_iColdThreshHigh"                */
{0x0F12, 0x140A}, /* "Despeckle_iHotThreshLowDespeckle_iHotThreshHigh"                  */
{0x0F12, 0x2828}, /* "Denoise1_iLowMaxSlopeAllowedDenoise1_iHighMaxSlopeAllowed"        */
{0x0F12, 0x0606}, /* "Denoise1_iLowSlopeThreshDenoise1_iHighSlopeThresh"                */
{0x0F12, 0x0205}, /* "Denoise1_iRadialPowerDenoise1_iRadialDivideShift"                 */
{0x0F12, 0x0880}, /* "Denoise1_iRadialLimitDenoise1_iLWBNoise"                          */
{0x0F12, 0x000F}, /* "Denoise1_iWideDenoise1_iWideWide"                                 */
{0x0F12, 0x0005}, /* "Demosaic4_iHystGrayRangeUVDenoise_iYSupport"                      */
{0x0F12, 0x2803}, /* "UVDenoise_iUVSupportDSMix1_iLowPower_Wide"                        */
{0x0F12, 0x2811}, /* "DSMix1_iLowPower_Wide_BinDSMix1_iHighPower_Wide"                  */
{0x0F12, 0x0A0F}, /* "DSMix1_iHighPower_Wide_BinDSMix1_iLowThresh_Wide"                 */
{0x0F12, 0x050A}, /* "DSMix1_iHighThresh_WideDSMix1_iReduceNegativeWide"                */
{0x0F12, 0x2020}, /* "DSMix1_iLowPower_FineDSMix1_iLowPower_Fine_Bin"                   */
{0x0F12, 0x2020}, /* "DSMix1_iHighPower_FineDSMix1_iHighPower_Fine_Bin"                 */
{0x0F12, 0x980A}, /* "DSMix1_iLowThresh_FineDSMix1_iHighThresh_Fine"                    */
{0x0F12, 0x0007}, /* "DSMix1_iReduceNegativeFineDSMix1_iRGBMultiplier"                  */
{0x0F12, 0x0408}, /* "Mixer1_iNLowNoisePowerMixer1_iNLowNoisePower_Bin"                 */
{0x0F12, 0x0406}, /* "Mixer1_iNVeryLowNoisePowerMixer1_iNVeryLowNoisePower_Bin"         */
{0x0F12, 0x0000}, /* "Mixer1_iNHighNoisePowerMixer1_iNHighNoisePower_Bin"               */
{0x0F12, 0x0608}, /* "Mixer1_iWLowNoisePowerMixer1_iWVeryLowNoisePower"                 */
{0x0F12, 0x0000}, /* "Mixer1_iWHighNoisePowerMixer1_iWLowNoiseCeilGain"                 */
{0x0F12, 0x1006}, /* "Mixer1_iWHighNoiseCeilGainMixer1_iWNoiseCeilGain"                 */
{0x0F12, 0x0180}, /* "CCM_Oscar_iSaturationCCM_Oscar_bSaturation"                       */
{0x0F12, 0x0180}, /* "RGBGamma2_iLinearityRGBGamma2_bLinearity"                         */
{0x0F12, 0x0100}, /* "RGBGamma2_iDarkReduceRGBGamma2_bDarkReduce"                       */
{0x0F12, 0x8050}, /* "byr_gas2_iShadingPowerRGB2YUV_iRGBGain"                           */
{0x0F12, 0x0180}, /* "RGB2YUV_iSaturationRGB2YUV_bGainOffset"                           */
{0x0F12, 0x0000}, /*  RGB2YUV_iYOffset                                                  */

{0x0F12, 0x00FF},
{0x0F12, 0x00FF},
{0x0F12, 0x0800},
{0x0F12, 0x0600},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0000},
{0x0F12, 0x0300},
{0x0F12, 0x0002},
{0x0F12, 0x0400},
{0x0F12, 0x0106},
{0x0F12, 0x0005},
{0x0F12, 0x0000},
{0x0F12, 0x0703},
{0x0F12, 0x0000},
{0x0F12, 0xFFD6},
{0x0F12, 0x53C1},
{0x0F12, 0xE1FE},
{0x0F12, 0x0001},

/* Update Changed Registers */
{0x002A, 0x03FC},
{0x0F12, 0x0001}, /* REG_TC_DBG_ReInitCmd */

{0x0028, 0xD000},
{0x002A, 0xB0CC},
{0x0F12, 0x000B}, /* Non contious mode */
/* Recording 25fps Anti-Flicker 50Hz END of Initial */

/* MIPI */
{0x0028, 0xD000},
{0x002A, 0xB0CC},
{0x0F12, 0x000B},

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_preview_640x480[] = {
{0xFCFC, 0xD000},
{0x0028, 0x7000},

{0x002A, 0x021C},
{0x0F12, 0x0000},	//REG_TC_GP_ActivePrevConfig
{0x002A, 0x0220},
{0x0F12, 0x0001},	//REG_TC_GP_PrevOpenAfterChange
{0x002A, 0x01F8},
{0x0F12, 0x0001},	//REG_TC_GP_NewConfigSync
{0x002A, 0x021E},
{0x0F12, 0x0001},	//REG_TC_GP_PrevConfigChanged
{0x002A, 0x01F0},
{0x0F12, 0x0001},	//REG_TC_GP_EnablePreview
{0x0F12, 0x0001},	//REG_TC_GP_EnablePreviewChanged
{S5K5BAFX_TABLE_WAIT_MS, 0x0096}, //150ms

// MIP, 0xI   }
{0x0028, 0xD000},
{0x002A, 0xB0CC},
{0x0F12, 0x000B},

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_preview_800x600[] = {
/* PREVIEW */
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x10EE},
{0x0F12, 0x0000},
{0x002A, 0x021C},
{0x0F12, 0x0000},
{0x002A, 0x0220},
{0x0F12, 0x0001},
{0x002A, 0x01F8},
{0x0F12, 0x0001},
{0x002A, 0x021E},
{0x0F12, 0x0001},
{0x002A, 0x01F0},
{0x0F12, 0x0001},
{0x0F12, 0x0001},
{S5K5BAFX_TABLE_WAIT_MS, 0x0064},

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_capture_1600x1200[] = {
{0xFCFC, 0xD000},
{0x0028, 0x7000},

{0x002A, 0x0224},
{0x0F12, 0x0000},	//REG_TC_GP_ActiveCapConfig
{0x002A, 0x01F8},
{0x0F12, 0x0001},	//REG_TC_GP_NewConfigSync
{0x002A, 0x0226},
{0x0F12, 0x0001},	//REG_TC_GP_CapConfigChanged
{0x002A, 0x01F4},
{0x0F12, 0x0001},	//REG_TC_GP_EnableCapture
{0x0F12, 0x0001},	//REG_TC_GP_EnableCaptureChanged
{S5K5BAFX_TABLE_WAIT_MS, 0x0096}, //150ms

// MIP, 0xI
{0x0028, 0xD000},
{0x002A, 0xB0CC},
{0x0F12, 0x000B},
};

static const struct s5k5bafx_reg mode_check_capture_staus[] = {
{0x002C, 0x7000},
{0x002E, 0x0142},

{0xFFFF, 0xFFFF},
};

#ifdef FACTORY_TEST
static struct s5k5bafx_reg mode_test_pattern[] = {
{S5K5BAFX_TABLE_WAIT_MS, 0x01f4}, //150ms

{0xfcfc, 0xd000},
{0x0028, 0xd000},
{0x002a, 0x4100},
{0x0f12, 0x08a3}, //gas bypass
{0x002a, 0x6600},
{0x0f12, 0x0001}, //ccm bypass
{0x002a, 0x6800},
{0x0f12, 0x0001}, //gamma bypass
{0x002a, 0x4400},
{0x0f12, 0x0001}, //awb bypass

{0x0028, 0x7000},
{0x002A, 0x03b6},
{0x0F12, 0x0001},
{0x002A, 0x03ba},
{0x0F12, 0x0001},	// LEI control

{0x0028, 0xD000},
{0x002A, 0x3118},
{0x0F12, 0x0320},	// Colorbar pattern x size
{0x0F12, 0x0258},	// Colorbar pattern y size
{0x0F12, 0x0000},
{0x002A, 0x3100},
{0x0F12, 0x0002},	// Colorbar pattern

{0xFFFF, 0xFFFF},
};

static struct s5k5bafx_reg mode_test_pattern_off[] = {
{0xfcfc, 0xd000},
{0x0028, 0xd000},
{0x002a, 0x4100},
{0x0f12, 0x08a2},
{0x002a, 0x6600},
{0x0f12, 0x0000},
{0x002a, 0x6800},
{0x0f12, 0x0000},
{0x002a, 0x4400},
{0x0f12, 0x0000},

{0x0028, 0x7000},
{0x002A, 0x03F8},
{0x0F12, 0x0079},

{S5K5BAFX_TABLE_WAIT_MS, 0x01F4},

{0x002A, 0x03F8},
{0x0F12, 0x007F},

{0x0028, 0xD000},
{0x002A, 0x3118},
{0x0F12, 0x0280},
{0x0F12, 0x01E0},
{0x0F12, 0x0000},
{0x002A, 0x3100},
{0x0F12, 0x0000},

{0xFFFF, 0xFFFF},
};
#endif


static const struct s5k5bafx_reg mode_coloreffect_none[] = {
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x01EE},
{0x0F12, 0x0000},

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_coloreffect_mono[] = {
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x01EE},
{0x0F12, 0x0001},

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_coloreffect_sepia[] = {
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x01EE},
{0x0F12, 0x0003},

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_coloreffect_negative[] = {
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x01EE},
{0x0F12, 0x0002},

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_WB_auto[] = {
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x1B66},
{0x0F12, 0x0001},

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_WB_daylight[] = {
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x1B66},
{0x0F12, 0x0000},

{0x002A, 0x03C6},
{0x0F12, 0x0620},
{0x0F12, 0x0001},
{0x0F12, 0x0400},
{0x0F12, 0x0001},
{0x0F12, 0x0540},
{0x0F12, 0x0001},

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_WB_incandescent[] = {
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x1B66},
{0x0F12, 0x0000},

{0x002A, 0x03C6},
{0x0F12, 0x03C0},
{0x0F12, 0x0001},
{0x0F12, 0x0400},
{0x0F12, 0x0001},
{0x0F12, 0x0980},
{0x0F12, 0x0001},

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_WB_fluorescent[] = {
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x1B66},
{0x0F12, 0x0000},

{0x002A, 0x03C6},
{0x0F12, 0x0560},
{0x0F12, 0x0001},
{0x0F12, 0x0400},
{0x0F12, 0x0001},
{0x0F12, 0x08A0},
{0x0F12, 0x0001},

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_exposure_p2p0[] = {
// Brightness +4
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x01E4},
{0x0F12, 0x0080}, 	//REG_TC_UserBrightness

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_exposure_p1p5[] = {
// Brightness +3
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x01E4},
{0x0F12, 0x0060}, 	//REG_TC_UserBrightness

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_exposure_p1p0[] = {
// Brightness +2
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x01E4},
{0x0F12, 0x0040}, 	//REG_TC_UserBrightness

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_exposure_p0p5[] = {
// Brightness +1
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x01E4},
{0x0F12, 0x0020}, 	//REG_TC_UserBrightness

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_exposure_0[] = {
// Brightness 0
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x01E4},
{0x0F12, 0x0000}, 	//REG_TC_UserBrightness

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_exposure_m0p5[] = {
// Brightness -1
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x01E4},
{0x0F12, 0xFFF2}, 	//REG_TC_UserBrightness

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_exposure_m1p0[] = {
// Brightness -2
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x01E4},
{0x0F12, 0xFFDC}, 	//REG_TC_UserBrightness

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_exposure_m1p5[] = {
// Brightness -3
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x01E4},
{0x0F12, 0xFFBC}, 	//REG_TC_UserBrightness

{0xFFFF, 0xFFFF},
};

static const struct s5k5bafx_reg mode_exposure_m2p0[] = {
// Brightness -4
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x01E4},
{0x0F12, 0xFF80}, 	//REG_TC_UserBrightness

{0xFFFF, 0xFFFF},
};

//==========================================================
//	CAMERA_VT_PRETTY_0 Default
//==========================================================
static const struct s5k5bafx_reg mode_pretty_0[] =
{
//0xffff000A,
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x04CC},

{0x0F12, 0x0000}, //0000, //0000, //0000, //0000, //0000, //0000, //0000, //0000,	//0000,
{0x0F12, 0x0002}, //0006, //0007, //0007, //0007, //0005, //0005, //0005, //0005, //09, //0a, //000D,
{0x0F12, 0x0008}, //000C, //000D, //000D, //000D, //000c, //000c, //000c, //0010, //13, //16, //001B,
{0x0F12, 0x0018}, //0014, //0013, //0013, //0013, //0012, //10, //0010, //1f, //001f, //0030, //35, //003C,
{0x0F12, 0x005A}, //0040, //0079, //0079, //0079, //0079, //0075, //81, //0081,
{0x0F12, 0x00DF}, //00A0, //00FE, //00FE, //00FE, //00FE, //00FE, //00FE,
{0x0F12, 0x013F}, //0106, //0159, //0159, //0159, //0159, //0159, //0159,
{0x0F12, 0x0186}, //0152, //01A1, //01A1, //01A1, //01A1, //01A1, //01A1,
{0x0F12, 0x01E6}, //01DE, //0210, //0210, //0210, //0210, //0210, //0210,
{0x0F12, 0x0236}, //0224, //0263, //0263, //0263, //0263, //0263, //0263,
{0x0F12, 0x02BA}, //0295, //02D5, //02D5, //02D5, //02D5, //02D5, //02D8,
{0x0F12, 0x032A}, //0302, //0330, //0330, //0330, //0330, //0330, //0338,
{0x0F12, 0x0385}, //0346, //0377, //0377, //0377, //78, //037A, //80, //0380, //0384,
{0x0F12, 0x03C2}, //0388, //03BE, //C0, //03B9, //03B9, //B3, //B4, //03B4, //B5, //03BD, //C0, //3BC,
{0x0F12, 0x03EA}, //03C5, //03F0, //03ED, //03ED, //EC, //03EB, //EA, //03E8, //03EF, //3E8,
{0x0F12, 0x03FF}, //03FF, //0400, //0400, //0400, //0400, //0400, //0400,

{0x0F12, 0x0000}, //0000, //0000, //0000, //0000, //0000, //0000, //0000, //0000, //0000,	//0000,	//0000,
{0x0F12, 0x0002}, //0006, //0007, //0007, //0007, //0005, //0005, //0005, //0005, //0005, //0009, //000a, //000D,
{0x0F12, 0x0008}, //000C, //000D, //000D, //000D, //000c, //000c, //000c, //000c, //0010, //0013, //0016, //001B,
{0x0F12, 0x0018}, //0014, //0013, //0013, //0012, //0012, //0010, //0010, //1f, //001f, //0030, //0035, //003C,
{0x0F12, 0x005A}, //0040, //0079, //0079, //0079, //0079, //0079, //0075, //81, //0081,
{0x0F12, 0x00DF}, //00A0, //00FE, //00FE, //00FE, //00FE, //00FE, //00FE, //00FE,
{0x0F12, 0x013F}, //0106, //0159, //0159, //0159, //0159, //0159, //0159, //0159,
{0x0F12, 0x0186}, //0152, //01A1, //01A1, //01A1, //01A1, //01A1, //01A1, //01A1,
{0x0F12, 0x01E6}, //01DE, //0210, //0210, //0210, //0210, //0210, //0210, //0210,
{0x0F12, 0x0236}, //0224, //0263, //0263, //0263, //0263, //0263, //0263, //0263,
{0x0F12, 0x02BA}, //0295, //02D5, //02D5, //02D5, //02D5, //02D5, //02D5, //02D8,
{0x0F12, 0x032A}, //0302, //0330, //0330, //0330, //0330, //0330, //0330, //0338,
{0x0F12, 0x0385}, //0346, //0377, //0377, //0378, //037A, //80, //0380, //0380, //0384,
{0x0F12, 0x03C2}, //0388, //03BE, //C0, //03B9, //03B4, //03B4, //03B5, //03BD, //C0, //3BC,
{0x0F12, 0x03EA}, //03C5, //03F0, //03ED, //03EC, //03EB, //03EA, //03E8, //03EF, //3E8,
{0x0F12, 0x03FF}, //03FF, //0400, //0400, //0400, //0400, //0400, //0400, //0400,

{0x0F12, 0x0000}, //0000, //0000, //0000, //0000, //0000, //0000, //0000, //0000, //0000,	//0000,	//0000,
{0x0F12, 0x0002}, //0006, //0007, //0007, //0007, //0005, //0005, //0005, //0005, //0005, //0009, //000a, //000D,
{0x0F12, 0x0008}, //000C, //000D, //000D, //000D, //000c, //000c, //000c, //000c, //0010, //0013, //0016, //001B,
{0x0F12, 0x0018}, //0014, //0013, //0013, //0012, //0012, //0010, //0010, //1f, //001f, //0030, //0035, //003C,
{0x0F12, 0x005A}, //0040, //0079, //0079, //0079, //0079, //0079, //0075, //81, //0081,
{0x0F12, 0x00DF}, //00A0, //00FE, //00FE, //00FE, //00FE, //00FE, //00FE, //00FE,
{0x0F12, 0x013F}, //0106, //0159, //0159, //0159, //0159, //0159, //0159, //0159,
{0x0F12, 0x0186}, //0152, //01A1, //01A1, //01A1, //01A1, //01A1, //01A1, //01A1,
{0x0F12, 0x01E6}, //01DE, //0210, //0210, //0210, //0210, //0210, //0210, //0210,
{0x0F12, 0x0236}, //0224, //0263, //0263, //0263, //0263, //0263, //0263, //0263,
{0x0F12, 0x02BA}, //0295, //02D5, //02D5, //02D5, //02D5, //02D5, //02D5, //02D8,
{0x0F12, 0x032A}, //0302, //0330, //0330, //0330, //0330, //0330, //0330, //0338,
{0x0F12, 0x0385}, //0346, //0377, //0377, //0378, //037A, //80, //0380, //0380, //0384,
{0x0F12, 0x03C2}, //0388, //03BE, //C0, //03B9, //03B4, //03B4, //03B5, //03BD, //C0, //3BC,
{0x0F12, 0x03EA}, //03C5, //03F0, //03ED, //03EC, //03EB, //03EA, //03E8, //03EF, //3E8,
{0x0F12, 0x03FF}, //03FF, //0400, //0400, //0400, //0400, //0400, //0400, //0400,

{0xFFFF, 0xFFFF},

};

//==========================================================
//	CAMERA_VT_PRETTY_1
//==========================================================

//const uint32 reg_VT_pretty_level_1_6aafx[] =
static const struct s5k5bafx_reg mode_pretty_1[] =
{
//0xffff000A,
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x04CC},

{0x0F12, 0x0000}, //0000, //0000, //000, //
{0x0F12, 0x000D}, //000D, //000D, //00D, //
{0x0F12, 0x001B}, //001B, //001B, //01B, //
{0x0F12, 0x0046}, //0046, //0046, //04B, //
{0x0F12, 0x00AA}, //00AA, //00AA, //0AA, //
{0x0F12, 0x0120}, //128, //0130, //0118, //122, //
{0x0F12, 0x0190}, //19A, //01B0, //0186, //190, //
{0x0F12, 0x01E0}, //1E6, //0200, //01E0, //1D6, //
{0x0F12, 0x0250}, //25F, //0270, //0258, //24E, //
{0x0F12, 0x02A5}, //2B0, //02CA, //02A8, //2A8, //
{0x0F12, 0x0320}, //328, //033A, //0320, //320, //
{0x0F12, 0x0370}, //378, //0380, //0370, //370, //
{0x0F12, 0x03B0}, //3B9, //03C0, //03B1, //3B1, //
{0x0F12, 0x03D8}, //03D4, //03D4, //3D4, //
{0x0F12, 0x03F2}, //03F2, //03F2, //3F2, //
{0x0F12, 0x0400}, //0400, //0400, //400, //

{0x0F12, 0x0000}, //0000, //0000, //
{0x0F12, 0x000D}, //000D, //000D, //
{0x0F12, 0x001B}, //001B, //001B, //
{0x0F12, 0x0046}, //0046, //004B, //
{0x0F12, 0x00AA}, //00AA, //00AA, //
{0x0F12, 0x0120}, //0128, //0122, //
{0x0F12, 0x0190}, //019A, //0190, //
{0x0F12, 0x01E0}, //01E6, //01D6, //
{0x0F12, 0x0250}, //025F, //024E, //
{0x0F12, 0x02A5}, //02B0, //02A8, //
{0x0F12, 0x0320}, //0328, //0320, //
{0x0F12, 0x0370}, //0378, //0370, //
{0x0F12, 0x03B0}, //03B9, //03B1, //
{0x0F12, 0x03D8}, //03D8, //03D4, //
{0x0F12, 0x03F2}, //03F2, //03F2, //
{0x0F12, 0x0400}, //0400, //0400, //

{0x0F12, 0x0000}, //0000, //0000, //
{0x0F12, 0x000D}, //000D, //000D, //
{0x0F12, 0x001B}, //001B, //001B, //
{0x0F12, 0x0046}, //0046, //004B, //
{0x0F12, 0x00AA}, //00AA, //00AA, //
{0x0F12, 0x0120}, //0128, //0122, //
{0x0F12, 0x0190}, //019A, //0190, //
{0x0F12, 0x01E0}, //01E6, //01D6, //
{0x0F12, 0x0250}, //025F, //024E, //
{0x0F12, 0x02A5}, //02B0, //02A8, //
{0x0F12, 0x0320}, //0328, //0320, //
{0x0F12, 0x0370}, //0378, //0370, //
{0x0F12, 0x03B0}, //03B9, //03B1, //
{0x0F12, 0x03D8}, //03D8, //03D4, //
{0x0F12, 0x03F2}, //03F2, //03F2, //
{0x0F12, 0x0400}, //0400, //0400, //

{0xFFFF, 0xFFFF},

};

//==========================================================
//	CAMERA_VT_PRETTY_2
//==========================================================

//const uint32 reg_VT_pretty_level_2_6aafx[] =
static const struct s5k5bafx_reg mode_pretty_2[] =
{
//0xffff000A,
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x04CC},

{0x0F12, 0x0000}, //0000, //
{0x0F12, 0x000D}, //000D, //
{0x0F12, 0x001B}, //001B, //
{0x0F12, 0x0055}, //0050, //
{0x0F12, 0x00C0}, //00B4, //
{0x0F12, 0x0164}, //0154, //
{0x0F12, 0x01C0}, //01B8, //
{0x0F12, 0x0220}, //0212, //
{0x0F12, 0x02A0}, //0294, //
{0x0F12, 0x02F0}, //02E4, //
{0x0F12, 0x0365}, //0352, //
{0x0F12, 0x03A0}, //0398, //
{0x0F12, 0x03D4}, //03D4, //
{0x0F12, 0x03E8}, //03E8, //
{0x0F12, 0x03F7}, //03F7, //
{0x0F12, 0x0400}, //0400, //

{0x0F12, 0x0000}, //0000, //
{0x0F12, 0x000D}, //000D, //
{0x0F12, 0x001B}, //001B, //
{0x0F12, 0x0055}, //0050, //
{0x0F12, 0x00C0}, //00B4, //
{0x0F12, 0x0164}, //0154, //
{0x0F12, 0x01C0}, //01B8, //
{0x0F12, 0x0220}, //0212, //
{0x0F12, 0x02A0}, //0294, //
{0x0F12, 0x02F0}, //02E4, //
{0x0F12, 0x0365}, //0352, //
{0x0F12, 0x03A0}, //0398, //
{0x0F12, 0x03D4}, //03D4, //
{0x0F12, 0x03E8}, //03E8, //
{0x0F12, 0x03F7}, //03F7, //
{0x0F12, 0x0400}, //0400, //

{0x0F12, 0x0000}, //0000, //
{0x0F12, 0x000D}, //000D, //
{0x0F12, 0x001B}, //001B, //
{0x0F12, 0x0055}, //0050, //
{0x0F12, 0x00C0}, //00B4, //
{0x0F12, 0x0164}, //0154, //
{0x0F12, 0x01C0}, //01B8, //
{0x0F12, 0x0220}, //0212, //
{0x0F12, 0x02A0}, //0294, //
{0x0F12, 0x02F0}, //02E4, //
{0x0F12, 0x0365}, //0352, //
{0x0F12, 0x03A0}, //0398, //
{0x0F12, 0x03D4}, //03D4, //
{0x0F12, 0x03E8}, //03E8, //
{0x0F12, 0x03F7}, //03F7, //
{0x0F12, 0x0400}, //0400, //

{0xFFFF, 0xFFFF},

};

//==========================================================
//	CAMERA_VT_PRETTY_3
//==========================================================

//const uint32 reg_VT_pretty_level_3_6aafx[] =
static const struct s5k5bafx_reg mode_pretty_3[] =
{
//0xffff000A,
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x04CC},

{0x0F12, 0x0000}, //0000, //0000, //
{0x0F12, 0x000D}, //000D, //000D, //
{0x0F12, 0x001B}, //001B, //001B, //
{0x0F12, 0x0064}, //0064, //0064, //
{0x0F12, 0x00E5}, //00E5, //00DC, //
{0x0F12, 0x0190}, //95, //0195, //0186, //
{0x0F12, 0x01F5}, //01F5, //01EA, //
{0x0F12, 0x0260}, //0265, //024E, //
{0x0F12, 0x02E5}, //02F0, //02DA, //
{0x0F12, 0x032A}, //30, //0335, //0320, //
{0x0F12, 0x038A}, //90, //0395, //038E, //
{0x0F12, 0x03C5}, //CA, //03D0, //03CA, //
{0x0F12, 0x03E0}, //E5, //03E8, //03E8, //
{0x0F12, 0x03EC}, //F0, //03F2, //03F2, //
{0x0F12, 0x03F7}, //03F7, //03F7, //
{0x0F12, 0x0400}, //0400, //0400, //

{0x0F12, 0x0000}, //0000, //0000, //0000, //
{0x0F12, 0x000D}, //000D, //000D, //000D, //
{0x0F12, 0x001B}, //001B, //001B, //001B, //
{0x0F12, 0x0064}, //0064, //0064, //0064, //
{0x0F12, 0x00E5}, //00E5, //00E5, //00DC, //
{0x0F12, 0x0190}, //0195, //0195, //0186, //
{0x0F12, 0x01F5}, //01F5, //01F5, //01EA, //
{0x0F12, 0x0260}, //0260, //0265, //024E, //
{0x0F12, 0x02E5}, //02E5, //02F0, //02DA, //
{0x0F12, 0x032A}, //0330, //0335, //0320, //
{0x0F12, 0x038A}, //0390, //0395, //038E, //
{0x0F12, 0x03C5}, //03CA, //03D0, //03CA, //
{0x0F12, 0x03E0}, //03E5, //03E8, //03E8, //
{0x0F12, 0x03EC}, //03F0, //03F2, //03F2, //
{0x0F12, 0x03F7}, //03F7, //03F7, //03F7, //
{0x0F12, 0x0400}, //0400, //0400, //0400, //

{0x0F12, 0x0000}, //0000, //0000, //0000, //
{0x0F12, 0x000D}, //000D, //000D, //000D, //
{0x0F12, 0x001B}, //001B, //001B, //001B, //
{0x0F12, 0x0064}, //0064, //0064, //0064, //
{0x0F12, 0x00E5}, //00E5, //00E5, //00DC, //
{0x0F12, 0x0190}, //0195, //0195, //0186, //
{0x0F12, 0x01F5}, //01F5, //01F5, //01EA, //
{0x0F12, 0x0260}, //0260, //0265, //024E, //
{0x0F12, 0x02E5}, //02E5, //02F0, //02DA, //
{0x0F12, 0x032A}, //0330, //0335, //0320, //
{0x0F12, 0x038A}, //0390, //0395, //038E, //
{0x0F12, 0x03C5}, //03CA, //03D0, //03CA, //
{0x0F12, 0x03E0}, //03E5, //03E8, //03E8, //
{0x0F12, 0x03EC}, //03F0, //03F2, //03F2, //
{0x0F12, 0x03F7}, //03F7, //03F7, //03F7, //
{0x0F12, 0x0400}, //0400, //0400, //0400, //

{0xFFFF, 0xFFFF},

};
