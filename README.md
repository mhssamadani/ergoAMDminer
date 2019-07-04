# ergoAMDminer
AMD miner for [ergoplatform.com](https://github.com/ergoplatform)

You can find CUDA miner at:  [CUDA miner](https://github.com/ergoplatform/Autolykos-GPU-miner/tree/master/secp256k1)

Please note that this is the first release of the AMD miner and needs some optimization for improved hashrate.

The code is based on opencl 2.0 so it cannot be used for nvidia gpus.

These files must be in the same folder as .exe file:
  * Miningkernel.cl
  * Prehashkernel.cl
  * Ocldecs.h
  * Ocldefs.h
  * Libcurl.dll
  * Config.json
  
 Change the config.json file with your seed and node's address. For now only use keepPrehash:false.
 
 # Requirements
 
 1- AMD graphic card with driver installed
 
 2 - Download and Install [AMD APK](amd-dev.wpengine.netdna-cdn.com/app-sdk/installers/APPSDKInstaller/3.0.130.135-GA/full/AMD-APP-SDKInstaller-v3.0.130.135-GA-windows-F-x64.exe)
 
 3 - Build libcurl from sources with Visual Studio [toolchain instruction](https://medium.com/@chuy.max/compile-libcurl-on-windows-with-visual-studio-2017-x64-and-ssl-winssl-cff41ac7971d) 
 
 4 - Download OpenSSL 1.0.2 [installer from slproweb.com](https://slproweb.com/download/Win64OpenSSL-1_0_2r.exe)
 
# for build
using visual studio

add OpenCL , LibCurl , OpenSSl libreries

Include Directories:

![](https://github.com/amin63moradi/ergoAMDminer/blob/master/img/includeDir.png)


Additional Include Directories:

![](https://github.com/amin63moradi/ergoAMDminer/blob/master/img/AddInc.png)



Additional Library Directories:


![](https://github.com/amin63moradi/ergoAMDminer/blob/master/img/AddLib.png)



Additional Dependencies:


![](https://github.com/amin63moradi/ergoAMDminer/blob/master/img/AddDep.png)


in ErgoOpenCL.cpp file comment 

*#define _TEST_* 

for build miner or uncomment for build tester
