//MNIST - Network 1
//32C3, P2, 32C3, 256, 1

//16bits
//ZCU104
#ifdef __SDSCC__
typedef ap_fixed<16,8,AP_RND> w_dt;
typedef ap_fixed<18,12,AP_RND> q_dt;
typedef ap_fixed<20,14,AP_RND> q2_dt;
typedef ap_uint<8> i_dt;
typedef ap_uint<15> sr_dt;
typedef ap_uint<12> sc_dt;
const int INTERFACE_WIDTH = 128;
const int WWIDTH = 16;
#define IW ap_uint<INTERFACE_WIDTH>
#define WW ap_uint<WWIDTH>
const int WIDTH_FACTOR = INTERFACE_WIDTH/WWIDTH;
#else
typedef float w_dt;
typedef float q_dt;
typedef float q2_dt;
typedef float i_dt;
typedef int sr_dt;
typedef float sc_dt;
#endif
const w_dt vth = 1.0;
const w_dt sf1 = 1;
const w_dt sf2 = 1;
const w_dt sf3 = 1;
const w_dt sf4 = 1;
const w_dt sf5 = 1;
const w_dt sf6 =  1;
const int NumKernels = 2;
const int BufRows = 800;
const int IMaps = 1;
const int Maps = 32;
const int CIMaps = 8;
const int FUnroll = 64;
const int TSims = 30;
const int Sims = 30;
const int MaxNeurons = 440;
const int Part = CIMaps;
const int EMaps = CIMaps;
const int PMaps = CIMaps;




// ////8bits
// #ifdef __SDSCC__
// typedef ap_fixed<8,2,AP_RND> w_dt;
// typedef ap_fixed<15,11,AP_RND> q_dt;
// typedef ap_fixed<17,13,AP_RND> q2_dt;
// typedef ap_uint<8> i_dt;
// typedef ap_uint<15> sr_dt;
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
// const w_dt sf1 = 0.5;
// const w_dt sf2 = 1;
// const w_dt sf3 = 0.5;
// const w_dt sf4 = 1;
// const w_dt sf5 = 0.3;
// const w_dt sf6 =  0.15;
// const int NumKernels = 2;
// const int BufRows = 800;
// const int IMaps = 1;
// const int Maps = 32;
// const int CIMaps = 8;
// const int Part = 16;
// const int EMaps = 16;
// const int PMaps = 16;
// const int FUnroll = 64;
// const int TSims = 30;
// const int Sims = 30;
// const int MaxNeurons = 440;



// //////4bits
// #ifdef __SDSCC__
// typedef ap_fixed<4,2,AP_RND> w_dt;
// typedef ap_fixed<13,11,AP_RND> q_dt;
// typedef ap_fixed<15,13,AP_RND> q2_dt;
// typedef ap_uint<8> i_dt;
// typedef ap_uint<15> sr_dt;
// typedef ap_uint<12> sc_dt;
// const int INTERFACE_WIDTH = 8;
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
// const w_dt sf3 = 0.4;
// const w_dt sf4 = 1;
// const w_dt sf5 = 0.3;
// const w_dt sf6 =  0.15;
// const int NumKernels = 2;
// const int BufRows = 800;
// const int IMaps = 1;
// const int Maps = 32;
// const int CIMaps = 8;
// const int Part = 32;
// const int EMaps = 32;
// const int PMaps = 16;
// const int FUnroll = 64;
// const int TSims = 30;
// const int Sims = 30;
// const int MaxNeurons = 440;

//ZCU102
//#ifdef __SDSCC__
//typedef ap_fixed<4,2,AP_RND> w_dt;
//typedef ap_fixed<13,11,AP_RND> q_dt;
//typedef ap_fixed<15,13,AP_RND> q2_dt;
//typedef ap_uint<8> i_dt;
//typedef ap_uint<15> sr_dt;
//typedef ap_uint<12> sc_dt;
//const int INTERFACE_WIDTH = 128;
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
//const w_dt sf3 = 0.4;
//const w_dt sf4 = 1;
//const w_dt sf5 = 0.3;
//const w_dt sf6 =  0.15;
//const int NumKernels = 2;
//const int BufRows = 800;
//const int IMaps = 1;
//const int Maps = 32;
//const int CIMaps = 16;
//const int Part = 32;
//const int EMaps = 32;
//const int PMaps = 16;
//const int FUnroll = 64;
//const int TSims = 30;
//const int Sims = 30;
//const int MaxNeurons = 440;



////ZED
//#ifdef __SDSCC__
//typedef ap_fixed<4,2,AP_RND> w_dt;
//typedef ap_fixed<13,11,AP_RND> q_dt;
//typedef ap_fixed<15,13,AP_RND> q2_dt;
//typedef ap_uint<8> i_dt;
//typedef ap_uint<15> sr_dt;
//typedef ap_uint<12> sc_dt;
//const int INTERFACE_WIDTH = 128;
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
//const w_dt sf3 = 0.4;
//const w_dt sf4 = 1;
//const w_dt sf5 = 0.3;
//const w_dt sf6 =  0.15;
//const int NumKernels = 2;
//const int BufRows = 0;
//const int IMaps = 1;
//const int Maps = 32;
//const int CIMaps = 1;
//const int Part = 1;
//const int EMaps = 1;
//const int PMaps = 1;
//const int FUnroll = 1;
//const int TSims = 30;
//const int Sims = 30;
//const int MaxNeurons = 440;



//Common
const int NumLayers = 7;
const int StrideC = 1;
const int StrideP = 2;
#define scale 0.25
const int KSide = 3;
const int MapsMax = 32;
const int SideMax = 26;
const int Sources =784;
const int Output = 10;
const int PaddingC = 0;
const int PaddingP = 0;
const int Side1 = 28;
const int Side2 =26;
const int Side3 =13;
const int Side4 =11;
const int Side5 =5;
const int KSide1 =3;
const int KSide2 =2;
const int KSide3 =3;
const int KSide4 =2;
const int Maps1 =1;
const int Maps2 =32;
const int Maps3 =32;
const int Maps4 =32;
const int Maps5 =32;
const int Layer1 = 800;
const int Layer2 = 256;
const int MaxRows = 800;
const int MaxCols = 256;
const int MaxFc = 256;
