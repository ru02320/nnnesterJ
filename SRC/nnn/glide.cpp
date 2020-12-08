
#include <glide.h>

void RenderGlide (SSurface Src, SSurface Dst, RECT *);

typedef struct
{
	bool8	voodoo_present;
	GrVertex	sq[4];
	GrTexInfo	texture;
	int32	texture_mem_size;
	int32	texture_mem_start;
	float	x_offset, y_offset;
	float	x_scale, y_scale;
	float	voodoo_width;
	float	voodoo_height;
} GlideData;

bool8 S9xGlideEnable (bool8 enable);
void S9xGlideDeinit ();
bool8 S9xGlideInit ();
bool8 S9xVoodooInitialise ();


typedef void (FX_CALL *tpColorCombine) (
			   GrCombineFunction_t function, GrCombineFactor_t factor,
			   GrCombineLocal_t local, GrCombineOther_t other,
			   FxBool invert );
static tpColorCombine pgrColorCombine = NULL;

typedef void (FX_CALL *tpTexCombine) (
			 GrChipID_t tmu,
			 GrCombineFunction_t rgb_function,
			 GrCombineFactor_t rgb_factor, 
			 GrCombineFunction_t alpha_function,
			 GrCombineFactor_t alpha_factor,
			 FxBool rgb_invert,
			 FxBool alpha_invert
			 );
static tpTexCombine pgrTexCombine = NULL;

typedef void (FX_CALL *tpTexMipMapMode) (
				 GrChipID_t	 tmu, 
				 GrMipMapMode_t mode,
				 FxBool		 lodBlend );
static tpTexMipMapMode pgrTexMipMapMode = NULL;

typedef void (FX_CALL *tpTexClampMode) (
			   GrChipID_t tmu,
			   GrTextureClampMode_t s_clampmode,
			   GrTextureClampMode_t t_clampmode
			   );
static tpTexClampMode pgrTexClampMode = NULL;

typedef void (FX_CALL *tpTexFilterMode) (
				GrChipID_t tmu,
				GrTextureFilterMode_t minfilter_mode,
				GrTextureFilterMode_t magfilter_mode
				);
static tpTexFilterMode pgrTexFilterMode = NULL;

typedef FxU32 (FX_CALL *tpTexTextureMemRequired) (
				FxU32	 evenOdd,
				GrTexInfo *info   );

static tpTexTextureMemRequired pgrTexTextureMemRequired = NULL;

typedef FxU32 (FX_CALL *tpTexMinAddress) (
				GrChipID_t);
static tpTexMinAddress pgrTexMinAddress = NULL;

typedef void (FX_CALL *tpGlideInit) ();

static tpGlideInit pgrGlideInit = NULL;

typedef FxBool (FX_CALL *tpQueryHardware) (
				 GrHwConfiguration *hwconfig);
static tpQueryHardware pgrSstQueryHardware = NULL;

typedef void (FX_CALL *tpSelect) (int);
static tpSelect pgrSstSelect = NULL;

typedef FxBool (FX_CALL *tpWinOpen) (
		  FxU32				hWnd,
		  GrScreenResolution_t screen_resolution,
		  GrScreenRefresh_t	refresh_rate,
		  GrColorFormat_t	  color_format,
		  GrOriginLocation_t   origin_location,
		  int				  nColBuffers,
		  int				  nAuxBuffers);
static tpWinOpen pgrSstWinOpen = NULL;

typedef void (FX_CALL *tpBufferClear) (
			   GrColor_t color, GrAlpha_t alpha, FxU16 depth);
static tpBufferClear pgrBufferClear = NULL;

typedef FxU32 (FX_CALL *tpScreenWidth) ();
static tpScreenWidth pgrSstScreenWidth = NULL;

typedef FxU32 (FX_CALL *tpScreenHeight) ();
static tpScreenHeight  pgrSstScreenHeight = NULL;

typedef void (FX_CALL *tpGlideShutdown) ();
static tpGlideShutdown pgrGlideShutdown = NULL;

typedef void (FX_CALL *tpTexDownloadMipMapLevel) (
			   GrChipID_t		tmu,
			   FxU32			 startAddress,
			   GrLOD_t		   thisLod,
			   GrLOD_t		   largeLod,
			   GrAspectRatio_t   aspectRatio,
			   GrTextureFormat_t format,
			   FxU32			 evenOdd,
			   void			  *data );

static tpTexDownloadMipMapLevel pgrTexDownloadMipMapLevel = NULL;

typedef void (FX_CALL *tpTexSource) (
			   GrChipID_t tmu,
			   FxU32	  startAddress,
			   FxU32	  evenOdd,
			   GrTexInfo  *info );
static tpTexSource pgrTexSource = NULL;

typedef void (FX_CALL *tpDrawTriangle) (
			   const GrVertex *a, 
			   const GrVertex *b, 
			   const GrVertex *c );
static tpDrawTriangle pgrDrawTriangle = NULL;

typedef void (FX_CALL *tpBufferSwap) (
			   int swap_interval);
static tpBufferSwap pgrBufferSwap = NULL;


GlideData Glide;

static HMODULE GlideDLL = NULL;



#define TEXTURE_SIZE 256

static uint16 *texture_download_buffer = NULL;
static GrTexInfo texture_info;
static int texture_mem_start [4];

static void S9xGlideCalcSquare (int snes_width, int snes_height, 
				int x, int y, int width, int height,
				GrVertex *square)
{
	float snes_aspect_ratio = snes_width / (float) snes_height;
	float voodoo_aspect_ratio = Glide.voodoo_width / (float) Glide.voodoo_height;
	float virtual_height;
	float virtual_width;
	float virtual_x;
	float virtual_y;

#if 0
	if (snes_aspect_ratio < voodoo_aspect_ratio)
	{
	virtual_height = (float) Glide.voodoo_height;
	virtual_width = virtual_height * snes_aspect_ratio;
	virtual_x = ((float) Glide.voodoo_width - virtual_width) / 2.0;
	virtual_y = 0.0;
	}
	else
	{
	virtual_width = Glide.voodoo_width;
	virtual_height = virtual_width / snes_aspect_ratio;
	virtual_x = 0;
	virtual_y = (Glide.voodoo_height - virtual_height) / 2.0;
	}
#endif

	virtual_x = 0;
	virtual_y = 0;
	virtual_width = Glide.voodoo_width;
	virtual_height = Glide.voodoo_height;

	for (int i = 0; i < 4; i++)
	{
	square [i].oow = 1.0;
	square [i].ooz = 65535.0;
	square [i].tmuvtx [0].oow = 1.0;
	}

	square [0].tmuvtx [0].sow = 0.0;
	square [0].tmuvtx [0].tow = 0.0;
	square [1].tmuvtx [0].sow = (float) width;
	square [1].tmuvtx [0].tow = 0.0;
	square [2].tmuvtx [0].sow = (float) width;
	square [2].tmuvtx [0].tow = (float) height;
	square [3].tmuvtx [0].sow = 0.0;
	square [3].tmuvtx [0].tow = (float) height;
	
	float width_percent;
	float height_percent;
	float width_percent2;
	float height_percent2;

	if ((width_percent = virtual_width * TEXTURE_SIZE / snes_width) > virtual_width)
	width_percent = virtual_width;
	if ((height_percent = virtual_height * TEXTURE_SIZE / snes_height) > virtual_height)
	height_percent = virtual_height;

	width_percent2 = width_percent;
	height_percent2 = height_percent;
#if 1
	if (virtual_x + (x + 1) * width_percent2 > virtual_width)
		width_percent2 -= ((virtual_x + (x + 1) * width_percent2) - virtual_width) / (x + 1);
	if (virtual_y + (y + 1) * height_percent2 > virtual_height)
		height_percent2 -= ((virtual_y + (y + 1) * height_percent2) - virtual_height) / (y + 1);
#endif
	square [0].x = virtual_x + x * width_percent;
	square [0].y = virtual_y + y * height_percent;
	square [1].x = virtual_x + (x + 1) * width_percent2;
	square [1].y = virtual_y + y * height_percent;
	square [2].x = virtual_x + (x + 1) * width_percent2;
	square [2].y = virtual_y + (y + 1) * height_percent2;
	square [3].x = virtual_x + x * width_percent;
	square [3].y = virtual_y + (y + 1) * height_percent2;
}

static void S9xGlideInitTextures ()
{
	(*pgrColorCombine) (GR_COMBINE_FUNCTION_SCALE_OTHER,
						GR_COMBINE_FACTOR_ONE,
						GR_COMBINE_LOCAL_NONE,
						GR_COMBINE_OTHER_TEXTURE,
						FXFALSE);

	(*pgrTexCombine) (GR_TMU0,
					  GR_COMBINE_FUNCTION_LOCAL,
					  GR_COMBINE_FACTOR_NONE,
					  GR_COMBINE_FUNCTION_NONE,
					  GR_COMBINE_FACTOR_NONE,
					  FXFALSE, 
					  FXFALSE);

	(*pgrTexMipMapMode) (GR_TMU0,
						 GR_MIPMAP_DISABLE,
						 FXFALSE);

	(*pgrTexClampMode) (GR_TMU0,
						GR_TEXTURECLAMP_CLAMP,
						GR_TEXTURECLAMP_CLAMP);

	(*pgrTexFilterMode) (GR_TMU0, GR_TEXTUREFILTER_BILINEAR,
						 GR_TEXTUREFILTER_BILINEAR);

	texture_info.smallLod = GR_LOD_256;
	texture_info.largeLod = GR_LOD_256;
	texture_info.aspectRatio = GR_ASPECT_1x1;
	texture_info.format = GR_TEXFMT_RGB_565;

	Glide.texture.smallLod = GR_LOD_8;
	Glide.texture.largeLod = GR_LOD_8;
	Glide.texture.aspectRatio = GR_ASPECT_1x1;
	Glide.texture.format = GR_TEXFMT_RGB_565;
	Glide.texture_mem_size = (*pgrTexTextureMemRequired) (GR_MIPMAPLEVELMASK_BOTH,
							  &Glide.texture);
	Glide.texture_mem_start = (*pgrTexMinAddress) (GR_TMU0);

	int texture_mem_size = (*pgrTexTextureMemRequired) (GR_MIPMAPLEVELMASK_BOTH,
							&texture_info);
	int address = (*pgrTexMinAddress) (GR_TMU0);

	// At maximum SNES resolution (512x478) four seperate Voodoo textures will
	// be needed since the maximum texture size on Voodoo cards is 256x256.
	
	for (int t = 0; t < 4; t++)
	{
	texture_mem_start [t] = address;
	address += texture_mem_size;
	}
	
	texture_download_buffer = (uint16 *) malloc (texture_mem_size);

	for (int i = 0; i < 4; i++)
	{
	Glide.sq [i].oow = 1.0;
	Glide.sq [i].ooz = 65535.0;
	Glide.sq [i].tmuvtx [0].oow = 1.0;
	}

	Glide.x_offset = 0.0;
	Glide.y_offset = 0.0;
	// XXX: Do this when the SNES screen resolution is known.
	Glide.x_scale = (float) (Glide.voodoo_width * 8.0 / 256);
	Glide.y_scale = (float) (Glide.voodoo_height * 8.0 / 224);
}


bool S9xVoodooInitialise ()
{

	if (GlideDLL == NULL){
		if (!(GlideDLL = LoadLibrary ("glide2x.dll")))
		{
//			MessageBox(hWnd, "Error", MB_OK | MB_ICONINFORMATION);
			return (FALSE);
		}

		pgrColorCombine = (tpColorCombine) GetProcAddress (GlideDLL, "_grColorCombine@20");
		pgrTexCombine = (tpTexCombine) GetProcAddress (GlideDLL, "_grTexCombine@28");
		pgrTexMipMapMode = (tpTexMipMapMode) GetProcAddress (GlideDLL, "_grTexMipMapMode@12");
		pgrTexClampMode = (tpTexClampMode) GetProcAddress (GlideDLL, "_grTexClampMode@12");
		pgrTexFilterMode = (tpTexFilterMode) GetProcAddress (GlideDLL, "_grTexFilterMode@12");
		pgrTexTextureMemRequired = (tpTexTextureMemRequired) GetProcAddress (GlideDLL, "_grTexTextureMemRequired@8");
		pgrTexMinAddress = (tpTexMinAddress) GetProcAddress (GlideDLL, "_grTexMinAddress@4");
		pgrGlideInit = (tpGlideInit) GetProcAddress (GlideDLL, "_grGlideInit@0");
		pgrSstQueryHardware = (tpQueryHardware) GetProcAddress (GlideDLL, "_grSstQueryHardware@4");
		pgrSstSelect = (tpSelect) GetProcAddress (GlideDLL, "_grSstSelect@4");
		pgrSstWinOpen = (tpWinOpen) GetProcAddress (GlideDLL, "_grSstWinOpen@28");
		pgrBufferClear = (tpBufferClear) GetProcAddress (GlideDLL, "_grBufferClear@12");
		pgrSstScreenWidth = (tpScreenWidth) GetProcAddress (GlideDLL, "_grSstScreenWidth@0");
		pgrSstScreenHeight = (tpScreenHeight) GetProcAddress (GlideDLL, "_grSstScreenHeight@0");
		pgrGlideShutdown = (tpGlideShutdown) GetProcAddress (GlideDLL, "_grGlideShutdown@0");
		pgrTexDownloadMipMapLevel = (tpTexDownloadMipMapLevel) GetProcAddress (GlideDLL, "_grTexDownloadMipMapLevel@32");
		pgrTexSource = (tpTexSource) GetProcAddress (GlideDLL, "_grTexSource@16");
		pgrDrawTriangle = (tpDrawTriangle) GetProcAddress (GlideDLL, "_grDrawTriangle@12");
		pgrBufferSwap = (tpBufferSwap) GetProcAddress (GlideDLL, "_grBufferSwap@4");
		//pgr = (tp) GetProcAddress (GlideDLL, "gr");

		if (!pgrColorCombine || !pgrTexCombine || !pgrTexMipMapMode || !pgrTexClampMode ||
			!pgrTexFilterMode || !pgrTexTextureMemRequired || !pgrTexMinAddress || !pgrGlideInit ||
			!pgrSstQueryHardware || !pgrSstSelect || !pgrSstWinOpen || !pgrBufferClear ||
			!pgrSstScreenWidth || !pgrSstScreenHeight || !pgrGlideShutdown || !pgrTexDownloadMipMapLevel ||
			!pgrTexSource || !pgrDrawTriangle || !pgrBufferSwap){
//			MessageBox (GUI.hWnd, "", "Snes9x - Missing Symbols in Library (glide2x.dll) Error", MB_OK | MB_ICONINFORMATION);
			FreeLibrary(GlideDLL);
			GlideDLL = NULL;
			return (FALSE);
		}
		(*pgrGlideInit)();

		GrHwConfiguration hwconfig;
		Glide.voodoo_present = (*pgrSstQueryHardware) (&hwconfig);
		if (!Glide.voodoo_present)
		{
//			MessageBox (GUI.hWnd, "" "Snes9x - GLIDE Initialisation Error", MB_OK | MB_ICONINFORMATION);
			FreeLibrary(GlideDLL);
			GlideDLL = NULL;
			return (FALSE);
		}
	}
	return (Glide.voodoo_present);
}

bool8 S9xGlideInit ()
{
	static GrHwConfiguration hwconfig;
	// XXX:
	GrScreenResolution_t resolution = GR_RESOLUTION_640x480;
	GrScreenRefresh_t refresh_rate = GR_REFRESH_75Hz;

	if ((*pgrSstQueryHardware) (&hwconfig))
	{
		FxU32 hwnd = (FxU32) GUI.hWnd;

		if (hwconfig.SSTs [0].type == GR_SSTTYPE_SST96 ||
			hwconfig.SSTs [0].type > GR_SSTTYPE_Voodoo2)
		{
			resolution = GR_RESOLUTION_NONE;
		}
	(*pgrSstSelect) (0);

	if ((*pgrSstWinOpen) (hwnd, resolution, refresh_rate, 
							  GR_COLORFORMAT_ABGR,
							  GR_ORIGIN_UPPER_LEFT, 2, 1))
	{
		(*pgrBufferClear) (0, 0, GR_ZDEPTHVALUE_FARTHEST);
		Glide.voodoo_width = (float) (*pgrSstScreenWidth) ();
		Glide.voodoo_height = (float) (*pgrSstScreenHeight) ();
		S9xGlideInitTextures ();
		return (TRUE);
	}
	}
	
	Glide.voodoo_present = FALSE;
	return (FALSE);
}

void S9xGlideDeinit ()
{
	free ((char *) texture_download_buffer);
	(*pgrGlideShutdown) ();
	
	// Free Library
	if(GlideDLL){
		FreeLibrary(GlideDLL);
		GlideDLL = NULL;
	}
}

void S9xGlidePutImage (int snes_width, int snes_height)
{
	GrVertex square [4];

	Glide.voodoo_width = (float) (*pgrSstScreenWidth) ();
	Glide.voodoo_height = (float) (*pgrSstScreenHeight) ();

	//(*pgrBufferClear) (0, 0, GR_ZDEPTHVALUE_FARTHEST);

	if (snes_width <= TEXTURE_SIZE && snes_height <= TEXTURE_SIZE)
	{
	// Special case 256x224 and 256x239 SNES resolutions.
	texture_info.data = (void *) GFX.Screen;

	(*pgrTexDownloadMipMapLevel) (GR_TMU0, texture_mem_start [0],
				  GR_LOD_256, GR_LOD_256, GR_ASPECT_1x1,
				  GR_TEXFMT_RGB_565,
				  GR_MIPMAPLEVELMASK_BOTH,
				  (void *) GFX.Screen);

	(*pgrTexSource) (GR_TMU0, texture_mem_start [0],
			 GR_MIPMAPLEVELMASK_BOTH, &texture_info);

	S9xGlideCalcSquare (snes_width, snes_height, 0, 0, 
				snes_width, snes_height, square);
	(*pgrDrawTriangle) (&square [0], &square [3], &square [2]);
	(*pgrDrawTriangle) (&square [0], &square [1], &square [2]);
	}
	else
	{
	// Have to chop the rendered SNES screen into a maximum 256x256 size
	// chunks because that's all the Voodoo card can cope with.
	int t = 0;
	int vert_squares = (snes_height + (TEXTURE_SIZE - 1)) / TEXTURE_SIZE;
	int horiz_squares = (snes_width + (TEXTURE_SIZE - 1)) / TEXTURE_SIZE;

	texture_info.data = (void *) texture_download_buffer;

	for (int y = 0; y < vert_squares; y++)
	{
		for (int x = 0; x < horiz_squares; x++, t++)
		{
		int square_height = y != vert_squares - 1 ? TEXTURE_SIZE : 
					snes_height - (TEXTURE_SIZE * y);
		int square_width = x != horiz_squares - 1 ? TEXTURE_SIZE :
				   snes_width - (TEXTURE_SIZE * x);
		int offset = x * TEXTURE_SIZE * sizeof (uint16) +
				 y * TEXTURE_SIZE * GFX.Pitch;

		for (int l = 0; l < square_height; l++)
		{
			memmove ((void *) (texture_download_buffer + l * 256),
				 GFX.Screen + l * GFX.RealPitch + offset,
				 sizeof (uint16) * 256);
		}
		(*pgrTexDownloadMipMapLevel) (GR_TMU0, texture_mem_start [t],
					  GR_LOD_256, GR_LOD_256, 
					  GR_ASPECT_1x1,
					  GR_TEXFMT_RGB_565,
					  GR_MIPMAPLEVELMASK_BOTH,
					  (void *) texture_download_buffer);

		(*pgrTexSource) (GR_TMU0, texture_mem_start [t],
								 GR_MIPMAPLEVELMASK_BOTH, &texture_info);

		S9xGlideCalcSquare (snes_width, snes_height, x, y,
					square_width, square_height,
					square);
		(*pgrDrawTriangle) (&square [0], &square [3], &square [2]);
		(*pgrDrawTriangle) (&square [0], &square [1], &square [2]);
		}
	}
	}

	(*pgrBufferSwap) (1);
}

bool8 S9xGlideEnable (bool8 enable)
{
	static uint32 previous_graphic_format = RGB565;
	extern uint32 current_graphic_format;

	if (Settings.GlideEnable != enable)
	{
		WINDOWPLACEMENT place;
		place.length = sizeof (WINDOWPLACEMENT);
		GetWindowPlacement (GUI.hWnd, &place);

	if (enable)
	{
		if (S9xGlideInit ())
		{
		previous_graphic_format = current_graphic_format;
		S9xGraphicsDeinit ();
		S9xSetRenderPixelFormat (RGB565);
		S9xGraphicsInit ();
		Settings.GlideEnable = TRUE;
				if (place.showCmd == SW_SHOWNORMAL)
				{
					place.showCmd = SW_SHOWMAXIMIZED;
					SetWindowPlacement (GUI.hWnd, &place);
				}
		}
	}
	else
	{
		Settings.GlideEnable = FALSE;
		S9xGlideDeinit ();
		S9xSetRenderPixelFormat (previous_graphic_format);
		S9xGraphicsDeinit ();
		S9xGraphicsInit ();
			if (place.showCmd == SW_SHOWMAXIMIZED && !VOODOO_MODE)
			{
				place.showCmd = SW_SHOWNORMAL;
				SetWindowPlacement (GUI.hWnd, &place);
			}
	}
	}
	return (TRUE);
}
#endif

void RenderGlide (SSurface Src, SSurface Dst, RECT *rect)
{
	rect->left = 0;
	rect->right = DirectX.Width;
	rect->top = 0;
	rect->bottom = DirectX.Height;

	if (Glide.voodoo_present && !Settings.GlideEnable)
		S9xGlideEnable (TRUE);

	if (Settings.GlideEnable && Glide.voodoo_present)
		S9xGlidePutImage (Src.Width, Src.Height);
}



