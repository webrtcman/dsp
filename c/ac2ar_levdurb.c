//Gets autoregressive (AR) params from the autocorrelation (AC) function for each vector in X.
//Uses a Levinson-Durbin recursion from the AC values.

//The complex-valued case here uses multiply and divide with no conjugation.
//This matches the real-valued case (i.e., make a complex-valued input with 0 imaginary parts),
//but does not match Octave. However, I can't even match the very first number with Octave,
//even after trying every conj or no-conj combination, so something else is going on.
//Thus, if really requiring the complex-valued case, reconsider first.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
namespace codee {
extern "C" {
#endif

int ac2ar_levdurb_s (float *Y, const float *X, const size_t R, const size_t C, const size_t S, const size_t H, const int iscolmajor, const size_t dim);
int ac2ar_levdurb_d (double *Y, const double *X, const size_t R, const size_t C, const size_t S, const size_t H, const int iscolmajor, const size_t dim);
int ac2ar_levdurb_c (float *Y, const float *X, const size_t R, const size_t C, const size_t S, const size_t H, const int iscolmajor, const size_t dim);
int ac2ar_levdurb_z (double *Y, const double *X, const size_t R, const size_t C, const size_t S, const size_t H, const int iscolmajor, const size_t dim);


int ac2ar_levdurb_s (float *Y, const float *X, const size_t R, const size_t C, const size_t S, const size_t H, const int iscolmajor, const size_t dim)
{
    if (dim>3u) { fprintf(stderr,"error in ac2ar_levdurb_s: dim must be in [0 3]\n"); return 1; }

    const size_t N = R*C*S*H;
    const size_t Lx = (dim==0u) ? R : (dim==1u) ? C : (dim==2u) ? S : H;

    if (N==0u || Lx<2u) {}
    else if (Lx==2u)
    {
        if (Lx==N) { *Y++ = *(X+1) / *X; }
        else
        {
            const size_t K = (iscolmajor) ? ((dim==0u) ? 1u : (dim==1u) ? R : (dim==2u) ? R*C : R*C*S) : ((dim==0u) ? C*S*H : (dim==1u) ? S*H : (dim==2u) ? H : 1u);
            const size_t B = (iscolmajor && dim==0u) ? C*S*H : K;
            const size_t V = N/Lx, G = V/B;

            if (K==1u && (G==1u || B==1u))
            {
                for (size_t v=0u; v<V; ++v, X+=2) { *Y++ = *(X+1) / *X; }
            }
            else
            {
                for (size_t g=0u; g<G; ++g, X+=B)
                {
                    for (size_t b=0u; b<B; ++b, ++X, ++Y) { *Y = *(X+K) / *X; }
                }
            }
        }
    }
    else
    {
        const size_t P = Lx - 1u;
        float *A, a, e;
        if (!(A=(float *)malloc((P-1u)*sizeof(float)))) { fprintf(stderr,"error in ac2ar_levdurb_s: problem with malloc. "); perror("malloc"); return 1; }
        
        if (Lx==N)
        {
            a = -*(X+1) / *X;
            *Y = -a; //*rcs++ = a;
            e = *X++; e += a * *X++;
            for (size_t p=1u; p<P; ++p, X+=p)
            {
                a = *X;
                for (size_t q=0u; q<p; ++q, ++Y) { --X; a -= *X * *Y; }
                a /= -e;
                *Y = -a; //*rcs++ = a;
                for (size_t q=0u; q<p; ++q, ++A) { --Y; *A = *Y; }
                Y += p;
                for (size_t q=0u; q<p; ++q) { --A; --Y; *Y += a * *A; }
                e *= 1.0f - a*a;
            }
        }
        else
        {
            const size_t K = (iscolmajor) ? ((dim==0u) ? 1u : (dim==1u) ? R : (dim==2u) ? R*C : R*C*S) : ((dim==0u) ? C*S*H : (dim==1u) ? S*H : (dim==2u) ? H : 1u);
            const size_t B = (iscolmajor && dim==0u) ? C*S*H : K;
            const size_t V = N/Lx, G = V/B;

            if (K==1u && (G==1u || B==1u))
            {
                for (size_t v=0u; v<V; ++v, Y+=P)
                {
                    a = -*(X+1) / *X;
                    *Y = -a;
                    e = *X++; e += a * *X++;
                    for (size_t p=1u; p<P; ++p, X+=p)
                    {
                        a = *X;
                        for (size_t q=0u; q<p; ++q, ++Y) { --X; a -= *X * *Y; }
                        a /= -e;
                        *Y = -a;
                        for (size_t q=0u; q<p; ++q, ++A) { --Y; *A = *Y; }
                        Y += p;
                        for (size_t q=0u; q<p; ++q) { --A; --Y; *Y += a * *A; }
                        e *= 1.0f - a*a;
                    }
                }
            }
            else
            {
                for (size_t g=0u; g<G; ++g, X+=B*P, Y+=B*(P-1u))
                {
                    for (size_t b=0u; b<B; ++b, X-=K*Lx-1u, ++Y)
                    {
                        a = -*(X+K) / *X;
                        *Y = -a;
                        e = *X; X += K;
                        e += a * *X; X += K;
                        for (size_t p=1u; p<P; ++p, X+=p*K)
                        {
                            a = *X;
                            for (size_t q=0u; q<p; ++q, Y+=K) { X-=K; a -= *X * *Y; }
                            a /= -e;
                            *Y = -a;
                            for (size_t q=0u; q<p; ++q, ++A) { Y-=K; *A = *Y; }
                            Y += p*K;
                            for (size_t q=0u; q<p; ++q) { --A; Y-=K; *Y += a * *A; }
                            e *= 1.0f - a*a;
                        }
                    }
                }
            }
        }
        free(A);
    }
	
	return 0;
}


int ac2ar_levdurb_d (double *Y, const double *X, const size_t R, const size_t C, const size_t S, const size_t H, const int iscolmajor, const size_t dim)
{
	if (dim>3u) { fprintf(stderr,"error in ac2ar_levdurb_d: dim must be in [0 3]\n"); return 1; }

    const size_t N = R*C*S*H;
    const size_t Lx = (dim==0u) ? R : (dim==1u) ? C : (dim==2u) ? S : H;

    if (N==0u || Lx<2u) {}
    else if (Lx==2u)
    {
        if (Lx==N) { *Y++ = *(X+1) / *X; }
        else
        {
            const size_t K = (iscolmajor) ? ((dim==0u) ? 1u : (dim==1u) ? R : (dim==2u) ? R*C : R*C*S) : ((dim==0u) ? C*S*H : (dim==1u) ? S*H : (dim==2u) ? H : 1u);
            const size_t B = (iscolmajor && dim==0u) ? C*S*H : K;
            const size_t V = N/Lx, G = V/B;

            if (K==1u && (G==1u || B==1u))
            {
                for (size_t v=0u; v<V; ++v, X+=2) { *Y++ = *(X+1) / *X; }
            }
            else
            {
                for (size_t g=0u; g<G; ++g, X+=B)
                {
                    for (size_t b=0u; b<B; ++b, ++X, ++Y) { *Y = *(X+K) / *X; }
                }
            }
        }
    }
    else
    {
        const size_t P = Lx - 1u;
        double *A, a, e;
        if (!(A=(double *)malloc((P-1u)*sizeof(double)))) { fprintf(stderr,"error in ac2ar_levdurb_d: problem with malloc. "); perror("malloc"); return 1; }
        
        if (Lx==N)
        {
            a = -*(X+1) / *X;
            *Y = -a;
            e = *X++; e += a * *X++;
            for (size_t p=1u; p<P; ++p, X+=p)
            {
                a = *X;
                for (size_t q=0u; q<p; ++q, ++Y) { --X; a -= *X * *Y; }
                a /= -e;
                *Y = -a;
                for (size_t q=0u; q<p; ++q, ++A) { --Y; *A = *Y; }
                Y += p;
                for (size_t q=0u; q<p; ++q) { --A; --Y; *Y += a * *A; }
                e *= 1.0 - a*a;
            }
        }
        else
        {
            const size_t K = (iscolmajor) ? ((dim==0u) ? 1u : (dim==1u) ? R : (dim==2u) ? R*C : R*C*S) : ((dim==0u) ? C*S*H : (dim==1u) ? S*H : (dim==2u) ? H : 1u);
            const size_t B = (iscolmajor && dim==0u) ? C*S*H : K;
            const size_t V = N/Lx, G = V/B;

            if (K==1u && (G==1u || B==1u))
            {
                for (size_t v=0u; v<V; ++v, Y+=P)
                {
                    a = -*(X+1) / *X;
                    *Y = -a;
                    e = *X++; e += a * *X++;
                    for (size_t p=1u; p<P; ++p, X+=p)
                    {
                        a = *X;
                        for (size_t q=0u; q<p; ++q, ++Y) { --X; a -= *X * *Y; }
                        a /= -e;
                        *Y = -a;
                        for (size_t q=0u; q<p; ++q, ++A) { --Y; *A = *Y; }
                        Y += p;
                        for (size_t q=0u; q<p; ++q) { --A; --Y; *Y += a * *A; }
                        e *= 1.0 - a*a;
                    }
                }
            }
            else
            {
                for (size_t g=0u; g<G; ++g, X+=B*P, Y+=B*(P-1u))
                {
                    for (size_t b=0u; b<B; ++b, X-=K*Lx-1u, ++Y)
                    {
                        a = -*(X+K) / *X;
                        *Y = -a;
                        e = *X; X += K;
                        e += a * *X; X += K;
                        for (size_t p=1u; p<P; ++p, X+=p*K)
                        {
                            a = *X;
                            for (size_t q=0u; q<p; ++q, Y+=K) { X-=K; a -= *X * *Y; }
                            a /= -e;
                            *Y = -a;
                            for (size_t q=0u; q<p; ++q, ++A) { Y-=K; *A = *Y; }
                            Y += p*K;
                            for (size_t q=0u; q<p; ++q) { --A; Y-=K; *Y += a * *A; }
                            e *= 1.0 - a*a;
                        }
                    }
                }
            }
        }
        free(A);
    }
	
	return 0;
}


int ac2ar_levdurb_c (float *Y, const float *X, const size_t R, const size_t C, const size_t S, const size_t H, const int iscolmajor, const size_t dim)
{
    if (dim>3u) { fprintf(stderr,"error in ac2ar_levdurb_c: dim must be in [0 3]\n"); return 1; }

    const size_t N = R*C*S*H;
    const size_t Lx = (dim==0u) ? R : (dim==1u) ? C : (dim==2u) ? S : H;

    if (N==0u || Lx<2u) {}
    else if (Lx==2u)
    {
        if (Lx==N)
        {
            const float den = *X**X + *(X+1)**(X+1);
            *Y++ = (*X**(X+2) + *(X+1)**(X+3)) / den;
            *Y++ = (*X**(X+3) - *(X+1)**(X+2)) / den;
        }
        else
        {
            const size_t K = (iscolmajor) ? ((dim==0u) ? 1u : (dim==1u) ? R : (dim==2u) ? R*C : R*C*S) : ((dim==0u) ? C*S*H : (dim==1u) ? S*H : (dim==2u) ? H : 1u);
            const size_t B = (iscolmajor && dim==0u) ? C*S*H : K;
            const size_t V = N/Lx, G = V/B;
            float den;

            if (K==1u && (G==1u || B==1u))
            {
                for (size_t v=0u; v<V; ++v, X+=4)
                {
                    den = *X**X + *(X+1)**(X+1);
                    *Y++ = (*X**(X+2) + *(X+1)**(X+3)) / den;
                    *Y++ = (*X**(X+3) - *(X+1)**(X+2)) / den;
                }
            }
            else
            {
                for (size_t g=0u; g<G; ++g, X+=2u*B)
                {
                    for (size_t b=0u; b<B; ++b, X+=2u, Y+=2)
                    {
                        *Y = *(X+K) / *X;
                        den = *X**X + *(X+1)**(X+1);
                        *Y = (*X**(X+2u*K) + *(X+1)**(X+2u*K+1u)) / den;
                        *(Y+1) = (*X**(X+2u*K+1u) - *(X+1)**(X+2u*K)) / den;
                    }
                }
            }
        }
    }
    else
    {
        const size_t P = Lx - 1u;
        float *A, ar, ai, e, den;
        if (!(A=(float *)malloc(2u*(P-1u)*sizeof(float)))) { fprintf(stderr,"error in ac2ar_levdurb_c: problem with malloc. "); perror("malloc"); return 1; }
        
        if (Lx==N)
        {
            den = *X**X + *(X+1)**(X+1);
            ar = -(*(X+2)**X + *(X+3)**(X+1)) / den;
            ai = -(*(X+3)**X - *(X+1)**(X+2)) / den;
            *Y = -ar; *(Y+1) = -ai;
            e = *X * (1.0f - (ar*ar+ai*ai));
            X += 4;
            for (size_t p=1u; p<P; ++p, X+=2u*p)
            {
                ar = *X; ai = *(X+1);
                for (size_t q=0u; q<p; ++q, Y+=2)
                {
                    X -= 2;
                    ar -= *X**Y - *(X+1)**(Y+1);
                    ai -= *(X+1)**Y + *X**(Y+1);
                }
                ar /= -e; ai /= -e;
                *Y = -ar; *(Y+1) = -ai;
                for (size_t q=0u; q<p; ++q, A+=2) { Y-=2; *A = *Y; *(A+1) = -*(Y+1); }
                Y += 2u*p;
                for (size_t q=0u; q<p; ++q)
                {
                    A -= 2; Y -= 2;
                    *Y += ar**A - ai**(A+1);
                    *(Y+1) += ar**(A+1) + ai**A;
                }
                e *= 1.0f - (ar*ar + ai*ai);
            }
        }
        else
        {
            const size_t K = (iscolmajor) ? ((dim==0u) ? 1u : (dim==1u) ? R : (dim==2u) ? R*C : R*C*S) : ((dim==0u) ? C*S*H : (dim==1u) ? S*H : (dim==2u) ? H : 1u);
            const size_t B = (iscolmajor && dim==0u) ? C*S*H : K;
            const size_t V = N/Lx, G = V/B;

            if (K==1u && (G==1u || B==1u))
            {
                for (size_t v=0u; v<V; ++v, Y+=2u*P)
                {
                    den = *X**X + *(X+1)**(X+1);
                    ar = -(*(X+2)**X + *(X+3)**(X+1)) / den;
                    ai = -(*(X+3)**X - *(X+1)**(X+2)) / den;
                    *Y = -ar; *(Y+1) = -ai;
                    e = *X * (1.0f - (ar*ar+ai*ai));
                    X += 4;
                    for (size_t p=1u; p<P; ++p, X+=2u*p)
                    {
                        ar = *X; ai = *(X+1);
                        for (size_t q=0u; q<p; ++q, Y+=2)
                        {
                            X -= 2;
                            ar -= *X**Y - *(X+1)**(Y+1);
                            ai -= *(X+1)**Y + *X**(Y+1);
                        }
                        ar /= -e; ai /= -e;
                        *Y = -ar; *(Y+1) = -ai;
                        for (size_t q=0u; q<p; ++q, A+=2) { Y-=2; *A = *Y; *(A+1) = -*(Y+1); }
                        Y += 2u*p;
                        for (size_t q=0u; q<p; ++q)
                        {
                            A -= 2; Y -= 2;
                            *Y += ar**A - ai**(A+1);
                            *(Y+1) += ar**(A+1) + ai**A;
                        }
                        e *= 1.0f - (ar*ar + ai*ai);
                    }
                }
            }
            else
            {
                for (size_t g=0u; g<G; ++g, X+=2u*B*P, Y+=2u*B*(P-1u))
                {
                    for (size_t b=0u; b<B; ++b, X-=2u*K*Lx-2u, Y+=2)
                    {
                        den = *X**X + *(X+1)**(X+1);
                        ar = -(*(X+2u*K)**X + *(X+2u*K+1u)**(X+1)) / den;
                        ai = -(*(X+2u*K+1u)**X - *(X+1)**(X+2u*K)) / den;
                        *Y = -ar; *(Y+1) = -ai;
                        e = *X * (1.0f - (ar*ar+ai*ai));
                        X += 4u*K;
                        for (size_t p=1u; p<P; ++p, X+=2u*p*K)
                        {
                            ar = *X; ai = *(X+1);
                            for (size_t q=0u; q<p; ++q, Y+=2u*K)
                            {
                                X -= 2u*K;
                                ar -= *X**Y - *(X+1)**(Y+1);
                                ai -= *(X+1)**Y + *X**(Y+1);
                            }
                            ar /= -e; ai /= -e;
                            *Y = -ar; *(Y+1) = -ai;
                            for (size_t q=0u; q<p; ++q, A+=2) { Y-=2u*K; *A = *Y; *(A+1) = -*(Y+1); }
                            Y += 2u*p*K;
                            for (size_t q=0u; q<p; ++q)
                            {
                                A -= 2; Y -= 2u*K;
                                *Y += ar**A - ai**(A+1);
                                *(Y+1) += ar**(A+1) + ai**A;
                            }
                            e *= 1.0f - (ar*ar + ai*ai);
                        }
                    }
                }
            }
        }
        free(A);
    }
	
	return 0;
}


int ac2ar_levdurb_z (double *Y, const double *X, const size_t R, const size_t C, const size_t S, const size_t H, const int iscolmajor, const size_t dim)
{
    if (dim>3u) { fprintf(stderr,"error in ac2ar_levdurb_z: dim must be in [0 3]\n"); return 1; }

    const size_t N = R*C*S*H;
    const size_t Lx = (dim==0u) ? R : (dim==1u) ? C : (dim==2u) ? S : H;

    if (N==0u || Lx<2u) {}
    else if (Lx==2u)
    {
        if (Lx==N)
        {
            const double den = *X**X + *(X+1)**(X+1);
            *Y++ = (*X**(X+2) + *(X+1)**(X+3)) / den;
            *Y++ = (*X**(X+3) - *(X+1)**(X+2)) / den;
        }
        else
        {
            const size_t K = (iscolmajor) ? ((dim==0u) ? 1u : (dim==1u) ? R : (dim==2u) ? R*C : R*C*S) : ((dim==0u) ? C*S*H : (dim==1u) ? S*H : (dim==2u) ? H : 1u);
            const size_t B = (iscolmajor && dim==0u) ? C*S*H : K;
            const size_t V = N/Lx, G = V/B;
            double den;

            if (K==1u && (G==1u || B==1u))
            {
                for (size_t v=0u; v<V; ++v, X+=4)
                {
                    den = *X**X + *(X+1)**(X+1);
                    *Y++ = (*X**(X+2) + *(X+1)**(X+3)) / den;
                    *Y++ = (*X**(X+3) - *(X+1)**(X+2)) / den;
                }
            }
            else
            {
                for (size_t g=0u; g<G; ++g, X+=2u*B)
                {
                    for (size_t b=0u; b<B; ++b, X+=2u, Y+=2)
                    {
                        *Y = *(X+K) / *X;
                        den = *X**X + *(X+1)**(X+1);
                        *Y = (*X**(X+2u*K) + *(X+1)**(X+2u*K+1u)) / den;
                        *(Y+1) = (*X**(X+2u*K+1u) - *(X+1)**(X+2u*K)) / den;
                    }
                }
            }
        }
    }
    else
    {
        const size_t P = Lx - 1u;
        double *A, ar, ai, e, den;
        if (!(A=(double *)malloc(2u*(P-1u)*sizeof(double)))) { fprintf(stderr,"error in ac2ar_levdurb_z: problem with malloc. "); perror("malloc"); return 1; }
        
        if (Lx==N)
        {
            den = *X**X + *(X+1)**(X+1);
            ar = -(*(X+2)**X + *(X+3)**(X+1)) / den;
            ai = -(*(X+3)**X - *(X+1)**(X+2)) / den;
            *Y = -ar; *(Y+1) = -ai;
            e = *X * (1.0 - (ar*ar+ai*ai));
            X += 4;
            for (size_t p=1u; p<P; ++p, X+=2u*p)
            {
                ar = *X; ai = *(X+1);
                for (size_t q=0u; q<p; ++q, Y+=2)
                {
                    X -= 2;
                    ar -= *X**Y - *(X+1)**(Y+1);
                    ai -= *(X+1)**Y + *X**(Y+1);
                }
                ar /= -e; ai /= -e;
                *Y = -ar; *(Y+1) = -ai;
                for (size_t q=0u; q<p; ++q, A+=2) { Y-=2; *A = *Y; *(A+1) = -*(Y+1); }
                Y += 2u*p;
                for (size_t q=0u; q<p; ++q)
                {
                    A -= 2; Y -= 2;
                    *Y += ar**A - ai**(A+1);
                    *(Y+1) += ar**(A+1) + ai**A;
                }
                e *= 1.0 - (ar*ar + ai*ai);
            }
        }
        else
        {
            const size_t K = (iscolmajor) ? ((dim==0u) ? 1u : (dim==1u) ? R : (dim==2u) ? R*C : R*C*S) : ((dim==0u) ? C*S*H : (dim==1u) ? S*H : (dim==2u) ? H : 1u);
            const size_t B = (iscolmajor && dim==0u) ? C*S*H : K;
            const size_t V = N/Lx, G = V/B;

            if (K==1u && (G==1u || B==1u))
            {
                for (size_t v=0u; v<V; ++v, Y+=2u*P)
                {
                    den = *X**X + *(X+1)**(X+1);
                    ar = -(*(X+2)**X + *(X+3)**(X+1)) / den;
                    ai = -(*(X+3)**X - *(X+1)**(X+2)) / den;
                    *Y = -ar; *(Y+1) = -ai;
                    e = *X * (1.0 - (ar*ar+ai*ai));
                    X += 4;
                    for (size_t p=1u; p<P; ++p, X+=2u*p)
                    {
                        ar = *X; ai = *(X+1);
                        for (size_t q=0u; q<p; ++q, Y+=2)
                        {
                            X -= 2;
                            ar -= *X**Y - *(X+1)**(Y+1);
                            ai -= *(X+1)**Y + *X**(Y+1);
                        }
                        ar /= -e; ai /= -e;
                        *Y = -ar; *(Y+1) = -ai;
                        for (size_t q=0u; q<p; ++q, A+=2) { Y-=2; *A = *Y; *(A+1) = -*(Y+1); }
                        Y += 2u*p;
                        for (size_t q=0u; q<p; ++q)
                        {
                            A -= 2; Y -= 2;
                            *Y += ar**A - ai**(A+1);
                            *(Y+1) += ar**(A+1) + ai**A;
                        }
                        e *= 1.0 - (ar*ar + ai*ai);
                    }
                }
            }
            else
            {
                for (size_t g=0u; g<G; ++g, X+=2u*B*P, Y+=2u*B*(P-1u))
                {
                    for (size_t b=0u; b<B; ++b, X-=2u*K*Lx-2u, Y+=2)
                    {
                        den = *X**X + *(X+1)**(X+1);
                        ar = -(*(X+2u*K)**X + *(X+2u*K+1u)**(X+1)) / den;
                        ai = -(*(X+2u*K+1u)**X - *(X+1)**(X+2u*K)) / den;
                        *Y = -ar; *(Y+1) = -ai;
                        e = *X * (1.0 - (ar*ar+ai*ai));
                        X += 4u*K;
                        for (size_t p=1u; p<P; ++p, X+=2u*p*K)
                        {
                            ar = *X; ai = *(X+1);
                            for (size_t q=0u; q<p; ++q, Y+=2u*K)
                            {
                                X -= 2u*K;
                                ar -= *X**Y - *(X+1)**(Y+1);
                                ai -= *(X+1)**Y + *X**(Y+1);
                            }
                            ar /= -e; ai /= -e;
                            *Y = -ar; *(Y+1) = -ai;
                            for (size_t q=0u; q<p; ++q, A+=2) { Y-=2u*K; *A = *Y; *(A+1) = -*(Y+1); }
                            Y += 2u*p*K;
                            for (size_t q=0u; q<p; ++q)
                            {
                                A -= 2; Y -= 2u*K;
                                *Y += ar**A - ai**(A+1);
                                *(Y+1) += ar**(A+1) + ai**A;
                            }
                            e *= 1.0 - (ar*ar + ai*ai);
                        }
                    }
                }
            }
        }
        free(A);
    }
	
	return 0;
}


#ifdef __cplusplus
}
}
#endif
