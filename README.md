# MapleLeaf

a real-time render.


## Prerequisties
+ Visual Studio Code
+ Xmake 2.8.0 or newer version
+ Vulkan SDK 1.3 or higher version

## Build MapleLeaf
``` shell
git clone https://github.com/CuteLoong/MapleLeaf.git
xmake m -f release
xmake

xmake run
```
  
For more details about xmake, see: [xmake](https://xmake.io/#/zh-cn/getting_started)
## To do:

1. renderpass resource defined by itself, managed by frame buffer, but can be see global or set to global.

2. frame buffer create resource accroding component count.

3. preFrame function.

4. renderpass's ui component

5. separate sampler and image2D


## Renference

+ Acid (https://github.com/EQMG/Acid/tree/1bd67dc8c2c2ebee2ca95f0cabb40b926f747fcf)

+ Falcor (https://github.com/NVIDIAGameWorks/Falcor)