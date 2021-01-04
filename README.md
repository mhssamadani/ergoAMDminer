# This miner will be deprecated due to incomming fork of ergoplatform
new miners for Autolykos v2 can be found here:
[AMD Miner](https://github.com/mhssamadani/Autolykos2_AMD_Miner), [Nvidia Miner](https://github.com/mhssamadani/Autolykos2_NV_Miner)
# ergoAMDminer
OpenCL miner for [ergoplatform.com](https://github.com/ergoplatform)

You can find CUDA miner at:  [CUDA miner](https://github.com/ergoplatform/Autolykos-GPU-miner/tree/master/secp256k1)
# Very Important Notice
*Due to ERGO's specification (unlike many other coins), it is __NOT SAFE__ to mine using any closed source miner; such a miner could steal your key easily.*
<blockquote class="twitter-tweet"><p lang="en" dir="ltr">Please be aware of closed-source miners like the following, such a miner could steal your key easily: <a href="https://t.co/Y93wqb2IkA">https://t.co/Y93wqb2IkA</a></p>&mdash; Alex Chepurnoy (@chepurnoy) <a href="https://twitter.com/chepurnoy/status/1155901160915701760?ref_src=twsrc%5Etfw">July 29, 2019</a></blockquote>


# Hashrate
The code is based on opencl 1.2 and tested on RX570, RX580, and RX VEGA56.

Reported Hashrates:
* rx570-8g: 22-24Mh/s 
* rx580-8g: 23-25Mh/s 
* rx Vega56: 24-26Mh/s. 


# Configuration
These files must be in the same folder as Executable file:
  * Miningkernel.cl
  * Prehashkernel.cl
  * Ocldecs.h
  * Ocldefs.h
  * Libcurl.dll( in windows)::: There is a libcurl.dll file in the win64 folder, use it or better replace it with your own built file
  * Config.json
  
 Change the config.json file with __your seed__ and node's address. 
 
 ~~For now only use keepPrehash:false.~~
 
 Set __keepPrehash: true__ in config.json file.
 
 # Requirements (Linux)
 
  1- Installing AMDGPU PRO driver for Ubuntu.
  
  Download AMDGPU-PRO package for your Linux distribution from amd.com, upack the downloaded file and run: 
  
     $ amdgpu-pro-install --opencl=legacy,pal --headless
  
  2 - Opencl headers
  
    $ sudo apt-get install opencl-headers
  
  3 - libcurl library: to install run
  
    $ apt install libcurl4-openssl-dev
  
  4- OpenSSL 1.0.2 library: to install run
  
    $ apt install libssl-dev
  
 # Requirements (Windows)
 
 1- AMD graphic card with driver installed
 
 2 - Download and Install [AMD APP SDK](https://www.softpedia.com/get/Programming/SDK-DDK/ATI-Stream-SDK.shtml)
 
 3 - Build libcurl from sources with Visual Studio [toolchain instruction](https://medium.com/@chuy.max/compile-libcurl-on-windows-with-visual-studio-2017-x64-and-ssl-winssl-cff41ac7971d) 
 
 4 - Download OpenSSL 1.0.2 [installer from slproweb.com](https://slproweb.com/download/Win64OpenSSL-1_0_2t.exe)
 
 5 - Download and install [Visual C++ redistributable](https://aka.ms/vs/16/release/vc_redist.x64.exe)
 
 # Build (Linux)
 
 
1- in ErgoOpenCL.cpp file comment 

*#define _TEST_* 

for build miner or uncomment for build tester

2- Change directory to Ubuntu
 
3- Run make 

4- If make completed successfully there will appear an executable **ergoAMDminer/Ubuntu/ErgoOpenCL** that can start with **./ErgoOpenCL** 
 
 
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

for build miner or uncomment for build tester.

# Http info
Miner has a HTTP info page located at `http://miningnode:36207` in json format which shows status of gpus (hashrate, fan speed, temp, ...)

# Hiveos
### Very Important Notice
<blockquote class="twitter-tweet"><p lang="en" dir="ltr">Hiveos is the ultimate mining platform that allows users to setup, mine and control processes more efficiently and hustle-free across thousands of rigs all in one place. <a href="https://hiveos.farm/">Hiveos</a></blockquote>

*Due to ERGO's whitepaper (unlike many other coins), mining process utilizes __PRIVATE KEYS__, therefor you should change configuration file (/hive/miners/custom/ergoopencl/ergoopencl.conf) with __your seed and mnemonicpass__  after installing Hiveos miner .*

This means that Hiveos could have access to your seed, so use Hiveos at __your own risk__.

![](https://github.com/mhssamadani/ergoAMDminer/blob/master/Hiveos/Hiveos-Flight_Sheet.png)

Installation URL: <a href="https://raw.github.com/mhssamadani/ergoAMDminer/master/Hiveos/ergoopencl-2.0.tar.gz">link</a>

Replace IP:PORT with your node's IP and PORT

# Donations and Support

Note that the miner is free to use and we do not charge any fee from what you mine.
To support all the work we're doing, we welcome donations from ERGO miners!

ERGO: 9fFUw6DqRuyFCv13nQyoDuDz4TiR4GvVvWRcSvqzs39eBVcb5S1
