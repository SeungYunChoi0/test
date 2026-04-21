/*
 * Copyright Telechips Inc.
 *
 * TCC Version 1.0
 *
 * This source code contains confidential information of Telechips.
 *
 * Any unauthorized use without a written permission of Telechips including not
 * limited to re-distribution in source or binary form is strictly prohibited.
 *
 * This source code is provided "AS IS" and nothing contained in this source code
 * shall constitute any express or implied warranty of any kind, including without
 * limitation, any warranty of merchantability, fitness for a particular purpose
 * or non-infringement of any patent, copyright or other third party intellectual
 * property right.
 * No warranty is made, express or implied, regarding the information's accuracy,
 * completeness, or performance.
 *
 * In no event shall Telechips be liable for any claim, damages or other
 * liability arising from, out of or in connection with this source code or
 * the use in the source code.
 *
 * This source code is provided subject to the terms of a Mutual Non-Disclosure
 * Agreement between Telechips and Company.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include "NnAppMain.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define DEFAULT_NETWORK_PATH_1 "/usr/share/yolov5s_quantized/"
#define DEFAULT_NETWORK_PATH_2 "/usr/share/mobilenetv2_10_quantized/"
#define DEFAULT_HAND_NETWORK_ALIAS "hand640_pt"
#define DEFAULT_HAND_NETWORK_DIR "hand640_qt"
#define DEFAULT_INPUT_PATH "/dev/video2"
#define DEFAULT_OUTPUT_PATH "/dev/overlay"
#define DEFAULT_INPUT_WIDTH 1280
#define DEFAULT_INPUT_HEIGHT 720
#define DEFAULT_OUTPUT_WIDTH 800
#define DEFAULT_OUTPUT_HEIGHT 480
#define DEFAULT_INPUT_MODE INPUT_MODE_CAMERA
#define DEFAULT_OUTPUT_MODE OUTPUT_MODE_LCD
#define DEFAULT_DEBUG_MODE DEBUG_MODE_DISABLE
#define DEFAULT_NPU_DEBUG_MODE NPU_DEBUG_MODE_DISABLE
#define DEFAULT_NPU_RUN_MODE NPU_RUN_ASYNC
#define DEFAULT_NETWORK_INDEX NETWORK_INDEX_MAX
#define DEFAULT_OUTPUT_SX 0
#define DEFAULT_OUTPUT_SY 0
#define INPUT_RTPM_WIDTH 1280
#define INPUT_RTPM_HEIGHT 720
#define OUTPUT_RTPM_WIDTH 1280
#define OUTPUT_RTPM_HEIGHT 720
#define DEFAULT_TARGET_IP_ADDRESS "192.168.0.8"

#define INTERACTIVE_MODE

typedef struct
{
	CameraHandle cam_handle;
	ScalerHandle scaler_handle;
	DisplayHandle display_handle;
	MessageHandle msg_handle;
} app_obj_t;

/* ========================================================================== */
/*                       Internal Function Declarations                       */
/* ========================================================================== */
static void NnparseArgs(param_info_t *param, int argc, char** argv);
static void NnprintUsage(void);
static void NnModeChecker(param_info_t *param);
static void NnInitAppContext(app_context_t *pContext, param_info_t *pParam);
static void NnNormalizeNetworkPath(param_info_t *param, network_index_t index);

static float NnGetFPS();
static int32_t NnCreateAPI(app_context_t *pContext, app_obj_t *pObj);
static int32_t NnDestroyAPI(app_context_t *pContext, app_obj_t *pObj);

static void NnInputModeInit(app_context_t *pContext, CameraHandle CameraHandle, MessageHandle msgHandle);
static void NnInputModeDeinit(app_context_t *pContext, CameraHandle CameraHandle, MessageHandle msgHandle);
static void NnGetFrame(app_context_t *pContext, CameraHandle CameraHandle, MessageHandle msgHandle);

static void NnOutputModeInit(app_context_t *pContext, DisplayHandle dispHandle, MessageHandle msgHandle);
static void NnOutputModeDeinit(app_context_t *pContext, DisplayHandle dispHandle, MessageHandle msgHandle);
static int32_t NnOutputResultFrame(app_context_t *pContext, DisplayHandle dispHandle, MessageHandle msgHandle);

static int32_t NnScalerInit(ScalerHandle handle);
static void NnScalerDenit(ScalerHandle handle);
static void NnResizeInputFrame(app_context_t *pContext, ScalerHandle handle);
static void NnResizeOutputFrame(app_context_t *pContext, ScalerHandle scalerHandle, MessageHandle msgHandle);

static int32_t NnOutputResultData(app_context_t *pContext, MessageHandle msgHandle);
static int32_t NnReleaseFrame(app_context_t *pContext, CameraHandle cameraHandle, MessageHandle msgHandle);
static void NnDrawResult(app_context_t *pContext);

static void NnPerfMonitorInit(app_context_t *pContext, MessageHandle msgHandle);
static void NnPerfMonitorDeinit(app_context_t *pContext);

static void NnShowUsage(int32_t argc, char *argv[]);
static int32_t adjustRes(uint16_t *punXres, uint16_t *punYres);

#ifdef INTERACTIVE_MODE
static void *NnInteractive(void *arg);
#endif // _INTERACTIVE_MODE
/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */
static const struct {
	char *inputModeStr[3];
	char *outputModeStr[3];
	char *networkModeStr[2];
	char *debugModeStr[3];
	char *npuDebugModeStr[3];
	char *npuRunModeStr[2];
	char *imgFmtStr[2];
} opt_str = {
	.inputModeStr = {"camera", "file", "rtpm"},
	.outputModeStr = {"display", "file", "rtpm"},
	.networkModeStr = {"on", "off"},
	.debugModeStr = {"off", "log", "file"},
	.npuDebugModeStr = {"off", "log", "file"},
	.npuRunModeStr = {"SyncMode", "AsyncMode"},
	.imgFmtStr = {"ARGB8888", "RGB888"},
};

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
uint64_t syncStamp = 0; //global
app_obj_t g_AppObj;
pthread_t g_InteractiveThread = (pthread_t)NULL;

#ifdef INTERACTIVE_MODE
static char menu[] = {
	"\n"
	"\n ========================="
	"\n Demo : tc-nn-app Demo"
	"\n ========================="
	"\n"
	"\n c: TBD"
	"\n"
	"\n p: TBD"
	"\n"
	"\n x: Exit"
	"\n"
	"\n Enter Choice: "
};
#endif // _INTERACTIVE_MODE

static char logo[] = {
    "\n"
    " ===================================================================================================\n"
    " ==   _________  _______   ___       _______   ________  ___  ___  ___  ________  ________        ==\n"
    " ==  |\\___   ___\\\\  ___ \\ |\\  \\     |\\  ___ \\ |\\   ____\\|\\  \\|\\  \\|\\  \\|\\   __  \\|\\   ____\\       ==\n"
    " ==  \\|___ \\  \\_\\ \\   __/|\\ \\  \\    \\ \\   __/|\\ \\  \\___|\\ \\  \\\\\\  \\ \\  \\ \\  \\|\\  \\ \\  \\___|_      ==\n"
    " ==       \\ \\  \\ \\ \\  \\_|/_\\ \\  \\    \\ \\  \\_|/_\\ \\  \\    \\ \\   __  \\ \\  \\ \\   ____\\ \\_____  \\     ==\n"
    " ==        \\ \\  \\ \\ \\  \\_|\\ \\ \\  \\____\\ \\  \\_|\\ \\ \\  \\____\\ \\  \\ \\  \\ \\  \\ \\  \\___|\\|____|\\  \\    ==\n"
    " ==         \\ \\__\\ \\ \\_______\\ \\_______\\ \\_______\\ \\_______\\ \\__\\ \\__\\ \\__\\ \\__\\     ____\\_\\  \\   ==\n"
    " ==          \\|__|  \\|_______|\\|_______|\\|_______|\\|_______|\\|__|\\|__|\\|__|\\|__|    |\\_________\\  ==\n"
    " ==                                                                                 \\|_________|  ==\n"
	" ===================================================================================================\n"
	"\n"
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */
static void NnShowUsage(int32_t argc, char *argv[])
{
	printf("\n");
	printf(" tc-nn-app Demo - (c) telechips\n");
	printf(" ========================================================\n");
	printf("\n");
	printf(" Usage,\n");
	printf(" ex) [%d]  %s -?\n", argc, argv[0]);
	printf("\n");
}

#ifdef INTERACTIVE_MODE
static void *NnInteractive(void *arg)
{
	char ch;
	(void)arg;

	while (NnCheckExitFlag() != true)
	{
		printf("%s", menu);
		ch = getchar();
		printf("\n");

		switch (ch)
		{
		case 'c':
			// TBD
			break;
		case 'p':
			// TBD
			break;
		case 'e':
			// TBD
			break;
		case 'x':
			NnSetSignalFlag(true);
			break;
		default:
			// printf("Invalid option! Please try again.\n");
			break;
		}
	}
	if (NnCheckExitFlag())
	{
		printf("[INFO] [%s] end!\n", __FUNCTION__);
	}
	return NULL;
}
#endif // _INTERACTIVE_MODE

static void NnparseArgs(param_info_t *param, int argc, char** argv)
{
	int32_t opt;

	// Setting Default Parameters
	param->networkCnt = DEFAULT_NETWORK_INDEX;
	param->networkPath[NETWORK_INDEX_0] = DEFAULT_NETWORK_PATH_1;
	param->networkPath[NETWORK_INDEX_1] = DEFAULT_NETWORK_PATH_2;
	param->inputPath = DEFAULT_INPUT_PATH;
	param->outputPath = DEFAULT_OUTPUT_PATH;
	param->inputWidth = DEFAULT_INPUT_WIDTH;
	param->inputHeight = DEFAULT_INPUT_HEIGHT;
	param->inputFormat = IMAGE_FMT_RGBA32; //IMAGE_FMT_RGBA32
	param->outputWidth = DEFAULT_OUTPUT_WIDTH;
	param->outputHeight = DEFAULT_OUTPUT_HEIGHT;
	param->npuDebugMode = NPU_DEBUG_MODE_DISABLE;
	param->outputFormat = IMAGE_FMT_RGB24; //IMAGE_FMT_RGB24
	param->inputMode = DEFAULT_INPUT_MODE;
	param->outputMode = DEFAULT_OUTPUT_MODE;
	param->outputSx = DEFAULT_OUTPUT_SX;
	param->outputSy = DEFAULT_OUTPUT_SY;
	param->debugMode = DEFAULT_DEBUG_MODE;
	param->npuRunMode = DEFAULT_NPU_RUN_MODE;
	param->pRtpmIpAdress = DEFAULT_TARGET_IP_ADDRESS;

	if (argc == 1)
	{
		printf("%s", logo);
		NnShowUsage(argc, argv);
		// exit(0);
	}
	else
	{
		printf("%s", logo);
	}

	while(-1 != (opt = getopt(argc, argv, "i:o:w:W:h:H:n:N:p:P:X:Y:g:u:a:?")))
	{
		switch (opt) {
			// Common Parameter
			case 'i':
				if(strcmp(optarg, "camera") == 0)
				{
					param->inputMode = INPUT_MODE_CAMERA;
					param->inputFormat = IMAGE_FMT_RGBA32;
				}
				else if(strcmp(optarg, "rtpm") == 0)
				{
					param->inputMode = INPUT_MODE_RTPM;
					param->inputFormat = IMAGE_FMT_RGB24;
					param->inputWidth = INPUT_RTPM_WIDTH;
					param->inputHeight = INPUT_RTPM_HEIGHT;
				}
				else if(strcmp(optarg, "file") == 0)
				{
					param->inputMode = INPUT_MODE_FILE;
					param->inputFormat = IMAGE_FMT_RGBA32;
				}
				else
				{
					printf("invalid parameter %s", optarg);
					NnprintUsage();
					exit(0);
				}
				break;
			case 'o':
				if(strcmp(optarg, "display") == 0)
				{
					param->outputMode = OUTPUT_MODE_LCD;
					param->outputFormat = IMAGE_FMT_RGB24;
				}
				else if(strcmp(optarg, "rtpm") == 0)
				{
					param->outputMode = OUTPUT_MODE_RTPM;
					param->outputFormat = IMAGE_FMT_RGB24;
					param->outputWidth = OUTPUT_RTPM_WIDTH;
					param->outputHeight = OUTPUT_RTPM_HEIGHT;
				}
				else if(strcmp(optarg, "file") == 0)
				{
					param->outputMode = OUTPUT_MODE_FILE;
					param->outputFormat = IMAGE_FMT_RGB24;
				}
				else
				{
					printf("invalid parameter %s", optarg);
					NnprintUsage();
					exit(0);
				}
				break;
			case 'w':
				param->inputWidth = atoi(optarg);
				break;
			case 'W':
				param->outputWidth = atoi(optarg);
				break;
			case 'h':
				param->inputHeight = atoi(optarg);
				break;
			case 'H':
				param->outputHeight = atoi(optarg);
				break;
			case 'u':
				if(strcmp(optarg, "off") == 0)
				{
					param->npuDebugMode = NPU_DEBUG_MODE_DISABLE;
				}
				else if(strcmp(optarg, "log") == 0)
				{
					param->npuDebugMode = NPU_DEBUG_MODE_LOG;
				}
				else if(strcmp(optarg, "file") == 0)
				{
					param->npuDebugMode = NPU_DEBUG_MODE_FILE;
				}
				else
				{
					printf("invalid parameter %s", optarg);
					NnprintUsage();
					exit(0);
				}
				break;
			case 'n':
				param->networkPath[0] = optarg;
				param->networkCnt = 1;
				break;
			case 'N':
				param->networkPath[1] = optarg;
				param->networkCnt = 2;
				break;
			case 'p':
				param->inputPath = optarg;
				break;
			case 'P':
				param->outputPath = optarg;
				break;
			case 'X':
				param->outputSx = atoi(optarg);
				break;
			case 'Y':
				param->outputSy = atoi(optarg);
				break;
			case 'g':
				if(strcmp(optarg, "off") == 0)
				{
					param->debugMode = DEBUG_MODE_DISABLE;
				}
				else if(strcmp(optarg, "log") == 0)
				{
					param->debugMode = DEBUG_MODE_LOG;
				}
				else if(strcmp(optarg, "file") == 0)
				{
					param->debugMode = DEBUG_MODE_FILE;
				}
				else
				{
					printf("invalid parameter %s", optarg);
					NnprintUsage();
					exit(0);
				}
				break;
			case 'a':
				if(strcmp(optarg, "sync") == 0)
				{
					param->npuRunMode = NPU_RUN_SYNC;
				}
				else if(strcmp(optarg, "async") == 0)
				{
					param->npuRunMode = NPU_RUN_ASYNC;
				}
				else
				{
					printf("[WARN] invalid parameter %s, Using a Default MODE [%s]\n",
							optarg , opt_str.npuRunModeStr[DEFAULT_NPU_RUN_MODE]);
				}
				break;
			case 't':
				param->pRtpmIpAdress = optarg;
				break;
			case '?':
				NnprintUsage();
				exit(0);
				break;
			default:
				NnprintUsage();
				exit(0);
				break;
		}
	}

	NnNormalizeNetworkPath(param, NETWORK_INDEX_0);
	NnNormalizeNetworkPath(param, NETWORK_INDEX_1);

   	if (param->outputMode == OUTPUT_MODE_LCD) {
    	int32_t bAdjust = adjustRes(&param->outputWidth, &param->outputHeight);
    	if (bAdjust == -1) {
        	printf("---!!!! adusting resolution failed\n");
    	}
	}

	printf("----------------Parameter Info----------------\n");
	printf("\n");
	printf("[Network1]Network Path         : %s\n", param->networkPath[0]);
	printf("[Network2]Network Path         : %s\n", param->networkPath[1]);
	printf("\n");
	printf("[Common]Input Mode             : %s\n", opt_str.inputModeStr[param->inputMode]);
	printf("[Common]Output Mode            : %s\n", opt_str.outputModeStr[param->outputMode]);
	printf("[Common]Input Size             : %d x %d\n", param->inputWidth, param->inputHeight);
	printf("[Common]Output Size            : %d x %d\n", param->outputWidth, param->outputHeight);
	printf("[Common]Input Path             : %s\n", param->inputPath);
	printf("[Common]Output Path            : %s\n", param->outputPath);
	printf("[Common]Output Position X      : %d\n", param->outputSx);
	printf("[Common]Output Position Y      : %d\n", param->outputSy);
	printf("\n");
	printf("[Debug]Debug Mode              : %s\n", opt_str.debugModeStr[param->debugMode]);
	printf("[Debug]NPU Debug Mode          : %s\n", opt_str.debugModeStr[param->npuDebugMode]);
	printf("[Debug]NPU Running Mode        : %s\n", opt_str.npuRunModeStr[param->npuRunMode]);
	printf("----------------Parameter End----------------\n\n");
}

static void NnNormalizeNetworkPath(param_info_t *param, network_index_t index)
{
	if (param->networkPath[index] == NULL)
	{
		return;
	}

	if (strcmp(param->networkPath[index], DEFAULT_HAND_NETWORK_ALIAS) == 0)
	{
		param->networkPath[index] = DEFAULT_HAND_NETWORK_DIR;
	}
}

static void NnprintUsage(void)
{
	printf("---------------- Usage ----------------\n");
    printf("[Network1]Network Path         : -n default[%s]\n", DEFAULT_NETWORK_PATH_1);
    printf("[Network2]Network Path         : -N default[%s]\n", DEFAULT_NETWORK_PATH_2);
	printf("\n");
	printf("[Common]Input Mode             : -i default[%s] option: camera, rtpm, file\n", opt_str.inputModeStr[DEFAULT_INPUT_MODE]);
	printf("[Common]Output Mode            : -o default[%s] option: display, rtpm, file\n", opt_str.outputModeStr[DEFAULT_OUTPUT_MODE]);
	printf("[Common]Input Width            : -w default[%d] max: 1920 \n", DEFAULT_INPUT_WIDTH);
	printf("[Common]Input Height           : -h default[%d] max 1080\n", DEFAULT_INPUT_HEIGHT);
	printf("[Common]Output Width           : -W default[%d] max: 1920\n", DEFAULT_OUTPUT_WIDTH);
	printf("[Common]Output Height          : -H default[%d] max 720\n", DEFAULT_OUTPUT_HEIGHT);
	printf("[Common]Input Path             : -p default[%s]\n", DEFAULT_INPUT_PATH);
	printf("[Common]Output Path            : -P default[%s]\n", DEFAULT_OUTPUT_PATH);
	printf("[Common]Output Position X      : -X default[%d]\n", DEFAULT_OUTPUT_SX);
	printf("[Common]Output Position Y      : -Y default[%d]\n", DEFAULT_OUTPUT_SY);
	printf("\n");
	printf("[Debug]Debug Mode              : -g default[%s] option: off, log, file\n", opt_str.debugModeStr[DEFAULT_DEBUG_MODE]);
	printf("[Debug]NPU Debug Mode          : -u default[%s] option: off, log, file\n", opt_str.npuDebugModeStr[DEFAULT_NPU_DEBUG_MODE]);
	printf("[Debug]NPU Running Mode        : -a default[%s] option: sync, async\n", opt_str.npuRunModeStr[DEFAULT_NPU_RUN_MODE]);
	printf("---------------- Usage ----------------\n\n");
}


static int32_t adjustRes(uint16_t *punXres, uint16_t *punYres) {
	int resfd = -1;

    printf("[Info] reading framebuffer resolution from fb@0\n");
	resfd = open("/proc/device-tree/fb@0/xres", O_RDONLY);
	if(resfd > 0)
	{
		char buf[4];
		uint32_t xRes = 0;
		int bytes_read = read(resfd, buf, sizeof(buf));
		if(bytes_read > 0)
		{
			memcpy(&xRes, buf, 4);
			xRes = __builtin_bswap32(xRes);
		}

		*punXres = (xRes >= *punXres)? *punXres : xRes;
		close(resfd);
	} else {
		return -1;
	}	

	resfd = open("/proc/device-tree/fb@0/yres", O_RDONLY);
	if(resfd > 0)
	{
		char buf[4];
		uint32_t yRes = 0;
		int bytes_read = read(resfd, buf, sizeof(buf));
		if(bytes_read > 0)
		{
			memcpy(&yRes, buf, 4);
			yRes = __builtin_bswap32(yRes);
		}
		*punYres = (yRes >= *punYres)? *punYres : yRes;
		close(resfd);
	} else {
		return -1;
	}	

	printf("####### finall Witdh = %d and Height=%d\n", *punXres, *punYres);
	return 0;
}

static void NnModeChecker(param_info_t *param)
{

	if(param->inputMode == INPUT_MODE_RTPM && param->outputMode != OUTPUT_MODE_RTPM)
	{
		printf("When the Input Mode is set to RTPM, the Output Mode cannot be set to anything other than RTPM.\n");

		exit(0);
	}
	else if(param->inputMode == INPUT_MODE_FILE && param->outputMode == OUTPUT_MODE_RTPM)
	{
		printf("When the Input Mode is set to File, it is not possible to set the Output Mode to RTPM.\n");

		exit(0);
	}

	if(param->inputMode == INPUT_MODE_FILE)
	{
		const char *suffix = ".png";
		size_t str_len = strlen(param->inputPath);
		size_t suffix_len = strlen(suffix);
		const char *start = param->inputPath + str_len - suffix_len;
		FILE *file = fopen(param->inputPath, "r");

		if(strcmp(start, suffix) != 0)
		{
			printf("The extension of the input file must be png. If you have not set the input path, use the [-p ./filePath.png] option.\n");

			exit(0);
		}
		else if(file == NULL)
		{
			printf("Input file does not exist.\n");

			exit(0);
		}

		fclose(file);
	}

	if(param->outputMode == OUTPUT_MODE_FILE)
	{
		const char *suffix = ".png";
		size_t str_len = strlen(param->outputPath);
		size_t suffix_len = strlen(suffix);
		const char *start = param->outputPath + str_len - suffix_len;

		if(strcmp(start, suffix) != 0)
		{
			printf("The extension of the output file must be png. If you have not set the output path, use the [-P ./filePath.png] option.\n");

			exit(0);
		}
	}
}

static void NnInitAppContext(app_context_t *pContext, param_info_t *pParam)
{
	pContext->inferenceContext.neuralNetworkCnt = pParam->networkCnt;

	memset(&pContext->inferenceContext.neuralNetwork[NETWORK_INDEX_0], 0x0, sizeof(network_context_t));
	memset(&pContext->inferenceContext.neuralNetwork[NETWORK_INDEX_1], 0x0, sizeof(network_context_t));
	pContext->inferenceContext.neuralNetwork[NETWORK_INDEX_0].networkPath = pParam->networkPath[NETWORK_INDEX_0];
	pContext->inferenceContext.neuralNetwork[NETWORK_INDEX_1].networkPath = (pParam->networkCnt > 1) ? pParam->networkPath[NETWORK_INDEX_1] : NULL;

	//TODO: npuFD init
	pContext->debugMode = pParam->debugMode;
	pContext->inferenceContext.npuDebugMode = pParam->npuDebugMode;
	pContext->inferenceContext.npuRunMode = pParam->npuRunMode;

	pContext->memory_context.displayMemoryFd = 0;
	pContext->memory_context.fileMemoryFd = 0;

	pContext->phy_base_input = 0;
	pContext->map_base_input = NULL;
	pContext->memory_context.phy_base_output[0] = 0;
	pContext->memory_context.phy_base_output[1] = 0;
	pContext->memory_context.map_base_output[0] = NULL;
	pContext->memory_context.map_base_output[1] = NULL;

	pContext->phy_base_output_idx = 0;

	//TODO //reserved mem variable init

	pContext->inputDataType = pParam->inputMode;
	pContext->inputImageFormat = pParam->inputFormat;
	pContext->inputWidth = pParam->inputWidth;
	pContext->inputHeight = pParam->inputHeight;
	pContext->inputPath = pParam->inputPath;

	if(pContext->inputDataType == INPUT_MODE_CAMERA)
	{
		pContext->camCaptureRetryCnt = CAM_RETRY_CNT;
	}
	else if(pContext->inputDataType == INPUT_MODE_FILE)
	{
		// pContext->inputPath = pParam->inputPath;

	}
	else if(pContext->inputDataType == INPUT_MODE_RTPM)
	{
		pContext->inputRtpmBufferCount = 4;
	}
	else
	{
		exit(1);
	}

	pContext->outputDataType = pParam->outputMode;
	pContext->outputImageFormat = pParam->outputFormat;
	pContext->outputPath = pParam->outputPath;

	if(pContext->outputDataType == OUTPUT_MODE_LCD)
	{
		pContext->outputLcdSx = pParam->outputSx;
		pContext->outputLcdSy = pParam->outputSy;
	}
	else if(pContext->outputDataType == OUTPUT_MODE_FILE)
	{
		// pContext->outputPath = pParam->outputPath;

	}
	else if(pContext->outputDataType == OUTPUT_MODE_RTPM)
	{
		pContext->outputRtpmBufferCount = 4;
		pContext->pRtpmIpAdress = pParam->pRtpmIpAdress;
		pContext->streamPort = RTPM_STEAM_PORT;
		pContext->messagePort = RTPM_MESSAGE_PORT;
	}
	else
	{
		exit(1);
	}

	pContext->outputWidth = pParam->outputWidth;
	pContext->outputHeight = pParam->outputHeight;
}

static float NnGetFPS()
{
	static struct timeval bgn, end;
	float fps;

	gettimeofday(&end, NULL);
	fps = 1000.0 / (((end.tv_sec - bgn.tv_sec) * 1000.0) + ((end.tv_usec - bgn.tv_usec) / 1000.0));
	gettimeofday(&bgn, NULL);

	return fps;
}

static void NnInputModeInit(app_context_t *pContext, CameraHandle CameraHandle, MessageHandle msgHandle)
{
	int32_t ret;
	input_data_type_t inputMode = pContext->inputDataType;

	// Setting Input Mode
	if(inputMode == INPUT_MODE_CAMERA)
	{
		// Initialize Camera Driver
		ret = CameraOpenDevice(CameraHandle, pContext->inputPath);
		if (ret < 0)
		{
			NN_LOG("[ERROR] [CameraOpenDevice] %s error : %d\n", pContext->inputPath, ret);
			exit(1);
		}
		else
		{
			NN_LOG("[INFO] [CameraOpenDevice] %s successs : %d\n", pContext->inputPath, ret);
		}

		ret = CameraSetConfig(CameraHandle, pContext->inputWidth, pContext->inputHeight);
		if (ret < 0)
		{
			NN_LOG("[ERROR] [CameraSetConfig] error : %d\n", ret);
			exit(1);
		}
		else
		{
			NN_LOG("[INFO] [CameraSetConfig] success : %d\n", ret);
		}
	}
	else if(inputMode == INPUT_MODE_FILE)
	{
		pContext->phy_base_input = pContext->memory_context.reservedMemory[3][2];
		pContext->map_base_input = (uint8_t *)mmap(NULL, PMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, pContext->memory_context.displayMemoryFd, pContext->phy_base_input);
		if(pContext->map_base_input == MAP_FAILED)
		{
			NN_LOG("[ERROR] [%s] mmap fail file input buffer\n", __FUNCTION__);
			exit(1);
		}
		else
		{
			NN_LOG("[INFO] [%s] mmap success file input buffer\n", __FUNCTION__);
		}
	}
	else if(inputMode == INPUT_MODE_RTPM)
	{
		uint8_t colorFormatBytes;
		uint32_t rtpmFrameSize;
		uint8_t rtpmBufferCount;
		uint64_t recvPhyAddrAry[4] = {pContext->memory_context.reservedMemory[0][0], pContext->memory_context.reservedMemory[0][1], pContext->memory_context.reservedMemory[0][2], pContext->memory_context.reservedMemory[0][3]};
		uint16_t rtpmTargetStreamPort;
		uint16_t rtpmTargetMessagePort;
		char * pRtpmTargetipAddress;

		if(pContext->inputImageFormat == IMAGE_FMT_RGB24)
		{
			colorFormatBytes = 3;
		}
		else if(pContext->inputImageFormat == IMAGE_FMT_RGBA32)
		{
			colorFormatBytes = 4;
		}
		else
		{
			NN_LOG("[INFO] [%s] invalid color format\n", __FUNCTION__);
			exit(EXIT_FAILURE);
		}

		rtpmFrameSize = pContext->inputWidth * pContext->inputHeight * colorFormatBytes;
		rtpmBufferCount = pContext->inputRtpmBufferCount;
		pRtpmTargetipAddress = pContext->pRtpmIpAdress;
		rtpmTargetStreamPort = pContext->streamPort;
		rtpmTargetMessagePort = pContext->messagePort;

		ret = MessageOpen(msgHandle, MESSAGE_STREAM_RECV, recvPhyAddrAry, rtpmBufferCount, rtpmFrameSize, pRtpmTargetipAddress, rtpmTargetStreamPort, rtpmTargetMessagePort);
		if (ret < 0)
		{
			printf("[Message Open] error : %d\n", ret);
			exit(EXIT_FAILURE);
		}
		else
		{
			NN_LOG("[Message Open] is success\n", __FUNCTION__);
		}
	}
	else
	{
		NN_LOG("[Error] Invaild input mode\n", __FUNCTION__);
	}
	printf("Input mode init Done\n");
}

static void NnInputModeDeinit(app_context_t *pContext, CameraHandle CameraHandle, MessageHandle msgHandle)
{
	if(pContext->inputDataType == INPUT_MODE_CAMERA)
	{
		CameraCloseDevice(CameraHandle);
	}
	else if(pContext->inputDataType == INPUT_MODE_FILE)
	{
		// TODO:
	}
	else if(pContext->inputDataType == INPUT_MODE_RTPM)
	{
		MessageClose(msgHandle);
	}
}

static void NnOutputModeInit(app_context_t *pContext, DisplayHandle dispHandle, MessageHandle msgHandle)
{
	int32_t ret = -1;
	input_data_type_t input_mode = pContext->inputDataType;
	output_data_type_t output_mode = pContext->outputDataType;

	/* Setup initialize for output mode */
	switch (output_mode)
	{
		case OUTPUT_MODE_RTPM:
			uint8_t colorFormatBytes;
			uint32_t rtpmFrameSize;
			uint8_t rtpmBufferCount;
			uint64_t sendPhyAddrAry[4] = {pContext->memory_context.reservedMemory[1][0], pContext->memory_context.reservedMemory[1][1], pContext->memory_context.reservedMemory[1][2], pContext->memory_context.reservedMemory[1][3]};
			uint16_t rtpmTargetStreamPort;
			uint16_t rtpmTargetMessagePort;
			char * pRtpmTargetipAddress;

			if(pContext->outputImageFormat == IMAGE_FMT_RGB24)
			{
				colorFormatBytes = 3;
			}
			else if(pContext->outputImageFormat == IMAGE_FMT_RGBA32)
			{
				colorFormatBytes = 4;
			}
			else
			{
				printf("[error] [%s] invalid color format\n", __FUNCTION__);
				exit(EXIT_FAILURE);
			}

			rtpmFrameSize = pContext->outputWidth * pContext->outputHeight * colorFormatBytes;
			rtpmBufferCount = pContext->outputRtpmBufferCount;
			pRtpmTargetipAddress = pContext->pRtpmIpAdress;
			rtpmTargetStreamPort = pContext->streamPort;
			rtpmTargetMessagePort = pContext->messagePort;
			if(input_mode == INPUT_MODE_CAMERA)
			{
				ret = MessageOpen(msgHandle, MESSAGE_STREAM_SEND, sendPhyAddrAry, rtpmBufferCount, rtpmFrameSize, pRtpmTargetipAddress, rtpmTargetStreamPort, rtpmTargetMessagePort);
				if (ret < 0)
				{
					printf("[error] Message Open error : %d\n", ret);
					exit(EXIT_FAILURE);
				}
				else
				{
					printf("[INFO] Message Open succcess : %d\n", ret);
				}
			}
			else
			{
				// printf("Injection Mode\n");
			}
			break;

		case OUTPUT_MODE_LCD:
			// Initialize Display Driver
			ret = DisplayOpenDevice(dispHandle, pContext->outputPath);
			if (ret < 0)
			{
				printf("[ERROR] DisplayOpenDevice() error : %s \n", (char *)pContext->outputPath);
				exit(EXIT_FAILURE);
			}
			else
			{
				NN_LOG("[INFO] DisplayOpenDevice() success : %s \n", (char *)pContext->outputPath);
			}
			break;

		case OUTPUT_MODE_FILE:
			// TBD
			break;

		default:
			// Handle unknown output mode
			printf("[ERROR] Unknown output mode: %d\n", output_mode);
			exit(EXIT_FAILURE);
	}
	printf("Output mode init Done\n");
}

static void NnOutputModeDeinit(app_context_t *pContext, DisplayHandle dispHandle, MessageHandle msgHandle)
{
	// Deinitialize Output
	if(pContext->outputDataType == OUTPUT_MODE_LCD)
	{
		DisplayCloseDevice(dispHandle);
		printf("The Display Device is close done.\n");
	}
	else
	{
		printf("The output is not in LCD mode\n");
	}

	if(pContext->outputDataType == OUTPUT_MODE_RTPM)
	{
		if(pContext->inputDataType != INPUT_MODE_RTPM)
		{
			MessageClose(msgHandle);
		}
		else
		{
			// TBD
		}
	}
	else
	{
		// TBD
	}
}

static int32_t NnScalerInit(ScalerHandle handle)
{
	int32_t ret = -1;

	// Initialize Scaler Driver
	ret = ScalerOpenDevice(handle, SCALER_DEV_NAME_0, SCALER_INDEX_0);
	if (ret < 0)
	{
		NN_LOG("[ERROR] ScalerOpenDevice() error : /dev/scaler1 \n");
		exit(EXIT_FAILURE);
	}
	else
	{
		NN_LOG("[INFO] ScalerOpenDevice() success : /dev/scaler1 \n");
	}

	ret = ScalerOpenDevice(handle, SCALER_DEV_NAME_1, SCALER_INDEX_1);
	if (ret < 0)
	{
		NN_LOG("[ERROR] ScalerOpenDevice() error : /dev/scaler3 \n");
		exit(EXIT_FAILURE);
	}
	else
	{
		NN_LOG("[INFO] ScalerOpenDevice() success : /dev/scaler3 \n");
	}

	return ret;
}

static void NnScalerDenit(ScalerHandle handle)
{
	// Deinitialize Scaler Driver
	ScalerCloseDevice(handle, SCALER_INDEX_0);
	ScalerCloseDevice(handle, SCALER_INDEX_1);
}

static void NnGetFrame(app_context_t *pContext, CameraHandle CameraHandle, MessageHandle msgHandle)
{
	int32_t sizeRet = -1;
	input_data_type_t inputMode = pContext->inputDataType;
	int32_t retryCnt;

	if(inputMode == INPUT_MODE_CAMERA)
	{
		retryCnt = pContext->camCaptureRetryCnt;

		while((sizeRet <= 0) && (retryCnt > 0))
		{
			sizeRet = CameraGetBuffer(CameraHandle, &pContext->map_base_input, &pContext->phy_base_input);
			if(sizeRet <= 0)
			{
				usleep(1000);
				retryCnt--;
			}
			else
			{
				retryCnt = CAM_RETRY_CNT;
				break;
			}
		}
	}
	else if(inputMode == INPUT_MODE_FILE)
	{
		sizeRet = cvLoadImage(pContext->map_base_input, pContext->inputPath, &(pContext->inputWidth), &(pContext->inputHeight));
	}
	else if(inputMode == INPUT_MODE_RTPM)
	{
		sizeRet = MessagePopReceiveBuffer(msgHandle, &pContext->map_base_input, &pContext->phy_base_input, &syncStamp);
	}
	else
	{
		NN_LOG("[ERROR] Invalid inputMode=%d \n", inputMode);
		exit(0);
	}
}

static int32_t NnReleaseFrame(app_context_t *pContext, CameraHandle cameraHandle, MessageHandle msgHandle)
{
	int32_t ret = -1;
	input_data_type_t input_mode = pContext->inputDataType;
	output_data_type_t output_mode = pContext->outputDataType;

	if(input_mode == INPUT_MODE_CAMERA)
	{
		ret = CameraReleaseBuffer(cameraHandle);
	}
	else if(input_mode == INPUT_MODE_FILE)
	{
		//TBD
		ret = true;
	}
	else if(input_mode == INPUT_MODE_RTPM)
	{
		ret = MessagePushReceiveBuffer(msgHandle, pContext->phy_base_input);
	}
	else{
		//TBD
		ret = true;
	}

	if(output_mode == OUTPUT_MODE_LCD)
	{
		//TBD
		ret = true;
	}
	else if(output_mode == OUTPUT_MODE_FILE)
	{
		NnSetSignalFlag(true);
		ret = true;
	}
	else if(output_mode == OUTPUT_MODE_RTPM)
	{
		//TBD
		ret = true;
	}
	else{
		//TBD
		ret = true;
	}

	return ret;
}


static void NnResizeInputFrame(app_context_t *pContext, ScalerHandle handle)
{
	uint8_t nnCnt = 0;
	scaler_size_info_t scalerSrc;
	scaler_size_info_t scalerDes;
	scaler_index_t scalerIdx;

	scalerSrc.pmap = pContext->phy_base_input;
	scalerSrc.width = pContext->inputWidth;
	scalerSrc.height = pContext->inputHeight;
	if(pContext->inputImageFormat == IMAGE_FMT_RGBA32)
	{
		scalerSrc.format = SCALER_FORMAT_ARGB8888;
	}
	else
	{
		scalerSrc.format = SCALER_FORMAT_RGB888;
	}

	nnCnt = pContext->inferenceContext.neuralNetworkCnt;

	for(uint8_t i = 0; i < nnCnt; i++)
	{
		scalerIdx = pContext->inferenceContext.neuralNetwork[i].scalerIdx;

		scalerDes.pmap = pContext->inferenceContext.neuralNetwork[i].inputBuf->paddr;
		scalerDes.width = pContext->inferenceContext.neuralNetwork[i].nnWidth;
		scalerDes.height = pContext->inferenceContext.neuralNetwork[i].nnHeight;
		if(pContext->outputImageFormat == IMAGE_FMT_RGBA32)
		{
			scalerDes.format = SCALER_FORMAT_ARGB8888;
		}
		else
		{
			scalerDes.format = SCALER_FORMAT_RGB888;
		}

		ScalerResize(handle, scalerIdx, scalerSrc, scalerDes);
		ScalerPoll(handle, scalerIdx);
	}
}

static void NnResizeOutputFrame(app_context_t *pContext, ScalerHandle scalerHandle, MessageHandle msgHandle)
{
	scaler_size_info_t scalerSrc;
	scaler_size_info_t scalerDes;
	uint32_t outputIdx;
	input_data_type_t input_mode = pContext->inputDataType;
	output_data_type_t output_mode = pContext->outputDataType;

	scalerSrc.pmap = pContext->phy_base_input;
	scalerSrc.width = pContext->inputWidth;
	scalerSrc.height = pContext->inputHeight;
	if(pContext->inputImageFormat == IMAGE_FMT_RGBA32)
	{
		scalerSrc.format = SCALER_FORMAT_ARGB8888;
	}
	else
	{
		scalerSrc.format = SCALER_FORMAT_RGB888;
	}

	outputIdx = pContext->phy_base_output_idx;
	if(output_mode == OUTPUT_MODE_RTPM)
	{
		if(input_mode == INPUT_MODE_CAMERA)
		{
			MessagePopSendBuffer(msgHandle, &pContext->memory_context.map_base_output[outputIdx], &pContext->memory_context.phy_base_output[outputIdx], &pContext->inputRtpmBufferIndex);
		}
	}

	scalerDes.pmap = pContext->memory_context.phy_base_output[outputIdx];
	scalerDes.width = pContext->outputWidth;
	scalerDes.height = pContext->outputHeight;
	if(pContext->outputImageFormat == IMAGE_FMT_RGBA32)
	{
		scalerDes.format = SCALER_FORMAT_ARGB8888;
	}
	else
	{
		scalerDes.format = SCALER_FORMAT_RGB888;
	}

	ScalerResize(scalerHandle, pContext->outputScalerIdx, scalerSrc, scalerDes);
	ScalerPoll(scalerHandle, pContext->outputScalerIdx);
}

static void NnDrawResult(app_context_t *pContext)
{
	unsigned char *outputMapBase = NULL;
	uint16_t outputWidth = 0;
	uint16_t outputHeight = 0;
	int32_t netIdx = 0;
	int32_t netCnt = 0;
	network_context_t *pNeuralNetwork = NULL;
	uint32_t outputIdx = 0;
	double fontSize = 0.8;
	int32_t textOffset = 5;
	Color_t redColor = RGB(255, 0, 0);
	Color_t greenColor = RGB(0, 255, 0);
	// Color_t deepBlueColor = RGB(0, 0, 255);
	// Color_t skyBlueColor = RGB(0, 255, 255);
	Color_t whiteColor = RGB(255, 255, 255);
	Color_t customColor = RGB(0, 0, 0);
	int32_t baseXPos = 1600;
	int32_t baseYPos = 100;
	int32_t spacing = 30;
	int32_t currentYPos = baseYPos;

	netCnt = pContext->inferenceContext.neuralNetworkCnt;
	outputIdx = pContext->phy_base_output_idx;
	outputMapBase = pContext->memory_context.map_base_output[outputIdx];
	outputWidth = pContext->outputWidth;
	outputHeight = pContext->outputHeight;

	for(netIdx = 0; netIdx < netCnt; netIdx++)
	{
		pNeuralNetwork = &pContext->inferenceContext.neuralNetwork[netIdx];

		if(pNeuralNetwork->type == TELECHIPS_NPU_POST_CLASSIFIER)
		{
			int32_t xPos = 100;
			int32_t yPos = 150;
			customColor = redColor;

			cvDrawCls(outputMapBase, outputWidth, outputHeight, pNeuralNetwork->resultCls.class_ids[0], xPos, yPos, customColor, fontSize);
		}
		else if(pNeuralNetwork->type == TELECHIPS_NPU_POST_DETECTOR)
		{
			Box_t boundingBoxes[256];
			uint16_t boxImageWidth;
			uint16_t boxImageHeight;
			int32_t boxCount = pNeuralNetwork->resultObj.cnt;
			customColor = greenColor;

			for (int index = 0; index < boxCount; index++)
			{
				boxImageWidth = pNeuralNetwork->resultObj.obj[index].img_w;
				boxImageHeight = pNeuralNetwork->resultObj.obj[index].img_h;
				boundingBoxes[index].cls = pNeuralNetwork->resultObj.obj[index].cls;
				boundingBoxes[index].score  = pNeuralNetwork->resultObj.obj[index].score;
				boundingBoxes[index].xmin = (uint16_t)(pNeuralNetwork->resultObj.obj[index].x_min + 0.5);
				boundingBoxes[index].ymin  = (uint16_t)(pNeuralNetwork->resultObj.obj[index].y_min + 0.5);
				boundingBoxes[index].xmax  = (uint16_t)(pNeuralNetwork->resultObj.obj[index].x_max + 0.5);
				boundingBoxes[index].ymax  = (uint16_t)(pNeuralNetwork->resultObj.obj[index].y_max + 0.5);
			}
			if(boxCount > 0)
			{
				cvDrawBoxes(outputMapBase, boundingBoxes, boxCount, outputWidth, outputHeight, boxImageWidth, boxImageHeight, customColor, customColor, fontSize, textOffset);
			}
		} //===========================수정 위치 =====================
		else if(pNeuralNetwork->type == TELECHIPS_NPU_POST_CUSTOM)
		{
    		Color_t yellow = RGB(255, 255, 0);
    		Color_t cyan   = RGB(0, 255, 255);
    		int d, k;

    		for (d = 0; d < pNeuralNetwork->hand_data.num_det; d++)
    		{
        		hpd_detection_t *det = &pNeuralNetwork->hand_data.dets[d];

        		/* BBox: 모델 입력(640×640) → 출력 해상도 스케일링 */
        		Box_t box;
        		box.cls   = 0;
        		box.score = det->score;
        		box.xmin  = (uint16_t)(det->x1 * outputWidth  / HAND_MODEL_INPUT_WIDTH);
        		box.ymin  = (uint16_t)(det->y1 * outputHeight / HAND_MODEL_INPUT_HEIGHT);
        		box.xmax  = (uint16_t)(det->x2 * outputWidth  / HAND_MODEL_INPUT_WIDTH);
        		box.ymax  = (uint16_t)(det->y2 * outputHeight / HAND_MODEL_INPUT_HEIGHT);
        		cvDrawBoxes(outputMapBase, &box, 1,
                    		outputWidth, outputHeight,
                    		HAND_MODEL_INPUT_WIDTH, HAND_MODEL_INPUT_HEIGHT,
                    		yellow, yellow, fontSize, textOffset);

        /* Keypoints: conf > 0.5인 점만 그리기 */
        		Point_t kptPoints[HPD_NUM_KPT];
        		int kptCount = 0;
        		for (k = 0; k < HPD_NUM_KPT; k++)
        		{
            		if (det->kpts[k].conf > 0.5f)
            		{
                		kptPoints[kptCount].x = (int)(det->kpts[k].x * outputWidth  / HAND_MODEL_INPUT_WIDTH);
                		kptPoints[kptCount].y = (int)(det->kpts[k].y * outputHeight / HAND_MODEL_INPUT_HEIGHT);
                		kptCount++;
            		}
        		}
        		if (kptCount > 0)
        		{
            		cvDrawPoints(outputMapBase, outputWidth, outputHeight,
                         		kptPoints, kptCount, 3, cyan);
        		}
    		}
		} // =========================================수정위치 ================
		else 
		{
			// TELECHIPS_NPU_POST_NONE or Invaild PostProccess...
			;
		}
		cvDrawInfo(outputMapBase, outputWidth, outputHeight, DRAW_INFO_NETWORK, pNeuralNetwork->inferenceTime, netIdx, baseXPos, currentYPos, fontSize, customColor);
		currentYPos += spacing;
		cvDrawInfo(outputMapBase, outputWidth, outputHeight, DRAW_INFO_NPU, pNeuralNetwork->npuUtilization, netIdx, baseXPos, currentYPos, fontSize, customColor);
		currentYPos += spacing;
	}

	//for common draw
	cvDrawInfo(outputMapBase, outputWidth, outputHeight, DRAW_INFO_CPU, pContext->perfContext.pPerfInfo.cpuUtil[0], 0, baseXPos, currentYPos, fontSize, whiteColor);
	currentYPos += spacing;
	cvDrawInfo(outputMapBase, outputWidth, outputHeight, DRAW_INFO_MEMORY, pContext->perfContext.pPerfInfo.memUsage, 0, baseXPos, currentYPos, fontSize, whiteColor);

	cvDrawInfo(outputMapBase, outputWidth, outputHeight, DRAW_INFO_FPS, NnGetFPS(), 0, 100, 100, fontSize, whiteColor);
}

static int32_t NnOutputResultFrame(app_context_t *pContext, DisplayHandle dispHandle, MessageHandle msgHandle)
{
	int32_t sizeRet = -1;
	uint32_t phyBaseOutputIdx = pContext->phy_base_output_idx;
	unsigned char *outputMapBase;
	uint64_t outputPhyBase;
	uint32_t outputWidth;
	uint32_t outputHeight;

	input_data_type_t input_mode = pContext->inputDataType;
	output_data_type_t output_mode = pContext->outputDataType;

	outputMapBase = pContext->memory_context.map_base_output[phyBaseOutputIdx];
	outputPhyBase = pContext->memory_context.phy_base_output[phyBaseOutputIdx];
	outputWidth = pContext->outputWidth;
	outputHeight = pContext->outputHeight;

	switch (output_mode)
	{
		case OUTPUT_MODE_LCD:
			/* LCD mode */
			DisplayShow(dispHandle, outputPhyBase, pContext->outputLcdSx, pContext->outputLcdSy, outputWidth, outputHeight);
			break;

		case OUTPUT_MODE_FILE:
			/* File save mode */
			cvSaveImage(outputMapBase, pContext->outputPath, outputWidth, outputHeight);
			break;

		case OUTPUT_MODE_RTPM:
			if(input_mode == INPUT_MODE_CAMERA)
			{
				MessagePushSendBuffer(msgHandle, outputMapBase, pContext->inputRtpmBufferIndex);
			}
			break;

		default:
			// TBD
			break;
	}

	pContext->phy_base_output_idx = (phyBaseOutputIdx + 1) % 2;

	return sizeRet;
}

static int32_t NnOutputResultData(app_context_t *pContext, MessageHandle msgHandle)
{
	int32_t sizeRet = -1;
	output_data_type_t output_mode = pContext->outputDataType;

	if(output_mode == OUTPUT_MODE_RTPM)
	{
		/* Injection mode */
		int32_t objCount[NETWORK_INDEX_MAX] = {0, };
		boxes_t objs[NETWORK_INDEX_MAX] = {0, };
		message_result_type_t resultType[NETWORK_INDEX_MAX] = {0, };
		bool hasCustomResult = false;

		for(int networkIdx = 0; networkIdx < pContext->inferenceContext.neuralNetworkCnt; networkIdx++)
		{
			network_context_t *neural_network = &pContext->inferenceContext.neuralNetwork[networkIdx];
			enlight_postproc_t network_detect_type = pContext->inferenceContext.neuralNetwork[networkIdx].type;

			switch(network_detect_type)
			{
				case TELECHIPS_NPU_POST_CLASSIFIER:
					RtpmPostProcessClassifierResults(neural_network, resultType, objCount, networkIdx);
					break;

				case TELECHIPS_NPU_POST_DETECTOR:
					RtpmPostProcessDetectionResults(objs, neural_network, objCount, resultType, networkIdx, pContext->outputWidth, pContext->outputHeight);
					break;

				case TELECHIPS_NPU_POST_CUSTOM:
					hasCustomResult = true;
					break;

				default:
					NN_LOG("[INFO][Unknown Enlight Post-processing][%d]\n", network_detect_type);
					break;
			}
		}

		if (hasCustomResult)
		{
			sizeRet = RtpmSendHandResultAsJson(msgHandle,
											  syncStamp,
											  &pContext->inferenceContext.neuralNetwork[NETWORK_INDEX_0],
											  pContext->outputWidth,
											  pContext->outputHeight);
		}
		else
		{
			sizeRet = RtpmSendResultDataAsJson(msgHandle,
												syncStamp,
												resultType[NETWORK_INDEX_0],(uint16_t)objCount[NETWORK_INDEX_0], objs[NETWORK_INDEX_0].boxes,
												resultType[NETWORK_INDEX_1], (uint16_t)objCount[NETWORK_INDEX_1], objs[NETWORK_INDEX_1].boxes);
		}

		if(sizeRet < 0)
		{
			NN_LOG("[ERROR] [RtpmSendResultDataAsJson] size : %d\n", sizeRet);
		}
		else
		{
			NN_LOG("[INFO] [RtpmSendResultDataAsJson] size : %d\n", sizeRet);
		}

		sizeRet = RtpmSendInferencePerformanceToRTPM(msgHandle, &pContext->inferenceContext);
		if(sizeRet < 0)
		{
			NN_LOG("[ERROR] [RtpmSendInferencePerformanceToRTPM] size : %d\n", sizeRet);
		}
		else
		{
			NN_LOG("[INFO] [RtpmSendInferencePerformanceToRTPM] size : %d\n", sizeRet);
		}
	}
	else if(output_mode == OUTPUT_MODE_FILE)
	{
		/* File mode */
		// TODO : Save yolo format or coco format
	}
	else /* OUTPUT_MODE_LCD mode */
	{
		;
	}

	return sizeRet;
}

static void NnPerfMonitorInit(app_context_t *pContext, MessageHandle msgHandle)
{
	createSystemMonitorThread(&pContext->perfContext);				// Init Performance Monitor

	if(pContext->inferenceContext.npuRunMode == NPU_RUN_ASYNC) //profile mode, NPU CNT set to zero...
	{
		createNpuResourcesMonitorThread(&pContext->inferenceContext);
	}
	else
	{
		NN_LOG("[INFO] [%s] Monitoring of NPU usage cannot be used in sync mode.\n", __FUNCTION__);
	}

	if(pContext->inputDataType == INPUT_MODE_RTPM || pContext->outputDataType == OUTPUT_MODE_RTPM)
	{
		RtpmCreatePerfmanceDataSendThread(msgHandle);
	}
	else
	{
		NN_LOG("[INFO] [%s] Input/Output mode is not RTPM. Performance data send thread not created.\n", __FUNCTION__);
	}
}

static void NnPerfMonitorDeinit(app_context_t *pContext)
{
	destroySystemMonitorThread();

	if(pContext->inferenceContext.npuRunMode == NPU_RUN_ASYNC) //profile mode, NPU CNT set to zero...
	{
		destroyNpuResourcesMonitorThread();
	}
	else
	{
		NN_LOG("[INFO] [%s] Monitoring of NPU usage cannot be used in sync mode.\n", __FUNCTION__);
	}

	if(pContext->inputDataType == INPUT_MODE_RTPM || pContext->outputDataType == OUTPUT_MODE_RTPM)
	{
		RtpmDestroyPerfmanceDataSendThread();
	}
	else
	{
		NN_LOG("[INFO] [%s] Input/Output mode is not RTPM. Performance data send thread not destruction.\n", __FUNCTION__);
	}
}

static int32_t NnCreateAPI(app_context_t *pContext, app_obj_t *pObj)
{
	int32_t ret = -1;

	if(pContext->inputDataType == INPUT_MODE_CAMERA)
	{
		ret = CameraCreate(&pObj->cam_handle);
		if (ret < 0)
		{
			NN_LOG("[ERROR] [Camera Create] fail : %d\n", ret);
			exit(1);
		}
		else
		{
			NN_LOG("[INFO] [Camera Create] successs : %d\n", ret);
		}
	}
	else
	{
		NN_LOG("[INFO] Camera capture mode is not activated\n");
	}

	ret = ScalerCreate(&pObj->scaler_handle);
	if (ret < 0)
	{
		NN_LOG("[ERROR] ScalerCreate() error\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		NN_LOG("[INFO] ScalerCreate() success\n");
	}

	if(pContext->outputDataType == OUTPUT_MODE_LCD)
	{
		ret = DisplayCreate(&pObj->display_handle);
		if (ret < 0)
		{
			printf("[ERROR] Display create error\n");
			exit(EXIT_FAILURE);
		}
		else
		{
			NN_LOG("[INFO] Display create success\n");
		}
	}
	else
	{
		NN_LOG("[INFO] LCD Output mode is not activated\n");
	}

	if(pContext->inputDataType == INPUT_MODE_RTPM || pContext->outputDataType == OUTPUT_MODE_RTPM)
	{
		ret = MessageCreate(&pObj->msg_handle);
		if (ret < 0)
		{
			printf("[ERROR] Message create error\n");
			exit(EXIT_FAILURE);
		}
		else
		{
			NN_LOG("[INFO] Message create success\n");
		}
	}
	else
	{
		NN_LOG("[INFO] RTPM mode is not activated\n");
	}

	return ret;
}

static int32_t NnDestroyAPI(app_context_t *pContext, app_obj_t *pObj)
{
	int32_t ret = -1;

	if(pContext->inputDataType == INPUT_MODE_CAMERA)
	{
		ret = CameraDestroy(pObj->cam_handle);
		printf("[INFO] Camera capture mode destruction Done\n");
	}
	else
	{
		NN_LOG("[INFO] Camera capture mode is not activated\n");
	}

	if(pContext->outputDataType == OUTPUT_MODE_LCD)
	{
		ret = DisplayDestroy(pObj->display_handle);
		printf("[INFO] LCD Output mode destruction Done\n");
	}
	else
	{
		NN_LOG("[INFO] LCD Output mode is not activated\n");
	}

	ret = ScalerDestroy(pObj->scaler_handle);
	if(ret < 0)
	{
		printf("[INFO] Scaler destruction Fail\n");
	}
	else
	{
		NN_LOG("[INFO] Scaler destruction Done\n");
	}

	if(pContext->inputDataType == INPUT_MODE_RTPM || pContext->outputDataType == OUTPUT_MODE_RTPM)
	{
		ret = MessageDestroy(pObj->msg_handle);
		printf("[INFO] RTPM mode destruction Done\n");
	}
	else
	{
		NN_LOG("[INFO] RTPM mode is not activated\n");
	}

	return ret;
}

int main(int argc, char **argv)
{
	app_context_t *pContext = NULL;
	param_info_t *pParam = NULL;
	app_obj_t *pObj = &g_AppObj;

	pContext = (app_context_t *)malloc(sizeof(app_context_t));
	pParam = (param_info_t *)malloc(sizeof(param_info_t));

	/* Initialize */
	NnparseArgs(pParam, argc, argv);																				// Parsing parameters
	NnModeChecker(pParam);																							// Check mode
	NnInitAppContext(pContext, pParam);																				// Init App Context
	NnSignalChecker();																								// Handle Segfault
	NnSetDebugMode(pContext->debugMode, pContext->inferenceContext.npuDebugMode);									// Set Debug Log Level
	NnCreateAPI(pContext, pObj);																					// Create API handle
	NnMemoryInit(&pContext->memory_context, pContext->inputPath, pContext->outputWidth, pContext->outputHeight);	// Init Memory
	NnInputModeInit(pContext, pObj->cam_handle, pObj->msg_handle);													// Init input mode
	NnOutputModeInit(pContext, pObj->display_handle, pObj->msg_handle);												// Init output mode
	NnScalerInit(pObj->scaler_handle);																				// Init Scaler

	NnPerfMonitorInit(pContext, pObj->msg_handle);

#if defined(TCC7500) || defined(TCC7501)          // Dual Cluster
	/* The initialization order of NPU_CLUSTER_INDEX_0 and NPU_CLUSTER_INDEX_1 must not be changed. */
	NnNpuInit(&pContext->inferenceContext, NPU_CLUSTER_INDEX_0);
	NnNeuralNetworkInit(&pContext->inferenceContext, NPU_CLUSTER_INDEX_0 ,NETWORK_INDEX_0, SCALER_INDEX_0, IMAGE_FMT_RGB24); // Init Network, set pipeline, npu cluster - network
	if (pContext->inferenceContext.neuralNetworkCnt > 1)
	{
		NnNpuInit(&pContext->inferenceContext, NPU_CLUSTER_INDEX_1);
		NnNeuralNetworkInit(&pContext->inferenceContext, NPU_CLUSTER_INDEX_1, NETWORK_INDEX_1, SCALER_INDEX_1, IMAGE_FMT_RGB24); // Init Network, set pipeline, npu cluster - network
	}
#else                                             // Single Cluster
	NnNpuInit(&pContext->inferenceContext, NPU_CLUSTER_INDEX_0);

	NnNeuralNetworkInit(&pContext->inferenceContext, NPU_CLUSTER_INDEX_0 ,NETWORK_INDEX_0, SCALER_INDEX_0, IMAGE_FMT_RGB24); // Init Network, set pipeline, npu cluster - network
	if (pContext->inferenceContext.neuralNetworkCnt > 1)
	{
		NnNeuralNetworkInit(&pContext->inferenceContext, NPU_CLUSTER_INDEX_0, NETWORK_INDEX_1, SCALER_INDEX_1, IMAGE_FMT_RGB24); // Init Network, set pipeline, npu cluster - network
	}
#endif

#ifdef INTERACTIVE_MODE
	pthread_create(&g_InteractiveThread, NULL, NnInteractive, NULL);
#endif // _INTERACTIVE_MODE

	/* Run */
	while(NnCheckExitFlag() != true)
	{
		// Step 1. Get frame: 0.1ms
		NnGetFrame(pContext, pObj->cam_handle, pObj->msg_handle);
		// Step 2. Resize frame for inference: 10ms
		NnResizeInputFrame(pContext, pObj->scaler_handle);
		// Step 3. Run infernce: 15 ~ 20ms, yolov5 + mbv2
		NnCreateInferenceThread(&pContext->inferenceContext);
		// Step 4. Resize frame for output device: 5ms + depend on tcp send
		NnResizeOutputFrame(pContext, pObj->scaler_handle, pObj->msg_handle);
		// Step 5. Draw result
		NnDrawResult(pContext);
		// Step 6. Output Frame
		NnOutputResultFrame(pContext, pObj->display_handle, pObj->msg_handle);
		// Step 7. Output Data
		NnOutputResultData(pContext, pObj->msg_handle);
		// Step 8. Release Frame
		NnReleaseFrame(pContext, pObj->cam_handle, pObj->msg_handle);
	}

	/* Deinitialize */
	NnNeuralNetworkDeinit(&pContext->inferenceContext, NETWORK_INDEX_0);
	if (pContext->inferenceContext.neuralNetworkCnt > 1)
	{
		NnNeuralNetworkDeinit(&pContext->inferenceContext, NETWORK_INDEX_1);
	}
	NnNpuDeinit(&pContext->inferenceContext);
	NnScalerDenit(pObj->scaler_handle);
	NnInputModeDeinit(pContext, pObj->cam_handle, pObj->msg_handle);
	NnOutputModeDeinit(pContext, pObj->display_handle, pObj->msg_handle);
	NnMemoryDeinit(&pContext->memory_context, pContext->outputWidth, pContext->outputHeight);

	NnDestroyInferenceThread();

	NnPerfMonitorDeinit(pContext);

	// API Deinitialize
	NnDestroyAPI(pContext, pObj);

#ifdef INTERACTIVE_MODE
	pthread_join(g_InteractiveThread, NULL);
#endif // _INTERACTIVE_MODE

	free(pParam);
	free(pContext);

	printf("tc-nn-app finish\n");

	return 0;
}



