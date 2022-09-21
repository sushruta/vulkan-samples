## Code Samples in Vulkan

code samples to demonstrate vulkan usage

## Learning Vulkan

Vulkan is extremely verbose and I wrote over 2000 lines of C++ code to get a model loaded and displayed. At the end of it, it was very satisfying to see the model displayed on the screen.

I followed vulkan-tutorial for learning and very recommend it.

## Downloading Models

There is code to read and load `.obj` models. Also, the code in the repository does not have any models. Please download them using the shell script in the root project folder.

```
bash download-models.sh
```

## Building the excutable

All the code is in multiple header files and one cpp file. I have deliberately used only Makefile and targeted MacOS ecosystem. I do not use Xcode and therefore I needed a build system that didn't depend on Xcode. You should be able to make small modifications to it and be able to run it on linux as well. We may have to go the route of cmake if we want to true cross platform compatibility.

```
make clean; make build
```

## Running the code

Should be as simple as `./a.out` but please check the code if additional args are required
