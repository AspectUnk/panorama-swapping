# Panorama-Swapping
Swapping original CS:GO panorama files for custom ones

## Information
The main idea was to implement the possibility of swapping files in the archive csgo\panorama\code.pbin, which is checked when you run the game with a public key.

The project has a dependency on the [MinHook library](https://github.com/TsudaKageyu/minhook)

## Using
- You can get all the files you need to change by opening csgo\panorama\code.pbin as an archive
- The library must be loaded before the ParseFromBuffer function is called, which is called when you start the game. The best way to do this is to use the [Xenos Injector](https://github.com/DarthTon/Xenos) in Manual Launch mode
- The files for swapping are specified in the [files variable at the beginning of the dllmain.cpp file](https://github.com/AspectUnk/panorama-swapping/blob/ae3d6b9ba537e2f8e57da904205cf3496cc4eaa1/src/panorama_swapping/dllmain.cpp#L11)
- Perform injections only in -insecure mode

## Screenshot
![alt text](https://github.com/AspectUnk/panorama-swapping/blob/main/screenshot/z8jeWhL3mq.png?raw=true)

## Credits
[@zephire1](https://github.com/zephire1) - Idea Generator
