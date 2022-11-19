/****************************************************************************************************

		ExtraShadersDef.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/

#ifndef __ExtraShadersDef__
#define __ExtraShadersDef__

// Global flag
#define SHADERS_PLUS 1 // for Veloute 2

// We define the shaders CLSID

#define R_CLSID_TilingShader1 0xaafe9c33, 0x57cf, 0x4362, 0xb8, 0x2d, 0x1e, 0xfd, 0xa7, 0x87, 0x9f, 0x62
#define R_CLSID_GlobalTilingShader1 0xea565842, 0x2d1f, 0x4105, 0x99, 0x22, 0x7a, 0x29, 0xf4, 0xf9, 0xc7, 0x40

#define R_CLSID_TilingShader2 0x8d3c7ec2, 0x7f1a, 0x4edc, 0xaa, 0x10, 0x7f, 0xeb, 0x20, 0x1, 0xa8, 0xb7
#define R_CLSID_GlobalTilingShader2 0x82f22e23, 0xd11b, 0x4a1f, 0x8a, 0x98, 0x6, 0xd0, 0x9a, 0x8a, 0xa1, 0x3d

#define R_CLSID_TilingShader3 0x20bc9d11, 0xce37, 0x448d, 0x99, 0x40, 0xad, 0xb5, 0x9f, 0x78, 0x3f, 0xa7
#define R_CLSID_GlobalTilingShader3 0x1e0a986c, 0xfda3, 0x448e, 0x8d, 0xaa, 0x2f, 0xe3, 0x19, 0x1e, 0x3b, 0x84

#define R_CLSID_TilingShader4 0xa4a4fc44, 0x9a34, 0x44be, 0xa9, 0x3f, 0x37, 0xe2, 0x75, 0xdf, 0x4c, 0x2f
#define R_CLSID_GlobalTilingShader4 0x4a015c48, 0xae76, 0x4bdf, 0xa6, 0xb5, 0x8a, 0xf8, 0x3f, 0xa0, 0xd2, 0x22

#define R_CLSID_RoofShader1 0xe0926946, 0x7631, 0x46c1, 0x81, 0x75, 0xcf, 0xac, 0xb8, 0x58, 0x1f, 0xf6
#define R_CLSID_GlobalRoofShader1 0x62327eb3, 0xedf7, 0x4a77, 0xaf, 0x2f, 0x8c, 0xe0, 0x64, 0x4a, 0x2c, 0xc8

#define R_CLSID_BumpNoise2DShader 0x98b56420, 0x7c2, 0x4090, 0x8d, 0xc, 0xb9, 0xbc, 0xa9, 0xa0, 0xa7, 0xf9
#define R_CLSID_BumpNoise3DShader 0x36dcc213, 0xfca8, 0x4b25, 0x89, 0xcd, 0x3b, 0x4d, 0x44, 0x3a, 0xaa, 0xb
#define R_CLSID_FilterShader 0xe8c5afa0, 0x53bb, 0x4bb9, 0x8a, 0xd2, 0xa6, 0x8a, 0x29, 0x5b, 0x60, 0x3f
#define R_CLSID_PerturbationShader 0xc8a62318, 0xe810, 0x4732, 0xb0, 0xf1, 0xb3, 0x41, 0xa4, 0xe0, 0x4, 0xf7
#define R_CLSID_Gradient2DShader 0xbc64b8f9, 0x4cda, 0x4bc2, 0xbd, 0x8d, 0xa0, 0x70, 0xfe, 0x35, 0x7d, 0xaf
#define R_CLSID_Gradient3DShader 0x2cc28a8f, 0x7a55, 0x4c0e, 0x9b, 0x73, 0x4e, 0xed, 0x43, 0xfd, 0x63, 0x71

// Part extension
#define R_CLSID_TParamChooserPart1 0xc8172704, 0x4b92, 0x4d36, 0x92, 0xa2, 0xc3, 0xc0, 0xb, 0x72, 0x3a, 0x6c
#define R_CLSID_TParamChooserPart2 0xf857316b, 0x4246, 0x4595, 0x99, 0x5b, 0x93, 0x0, 0x5d, 0x5, 0xbc, 0xd7
#define R_CLSID_TParamChooserPart3 0x4546b4be, 0xa151, 0x4447, 0xa0, 0x6c, 0x1c, 0xa1, 0x3d, 0xeb, 0x36, 0x42

// Shaders Plus

#define R_CLSID_GridShader1 0x89948abe, 0x3d4c, 0x4c00, 0xb7, 0x7c, 0x23, 0x7d, 0x14, 0xc7, 0xce, 0xd2
#define R_CLSID_GlobalGridShader1 0xc789bde4, 0xaa7f, 0x4dc8, 0xa9, 0xc9, 0xfc, 0xf2, 0xdc, 0x44, 0x33, 0x9d

#define R_CLSID_GridShader2 0x2017b518, 0x58a3, 0x4d57, 0x8c, 0xc9, 0x4e, 0x9d, 0x14, 0xc6, 0xb0, 0xf1
#define R_CLSID_GlobalGridShader2 0x9dbc7adf, 0x4f69, 0x4970, 0x8d, 0x68, 0xd0, 0x6c, 0x4e, 0xe8, 0x47, 0xf7

#define R_CLSID_GridShader3 0xf72e7ded, 0x72e1, 0x49bf, 0xb4, 0x1d, 0xe4, 0xfc, 0xf1, 0xeb, 0xf2, 0xb4
#define R_CLSID_GlobalGridShader3 0x5c057d51, 0x47b1, 0x43fa, 0xa0, 0x52, 0x30, 0x45, 0x1, 0x70, 0xb3, 0x86

#define R_CLSID_GridShader4 0x49681200, 0xd9f, 0x4408, 0x98, 0x7c, 0x76, 0x90, 0xf7, 0x1e, 0xe7, 0x4e
#define R_CLSID_GlobalGridShader4 0x37a2d10c, 0x4e45, 0x4724, 0xbd, 0x58, 0xa8, 0xd3, 0xae, 0x3e, 0x99, 0x72

#define R_CLSID_WeaveShader1 0x12282966, 0xff1e, 0x43af, 0x9d, 0x5e, 0x4a, 0x79, 0xe, 0x8f, 0x66, 0x62
#define R_CLSID_GlobalWeaveShader1 0xefcf8c87, 0x30, 0x4104, 0xb7, 0x45, 0x6, 0x57, 0xa4, 0x1, 0xbc, 0x99

#define R_CLSID_RandomLines2DShader 0x6a8ff24d, 0xbc4a, 0x4f9f, 0xaf, 0x66, 0x85, 0xb1, 0x17, 0x13, 0xb6, 0x5
#define R_CLSID_RandomLines3DShader 0x65737d49, 0x6a81, 0x4545, 0x82, 0xa3, 0x3c, 0xa7, 0xe7, 0xfd, 0xfe, 0x65

// Part extension
#define R_CLSID_TParamChooserPart4 0x955f77bf, 0xd01e, 0x4532, 0xbe, 0x13, 0x94, 0xb9, 0xb3, 0x91, 0x90, 0xa3

#endif

