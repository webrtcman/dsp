//@author Erik Edwards
//@date 2018-present
//@license BSD 3-clause


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <valarray>
#include <unordered_map>
#include <argtable2.h>
#include "cmli.hpp"
#include "sig2poly_levdurb.c"

#ifdef I
#undef I
#endif


int main(int argc, char *argv[])
{
    using namespace std;


    //Declarations
    int ret = 0;
    const string errstr = ": \033[1;31merror:\033[0m ";
    const string warstr = ": \033[1;35mwarning:\033[0m ";
    const string progstr(__FILE__,string(__FILE__).find_last_of("/")+1,strlen(__FILE__)-string(__FILE__).find_last_of("/")-5);
    const valarray<size_t> oktypes = {1u,2u,101u,102u};
    const size_t I = 1u, O = 2u;
    ifstream ifs1; ofstream ofs1, ofs2;
    int8_t stdi1, stdo1, stdo2, wo1, wo2;
    ioinfo i1, o1, o2;
    size_t dim, L, Lx;
    int mnz, u;


    //Description
    string descr;
    descr += "Gets autoregressive (AR) coeffs starting from the signal (sig).\n";
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
    descr += "$ sig2poly_levdurb -l3 X -o Y -o V \n";
    descr += "$ sig2poly_levdurb -d1 -l5 X -o Y -o V \n";
    descr += "$ cat X | sig2poly_levdurb -z -l7 > Y \n";


    //Argtable
    int nerrs;
    struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
    struct arg_int    *a_l = arg_intn("l","L","<uint>",0,1,"number of AC lags to compute [default=1]");
    struct arg_int    *a_d = arg_intn("d","dim","<uint>",0,1,"dimension along which to operate [default=0]");
    struct arg_lit  *a_mnz = arg_litn("z","zero_mean",0,1,"subtract mean from each vec in X [default=false]");
    struct arg_lit    *a_u = arg_litn("u","unbiased",0,1,"use unbiased (N-l) denominator [default=biased]");
    struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output files (Y,V)");
    struct arg_lit *a_help = arg_litn("h","help",0,1,"display this help and exit");
    struct arg_end  *a_end = arg_end(5);
    void *argtable[] = {a_fi, a_l, a_d, a_mnz, a_u, a_fo, a_help, a_end};
    if (arg_nullcheck(argtable)!=0) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating argtable" << endl; return 1; }
    nerrs = arg_parse(argc, argv, argtable);
    if (a_help->count>0)
    {
        cout << "Usage: " << progstr; arg_print_syntax(stdout, argtable, "\n");
        cout << endl; arg_print_glossary(stdout, argtable, "  %-25s %s\n");
        cout << endl << descr; return 1;
    }
    if (nerrs>0) { arg_print_errors(stderr,a_end,(progstr+": "+to_string(__LINE__)+errstr).c_str()); return 1; }


    //Check stdin
    stdi1 = (a_fi->count==0 || strlen(a_fi->filename[0])==0 || strcmp(a_fi->filename[0],"-")==0);
    if (stdi1>0 && isatty(fileno(stdin))) { cerr << progstr+": " << __LINE__ << errstr << "no stdin detected" << endl; return 1; }


    //Check stdout
    if (a_fo->count>0) { stdo1 = (strlen(a_fo->filename[0])==0 || strcmp(a_fo->filename[0],"-")==0); }
    else { stdo1 = (!isatty(fileno(stdout))); }
    if (a_fo->count>1) { stdo2 = (strlen(a_fo->filename[1])==0 || strcmp(a_fo->filename[1],"-")==0); }
    else { stdo2 = (!isatty(fileno(stdout)) && a_fo->count==1 && stdo1==0); }
    if (stdo1+stdo2>1) { cerr << progstr+": " << __LINE__ << errstr << "can only use stdout for one output" << endl; return 1; }
    wo1 = (stdo1 || a_fo->count>0); wo2 = (stdo2 || a_fo->count>1);


    //Open input
    if (stdi1) { ifs1.copyfmt(cin); ifs1.basic_ios<char>::rdbuf(cin.rdbuf()); } else { ifs1.open(a_fi->filename[0]); }
    if (!ifs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening input file" << endl; return 1; }


    //Read input header
    if (!read_input_header(ifs1,i1)) { cerr << progstr+": " << __LINE__ << errstr << "problem reading header for input file" << endl; return 1; }
    if ((i1.T==oktypes).sum()==0)
    {
        cerr << progstr+": " << __LINE__ << errstr << "input data type must be in " << "{";
        for (auto o : oktypes) { cerr << int(o) << ((o==oktypes[oktypes.size()-1u]) ? "}" : ","); }
        cerr << endl; return 1;
    }


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


    //Set output header infos
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


    //Open outputs
    if (wo1)
    {
        if (stdo1) { ofs1.copyfmt(cout); ofs1.basic_ios<char>::rdbuf(cout.rdbuf()); } else { ofs1.open(a_fo->filename[0]); }
        if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening output file 1" << endl; return 1; }
    }
    if (wo2)
    {
        if (stdo2) { ofs2.copyfmt(cout); ofs2.basic_ios<char>::rdbuf(cout.rdbuf()); } else { ofs2.open(a_fo->filename[1]); }
        if (!ofs2) { cerr << progstr+": " << __LINE__ << errstr << "problem opening output file 2" << endl; return 1; }
    }


    //Write output headers
    if (wo1 && !write_output_header(ofs1,o1)) { cerr << progstr+": " << __LINE__ << errstr << "problem writing header for output file 1" << endl; return 1; }
    if (wo2 && !write_output_header(ofs2,o2)) { cerr << progstr+": " << __LINE__ << errstr << "problem writing header for output file 2" << endl; return 1; }


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
        if (codee::sig2poly_levdurb_s(Y,V,X,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),dim,L,mnz,u)) { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; }
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
    else if (i1.T==2)
    {
        double *X, *Y, *V;
        try { X = new double[i1.N()]; }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file (X)" << endl; return 1; }
        try { Y = new double[o1.N()]; }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file 1 (Y)" << endl; return 1; }
        try { V = new double[o2.N()]; }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file 2 (V)" << endl; return 1; }
        try { ifs1.read(reinterpret_cast<char*>(X),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file (X)" << endl; return 1; }
        if (codee::sig2poly_levdurb_d(Y,V,X,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),dim,L,mnz,u)) { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; }
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
        if (codee::sig2poly_levdurb_c(Y,V,X,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),dim,L,mnz,u)) { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; }
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
    else if (i1.T==102u)
    {
        double *X, *Y, *V;
        try { X = new double[2u*i1.N()]; }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file (X)" << endl; return 1; }
        try { Y = new double[2u*o1.N()]; }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file (Y)" << endl; return 1; }
        try { V = new double[o2.N()]; }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file 2 (V)" << endl; return 1; }
        try { ifs1.read(reinterpret_cast<char*>(X),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file (X)" << endl; return 1; }
        if (codee::sig2poly_levdurb_z(Y,V,X,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),dim,L,mnz,u)) { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; }
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
    else
    {
        cerr << progstr+": " << __LINE__ << errstr << "data type not supported" << endl; return 1;
    }
    

    //Exit
    return ret;
}

