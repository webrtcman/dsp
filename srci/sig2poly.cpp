//Includes
#include "sig2poly.c"

//Declarations
const valarray<size_t> oktypes = {1u,2u,101u,102u};
const size_t I = 1u, O = 2u;
size_t dim, L, Lx;
int mnz, u;

//Description
string descr;
descr += "Gets polynomial params starting from the signal (sig).\n";
descr += "This does linear prediction (LP) for each vector in X.\n";
descr += "\n";
descr += "This works by Levinson-Durbin recursion of the autocovariance (AC),\n";
descr += "and output Y holds the autoregressive (AR) coefficients.\n";
descr += "The 2nd output, V, holds the noise variances for each row or col.\n";
descr += "\n";
descr += "Use -l (--L) to give the number of lags at which to compute the AC.\n";
descr += "This is the length of each vector in the AC (i.e., lags 0 to L-1).\n";
descr += "where L-1 is also the order (P) of the linear prediction.\n";
descr += "This is also the length of each vector in the output Y.\n";
descr += "\n";
descr += "Use -d (--dim) to give the dimension along which to operate.\n";
descr += "Default is 0 (along cols), unless X is a row vector.\n";
descr += "\n";
descr += "If dim==0, then Y has size L x C x S x H.\n";
descr += "If dim==1, then Y has size R x L x S x H.\n";
descr += "If dim==2, then Y has size R x C x L x H.\n";
descr += "If dim==3, then Y has size R x C x S x L.\n";
descr += "\n";
descr += "Include -z (--zero_mean) to subtract the means from each vec in X [default=false].\n";
descr += "\n";
descr += "Include -u (--unbiased) to use unbiased calculation of AC [default is biased].\n";
descr += "This uses N-l instead of N in the denominator (it is actually just less biased).\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ sig2poly -l3 X -o Y -o V \n";
descr += "$ sig2poly -d1 -l5 X -o Y -o V \n";
descr += "$ cat X | sig2poly -z -l7 > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_int    *a_l = arg_intn("l","L","<uint>",0,1,"number of AC lags to compute [default=1]");
struct arg_int    *a_d = arg_intn("d","dim","<uint>",0,1,"dimension along which to operate [default=0]");
struct arg_lit  *a_mnz = arg_litn("z","zero_mean",0,1,"subtract mean from each vec in X [default=false]");
struct arg_lit    *a_u = arg_litn("u","unbiased",0,1,"use unbiased (N-l) denominator [default=biased]");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output files (Y,V)");

//Get options

//Get dim
if (a_d->count==0) { dim = i1.isrowvec() ? 1u : 0u; }
else if (a_d->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "dim must be nonnegative" << endl; return 1; }
else { dim = size_t(a_d->ival[0]); }
if (dim>3u) { cerr << progstr+": " << __LINE__ << errstr << "dim must be in {0,1,2,3}" << endl; return 1; }

//Get L
if (a_l->count==0) { L = 1u; }
else if (a_l->ival[0]<1) { cerr << progstr+": " << __LINE__ << errstr << "L must be positive" << endl; return 1; }
else { L = size_t(a_l->ival[0]); }

//Get mnz
mnz = (a_mnz->count>0);

//Get u
u = (a_u->count>0);

//Checks
Lx = (dim==0u) ? i1.R : (dim==1u) ? i1.C : (dim==2u) ? i1.S : i1.H;
if (i1.isempty()) { cerr << progstr+": " << __LINE__ << errstr << "input (X) found to be empty" << endl; return 1; }
if (Lx<2u) { cerr << progstr+": " << __LINE__ << errstr << "cannot work along a singleton dimension" << endl; return 1; }
if (Lx<L) { cerr << progstr+": " << __LINE__ << errstr << "requested more lags (L) than length of vecs in X" << endl; return 1; }

//Set output header info
o1.F = o2.F = i1.F;
o1.T = i1.T;
o2.T = i1.isreal() ? i1.T : i1.T-100u;
o1.R = (dim==0u) ? L : i1.R;
o1.C = (dim==1u) ? L : i1.C;
o1.S = (dim==2u) ? L : i1.S;
o1.H = (dim==3u) ? L : i1.H;
o2.R = (dim==0u) ? 1u : i1.R;
o2.C = (dim==1u) ? 1u : i1.C;
o2.S = (dim==2u) ? 1u : i1.S;
o2.H = (dim==3u) ? 1u : i1.H;

//Other prep

//Process
if (i1.T==1u)
{
    float *X, *Y, *V;
    try { X = new float[i1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file (X)" << endl; return 1; }
    try { Y = new float[o1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file 1 (Y)" << endl; return 1; }
    try { V = new float[o2.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file 2 (V)" << endl; return 1; }
    try { ifs1.read(reinterpret_cast<char*>(X),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file (X)" << endl; return 1; }
    if (codee::sig2poly_s(Y,V,X,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),dim,L,mnz,u)) { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(Y),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file 1 (Y)" << endl; return 1; }
    }
    if (wo2)
    {
        try { ofs2.write(reinterpret_cast<char*>(V),o2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file 2 (V)" << endl; return 1; }
    }
    delete[] X; delete[] Y; delete[] V;
}
else if (i1.T==101u)
{
    float *X, *Y, *V;
    try { X = new float[2u*i1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file (X)" << endl; return 1; }
    try { Y = new float[2u*o1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file (Y)" << endl; return 1; }
    try { V = new float[o2.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file 2 (V)" << endl; return 1; }
    try { ifs1.read(reinterpret_cast<char*>(X),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file (X)" << endl; return 1; }
    if (codee::sig2poly_c(Y,V,X,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),dim,L,mnz,u)) { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(Y),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file 1 (Y)" << endl; return 1; }
    }
    if (wo2)
    {
        try { ofs2.write(reinterpret_cast<char*>(V),o2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file 2 (V)" << endl; return 1; }
    }
    delete[] X; delete[] Y; delete[] V;
}

//Finish
