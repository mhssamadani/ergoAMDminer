#include "ergoAutolykos.h"



ergoAutolykos::ergoAutolykos()
{
}


ergoAutolykos::~ergoAutolykos()
{
}


////////////////////////////////////////////////////////////////////////////////
//  Miner thread cycle
////////////////////////////////////////////////////////////////////////////////
void ergoAutolykos::MinerThread(CLWarpper *clw,int deviceId, info_t * info, std::vector<double>* hashrates)
{
	state_t state = STATE_KEYGEN;
	char logstr[1000];

	//========================================================================//
	//  Host memory allocation
	//========================================================================//
	// CURL http request
	json_t request(0, REQ_LEN);

	// hash context
	// (212 + 4) bytes
	ctx_t ctx_h;

	// autolykos variables
	uint8_t bound_h[NUM_SIZE_8];
	uint8_t mes_h[NUM_SIZE_8];
	uint8_t sk_h[NUM_SIZE_8];
	uint8_t pk_h[PK_SIZE_8];
	uint8_t x_h[NUM_SIZE_8];
	uint8_t w_h[PK_SIZE_8];
	uint8_t res_h[NUM_SIZE_8];
	uint8_t nonce[NONCE_SIZE_8];

	char skstr[NUM_SIZE_4];
	char pkstr[PK_SIZE_4 + 1];
	char to[MAX_URL_SIZE];
	int keepPrehash = 0;

	// thread info variables
	uint_t blockId = 0;
	milliseconds start;

	//========================================================================//
	//  Copy from global to thread local data
	//========================================================================//
	info->info_mutex.lock();

	memcpy(sk_h, info->sk, NUM_SIZE_8);
	memcpy(mes_h, info->mes, NUM_SIZE_8);
	memcpy(bound_h, info->bound, NUM_SIZE_8);
	memcpy(pk_h, info->pk, PK_SIZE_8);
	memcpy(pkstr, info->pkstr, (PK_SIZE_4 + 1) * sizeof(char));
	memcpy(skstr, info->skstr, NUM_SIZE_4 * sizeof(char));
	memcpy(to, info->to, MAX_URL_SIZE * sizeof(char));
	// blockId = info->blockId.load();
	keepPrehash = info->keepPrehash;

	info->info_mutex.unlock();

	//========================================================================//
	//  Check GPU memory
	//========================================================================//
	size_t freeMem = 0;
	size_t totalMem = 0;

	// CL_DEVICE_MAX_MEM_ALLOC_SIZE
	cl_ulong max_mem_alloc_size = clw->getMaxAllocSizeMB();
	printf("  CL_DEVICE_MAX_MEM_ALLOC_SIZE:\t\t%u MByte\n", max_mem_alloc_size);

	//// CL_DEVICE_GLOBAL_MEM_SIZE
	cl_ulong mem_size = clw->getGlobalSizeMB();
	printf("  CL_DEVICE_GLOBAL_MEM_SIZE:\t\t%u MByte\n", mem_size);


	freeMem = max_mem_alloc_size * 1024 * 1024;
	if (freeMem < MIN_FREE_MEMORY)
	{
		LOG(ERROR) << "Not enough GPU memory for mining,"
			<< " minimum 2.8 GiB needed";

		return;
	}

	if (keepPrehash && freeMem < MIN_FREE_MEMORY_PREHASH)
	{
		LOG(ERROR) << "Not enough memory for keeping prehashes, "
			<< "setting keepPrehash to false";

		keepPrehash = 0;
	}

	//========================================================================//
//  Device memory allocation
//========================================================================//
	LOG(INFO) << "GPU " << deviceId << " allocating memory";

	// boundary for puzzle
	// (2 * PK_SIZE_8 + 2 + 4 * NUM_SIZE_8 + 212 + 4) bytes // ~0 MiB
	cl_mem bound_d = clw->Createbuffer((NUM_SIZE_8 + DATA_SIZE_8) * sizeof(char), CL_MEM_READ_WRITE);
	cl_uint* hbound_d = (cl_uint*)malloc((NUM_SIZE_8 + DATA_SIZE_8) * sizeof(char));

	// data: pk || mes || w || padding || x || sk || ctx
	//cl_uint * data_d = bound_d + NUM_SIZE_32;
	cl_mem data_d = clw->Createbuffer((2 * PK_SIZE_8 + 2 + 3 * NUM_SIZE_8 + 212 + 4) * sizeof(char), CL_MEM_READ_WRITE);
	cl_uint* hdata_d = (cl_uint*)malloc((2 * PK_SIZE_8 + 2 + 3 * NUM_SIZE_8 + 212 + 4) * sizeof(char));


	// precalculated hashes
	// N_LEN * NUM_SIZE_8 bytes // 2 GiB
	cl_mem hashes_d = clw->Createbuffer((cl_uint)N_LEN * NUM_SIZE_8 * sizeof(char), CL_MEM_READ_WRITE);
	//cl_uint* hhashes_d = (cl_uint*)malloc((cl_uint)N_LEN * NUM_SIZE_8 * sizeof(char));

	// WORKSPACE_SIZE_8 bytes // depends on macros, now ~512 MiB
	// potential solutions of puzzle
	cl_mem res_d = clw->Createbuffer((NUM_SIZE_8 + sizeof(cl_uint)) * sizeof(char), CL_MEM_WRITE_ONLY);
	cl_uint* hres_d = (cl_uint*)malloc((NUM_SIZE_8 + sizeof(cl_uint)) * sizeof(char));

	// indices of unfinalized hashes
	cl_mem indices_d = clw->Createbuffer(sizeof(cl_uint), CL_MEM_READ_WRITE);
	cl_uint* hindices_d = (cl_uint*)malloc(sizeof(cl_uint));
	memset(hindices_d,0, sizeof(cl_uint));
	clw->CopyBuffer(indices_d, hindices_d, sizeof(cl_uint), false);

	// unfinalized hash contexts
	// if keepPrehash == true // N_LEN * 80 bytes // 5 GiB
	cl_mem uctxs_d;
	uctx_t* huctxs_d;
	if (info->keepPrehash)
	{
		uctxs_d = clw->Createbuffer((cl_uint)N_LEN * sizeof(uctx_t), CL_MEM_READ_WRITE);
		huctxs_d = (uctx_t*)malloc((cl_uint)N_LEN * sizeof(uctx_t));
	}


	//========================================================================//
	//  Key-pair transfer form host to device
	//========================================================================//
	// copy public key
	memcpy(hdata_d, (void*)pk_h, PK_SIZE_8);

	// copy secret key
	memcpy(hdata_d + COUPLED_PK_SIZE_32 + 2 * NUM_SIZE_32, (void *)sk_h, NUM_SIZE_8);

	clw->CopyBuffer(data_d, hdata_d, (2 * PK_SIZE_8 + 2 + 3 * NUM_SIZE_8 + 212 + 4) * sizeof(char), false);
	//========================================================================//
	//  Autolykos puzzle cycle
	//========================================================================//
	cl_uint ind = 0;
	cl_ulong base = 0;
	PreHashClass *ph = new PreHashClass(clw);
	MiningClass *min = new MiningClass(clw);

	// set unfinalized hash contexts if necessary
	//if (keepPrehash)
	//{
	//	LOG(INFO) << "Preparing unfinalized hashes on GPU " << deviceId;
	//	ph->hUncompleteInitPrehash(data_d, uctxs_d);
	//}

	int cntCycles = 0;
	int NCycles = 50;
	start = duration_cast<milliseconds>(system_clock::now().time_since_epoch());


	// wait for the very first block to come before starting
	while (info->blockId.load() == 0) {}



	do
	{
		++cntCycles;

		if (!(cntCycles % NCycles))
		{
			milliseconds timediff
				= duration_cast<milliseconds>(
					system_clock::now().time_since_epoch()
					) - start;

			// change avg hashrate in global memory

			(*hashrates)[deviceId] = (double)NONCES_PER_ITER * (double)NCycles
				/ ((double)1000 * timediff.count());


			start = duration_cast<milliseconds>(
				system_clock::now().time_since_epoch()
				);
		}

		// if solution was found by this thread wait for new block to come
		if (state == STATE_KEYGEN)
		{
			while (info->blockId.load() == blockId) {}

			state = STATE_CONTINUE;
		}

		uint_t controlId = info->blockId.load();

		if (blockId != controlId)
		{
			// if info->blockId changed
			// read new message and bound to thread-local mem
			info->info_mutex.lock();

			memcpy(mes_h, info->mes, NUM_SIZE_8);
			memcpy(bound_h, info->bound, NUM_SIZE_8);

			info->info_mutex.unlock();

			LOG(INFO) << "GPU " << deviceId << " read new block data";
			blockId = controlId;

			GenerateKeyPair(x_h, w_h);

			VLOG(1) << "Generated new keypair,"
				<< " copying new data in device memory now";

			// copy boundary
			memcpy(hbound_d, (void*)bound_h, NUM_SIZE_8);

			// copy message
			memcpy((uint8_t*)hdata_d + PK_SIZE_8, mes_h, NUM_SIZE_8);

			// copy one time secret key
			memcpy((cl_uint*)hdata_d + COUPLED_PK_SIZE_32 + NUM_SIZE_32, (void*)x_h, NUM_SIZE_8);

			// copy one time public key
			memcpy((uint8_t*)hdata_d + PK_SIZE_8 + NUM_SIZE_8, (void*)w_h, PK_SIZE_8);


			cl_int ret = clw->CopyBuffer(bound_d, hbound_d, (NUM_SIZE_8 + DATA_SIZE_8) * sizeof(char), false);
			clw->CopyBuffer(data_d, hdata_d, (2 * PK_SIZE_8 + 2 + 3 * NUM_SIZE_8 + 212 + 4) * sizeof(char), false);

			VLOG(1) << "Starting prehashing with new block data";
			ph->Prehash(keepPrehash, data_d, /*uctxs_d ,*/hashes_d, res_d);

			//// calculate unfinalized hash of message
			VLOG(1) << "Starting InitMining";
			min->InitMining(&ctx_h, (cl_uint*)mes_h, NUM_SIZE_8);

			//// copy context
			ret = clw->CopyBuffer(data_d, hdata_d, (2 * PK_SIZE_8 + 2 + 3 * NUM_SIZE_8 + 212 + 4) * sizeof(char), true);
			memcpy(hdata_d + COUPLED_PK_SIZE_32 + 3 * NUM_SIZE_32, &ctx_h, sizeof(ctx_t));
			ret = clw->CopyBuffer(data_d, hdata_d, (2 * PK_SIZE_8 + 2 + 3 * NUM_SIZE_8 + 212 + 4) * sizeof(char), false);

			state = STATE_CONTINUE;

		}


		VLOG(1) << "Starting main BlockMining procedure";

		//// calculate solution candidates
		min->hBlockMining(bound_d, data_d, base, hashes_d, res_d, indices_d);

		VLOG(1) << "Trying to find solution";

		// restart iteration if new block was found
		if (blockId != info->blockId.load())
		{
			continue;
		}


		// try to find solution
		///*
		//LOG(INFO) << "try to find solution";
		cl_int ret = clw->CopyBuffer(indices_d, &ind, sizeof(cl_uint), true);

		//*/

		// solution found
		if (ind)
		{
			LOG(INFO) << "solution found";
			ret = clw->CopyBuffer(res_d, res_h, NUM_SIZE_8, true);

			*((cl_ulong *)nonce) = base + ind - 1;


			PrintPuzzleSolution(nonce, res_h, logstr);
			LOG(INFO) << "GPU " << deviceId
				<< " found and trying to POST a solution:\n" << logstr;

			PostPuzzleSolution(to, pkstr, w_h, nonce, res_h);

			state = STATE_KEYGEN;
			memset(indices_d,0, sizeof(cl_uint));
		}
		//else
		//{
		//	LOG(INFO) << "solution NOTTTT found";
		//}

		base += NONCES_PER_ITER;
	} while (1);


}
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
int ergoAutolykos::startAutolykos(int argc, char ** argv)
{

	//========================================================================//
	//  Setup log
	//========================================================================//
	START_EASYLOGGINGPP(argc, argv);

	el::Loggers::reconfigureAllLoggers(
		el::ConfigurationType::Format, "%datetime %level [%thread] %msg"
	);

	el::Helpers::setThreadName("main thread");

	char logstr[1000];

	// Mnemonic generation mode
	//if (argc > 1)
	//{
	//	if (!strcmp(argv[1], "-G"))
	//	{
	//		if (checkRandomDevice() == EXIT_SUCCESS)
	//		{
	//			std::string mnemonic = BIP39::generate_mnemonic(BIP39::entropy_bits_t::_192).to_string();
	//			LOG(INFO) << "!!!Generated new mnemonic, put it in your config.json file!!!\n" <<
	//				mnemonic <<
	//				"\n!!!Generated new mnemonic, put it in your config.json file!!!";
	//			char skstr[NUM_SIZE_4];
	//			char pkstr[PK_SIZE_4 + 1];
	//			uint8_t sk[NUM_SIZE_8];
	//			uint8_t pk[PK_SIZE_8];
	//			GenerateSecKeyNew(
	//				mnemonic.c_str(), strlen(mnemonic.c_str()), sk,
	//				skstr, ""
	//			);
	//			char logstr_gen[1000];
	//			GeneratePublicKey(skstr, pkstr, pk);
	//			PrintPublicKey(pkstr, logstr_gen);
	//			LOG(INFO) << "Generated public key:\n   " << logstr_gen;

	//			exit(EXIT_SUCCESS);
	//		}
	//		else
	//		{
	//			LOG(ERROR) << "No good randomness source, can't generate mnemonic";
	//			exit(EXIT_SUCCESS);
	//		}
	//	}
	//}



	//========================================================================//
	//  Check GPU availability
	//========================================================================//
	cl_uint deviceCount;
	cl_int errNum = clGetPlatformIDs(0, NULL, &deviceCount);

	int status = EXIT_SUCCESS;

	clw = new CLWarpper*[deviceCount];

	//LOG(INFO) << "Using " << deviceCount << " GPU devices";
	//========================================================================//
	//  Read configuration file
	//========================================================================//
	char confName[14] = "./config.json";
	char * fileName = (argc == 1) ? confName : argv[1];
	char from[MAX_URL_SIZE];
	info_t info;

	info.blockId = 0;
	info.keepPrehash = 0;

	LOG(INFO) << "Using configuration file " << fileName;

	// check access to config file
	if (access(fileName, F_OK) == -1)
	{
		LOG(ERROR) << "Configuration file " << fileName << " is not found";
		return EXIT_FAILURE;
	}

	// read configuration from file
	status = ReadConfig(
		fileName, info.sk, info.skstr, from, info.to, &info.keepPrehash
	);

	if (status == EXIT_FAILURE) { return EXIT_FAILURE; }

	LOG(INFO) << "Block getting URL:\n   " << from;
	LOG(INFO) << "Solution posting URL:\n   " << info.to;

	// generate public key from secret key
	GeneratePublicKey(info.skstr, info.pkstr, info.pk);

	PrintPublicKey(info.pkstr, logstr);
	LOG(INFO) << "Generated public key:\n   " << logstr;

	//========================================================================//
	//  Setup CURL
	//========================================================================//
	// CURL http request
	json_t request(0, REQ_LEN);

	// CURL init
	PERSISTENT_CALL_STATUS(curl_global_init(CURL_GLOBAL_ALL), CURLE_OK);

	//========================================================================//
	//  Fork miner threads
	//========================================================================//
	std::vector<std::thread> miners(deviceCount);
	std::vector<double> hashrates(deviceCount);

	for (int i = 0; i < deviceCount; ++i)
	{
		clw[i] = new CLWarpper(i);
		miners[i] = std::thread(MinerThread, clw[i],i, &info, &hashrates);
		hashrates[i] = 0;
	}

	// get first block
	status = EXIT_FAILURE;
	while (status != EXIT_SUCCESS)
	{
		status = GetLatestBlock(from, &request, &info, 1);
		std::this_thread::sleep_for(std::chrono::milliseconds(800));
		if (status != EXIT_SUCCESS)
		{
			LOG(INFO) << "Waiting for block data to be published by node...";
		}
	}

	//========================================================================//
	//  Main thread get-block cycle
	//========================================================================//
	uint_t curlcnt = 0;
	const uint_t curltimes = 2000;

	ch::milliseconds ms = ch::milliseconds::zero();

	// bomb node with HTTP with 10ms intervals, if new block came
	// signal miners with blockId
	while (1)
	{
		milliseconds start = duration_cast<milliseconds>(
			system_clock::now().time_since_epoch()
			);

		// get latest block
		status = GetLatestBlock(from, &request, &info, 0);

		if (status != EXIT_SUCCESS) { LOG(INFO) << "Getting block error"; }

		ms += duration_cast<milliseconds>(
			system_clock::now().time_since_epoch()
			) - start;

		++curlcnt;

		if (!(curlcnt % curltimes))
		{
			LOG(INFO) << "Average curling time "
				<< ms.count() / (double)curltimes << " ms";
			LOG(INFO) << "Current block candidate: " << request.ptr;
			ms = milliseconds::zero();
			std::stringstream hrBuffer;
			hrBuffer << "Average hashrates: ";
			double totalHr = 0;
			for (int i = 0; i < deviceCount; ++i)
			{
				hrBuffer << "GPU" << i << " " << hashrates[i] << " MH/s ";
				totalHr += hashrates[i];
			}
			hrBuffer << "Total " << totalHr << " MH/s ";
			LOG(INFO) << hrBuffer.str();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(8));
	}

	return EXIT_SUCCESS;
}
