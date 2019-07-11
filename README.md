# ergoAMDminer
OpenCL miner for [ergoplatform.com](https://github.com/ergoplatform)

You can find CUDA miner at:  [CUDA miner](https://github.com/ergoplatform/Autolykos-GPU-miner/tree/master/secp256k1)

Please note that this is the first release of the OpenCL miner and needs some optimization for improved hashrate.

Update: hashrate is 19Mh/s for rx570-8g

The code is based on opencl 1.2 and tested on RX570.

These files must be in the same folder as Executable file:
  * Miningkernel.cl
  * Prehashkernel.cl
  * Ocldecs.h
  * Ocldefs.h
  * Libcurl.dll( in windows)
  * Config.json
  
 Change the config.json file with your seed and node's address. For now only use keepPrehash:false.
 
 # Requirements (Linux)
 
  1- Installing AMDGPU PRO driver for Ubuntu 
  
  2 - Opencl headers
  
    $ sudo apt-get install opencl-headers
  
  3 - libcurl library: to install run
  
    $ apt install libcurl4-openssl-dev
  
  4- OpenSSL 1.0.2 library: to install run
  
    $ apt install libssl-dev
  
 # Requirements (Windows)
 
 1- AMD graphic card with driver installed
 
 2 - Download and Install [AMD APK](amd-dev.wpengine.netdna-cdn.com/app-sdk/installers/APPSDKInstaller/3.0.130.135-GA/full/AMD-APP-SDKInstaller-v3.0.130.135-GA-windows-F-x64.exe)
 
 3 - Build libcurl from sources with Visual Studio [toolchain instruction](https://medium.com/@chuy.max/compile-libcurl-on-windows-with-visual-studio-2017-x64-and-ssl-winssl-cff41ac7971d) 
 
 4 - Download OpenSSL 1.0.2 [installer from slproweb.com](https://slproweb.com/download/Win64OpenSSL-1_0_2r.exe)
 
 # Build (Linux)
 
 
1- in ErgoOpenCL.cpp file comment 

*#define _TEST_* 

for build miner or uncomment for build tester

2- Change directory to Ubuntu
 
3- Run make 
 
 
# Build (Windows)
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

