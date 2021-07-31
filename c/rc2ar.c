//Gets autoregressive (AR) params from reflection coefficients (RCs) for each vector in X.
//Input (X) and output (Y) have the same size.

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
namespace codee {
extern "C" {
#endif

int rc2ar_s (float *Y, const float *X, const size_t R, const size_t C, const size_t S, const size_t H, const char iscolmajor, const size_t dim);
int rc2ar_d (double *Y, const double *X, const size_t R, const size_t C, const size_t S, const size_t H, const char iscolmajor, const size_t dim);
int rc2ar_c (float *Y, const float *X, const size_t R, const size_t C, const size_t S, const size_t H, const char iscolmajor, const size_t dim);
int rc2ar_z (double *Y, const double *X, const size_t R, const size_t C, const size_t S, const size_t H, const char iscolmajor, const size_t dim);


int rc2ar_s (float *Y, const float *X, const size_t R, const size_t C, const size_t S, const size_t H, const char iscolmajor, const size_t dim)
{
    if (dim>3u) { fprintf(stderr,"error in rc2ar_s: dim must be in [0 3]\n"); return 1; }

    const size_t N = R*C*S*H;
    const size_t L = (dim==0u) ? R : (dim==1u) ? C : (dim==2u) ? S : H;

    if (N==0u) {}
    else
    {
        float sc, *y;
        if (!(y=(float *)malloc(L*sizeof(float)))) { fprintf(stderr,"error in rc2ar_s: problem with malloc. "); perror("malloc"); return 1; }

        if (L==N)
        {
            for (size_t l=0u; l<L; ++l, ++X, ++Y) { *Y = *X; }
            Y -= L;
            for (size_t l=1u; l<L; ++l)
            {
                for (size_t q=0u; q<l+1u; ++q, ++Y, ++y) { *y = *Y; }
                Y -= l + 1u; sc = *--y;
                for (size_t q=0u; q<l; ++q, ++Y) { --y; *Y -= sc * *y; }
                Y -= l;
            }
            for (size_t l=0u; l<L; ++l, ++Y) { *Y = -*Y; }
        }
        else
        {
            const size_t K = (iscolmajor) ? ((dim==0u) ? 1u : (dim==1u) ? R : (dim==2u) ? R*C : R*C*S) : ((dim==0u) ? C*S*H : (dim==1u) ? S*H : (dim==2u) ? H : 1u);
            const size_t B = (iscolmajor && dim==0u) ? C*S*H : K;
            const size_t V = N/L, G = V/B;

            if (K==1u && (G==1u || B==1u))
            {
                for (size_t v=0u; v<V; ++v)
                {
                    for (size_t l=0u; l<L; ++l, ++X, ++Y) { *Y = *X; }
                    Y -= L;
                    for (size_t l=1u; l<L; ++l)
                    {
                        for (size_t q=0u; q<l+1u; ++q, ++Y, ++y) { *y = *Y; }
                        Y -= l + 1u; sc = *--y;
                        for (size_t q=0u; q<l; ++q, ++Y) { --y; *Y -= sc * *y; }
                        Y -= l;
                    }
                    for (size_t l=0u; l<L; ++l, ++Y) { *Y = -*Y; }
                }
            }
            else
            {
                for (size_t g=0u; g<G; ++g, X+=B*(L-1u), Y+=B*(L-1u))
                {
                    for (size_t b=0u; b<B; ++b, X-=K*L-1u, Y-=K*L-1u)
                    {
                        for (size_t l=0u; l<L; ++l, X+=K, Y+=K) { *Y = *X; }
                        Y -= K*L;
                        for (size_t l=1u; l<L; ++l)
                        {
                            for (size_t q=0u; q<l+1u; ++q, Y+=K, ++y) { *y = *Y; }
                            Y -= K*(l+1u); sc = *--y;
                            for (size_t q=0u; q<l; ++q, Y+=K) { --y; *Y -= sc * *y; }
                            Y -= K*l;
                        }
                        for (size_t l=0u; l<L; ++l, Y+=K) { *Y = -*Y; }
                    }
                }
            }
        }
        free(y);
    }

    return 0;
}


int rc2ar_d (double *Y, const double *X, const size_t R, const size_t C, const size_t S, const size_t H, const char iscolmajor, const size_t dim)
{
    if (dim>3u) { fprintf(stderr,"error in rc2ar_d: dim must be in [0 3]\n"); return 1; }

    const size_t N = R*C*S*H;
    const size_t L = (dim==0u) ? R : (dim==1u) ? C : (dim==2u) ? S : H;

    if (N==0u) {}
    else
    {
        double sc, *y;
        if (!(y=(double *)malloc(L*sizeof(double)))) { fprintf(stderr,"error in rc2ar_d: problem with malloc. "); perror("malloc"); return 1; }

        if (L==N)
        {
            for (size_t l=0u; l<L; ++l, ++X, ++Y) { *Y = *X; }
            Y -= L;
            for (size_t l=1u; l<L; ++l)
            {
                for (size_t q=0u; q<l+1u; ++q, ++Y, ++y) { *y = *Y; }
                Y -= l + 1u; sc = *--y;
                for (size_t q=0u; q<l; ++q, ++Y) { --y; *Y -= sc * *y; }
                Y -= l;
            }
            for (size_t l=0u; l<L; ++l, ++Y) { *Y = -*Y; }
        }
        else
        {
            const size_t K = (iscolmajor) ? ((dim==0u) ? 1u : (dim==1u) ? R : (dim==2u) ? R*C : R*C*S) : ((dim==0u) ? C*S*H : (dim==1u) ? S*H : (dim==2u) ? H : 1u);
            const size_t B = (iscolmajor && dim==0u) ? C*S*H : K;
            const size_t V = N/L, G = V/B;

            if (K==1u && (G==1u || B==1u))
            {
                for (size_t v=0u; v<V; ++v)
                {
                    for (size_t l=0u; l<L; ++l, ++X, ++Y) { *Y = *X; }
                    Y -= L;
                    for (size_t l=1u; l<L; ++l)
                    {
                        for (size_t q=0u; q<l+1u; ++q, ++Y, ++y) { *y = *Y; }
                        Y -= l + 1u; sc = *--y;
                        for (size_t q=0u; q<l; ++q, ++Y) { --y; *Y -= sc * *y; }
                        Y -= l;
                    }
                    for (size_t l=0u; l<L; ++l, ++Y) { *Y = -*Y; }
                }
            }
            else
            {
                for (size_t g=0u; g<G; ++g, X+=B*(L-1u), Y+=B*(L-1u))
                {
                    for (size_t b=0u; b<B; ++b, X-=K*L-1u, Y-=K*L-1u)
                    {
                        for (size_t l=0u; l<L; ++l, X+=K, Y+=K) { *Y = *X; }
                        Y -= K*L;
                        for (size_t l=1u; l<L; ++l)
                        {
                            for (size_t q=0u; q<l+1u; ++q, Y+=K, ++y) { *y = *Y; }
                            Y -= K*(l+1u); sc = *--y;
                            for (size_t q=0u; q<l; ++q, Y+=K) { --y; *Y -= sc * *y; }
                            Y -= K*l;
                        }
                        for (size_t l=0u; l<L; ++l, Y+=K) { *Y = -*Y; }
                    }
                }
            }
        }
        free(y);
    }

    return 0;
}


int rc2ar_c (float *Y, const float *X, const size_t R, const size_t C, const size_t S, const size_t H, const char iscolmajor, const size_t dim)
{
    if (dim>3u) { fprintf(stderr,"error in rc2ar_c: dim must be in [0 3]\n"); return 1; }

    const size_t N = R*C*S*H;
    const size_t L = (dim==0u) ? R : (dim==1u) ? C : (dim==2u) ? S : H;

    if (N==0u) {}
    else
    {
        float scr, sci, *y;
        if (!(y=(float *)malloc(L*sizeof(float)))) { fprintf(stderr,"error in rc2ar_c: problem with malloc. "); perror("malloc"); return 1; }

        if (L==N)
        {
            for (size_t l=0u; l<2u*L; ++l, ++X, ++Y) { *Y = *X; }
            Y -= 2u*L;
            for (size_t l=1u; l<L; ++l)
            {
                for (size_t q=0u; q<2u*l+2u; ++q, ++y, ++Y) { *y = *Y; }
                Y -= 2u*l + 2u; sci = *--y; scr = *--y;
                for (size_t q=0u; q<l; ++q)
                {
                    y -= 2;
                    *Y++ -= scr**y - sci**(y+1);
                    *Y++ -= scr**(y+1) + sci**y;
                }
                Y -= 2u*l;
            }
            for (size_t l=0u; l<2u*L; ++l, ++Y) { *Y = -*Y; }
        }
        else
        {
            const size_t K = (iscolmajor) ? ((dim==0u) ? 1u : (dim==1u) ? R : (dim==2u) ? R*C : R*C*S) : ((dim==0u) ? C*S*H : (dim==1u) ? S*H : (dim==2u) ? H : 1u);
            const size_t B = (iscolmajor && dim==0u) ? C*S*H : K;
            const size_t V = N/L, G = V/B;

            if (K==1u && (G==1u || B==1u))
            {
                for (size_t v=0u; v<V; ++v)
                {
                    for (size_t l=0u; l<2u*L; ++l, ++X, ++Y) { *Y = *X; }
                    Y -= 2u*L;
                    for (size_t l=1u; l<L; ++l)
                    {
                        for (size_t q=0u; q<2u*l+2u; ++q, ++Y, ++y) { *y = *Y; }
                        Y -= 2u*l + 2u; sci = *--y; scr = *--y;
                        for (size_t q=0u; q<l; ++q)
                        {
                            y -= 2;
                            *Y++ -= scr**y - sci**(y+1);
                            *Y++ -= scr**(y+1) + sci**y;
                        }
                        Y -= 2u*l;
                    }
                    for (size_t l=0u; l<2u*L; ++l, ++Y) { *Y = -*Y; }
                }
            }
            else
            {
                for (size_t g=0u; g<G; ++g, X+=2u*B*(L-1u), Y+=2u*B*(L-1u))
                {
                    for (size_t b=0u; b<B; ++b, X-=2u*K*L-2u, Y-=2u*K*L-2u)
                    {
                        for (size_t l=0u; l<L; ++l, X+=2u*K, Y+=2u*K) { *Y = *X; *(Y+1) = *(X+1); }
                        Y -= 2u*K*L;
                        for (size_t l=1u; l<L; ++l)
                        {
                            for (size_t q=0u; q<l+1u; ++q, Y+=2u*K) { *y++ = *Y; *y++ = *(Y+1); }
                            Y -= 2u*K*(l+1u); sci = *--y; scr = *--y;
                            for (size_t q=0u; q<l; ++q, Y+=2u*K)
                            {
                                y -= 2;
                                *Y -= scr**y - sci**(y+1);
                                *(Y+1) -= scr**(y+1) + sci**y;
                            }
                            Y -= 2u*K*l;
                        }
                        for (size_t l=0u; l<L; ++l, Y+=2u*K) { *Y = -*Y; *(Y+1) = -*(Y+1); }
                    }
                }
            }
        }
        free(y);
    }

    return 0;
}


int rc2ar_z (double *Y, const double *X, const size_t R, const size_t C, const size_t S, const size_t H, const char iscolmajor, const size_t dim)
{
    if (dim>3u) { fprintf(stderr,"error in rc2ar_z: dim must be in [0 3]\n"); return 1; }

    const size_t N = R*C*S*H;
    const size_t L = (dim==0u) ? R : (dim==1u) ? C : (dim==2u) ? S : H;

    if (N==0u) {}
    else
    {
        double scr, sci, *y;
        if (!(y=(double *)malloc(L*sizeof(double)))) { fprintf(stderr,"error in rc2ar_z: problem with malloc. "); perror("malloc"); return 1; }

        if (L==N)
        {
            for (size_t l=0u; l<2u*L; ++l, ++X, ++Y) { *Y = *X; }
            Y -= 2u*L;
            for (size_t l=1u; l<L; ++l)
            {
                for (size_t q=0u; q<2u*l+2u; ++q, ++y, ++Y) { *y = *Y; }
                Y -= 2u*l + 2u; sci = *--y; scr = *--y;
                for (size_t q=0u; q<l; ++q)
                {
                    y -= 2;
                    *Y++ -= scr**y - sci**(y+1);
                    *Y++ -= scr**(y+1) + sci**y;
                }
                Y -= 2u*l;
            }
            for (size_t l=0u; l<2u*L; ++l, ++Y) { *Y = -*Y; }
        }
        else
        {
            const size_t K = (iscolmajor) ? ((dim==0u) ? 1u : (dim==1u) ? R : (dim==2u) ? R*C : R*C*S) : ((dim==0u) ? C*S*H : (dim==1u) ? S*H : (dim==2u) ? H : 1u);
            const size_t B = (iscolmajor && dim==0u) ? C*S*H : K;
            const size_t V = N/L, G = V/B;

            if (K==1u && (G==1u || B==1u))
            {
                for (size_t v=0u; v<V; ++v)
                {
                    for (size_t l=0u; l<2u*L; ++l, ++X, ++Y) { *Y = *X; }
                    Y -= 2u*L;
                    for (size_t l=1u; l<L; ++l)
                    {
                        for (size_t q=0u; q<2u*l+2u; ++q, ++Y, ++y) { *y = *Y; }
                        Y -= 2u*l + 2u; sci = *--y; scr = *--y;
                        for (size_t q=0u; q<l; ++q)
                        {
                            y -= 2;
                            *Y++ -= scr**y - sci**(y+1);
                            *Y++ -= scr**(y+1) + sci**y;
                        }
                        Y -= 2u*l;
                    }
                    for (size_t l=0u; l<2u*L; ++l, ++Y) { *Y = -*Y; }
                }
            }
            else
            {
                for (size_t g=0u; g<G; ++g, X+=2u*B*(L-1u), Y+=2u*B*(L-1u))
                {
                    for (size_t b=0u; b<B; ++b, X-=2u*K*L-2u, Y-=2u*K*L-2u)
                    {
                        for (size_t l=0u; l<L; ++l, X+=2u*K, Y+=2u*K) { *Y = *X; *(Y+1) = *(X+1); }
                        Y -= 2u*K*L;
                        for (size_t l=1u; l<L; ++l)
                        {
                            for (size_t q=0u; q<l+1u; ++q, Y+=2u*K) { *y++ = *Y; *y++ = *(Y+1); }
                            Y -= 2u*K*(l+1u); sci = *--y; scr = *--y;
                            for (size_t q=0u; q<l; ++q, Y+=2u*K)
                            {
                                y -= 2;
                                *Y -= scr**y - sci**(y+1);
                                *(Y+1) -= scr**(y+1) + sci**y;
                            }
                            Y -= 2u*K*l;
                        }
                        for (size_t l=0u; l<L; ++l, Y+=2u*K) { *Y = -*Y; *(Y+1) = -*(Y+1); }
                    }
                }
            }
        }
        free(y);
    }

    return 0;
}


#ifdef __cplusplus
}
}
#endif
