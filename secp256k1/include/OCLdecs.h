#pragma once

#ifndef OCLDECS_H
#define OCLDECS_H

typedef ulong        cl_ulong;
typedef uint        cl_uint;

#include "OCLdefs.h"////problem with relative path -- 


cl_uint fn_mulLow(cl_uint Val1, cl_uint Val2, cl_uint *Result)
{
	cl_ulong tmp = (cl_ulong)Val1 * (cl_ulong)Val2;
	*Result = tmp;
	return tmp >> 32;
}

cl_uint fn_mulHi(cl_uint Val1, cl_uint Val2, cl_uint cv, cl_uint *Result)
{
	cl_ulong tmp = (cl_ulong)Val1 * (cl_ulong)Val2 + (cl_ulong)+cv;
	*Result = tmp >> 32;
	return 0;
}

cl_uint fn_Add(cl_uint Val1, cl_uint Val2, cl_uint cv, cl_uint *Result)
{
	cl_ulong tmp = (cl_ulong)Val1 + (cl_ulong)Val2 + (cl_ulong)cv;
	*Result = tmp;
	return tmp >> 32;
}


cl_uint fn_Sub(cl_uint Val1, cl_uint Val2, cl_uint cv, cl_uint *Result)
{
	cl_uint newcv = 0;
	if (Val1 < Val2 && Val2 - Val1>cv)newcv = 1;
	*Result = Val1 - Val2 - cv;
	return newcv;
}


//cl_uint fn_MadLo(const cl_uint Val1, cl_uint Val2, cl_uint cv, cl_uint *Result)
//{
//	cl_ulong tmp = *Result;
//	/*r[1]*/tmp += (Val1 * Val2) + cv;
//	*Result = tmp;
//	tmp = tmp >> 32;
//	return tmp;
//}
cl_uint fn_MadLo(const cl_uint Val1, cl_uint Val2, cl_uint cv, cl_uint *Result)
{
	cl_ulong tmp = *Result;
	/*r[1]*/tmp += (cl_ulong)(Val1*Val2) + cv;
	*Result = tmp;
	tmp = tmp >> 32;
	return tmp;
}



cl_uint fn_MadHi(cl_uint Val1, cl_uint Val2, cl_uint cv, cl_uint *Result)
{
	cl_ulong tmp = *Result;
	tmp += (((cl_ulong)Val1 * (cl_ulong)Val2) >> 32) + cv;
	*Result = tmp;
	return tmp >> 32;
}


void fn_DEVICE_B2B_H(ctx_t * ctx, cl_ulong *aux)
{
	cl_uint CV = 0;																
		CV = fn_Add((cl_uint)(ctx->t[0]), 128, CV, (cl_uint *)(ctx->t[0]));			
		CV = fn_Add((cl_uint)(ctx->t[1]), 0, CV, (cl_uint *)(ctx->t[1]));              
		CV = fn_Add((cl_uint)(ctx->t[2]), 0, CV, (cl_uint *)(ctx->t[2]));              
		CV = fn_Add((cl_uint)(ctx->t[3]), 0, CV, (cl_uint *)(ctx->t[3]));            
		B2B_INIT(ctx, aux);                                                        
		B2B_FINAL(ctx, aux);                                                       
		((ctx_t *)(ctx))->c = 0;                                                   

}

void fn_DEVICE_B2B_H_LAST(ctx_t * ctx, cl_ulong *aux)
{
	cl_uint CV = 0;                                                               
		CV = fn_Add((cl_uint)(ctx->t[0]), ctx->c, CV, (cl_uint *)(ctx->t[0]));       
		CV = fn_Add((cl_uint)(ctx->t[1]), 0, CV, (cl_uint *)(ctx->t[1]));              
		CV = fn_Add((cl_uint)(ctx->t[2]), 0, CV, (cl_uint *)(ctx->t[2]));             
		CV = fn_Add((cl_uint)(ctx->t[3]), 0, CV, (cl_uint *)(ctx->t[3]));             
		while (((ctx_t *)(ctx))->c < BUF_SIZE_8)                                      
		{                                                                             
			((ctx_t *)(ctx))->b[((ctx_t *)(ctx))->c++] = 0;                           
		}                                                                            
			B2B_INIT(ctx, aux);                                                              
			((cl_ulong *)(aux))[14] = ~((cl_ulong *)(aux))[14];                                 
			B2B_FINAL(ctx, aux);                                                            
}


#define DEVICE_B2B_H(ctx, aux)                                                 \
do                                                                             \
{                                                                              \
cl_uint CV = 0;																\
CV = fn_Add((cl_uint)(ctx->t[0]), 128, CV, (cl_uint *)(ctx->t[0]));			\
CV = fn_Add((cl_uint)(ctx->t[1]), 0, CV, (cl_uint *)(ctx->t[1]));              \
CV = fn_Add((cl_uint)(ctx->t[2]), 0, CV, (cl_uint *)(ctx->t[2]));              \
CV = fn_Add((cl_uint)(ctx->t[3]), 0, CV, (cl_uint *)(ctx->t[3]));            \
    B2B_INIT(ctx, aux);                                                        \
    B2B_FINAL(ctx, aux);                                                       \
    ((ctx_t *)(ctx))->c = 0;                                                   \
}                                                                              \
while (0)

// blake2b last mixing procedure
#define DEVICE_B2B_H_LAST(ctx, aux)                                            \
do                                                                             \
{                                                                              \
cl_uint CV = 0;                                                               \
CV = fn_Add((cl_uint)(ctx->t[0]), ctx->c, CV, (cl_uint *)(ctx->t[0]));       \
CV = fn_Add((cl_uint)(ctx->t[1]), 0, CV, (cl_uint *)(ctx->t[1]));              \
CV = fn_Add((cl_uint)(ctx->t[2]), 0, CV, (cl_uint *)(ctx->t[2]));             \
CV = fn_Add((cl_uint)(ctx->t[3]), 0, CV, (cl_uint *)(ctx->t[3]));             \
while (((ctx_t *)(ctx))->c < BUF_SIZE_8)                                        \
{                                                                              \
	((ctx_t *)(ctx))->b[((ctx_t *)(ctx))->c++] = 0;                                \
}                                                                                 \
                                                                              \
B2B_INIT(ctx, aux);                                                              \
                                                                                    \
((cl_ulong *)(aux))[14] = ~((cl_ulong *)(aux))[14];                                 \
                                                                                  \
B2B_FINAL(ctx, aux);                                                            \
}                                                                           \
while (0)

#endif //