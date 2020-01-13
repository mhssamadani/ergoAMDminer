#include "clMining.h"
MiningClass::MiningClass(CLWarpper *cll)
{
	cl = cll;

	const  string buildOptions = "-I .";
	program = cl->buildProgramFromFile("MiningKernel.cl", buildOptions);

	int tr;
	tr = 0;

}
MiningClass::~MiningClass()
{

}

void MiningClass::InitMining(
	// context
	ctx_t * ctx,
	// message
	const cl_uint * mes,
	// message length in bytes
	const cl_uint meslen
	)
{
	cl_ulong aux[32];

	//========================================================================//
	//  Initialize context
	//========================================================================//
	memset(ctx->b, 0, BUF_SIZE_8);
	B2B_IV(ctx->h);
	ctx->h[0] ^= 0x01010000 ^ NUM_SIZE_8;
	memset(ctx->t, 0, 16);
	ctx->c = 0;

	//========================================================================//
	//  Hash message
	//========================================================================//
	for (uint_t j = 0; j < meslen; ++j)
	{
		if (ctx->c == BUF_SIZE_8) { HOST_B2B_H(ctx, aux); }

		ctx->b[ctx->c++] = ((const uint8_t *)mes)[j];
	}

	return;
}



void MiningClass::hBlockMining(
	// boundary for puzzle
	cl_mem bound,
	// pool boundary for puzzle
	cl_mem pbound,
	// data: pk || mes || w || padding || x || sk || ctx
	cl_mem data,
	// nonce base
	const cl_ulong base,
	// precalculated hashes
	cl_mem hashes,
	// results
	cl_mem res,
	// indices of valid solutions
	cl_mem valid,
	cl_mem vCount,

	// P results
	cl_mem Pres,
	// indices of P valid solutions
	cl_mem Pvalid,
	cl_mem pCount
	)
{
	cl_kernel kernel = program->getKernel("BlockMining");

	//cout << "\n Running kernel (BlockMining)" << flush;

	cl->checkError(clSetKernelArg(kernel, 0, sizeof(cl_mem), &bound));
	cl->checkError(clSetKernelArg(kernel, 1, sizeof(cl_mem), &pbound));
	cl->checkError(clSetKernelArg(kernel, 2, sizeof(cl_mem), &data));
	cl->checkError(clSetKernelArg(kernel, 3, sizeof(cl_ulong), &base));
	cl->checkError(clSetKernelArg(kernel, 4, sizeof(cl_mem), &hashes));
	cl->checkError(clSetKernelArg(kernel, 5, sizeof(cl_mem), &res));
	cl->checkError(clSetKernelArg(kernel, 6, sizeof(cl_mem), &valid));
	cl->checkError(clSetKernelArg(kernel, 7, sizeof(cl_mem), &vCount));

	cl->checkError(clSetKernelArg(kernel, 8, sizeof(cl_mem), &Pres));
	cl->checkError(clSetKernelArg(kernel, 9, sizeof(cl_mem), &Pvalid));
	cl->checkError(clSetKernelArg(kernel, 10, sizeof(cl_mem), &pCount));


	size_t t1 = ((THREADS_PER_ITER / BLOCK_DIM) + 1) * BLOCK_DIM;
	size_t global_work_size[1] = { t1 };
	size_t local_work_size[1] = { BLOCK_DIM };

	cl_int  err = clEnqueueNDRangeKernel(*cl->queue, kernel, 1, 0, global_work_size, local_work_size, 0, 0, 0);
	cl->checkError(err);
	err = clFinish(*cl->queue);
	cl->checkError(err);

}
