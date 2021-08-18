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
#include "../util/cmli.hpp"
#include "conv.c"

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
    const valarray<size_t> oktypes = {1u};
    const size_t I = 2u, O = 1u;
    ifstream ifs1, ifs2; ofstream ofs1;
    int8_t stdi1, stdi2, stdo1, wo1;
    ioinfo i1, i2, o1;
    size_t dim, Lx, W;
    string shape;


    //Description
    string descr;
    descr += "1D convolution by B of each vector (1D signal) in X.\n";
    descr += "This version emulates the conv function of Octave.\n";
    descr += "For more options (stride, dilation), see conv1d.\n";
    descr += "\n";
    descr += "B is in reverse chronological order (usual convention).\n";
    descr += "This performs non-causal convolution; use fir for causal.\n";
    descr += "\n";
    descr += "Use -d (--dim) to give the dimension (axis) along which to filter.\n";
    descr += "Use -d0 to operate along cols, -d1 to operate along rows, etc.\n";
    descr += "The default is 0 (along cols), unless X is a row vector.\n";
    descr += "\n";
    descr += "Use -s (--shape) to give the shape as 'full', 'same' or 'valid' [default='full'].\n";
    descr += "For 'same', Y has length Lx along dim (same as X).\n";
    descr += "For 'full', Y has length Lx+2*Lb-2 along dim.\n";
    descr += "For 'valid', Y has length Lx-Lb+1 along dim.\n";
    descr += "\n";
    descr += "Examples:\n";
    descr += "$ conv X B -o Y \n";
    descr += "$ conv -d1 X B > Y \n";
    descr += "$ cat X | conv -d1 -s'full' - B > Y \n";


    //Argtable
    int nerrs;
    struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input files (X,B)");
    struct arg_str   *a_sh = arg_strn("s","shape","<str>",0,1,"shape [default='full']");
    struct arg_int    *a_d = arg_intn("d","dim","<uint>",0,1,"dimension along which to filter [default=0]");
    struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");
    struct arg_lit *a_help = arg_litn("h","help",0,1,"display this help and exit");
    struct arg_end  *a_end = arg_end(5);
    void *argtable[] = {a_fi, a_sh, a_d, a_fo, a_help, a_end};
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
    stdi1 = (a_fi->count==0 || strlen(a_fi->filename[0])==0u || strcmp(a_fi->filename[0],"-")==0);
    stdi2 = (a_fi->count<=1 || strlen(a_fi->filename[1])==0u || strcmp(a_fi->filename[1],"-")==0);
    if (stdi1+stdi2>1) { cerr << progstr+": " << __LINE__ << errstr << "can only use stdin for one input" << endl; return 1; }
    if (stdi1+stdi2>0 && isatty(fileno(stdin))) { cerr << progstr+": " << __LINE__ << errstr << "no stdin detected" << endl; return 1; }


    //Check stdout
    if (a_fo->count>0) { stdo1 = (strlen(a_fo->filename[0])==0u || strcmp(a_fo->filename[0],"-")==0); }
    else { stdo1 = (!isatty(fileno(stdout))); }
    wo1 = (stdo1 || a_fo->count>0);


    //Open inputs
    if (stdi1) { ifs1.copyfmt(cin); ifs1.basic_ios<char>::rdbuf(cin.rdbuf()); } else { ifs1.open(a_fi->filename[0]); }
    if (!ifs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening input file 1" << endl; return 1; }
    if (stdi2) { ifs2.copyfmt(cin); ifs2.basic_ios<char>::rdbuf(cin.rdbuf()); } else { ifs2.open(a_fi->filename[1]); }
    if (!ifs2) { cerr << progstr+": " << __LINE__ << errstr << "problem opening input file 2" << endl; return 1; }


    //Read input headers
    if (!read_input_header(ifs1,i1)) { cerr << progstr+": " << __LINE__ << errstr << "problem reading header for input file 1" << endl; return 1; }
    if (!read_input_header(ifs2,i2)) { cerr << progstr+": " << __LINE__ << errstr << "problem reading header for input file 2" << endl; return 1; }
    if ((i1.T==oktypes).sum()==0 || (i2.T==oktypes).sum()==0)
    {
        cerr << progstr+": " << __LINE__ << errstr << "input data type must be in " << "{";
        for (auto o : oktypes) { cerr << int(o) << ((o==oktypes[oktypes.size()-1u]) ? "}" : ","); }
        cerr << endl; return 1;
    }


    //Get options

    //Get dim
    if (a_d->count==0) { dim = i1.isvec() ? i1.nonsingleton1() : 0u; }
    else if (a_d->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "dim must be nonnegative" << endl; return 1; }
    else { dim = size_t(a_d->ival[0]); }
    if (dim>3u) { cerr << progstr+": " << __LINE__ << errstr << "dim must be in {0,1,2,3}" << endl; return 1; }

    //Get shape
    if (a_sh->count==0) { shape = "full"; }
    else
    {
    	try { shape = string(a_sh->sval[0]); }
    	catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem getting string for shape" << endl; return 1; }
    }
    for (string::size_type c=0u; c<shape.size(); ++c) { shape[c] = char(tolower(shape[c])); }
    if (shape!="full" && shape!="same" && shape!="valid") { cerr << progstr+": " << __LINE__ << errstr << "shape string must be 'full', 'same' or 'valid'" << endl; return 1; }


    //Checks
    if (i1.T!=i2.T) { cerr << progstr+": " << __LINE__ << errstr << "inputs must have the same data type" << endl; return 1; }
    if (i1.isempty()) { cerr << progstr+": " << __LINE__ << errstr << "input 1 (X) found to be empty" << endl; return 1; }
    if (i2.isempty()) { cerr << progstr+": " << __LINE__ << errstr << "input 2 (B) found to be empty" << endl; return 1; }
    if (!i2.isvec()) { cerr << progstr+": " << __LINE__ << errstr << "input 2 (B) must be a vector" << endl; return 1; }


    //Set output header info
    Lx = (dim==0u) ? i1.R : (dim==1u) ? i1.C : (dim==2u) ? i1.S : i1.H;
    if (shape=="full") { W = Lx + i2.N() - 1u; }
    else if (shape=="same") { W = Lx; }
    else { W = (Lx>=i2.N()) ? Lx-i2.N()+1u : 0u; }
    o1.F = i1.F; o1.T = i1.T;
    o1.R = (dim==0u) ? W : i1.R;
    o1.C = (dim==1u) ? W : i1.C;
    o1.S = (dim==2u) ? W : i1.S;
    o1.H = (dim==3u) ? W : i1.H;


    //Open output
    if (wo1)
    {
        if (stdo1) { ofs1.copyfmt(cout); ofs1.basic_ios<char>::rdbuf(cout.rdbuf()); } else { ofs1.open(a_fo->filename[0]); }
        if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening output file 1" << endl; return 1; }
    }


    //Write output header
    if (wo1 && !write_output_header(ofs1,o1)) { cerr << progstr+": " << __LINE__ << errstr << "problem writing header for output file 1" << endl; return 1; }


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
        if (codee::conv_s(Y,X,B,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),i2.N(),shape.c_str(),dim))
        { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; } 
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(Y),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
        delete[] X; delete[] B; delete[] Y;
    }
    else
    {
        cerr << progstr+": " << __LINE__ << errstr << "data type not supported" << endl; return 1;
    }
    

    //Finish
    // else if (i1.T==101u)
    // {
    //     float *X, *B, *Y;
    //     try { X = new float[2u*i1.N()]; }
    //     catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file 1 (X)" << endl; return 1; }
    //     try { B = new float[2u*i2.N()]; }
    //     catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file 2 (B)" << endl; return 1; }
    //     try { Y = new float[2u*o1.N()]; }
    //     catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for output file (Y)" << endl; return 1; }
    //     try { ifs1.read(reinterpret_cast<char*>(X),i1.nbytes()); }
    //     catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X)" << endl; return 1; }
    //     try { ifs2.read(reinterpret_cast<char*>(B),i2.nbytes()); }
    //     catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (B)" << endl; return 1; }
    //     if (codee::fir_c(Y,X,B,i1.R,i1.C,i1.S,i1.H,i1.iscolmajor(),Q,shape.c_str(),dim))
    //     { cerr << progstr+": " << __LINE__ << errstr << "problem during function call" << endl; return 1; } 
    //     if (wo1)
    //     {
    //         try { ofs1.write(reinterpret_cast<char*>(Y),o1.nbytes()); }
    //         catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
    //     }
    //     delete[] X; delete[] B; delete[] Y;
    // }


    //Exit
    return ret;
}
