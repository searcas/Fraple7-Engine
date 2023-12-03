#pragma once




#define FPL_SUCCESS								0x00000000;
#define FPL_GENERIC_ERROR						0x00000001;
#define FPL_LOGIC_ERROR							0x00000002;
#define FPL_BAD_ARGUMENTS						0x00000003;
#define FPL_BUFFER_OVERFLOW						0x00000004;

#define FPL_ENGINE_NOT_RUNNING					0x000000F0;
#define FPL_ENGINE_FAILED_TO_START				0x000000F1;
#define FPL_ENGINE_FAILED_ON_RUN				0x000000F2;
#define FPL_ENGINE_FAILED_TO_SHUT				0x000000F3;

#define FPL_PIPELINE_ERROR						0x0000F000;
#define FPL_PIPELINE_SWAP_CHAIN_ERROR			0x0000F001;
#define FPL_PIPELINE_RENDER_TARGET_VIEW_ERROR	0x0000F002;



#define FPL_NOT_IMPLEMENTED			0xFFFF'FFFFU;
