#pragma once
#define DEV_GX_STR "Dev1"								//"Dev2"
#define DEV_GX_OUT_STR DEV_GX_STR ## "/ao2"				//"/ao0:1"
#define DEV_GX_TRIG_STR "/" ## DEV_GX_STR ## "/PFI0"	//"/PFI1"
#define DEV_GX_CLK_STR "/" ## DEV_GX_STR ## "/PFI14"

#define DEV_GY_STR "Dev1"
#define DEV_GY_OUT_STR DEV_GY_STR ## "/ao0"
#define DEV_GY_P_V_OUT_STR DEV_GY_STR ## "/ao1:3"
#define DEV_GY_TRIG_STR "/" ## DEV_GY_STR ## "/PFI0"	//"/PFI1"
#define DEV_GY_CLK_STR "/" ## DEV_GY_STR ## "/PFI14"

#define DEV_CTR "Dev2/ctr0"
#define DEV_READ_FREQ "/Dev2/PFI12"
#define DEV_RES "Dev2/ao0"								//"Dev1/ao0"

#define DEV_READ_POS "/Dev1/ai0:2"

#define VOLT_MIN (-10.0)
#define VOLT_MAX (10.0)

#define WRITE_TIMEOUT (10.0)
#define READ_TIMEOUT (10.0)

#define WAIT_TIME (0.1)

