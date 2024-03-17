# Banana Project

This is a mini-project for the software engineering course of the [FHGR B.Sc. Mobile Robotics](https://fhgr.ch/mr),
implemented by [Dominic Eicher](https://github.com/Nic822) and [Ralph Ursprung](https://github.com/rursprung).

This project analyses pictures of bananas to try and identify some information about them (e.g. their ripeness).
Note that we will currently only support [Cavendish Bananas](https://en.wikipedia.org/wiki/Cavendish_banana)
(_the_ standard banana) as others may exhibit other properties (e.g. [cooking bananas](https://en.wikipedia.org/wiki/Cooking_banana)
are green even when ripe).  

## Project Structure

The project consists of a library implementing the actual functionality and two applications, one feeding the library
with live pictures from an attached camera and one feeding it static images (mainly for manual testing).

## Building

To build this project you will need:

* A modern C++ compiler supporting C++23
* [CMake](https://cmake.org/) incl. CTest - this might well come included with your favourite IDE
* [vcpkg](https://vcpkg.io/)
