//Includes
#include <cfloat>
#include "stft.c"

//Declarations
const valarray<size_t> oktypes = {1u,2u};
const size_t I = 2u, O = 1u;
size_t L, stp, W, nfft, F;
int snip_edges, mn0, amp, lg;

//Description
string descr;
descr += "Does STFT (short-term Fourier transform) of univariate X1 using window X2.\n";
descr += "\n";
descr += "The window (X2) is made by a generating function (hamming, hann, etc.).\n";
descr += "The signal (X1) and the window (X2) must be real-valued.\n";
descr += "\n";
descr += "Each frame of X1 is windowed (element-wise multiplied) by X2,\n";
descr += "which is a window vector of length L (generated previously).\n";
descr += "The FFT is done on each windowed frame, and\n";
descr += "the real-valued power at each positive FFT freq is returned as the STFT.\n";
descr += "\n";
descr += "The output Y has size FxW or WxF, where F is nfft/2+1, \n";
descr += "and nfft is the next-pow-2 of L, \n";
descr += "and W is the number of frames (a.k.a. windows).\n";
descr += "\n";
descr += "Use -s (--step) to give the step-size (frame-shift) in samples [default=160].\n";
descr += "\n";
descr += "Use -e (--snip-edges) to set snip-edges to true [default=false].\n";
descr += "This is a setting from HTK, Kaldi, Librosa, etc., which controls\n";
descr += "the placement of the first/last frames w.r.t. the start/end of X1.\n";
descr += "\n";
descr += "The number of output frames (W) is set as in Kaldi:\n";
descr += "If snip-edges=true:  W = 1u + (N-L)/stp   \n";
descr += "If snip-edges=false: W = (N+stp/2u) / stp \n";
descr += "\n";
descr += "If snip-edges=true, the first frame starts at samp 0,\n";
descr += "and the last frame fits entirely within the length of X.\n";
descr += "If snip-edges=false, the first frame is centered at samp stp/2,\n";
descr += "and the last frame can overlap the end of X.\n";
descr += "\n";
descr += "The following framing convention is used here:\n";
descr += "Samples from one frame are contiguous in memory, for row- and col-major.\n";
descr += "So, if Y is row-major, then it has size W x F; \n";
descr += "but if Y is col-major, then it has size F x W. \n";
descr += "\n";
descr += "Include -z (--zero-mean) to subtract the mean from each frame [default=false].\n";
descr += "This is applied just after windowing.\n";
descr += "\n";
descr += "Include -a (--amplitude) to output amplitude rather than power [default=false].\n";
descr += "This simply takes the sqrt of each element of Y before output.\n";
descr += "\n";
descr += "Include -l (--log) to output log amplitude or power [default=false].\n";
descr += "This simply takes the log of each element of Y before output.\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ stft -s65 X1 X2 -o Y \n";
descr += "$ stft -e X1 X2 > Y \n";
descr += "$ cat X1 | stft -e - X2 > Y \n";
descr += "$ hamming -l401 | stft -e -s160 X1 > Y \n";
descr += "$ stft -e -s160 X1 <(hamming -l401) > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input files (X1,X2)");
struct arg_int  *a_stp = arg_intn("s","step","<uint>",0,1,"step in samps between each frame [default=160]");
struct arg_lit  *a_sne = arg_litn("e","snip-edges",0,1,"include to snip edges [default=false]");
struct arg_lit  *a_mnz = arg_litn("z","zero-mean",0,1,"include to zero the mean of each frame [default=false]");
struct arg_lit  *a_amp = arg_litn("a","amplitude",0,1,"include to output amplitude (sqrt of power) [default=false]");
struct arg_lit  *a_log = arg_litn("l","log",0,1,"include to output log of amplitude or power [default=false]");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Get stp
if (a_stp->count==0) { stp = 160u; }
else if (a_stp->ival[0]<1) { cerr << progstr+": " << __LINE__ << errstr << "stp must be positive" << endl; return 1; }
else { stp = size_t(a_stp->ival[0]); }

//Get snip_edges
snip_edges = (a_sne->count>0);

//Get mn0
mn0 = (a_mnz->count>0);

//Get amp
amp = (a_amp->count>0);

//Get lg
lg = (a_log->count>0);

//Checks
if (i1.iscomplex() || i2.iscomplex()) { cerr << progstr+": " << __LINE__ << errstr << "inputs must be real-valued" << endl; return 1; }
if (i1.T!=i2.T) { cerr << progstr+": " << __LINE__ << errstr << "inputs must have the same data type" << endl; return 1; }
if (!i1.isvec()) { cerr << progstr+": " << __LINE__ << errstr << "input 1 (X1) must be a vector" << endl; return 1; }
if (i1.isempty()) { cerr << progstr+": " << __LINE__ << errstr << "input 1 (X1) found to be empty" << endl; return 1; }
if (!i2.isvec()) { cerr << progstr+": " << __LINE__ << errstr << "input 2 (X2) must be a vector" << endl; return 1; }
if (i2.isempty()) { cerr << progstr+": " << __LINE__ << errstr << "input 2 (X2) found to be empty" << endl; return 1; }

//Set output header
L = i2.N();
nfft = 1u;
while (nfft<L) { nfft *= 2u; }
F = nfft/2u + 1u;
W = (snip_edges) ? 1u+(i1.N()-L)/stp : (i1.N()+stp/2u)/stp;
o1.F = i1.F; o1.T = i1.T;
o1.R = (i1.isrowmajor()) ? W : F;
o1.C = (i1.isrowmajor()) ? F : W;
o1.S = i1.S; o1.H = i1.H;

//Other prep

//Process
if (o1.T==1u)
{
    float *X1, *X2, *Y;
    try { X1 = new float[i1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file 1 (X1)" << endl; return 1; }
    try { X2 = new float[i2.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file 2 (X2)" << endl; return 1; }
    try { Y = new float[o1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file (Y)" << endl; return 1; }
    try { ifs1.read(reinterpret_cast<char*>(X1),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file (X)" << endl; return 1; }
    try { ifs2.read(reinterpret_cast<char*>(X2),i2.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
    if (codee::stft_s(Y,X1,X2,i1.N(),L,nfft,stp,snip_edges,mn0,amp,lg))
    { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(Y),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
    }
    delete[] X1; delete[] X2; delete[] Y;
}

//Finish
