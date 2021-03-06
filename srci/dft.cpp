//Includes
#include "dft.c"

//Declarations
const valarray<size_t> oktypes = {1u,2u,101u,102u};
const size_t I = 1u, O = 1u;
size_t dim, Lx, ndft, F;
int sc;

//Description
string descr;
descr += "1D DFT (discrete Fourier transform) of each vector (1D signal) in X.\n";
descr += "This version uses a direct matrix multiplication by the DFT matrix.\n";
descr += "\n";
descr += "Use -d (--dim) to give the dimension along which to transform.\n";
descr += "Use -d0 to operate along cols, -d1 to operate along rows, etc.\n";
descr += "The default is 0 (along cols), unless X is a row vector.\n";
descr += "\n";
descr += "Use -n (--ndft) to specify transform length [default=L].\n";
descr += "X is zero-padded as necessary to match ndft.\n";
descr += "The default (L) is the length of X along dim.\n";
descr += "\n";
descr += "Use -f (--F) to specify the number of output freq bins to keep.\n";
descr += "The default is ndft/2+1 for real X and ndft for complex X.\n";
descr += "This must be in [1 ndft], and can be used to compute fewer outputs.\n";
descr += "\n";
descr += "The output (Y) is complex-valued with length F along dim. \n";
descr += "\n";
descr += "Examples:\n";
descr += "$ dft -n16 X -o Y \n";
descr += "$ dft -d1 -n16 -f4 X > Y \n";
descr += "$ cat X | dft -d1 -n40 -f15 > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_int    *a_d = arg_intn("d","dim","<uint>",0,1,"dimension along which to transform [default=0]");
struct arg_int    *a_n = arg_intn("n","ndft","<uint>",0,1,"transform length [default=L]");
struct arg_int    *a_f = arg_intn("f","F","<uint>",0,1,"num freq bins to output [default=ndft/2+1 or ndft]");
struct arg_lit   *a_sc = arg_litn("s","scale",0,1,"include to scale by sqrt(0.5/n) [default=no scale]");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Get dim
if (a_d->count==0) { dim = i1.isrowvec() ? 1u : 0u; }
else if (a_d->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "dim must be nonnegative" << endl; return 1; }
else { dim = size_t(a_d->ival[0]); }
if (dim>3u) { cerr << progstr+": " << __LINE__ << errstr << "dim must be in {0,1,2,3}" << endl; return 1; }

//Get ndft
Lx = (dim==0u) ? i1.R : (dim==1u) ? i1.C : (dim==2u) ? i1.S : i1.H;
if (a_n->count==0) { ndft = Lx; }
else if (a_n->ival[0]<1) { cerr << progstr+": " << __LINE__ << errstr << "ndft must be positive" << endl; return 1; }
else { ndft = size_t(a_n->ival[0]); }
if (ndft<Lx) { cerr << progstr+": " << __LINE__ << errstr << "ndft must be >= Lx (length of vecs in X)" << endl; return 1; }

//Get F
if (a_f->count==0) { F = i1.isreal() ? ndft/2u+1u : ndft; }
else if (a_f->ival[0]<1) { cerr << progstr+": " << __LINE__ << errstr << "F must be positive" << endl; return 1; }
else { F = size_t(a_f->ival[0]); }
if (F>ndft) { cerr << progstr+": " << __LINE__ << errstr << "F must be <= ndft" << endl; return 1; }

//Get sc
sc = (a_sc->count>0);

//Checks
if (i1.isempty()) { cerr << progstr+": " << __LINE__ << errstr << "input (X) found to be empty" << endl; return 1; }

//Set output header info
o1.F = i1.F;
o1.T = i1.isreal() ? i1.T+100u : i1.T;
o1.R = (dim==0u) ? F : i1.R;
o1.C = (dim==1u) ? F : i1.C;
o1.S = (dim==2u) ? F : i1.S;
o1.H = (dim==3u) ? F : i1.H;

//Other prep

//Process
if (i1.T==1u)
{
    float *X, *Y;
    try { X = new float[i1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file (X)" << endl; return 1; }
    try { Y = new float[2u*o1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file (Y)" << endl; return 1; }
    try { ifs1.read(reinterpret_cast<char*>(X),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file (X)" << endl; return 1; }
    if (codee::dft_s(Y,X,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),dim,ndft,F,sc))
    { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(Y),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
    }
    delete[] X; delete[] Y;
}
else if (i1.T==101u)
{
    float *X, *Y;
    try { X = new float[2u*i1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file (X)" << endl; return 1; }
    try { Y = new float[2u*o1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file (Y)" << endl; return 1; }
    try { ifs1.read(reinterpret_cast<char*>(X),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file (X)" << endl; return 1; }
    if (codee::dft_c(Y,X,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),dim,ndft,F,sc))
    { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(Y),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
    }
    delete[] X; delete[] Y;
}

//Finish
