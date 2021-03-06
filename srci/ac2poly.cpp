//Includes
#include "ac2poly.c"

//Declarations
const valarray<size_t> oktypes = {1u,2u,101u,102u};
const size_t I = 1u, O = 2u;
size_t dim;

//Description
string descr;
descr += "Gets polynomial params starting from the autocovariance (AC).\n";
descr += "Does Levinson-Durbin recursion of each vector in X,\n";
descr += "where X has the autocovariance (AC) functions in each vector.\n";
descr += "\n";
descr += "The 2nd output, V, holds the noise variances for each vector in X.\n";
descr += "If X is a single vector, then V is a scalar.\n";
descr += "\n";
descr += "Use -d (--dim) to give the dimension along which to operate.\n";
descr += "Default is 0 (along cols), unless X is a row vector.\n";
descr += "\n";
descr += "Y has the same size and data type as X.\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ ac2poly X -o Y -o V \n";
descr += "$ ac2poly -d1 X -o Y -o V \n";
descr += "$ cat X | ac2poly > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_int    *a_d = arg_intn("d","dim","<uint>",0,1,"dimension along which to operate [default=0]");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output files (Y,V)");

//Get options

//Get dim
if (a_d->count==0) { dim = i1.isrowvec() ? 1u : 0u; }
else if (a_d->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "dim must be nonnegative" << endl; return 1; }
else { dim = size_t(a_d->ival[0]); }
if (dim>3u) { cerr << progstr+": " << __LINE__ << errstr << "dim must be in {0,1,2,3}" << endl; return 1; }

//Checks
if (i1.isempty()) { cerr << progstr+": " << __LINE__ << errstr << "input (X) found to be empty" << endl; return 1; }

//Set output header info
o1.F = o2.F = i1.F;
o1.T = i1.T;
o2.T = i1.isreal() ? i1.T : i1.T-100u;
o1.R = i1.R; o1.C = i1.C;
o1.S = i1.S; o1.H = i1.H;
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
    if (codee::ac2poly_s(Y,V,X,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),dim)) { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; }
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
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file 1 (Y)" << endl; return 1; }
    try { V = new float[o2.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file 2 (V)" << endl; return 1; }
    try { ifs1.read(reinterpret_cast<char*>(X),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file (X)" << endl; return 1; }
    if (codee::ac2poly_c(Y,V,X,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),dim)) { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; }
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
