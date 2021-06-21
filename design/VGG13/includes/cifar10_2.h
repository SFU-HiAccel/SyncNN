//ZCU104
#ifdef __SDSCC__
typedef ap_fixed<8,2,AP_RND> w_dt;
typedef ap_fixed<20,15,AP_RND> q_dt;
typedef ap_fixed<20,15,AP_RND> q2_dt;
typedef ap_uint<8> i_dt;
typedef ap_uint<16> sr_dt;
typedef ap_uint<14> sc_dt;
const int INTERFACE_WIDTH = 128;
const int WWIDTH = 8;
#define IW ap_uint<INTERFACE_WIDTH>
#define WW ap_uint<WWIDTH>
const int WIDTH_FACTOR = INTERFACE_WIDTH/WWIDTH;
#else
typedef float w_dt;
typedef float q_dt;
typedef float q2_dt;
typedef int i_dt;
typedef int sr_dt;
typedef int sc_dt;
#endif
const w_dt vth = 1.0;
const w_dt sf1 = 1;
const w_dt sf2 = 1;
const w_dt sf3 = 1;
const w_dt sf4 = 1;
const w_dt sf5 = 1;
const w_dt sf6 =  1;
const w_dt sf7 = 1;
const w_dt sf8 = 1;
const w_dt sf9 = 1;
const w_dt sf10 = 1;
const w_dt sf11 = 1;
const w_dt sf12 =  1;
const w_dt sf13 = 1;
const w_dt sf14 = 1;
const w_dt sf15 = 1;
const w_dt sf16 = 1;
const w_dt sf17 = 1;
const w_dt sf18 =  1;
const int IMaps = 1;
const int Maps = 64;
const int TSims = 1800;
const int Sims = 1800;

const int Part = 32;
const int EMaps = 32;
const int PMaps = 32;
const int BufMaps = 128;
const int BufRows = 0;
const int FUnroll = 16;


//Common
const int NumKernels = 10;
const int NumLayers = 19;
const int StrideC = 1;
const int StrideP = 2;
#define scale 0.25

const int KSide1 = 3;
const int MapsMax1 = 512;
const int MapsMax = 512;
const int MaxNeurons1 = 1100;
const int MaxNeurons = 1100;

const int SideMax = 32;
const int KSideMax = 3;
const int Sources =3072;
const int Output = 10;
const int PaddingC1 = 1;
const int PaddingP = 0;

const int Side1 = 32;
const int Side2 = 32;
const int Side3 = 32;
const int Side4 = 16;
const int Side5 = 16;
const int Side6 = 16;
const int Side7 = 8;
const int Side8 = 8;
const int Side9 = 8;
const int Side10 =4;
const int Side11 =4;
const int Side12 = 4;
const int Side13 = 2;
const int Side14 = 2;
const int Side15 = 2;
const int Side16 = 1;


const int Maps1 =3;
const int Maps2 =64;
const int Maps3 =64;
const int Maps4 =64;
const int Maps5 =128;
const int Maps6 =128;
const int Maps7 =128;
const int Maps8 =256;
const int Maps9 =256;
const int Maps10 =256;
const int Maps11 =512;
const int Maps12 =512;
const int Maps13 =512;
const int Maps14 =512;
const int Maps15 =512;
const int Maps16 =512;

const int Layer1 = 512;
const int Layer2 = 256;
const int Layer3 = 256;

const int MaxRows = 512;
const int MaxCols = 256;
const int MaxFc = MaxCols;

