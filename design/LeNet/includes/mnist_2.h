
//MNIST Network 2
//784,32C5,P2,64C5,P2,2048,10

////16bits
////ZCU104
// #ifdef __SDSCC__
// typedef ap_fixed<16,2,AP_RND> w_dt;
// typedef ap_fixed<18,12,AP_RND> q_dt;
// typedef ap_fixed<20,14,AP_RND> q2_dt;
// typedef ap_uint<8> i_dt;
// typedef ap_uint<16> sr_dt;
// typedef ap_uint<12> sc_dt;
// const int INTERFACE_WIDTH = 64;
// const int WWIDTH = 16;
// #define IW ap_uint<INTERFACE_WIDTH>
// #define WW ap_uint<WWIDTH>
// const int WIDTH_FACTOR = INTERFACE_WIDTH/WWIDTH;
// #else
// typedef float w_dt;
// typedef float q_dt;
// typedef float q2_dt;
// typedef int i_dt;
// typedef int sr_dt;
// typedef int sc_dt;
// #endif
// const w_dt vth = 1.0;
// const w_dt sf1 = 1;
// const w_dt sf2 = 1;
// const w_dt sf3 = 1;
// const w_dt sf4 = 1;
// const w_dt sf5 = 1;
// const w_dt sf6 =  1;
// const int NumKernels = 2;
// const int BufRows = 0;
// const int IMaps = 1;
// const int Maps = 64;
// const int CIMaps = 2;
// const int FUnroll = 4;
// const int TSims = 40;
// const int Sims = 40;
// const int MaxNeurons = 440;
// const int Part = 32;
// const int EMaps = 32;
// const int PMaps = 32;





// ////8bits
// ////ZCU104
// #ifdef __SDSCC__
// typedef ap_fixed<8,2,AP_RND> w_dt;
// typedef ap_fixed<15,11,AP_RND> q_dt;
// typedef ap_fixed<17,13,AP_RND> q2_dt;
// typedef ap_uint<8> i_dt;
// typedef ap_uint<16> sr_dt;
// typedef ap_uint<12> sc_dt;
// const int INTERFACE_WIDTH = 128;
// const int WWIDTH = 8;
// #define IW ap_uint<INTERFACE_WIDTH>
// #define WW ap_uint<WWIDTH>
// const int WIDTH_FACTOR = INTERFACE_WIDTH/WWIDTH;
// #else
// typedef float w_dt;
// typedef float q_dt;
// typedef float q2_dt;
// typedef int i_dt;
// typedef int sr_dt;
// typedef int sc_dt;
// #endif
// const w_dt vth = 1.0;
// const w_dt sf1 = 0.3;
// const w_dt sf2 = 1;
// const w_dt sf3 = 0.3;
// const w_dt sf4 = 1;
// const w_dt sf5 = 0.3;
// const w_dt sf6 =  0.02;
// const int NumKernels = 2;
// const int BufRows = 0;
// const int IMaps = 1;
// const int Maps = 64;
// const int CIMaps = 2;
// const int FUnroll = 16;
// const int TSims = 40;
// const int Sims = 40;
// const int MaxNeurons = 440;
// const int Part = 32;
// const int EMaps = 32;
// const int PMaps = 32;



////4bits
#ifdef __SDSCC__
typedef ap_fixed<4,2,AP_RND> w_dt;
typedef ap_fixed<13,11,AP_RND> q_dt;
typedef ap_fixed<17,15,AP_RND> q2_dt;
typedef ap_uint<8> i_dt;
typedef ap_uint<16> sr_dt;
typedef ap_uint<14> sc_dt;
const int INTERFACE_WIDTH = 128;
const int WWIDTH = 4;
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
const w_dt sf1 = 0.5;
const w_dt sf2 = 1;
const w_dt sf3 = 0.5;
const w_dt sf4 = 1;
const w_dt sf5 = 0.5;
const w_dt sf6 =  0.25;
const int NumKernels = 2;
const int BufRows = 0;
const int IMaps = 1;
const int Maps = 64;
const int CIMaps = 2;
const int Part = 32;
const int EMaps = 32;
const int PMaps = 32;
const int FUnroll = 32;
const int TSims = 30;
const int Sims = 30;
const int MaxNeurons = 460;


////ZCU102
// #ifdef __SDSCC__
// typedef ap_fixed<4,2,AP_RND> w_dt;
// typedef ap_fixed<13,11,AP_RND> q_dt;
// typedef ap_fixed<17,15,AP_RND> q2_dt;
// typedef ap_uint<8> i_dt;
// typedef ap_uint<16> sr_dt;
// typedef ap_uint<14> sc_dt;
// const int INTERFACE_WIDTH = 128;
// const int WWIDTH = 4;
// #define IW ap_uint<INTERFACE_WIDTH>
// #define WW ap_uint<WWIDTH>
// const int WIDTH_FACTOR = INTERFACE_WIDTH/WWIDTH;
// #else
// typedef float w_dt;
// typedef float q_dt;
// typedef float q2_dt;
// typedef int i_dt;
// typedef int sr_dt;
// typedef int sc_dt;
// #endif
// const w_dt vth = 1.0;
// const w_dt sf1 = 0.5;
// const w_dt sf2 = 1;
// const w_dt sf3 = 0.5;
// const w_dt sf4 = 1;
// const w_dt sf5 = 0.5;
// const w_dt sf6 =  0.25;
// const int NumKernels = 2;
// const int BufRows = 1800;
// const int IMaps = 1;
// const int Maps = 64;
// const int CIMaps = 8;
// const int Part = 32;
// const int EMaps = 32;
// const int PMaps = 32;
// const int FUnroll = 32;
// const int TSims = 30;
// const int Sims = 30;
// const int MaxNeurons = 480;











//////ZED
//#ifdef __SDSCC__
//typedef ap_fixed<4,2,AP_RND> w_dt;
//typedef ap_fixed<13,11,AP_RND> q_dt;
//typedef ap_fixed<17,15,AP_RND> q2_dt;
//typedef ap_uint<8> i_dt;
//typedef ap_uint<16> sr_dt;
//typedef ap_uint<14> sc_dt;
//const int INTERFACE_WIDTH = 8;
//const int WWIDTH = 4;
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
//const w_dt vth = 1.0;
//const w_dt sf1 = 0.5;
//const w_dt sf2 = 1;
//const w_dt sf3 = 0.5;
//const w_dt sf4 = 1;
//const w_dt sf5 = 0.5;
//const w_dt sf6 =  0.25;
//const int NumKernels = 2;
//const int BufRows = 0;
//const int IMaps = 1;
//const int Maps = 1;
//const int CIMaps = 1;
//const int Part = 1;
//const int EMaps = 1;
//const int PMaps = 1;
//const int FUnroll = 1;
//const int TSims = 30;
//const int Sims = 30;
//const int MaxNeurons = 480;













//Common
const int NumLayers = 7;
const int StrideC = 1;
const int StrideP = 2;
#define scale 0.25
const int KSide = 5;
const int MapsMax = 64;
const int SideMax = 28;
const int Sources =784;
const int Output = 10;
const int PaddingC = 2;
const int PaddingP = 0;
const int Side1 = 28;
const int Side2 =28;
const int Side3 =14;
const int Side4 =14;
const int Side5 =7;
const int Maps1 =1;
const int Maps2 =32;
const int Maps3 =32;
const int Maps4 =64;
const int Maps5 =64;
const int Layer1 = 3136;
const int Layer2 = 2048;
const int MaxRows = 3136;
const int MaxCols = 2048;
const int MaxFc = MaxCols;
