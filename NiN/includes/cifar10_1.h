//16bits
//ZCU104
#ifdef __SDSCC__
typedef ap_fixed<16,2,AP_RND> w_dt;
typedef ap_fixed<24,15,AP_RND> q_dt;
typedef ap_fixed<24,15,AP_RND> q2_dt;
typedef ap_uint<8> i_dt;
typedef ap_uint<20> sr_dt;
typedef ap_uint<14> sc_dt;
const int INTERFACE_WIDTH = 128;
const int WWIDTH = 16;
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
const w_dt sf13 =  1;

const int IMaps = 1;
const int Maps = 32;
const int CMaps = 32;
const int TSims = 1800;
const int Sims = 1800;
const int MaxNeurons = 3200;
const int Part = 32;
const int EMaps = 32;
const int PMaps = 32;



//////8bits
//////ZCU104
//#ifdef __SDSCC__
//typedef ap_fixed<8,2,AP_RND> w_dt;
//typedef ap_fixed<22,16,AP_RND> q_dt;
//typedef ap_fixed<22,16,AP_RND> q2_dt;
//typedef ap_uint<8> i_dt;
//typedef ap_uint<20> sr_dt;
//typedef ap_uint<15> sc_dt;
//
//const int INTERFACE_WIDTH = 8;
//const int WWIDTH = 8;
//#define IW ap_uint<INTERFACE_WIDTH>
//#define WW ap_uint<WWIDTH>
//const int WIDTH_FACTOR = INTERFACE_WIDTH/WWIDTH;
//#else
//typedef float w_dt;
//typedef float q_dt;
//typedef float q2_dt;
//typedef int i_dt;
//typedef int sr_dt;
//typedef int sc_dt;
//#endif
// const w_dt sf1 = 1;
// const w_dt sf2 = 0.890625;
// const w_dt sf3 = 0.53125;
// const w_dt sf4 = 0.375;
// const w_dt sf5 = 1;
// const w_dt sf6 =  0.125;
// const w_dt sf7 = 0.25;
// const w_dt sf8 = 0.28125;
// const w_dt sf9 = 1;
// const w_dt sf10 = 0.125;
// const w_dt sf11 = 0.125;
// const w_dt sf12 =  0.125;
// const w_dt sf13 =  0.125;
// const w_dt sf10 = 0.875;
// const w_dt sf11 = 0.25;
// const w_dt sf12 =  0.714;
// const w_dt sf13 =  0.71875;
//
//const int IMaps = 1;
//const int Maps = 32;
//const int CMaps = 32;
//const int KPart = 128;
//const int TSims = 1800;
//const int Sims = 1800;
//const int MaxNeurons = 3200;
//const int Part = 32;
//const int EMaps = 32;
//const int PMaps = 32;













//Common
const int NumKernels = 9;
const int NumKernels1 = 3;
const int NumKernels2 = 6;
const int NumLayers = 13;
const int StrideC = 1;
const int StrideP = 2;
#define scale 0.11
#define scale_2 0.015625
const int KSide1 = 5;
const int KSide2 = 1;
const int KSide3 = 3;
const int MapsMax = 192;
const int SideMax = 32;
const int KSideMax = 5;
const int Sources =3072;
const int Output = 10;
const int PaddingC1 = 2;
const int PaddingC2 = 0;
const int PaddingP = 0.5;
const int Side1 = 32;
const int Side2 =32;
const int Side3 = 32;
const int Side4 =32;
const int Side5 =16;
const int Side6 =16;
const int Side7 =16;
const int Side8 =16;
const int Side9 = 8;
const int Side10 =8;
const int Side11 =8;
const int Side12 = 8;


const int Maps1 =3;
const int Maps2 =192;
const int Maps3 =192;
const int Maps4 =192;
const int Maps5 =192;
const int Maps6 =192;
const int Maps7 =192;
const int Maps8 =192;
const int Maps9 =192;
const int Maps10 =192;
const int Maps11 =192;
const int Maps12 =10;

const int MaxFc = Maps12;

const w_dt vth1 = 1;
const w_dt vth2 = 1;
const w_dt vth3 = 1;
const w_dt vth4 = 1;
const w_dt vth5 = 1;
const w_dt vth6 = 1;
const w_dt vth7 = 1;
const w_dt vth8 = 1;
const w_dt vth9 = 1;
const w_dt vth10 = 1;
const w_dt vth11 = 1;
const w_dt vth12 = 1;

