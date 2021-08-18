//Includes
#include "conv1d.c"

//Declarations
const valarray<size_t> oktypes = {1u,2u,101u,102u};
const size_t I = 2u, O = 1u;
size_t dim, Lx, stp, W, dil;
int c0;

//Description
string descr;
descr += "1D convolution with B of each vector (1D signal) in X,\n";
descr += "This version gives full control over stride, dilation, etc.\n";
descr += "For a simpler interface with less flexibility, see conv.\n";
descr += "\n";
descr += "B is in reverse chronological order (usual convention).\n";
descr += "This performs non-causal convolution; use fir for causal.\n";
descr += "\n";
descr += "Use -d (--dim) to give the dimension (axis) along which to filter.\n";
descr += "Use -d0 to operate along cols, -d1 to operate along rows, etc.\n";
descr += "The default is 0 (along cols), unless X is a row vector.\n";
descr += "\n";
descr += "Use -s (--step) to give the step-size (frame-shift) in samples [default=1].\n";
descr += "\n";
descr += "Use -c (--c0) to give the center-sample of the first frame [default=0].\n";
descr += "This is a uint.\n";
descr += "\n";
descr += "Use -w (--W) to give W, the number of frames [default=(Lx-c0-1)/stp].\n";
descr += "This is a positive int (can use less than default to use only part of X).\n";
descr += "The output Y has length W along dim.\n";
descr += "\n";
descr += "Use -d (--dilation) to give the dilation factor [default=1].\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ conv1d X B -o Y \n";
descr += "$ conv1d -d1 -c-2 -w500 X B > Y \n";
descr += "$ cat X | conv1d -c2 -w490 -i2 -s3 - B > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input files (X,B)");
struct arg_int  *a_stp = arg_intn("s","step","<uint>",0,1,"step size in samps [default=1]");
struct arg_int   *a_c0 = arg_intn("c","c0","<uint>",0,1,"center samp of first frame [default=0]");
struct arg_int    *a_w = arg_intn("w","W","<uint>",0,1,"number of frames [default=(N-c0-1)/stp]");
struct arg_int  *a_dil = arg_intn("i","dilation","<uint>",0,1,"dilation factor [default=1]");
struct arg_int    *a_d = arg_intn("d","dim","<uint>",0,1,"dimension along which to filter [default=0]");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Get dim
if (a_d->count==0) { dim = i1.isvec() ? i1.nonsingleton1() : 0u; }
else if (a_d->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "dim must be nonnegative" << endl; return 1; }
else { dim = size_t(a_d->ival[0]); }
if (dim>3u) { cerr << progstr+": " << __LINE__ << errstr << "dim must be in {0,1,2,3}" << endl; return 1; }
Lx = (dim==0u) ? i1.R : (dim==1u) ? i1.C : (dim==2u) ? i1.S : i1.H;

//Get c0
if (a_c0->count==0) { c0 = 0u; }
else { c0 = a_c0->ival[0]; }
if (c0>=(int)Lx) { cerr << progstr+": " << __LINE__ << errstr << "c0 must be less than Lx (length of vecs in X)" << endl; return 1; }

//Get stp
if (a_stp->count==0) { stp = 1u; }
else if (a_stp->ival[0]<1) { cerr << progstr+": " << __LINE__ << errstr << "stp must be positive" << endl; return 1; }
else { stp = size_t(a_stp->ival[0]); }

//Get W
if (a_w->count==0) { W = (size_t)(int(Lx)-c0-1) / stp; }
else if (a_w->ival[0]<1) { cerr << progstr+": " << __LINE__ << errstr << "W (nframes) must be positive" << endl; return 1; }
else { W = size_t(a_w->ival[0]); }

//Get dil
if (a_dil->count==0) { dil = 1u; }
else if (a_dil->ival[0]<1) { cerr << progstr+": " << __LINE__ << errstr << "dilation must be positive" << endl; return 1; }
else { dil = size_t(a_dil->ival[0]); }

//Checks
if (i1.T!=i2.T) { cerr << progstr+": " << __LINE__ << errstr << "inputs must have the same data type" << endl; return 1; }
if (i1.isempty()) { cerr << progstr+": " << __LINE__ << errstr << "input 1 (X) found to be empty" << endl; return 1; }
if (i2.isempty()) { cerr << progstr+": " << __LINE__ << errstr << "input 2 (B) found to be empty" << endl; return 1; }
if (!i2.isvec()) { cerr << progstr+": " << __LINE__ << errstr << "input 2 (B) must be a vector" << endl; return 1; }

//Set output header info
o1.F = i1.F; o1.T = i1.T;
o1.R = (dim==0u) ? W : i1.R;
o1.C = (dim==1u) ? W : i1.C;
o1.S = (dim==2u) ? W : i1.S;
o1.H = (dim==3u) ? W : i1.H;

//Other prep

//Process
if (i1.T==1u)
{
    float *X, *B, *Y;
    try { X = new float[i1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file 1 (X)" << endl; return 1; }
    try { B = new float[i2.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file 2 (B)" << endl; return 1; }
    try { Y = new float[o1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file (Y)" << endl; return 1; }
    try { ifs1.read(reinterpret_cast<char*>(X),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X)" << endl; return 1; }
    try { ifs2.read(reinterpret_cast<char*>(B),i2.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (B)" << endl; return 1; }
    if (codee::conv1d_s(Y,X,B,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),i2.N(),W,c0,stp,dil,dim))
    { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; } 
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(Y),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
    }
    delete[] X; delete[] B; delete[] Y;
}
else if (i1.T==101u)
{
    float *X, *B, *Y;
    try { X = new float[2u*i1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file 1 (X)" << endl; return 1; }
    try { B = new float[2u*i2.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file 2 (B)" << endl; return 1; }
    try { Y = new float[2u*o1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file (Y)" << endl; return 1; }
    try { ifs1.read(reinterpret_cast<char*>(X),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X)" << endl; return 1; }
    try { ifs2.read(reinterpret_cast<char*>(B),i2.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (B)" << endl; return 1; }
    if (codee::conv1d_c(Y,X,B,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),i2.N(),W,c0,stp,dil,dim))
    { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; } 
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(Y),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
    }
    delete[] X; delete[] B; delete[] Y;
}

//Finish
