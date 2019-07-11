#include "clPreHash.h"
PreHashClass::PreHashClass(CLWarpper *cll)
{
	cl = cll;

	const  string buildOptions = "-O0 -w -I .";
	program = cl->buildProgramFromFile("PreHashKernel.cl", buildOptions);

}
PreHashClass::~PreHashClass()
{

}

// first iteration of hashes precalculation
void PreHashClass::hInitPrehash(
	// data: pk || mes || w || padding || x || sk
	const cl_uint * data,
	// hashes
	cl_uint * hashes,
	// indices of invalid range hashes
	cl_uint * invalid
	) 
{
	cl_kernel kernel = program->getKernel("InitPrehash");
}

// uncompleted first iteration of hashes precalculation
void PreHashClass::hUncompleteInitPrehash(
	// data: pk
	const cl_uint * data,
	// unfinalized hash contexts
	uctx_t * uctxs
	)
{
	cl_kernel kernel = program->getKernel("UncompleteInitPrehash");

}

// complete first iteration of hashes precalculation
void PreHashClass::hCompleteInitPrehash(
	// data: pk || mes || w || padding || x || sk
	const cl_uint * data,
	// unfinalized hash contexts
	const uctx_t * uctxs,
	// hashes
	cl_uint * hashes,
	// indices of invalid range hashes
	cl_uint * invalid
	)
{
	cl_kernel kernel = program->getKernel("CompleteInitPrehash");

}

// unfinalized hashes update
void PreHashClass::hUpdatePrehash(
	// hashes
	cl_uint * hashes,
	// indices of invalid range hashes
	cl_uint * invalid,
	// length of invalid
	const cl_uint len
	)
{
	cl_kernel kernel = program->getKernel("UpdatePrehash");

}

// hashes modulo Q 
void PreHashClass::hFinalPrehash(
	// hashes
	cl_uint * hashes
	)
{
	cl_kernel kernel = program->getKernel("FinalPrehash");

}

// hashes by secret key multiplication modulo Q 
void PreHashClass::hFinalPrehashMultSecKey(
	// data: pk || mes || w || padding || x || sk
	cl_mem data,
	// hashes
	cl_mem hashes
	)
{

	cl_kernel kernel = program->getKernel("FinalPrehashMultSecKey");


	cl->checkError(clSetKernelArg(kernel, 0, sizeof(cl_mem), &data));
	cl->checkError(clSetKernelArg(kernel, 1, sizeof(cl_mem), &hashes));

	// Run the kernel.
	cout << "\n Running kernel (FinalPrehashMultSecKey) " << flush;

	size_t t1 = ((N_LEN / BLOCK_DIM) + 1) * BLOCK_DIM;
	size_t global_work_size[1] = { t1 };
	size_t local_work_size[1] = { BLOCK_DIM };

	cl_int  err = clEnqueueNDRangeKernel(*cl->queue, kernel, 1, 0, global_work_size, local_work_size, 0, 0, 0);
	cl->checkError(err);
	err = clFinish(*cl->queue);
	cl->checkError(err);




}

int PreHashClass::Prehash(
	const int keep,
	// data: pk || mes || w || padding || x || sk
	cl_mem  data,
	// unfinalized hashes contexts
	/*uctx_t * uctxs,*/
	// hashes
	cl_mem   hashes,
	// indices of invalid range hashes
	cl_mem   invalid
	)
{

	cl_uint len = N_LEN;


	if (keep)
	{
		cl_kernel kernel = program->getKernel("CompleteInitPrehash");

		cl->checkError(clSetKernelArgSVMPointer(kernel, 0, data));
		cl->checkError(clSetKernelArgSVMPointer(kernel, 1, NULL/*uctxs*/));
		cl->checkError(clSetKernelArgSVMPointer(kernel, 2, hashes));
		cl->checkError(clSetKernelArgSVMPointer(kernel, 3, invalid));



		size_t t1 = ((N_LEN / BLOCK_DIM) + 1) * BLOCK_DIM;
		size_t global_work_size[1] = { t1 };
		size_t local_work_size[1] = { BLOCK_DIM };

		// Run the kernel.
		cout << "\n Running kernel..., CompleteInitPrehash" << flush;

		cl_int  err = clEnqueueNDRangeKernel(*cl->queue, kernel, 1, 0, global_work_size, local_work_size, 0, 0, 0);
		cl->checkError(err);
		err = clFinish(*cl->queue);
		cl->checkError(err);

	}
	else
	{
		cl_kernel kernel = program->getKernel("InitPrehash");

		cl->checkError(clSetKernelArg(kernel, 0, sizeof(cl_mem), &data));
		cl->checkError(clSetKernelArg(kernel, 1, sizeof(cl_mem), &hashes));
		cl->checkError(clSetKernelArg(kernel, 2, sizeof(cl_mem), &invalid));



		size_t t1 = ((N_LEN / BLOCK_DIM) + 1) * BLOCK_DIM;
		size_t global_work_size[1] = { t1 };

		size_t local_work_size[1] = { BLOCK_DIM };

		// Run the kernel.
		cout << "\n Running kernel (InitPrehash)" << flush;

		cl_int  err = clEnqueueNDRangeKernel(*cl->queue, kernel, 1, 0, global_work_size, local_work_size, 0, 0, 0);
		cl->checkError(err);
		err = clFinish(*cl->queue);
		cl->checkError(err);

	}

	// multiply by secret key moq Q
	hFinalPrehashMultSecKey (data, hashes);

	return EXIT_SUCCESS;
}
