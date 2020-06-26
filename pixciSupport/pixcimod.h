/*
 *
 *	pixcimod.h	External	17-Nov-2017
 *
 *	Copyright (C)  1995-2017  EPIX, Inc.  All rights reserved.
 *	Permission is granted to use this code, or any part thereof,
 *      in programs which operate the PIXCI(R) Imaging Board.
 *
 *	PIXCI: Codes for various models.
 *
 */



#define PIXCI_SV2	0x0001
#define PIXCI_SV3	0x0002
#define PIXCI_DLS	0x0003	/* never used!				*/
#define PIXCI_SV4	0x0004
#define PIXCI_D 	0x0005
#define PIXCI_D32	0x0006	/* never used! appears as PIXCI_D	*/
#define PIXCI_A 	0x0007
#define PIXCI_D24	0x0008	/* never used! appears as PIXCI_D	*/
#define PIXCI_DVO	0x0009
#define PIXCI_SV5	0x000A
#define PIXCI_D2X	0x000B
#define PIXCI_D2X_DVC	0x000C
#define PIXCI_CL3SD_NAC 0x000D
#define PIXCI_CL1	0x000E
#define PIXCI_D3X	0x000F
#define PIXCI_CL3SD	0x0010
#define PIXCI_CL2	0x0011
#define PIXCI_SI	0x0012
#define PIXCI_SV6	0x0013
#define PIXCI_E1	0x0014
#define PIXCI_E1DB	0x0015
#define PIXCI_CL1_AA	0x0016
#define PIXCI_E4	0x0017
#define PIXCI_E1_AA	0x0018
#define PIXCI_E1DB_AA	0x0019
#define PIXCI_EL1	0x001A
#define PIXCI_E4DB	0x001B
#define PIXCI_EC1	0x001C
#define PIXCI_EL1DB	0x001D
// Version 3.0 ...
#define PIXCI_D2X_AA	0x001E
#define PIXCI_EL1_AA	0x001F
#define PIXCI_EL1_IR	0x0020
#define PIXCI_EL1DB_IR	0x0021
#define PIXCI_SI4	0x0022
#define PIXCI_SI_UI	0x0023
#define PIXCI_SI4_UI	0x0024
#define PIXCI_ECB1	0x0025
#define PIXCI_EB1	0x0026
#define PIXCI_EB1POCL	0x0027
#define PIXCI_SI1	0x0028
#define PIXCI_ECB134	0x0029
#define PIXCI_ECB2	0x002A
#define PIXCI_SI2	0x002B
// Version 3.7 ...
#define PIXCI_SV7	0x002C
#define PIXCI_SI2_UI	0x002D
#define PIXCI_ELS2	0x002E
#define PIXCI_A310	0x002F
#define PIXCI_E8	0x0030
#define PIXCI_E8DB	0x0031
#define PIXCI_E8CAM	0x0032
#define PIXCI_EC1_UIS	0x0033
// Version 3.8 ...
#define PIXCI_D3XE	0x0034
#define PIXCI_SV8	0x0035
#define PIXCI_E104X42F	0x0036
#define PIXCI_TNTX1	0x0037
#define PIXCI_E104X44B	0x0038
#define PIXCI_E104X4F2B 0x0039	/* a Full unit of a PIXCI_E104X4F2B	*/
#define PIXCI_E104X42BF 0x003A	/* a Base unit of a PIXCI_E104X4F2B	*/
#define PIXCI_A110	0x003B
#define PIXCI_EB1MINI	0x003C
#define PIXCI_E4G22F	0x003D
#define PIXCI_E4G24B	0x003E
#define PIXCI_E4G22BF	0x003F	/* a Base unit of a PIXCI_E4G2F2B	*/
#define PIXCI_E4G2F2B	0x0040	/* a Full unit of a PIXCI_E4G2F2B	*/
#define PIXCI_EB1MINING 0x0041
#define PIXCI_EB1TG	0x0042
#define PIXCI_SI4B	0x0043
#define PIXCI_E4V7G	0x0044	/* E4 version 7G supports gen2 x4	*/
#define PIXCI_E4DBV7G	0x0045	/* E4 version 7G supports gen2 x4	*/
#define PIXCI_A1107I1O	0x0046
#define PIXCI_EL1V7G	0x0047	/* EL1 version 7 supports gen2 x1	*/
#define PIXCI_EL1DBV7G	0x0048	/* EL1 version 7 supports gen2 x1	*/


/*
 * PIXCI(R) frame grabber submodel codes.
 */
#define PIXCI_SV5_SV5	0x0000
#define PIXCI_SV5_SV5A	'A'
#define PIXCI_SV5_SV5B	'B'
#define PIXCI_SV5_SV5L	'L'
