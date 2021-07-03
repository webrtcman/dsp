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
#include <cfloat>
#include "violet.c"


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
    const size_t O = 1u;
    ofstream ofs1;
    int8_t stdo1, wo1;
    ioinfo o1;
    size_t N, dim;
    char zmn;
    double std;
    


    //Description
    string descr;
    descr += "Zero-mean, Gaussian violet noise (1-D).\n";
    descr += "Makes vector of Gaussian white noise with specified stddev, and then\n";
    descr += "differentiates (diff) to output the violet noise (f^2 characteristic).\n";
    descr += "\n";
    descr += "This uses modified code from PCG random, but does not require it to be installed.\n";
    descr += "\n";
    descr += "Use -n (--N) to give the number of samples in the output vec.\n";
    descr += "\n";
    descr += "Use -d (--dim) to give the nonsingleton dim of the output vec.\n";
    descr += "If d=0, then Y is a column vector [default].\n";
    descr += "If d=1, then Y is a row vector.\n";
    descr += "(d=2 and d=3 are also possible, but rarely used.)\n";
    descr += "\n";
    descr += "Use -z (--zero_mean) to zero the mean before output.\n";
    descr += "Although the expected value of the mean is 0, the generated mean is not.\n";
    descr += "\n";
    descr += "For complex output, real/imag parts are separately set using the same params.\n";
    descr += "\n";
    descr += "Examples:\n";
    descr += "$ violet -z -n16 -o Y \n";
    descr += "$ violet -u -d1 -n16 -t1 > Y \n";
    descr += "$ violet -d1 -n16 -t102 > Y \n";
    
    //Argtable
    struct arg_dbl  *a_std = arg_dbln("s","std","<dbl>",0,1,"std dev parameter [default=1.0]");
    struct arg_lit  *a_zmn = arg_litn("z","zero_mean",0,1,"include to zero mean before output");
    struct arg_int    *a_n = arg_intn("n","N","<uint>",0,1,"num samples in output [default=1]");
    struct arg_int    *a_d = arg_intn("d","dim","<uint>",0,1,"nonsingleton dimension [default=0 -> col vec]");
    struct arg_int *a_otyp = arg_intn("t","type","<uint>",0,1,"output data type [default=1]");
    struct arg_int *a_ofmt = arg_intn("f","fmt","<uint>",0,1,"output file format [default=147]");
    struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");
    
    //Get options
    
    //Get o1.F
    if (a_ofmt->count==0) { o1.F = 147u; }
    else if (a_ofmt->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "output file format must be nonnegative" << endl; return 1; }
    else if (a_ofmt->ival[0]>255) { cerr << progstr+": " << __LINE__ << errstr << "output file format must be < 256" << endl; return 1; }
    else { o1.F = size_t(a_ofmt->ival[0]); }
    
    //Get o1.T
    if (a_otyp->count==0) { o1.T = 1u; }
    else if (a_otyp->ival[0]<1) { cerr << progstr+": " << __LINE__ << errstr << "output data type must be positive" << endl; return 1; }
    else { o1.T = size_t(a_otyp->ival[0]); }
    if ((o1.T==oktypes).sum()==0)
    {
