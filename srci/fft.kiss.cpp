//Includes
#include "fft.kiss.c"
#include "/opt/kissfft/kissfft.hh"

//Declarations
const valarray<size_t> oktypes = {1u,2u,101u,102u};
const size_t I = 1u, O = 1u;
size_t dim, nfft, Lx, Ly;
int sc;

//Description
string descr;
descr += "1D FFT (fast Fourier transform) of each vector (1D signal) in X.\n";
descr += "This uses the KISS (keep it simple stupid) FFT library.\n";
descr += "\n";
descr += "Use -d (--dim) to give the dimension along which to transform.\n";
descr += "Use -d0 to operate along cols, -d1 to operate along rows, etc.\n";
descr += "The default is 0 (along cols), unless X is a row vector.\n";
descr += "\n";
descr += "Use -n (--nfft) to specify transform length [default=L].\n";
descr += "The default (L) is the length of X along dim.\n";
descr += "X is zero-padded as necessary to match nfft.\n";
descr += "\n";
descr += "The output (Y) is complex-valued with length nfft along dim\n";
descr += "for complex X, and length nfrqs along dim for real X, \n";
descr += "where nfrqs = floor(nfft/2)+1 = num nonnegative FFT frequencies.\n";
descr += "\n";
descr += "Note: to get same result + negative freqs, just convert X to complex.\n";
descr += "\n";
descr += "Include -s (--scale) to scale by sqrt(0.5/L), for formal definition.\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ fft.kiss -n256 X -o Y \n";
descr += "$ fft.kiss -n256 -d1 X > Y \n";
descr += "$ cat X | fft.kiss -n256 > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_int    *a_d = arg_intn("d","dim","<uint>",0,1,"dimension along which to transform [default=0]");
struct arg_int    *a_n = arg_intn("n","nfft","<uint>",0,1,"transform length [default=L]");
struct arg_lit   *a_sc = arg_litn("s","scale",0,1,"include to scale by sqrt(0.5/n) [default=no scale]");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Get dim
if (a_d->count==0) { dim = i1.isrowvec() ? 1u : 0u; }
else if (a_d->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "dim must be nonnegative" << endl; return 1; }
else { dim = size_t(a_d->ival[0]); }
if (dim>3u) { cerr << progstr+": " << __LINE__ << errstr << "dim must be in {0,1,2,3}" << endl; return 1; }

//Get nfft
Lx = (dim==0u) ? i1.R : (dim==1u) ? i1.C : (dim==2u) ? i1.S : i1.H;
if (a_n->count==0) { nfft = Lx; }
else if (a_n->ival[0]<1) { cerr << progstr+": " << __LINE__ << errstr << "nfft must be positive" << endl; return 1; }
else { nfft = size_t(a_n->ival[0]); }
if (nfft<Lx) { cerr << progstr+": " << __LINE__ << errstr << "nfft must be >= Lx (length of vecs in X)" << endl; return 1; }

//Get sc
sc = (a_sc->count>0);

//Checks
if (i1.isempty()) { cerr << progstr+": " << __LINE__ << errstr << "input (X) found to be empty" << endl; return 1; }
if (i1.T==2u || i1.T==102u) { cerr << progstr+": " << __LINE__ << warstr << "double precision not directly supported (will cast to float and back)" << endl; }

//Set output header info
o1.F = i1.F;
o1.T = i1.isreal() ? i1.T+100u : i1.T;
Ly = i1.isreal() ? nfft/2u+1u : nfft;
o1.R = (dim==0u) ? Ly : i1.R;
o1.C = (dim==1u) ? Ly : i1.C;
o1.S = (dim==2u) ? Ly : i1.S;
o1.H = (dim==3u) ? Ly : i1.H;

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
    if (codee::fft_kiss_s(Y,X,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),dim,nfft,sc))
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
    if (codee::fft_kiss_c(Y,X,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),dim,nfft,sc))
    { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(Y),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
    }
    delete[] X; delete[] Y;
}

//Finish
