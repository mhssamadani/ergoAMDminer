# ergoAMDminer
AMD miner for [ergoplatform.com](https://github.com/ergoplatform)

You can find CUDA miner at:  [CUDA miner](https://github.com/ergoplatform/Autolykos-GPU-miner/tree/master/secp256k1)

Please note that this is the first release of the AMD miner and needs some optimization for improved hashrate.

The code is based on opencl 2.0 so it cannot be used for nvidia gpus.

These files must be in the same folder as .exe file:
  * Miningkernel.dat
  * Prehashkernel.dat
  * Ocldecs.h
  * Ocldefs.h
  * Libcurl.dll
  * Config.json
  
 Change the config.json file with your seed and node's address. For now only use keepPrehash:false.
 
