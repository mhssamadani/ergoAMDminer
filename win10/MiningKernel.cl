
__kernel void test()
{
}
#include "OCLdecs.h"////problem with relative path
__kernel void BlockMining(
	// boundary for puzzle
	global const cl_uint* bound,
	// data: pk || mes || w || padding || x || sk || ctx
	global const cl_uint* data,
	// nonce base
	const cl_ulong base,
	// precalculated hashes
	global const cl_uint* hashes,
	// results
	global cl_uint* res,
	// indices of valid solutions
	global cl_uint* valid)
{
	cl_uint tid = get_local_id(0);

	// shared memory

	__local cl_uint sdata[ROUND_NC_SIZE_32];

#pragma unroll
	for (int i = 0; i < NC_SIZE_32_BLOCK; ++i)
	{
		sdata[NC_SIZE_32_BLOCK * tid + i] = data[NC_SIZE_32_BLOCK * tid + NUM_SIZE_32 * 2 + COUPLED_PK_SIZE_32 + i];
	}
	barrier(CLK_LOCAL_MEM_FENCE);

	// NUM_SIZE_8 bytes
	__private cl_uint sk[8];// = (__local cl_uint*)sdata;
	sk[0] = (cl_uint*)sdata[0];
	sk[1] = (cl_uint*)sdata[1];
	sk[2] = (cl_uint*)sdata[2];
	sk[3] = (cl_uint*)sdata[3];
	sk[4] = (cl_uint*)sdata[4];
	sk[5] = (cl_uint*)sdata[5];
	sk[6] = (cl_uint*)sdata[6];
	sk[7] = (cl_uint*)sdata[7];
	// local memory
	// 472 bytes
	cl_uint ldata[/*118 aminm */ 200];

	// 256 bytes
	cl_ulong* aux = (cl_ulong*)ldata;
	// (4 * K_LEN) bytes
	cl_uint* ind = ldata;
	// (NUM_SIZE_8 + 4) bytes
	cl_uint* r = ind + K_LEN;
	// (212 + 4) bytes
	ctx_t* ctx = (ctx_t*)(ldata + 64);

	cl_uint CV;

#pragma unroll
	for (int t = 0; t < NONCES_PER_THREAD; ++t)
	{
		*ctx = *((ctx_t __local*)(sdata + NUM_SIZE_32));

		//tid = threadIdx.x + blockDim.x * blockIdx.x + t * gridDim.x * blockDim.x;

		tid = get_global_id(0) + t * get_global_size(0);

		if (tid < NONCES_PER_ITER)
		{
			cl_uint j;
			cl_uint non[NONCE_SIZE_32];

			fn_Add(((cl_uint*)& base)[0], tid, 0, non[0], CV);

			//asm volatile ("add.cc.u32 %0, %1, %2;":"=r"(non[0]): "r"(((cl_uint *)&base)[0]), "r"(tid));

			non[1] = 0;
			fn_Add(((cl_uint*)& base)[1], 0, CV, non[1], CV);

			//asm volatile ("addc.u32 %0, %1, 0;": "=r"(non[1]): "r"(((cl_uint *)&base)[1]));

			//================================================================//
			//  Hash nonce
			//================================================================//

#pragma unroll
			for (j = 0; ctx->c < BUF_SIZE_8 && j < NONCE_SIZE_8; ++j)
			{
				ctx->b[ctx->c++] = ((uint8_t *)non)[NONCE_SIZE_8 - j - 1];
			}

#pragma unroll
			for (; j < NONCE_SIZE_8;)
			{
				HOST_B2B_H(ctx, aux);

#pragma unroll
				for (; ctx->c < BUF_SIZE_8 && j < NONCE_SIZE_8; ++j)
				{
					ctx->b[ctx->c++] = ((uint8_t*)non)[NONCE_SIZE_8 - j - 1];
				}
			}

			//================================================================//
			//  Finalize hashes
			//================================================================//

			HOST_B2B_H_LAST(ctx, aux);

#pragma unroll
			for (j = 0; j < NUM_SIZE_8; ++j)
			{
				((uint8_t*)r)[(j & 0xFFFFFFFC) + (3 - (j & 3))] = (ctx->h[j >> 3] >> ((j & 7) << 3)) & 0xFF;
			}

			//================================================================//
			//  Generate indices
			//================================================================//

#pragma unroll
			for (int i = 1; i < INDEX_SIZE_8; ++i)
			{
				((uint8_t*)r)[NUM_SIZE_8 + i] = ((uint8_t*)r)[i];
			}

#pragma unroll
			for (int k = 0; k < K_LEN; k += INDEX_SIZE_8)
			{
				ind[k] = r[k >> 2] & N_MASK;

#pragma unroll
				for (int i = 1; i < INDEX_SIZE_8; ++i)
				{
					ind[k + i] = ((r[k >> 2] << (i << 3)) | (r[(k >> 2) + 1] >> (32 - (i << 3)))) & N_MASK;
				}
			}

			//================================================================//
			//  Calculate result
			//================================================================//
			// first addition of hashes -> r
			fn_Add(hashes[ind[0] << 3], hashes[ind[1] << 3], 0, r[0], CV);
			// asm volatile ("add.cc.u32 %0, %1, %2;":"=r"(r[0]): "r"(hashes[ind[0] << 3]), "r"(hashes[ind[1] << 3]));

#pragma unroll
			for (int i = 1; i < 8; ++i)
			{
				fn_Add(hashes[(ind[0] << 3) + i], hashes[(ind[1] << 3) + i], CV, r[i], CV);
				//asm volatile ("addc.cc.u32 %0, %1, %2;":"=r"(r[i]):"r"(hashes[(ind[0] << 3) + i]),"r"(hashes[(ind[1] << 3) + i]));
			}
			r[8] = 0;
			fn_Add(r[8], 0, CV, r[8], CV);
			// asm volatile ("addc.u32 %0, 0, 0;": "=r"(r[8]));

			// remaining additions

#pragma unroll
			for (int k = 2; k < K_LEN; ++k)
			{
				fn_Add(r[0], hashes[ind[k] << 3], 0, r[0], CV);
				//asm volatile ("add.cc.u32 %0, %0, %1;":"+r"(r[0]): "r"(hashes[ind[k] << 3]));

#pragma unroll
				for (int i = 1; i < 8; ++i)
				{
					fn_Add(r[i], hashes[(ind[k] << 3) + i], CV, r[i], CV);
					//asm volatile ("addc.cc.u32 %0, %0, %1;":"+r"(r[i]): "r"(hashes[(ind[k] << 3) + i]));
				}
				fn_Add(r[8], 0, CV, r[8], CV);
				//asm volatile ("addc.u32 %0, %0, 0;": "+r"(r[8]));
			}

			// subtraction of secret key
			fn_Sub(r[0], sk[0], 0, r[0], CV);
			//asm volatile ("sub.cc.u32 %0, %0, %1;": "+r"(r[0]): "r"(sk[0]));

#pragma unroll
			for (int i = 1; i < 8; ++i)
			{
				fn_Sub(r[i], sk[i], CV, r[i], CV);
				//asm volatile("subc.cc.u32 %0, %0, %1;": "+r"(r[i]): "r"(sk[i]));
			}

			fn_Sub(r[8], 0, CV, r[8], CV);
			//asm volatile ("subc.u32 %0, %0, 0;": "+r"(r[8]));

			//================================================================//
			//  Result mod Q
			//================================================================//
			// 20 bytes
			cl_uint* med = ind;
			// 4 bytes
			cl_uint* d = ind + 5;
			cl_uint* carry = d;

			d[0] = r[8];

			//================================================================//
			fn_mulLow(d[0], (cl_uint)q0_s, med[0],CV);
			//asm volatile ("mul.lo.u32 %0, %1, " q0_s ";": "=r"(med[0]) : "r"(*d));
			fn_mulHi(d[0], (cl_uint)q0_s, CV, med[1],CV);
			//asm volatile ("mul.hi.u32 %0, %1, " q0_s ";": "=r"(med[1]) : "r"(*d));
			fn_mulLow(d[0], (cl_uint)q2_s, med[2], CV);
			//asm volatile ("mul.lo.u32 %0, %1, " q2_s ";": "=r"(med[2]) : "r"(*d));
			fn_mulHi(d[0], (cl_uint)q2_s, CV, med[3], CV);
			//asm volatile ("mul.hi.u32 %0, %1, " q2_s ";": "=r"(med[3]) : "r"(*d));
			fn_MadLo(d[0], (cl_uint)q1_s, 0, med[1], CV);
			//asm volatile ("mad.lo.cc.u32 %0, %1, " q1_s ", %0;": "+r"(med[1]) : "r"(*d));
			fn_MadHi(d[0], (cl_uint)q1_s, CV, med[2], CV);
			//asm volatile ("madc.hi.cc.u32 %0, %1, " q1_s ", %0;": "+r"(med[2]) : "r"(*d));
			fn_MadLo(d[0], (cl_uint)q3_s, CV, med[3], CV);
			//asm volatile ("madc.lo.cc.u32 %0, %1, " q3_s ", %0;": "+r"(med[3]) : "r"(*d));
			med[4] = 0;
			fn_MadHi(d[0], (cl_uint)q3_s, CV, med[4], CV);
			//asm volatile ("madc.hi.u32 %0, %1, " q3_s ", 0;": "=r"(med[4]) : "r"(*d));
			CV = 0;
			//================================================================//
			fn_Sub(r[0], med[0], CV, r[0], CV);
			//asm volatile ("sub.cc.u32 %0, %0, %1;": "+r"(r[0]): "r"(med[0]));

#pragma unroll
			for (int i = 1; i < 5; ++i)
			{
				fn_Sub(r[i], med[i], CV, r[i],  CV);
				//asm volatile ("subc.cc.u32 %0, %0, %1;": "+r"(r[i]): "r"(med[i]));
			}

#pragma unroll
			for (int i = 5; i < 7; ++i)
			{
				fn_Sub(r[i], 0, CV, r[i], CV);
				//asm volatile ("subc.cc.u32 %0, %0, 0;": "+r"(r[i]));
			}
			 fn_Sub(r[7], 0, CV, r[7], CV);
			//asm volatile ("subc.u32 %0, %0, 0;": "+r"(r[7]));

			//================================================================//
			d[1] = d[0] >> 31;
			d[0] <<= 1;

			fn_Add(r[4], d[0], 0, r[4], CV);
			//asm volatile ("add.cc.u32 %0, %0, %1;": "+r"(r[4]): "r"(d[0]));

			fn_Add(r[5], d[1], CV, r[5], CV);
			//asm volatile ("addc.cc.u32 %0, %0, %1;": "+r"(r[5]): "r"(d[1]));

			fn_Add(r[6], 0, CV, r[6], CV);
			//asm volatile ("addc.cc.u32 %0, %0, 0;": "+r"(r[6]));

			fn_Add(r[7], 0, CV, r[7], CV);
			//asm volatile ("addc.u32 %0, %0, 0;": "+r"(r[7]));

			//================================================================//
			CV = 0;
			fn_Sub(r[0], (cl_uint)q0_s, CV, r[0], CV);
			//asm volatile ("sub.cc.u32 %0, %0, " q0_s ";": "+r"(r[0]));

			fn_Sub(r[1], (cl_uint)q1_s, CV, r[1], CV);
			//asm volatile ("subc.cc.u32 %0, %0, " q1_s ";": "+r"(r[1]));

			fn_Sub(r[2], (cl_uint)q2_s, CV, r[2], CV);
			//asm volatile ("subc.cc.u32 %0, %0, " q2_s ";": "+r"(r[2]));

			fn_Sub(r[3], (cl_uint)q3_s, CV, r[3], CV);
			//asm volatile ("subc.cc.u32 %0, %0, " q3_s ";": "+r"(r[3]));

			fn_Sub(r[4], (cl_uint)q4_s, CV, r[4], CV);
			//asm volatile ("subc.cc.u32 %0, %0, " q4_s ";": "+r"(r[4]));

#pragma unroll
			for (int i = 5; i < 8; ++i)
			{
				fn_Sub(r[i], (cl_uint)qhi_s, CV, r[i], CV);
				//asm volatile ("subc.cc.u32 %0, %0, " qhi_s ";": "+r"(r[i]));
			}

			carry[0] = 0;
			fn_Sub(0, 0, CV, carry[0], CV);
			//asm volatile ("subc.u32 %0, 0, 0;": "=r"(*carry));

			*carry = 0 - *carry;

			//================================================================//
			CV = 0;
			fn_MadLo(carry[0], (cl_uint)q0_s, CV, r[0], CV);
			//asm volatile ("mad.lo.cc.u32 %0, %1, " q0_s ", %0;": "+r"(r[0]) : "r"(*carry));

			fn_MadLo(carry[0], (cl_uint)q1_s, CV, r[1], CV);
			//asm volatile ("madc.lo.cc.u32 %0, %1, " q1_s ", %0;": "+r"(r[1]) : "r"(*carry));

			fn_MadLo(carry[0], (cl_uint)q2_s, CV, r[2], CV);
			//asm volatile ("madc.lo.cc.u32 %0, %1, " q2_s ", %0;": "+r"(r[2]) : "r"(*carry));

			fn_MadLo(carry[0], (cl_uint)q3_s, CV, r[3], CV);
			//asm volatile ("madc.lo.cc.u32 %0, %1, " q3_s ", %0;": "+r"(r[3]) : "r"(*carry));

			fn_MadLo(carry[0], (cl_uint)q4_s, CV, r[4], CV);
			//asm volatile ("madc.lo.cc.u32 %0, %1, " q4_s ", %0;": "+r"(r[4]) : "r"(*carry));

#pragma unroll
			for (int i = 5; i < 7; ++i)
			{
				fn_MadLo(carry[0], (cl_uint)qhi_s, CV, r[i], CV);
				//asm volatile ("madc.lo.cc.u32 %0, %1, " qhi_s ", %0;":"+r"(r[i]) : "r"(*carry));
			}

			fn_MadLo(carry[0], (cl_uint)qhi_s, CV, r[7], CV);
			//asm volatile ("madc.lo.u32 %0, %1, " qhi_s ", %0;": "+r"(r[7]) : "r"(*carry));

			//================================================================//
			//  Dump result to global memory -- LITTLE ENDIAN
			//================================================================//
			j = ((cl_ulong*)r)[3] < ((cl_ulong global*)bound)[3] || ((cl_ulong*)r)[3] == ((cl_ulong global*)bound)[3] && (((cl_ulong*)r)[2] < ((cl_ulong global*)bound)[2] || ((cl_ulong*)r)[2] == ((cl_ulong global*)bound)[2] && (((cl_ulong*)r)[1] < ((cl_ulong global*)bound)[1] || ((cl_ulong*)r)[1] == ((cl_ulong global*)bound)[1] && ((cl_ulong*)r)[0] < ((cl_ulong global*)bound)[0]));

			

			if (j)
			{

				//printf("\n %d", j);
				valid[0] = tid + 1;
#pragma unroll
				for (int i = 0; i < NUM_SIZE_32; ++i)
				{
					res[i] = r[i];
				}

			}
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}
	return;
}
