#pragma once

#include "definitions.h"
#include "../../cl_warpper.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

class MiningClass
{

	CLWarpper *cl ;
	cl_kernel kernel;
	std::shared_ptr<CLProgram> program;
public:
	MiningClass(CLWarpper *cll);
	~MiningClass();

	// unfinalized hash of message
	void InitMining(
		// context
		ctx_t * ctx,
		// message
		const cl_uint * mes,
		// message length in bytes
		const cl_uint meslen
		);


	// block mining iteration
	void hBlockMining(
		// boundary for puzzle
		const cl_uint * bound,
		// data: pk || mes || w || padding || x || sk || ctx
		const cl_uint * data,
		// nonce base
		const cl_ulong base,
		// precalculated hashes
		const cl_uint * hashes,
		// results
		cl_uint * res,
		// indices of valid solutions
		cl_uint * valid
		);

};

